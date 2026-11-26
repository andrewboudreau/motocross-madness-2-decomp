"""Inventory of what still keeps src/ from reading like original source.

Counts, per tree (krusty2 / reconstructed / bringup) and per file:
  unnamed_fields   members named by offset: field_0x.., pad_0x.., x_0x.. (h_0xa4, n_0xe4, ...)
  unnamed_types    stand-in type names: Unknown*, *View, <Class>A<hex>, <Class>Q, *Global_<addr>, *_<addr>
  unnamed_funcs    UnknownVirtualSlotN / FUN_ / sub_ / Unknown* function names, Method_<addr>
  reinterpret      reinterpret_cast< occurrences
  offset_casts     raw byte-offset pointer arithmetic: (char*)x + 0x.., *(T*)((...)+0x..)
  addr_literals    hard-coded retail addresses used as values: (T*)0x00xxxxxx
  ptr_ints         pointers stored as int: 'int' members whose comment says pointer/object
Usage: python tools/source_inventory.py [--files N] [--json work/inventory/inventory.json]
Regex heuristics: track the trend, not exact counts.
"""
import re, sys, json, os, collections, argparse
ROOT = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
ap = argparse.ArgumentParser(); ap.add_argument('--files', type=int, default=15); ap.add_argument('--json'); a = ap.parse_args()

def strip_comments(t):
    t = re.sub(r'/\*.*?\*/', lambda m: '\n' * m.group(0).count('\n'), t, flags=re.S)
    return re.sub(r'//[^\n]*', '', t)

R = {
 'unnamed_fields': re.compile(r'\b(?:field|pad|unk|f|d|g|h|i|l|n|q|r|s|w|c|v|m|b|p|x|y|k|t|e|o)_0x[0-9a-fA-F]+\b|\bd3d_field_0x[0-9a-f]+\b'),
 'unnamed_types': re.compile(r'\b(?:Unknown[A-Z]\w*|[A-Z]\w*View\b|[A-Z][a-z]\w*?A[0-9A-F]{2,4}\b|[A-Z]\w*Global_[0-9a-f]{6,8}\b|[A-Z]\w+_00[0-9a-f]{6}\b)'),
 'unnamed_funcs': re.compile(r'\b(?:UnknownVirtualSlot\d+|FUN_[0-9a-f]+|sub_[0-9a-f]+|Method_?[0-9a-f]{6,8}|Slot\d+_?[0-9a-f]*)\s*\('),
 'reinterpret': re.compile(r'reinterpret_cast\s*<'),
 'offset_casts': re.compile(r'\(\s*(?:unsigned\s+)?char\s*\*\s*\)\s*\(?\s*\w+[\w\->.]*\s*\)?\s*\+\s*(?:0x[0-9a-fA-F]+|\d+)|\(\s*\w+\s*\*\s*\)\s*\(\s*\(\s*(?:unsigned\s+)?char\s*\*\s*\)'),
 'addr_literals': re.compile(r'\(\s*[\w:]+\s*\*+\s*\)\s*0x00[45][0-9a-fA-F]{5}\b'),
}
VIEW_DEF = re.compile(r'^\s*(?:struct|class)\s+(\w+)\s*(?::[^{]*)?\{', re.M)
STANDIN = re.compile(r'^(?:Unknown\w+|\w+View|[A-Z]\w*?A[0-9A-F]{2,4}|\w+Global_[0-9a-f]+|\w+Q)$')
RTTI = {c['name'] for c in json.load(open(os.path.join(ROOT, 'analysis', 'rtti_classes.json')))}
tot = collections.defaultdict(collections.Counter); per = {}
defs = collections.defaultdict(list)
for tree in ('krusty2', 'reconstructed', 'bringup'):
    for dp, dn, fn in os.walk(os.path.join(ROOT, 'src', tree)):
        for f in fn:
            if not f.endswith(('.cpp', '.h')): continue
            p = os.path.join(dp, f); rel = os.path.relpath(p, ROOT).replace(os.sep, '/')
            t = strip_comments(open(p, encoding='latin1').read())
            c = collections.Counter({k: len(r.findall(t)) for k, r in R.items()})
            c['unnamed_types'] = sum(1 for n in R['unnamed_types'].findall(t) if n not in RTTI)
            for m in VIEW_DEF.finditer(t):
                if STANDIN.match(m.group(1)) and m.group(1) not in RTTI: defs[tree].append((m.group(1), rel)); c['standin_defs'] += 1
            c['lines'] = t.count('\n')
            per[rel] = c; tot[tree].update(c)
keys = ['lines', 'unnamed_fields', 'standin_defs', 'unnamed_types', 'unnamed_funcs', 'reinterpret', 'offset_casts', 'addr_literals']
print('%-14s' % 'tree' + ''.join('%15s' % k for k in keys))
for tree, c in tot.items(): print('%-14s' % tree + ''.join('%15d' % c[k] for k in keys))
allc = sum(tot.values(), collections.Counter()); print('%-14s' % 'TOTAL' + ''.join('%15d' % allc[k] for k in keys))
def score(c): return c['unnamed_fields'] + 3 * c['standin_defs'] + c['unnamed_types'] + 2 * c['reinterpret'] + 3 * c['offset_casts'] + 2 * c['addr_literals']
print('\nworst files (weighted):')
for rel, c in sorted(per.items(), key=lambda kv: -score(kv[1]))[:a.files]:
    print('%5d  %-58s' % (score(c), rel) + ' '.join('%s=%d' % (k[:8], c[k]) for k in keys[1:] if c[k]))
print('\nstand-in type definitions: %d' % sum(len(v) for v in defs.values()))
for tree, v in defs.items():
    print(' ', tree, len(v), ', '.join(sorted({n for n, _ in v}))[:900])
if a.json: json.dump({'totals': tot, 'files': per, 'standins': defs}, open(a.json, 'w'), indent=1)
