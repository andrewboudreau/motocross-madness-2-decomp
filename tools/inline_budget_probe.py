#!/usr/bin/env python3
"""Probe VC6 SP3's inline-expansion budget (docs/VC6_INLINE_BUDGET.md).

Writes a synthetic translation unit, compiles it with the default VC6 profile
and reports, per probe function, how many of its inline helper sites were
expanded and which helpers were called out of line instead.

    python3 tools/inline_budget_probe.py --vc6-root "$VC6_ROOT" [--out DIR]
"""
from __future__ import annotations
import argparse, collections, os, re, subprocess, sys
from pathlib import Path

sys.path.insert(0, str(Path(__file__).resolve().parent.parent))
from mcm2tool.coff import CoffObject

HELPERS = '''int Ext(int);
struct V3 { V3() {} V3(float a, float b, float c) { x = a; y = b; z = c; } float x, y, z; };
inline V3 operator+(const V3& a, const V3& b) { return V3(a.x + b.x, a.y + b.y, a.z + b.z); }
void Sink(V3*);
// four-statement helper: the unit every probe below counts
inline int I4(int t) { int b = 0; b += Ext(t + 1); b += Ext(t + 2); b += Ext(t + 3); b += Ext(t + 4); return b; }
inline int I8(int t) { int b = 0; b += Ext(t + 1); b += Ext(t + 2); b += Ext(t + 3); b += Ext(t + 4);
                       b += Ext(t + 5); b += Ext(t + 6); b += Ext(t + 7); b += Ext(t + 8); return b; }
inline int Wrap(int t) { int c = I4(t + 10); return c + 1; }
'''

def sites(helper: str, n: int) -> str:
    return ' '.join(f's += {helper}({1000 * (i + 1)});' for i in range(n))

def probes() -> list[tuple[str, str, str, int]]:
    """(name, body, counted helper, site count)"""
    out = []
    # 1. floor: the same 18 expansions whatever the site count (20..40)
    for n in (16, 20, 32, 40):
        out.append((f'floor_{n}', f'int floor_{n}(int q) {{ int s = 0; {sites("I4", n)} return s; }}', 'I4', n))
    # 2. growth: filler statements raise the budget (about 1.5x their node count)
    for k in (0, 50, 100, 200):
        fill = ' '.join(f'q += Ext({i + 2});' for i in range(k))
        out.append((f'grow_{k}', f'int grow_{k}(int q) {{ int s = 0; {fill} {sites("I4", 96)} return s; }}', 'I4', 96))
    # 3. statement weights: 64 copies of one statement kind, 96 I4 sites
    kinds = {'empty': ';', 'block': '{}', 'assign': 'q = 7;', 'add': 'q = q + 2;',
             'call': 'q += Ext(2);', 'if': 'if (q > 3) q = 1;', 'dead': 'if (0) q = 1;',
             'vctor': 'v = V3(f, f, f);', 'vplus': 'v = a + b;'}
    for name, stmt in kinds.items():
        out.append((f'weight_{name}', f'int weight_{name}(int q, float f, V3 v, V3 a, V3 b) {{ int s = 0; '
                    f'{" ".join([stmt] * 64)} {sites("I4", 96)} Sink(&v); return s; }}', 'I4', 96))
    # 4. helper size: bigger helpers get fewer expansions
    out.append(('size_i8', f'int size_i8(int q) {{ int s = 0; {sites("I8", 64)} return s; }}', 'I8', 64))
    # 5. nested sites are not degraded in source order
    out.append(('nested_48', f'int nested_48(int q) {{ int s = 0; {sites("Wrap", 48)} return s; }}', 'I4', 48))
    return out

def main() -> None:
    ap = argparse.ArgumentParser()
    ap.add_argument('--vc6-root', default=os.environ.get('VC6_ROOT'))
    ap.add_argument('--out', default='work/inline_budget')
    ap.add_argument('--profile', default='vc6_o2_mt')
    a = ap.parse_args()
    if not a.vc6_root:
        raise SystemExit('set VC6_ROOT or pass --vc6-root')
    out = Path(a.out); out.mkdir(parents=True, exist_ok=True)
    src = out / 'probe.cpp'; obj = out / 'probe.obj'
    plan = probes()
    src.write_text(HELPERS + '\n'.join(p[1] for p in plan) + '\n')
    root = Path(__file__).resolve().parent.parent
    r = subprocess.run([sys.executable, str(root / 'tools/compile.py'), '--compiler', 'vc6', '--vc6-root', a.vc6_root,
                        '--profile', a.profile, str(src), '-o', str(obj)], capture_output=True, text=True, cwd=root)
    if r.returncode:
        print(r.stdout[-3000:], r.stderr[-1000:]); raise SystemExit(r.returncode)
    o = CoffObject(str(obj))
    byidx = {s.index: s.name for s in o.symbols}
    plan_by_name = {p[0]: p for p in plan}
    for s in o.symbols:
        base = re.match(r'\?(\w+)@@', s.name)
        if s.section_number <= 0 or s.storage_class != 2 or not base or base.group(1) not in plan_by_name:
            continue
        raw, _, rels = o.symbol_extent(s)
        calls = collections.Counter(re.sub(r'@@.*', '', byidx.get(r.symbol_index, '')) for r in rels)
        _, _, helper, n = plan_by_name[base.group(1)]
        out_of_line = calls.get('?' + helper, 0) + calls.get('?Wrap', 0)
        others = ', '.join(f'{k}x{v}' for k, v in sorted(calls.items()) if k and k not in ('?Ext', '?Sink') and not k.startswith('_'))
        print(f'{base.group(1):14} size {len(raw):5}  {helper} sites {n:3}  expanded {n - out_of_line:3}  out of line: {others or "-"}')

if __name__ == '__main__':
    main()
