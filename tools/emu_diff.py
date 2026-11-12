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
    SYMBOL = '?ApplyFourMotionBlendAtPhase@Character@@QAEXHHHHMMM@Z'
    SOURCE = 'samples/physics/motion/CharacterMotionFourPose.cpp'   # default object lookup
    BINDINGS = {'?Helper@@YAXXZ': 0x4a8470}   # optional, name -> retail VA
    PREFER_OBJECT = set()            # optional, defined symbols to keep from the object
    PROBES = {0x4a91d5: 'fistp'}     # optional retail VAs whose hit counts are reported
    FPCWS = (0x027f, 0x007f)         # optional control words
    FLOAT_ULPS = 0                   # optional: outcomes whose only differences are float32
                                     # words (writes, logged call arguments) within this many
                                     # ulps count as ``within_ulps``, not as mismatches; a dict
                                     # {fpcw: ulps} keeps the other control words exact
    FLOAT_ABS = 0.0                  # optional, with FLOAT_ULPS: words this close in value also
                                     # agree (cancellation near zero, where ulps mean little)
    def cases(rng, n): ...           # yields picklable case descriptions
    def setup(ctx, case): ...        # builds memory and stubs, returns Call(...)

``setup`` runs once per run with ``ctx.rng`` seeded from the case, so retail and candidate
see identical memory. See ``Context`` for the helpers (alloc, write, stub, stub_at, ...);
``ctx.entry`` is the address under test (retail VA or placed candidate), for vtables that
recurse into it, and ``ctx.call`` runs code such as a retail .bss table initializer.

    python tools/emu_diff.py four_pose [-n 300] [--object OBJ | --flags "/QIfist /Op"]

Without ``--object`` the object comes from the newest ``work/game-build`` build, or, with
``--flags`` (or ``--compile``), from compiling ``SOURCE`` with tools/compile.py the way the
game build does (``--flags`` replaces the configured extra flags). Agreement is behavioural
evidence for the inputs tried, not proof of equivalence.

Call-record/replay mode (opt-in: the case declares ``REPLAY = dict(...)``, see ``Replay``) is
for large bodies whose callees cannot all be stubbed by hand. Retail runs first; in both runs
every call that leaves the body (direct targets outside it, register and memory-indirect
calls, so virtual calls resolve through the object's real or fake vtable) is logged and
skipped. The candidate's relocations name its targets by retail VA, so the two logs compare
directly; a virtual call reads the object's retail vtable in both runs. Each logged call holds
the target, ECX for member calls, the stack argument words (count from the demangled
signature, else the retail callee's ``ret N``), the pointee bytes of const pointer arguments,
the x87 depth and optionally ST0. The callee's result (EAX, or ST0 through a trampoline that
loads it) and its out-parameter writes are synthetic, seeded from (seed, target, occurrence),
so both bodies see the same callee behaviour and the comparison sees the body alone: the
call sequence, the final writes, EAX and the stack. Synthetic pointers point into a seeded
scratch arena (``ARENA``). Cases declare callee behaviour that matters (``effects``,
callable ``returns``), fake vtables for unmodelled objects (``Context.fake_vtable``) and,
through ``Call(ignore=...)``, write ranges left out of the comparison by design. A mismatch
report names the first diverging call. ``--explain`` lists how each retail direct call
would be replayed.
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
FAKE = 0x22000000      # fake code addresses (Context.fake_vtable); never executed
FAKE_SIZE = 0x10000
ARENA = 0x50000000     # replay mode: seeded scratch memory behind synthetic pointers
ARENA_SIZE = 0x40000
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
    ignore: Any = ()            # (address, size) ranges left out of the write comparison


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
        self.entry: int | None = None   # the body under test, set before setup runs
        self.patched: dict[int, int] = {}            # stub_at: code VA -> stub thunk
        self.fakes: dict[int, tuple] = {}            # fake code VA -> (label, pop, return kind)
        self.fake_next = FAKE

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
        self.patched[va] = thunk
        return thunk

    def fake_vtable(self, label: str, count: int, pops: dict[int, int] | None = None,
                    returns: dict[int, Any] | None = None) -> int:
        """Replay mode: a vtable of ``count`` distinct fake code addresses named ``label slot N``.

        The addresses are never executed: a call to one is recorded and replayed like any other
        callee. ``pops`` gives callee-popped bytes per slot (default 0), ``returns`` a return
        kind per slot (see ``Replay``; default 'mixed').
        """
        entries = []
        for k in range(count):
            at = self.fake_next
            self.fake_next += 4
            self.fakes[at] = ('%s slot %d' % (label, k), (pops or {}).get(k, 0),
                              (returns or {}).get(k, 'mixed'))
            entries.append(at)
        return self.vtable(entries)

    def call(self, va: int, args=(), ecx: int | None = None, fpcw: int = 0x027f,
             max_insns: int = 10_000_000) -> int:
        """Run ``va`` to its return during setup (no hooks, nothing logged); returns EAX.

        For initializers whose output the target reads, e.g. retail .bss lookup tables. It
        uses a scratch stack at the end of the stub area, which the comparison ignores.
        """
        from unicorn import UcError
        ret = STUB + STUB_SIZE - 0x10
        self.uc.mem_write(ret, bytes([0xf4]))                  # hlt (never reached)
        stackargs = pack_args(args)
        sp = STUB + STUB_SIZE - 0x100 - len(stackargs)
        self.uc.mem_write(sp, stackargs)
        sp -= 4
        self.uc.mem_write(sp, struct.pack('<I', ret))
        self.uc.reg_write(_reg('ESP'), sp)
        if ecx is not None:
            self.uc.reg_write(_reg('ECX'), u32(ecx))
        self.uc.reg_write(_reg('FPCW'), fpcw)
        try:
            self.uc.emu_start(va, ret, count=max_insns)
        except UcError as e:
            raise RuntimeError('setup call %#x faulted: %s' % (va, e)) from e
        if self.uc.reg_read(_reg('EIP')) != ret:
            raise RuntimeError('setup call %#x did not return' % va)
        return self.uc.reg_read(_reg('EAX'))

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


def _in_stack(value: int) -> bool:
    return STACK_TOP - STACK_SIZE <= value < STACK_TOP


# ---------------------------------------------------------------- MSVC signatures (replay mode)

@dataclass(frozen=True)
class MType:
    """One decoded MSVC type: kind is int, bool, float, double, void, ptr or struct."""
    kind: str
    size: int | None
    const: bool = False              # ptr/ref: the pointee is const
    pointee: 'MType | None' = None
    name: str = ''

    @property
    def words(self) -> int | None:
        """Stack dwords the type takes as an argument."""
        if self.size is None:
            return None
        return max(1, (self.size + 3) // 4)


@dataclass(frozen=True)
class Signature:
    convention: str                  # cdecl, thiscall, stdcall, fastcall, pascal
    member: bool                     # has a this pointer
    ret: MType
    params: tuple
    variadic: bool = False

    def hidden_return(self) -> bool:
        """The caller passes the result buffer as the first stack argument (returned in EAX)."""
        if self.ret.kind != 'struct':
            return False
        return self.member or self.ret.size not in (1, 2, 4, 8)

    def stack_words(self) -> int | None:
        total = 1 if self.hidden_return() else 0
        for p in self.params:
            if p.words is None:
                return None
            total += p.words
        return total


DEFAULT_STRUCT_SIZES = {'Vec3': 12, 'GridVec3': 12}
_SIMPLE_TYPES = {'C': ('int', 1), 'D': ('int', 1), 'E': ('int', 1), 'F': ('int', 2), 'G': ('int', 2),
                 'H': ('int', 4), 'I': ('int', 4), 'J': ('int', 4), 'K': ('int', 4), 'M': ('float', 4),
                 'N': ('double', 8), 'O': ('double', 8), 'X': ('void', 0)}
_EXT_TYPES = {'N': ('bool', 1), 'J': ('int', 8), 'K': ('int', 8), 'W': ('int', 2)}
_CONVENTIONS = {'A': 'cdecl', 'C': 'pascal', 'E': 'thiscall', 'G': 'stdcall', 'I': 'fastcall'}


class _Demangler:
    """Just enough of the VC6 name decoration to read a function's parameter and return types."""

    def __init__(self, text: str, sizes: dict[str, int]):
        self.s = text
        self.i = 0
        self.sizes = sizes
        self.names: list[str] = []
        self.args: list[MType] = []

    def peek(self) -> str:
        if self.i >= len(self.s):
            raise ValueError('truncated')
        return self.s[self.i]

    def take(self) -> str:
        c = self.peek()
        self.i += 1
        return c

    def fragment(self) -> str:
        c = self.peek()
        if c.isdigit():
            self.i += 1
            return self.names[int(c)]
        if c == '?':
            raise ValueError('template or special name')
        j = self.s.index('@', self.i)
        frag = self.s[self.i:j]
        self.i = j + 1
        if len(self.names) < 10:
            self.names.append(frag)
        return frag

    def qualified(self) -> str:
        frags = []
        while self.peek() != '@':
            frags.append(self.fragment())
        self.i += 1
        return '::'.join(reversed(frags))

    def type(self, arg: bool = False) -> MType:
        start = self.i
        c = self.take()
        if arg and c.isdigit():
            return self.args[int(c)]
        if c == '_':
            kind, size = _EXT_TYPES[self.take()]
            t = MType(kind, size)
        elif c in _SIMPLE_TYPES:
            kind, size = _SIMPLE_TYPES[c]
            t = MType(kind, size)
        elif c in 'PQRSAB':
            cv = self.take()
            if cv == '6' or cv not in 'ABCD':
                raise ValueError('function pointer or unsupported pointer')
            t = MType('ptr', 4, const=cv in 'BD', pointee=self.type())
        elif c in 'UVT':
            name = self.qualified()
            t = MType('struct', self.sizes.get(name), name=name)
        elif c == 'W':
            self.take()
            t = MType('int', 4, name=self.qualified())
        else:
            raise ValueError('type code %r' % c)
        if arg and self.i - start > 1 and len(self.args) < 10:
            self.args.append(t)
        return t

    def signature(self) -> Signature:
        if self.take() != '?':
            raise ValueError('not a C++ name')
        if self.peek() == '?':
            self.i += 1
            code = self.take()
            if code == '_':
                self.take()
        else:
            self.fragment()
        while self.peek() != '@':
            self.fragment()
        self.i += 1
        code = self.take()
        if code in 'YZ':
            member = False
        elif code in 'ABEFIJMNQRUV':
            member = True
            self.take()                               # this cv
        elif code in 'CDKLST':
            member = False
        elif code in 'GHOPWX':                        # this-adjusting thunk: displacement first
            if self.peek().isdigit():
                self.i += 1
            else:
                self.i = self.s.index('@', self.i) + 1
            member = True
            self.take()
        else:
            raise ValueError('vtordisp thunk or data symbol')
        convention = _CONVENTIONS[self.take()]
        if self.peek() == '@':
            self.i += 1
            ret = MType('void', 0)
        else:
            if self.s.startswith('?A', self.i):
                self.i += 2
            ret = self.type()
        params, variadic = [], False
        if self.peek() == 'X':
            self.i += 1
        else:
            while True:
                c = self.peek()
                if c == '@':
                    break
                if c == 'Z':
                    variadic = True
                    break
                params.append(self.type(arg=True))
        return Signature(convention, member, ret, tuple(params), variadic)


def demangle_signature(name: str, sizes: dict[str, int] | None = None) -> Signature | None:
    """Decode a VC6-mangled function name's calling convention, return and parameter types."""
    try:
        return _Demangler(name, {**DEFAULT_STRUCT_SIZES, **(sizes or {})}).signature()
    except (ValueError, IndexError, KeyError):
        return None


# ---------------------------------------------------------------- call record/replay

# CRT routines that compute without touching game state; replay runs them natively.
PURE_CRT = ('__ftol', '__CIsqrt', '__CIsin', '__CIcos', '__CItan', '__CIasin', '__CIacos', '__CIatan',
            '__CIatan2', '__CIexp', '__CIlog', '__CIlog10', '__CIpow', '__CIfmod', '_sqrt', '_fabs',
            '_floor', '_ceil', '_fmod', '_atan2', '_atan', '_asin', '_acos', '_sin', '_cos', '_tan',
            '_pow', '_exp', '_log', '_finite', '__finite', '_isnan', '__isnan', '_abs', '_labs')

RETURN_KINDS = ('void', 'int', 'bool', 'ptr', 'float', 'double', 'mixed')


@dataclass
class TargetInfo:
    """What replay knows about one callee (static, shared by both runs)."""
    name: str
    signature: Signature | None = None
    pop: int = 0                       # callee-popped bytes
    nargs: int = 0                     # stack dwords logged
    thiscall: bool = True              # log ECX
    ret: Any = 'mixed'                 # return kind or callable(ctx, rec)
    float_ret: bool = False            # push ST0


@dataclass
class ReplayCall:
    """One replayed call, as handed to a case's effect callable."""
    index: int                         # position in the run's replayed calls
    target: Any                        # retail VA, fake VA, or ('iat', slot) / ('bad', ea)
    name: str
    occurrence: int                    # how many earlier calls went to the same target
    ecx: int
    esp: int                           # address of the first stack argument
    args: tuple
    rng: random.Random                 # seeded from (seed, target, occurrence)
    info: TargetInfo
    eax: int = 0
    fval: float | None = None          # ST0 result when the target returns a float

    def arg_ptr(self, k: int) -> int:
        return self.args[k]


class Replay:
    """Call-record/replay settings and per-target knowledge for one case (opt-in, ``REPLAY``).

    Every call that leaves the body under test is intercepted at the call instruction. The target
    is decoded (direct, register or memory operand, so a vtable slot resolves through the object's
    real or fake vtable), the call is logged, skipped and given a synthetic result that depends only
    on (seed, target, how many earlier calls went to that target). Both bodies therefore see the same
    callee results in the same order and the comparison checks the body alone.

    ``spec`` keys (all optional):
      passthrough  retail VAs or symbols that run natively, not logged (pure CRT math by default)
      inline       retail VAs whose bodies count as part of the function under test
      unlogged     VAs or symbols replayed but left out of the call log: a platform adaptation
                   that only one body calls (a QPC read standing in for retail's RDTSC)
      returns      VA -> 'void' | 'int' | 'bool' | 'ptr' | 'float' | 'double' | 'mixed' | callable
      pops         VA -> callee-popped bytes (default: the retail callee's ``ret N``)
      args         VA -> stack dwords to log (default: from the signature, else pop / 4)
      ecx          VA -> bool, log ECX (default: thiscall, or unknown signature)
      signatures   VA -> mangled name (default: the candidate's relocation, else bindings)
      effects      VA -> callable(ctx, rec) run after the synthetic model (out-pointer writes)
      sizes        struct name -> bytes (Vec3 is 12) for by-value, pointee and result sizes
      st0          VAs whose ST0 at the call is logged
      log_pointees bool (default True): log the pointee bytes of const pointer/reference args
                   (and of non-const ones outside the stack) whose size is known
      pointees     VA -> stack-word indexes whose pointees are logged even when non-const and
                   on the stack (in/out locals the body fills before the call); an entry may
                   be (index, bytes) to log a size the signature does not give (arrays)
      inout        VA -> stack-word indexes (or (index, bytes)) of in/out pointees: logged as
                   ``pointees`` and left out of the synthetic out-writes, so an effect reads
                   what the body passed (a ground query that keeps x and z and writes y)
      out_writes   bool (default True): fill non-const stack pointees of known size with
                   synthetic values, so out-parameters the body reads back are deterministic
    """

    def __init__(self, spec: dict, pe: PEImage | None = None, names: dict[int, list[str]] | None = None,
                 crt: dict[str, int] | None = None, slot_labels: dict[int, list[str]] | None = None):
        self.pe = pe
        self.names = names or {}
        self.slot_labels = slot_labels or {}
        self.by_name: dict[str, int] = {}
        for va, ns in self.names.items():
            for n in ns:
                if isinstance(va, int):
                    self.by_name.setdefault(n, va)
        self.by_name.update(crt or {})
        self.spec = {}
        for key, value in dict(spec or {}).items():
            if isinstance(value, dict) and key != 'sizes':
                value = {self.resolve_key(k): v for k, v in value.items()}
            elif isinstance(value, (set, frozenset, list, tuple)):
                value = type(value)(self.resolve_key(k) for k in value)
            self.spec[key] = value
        self.sizes = {**DEFAULT_STRUCT_SIZES, **self.spec.get('sizes', {})}

        def resolve(items):
            return {x for x in (self.resolve_key(k) for k in items) if isinstance(x, int)}
        self.passthrough = resolve(PURE_CRT) | resolve(self.spec.get('passthrough', ()))
        self.inline = resolve(self.spec.get('inline', ()))
        self.unlogged = resolve(self.spec.get('unlogged', ()))
        self.log_pointees = self.spec.get('log_pointees', True)
        self.out_writes = self.spec.get('out_writes', True)
        self._info: dict[Any, TargetInfo] = {}
        self._decoded: dict[tuple, Any] = {}
        self.entry_names: dict[int, list[str]] = {}   # vtable entry address -> built slot symbols

    # static knowledge ----------------------------------------------
    def resolve_key(self, key):
        """('vslot', vtable VA, slot) -> that retail vtable entry; a symbol -> its bound VA."""
        if isinstance(key, tuple) and len(key) == 3 and key[0] == 'vslot':
            if self.pe is None:
                raise ValueError('vslot keys need the retail image')
            return self.pe.u32(self.pe.va_to_offset(key[1] + 4 * key[2]))
        if isinstance(key, str) and key not in ('self',):
            return self.by_name.get(key, key)
        return key

    def label(self, va) -> str:
        if isinstance(va, tuple):
            return '%s %#x' % va
        if isinstance(va, str):
            return va
        names = self.names.get(va)
        if names:
            return names[0]
        slots = self.slot_labels.get(va)
        if slots:
            return slots[0]
        return '%#x' % va

    def retail_pop(self, va: int) -> int | None:
        """The retail callee's ``ret N`` (following jmp thunks), or None."""
        if self.pe is None or not self.pe.is_code_va(va):
            return None
        from tools.disasm_fn import disassemble
        for _ in range(4):
            try:
                insns, _, _, _ = disassemble(self.pe, va, 0x4000)
            except Exception:
                return None
            pops = {ins.operands[0].imm if ins.operands else 0 for ins in insns if ins.mnemonic in ('ret', 'retn')}
            if pops:
                return max(pops)
            last = insns[-1] if insns else None
            if last is not None and last.mnemonic == 'jmp' and last.operands and last.operands[0].type == 2:
                va = last.operands[0].imm
                continue
            return None
        return None

    def info(self, target, fakes: dict | None = None, via: int | None = None) -> TargetInfo:
        """Callee knowledge; ``via`` is the vtable entry address the call read, if any."""
        if fakes and target in fakes:
            label, pop, ret = fakes[target]
            o = self.spec
            return TargetInfo(label, None, o.get('pops', {}).get(target, pop),
                              o.get('args', {}).get(target, pop // 4), o.get('ecx', {}).get(target, True),
                              o.get('returns', {}).get(target, ret),
                              o.get('returns', {}).get(target, ret) in ('float', 'double'))
        slot_names = tuple(self.entry_names.get(via, ())) if via is not None else ()
        key = (target, slot_names)
        if key in self._info:
            return self._info[key]
        o = self.spec
        sig = None
        mangled = o.get('signatures', {}).get(target)
        # A slot's own declaration first: folded retail bodies serve several signatures.
        candidates = [mangled] if mangled else list(slot_names) + list(self.names.get(target, ()))
        for n in candidates:
            sig = demangle_signature(n, self.sizes)
            if sig is not None:
                break
        pop = o.get('pops', {}).get(target)
        if pop is None and isinstance(target, int):
            pop = self.retail_pop(target)
        if pop is None and sig is not None:
            words = sig.stack_words()
            pop = 4 * words if words is not None and sig.convention in ('thiscall', 'stdcall', 'pascal') \
                and not sig.variadic else 0
        pop = pop or 0
        nargs = o.get('args', {}).get(target)
        if nargs is None:
            words = sig.stack_words() if sig is not None else None
            nargs = words if words is not None else pop // 4
        thiscall = o.get('ecx', {}).get(target, sig.member if sig is not None else True)
        ret = o.get('returns', {}).get(target)
        if ret is None:
            ret = _return_kind(sig.ret) if sig is not None else 'mixed'
        float_ret = ret in ('float', 'double') or (callable(ret) and target in o.get('float_returns', ()))
        info = TargetInfo(self.label(target), sig, pop, nargs, thiscall, ret, float_ret)
        self._info[key] = info
        return info

    def decode_call(self, uc, address: int, size: int):
        """None if the instruction is not a call, else a decoded operand description (cached)."""
        code = bytes(uc.mem_read(address, size))
        key = (address, code)
        if key in self._decoded:
            return self._decoded[key]
        out = None
        if code[0] == 0xe8 and size == 5:
            out = ('rel', u32(address + 5 + struct.unpack_from('<i', code, 1)[0]))
        elif code[0] == 0xff and size >= 2 and (code[1] >> 3) & 7 == 2:
            import capstone
            from capstone import x86
            md = capstone.Cs(capstone.CS_ARCH_X86, capstone.CS_MODE_32)
            md.detail = True
            ins = next(md.disasm(code, address, 1))
            op = ins.operands[0]
            if op.type == x86.X86_OP_REG:
                out = ('reg', _reg(ins.reg_name(op.reg).upper()))
            else:
                m = op.mem
                out = ('mem', _reg(ins.reg_name(m.base).upper()) if m.base else None,
                       _reg(ins.reg_name(m.index).upper()) if m.index else None, m.scale, m.disp)
        self._decoded[key] = out
        return out


def _return_kind(t: MType):
    if t.kind == 'void':
        return 'void'
    if t.kind == 'bool':
        return 'bool'
    if t.kind == 'float':
        return 'float'
    if t.kind == 'double':
        return 'double'
    if t.kind == 'ptr':
        return 'ptr'
    if t.kind == 'struct':
        return 'struct'
    return 'int'


def synthetic_rng(seed, target, occurrence: int) -> random.Random:
    key = target if not isinstance(target, int) else '%#x' % target
    return random.Random('%r|%s|%d' % (seed, key, occurrence))


def synthetic_value(rng: random.Random, kind: str) -> tuple[int, float | None]:
    """EAX (and the ST0 value for float kinds) the replay model returns for one call."""
    if kind == 'void':
        return rng.getrandbits(32), None
    if kind == 'bool':
        return rng.randrange(2), None
    if kind == 'int':
        return u32(rng.choice([0, 0, 1, 1, 2, 3, -1, rng.randrange(256)])), None
    if kind == 'ptr':
        return arena_pointer(rng), None
    if kind == 'float':
        return rng.getrandbits(32), f32(synthetic_float(rng))
    if kind == 'double':
        return rng.getrandbits(32), synthetic_float(rng)
    # mixed: zero, one or a readable pointer
    return rng.choice([0, 1, arena_pointer(rng)]), None


def synthetic_float(rng: random.Random) -> float:
    return rng.choice([0.0, 1.0, -1.0, rng.uniform(-2, 2), rng.uniform(0, 1), rng.uniform(-50, 50)])


def arena_pointer(rng: random.Random) -> int:
    return ARENA + 16 * rng.randrange((ARENA_SIZE - 0x2000) // 16)


def synthetic_bytes(rng: random.Random, t: MType) -> bytes | None:
    """Synthetic contents for an out-parameter or result buffer of type ``t``."""
    if t.size is None or t.kind == 'void':
        return None
    if t.kind == 'float':
        return struct.pack('<f', synthetic_float(rng))
    if t.kind == 'double':
        return struct.pack('<d', synthetic_float(rng))
    if t.kind == 'bool':
        return bytes([rng.randrange(2)])
    if t.kind == 'ptr':
        return struct.pack('<I', arena_pointer(rng))
    if t.kind == 'int':
        return struct.pack('<q', rng.choice([0, 1, 2, 3, -1, rng.randrange(256)]))[:t.size]
    # struct: floats per dword (a Vec3 gets three finite floats)
    raw = b''.join(struct.pack('<f', synthetic_float(rng)) for _ in range((t.size + 3) // 4))
    return raw[:t.size]


def arena_bytes(seed) -> bytes:
    """Arena contents: dwords 0..2 of each 16-byte cell are floats in [-4, 4], dword 3 an arena pointer."""
    rng = random.Random('arena|%r' % (seed,))
    out = bytearray()
    for _ in range(ARENA_SIZE // 16):
        out += struct.pack('<3fI', rng.uniform(-4, 4), rng.uniform(-4, 4), rng.uniform(-4, 4), arena_pointer(rng))
    return bytes(out)


class ReplayRun:
    """The per-run interception hook for ``Replay`` (installed by ``Emulator.run``)."""

    def __init__(self, replay: Replay, ctx: 'Context', seed, entry: int):
        self.replay = replay
        self.ctx = ctx
        self.seed = seed
        self.entry = entry
        self.suspended: tuple[int, int] | None = None
        self.occurrence: dict[Any, int] = {}
        self.count = 0
        self.tramp = STUB + 0xa0
        self.slot = STUB + 0xe0
        ctx.uc.mem_write(self.tramp, b'\xdd\x05' + struct.pack('<I', self.slot)            # fld qword [slot]
                         + b'\xff\x25' + struct.pack('<I', self.slot + 8))                 # jmp [slot+8]

    def target_of(self, uc, op):
        """(target, address of the pointer the call read or None)."""
        if op[0] == 'rel':
            return op[1], None
        if op[0] == 'reg':
            return uc.reg_read(op[1]), None
        _, base, index, scale, disp = op
        ea = disp
        if base is not None:
            ea += uc.reg_read(base)
        if index is not None:
            ea += uc.reg_read(index) * scale
        ea = u32(ea)
        try:
            value = struct.unpack('<I', bytes(uc.mem_read(ea, 4)))[0]
        except Exception:
            return ('bad', ea), None
        if base is None and index is None and not self._code(value):
            return ('iat', ea), None       # import slot: name it by the slot, not its raw contents
        return value, ea

    def _code(self, va: int) -> bool:
        pe = self.replay.pe
        return (CAND <= va < CAND + CAND_SIZE or STUB <= va < STUB + STUB_SIZE or FAKE <= va < FAKE + FAKE_SIZE
                or (pe is not None and pe.is_code_va(va)))

    def hook(self, uc, address, size, _user):
        from unicorn import UcError
        if self.suspended is not None:
            ret, esp = self.suspended
            if address != ret or uc.reg_read(_reg('ESP')) < esp:
                return
            self.suspended = None
        try:
            op = self.replay.decode_call(uc, address, size)
        except UcError:
            return
        if op is None:
            return
        target, via = self.target_of(uc, op)
        replay = self.replay
        if isinstance(target, int):
            if target == self.entry:
                target = 'self'
            elif target in replay.passthrough:
                self.suspended = (address + size, uc.reg_read(_reg('ESP')))
                return
            elif (target in replay.inline or target in self.ctx.patched or STUB <= target < STUB + STUB_SIZE
                  or CAND <= target < CAND + CAND_SIZE):
                return                       # runs as part of the body (or as a case stub)
        self.replay_call(uc, address, size, target, via)

    def replay_call(self, uc, address, size, target, via=None):
        ctx, replay = self.ctx, self.replay
        info = replay.info(target, ctx.fakes, via)
        esp = uc.reg_read(_reg('ESP'))
        ecx = uc.reg_read(_reg('ECX'))
        args = struct.unpack('<%dI' % info.nargs, ctx.read(esp, 4 * info.nargs)) if info.nargs else ()
        occ = self.occurrence.get(target, 0)
        self.occurrence[target] = occ + 1
        rng = synthetic_rng(self.seed, target, occ)
        rec = ReplayCall(self.count, target, info.name, occ, ecx, esp, args, rng, info)
        self.count += 1
        extra = []
        fpsw = uc.reg_read(_reg('FPSW'))
        top = (fpsw >> 11) & 7
        depth = (8 - top) & 7
        if depth:
            extra.append(('x87depth', depth))
        if target in replay.spec.get('st0', ()):
            mant, exp = uc.reg_read(_reg('FP%d' % top))
            extra.append(('st0', '%04x%016x' % (exp, mant)))
        sig = info.signature
        params = []
        if sig is not None:
            k = 1 if sig.hidden_return() else 0
            for p in sig.params:
                if p.words is None:
                    break
                params.append((k, p))
                k += p.words
        inout = _index_sizes(replay.spec.get('inout', {}).get(target, ()))
        if replay.log_pointees:
            forced = {**_index_sizes(replay.spec.get('pointees', {}).get(target, ())), **inout}
            typed = dict(params)
            for k in sorted(set(typed) | set(forced)):
                p = typed.get(k)
                nbytes = forced.get(k) or (p.pointee.size if p is not None and p.kind == 'ptr'
                                           and p.pointee is not None else None)
                if not nbytes or k >= len(args):
                    continue
                if k in forced or p.const or not _in_stack(args[k]):
                    try:
                        extra.append(('*arg%d' % k, ctx.read(args[k], nbytes).hex()))
                    except Exception:
                        extra.append(('*arg%d' % k, 'unmapped'))
        logged = [_frame_token(a) for a in args]
        for k, p in params:
            # Sub-dword arguments: the bytes above the value are whatever the push register held.
            if p.kind in ('int', 'bool') and p.size and p.size < 4 and k < len(logged):
                logged[k] = args[k] & ((1 << (8 * p.size)) - 1)
        if target not in replay.unlogged:
            ctx.calls.append((info.name, _frame_token(ecx) if info.thiscall else None, tuple(logged), tuple(extra)))
        # synthetic results
        ret = info.ret
        if callable(ret):
            value = ret(ctx, rec)
            rec.eax, rec.fval = (value if isinstance(value, tuple) else (value, None))
            rec.eax = u32(rec.eax or 0)
        elif ret == 'struct':
            rec.eax = args[0] if args else 0
            data = synthetic_bytes(rng, sig.ret) if sig is not None else None
            if data and args and _in_stack(args[0]):
                ctx.write(args[0], data)
        else:
            rec.eax, rec.fval = synthetic_value(rng, ret)
        if replay.out_writes:
            for k, p in params:
                if (p.kind == 'ptr' and not p.const and p.pointee is not None and k < len(args)
                        and _in_stack(args[k]) and k not in inout):
                    data = synthetic_bytes(rng, p.pointee)
                    if data:
                        ctx.write(args[k], data)
        effect = replay.spec.get('effects', {}).get(target)
        if effect is not None:
            effect(ctx, rec)
        if info.float_ret and rec.fval is None:
            rec.fval = 0.0
        uc.reg_write(_reg('EAX'), u32(rec.eax))
        uc.reg_write(_reg('ECX'), 0x0bad0ecc)
        uc.reg_write(_reg('EDX'), 0x0bad0edd)
        uc.reg_write(_reg('ESP'), u32(esp + info.pop))
        if info.float_ret:
            ctx.write(self.slot, struct.pack('<dI', rec.fval, address + size))
            uc.reg_write(_reg('EIP'), self.tramp)
        else:
            uc.reg_write(_reg('EIP'), address + size)


def _index_sizes(entries) -> dict:
    """``pointees``/``inout`` entries (index or (index, bytes)) -> {index: bytes or None}."""
    out = {}
    for e in entries:
        k, size = e if isinstance(e, tuple) else (e, None)
        out[k] = size
    return out


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
        uc.mem_map(FAKE, FAKE_SIZE)
        uc.mem_write(FAKE, b'\xf4' * FAKE_SIZE)
        uc.mem_map(ARENA, ARENA_SIZE)
        uc.mem_map(STACK_TOP - STACK_SIZE, STACK_SIZE)
        uc.mem_write(STACK_TOP - STACK_SIZE, bytes([STACK_FILL]) * STACK_SIZE)
        uc.mem_map(HEAP, HEAP_SIZE)
        return uc

    def run(self, entry: int, setup: Callable, case, fpcw: int, seed, probes=(),
            replay: Replay | None = None) -> RunResult:
        from unicorn import UC_HOOK_CODE, UC_HOOK_MEM_WRITE, UcError
        uc = self._new_uc()
        ctx = Context(uc, random.Random(repr(seed)))
        ctx.entry = entry
        if replay is not None:
            uc.mem_write(ARENA, arena_bytes(seed))
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
        if replay is not None:
            uc.hook_add(UC_HOOK_CODE, ReplayRun(replay, ctx, seed, entry).hook)

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
                     if not (frame_low <= a < frame_high or STUB <= a < STUB + STUB_SIZE
                             or any(lo <= a < lo + size for lo, size in call.ignore))}
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
                if len(ca) != len(cb):
                    longer, who = (ca, 'retail') if len(ca) > len(cb) else (cb, 'candidate')
                    out.append(f'first extra call ({who}) {_fmt_call(longer[min(len(ca), len(cb))])}')
        else:
            out.append(f'{key} retail={da.get(key)!r} candidate={db.get(key)!r}')
    return out


def _f32_order(word: int) -> int:
    """float32 bits -> an integer that orders like the value (adjacent floats differ by 1)."""
    return word if word < 0x80000000 else 0x80000000 - word


def ulp_distance(a: int, b: int) -> int | None:
    """Distance in float32 ulps between two 32-bit words, None if either is NaN or infinite."""
    if (a & 0x7f800000) == 0x7f800000 or (b & 0x7f800000) == 0x7f800000:
        return None
    return abs(_f32_order(a) - _f32_order(b))


def _bytes_by_address(runs) -> dict:
    return {s + i: int(h[2 * i:2 * i + 2], 16) for s, h in runs for i in range(len(h) // 2)}


def _word_close(a: int, b: int, tol: tuple, worst: list) -> bool:
    """Two float32 words within ``tol`` = (ulps, absolute); ``worst`` keeps the largest ulp distance."""
    d = ulp_distance(a, b)
    if d is None:
        return False
    if d <= tol[0]:
        worst[0] = max(worst[0], d)
        return True
    fa, fb = struct.unpack('<2f', struct.pack('<2I', a, b))
    return abs(fa - fb) <= tol[1]


def _words_close(ba: dict, bb: dict, tol: tuple, worst: list) -> bool:
    """Every 4-aligned word with a differing byte is written whole on both sides and is close."""
    for word in sorted({x & ~3 for x in set(ba) | set(bb) if ba.get(x) != bb.get(x)}):
        span = range(word, word + 4)
        if not all(x in ba and x in bb for x in span):
            return False
        if not _word_close(int.from_bytes(bytes(ba[x] for x in span), 'little'),
                           int.from_bytes(bytes(bb[x] for x in span), 'little'), tol, worst):
            return False
    return True


def _hexwords_close(x, y, tol: tuple, worst: list) -> bool:
    if x == y:
        return True
    if not (isinstance(x, str) and isinstance(y, str)) or len(x) != len(y) or len(x) % 8:
        return False
    try:
        ba = dict(enumerate(bytes.fromhex(x)))
        bb = dict(enumerate(bytes.fromhex(y)))
    except ValueError:
        return False
    return _words_close(ba, bb, tol, worst)


def _calls_close(ca, cb, tol: tuple, worst: list) -> bool:
    if len(ca) != len(cb):
        return False
    for x, y in zip(ca, cb):
        if x == y:
            continue
        if x[0] == 'note' or y[0] == 'note' or len(x) != len(y) or x[:2] != y[:2] or len(x[2]) != len(y[2]):
            return False
        for u, v in zip(x[2], y[2]):
            if u == v:
                continue
            if not (isinstance(u, int) and isinstance(v, int)
                    and _word_close(u & 0xffffffff, v & 0xffffffff, tol, worst)):
                return False
        ex, ey = (x[3], y[3]) if len(x) > 3 else ((), ())
        if len(ex) != len(ey):
            return False
        for (kx, vx), (ky, vy) in zip(ex, ey):
            if kx != ky or not _hexwords_close(vx, vy, tol, worst):
                return False
    return True


def tolerance_for(module, fpcw: int) -> tuple:
    """(ulps, absolute) a case allows at one control word; FLOAT_ULPS may map fpcw -> ulps."""
    ulps = getattr(module, 'FLOAT_ULPS', 0)
    if isinstance(ulps, dict):
        ulps = ulps.get(fpcw, 0)
    return ulps, (getattr(module, 'FLOAT_ABS', 0.0) if ulps else 0.0)


def outcomes_within_ulps(a: tuple, b: tuple, ulps: int, absolute: float = 0.0) -> int | None:
    """The largest float32 ulp distance between two outcomes that differ only in float words
    (written memory, logged call arguments and pointees) by at most ``ulps`` (or by at most
    ``absolute`` in value); None otherwise.

    A differing word must be written whole by both runs; status, registers, the call targets
    and every other field must be equal. Words that hold integers or pointers are compared the
    same way, so a tolerance only suits cases whose differing words are floats."""
    if ulps <= 0 or len(a) != len(b):
        return None
    worst = [0]
    tol = (ulps, absolute)
    for x, y in zip(a, b):
        if x == y:
            continue
        if not (isinstance(x, tuple) and isinstance(y, tuple) and len(x) == 2 and x[0] == y[0]):
            return None
        if x[0] == 'writes':
            if not _words_close(_bytes_by_address(x[1]), _bytes_by_address(y[1]), tol, worst):
                return None
        elif x[0] == 'calls':
            if not _calls_close(x[1], y[1], tol, worst):
                return None
        else:
            return None
    return worst[0]


def _fmt_call(c):
    if c[0] == 'note':
        return repr(c)
    name, ecx, args = c[:3]
    text = f'{name}(ecx={_hex(ecx)}, ' + ', '.join(_hex(x) for x in args) + ')'
    if len(c) > 3 and c[3]:
        text += ' ' + ' '.join(f'{k}={v}' for k, v in c[3])
    return text


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
    within_ulps: int = 0
    worst_ulps: int = 0
    retail_faults: int = 0
    examples: list = field(default_factory=list)
    probes: dict = field(default_factory=dict)
    by_fpcw: dict = field(default_factory=dict)

    def as_dict(self):
        return {'case': self.case, 'runs': self.runs, 'mismatches': self.mismatches,
                'within_ulps': self.within_ulps, 'worst_ulps': self.worst_ulps,
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


_SLOT_NAMES: dict[tuple, tuple] = {}


def vtable_slot_names(pe: PEImage, build: Path | None = None) -> tuple[dict, dict]:
    """Built symbols found in the same (class, object offset, slot) as each retail vtable entry.

    Returns (retail function VA -> names, retail vtable entry address -> names). Uses the newest
    ``work/game-build`` (its exe and map); empty without one. Evidence about the build's
    declarations only: it supplies the signature the source compiled each call site against.
    """
    from tools.audit_game_vtables import image_vtables, latest_build, parse_map
    build = build or latest_build()
    if build is None or not (build / 'mcm2.exe').is_file():
        return {}, {}
    key = (str(pe.path), str(build))
    if key not in _SLOT_NAMES:
        symbols = parse_map((build / 'mcm2.map').read_text(encoding='latin-1'))
        retail = image_vtables(pe)['vtables']
        built = image_vtables(PEImage(build / 'mcm2.exe'))['vtables']
        by_va: dict[int, list[str]] = {}
        by_entry: dict[int, list[str]] = {}
        for k, rv in retail.items():
            bv = built.get(k)
            if bv is None:
                continue
            for slot, (ra, ba) in enumerate(zip(rv['entries'], bv['entries'])):
                found = [n for n in symbols.get(ba, ()) if n.startswith('?')]
                by_entry[rv['vtable_va'] + 4 * slot] = found
                for name in found:
                    if name not in by_va.setdefault(ra, []):
                        by_va[ra].append(name)
        _SLOT_NAMES[key] = (by_va, by_entry)
    return _SLOT_NAMES[key]


def build_replay(module, pe: PEImage, placement: Placement, symbol: str | None = None,
                 atlas: Path | None = None) -> Replay | None:
    """The case's ``Replay`` (None unless it declares ``REPLAY``), named from the link's bindings."""
    spec = getattr(module, 'REPLAY', None)
    if spec is None:
        return None
    from tools.audit_game_vtables import load_bindings
    names: dict[Any, list[str]] = {}
    # The candidate's own references first: they carry the signature the body was compiled against.
    for name, (_, va) in sorted(placement.resolved.items()):
        names.setdefault(va, []).append(name)
    for name, va in getattr(module, 'BINDINGS', {}).items():
        names.setdefault(va, []).append(name)
    for name, vas in sorted(load_bindings(ROOT).items()):
        if len(vas) == 1:
            va = next(iter(vas))
            if name not in names.setdefault(va, []):
                names[va].append(name)
    names['self'] = [symbol or module.SYMBOL]
    # Virtual targets without a binding: the built game's symbol in the same vtable slot.
    by_va, by_entry = vtable_slot_names(pe)
    for va, built in by_va.items():
        for name in built:
            if name not in names.setdefault(va, []):
                names[va].append(name)
    try:
        from tools.disasm_fn import load_slot_map
        slots = load_slot_map(ROOT / 'analysis')
    except Exception:
        slots = {}
    replay = Replay(spec, pe, names, crt_bindings(atlas), slots)
    replay.entry_names = by_entry
    return replay


def run_diff(module, obj_path: Path, n: int = 200, seed: int = 1, fpcws=None, retail: Path | None = None,
             symbol: str | None = None, retail_va: int | None = None, show: int = 5,
             atlas: Path | None = None) -> Summary:
    pe = PEImage(retail or os.environ.get('MCM2_EXE') or DEFAULT_RETAIL)
    placement = build_placement(module, obj_path, symbol, atlas)
    emu = Emulator(pe, placement, getattr(module, 'MAX_INSNS', 20_000_000))
    replay = build_replay(module, pe, placement, symbol, atlas)
    retail_va = retail_va or module.RETAIL_VA
    fpcws = fpcws or getattr(module, 'FPCWS', DEFAULT_FPCWS)
    probes = dict(getattr(module, 'PROBES', {}))
    summary = Summary(getattr(module, '__name__', 'case'), probes={va: 0 for va in probes})
    rng = random.Random(seed)
    case_list = list(module.cases(rng, n))
    for fpcw in fpcws:
        bad = 0
        ulps, absolute = tolerance_for(module, fpcw)
        for index, case in enumerate(case_list):
            r = emu.run(retail_va, module.setup, case, fpcw, (seed, index), probes, replay)
            c = emu.run(placement.entry, module.setup, case, fpcw, (seed, index), (), replay)
            summary.runs += 1
            for va, hits in r.probes.items():
                summary.probes[va] += bool(hits)
            if r.faulted:
                summary.retail_faults += 1
            if r.outcome != c.outcome:
                close = outcomes_within_ulps(r.outcome, c.outcome, ulps, absolute)
                if close is not None:
                    summary.within_ulps += 1
                    summary.worst_ulps = max(summary.worst_ulps, close)
                    continue
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
    replay = build_replay(module, pe, placement, symbol, atlas)
    print(f'retail {retail_va:#x}..{end:#x} calls:')
    for ins in insns:
        if ins.mnemonic == 'call':
            line = f'  {ins.address:#x}: call {ins.op_str}'
            if replay is not None and ins.operands[0].type == 2:
                t = ins.operands[0].imm
                i = replay.info(t)
                how = 'passthrough' if t in replay.passthrough else 'inline' if t in replay.inline else \
                    f'pop {i.pop} args {i.nargs} ecx {int(i.thiscall)} ret {i.ret if not callable(i.ret) else "fn"}'
                line += f'  [{i.name}; {how}]'
            print(line)

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
    parser.add_argument('--ulps', type=int, help="override the case's FLOAT_ULPS tolerance (0 = exact)")
    parser.add_argument('--json-out', type=Path)
    parser.add_argument('--explain', action='store_true',
                        help='list how relocations resolved and the retail direct calls, then exit')
    args = parser.parse_args(argv)
    module = load_case(args.case)
    if args.ulps is not None:
        module.FLOAT_ULPS = args.ulps
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
    if summary.within_ulps:
        print(f'within FLOAT_ULPS {getattr(module, "FLOAT_ULPS", 0)}: {summary.within_ulps} runs '
              f'(worst {summary.worst_ulps})')
    print(f'{summary.case}: runs {summary.runs}, mismatches {summary.mismatches}, '
          f'retail faults {summary.retail_faults}')
    if args.json_out:
        args.json_out.write_text(json.dumps(summary.as_dict(), indent=2) + '\n', encoding='utf-8')
    return 1 if summary.mismatches else 0


if __name__ == '__main__':
    raise SystemExit(main())
