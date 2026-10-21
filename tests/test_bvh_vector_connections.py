"""Complete BVH objects and native point-leaf calls use actual vector providers."""
from collections import Counter
import hashlib
import json
import os
from pathlib import Path
import re
import subprocess
import sys
import tempfile
import unittest

from mcm2tool.coff import CoffObject
from mcm2tool.pe import PEImage
from mcm2tool.toolchain import find_child_ci, find_vc6_bin, infer_vc98_root
from tools.match import compare

ROOT = Path(__file__).resolve().parents[1]
QUERY = 'src/krusty2/bvh/BoundingBoxTreeQuery.cpp'
SAMPLE = 'samples/physics/bvh/BoundingBoxTreeQueryNearMisses.cpp'
VECTOR = 'src/reconstructed/VectorCalls.cpp'
LEFT = 'src/reconstructed/VectorScaleLeft.cpp'
COLLISION = 'src/krusty2/collision/CollisionObject.cpp'
FAST = 'src/reconstructed/FastMath.cpp'
FIXTURE = 'tests/fixtures/bvh_vector_compare.cpp'
RENAMES = {
    '?Vec3Add@@YA?AUVec3@@ABU1@0@Z': '?Vec3AddCall@@YAPAUVec3@@PAU1@PBU1@1@Z',
    '?Vec3Sub@@YA?AUVec3@@ABU1@0@Z': '?Vec3SubtractCall@@YAPAUVec3@@PAU1@PBU1@1@Z',
    '?Vec3Scale@@YA?AUVec3@@ABU1@M@Z': '?Vec3ScaleCall@@YAPAUVec3@@PAU1@PBU1@M@Z',
    '?Vec3Length@@YAMABUVec3@@@Z': '?CollisionLength@@YAMPBUCollisionVec3@@@Z',
}


def definition(text, name):
    match = re.search(r'^[^\n;{}]*\b' + re.escape(name) + r'\([^;{}]*\)\s*\{', text, re.M)
    assert match, name
    depth, end = 1, match.end()
    while depth:
        depth += (text[end] == '{') - (text[end] == '}')
        end += 1
    return text[match.start():end]


def object_fingerprint(obj, renames=None):
    renames = renames or {}
    sections = [s for s in obj.sections if not s.name.startswith('.debug')]
    keys = {s.index: tuple(sorted(t.name for t in obj.symbols
                                  if t.section_number == s.index and t.storage_class == 2))
            or ('section', n, s.name) for n, s in enumerate(sections)}
    def target(symbol):
        if symbol.section_number > 0:
            return (keys[symbol.section_number], symbol.value)
        return ('external', renames.get(symbol.name, symbol.name), symbol.value, symbol.section_number)
    rows = []
    for section in sections:
        data = (obj.data[section.raw_ptr:section.raw_ptr + section.raw_size]
                if section.raw_ptr else bytes(section.raw_size))
        relocations = [(r.virtual_address, r.type, target(obj.symbol_by_index[r.symbol_index]))
                       for r in obj.relocations if r.section_number == section.index]
        rows.append((keys[section.index], section.name, section.characteristics, data.hex(), relocations))
    return {'sections': len(rows), 'relocations': sum(len(row[4]) for row in rows),
            'sha256': hashlib.sha256(json.dumps(rows).encode()).hexdigest()}


@unittest.skipUnless(os.name == 'nt' and os.environ.get('VC6_ROOT') and os.environ.get('MCM2_EXE'),
                     'requires native VC6 and private retail image')
class BvhVectorConnectionsTests(unittest.TestCase):
    def test_complete_objects_providers_and_point_leaf_integration(self):
        import capstone
        image = Path(os.environ['MCM2_EXE']).resolve()
        self.assertEqual(hashlib.sha256(image.read_bytes()).hexdigest(),
                         '31fde4cc686a5ee89ef9095b90235325b195596867ecacefe511263e1509b874')
        retail = PEImage(image)
        decoder = capstone.Cs(capstone.CS_ARCH_X86, capstone.CS_MODE_32)
        decoder.detail = True
        baseline = json.loads((ROOT/'tests/fixtures/bvh_vector_objects.json').read_text())['objects']
        with tempfile.TemporaryDirectory(prefix='mcm2-bvh-vectors-') as directory:
            output = Path(directory)
            query_text = (ROOT/QUERY).read_text()
            extracted = {
                'capsule': '#include "bvh/BoundingBoxTreeQuery.h"\n#include "collision/CollisionPoint.h"\nstatic Vec3* s_vertices;\n' +
                    definition(query_text, 'Vec3Call::Vec3Call') + '\n' + definition(query_text, 'CapsuleTreeQuery'),
                'length': '#include "collision/CollisionPoint.h"\n' +
                    definition((ROOT/COLLISION).read_text(), 'CollisionLength'),
            }
            paths, objects = {}, {}
            jobs = [(source, ROOT/source, []) for source in (QUERY, SAMPLE, VECTOR, LEFT, COLLISION, FAST)]
            for key, text in extracted.items():
                path = output/(key+'.cpp')
                path.write_text(text+'\n')
                jobs.append((key, path, []))
            for index, (key, source, extra) in enumerate(jobs):
                obj = output/('unit%d.obj'%index)
                result = subprocess.run([sys.executable, str(ROOT/'tools/compile.py'), str(source), '-o', str(obj),
                    '--vc6-root', os.environ['VC6_ROOT'], '--profile', 'vc6_o2_mt',
                    '--include', str(ROOT/'src/krusty2'), *extra], cwd=ROOT, capture_output=True,
                    text=True, encoding='latin1', env={**os.environ, 'PYTHONPATH': str(ROOT)})
                self.assertEqual(result.returncode, 0, result.stdout+result.stderr)
                paths[key], objects[key] = obj, CoffObject(obj)
            for source in (QUERY, SAMPLE):
                self.assertEqual(object_fingerprint(objects[source]), baseline[source], source)
            imports = {s.name for s in objects[QUERY].symbols if s.section_number == 0}
            self.assertFalse(set(RENAMES) & imports)
            self.assertTrue(set(RENAMES.values()) <= imports)
            # Separate aggregate methods retain their existing provisional calls.
            self.assertTrue(set(RENAMES) & {s.name for s in objects[SAMPLE].symbols if s.section_number == 0})
            strict = 0
            targets = []
            for base in ('src/krusty2/bvh', 'samples/physics/bvh'):
                manifest = ROOT/base/'targets.json'
                for target in json.loads(manifest.read_text()):
                    if not target.get('source') or not target.get('bindings'):
                        continue
                    source = (manifest.parent/target['source']).resolve().relative_to(ROOT).as_posix()
                    if source in (QUERY, SAMPLE) and target.get('expect') == 'exact':
                        targets.append((source, target['candidate_symbol_contains'], int(target['target_va'], 0),
                                        target['target_size'], manifest.parent/target['bindings']))
            providers = [(VECTOR, '?Vec3AddCall@@', 0x421cb0, 69),
                         (VECTOR, '?Vec3SubtractCall@@', 0x421d00, 69),
                         (VECTOR, '?Vec3ScaleCall@@', 0x5015b0, 69),
                         (VECTOR, '?Vec3DotCall@@', 0x40ae30, 29),
                         (LEFT, '?Vec3ScaleLeft@@', 0x428090, 69),
                         (COLLISION, '?CollisionLength@@', 0x435ec0, 74),
                         (FAST, '?FastSqrt@@', 0x460b50, 85),
                         (FAST, '?InitFastSqrtTable@@', 0x460ae0, 97)]
            targets += [(source, symbol, va, size, (ROOT/source).with_suffix('.bindings.json'))
                        for source, symbol, va, size in providers]
            targets += [('capsule', '?CapsuleTreeQuery@@', 0x4298c0, 1436, (ROOT/QUERY).with_suffix('.bindings.json')),
                        ('capsule', '??0Vec3Call@@', 0x404e60, 25, (ROOT/QUERY).with_suffix('.bindings.json')),
                        ('length', '?CollisionLength@@', 0x435ec0, 74, (ROOT/COLLISION).with_suffix('.bindings.json'))]
            for source, symbol, va, size, bindings in dict.fromkeys(targets):
                result = compare(retail, objects[source], symbol, va, size, bindings)
                self.assertTrue(result['strict_exact'], (source, symbol, result))
                self.assertEqual(result['candidate_size'], size)
                strict += 1
            caller = objects[QUERY]
            entry = caller.find_symbol('?CapsuleTreeQuery@@')
            _, _, relocations = caller.symbol_extent(entry)
            names = Counter(caller.symbol_by_index[r.symbol_index].name for r in relocations if r.type==20)
            wanted = dict(zip(RENAMES.values(), (2, 8, 1, 1)))
            self.assertEqual({name: names[name] for name in wanted}, wanted)
            addresses = (0x421cb0, 0x421d00, 0x5015b0, 0x435ec0)
            instructions = list(decoder.disasm(retail.bytes_at_va(0x4298c0, 1436), 0x4298c0))
            self.assertEqual(sum(i.size for i in instructions), 1436)
            direct = Counter(i.operands[0].imm for i in instructions if i.mnemonic=='call' and
                             i.operands[0].type==capstone.x86.X86_OP_IMM)
            self.assertEqual({va: direct[va] for va in addresses}, dict(zip(addresses,(2,8,1,1))))
            # Only reference dependency calls are redirected. The full 1436-byte
            # caller and the actual candidate/extracted objects remain unchanged.
            patch_targets = {0x421cb0:'Vec3AddCall', 0x421d00:'Vec3SubtractCall', 0x5015b0:'Vec3ScaleCall',
                             0x435ec0:'CollisionLength', 0x40ae30:'Vec3DotCall', 0x428090:'Vec3ScaleLeft'}
            patches = ['static void InstallPatches() {']
            for instruction in instructions:
                if instruction.mnemonic=='call' and instruction.operands[0].type==capstone.x86.X86_OP_IMM:
                    va=instruction.operands[0].imm
                    if va in patch_targets:
                        name=patch_targets[va]
                        address='(float (__cdecl*)(const Vec3*,const Vec3*))Vec3DotCall' if va==0x40ae30 else name
                        patches.append('    PatchCall(0x%x, 0x%x, (void*)%s);'%(instruction.address,va,address))
            patches.append('}')
            (output/'bvh_vector_patches.h').write_text('\n'.join(patches)+'\n')
            fixture = output/'fixture.obj'
            result = subprocess.run([sys.executable, str(ROOT/'tools/compile.py'), str(ROOT/FIXTURE), '-o', str(fixture),
                '--vc6-root', os.environ['VC6_ROOT'], '--profile', 'vc6_o2_mt', '--include', str(ROOT/'src/krusty2'),
                '--include', str(output)], cwd=ROOT, capture_output=True, text=True, encoding='latin1',
                env={**os.environ, 'PYTHONPATH': str(ROOT)})
            self.assertEqual(result.returncode, 0, result.stdout+result.stderr)
            compiler=Path(os.environ['VC6_ROOT'])
            executable=output/'bvh.exe'
            result=subprocess.run([str(find_child_ci(find_vc6_bin(compiler),'link.exe')), '/nologo',
                '/subsystem:console', '/base:0x100000', '/fixed', '/opt:ref', '/out:'+str(executable),
                '/libpath:'+str(find_child_ci(infer_vc98_root(compiler),'lib')),
                *(str(paths[key]) for key in ('capsule','length',VECTOR,LEFT,FAST)), str(fixture), 'kernel32.lib'],
                cwd=ROOT, capture_output=True, text=True, encoding='latin1')
            self.assertEqual(result.returncode, 0, result.stdout+result.stderr)
            result=subprocess.run([str(executable),str(image)], capture_output=True, text=True, timeout=60)
            self.assertEqual(result.returncode, 0, result.stdout+result.stderr)
            self.assertEqual(result.stdout.strip(), 'BVH vectors: 36866 point-leaf and boundary cases passed')
            print('%d strict checks; two complete objects; twelve converted calls; %s'%(strict,result.stdout.strip()))


if __name__ == '__main__':
    unittest.main()
