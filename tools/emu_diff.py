#!/usr/bin/env python3
"""Differential emulation: run one retail function and one compiled candidate on the same inputs.

Both bodies run under unicorn inside the retail image. The candidate object's sections are
copied to a scratch address and its relocations are resolved, in this order, through
the case's ``BINDINGS``, the repository's ``*.bindings.json`` files and calibration cases
(only symbols bound to exactly one VA), the VC6 CRT atlas (``work/vc6-crt-atlas/atlas.json``,
optional) and the object's own definitions. A defined symbol that is also bound uses the
retail address, so callees and globals are retail's in both runs; ``PREFER_OBJECT`` in a case
keeps the object's copy instead. Every case runs once per x87 control word (53-bit and 24-bit
precision control by default, both round-to-nearest) on freshly mapped memory.

Each run records the final value of every byte the function wrote outside its own stack frame
(heap, retail globals, arguments), the calls it made into stubs (name, ECX, stack arguments),
EAX (and ST0 when the case declares a float return), the ESP balance, the callee-saved
registers and the x87 stack top. Two runs agree when all of these are equal. A fault or the
instruction limit is a result like any other, so a case where both bodies fault identically
counts as agreement; ``retail_faults`` in the summary flags such cases, because they usually
mean the case built incomplete inputs.

A case module (``tools/emu_cases/<name>.py`` or any path) supplies:

    RETAIL_VA = 0x004a9050           # retail entry
    SYMBOL = '?Method_0x004a9050@Character@@QAEXHHHHMMM@Z'
    SOURCE = 'samples/physics/motion/CharacterMotionFourPose.cpp'   # default object lookup
    BINDINGS = {'?Helper@@YAXXZ': 0x4a8470}   # optional, name -> retail VA
    PREFER_OBJECT = set()            # optional, defined symbols to keep from the object
    PROBES = {0x4a91d5: 'fistp'}     # optional retail VAs whose hit counts are reported
    FPCWS = (0x027f, 0x007f)         # optional control words
    def cases(rng, n): ...           # yields picklable case descriptions
    def setup(ctx, case): ...        # builds memory and stubs, returns Call(...)

``setup`` runs once per run with ``ctx.rng`` seeded from the case, so retail and candidate
see identical memory. See ``Context`` for the helpers (alloc, write, stub, stub_at, ...).

    python tools/emu_diff.py four_pose [-n 300] [--object OBJ | --flags "/QIfist /Op"]

Without ``--object`` the object comes from the newest ``work/game-build`` build, or, with
``--flags`` (or ``--compile``), from compiling ``SOURCE`` with tools/compile.py the way the
game build does (``--flags`` replaces the configured extra flags). Agreement is behavioural
evidence for the inputs tried, not proof of equivalence.
"""
from __future__ import annotations

import argparse
import importlib.util
import json
import os
import random
import struct
import subprocess
import sys
import tempfile
from dataclasses import dataclass, field
from pathlib import Path
from typing import Any, Callable

ROOT = Path(__file__).resolve().parents[1]
if str(ROOT) not in sys.path:
    sys.path.insert(0, str(ROOT))

from mcm2tool.coff import CoffObject  # noqa: E402
from mcm2tool.pe import PEImage  # noqa: E402

CASES_DIR = ROOT / 'tools' / 'emu_cases'
DEFAULT_RETAIL = ROOT / 'work' / 'game' / 'mcm2.exe'
DEFAULT_ATLAS = ROOT / 'work' / 'vc6-crt-atlas' / 'atlas.json'
DEFAULT_FPCWS = (0x027f, 0x007f)

CAND = 0x20000000      # placed candidate sections
CAND_SIZE = 0x200000
STUB = 0x21000000      # stub thunks and the return trampoline
STUB_SIZE = 0x40000
STACK_TOP = 0x30000000
STACK_SIZE = 0x100000
HEAP = 0x40000000
HEAP_SIZE = 0x1000000
STACK_FILL = 0xcd

# LIBCMT entry points used when no CRT atlas is available (atlas libcmt matches).
CRT_FALLBACK = {
    '__ftol': 0x534a88,
    '__control87': 0x545b28,
}

REL32 = 0x14
DIR32 = 0x06


def u32(v: int) -> int:
    return v & 0xffffffff


def f32(x: float) -> float:
    """Round a Python float to binary32."""
    return struct.unpack('<f', struct.pack('<f', x))[0]


class Double(float):
    """Marks a float argument that is passed as a 64-bit double."""


def pack_args(args) -> bytes:
    """Pack stack arguments: int -> dword, float -> binary32, Double -> binary64, bytes as is."""
    out = bytearray()
    for a in args:
        if isinstance(a, (bytes, bytearray)):
            out += a
        elif isinstance(a, Double):
            out += struct.pack('<d', a)
        elif isinstance(a, float):
            out += struct.pack('<f', a)
        else:
            out += struct.pack('<I', u32(int(a)))
    return bytes(out)


# ---------------------------------------------------------------- bindings

def repo_bindings(root: Path = ROOT) -> dict[str, int]:
    """Symbol -> retail VA for symbols that bindings files and calibration bind to one VA."""
    from tools.audit_game_vtables import load_bindings
    return {k: next(iter(v)) for k, v in load_bindings(root).items() if len(v) == 1}


def crt_bindings(atlas: Path | None = None) -> dict[str, int]:
    """LIBCMT symbol -> retail VA from the CRT atlas, else the small fallback table."""
    out = dict(CRT_FALLBACK)
    candidates = [atlas] if atlas else [DEFAULT_ATLAS]
    env = os.environ.get('MCM2_CRT_ATLAS')
    if env:
        candidates.insert(0, Path(env))
    for path in candidates:
        if path and path.is_file():
            data = json.loads(path.read_text(encoding='utf-8'))
            for m in data.get('libcmt', {}).get('matches', []):
                if m.get('matching_archive_entries', 1) == 1 and m.get('section') == '.text':
                    out.setdefault(m['symbol'], m['target_va'])
            break
    return out


# ---------------------------------------------------------------- placement

class PlacementError(RuntimeError):
    pass


class Placement:
    """Copy the sections an entry symbol reaches into one blob at ``base``, relocated."""

    def __init__(self, obj: CoffObject, entry: str, bindings: dict[str, int],
                 fallback: dict[str, int] | None = None, prefer_object: set[str] = frozenset(),
                 base: int = CAND, strict: bool = True):
        self.obj = obj
        self.base = base
        self.bindings = bindings
        self.fallback = fallback or {}
        self.prefer_object = set(prefer_object)
        self.blob = bytearray()
        self.section_at: dict[int, int] = {}
        self.common_at: dict[str, int] = {}
        self.resolved: dict[str, tuple[str, int]] = {}
        self.unbound: set[str] = set()
        self.entry_symbol = obj.find_symbol(entry)
        self._pending: list[int] = []
        self.entry = self._defined_address(self.entry_symbol)
        while self._pending:
            self._relocate(self._pending.pop())
        if self.unbound and strict:
            raise PlacementError('unbound symbols: ' + ', '.join(sorted(self.unbound)))

    def _place_section(self, number: int) -> int:
        if number in self.section_at:
            return self.section_at[number]
        sec = self.obj.section(number)
        if sec.name.startswith('.debug') or sec.name == '.drectve':
            raise PlacementError(f'relocation into {sec.name}')
        while len(self.blob) % 16:
            self.blob.append(0xcc)
        at = len(self.blob)
        if sec.raw_ptr and not sec.characteristics & 0x80:
            self.blob += self.obj.data[sec.raw_ptr:sec.raw_ptr + sec.raw_size]
        else:
            self.blob += bytes(sec.raw_size)
        self.section_at[number] = self.base + at
        self._pending.append(number)
        return self.base + at

    def _defined_address(self, s) -> int:
        return self._place_section(s.section_number) + s.value

    def address(self, s) -> int:
        name = s.name
        if name in self.bindings and name not in self.prefer_object:
            self.resolved[name] = ('binding', self.bindings[name])
            return self.bindings[name]
        if s.section_number > 0:
            if s.storage_class == 2 and name in self.fallback and name not in self.prefer_object:
                self.resolved[name] = ('repo binding', self.fallback[name])
                return self.fallback[name]
            return self._defined_address(s)
        if s.section_number == -1:
            return s.value
        if name in self.fallback:
            self.resolved[name] = ('repo binding', self.fallback[name])
            return self.fallback[name]
        if s.value:  # common symbol: zeroed storage of that size
            if name not in self.common_at:
                while len(self.blob) % 16:
                    self.blob.append(0)
                self.common_at[name] = self.base + len(self.blob)
                self.blob += bytes(s.value)
            return self.common_at[name]
        self.unbound.add(name)
        return 0

    def _relocate(self, number: int):
        at0 = self.section_at[number] - self.base
        for r in self.obj.relocations:
            if r.section_number != number:
                continue
            target = self.address(self.obj.symbol_by_index[r.symbol_index])
            at = at0 + r.virtual_address
            addend = struct.unpack_from('<i', self.blob, at)[0]
            if r.type == DIR32:
                struct.pack_into('<I', self.blob, at, u32(target + addend))
            elif r.type == REL32:
                struct.pack_into('<I', self.blob, at, u32(target + addend - (self.base + at + 4)))
            else:
                raise PlacementError(f'relocation type {r.type:#x} in section {number}')


# ---------------------------------------------------------------- run context

@dataclass
class Call:
    """How to enter the function: ECX (thiscall), stack arguments and the return kind."""
    args: Any = ()
    ecx: int | None = None
    ret_float: bool = False     # read ST0 (as an 80-bit value) after return
    ret_edx: bool = False       # include EDX (64-bit integer returns)
    ret_eax: bool = True


class Context:
    """Per-run state handed to a case's ``setup``."""

    def __init__(self, uc, rng: random.Random):
        self.uc = uc
        self.rng = rng
        self.heap = HEAP
        self.stub_next = STUB + 0x100
        self.calls: list[tuple] = []
        self.stubs: dict[int, tuple] = {}
        self.data: dict[str, Any] = {}

    # memory -------------------------------------------------------
    def alloc(self, size: int, data: bytes | None = None, align: int = 16, fill: int = 0) -> int:
        while self.heap % align:
            self.heap += 1
        at = self.heap
        self.heap += max(size, 1)
        if self.heap > HEAP + HEAP_SIZE:
            raise MemoryError('emulated heap exhausted')
        if data is not None:
            self.uc.mem_write(at, bytes(data))
        elif fill:
            self.uc.mem_write(at, bytes([fill]) * size)
        return at

    def write(self, addr: int, data: bytes):
        self.uc.mem_write(addr, bytes(data))

    def pack(self, addr: int, fmt: str, *vals):
        self.uc.mem_write(addr, struct.pack('<' + fmt, *vals))

    def read(self, addr: int, size: int) -> bytes:
        return bytes(self.uc.mem_read(addr, size))

    def unpack(self, addr: int, fmt: str):
        fmt = '<' + fmt
        return struct.unpack(fmt, self.read(addr, struct.calcsize(fmt)))

    # stubs --------------------------------------------------------
    def stub(self, name: str, pop: int = 0, nargs: int | None = None,
             ret: int | Callable | None = 0, ret_float: str | None = None, log: bool = True) -> int:
        """Create a stub thunk and return its address.

        ``pop`` is the callee-popped byte count (stdcall/thiscall), ``nargs`` how many stack
        dwords to log (default pop // 4). ``ret`` is EAX or ``ret(ctx, ecx, args)`` returning
        EAX, (EAX, float) or None; ``ret_float`` ('f' or 'd') makes the thunk load a float
        return into ST0. Calls are logged in order and compared between runs.
        """
        at = self.stub_next
        self.stub_next += 0x40
        slot = at + 0x30
        code = bytearray(b'\x90')                                   # hook site
        code += b'\xa1' + struct.pack('<I', slot)                   # mov eax, [slot]
        if ret_float == 'f':
            code += b'\xd9\x05' + struct.pack('<I', slot + 4)       # fld dword [slot+4]
        elif ret_float == 'd':
            code += b'\xdd\x05' + struct.pack('<I', slot + 4)       # fld qword [slot+4]
        code += (b'\xc2' + struct.pack('<H', pop)) if pop else b'\xc3'
        self.uc.mem_write(at, bytes(code))
        self.stubs[at] = (name, pop, pop // 4 if nargs is None else nargs, ret, ret_float, slot, log)
        return at

    def stub_at(self, va: int, name: str, **kw) -> int:
        """Redirect a retail function (or any code address) to a new stub."""
        thunk = self.stub(name, **kw)
        self.uc.mem_write(va, b'\xe9' + struct.pack('<i', thunk - (va + 5)))
        return thunk

    def note(self, *items):
        """Append extra data to the compared call log (e.g. a buffer a stub was handed)."""
        self.calls.append(('note',) + tuple(items))

    def vtable(self, entries: list[int]) -> int:
        return self.alloc(4 * len(entries), struct.pack('<%dI' % len(entries), *entries))

    def _enter_stub(self, at: int):
        name, pop, nargs, ret, ret_float, slot, log = self.stubs[at]
        esp = self.uc.reg_read(_reg('ESP'))
        ecx = self.uc.reg_read(_reg('ECX'))
        args = struct.unpack('<%dI' % nargs, self.read(esp + 4, 4 * nargs)) if nargs else ()
        if log:
            # Addresses inside the emulated stack depend on frame layout, not behaviour.
            self.calls.append((name, _frame_token(ecx), tuple(_frame_token(a) for a in args)))
        value = ret(self, ecx, args) if callable(ret) else ret
        eax, fval = (value if isinstance(value, tuple) else (value, 0.0))
        self.uc.mem_write(slot, struct.pack('<I', u32(eax or 0)))
        if ret_float == 'f':
            self.uc.mem_write(slot + 4, struct.pack('<f', fval))
        elif ret_float == 'd':
            self.uc.mem_write(slot + 4, struct.pack('<d', fval))


def _frame_token(value: int):
    return 'stack' if STACK_TOP - STACK_SIZE <= value < STACK_TOP else value


def _reg(name: str) -> int:
    from unicorn import x86_const
    return getattr(x86_const, 'UC_X86_REG_' + name)


# ---------------------------------------------------------------- emulator

@dataclass
class RunResult:
    outcome: tuple
    probes: dict[int, int] = field(default_factory=dict)
    info: dict = field(default_factory=dict)

    @property
    def faulted(self) -> bool:
        return self.outcome[0] != 'ret'


class Emulator:
    def __init__(self, pe: PEImage, placement: Placement | None, max_insns: int = 20_000_000):
        self.pe = pe
        self.placement = placement
        self.max_insns = max_insns
        self.sections = []
        for s in pe.sections:
            raw = pe.data[s.raw_offset:s.raw_offset + min(s.raw_size, s.virtual_size or s.raw_size)]
            self.sections.append((pe.image_base + s.virtual_address, raw))

    def _new_uc(self):
        from unicorn import Uc, UC_ARCH_X86, UC_MODE_32
        uc = Uc(UC_ARCH_X86, UC_MODE_32)
        base = self.pe.image_base
        uc.mem_map(base, (self.pe.size_of_image + 0xfff) & ~0xfff)
        uc.mem_write(base, self.pe.data[:self.pe.size_of_headers])
        for va, raw in self.sections:
            uc.mem_write(va, raw)
        uc.mem_map(CAND, CAND_SIZE)
        if self.placement is not None:
            uc.mem_write(CAND, bytes(self.placement.blob))
        uc.mem_map(STUB, STUB_SIZE)
        uc.mem_map(STACK_TOP - STACK_SIZE, STACK_SIZE)
        uc.mem_write(STACK_TOP - STACK_SIZE, bytes([STACK_FILL]) * STACK_SIZE)
        uc.mem_map(HEAP, HEAP_SIZE)
        return uc

    def run(self, entry: int, setup: Callable, case, fpcw: int, seed, probes=()) -> RunResult:
        from unicorn import UC_HOOK_CODE, UC_HOOK_MEM_WRITE, UcError
        uc = self._new_uc()
        ctx = Context(uc, random.Random(repr(seed)))
        call = setup(ctx, case)
        if not isinstance(call, Call):
            call = Call(**call) if isinstance(call, dict) else Call(args=call)
        ret = STUB
        tail = bytearray()
        if call.ret_float:
            tail += b'\xdb\x3d' + struct.pack('<I', STUB + 0x80)      # fstp tbyte [STUB+0x80]
        tail += b'\xf4'                                              # hlt (never reached)
        uc.mem_write(ret, bytes(tail))
        stop = ret + len(tail) - 1

        stackargs = pack_args(call.args) if not isinstance(call.args, (bytes, bytearray)) else bytes(call.args)
        sp = STACK_TOP - 0x1000 - len(stackargs)
        uc.mem_write(sp, stackargs)
        sp -= 4
        uc.mem_write(sp, struct.pack('<I', ret))
        regs = {'EAX': 0x0badf00d, 'EBX': 0x11111111, 'ECX': call.ecx if call.ecx is not None else 0x0bad0ecc,
                'EDX': 0x0bad0edd, 'ESI': 0x22222222, 'EDI': 0x33333333, 'EBP': 0x44444444, 'ESP': sp}
        for k, v in regs.items():
            uc.reg_write(_reg(k), u32(v))
        uc.reg_write(_reg('FPCW'), fpcw)

        written: set[int] = set()

        def on_write(uc_, access, address, size, value, user):
            written.update(range(address, address + size))
        uc.hook_add(UC_HOOK_MEM_WRITE, on_write)
        for at in ctx.stubs:
            uc.hook_add(UC_HOOK_CODE, lambda u, a, s, d: ctx._enter_stub(a), begin=at, end=at)
        hits = {va: 0 for va in probes}

        def on_probe(u, a, s, d):
            hits[a] += 1
        for va in probes:
            uc.hook_add(UC_HOOK_CODE, on_probe, begin=va, end=va)

        try:
            uc.emu_start(entry, stop, count=self.max_insns)
            eip = uc.reg_read(_reg('EIP'))
            status = ('ret',) if eip == stop else ('limit', eip)
        except UcError as e:
            status = ('fault', str(e))
        info = {'eip': uc.reg_read(_reg('EIP'))}
        frame_low = STACK_TOP - STACK_SIZE
        # The callee owns its frame and its incoming argument slots; neither is observable.
        frame_high = sp + 4 + len(stackargs)
        out_bytes = {a: None for a in written
                     if not (frame_low <= a < frame_high or STUB <= a < STUB + STUB_SIZE)}
        data = {}
        for a in sorted(out_bytes):
            try:
                data[a] = uc.mem_read(a, 1)[0]
            except UcError:
                pass    # the faulting write itself; the fault status records it
        regs_after = {k: uc.reg_read(_reg(k)) for k in ('EBX', 'ESI', 'EDI', 'EBP')}
        fpsw = uc.reg_read(_reg('FPSW'))
        outcome = [status[0]]
        if status[0] == 'fault':
            outcome.append(status[1])
        outcome.append(('eax', uc.reg_read(_reg('EAX')) if call.ret_eax else None))
        if call.ret_edx:
            outcome.append(('edx', uc.reg_read(_reg('EDX'))))
        if call.ret_float and status[0] == 'ret':
            outcome.append(('st0', bytes(uc.mem_read(STUB + 0x80, 10)).hex()))
        outcome.append(('esp', u32(uc.reg_read(_reg('ESP')) - sp)))
        outcome.append(('saved', tuple(regs_after[k] == regs[k] for k in regs_after)))
        outcome.append(('x87top', (fpsw >> 11) & 7))
        outcome.append(('calls', tuple(ctx.calls)))
        outcome.append(('writes', tuple(_ranges(data))))
        return RunResult(tuple(outcome), hits, info)


def _ranges(data: dict[int, int]):
    """Collapse address -> byte into (start, hex bytes) runs."""
    run_start, run = None, bytearray()
    prev = None
    for a in sorted(data):
        if prev is not None and a == prev + 1:
            run.append(data[a])
        else:
            if run_start is not None:
                yield (run_start, run.hex())
            run_start, run = a, bytearray([data[a]])
        prev = a
    if run_start is not None:
        yield (run_start, run.hex())


def describe_difference(a: tuple, b: tuple) -> list[str]:
    """Human-readable differences between two run outcomes."""
    out = []
    da = {x[0]: x[1] for x in a[1:] if isinstance(x, tuple) and len(x) == 2 and isinstance(x[0], str)}
    db = {x[0]: x[1] for x in b[1:] if isinstance(x, tuple) and len(x) == 2 and isinstance(x[0], str)}
    if a[0] != b[0] or (a[0] == 'fault' and a[1] != b[1]):
        out.append(f'status retail={a[:2]} candidate={b[:2]}')
    for key in sorted(set(da) | set(db)):
        if da.get(key) == db.get(key):
            continue
        if key == 'writes':
            wa, wb = dict(da.get(key, ())), dict(db.get(key, ()))
            ba = {s + i: h[2 * i:2 * i + 2] for s, h in wa.items() for i in range(len(h) // 2)}
            bb = {s + i: h[2 * i:2 * i + 2] for s, h in wb.items() for i in range(len(h) // 2)}
            diff = sorted(x for x in set(ba) | set(bb) if ba.get(x) != bb.get(x))
            shown = ', '.join(f'{x:#x}:{ba.get(x, "--")}/{bb.get(x, "--")}' for x in diff[:12])
            out.append(f'writes differ at {len(diff)} bytes (retail/candidate): {shown}')
        elif key == 'calls':
            ca, cb = da.get(key, ()), db.get(key, ())
            for i, (x, y) in enumerate(zip(ca, cb)):
                if x != y:
                    out.append(f'call {i} retail={_fmt_call(x)} candidate={_fmt_call(y)}')
                    break
            else:
                out.append(f'call count retail={len(ca)} candidate={len(cb)}')
        else:
            out.append(f'{key} retail={da.get(key)!r} candidate={db.get(key)!r}')
    return out


def _fmt_call(c):
    if c[0] == 'note':
        return repr(c)
    name, ecx, args = c
    return f'{name}(ecx={_hex(ecx)}, ' + ', '.join(_hex(x) for x in args) + ')'


def _hex(x):
    return f'{x:#x}' if isinstance(x, int) else str(x)


# ---------------------------------------------------------------- cases, objects

def load_case(name: str):
    path = Path(name)
    if not path.suffix:
        path = CASES_DIR / (name + '.py')
    if not path.is_file():
        raise SystemExit(f'no case module {name}')
    spec = importlib.util.spec_from_file_location('emu_case_' + path.stem, path)
    module = importlib.util.module_from_spec(spec)
    spec.loader.exec_module(module)
    return module


def latest_build_objects() -> Path | None:
    builds = sorted((ROOT / 'work' / 'game-build').glob('build-*/report.json'), key=lambda p: p.stat().st_mtime)
    return builds[-1].parent / 'objects' if builds else None


def configured_flags(source: str) -> list[str]:
    config = json.loads((ROOT / 'config' / 'game_build.json').read_text(encoding='utf-8'))
    for key in ('candidate_sources', 'bringup_flags'):
        for entry in config.get(key, []):
            if entry.get('path') == source:
                return list(entry.get('extra_flags', []))
    return []


def compile_source(source: str, out: Path, flags: list[str] | None = None,
                   vc6_root: str | None = None) -> Path:
    """Compile one source as the game build does (vc6_o2_mt, game include paths)."""
    vc6_root = vc6_root or os.environ.get('VC6_ROOT')
    if not vc6_root:
        raise SystemExit('VC6_ROOT is required to compile')
    flags = configured_flags(source) if flags is None else flags
    command = [sys.executable, str(ROOT / 'tools' / 'compile.py'), str(ROOT / source), '-o', str(out),
               '--compiler', 'vc6', '--vc6-root', vc6_root, '--profile', 'vc6_o2_mt',
               '--include', str(ROOT / 'src' / 'krusty2'), '--include', str(ROOT / 'src' / 'reconstructed')]
    for f in flags:
        command += ['--extra', f]
    result = subprocess.run(command, cwd=ROOT, capture_output=True, text=True, encoding='latin-1',
                            env={**os.environ, 'PYTHONPATH': str(ROOT)})
    if result.returncode:
        raise SystemExit(result.stdout + result.stderr)
    return out


# ---------------------------------------------------------------- driver

@dataclass
class Summary:
    case: str
    runs: int = 0
    mismatches: int = 0
    retail_faults: int = 0
    examples: list = field(default_factory=list)
    probes: dict = field(default_factory=dict)
    by_fpcw: dict = field(default_factory=dict)

    def as_dict(self):
        return {'case': self.case, 'runs': self.runs, 'mismatches': self.mismatches,
                'retail_faults': self.retail_faults, 'by_fpcw': {hex(k): v for k, v in self.by_fpcw.items()},
                'probes': {hex(k): v for k, v in self.probes.items()}, 'examples': self.examples}


def build_placement(module, obj_path: Path, symbol: str | None = None, atlas: Path | None = None,
                    strict: bool = True) -> Placement:
    obj = CoffObject(obj_path)
    fallback = dict(crt_bindings(atlas))
    fallback.update(repo_bindings())
    entry = symbol or module.SYMBOL
    # The entry symbol itself is always the object's body.
    prefer = set(getattr(module, 'PREFER_OBJECT', ())) | {obj.find_symbol(entry).name}
    return Placement(obj, entry, dict(getattr(module, 'BINDINGS', {})), fallback, prefer, strict=strict)


def run_diff(module, obj_path: Path, n: int = 200, seed: int = 1, fpcws=None, retail: Path | None = None,
             symbol: str | None = None, retail_va: int | None = None, show: int = 5,
             atlas: Path | None = None) -> Summary:
    pe = PEImage(retail or os.environ.get('MCM2_EXE') or DEFAULT_RETAIL)
    placement = build_placement(module, obj_path, symbol, atlas)
    emu = Emulator(pe, placement, getattr(module, 'MAX_INSNS', 20_000_000))
    retail_va = retail_va or module.RETAIL_VA
    fpcws = fpcws or getattr(module, 'FPCWS', DEFAULT_FPCWS)
    probes = dict(getattr(module, 'PROBES', {}))
    summary = Summary(getattr(module, '__name__', 'case'), probes={va: 0 for va in probes})
    rng = random.Random(seed)
    case_list = list(module.cases(rng, n))
    for fpcw in fpcws:
        bad = 0
        for index, case in enumerate(case_list):
            r = emu.run(retail_va, module.setup, case, fpcw, (seed, index), probes)
            c = emu.run(placement.entry, module.setup, case, fpcw, (seed, index))
            summary.runs += 1
            for va, hits in r.probes.items():
                summary.probes[va] += bool(hits)
            if r.faulted:
                summary.retail_faults += 1
            if r.outcome != c.outcome:
                bad += 1
                summary.mismatches += 1
                if len(summary.examples) < show:
                    summary.examples.append({'fpcw': hex(fpcw), 'case': repr(case),
                                             'differences': describe_difference(r.outcome, c.outcome)})
        summary.by_fpcw[fpcw] = {'runs': len(case_list), 'mismatches': bad}
    return summary


def explain(module, obj_path: Path, symbol=None, retail_va=None, retail=None, atlas=None):
    """Print the candidate's resolved externals and the retail body's direct call targets."""
    placement = build_placement(module, obj_path, symbol, atlas, strict=False)
    for name in sorted(placement.unbound):
        print(f'  UNBOUND {name}')
    print(f'candidate entry {placement.entry:#x}; sections placed {len(placement.section_at)}')
    for name, (how, va) in sorted(placement.resolved.items(), key=lambda x: x[1][1]):
        print(f'  {va:#010x} {how:12} {name}')
    retail_va = retail_va or module.RETAIL_VA
    pe = PEImage(retail or os.environ.get('MCM2_EXE') or DEFAULT_RETAIL)
    from tools.disasm_fn import disassemble
    insns, end, _, _ = disassemble(pe, retail_va)
    print(f'retail {retail_va:#x}..{end:#x} calls:')
    for ins in insns:
        if ins.mnemonic == 'call':
            print(f'  {ins.address:#x}: call {ins.op_str}')

def main(argv=None) -> int:
    parser = argparse.ArgumentParser(description=__doc__.splitlines()[0])
    parser.add_argument('case', help='case module name under tools/emu_cases, or a path')
    parser.add_argument('-n', type=int, default=200, help='random cases (modules may add fixed ones)')
    parser.add_argument('--seed', type=int, default=1)
    parser.add_argument('--object', type=Path, help='candidate object (default: newest game build)')
    parser.add_argument('--compile', action='store_true', help='compile SOURCE with its configured flags')
    parser.add_argument('--flags', help='compile SOURCE with these extra flags instead (e.g. "/QIfist /Op")')
    parser.add_argument('--symbol', help='override the case SYMBOL')
    parser.add_argument('--va', type=lambda s: int(s, 0), help='override the case RETAIL_VA')
    parser.add_argument('--retail', type=Path)
    parser.add_argument('--atlas', type=Path, help='VC6 CRT atlas.json (default work/vc6-crt-atlas)')
    parser.add_argument('--fpcw', help='comma-separated control words (default 0x27f,0x7f)')
    parser.add_argument('--show', type=int, default=5)
    parser.add_argument('--json-out', type=Path)
    parser.add_argument('--explain', action='store_true',
                        help='list how relocations resolved and the retail direct calls, then exit')
    args = parser.parse_args(argv)
    module = load_case(args.case)
    fpcws = tuple(int(x, 0) for x in args.fpcw.split(',')) if args.fpcw else None
    with tempfile.TemporaryDirectory(prefix='emu-diff-') as tmp:
        if args.object:
            obj = args.object
        elif args.compile or args.flags is not None:
            flags = args.flags.split() if args.flags is not None else None
            obj = compile_source(module.SOURCE, Path(tmp) / 'candidate.obj', flags)
        else:
            objects = latest_build_objects()
            if objects is None:
                raise SystemExit('no game build; pass --object or --compile')
            obj = objects / Path(module.SOURCE).with_suffix('.obj')
        if args.explain:
            explain(module, obj, args.symbol, args.va, args.retail, args.atlas)
            return 0
        summary = run_diff(module, obj, args.n, args.seed, fpcws, args.retail, args.symbol, args.va,
                           args.show, args.atlas)
    for ex in summary.examples:
        print(f"DIFF fpcw={ex['fpcw']} case={ex['case'][:200]}")
        for line in ex['differences']:
            print('   ', line)
    for fpcw, row in summary.by_fpcw.items():
        print(f'fpcw {fpcw:#06x}: {row["mismatches"]} of {row["runs"]} differ')
    for va, hits in summary.probes.items():
        print(f'probe {va:#x} {getattr(module, "PROBES")[va]}: reached in {hits} retail runs')
    print(f'{summary.case}: runs {summary.runs}, mismatches {summary.mismatches}, '
          f'retail faults {summary.retail_faults}')
    if args.json_out:
        args.json_out.write_text(json.dumps(summary.as_dict(), indent=2) + '\n', encoding='utf-8')
    return 1 if summary.mismatches else 0


if __name__ == '__main__':
    raise SystemExit(main())
