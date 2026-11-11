#!/usr/bin/env python3
"""Values that C++ dynamic initializers give data, found by running them under unicorn.

A namespace-scope ``static const Vec3 kVec3XAxis = Vec3(1.0f, 0.0f, 0.0f);`` is
``.bss`` in the load image of every unit: its value is written by the unit's
``$E`` initializer, which ``__cinit`` calls through the ``.CRT$XC*`` table
(``_initterm(__xc_a, __xc_z)``). A byte compare of the image therefore sees
zeros on both sides. This module runs every entry of that table, in table order,
in one emulated copy of the image and records the bytes each entry writes.

Three LIBCMT routines are replaced, by name (``STUBS``): ``__nh_malloc``
(``malloc`` and ``operator new``) hands out zeroed blocks from a scratch heap,
``_free`` does nothing and ``__onexit`` (``atexit``) returns its argument.
``fs`` points at a scratch thread block, so SEH frames work. An entry that
faults, calls an import or runs past the instruction limit is undone (its
writes are reverted) and counted as failed. The data addresses its body
references and the bytes it wrote before it stopped become *uncertain*: a
compare that touches them is not evidence. Heap addresses differ between two
images, so a word in ``[HEAP, HEAP_END)`` reads as a pointer, like an image
address.

    python tools/static_initializers.py [EXE] [--map MAP] [--initterm VA]
"""
from __future__ import annotations

import json
import struct
import sys
from dataclasses import dataclass, field
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
if str(ROOT) not in sys.path:
    sys.path.insert(0, str(ROOT))

STACK_TOP = 0x30000000
STACK_SIZE = 0x40000
STOP = 0x31000000         # return address of every entry (a mapped hlt)
IMPORT_STUB = 0x32000000  # every IAT slot points here (unmapped): calling an import faults
TIB = 0x33000000          # fs base
GDT = 0x34000000          # descriptors for ss (flat) and fs (TIB)
SS_INDEX, FS_INDEX = 2, 3
HEAP = 0x40000000         # __nh_malloc blocks
HEAP_END = HEAP + 0x1000000
MAX_INSNS = 2_000_000
HLT = 0xf4
# Replaced routines: (action, argument index, bytes the callee pops). Actions:
# 'alloc' returns a zeroed scratch-heap block of argument-many bytes, 'size' the size of the
# block the argument names, 'arg' the argument, an int that constant.
# LIBCMT routines (by map/atlas name, __cdecl):
STUBS = {'__nh_malloc': ('alloc', 0, 0), '_free': (0, 0, 0), '__onexit': ('arg', 0, 0)}
# Imports (by IAT name, __stdcall) an initializer may call without changing what it writes.
# The game's operator new (MemTag.cpp) calls HeapAlloc and HeapSize itself.
IMPORTS = {'__imp__GetProcessHeap': (1, 0, 0),
           '__imp__HeapAlloc': ('alloc', 2, 12),
           '__imp__HeapSize': ('size', 2, 12),
           '__imp__HeapFree': (1, 0, 12),
           '__imp__IsBadReadPtr': (0, 0, 8),             # the pointer is readable
           '__imp__timeBeginPeriod': (0, 0, 4),          # TIMERR_NOERROR
           '__imp__timeEndPeriod': (0, 0, 4),
           '__imp__timeGetTime': (0, 0, 0),              # a constant clock: both images read 0
           '__imp__EnterCriticalSection': (0, 0, 4),
           '__imp__LeaveCriticalSection': (0, 0, 4),
           '__imp__InitializeCriticalSection': (0, 0, 4),
           '__imp__QueryPerformanceFrequency': (0, 0, 4),  # no counter (FALSE)
           '__imp__CoInitialize': (0, 0, 4)}             # S_OK


@dataclass
class InitResult:
    table: tuple[int, int] | None                       # (__xc_a, __xc_z)
    values: dict[int, int] = field(default_factory=dict)  # address -> byte, successful entries
    writer: dict[int, int] = field(default_factory=dict)  # address -> initializer entry VA
    uncertain: set[int] = field(default_factory=set)      # addresses failed entries may write
    # Addresses several entries write: shared running state (the allocator's byte
    # counters), whose value depends on every initializer before it, not on one object.
    shared: set[int] = field(default_factory=set)
    ok: list[int] = field(default_factory=list)
    failed: dict[int, str] = field(default_factory=dict)  # entry VA -> reason


def _text_range(pe) -> tuple[int, int]:
    s = next(s for s in pe.sections if s.characteristics & 0x20)
    return pe.image_base + s.virtual_address, pe.image_base + s.virtual_address + s.virtual_size


def _read(pe, va: int, size: int) -> bytes:
    out = bytearray(size)
    for s in pe.sections:
        lo = pe.image_base + s.virtual_address
        hi = lo + max(s.virtual_size, s.raw_size)
        a, b = max(va, lo), min(va + size, hi)
        if a >= b:
            continue
        fb = min(b, lo + s.raw_size)
        if a < fb:
            off = s.raw_offset + (a - lo)
            out[a - va:fb - va] = pe.data[off:off + (fb - a)]
    return bytes(out)


def initterm_tables(pe, initterm_va: int) -> list[tuple[int, int]]:
    """(first, end) of every ``push end; push first; call _initterm`` in .text."""
    lo, hi = _text_range(pe)
    text = _read(pe, lo, hi - lo)
    out = []
    i = text.find(b'\x68')
    while 0 <= i <= len(text) - 15:
        if text[i + 5] == 0x68 and text[i + 10] == 0xe8:
            target = (lo + i + 15 + struct.unpack_from('<i', text, i + 11)[0]) & 0xffffffff
            if target == initterm_va:
                end = struct.unpack_from('<I', text, i + 1)[0]
                first = struct.unpack_from('<I', text, i + 6)[0]
                if first < end:
                    out.append((first, end))
        i = text.find(b'\x68', i + 1)
    return out


def cxx_table(pe, initterm_va: int) -> tuple[int, int] | None:
    """The ``__xc_a``/``__xc_z`` table: the ``_initterm`` table with the most code pointers."""
    lo, hi = _text_range(pe)
    best, best_n = None, 0
    for first, end in initterm_tables(pe, initterm_va):
        words = struct.unpack(f'<{(end - first) // 4}I', _read(pe, first, end - first))
        n = sum(1 for w in words if lo <= w < hi)
        if n > best_n:
            best, best_n = (first, end), n
    return best


def _descriptor(base: int, limit: int, flags: int = 0x4) -> bytes:
    """A present ring-0 read/write data segment descriptor (32-bit; flags 0xc: 4 KiB units)."""
    d = limit & 0xffff | (base & 0xffffff) << 16 | 0x92 << 40 | ((limit >> 16) & 0xf) << 48
    d |= flags << 52 | ((base >> 24) & 0xff) << 56
    return struct.pack('<Q', d)


def _data_refs(pe, va: int, limit: int = 0x200) -> set[int]:
    """Absolute data addresses a function body spells (imm32/disp32 words), up to its ret."""
    _, hi = _text_range(pe)
    raw = _read(pe, va, limit)
    end = raw.find(b'\xc3')
    raw = raw[:end + 1 if end >= 0 else limit]
    image_hi = pe.image_base + pe.size_of_image
    out = set()
    for k in range(len(raw) - 3):
        w = struct.unpack_from('<I', raw, k)[0]
        if hi <= w < image_hi:
            out.update(range(w, w + 4))
    return out


def run_initializers(pe, table: tuple[int, int] | None, iat: dict[str, int] | None = None,
                     max_insns: int = MAX_INSNS, stubs: dict[int, tuple] | None = None) -> InitResult:
    """Run each table entry in order in one emulated image; see the module docstring.

    ``stubs``: VA -> (action, argument index, pops), as in ``STUBS``.
    """
    from unicorn import Uc, UcError, UC_ARCH_X86, UC_MODE_32, UC_HOOK_MEM_WRITE, UC_HOOK_CODE
    from unicorn.x86_const import (UC_X86_REG_EAX, UC_X86_REG_EBP, UC_X86_REG_EIP,
                                   UC_X86_REG_ESP, UC_X86_REG_FPCW, UC_X86_REG_FS, UC_X86_REG_SS,
                                   UC_X86_REG_GDTR)
    result = InitResult(table)
    if table is None:
        return result
    base = pe.image_base
    uc = Uc(UC_ARCH_X86, UC_MODE_32)
    uc.mem_map(base, (pe.size_of_image + 0xfff) & ~0xfff)
    uc.mem_write(base, pe.data[:pe.size_of_headers])
    for s in pe.sections:
        raw = pe.data[s.raw_offset:s.raw_offset + min(s.raw_size, s.virtual_size or s.raw_size)]
        uc.mem_write(base + s.virtual_address, raw)
    uc.mem_map(STACK_TOP - STACK_SIZE, STACK_SIZE)
    uc.mem_map(STOP, 0x1000)
    uc.mem_write(STOP, bytes([HLT]))
    uc.mem_map(TIB, 0x1000)
    uc.mem_write(TIB, struct.pack('<I', 0xffffffff))
    uc.mem_map(GDT, 0x1000)
    # Loading a GDT makes the stack segment's size matter: give ss a flat 32-bit one.
    uc.mem_write(GDT + 8 * SS_INDEX, _descriptor(0, 0xfffff, 0xc))
    uc.mem_write(GDT + 8 * FS_INDEX, _descriptor(TIB, 0xfff))
    uc.reg_write(UC_X86_REG_GDTR, (0, GDT, 8 * (FS_INDEX + 1) - 1, 0))
    uc.mem_map(HEAP, HEAP_END - HEAP)
    # Imports in IMPORTS point into a mapped stub page (a code hook answers them); the
    # rest point into unmapped memory, so calling one faults at an address that names it.
    hooked = dict(stubs or {})
    imports = {}
    uc.mem_map(IMPORT_STUB, 0x10000)
    uc.mem_write(IMPORT_STUB, bytes([HLT]) * 0x10000)
    for k, (name, slot) in enumerate(sorted((iat or {}).items())):
        at = IMPORT_STUB + 16 * k + (0 if name in IMPORTS else 0x100000)
        uc.mem_write(slot, struct.pack('<I', at))
        imports[at] = name
        if name in IMPORTS:
            hooked[at] = IMPORTS[name]
    image_lo, image_hi = base, base + pe.size_of_image
    log: list[tuple[int, bytes]] = []
    heap = [HEAP]
    blocks: dict[int, int] = {}

    def on_write(uc_, access, address, size, value, user):
        if image_lo <= address < image_hi:
            log.append((address, bytes(uc_.mem_read(address, size))))

    def on_stub(uc_, address, size, spec):
        action, index, pops = spec
        sp = uc_.reg_read(UC_X86_REG_ESP)
        ret = struct.unpack('<I', bytes(uc_.mem_read(sp, 4)))[0]
        arg = struct.unpack('<I', bytes(uc_.mem_read(sp + 4 + 4 * index, 4)))[0]
        if action == 'alloc':
            block = heap[0]
            heap[0] = (block + max(arg, 1) + 15) & ~15
            value = block if heap[0] <= HEAP_END else 0
            blocks[block] = arg
        elif action == 'size':
            value = blocks.get(arg, 0xffffffff)
        elif action == 'arg':
            value = arg
        else:
            value = action
        uc_.reg_write(UC_X86_REG_EAX, value)
        uc_.reg_write(UC_X86_REG_ESP, sp + 4 + pops)
        uc_.reg_write(UC_X86_REG_EIP, ret)

    uc.hook_add(UC_HOOK_MEM_WRITE, on_write)
    for va, spec in hooked.items():
        uc.hook_add(UC_HOOK_CODE, on_stub, spec, begin=va, end=va)

    first, end = table
    entries = struct.unpack(f'<{(end - first) // 4}I', _read(pe, first, end - first))
    text_lo, text_hi = _text_range(pe)
    for entry in entries:
        if not text_lo <= entry < text_hi:
            continue
        log.clear()
        sp = STACK_TOP - 0x100
        uc.mem_write(sp, struct.pack('<I', STOP))
        uc.reg_write(UC_X86_REG_ESP, sp)
        uc.reg_write(UC_X86_REG_EBP, sp + 0x40)
        uc.reg_write(UC_X86_REG_FPCW, 0x027f)
        uc.reg_write(UC_X86_REG_SS, SS_INDEX * 8)
        uc.reg_write(UC_X86_REG_FS, FS_INDEX * 8)
        try:
            uc.emu_start(entry, STOP, count=max_insns)
            reason = None if uc.reg_read(UC_X86_REG_EIP) == STOP else 'instruction limit'
        except UcError as e:
            eip = uc.reg_read(UC_X86_REG_EIP)
            reason = f'calls {imports[eip]}' if eip in imports else f'{e} at {eip:#010x}'
        if reason is None:
            result.ok.append(entry)
            for address, old in log:
                for k in range(len(old)):
                    if result.writer.get(address + k, entry) != entry:
                        result.shared.add(address + k)
                    result.writer[address + k] = entry
            continue
        result.failed[entry] = reason
        for address, old in reversed(log):
            uc.mem_write(address, old)
            result.uncertain.update(range(address, address + len(old)))
        result.uncertain |= _data_refs(pe, entry)
        raw = _read(pe, entry, 5)
        if raw[0] == 0xe9:      # the entry only jumps to the body
            result.uncertain |= _data_refs(pe, (entry + 5 + struct.unpack_from('<i', raw, 1)[0])
                                           & 0xffffffff)
    for address in result.writer:
        result.values[address] = uc.mem_read(address, 1)[0]
    return result


def initializers(pe, initterm_va: int | None, iat: dict[str, int] | None = None,
                 names: dict[str, int] | None = None) -> InitResult:
    """Run the C++ initializer table; ``names`` gives the VAs of the ``STUBS`` routines."""
    if initterm_va is None:
        return InitResult(None)
    stubs = {names[n]: spec for n, spec in STUBS.items() if n in (names or {})}
    return run_initializers(pe, cxx_table(pe, initterm_va), iat, stubs=stubs)


def atlas_names(root: Path = ROOT) -> dict[str, int]:
    """LIBCMT name -> retail VA, for names the CRT atlas places once."""
    path = root / 'work/vc6-crt-atlas/atlas.json'
    if not path.is_file():
        return {}
    per: dict[str, set[int]] = {}
    for m in json.loads(path.read_text()).get('libcmt', {}).get('matches', []):
        for n in [m['symbol']] + m.get('aliases', []):
            per.setdefault(n, set()).add(int(m['target_va']))
    return {n: next(iter(v)) for n, v in per.items() if len(v) == 1}


def main(argv=None) -> int:
    import argparse
    import os
    from mcm2tool.pe import PEImage
    ap = argparse.ArgumentParser(description=__doc__.splitlines()[0])
    ap.add_argument('exe', nargs='?', type=Path,
                    default=Path(os.environ.get('MCM2_EXE', ROOT / 'work/game/mcm2.exe')))
    ap.add_argument('--map', type=Path, default=None, help="the image's /MAP (a source build)")
    ap.add_argument('--initterm', type=lambda s: int(s, 0), default=None,
                    help='_initterm VA (default: from --map or the CRT atlas)')
    a = ap.parse_args(argv)
    sys.path.insert(0, str(ROOT / 'tools'))
    from linked_data_accuracy import iat_slots, parse_map
    pe = PEImage(a.exe)
    names = atlas_names()
    if a.map:
        names = {m.name: m.va for m in parse_map(a.map.read_text(encoding='latin-1'))}
    res = initializers(pe, a.initterm or names.get('__initterm'), iat_slots(pe), names)
    print(f'table {res.table and tuple(hex(v) for v in res.table)}: {len(res.ok)} ran, '
          f'{len(res.failed)} failed, {len(res.values)} bytes written, '
          f'{len(res.uncertain)} uncertain')
    for entry, why in sorted(res.failed.items()):
        print(f'  failed {entry:#010x}: {why}')
    return 0


if __name__ == '__main__':
    raise SystemExit(main())
