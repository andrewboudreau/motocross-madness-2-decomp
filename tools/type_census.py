"""Census of every struct/class defined in src/: RTTI-confirmed, stand-in views, other provisional.
A stand-in is a type that re-describes part of another object (name ends in View, Unknown*, <X>A<hex>,
*Global_<addr>, *Q opaque) -- each one is a place the source does not yet use the real class.
Usage: python tools/type_census.py [--json work/inventory/types.json]"""
import re, os, json, collections, argparse
os.chdir(os.path.dirname(os.path.dirname(os.path.abspath(__file__))))
ap = argparse.ArgumentParser(); ap.add_argument('--json'); a = ap.parse_args()
rtti = {c['name'] for c in json.load(open('analysis/rtti_classes.json'))}
DEF = re.compile(r'^\s*(?:typedef\s+)?(struct|class)\s+(\w+)\s*(?::[^{;]*)?\{', re.M)
TD = re.compile(r'^\s*typedef\s+(?:struct\s+|class\s+)?(\w+)\s+(\w+)\s*;', re.M)
STAND = re.compile(r'(View$|^Unknown|^[A-Z][a-z]\w*?A[0-9A-F]{2,4}$|Global_[0-9a-f]+$|_00[0-9a-f]{6}$|^[A-Z]\w*Q$|Stub$|Proxy$|Shim$)')
defs = collections.defaultdict(list); aliases = []
for dp, dn, fn in os.walk('src'):
    for f in fn:
        if not f.endswith(('.cpp', '.h')): continue
        p = os.path.join(dp, f).replace(os.sep, '/')
        t = re.sub(r'//[^\n]*', '', open(p, encoding='latin1').read())
        for m in DEF.finditer(t): defs[m.group(2)].append(p)
        for m in TD.finditer(t):
            if m.group(1) != m.group(2) and m.group(1)[:1].isupper(): aliases.append((m.group(2), m.group(1), p))
names = set(defs)
real = sorted(n for n in names if n in rtti)
stand = sorted(n for n in names if n not in rtti and STAND.search(n))
dup = sorted(n for n in names if len(set(defs[n])) > 1)
other = sorted(n for n in names if n not in rtti and not STAND.search(n))
print('distinct types defined: %d   RTTI-confirmed: %d (of %d RTTI classes)   stand-in views: %d   other provisional: %d' % (len(names), len(real), len(rtti), len(stand), len(other)))
print('types defined in more than one file (duplicate declarations): %d' % len(dup))
print('typedef aliases of a capitalised type: %d' % len(aliases))
missing = sorted(rtti - names)
print('RTTI classes with no definition in src/: %d' % len(missing)); print('  ' + ', '.join(missing))
byfile = collections.Counter(defs[n][0] for n in stand)
print('stand-ins by file:'); [print('  %3d %s' % (c, f)) for f, c in byfile.most_common(25)]
if a.json: json.dump({'standins': {n: defs[n] for n in stand}, 'dup': {n: sorted(set(defs[n])) for n in dup}, 'aliases': aliases, 'missing_rtti': missing}, open(a.json, 'w'), indent=1)
