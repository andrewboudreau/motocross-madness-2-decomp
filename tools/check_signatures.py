#!/usr/bin/env python3
"""Check bound functions' declared parameters against the retail bodies they bind.

Every *.bindings.json under src/ and samples/ maps a mangled name to a retail VA.  The tool decodes
the parameter list from the MSVC name and disassembles the retail body.  Two checks:

* Stack bytes (confirmed tier when it fires).  A __thiscall/__stdcall body must `ret N` with N the
  declared parameter bytes (plus 4 for a hidden class-return pointer); a __cdecl body must `ret`.
  A constructor of a class with a virtual base (RTTI pdisp >= 0, also through a
  config/game_build.json link alias) takes VC6's hidden most-derived flag: 4 more bytes.  Bodies
  whose decoded extent shows several `ret` widths (tail jumps into a neighbour) count as agreeing
  when the expected width is among them.
* Const pointers (heuristic).  The registers loaded from each pointer parameter's stack slot are
  followed (copies, lea/add offsets, the state at branch targets).  A store whose address still
  derives from a parameter declared `const T*` or `const T&` contradicts the declaration, and
  usually the names around it: the parameter is an output.  Only the pointer itself is tracked, so
  a store through a pointer loaded *from* the pointee (shallow const) is not reported.  The scan is
  linear (no full CFG); a report is a lead to check in the disassembly.

`--all` also lists stores through non-const pointer parameters (expected, for review only).
`--fail` exits 1 on any finding (tests/test_check_signatures.py runs it against the retail exe).

    PYTHONPATH=. python tools/check_signatures.py --exe work/game/mcm2.exe
"""
from __future__ import annotations

import argparse
import json
import os
import sys
from pathlib import Path

import capstone
from capstone import x86

ROOT = Path(__file__).resolve().parents[1]
sys.path.insert(0, str(ROOT))
sys.path.insert(0, str(ROOT / 'tools'))
from mcm2tool.pe import PEImage  # noqa: E402
from disasm_fn import disassemble  # noqa: E402

SIMPLE = {'C': 1, 'D': 1, 'E': 1, 'F': 1, 'G': 1, 'H': 1, 'I': 1, 'J': 1, 'K': 1, 'M': 1, 'N': 2, 'O': 2}


class Unsupported(Exception):
    pass


class Demangler:
    def __init__(self, s):
        self.s = s
        self.i = 0
        self.names = []
        self.argtypes = []

    def peek(self):
        return self.s[self.i] if self.i < len(self.s) else ''

    def take(self, n=1):
        r = self.s[self.i:self.i + n]
        self.i += n
        return r

    def fragment(self):
        c = self.peek()
        if c.isdigit():
            self.i += 1
            return self.names[int(c)]
        if c == '?':
            raise Unsupported('template/special name')
        j = self.s.index('@', self.i)
        name = self.s[self.i:j]
        self.i = j + 1
        if len(self.names) < 10:
            self.names.append(name)
        return name

    def qualified(self):
        parts = []
        while self.peek() != '@':
            parts.append(self.fragment())
        self.i += 1
        return '::'.join(reversed(parts))

    def type(self):
        """Returns (kind, const, slots): kind 'ptr' for pointers/references, 'val' otherwise."""
        start = self.i
        c = self.take()
        if c in SIMPLE:
            return ('val', False, SIMPLE[c])
        if c == 'X':
            return ('void', False, 0)
        if c == '_':
            d = self.take()
            r = ('val', False, 2 if d in 'JK' else 1)
            if len(self.argtypes) < 10:
                self.argtypes.append(r)
            return r
        if c.isdigit():
            return self.argtypes[int(c)]
        if c in 'PQAB':
            q = self.take()
            if q == '6':
                raise Unsupported('function pointer')
            const = q in 'BD'
            self.type_noreg()
            r = ('ptr', const, 1)
        elif c in 'UVT':
            self.qualified()
            r = ('agg', False, None)
        elif c == 'W':
            self.take()
            self.qualified()
            r = ('val', False, 1)
        else:
            raise Unsupported('type code %r' % c)
        if self.i - start > 1:
            if len(self.argtypes) < 10:
                self.argtypes.append(r)
        return r

    def parse(self):
        if not self.s.startswith('?'):
            raise Unsupported('not mangled')
        structor = False
        self.code = ''
        if self.s.startswith('??'):
            code = self.s[2]
            if code == '$' or self.s[2:4] in ('_R', '_7', '_8', '_C'):
                raise Unsupported('special name')
            self.i = 4 if code == '_' else 3
            self.code = self.s[2:self.i]
            structor = code in '01'
        else:
            self.i = 1
        self.qname = self.qualified()
        acc = self.take()
        if acc == 'Y':
            conv = self.take()
            member = False
        elif acc in 'QUAEIMCSKGO':
            static = acc in 'SKCG'  # static members carry no this-qualifier
            if not static:
                self.take()          # this cv-qualifier
            conv = self.take()
            member = not static
        else:
            raise Unsupported('access %r' % acc)
        hidden = False
        if structor:
            self.take()                 # '@': no return type
        elif self.peek() == '?':
            self.i += 2
            hidden = True
            self.type_noreg()
        else:
            r = self.type_noreg()
            hidden = r[0] == 'agg'      # class returned by value: hidden result pointer first
        args = []
        while self.peek() not in ('@', 'Z', ''):
            t = self.type()
            if t[0] == 'void':
                break
            args.append(t)
        thiscall = member and conv == 'E'
        self.conv = conv
        return args, hidden, thiscall

    def type_noreg(self):
        saved = len(self.argtypes)
        r = self.type()
        del self.argtypes[saved:]
        return r


REG_ALIASES = {}
for full, parts in (('eax', ('ax', 'al', 'ah')), ('ebx', ('bx', 'bl', 'bh')), ('ecx', ('cx', 'cl', 'ch')),
                    ('edx', ('dx', 'dl', 'dh')), ('esi', ('si',)), ('edi', ('di',)), ('ebp', ('bp',)), ('esp', ('sp',))):
    REG_ALIASES[full] = full
    for p in parts:
        REG_ALIASES[p] = full

NO_DEST_WRITE = {'cmp', 'test', 'push', 'fcom', 'fcomp', 'fucom', 'fucomp', 'bt', 'call', 'jmp'}
MEM_WRITERS = {'mov', 'fstp', 'fst', 'fistp', 'fist', 'fnstsw', 'inc', 'dec', 'add', 'sub', 'and', 'or', 'xor',
               'neg', 'not', 'shl', 'shr', 'sar', 'sal', 'adc', 'sbb', 'movsx', 'stosd', 'stosb', 'stosw',
               'xchg', 'setne', 'sete', 'setg', 'setl', 'setge', 'setle', 'seta', 'setb', 'setae', 'setbe', 'pop',
               'fnstcw', 'fstcw', 'fbstp'}


def callee_pop(pe, target, cache):
    if target in cache:
        return cache[target]
    n = 0
    try:
        insns, _, _, _ = disassemble(pe, target, 0x4000)
        for ins in insns:
            if ins.mnemonic in ('ret', 'retn'):
                n = ins.operands[0].imm if ins.operands else 0
                break
    except Exception:
        n = 0
    cache[target] = n
    return n


def analyse(pe, md_va, args, hidden, thiscall, cache):
    """Returns {arg index: [store VAs]} for pointer args."""
    slots = {}
    off = 4
    if hidden:
        off += 4
    for k, (kind, const, size) in enumerate(args):
        if size is None:
            break               # by-value aggregate of unknown size: later offsets unknown
        if kind == 'ptr':
            slots[off] = k
        off += 4 * size
    if not slots:
        return {}
    insns, _, _, _ = disassemble(pe, md_va, 0x4000)
    delta = 0                   # esp relative to entry
    frame = None                # esp delta at `mov ebp, esp`
    taint = {}
    dead = set()
    hits = {}

    def slot_of(mem):
        if mem.index:
            return None
        base = insns_reg(mem.base)
        if base == 'ebp' and frame is not None:
            o = mem.disp + frame
        elif base == 'esp':
            o = mem.disp + delta
        else:
            return None
        return o

    def insns_reg(r):
        if not r:
            return None
        return REG_ALIASES.get(cs.reg_name(r))

    cs = capstone.Cs(capstone.CS_ARCH_X86, capstone.CS_MODE_32)
    snapshots = {}
    dead_fallthrough = False
    for ins in insns:
        m = ins.mnemonic
        ops = ins.operands
        if dead_fallthrough and ins.address in snapshots:
            delta, saved = snapshots[ins.address]
            taint = dict(saved)
        dead_fallthrough = m in ('ret', 'retn', 'jmp')
        if ins.group(capstone.CS_GRP_JUMP) and ops and ops[0].type == x86.X86_OP_IMM:
            snapshots.setdefault(ops[0].imm, (delta, dict(taint)))
        # stores through tainted registers
        if ops and ops[0].type == x86.X86_OP_MEM and m in MEM_WRITERS:
            mem = ops[0].mem
            for r in (insns_reg(mem.base), insns_reg(mem.index)):
                if r in taint:
                    hits.setdefault(taint[r], []).append(ins.address)
            o = slot_of(mem)
            if o in slots:
                dead.add(o)
        if m in ('stosd', 'stosb', 'stosw', 'movsd', 'movsb', 'movsw') and 'edi' in taint:
            if not (m == 'movsd' and ops):  # SSE movsd never appears in VC6 code
                hits.setdefault(taint['edi'], []).append(ins.address)
        # stack tracking
        if m == 'push':
            delta -= 4
        elif m == 'pop':
            delta += 4
            if ops and ops[0].type == x86.X86_OP_REG:
                taint.pop(insns_reg(ops[0].reg), None)
            continue
        elif m in ('sub', 'add') and ops[0].type == x86.X86_OP_REG and insns_reg(ops[0].reg) == 'esp' \
                and ops[1].type == x86.X86_OP_IMM:
            delta += -ops[1].imm if m == 'sub' else ops[1].imm
            continue
        elif m == 'call':
            for r in ('eax', 'ecx', 'edx'):
                taint.pop(r, None)
            if ops and ops[0].type == x86.X86_OP_IMM:
                delta += callee_pop(pe, ops[0].imm, cache)
            continue
        elif m in ('ret', 'retn'):
            continue
        if m == 'mov' and len(ops) == 2 and ops[0].type == x86.X86_OP_REG and insns_reg(ops[0].reg) == 'ebp' \
                and ops[1].type == x86.X86_OP_REG and insns_reg(ops[1].reg) == 'esp':
            frame = delta
            continue
        # register dataflow
        if not ops or ops[0].type != x86.X86_OP_REG or m in NO_DEST_WRITE:
            continue
        dst = insns_reg(ops[0].reg)
        src = ops[1] if len(ops) > 1 else None
        if m == 'mov' and src is not None:
            if src.type == x86.X86_OP_REG and insns_reg(src.reg) in taint:
                taint[dst] = taint[insns_reg(src.reg)]
            elif src.type == x86.X86_OP_MEM and slot_of(src.mem) in slots and slot_of(src.mem) not in dead:
                taint[dst] = slots[slot_of(src.mem)]
            else:
                taint.pop(dst, None)
        elif m == 'lea' and src is not None:
            b = insns_reg(src.mem.base)
            x = insns_reg(src.mem.index)
            t = taint.get(b, taint.get(x) if src.mem.scale == 1 else None)
            if t is not None:
                taint[dst] = t
            else:
                taint.pop(dst, None)
        elif m in ('add', 'sub') and src is not None:
            if dst not in taint and m == 'add' and src.type == x86.X86_OP_REG and insns_reg(src.reg) in taint:
                taint[dst] = taint[insns_reg(src.reg)]
        elif m == 'xchg' and src is not None and src.type == x86.X86_OP_REG:
            a, b = dst, insns_reg(src.reg)
            ta, tb = taint.pop(a, None), taint.pop(b, None)
            if tb is not None:
                taint[a] = tb
            if ta is not None:
                taint[b] = ta
        else:
            taint.pop(dst, None)
    return hits


def load_bindings():
    bindings = {}
    for p in list((ROOT / 'src').rglob('*.bindings.json')) + list((ROOT / 'samples').rglob('*.bindings.json')):
        try:
            d = json.loads(p.read_text(encoding='latin1'))
        except ValueError:
            continue
        for k, v in d.items():
            if isinstance(v, str) and v.startswith('0x'):
                bindings.setdefault(k, (int(v, 16), p.relative_to(ROOT).as_posix()))
    return bindings


def virtual_base_classes():
    """Class names whose constructors take the hidden most-derived flag."""
    names = set()
    path = ROOT / 'analysis' / 'rtti_classes.json'
    if path.exists():
        for c in json.loads(path.read_text(encoding='utf-8')):
            if any(b.get('pdisp', -1) >= 0 for b in c.get('bases', ())):
                names.add(c['name'])
    config = ROOT / 'config' / 'game_build.json'
    if config.exists():
        aliases = json.loads(config.read_text(encoding='latin1'))
        aliases = aliases.get('link_aliases', aliases) if isinstance(aliases, dict) else {}
        for view, provider in aliases.items():
            if (isinstance(provider, str) and view.startswith('??0') and provider.startswith('??0')
                    and provider[3:].split('@', 1)[0] in names):
                names.add(view[3:].split('@', 1)[0])
    return names


def check_stack_bytes(pe, bindings=None):
    """Returns (findings, checked); findings are (va, name, expected, retail ret widths, file)."""
    bindings = bindings if bindings is not None else load_bindings()
    vbases = virtual_base_classes()
    findings = []
    checked = 0
    for name, (va, src) in sorted(bindings.items(), key=lambda kv: kv[1][0]):
        d = Demangler(name)
        try:
            args, hidden, _ = d.parse()
        except (Unsupported, ValueError, IndexError):
            continue
        if any(a[2] is None for a in args) or d.conv not in 'AEG':
            continue
        expected = 0 if d.conv == 'A' else 4 * sum(a[2] for a in args) + (4 if hidden else 0)
        if d.code == '0' and d.qname.split('::')[-1] in vbases:
            expected += 4
        try:
            insns = disassemble(pe, va, 0x4000)[0]
        except Exception:  # noqa: BLE001 - not decodable
            continue
        widths = sorted({(i.operands[0].imm if i.operands else 0) for i in insns if i.mnemonic in ('ret', 'retn')})
        if not widths:
            continue
        checked += 1
        if expected not in widths:
            findings.append((va, name, expected, widths, src))
    return findings, checked


def main(argv=None):
    ap = argparse.ArgumentParser(description=__doc__.split(chr(10))[1])
    ap.add_argument('--exe', default=os.environ.get('MCM2_EXE', str(ROOT / 'work/game/mcm2.exe')))
    ap.add_argument('--all', action='store_true', help='also list non-const pointer args that are written')
    ap.add_argument('--fail', action='store_true', help='exit 1 on any finding')
    a = ap.parse_args(argv)
    pe = PEImage(a.exe)
    bindings = load_bindings()
    pops, pop_checked = check_stack_bytes(pe, bindings)
    for va, name, expected, widths, src in pops:
        print('0x%08x stack bytes: declared %d, retail ret %s  %s  (%s)'
              % (va, expected, '/'.join(map(str, widths)), name, src))
    flagged, checked, unsupported = scan(pe, a.all, bindings)
    for va, name, k, const, addrs, src in flagged:
        print('0x%08x arg%d %s  stores %s  %s  (%s)' % (va, k, 'CONST' if const else 'mut',
                                                        ','.join('0x%x' % x for x in addrs[:3]), name, src))
    print('stack bytes: %d functions, %d disagree; pointer stores: %d functions, %d flagged; %d names not decoded'
          % (pop_checked, len(pops), checked, len(flagged), unsupported), file=sys.stderr)
    return 1 if a.fail and (pops or any(f[3] for f in flagged)) else 0


def scan(pe, include_mutable=False, bindings=None):
    """Returns (flagged, checked, undecoded); flagged rows are (va, name, arg, const, store VAs, file)."""
    bindings = bindings if bindings is not None else load_bindings()
    cache = {}
    unsupported = 0
    checked = 0
    flagged = []
    for name, (va, src) in sorted(bindings.items(), key=lambda kv: kv[1][0]):
        try:
            args, hidden, thiscall = Demangler(name).parse()
        except (Unsupported, ValueError, IndexError):
            unsupported += 1
            continue
        if not any(t[0] == 'ptr' and (t[1] or include_mutable) for t in args):
            continue
        try:
            hits = analyse(pe, va, args, hidden, thiscall, cache)
        except Exception:  # noqa: BLE001 - undecodable body
            unsupported += 1
            continue
        checked += 1
        for k, addrs in sorted(hits.items()):
            if args[k][1] or include_mutable:
                flagged.append((va, name, k, args[k][1], addrs, src))
    return flagged, checked, unsupported


if __name__ == '__main__':
    raise SystemExit(main())
