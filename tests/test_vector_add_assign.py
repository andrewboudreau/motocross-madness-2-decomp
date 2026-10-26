"""Existing in-place vector boundaries retain complete callers and sequential writes."""
from collections import Counter
import hashlib
import json
import os
import posixpath
from pathlib import Path
import subprocess
import sys
import tempfile
import unittest

from mcm2tool.coff import CoffObject
from mcm2tool.pe import PEImage
from mcm2tool.toolchain import find_child_ci, find_vc6_bin, infer_vc98_root
from tools.match import compare

ROOT = Path(__file__).resolve().parents[1]
SOURCE = 'src/reconstructed/VectorAddAssign.cpp'
VEHICLE = 'src/krusty2/vehicle/Vehicle.cpp'
QUERY = 'src/krusty2/bvh/BoundingBoxTreeQuery.cpp'
TRANSFORM = 'src/reconstructed/CollisionTransforms.cpp'
SAMPLE = 'samples/physics/bvh/BoundingBoxTreeQueryNearMisses.cpp'
COLLISION = 'src/krusty2/collision/CollisionObject.cpp'
FIXTURE = 'tests/fixtures/vector_add_assign_compare.cpp'
VIEW = '??YVec3AddAssignCall@@QAEAAU0@ABUVec3@@@Z'
CANONICAL = '??YVec3@@QAEAAU0@ABU0@@Z'
OLD = '??YVehVec3AddAssign@@QAEAAU0@ABUVec3@@@Z'


def fingerprint(obj, source):
    # Only Vehicle gains a canonical operator COMDAT. Its entire body is checked
    # separately against both the existing provider and retail, not ignored.
    added = {s.section_number for s in obj.symbols if source == VEHICLE and
             s.name == CANONICAL and s.section_number > 0}
    sections = [s for s in obj.sections if not s.name.startswith('.debug') and s.index not in added]
    def filename(data):
        # A complete __FILE__ literal is checkout-dependent. Preserve its source
        # suffix and any following bytes; no function/data payload is normalized.
        value, separator, suffix = data.partition(b'\0')
        literal = posixpath.normpath(value.decode('latin1').replace('\\', '/'))
        if separator and literal.endswith('/'+source):
            return ('source:'+source).encode()+separator+suffix
        return data
    def name(symbol):
        # The literal's mangled name spells the same checkout path.
        if symbol.name.startswith('??_C@') and symbol.section_number > 0:
            section = obj.section(symbol.section_number)
            data = obj.data[section.raw_ptr+symbol.value:section.raw_ptr+section.raw_size]
            if filename(data) != data:
                return 'source:'+source
        return symbol.name
    keys = {s.index: tuple(sorted(name(t) for t in obj.symbols if t.section_number == s.index and t.storage_class == 2))
            or ('section', i, s.name) for i, s in enumerate(sections)}
    def target(symbol):
        if source == VEHICLE and symbol.name in (CANONICAL, OLD):
            assert symbol.value == 0
            return ('verified-428060-provider', 0)
        if symbol.section_number > 0:
            return (keys[symbol.section_number], symbol.value)
        return ('external', symbol.name, symbol.value, symbol.section_number)
    rows = []
    for section in sections:
        data = obj.data[section.raw_ptr:section.raw_ptr+section.raw_size] if section.raw_ptr else bytes(section.raw_size)
        data = filename(data)
        relocations = [(r.virtual_address, r.type, target(obj.symbol_by_index[r.symbol_index]))
                       for r in obj.relocations if r.section_number == section.index]
        rows.append((keys[section.index], section.name, section.characteristics, data.hex(), relocations))
    return {'sections': len(rows), 'relocations': sum(len(row[4]) for row in rows),
            'sha256': hashlib.sha256(json.dumps(rows).encode()).hexdigest()}


@unittest.skipUnless(os.name == 'nt' and os.environ.get('VC6_ROOT') and os.environ.get('MCM2_EXE'),
                     'requires authentic VC6 and private retail')
class VectorAddAssignTests(unittest.TestCase):
    def test_full_callers_providers_and_native_sequential_writes(self):
        import capstone
        image = Path(os.environ['MCM2_EXE']).resolve()
        self.assertEqual(hashlib.sha256(image.read_bytes()).hexdigest(),
                         '31fde4cc686a5ee89ef9095b90235325b195596867ecacefe511263e1509b874')
        retail = PEImage(image)
        decoder = capstone.Cs(capstone.CS_ARCH_X86, capstone.CS_MODE_32)
        decoder.detail = True
        instructions = list(decoder.disasm(retail.bytes_at_va(0x428060,33),0x428060))
        self.assertEqual(sum(i.size for i in instructions),33)
        self.assertEqual([(i.mnemonic,i.op_str) for i in instructions], [
            ('mov','eax, ecx'), ('mov','ecx, dword ptr [esp + 4]'),
            ('fld','dword ptr [ecx]'), ('fadd','dword ptr [eax]'), ('fstp','dword ptr [eax]'),
            ('fld','dword ptr [ecx + 4]'), ('fadd','dword ptr [eax + 4]'), ('fstp','dword ptr [eax + 4]'),
            ('fld','dword ptr [ecx + 8]'), ('fadd','dword ptr [eax + 8]'), ('fstp','dword ptr [eax + 8]'),
            ('ret','4')])
        self.assertEqual(retail.bytes_at_va(0x428081,15), b'\x90'*15)
        baseline = json.loads((ROOT/'tests/fixtures/vector_add_assign_objects.json').read_text())['objects']
        with tempfile.TemporaryDirectory(prefix='mcm2-add-assign-') as directory:
            output = Path(directory)
            # Taking the actual header member's address naturally emits its
            # canonical COMDAT. This test-only object is checked byte-for-byte
            # against both actual production emissions, not a rewritten method.
            emission = output/'canonical.cpp'
            emission.write_text('#include "math/Vec3.h"\nVec3& (Vec3::*g_CanonicalAdd)(const Vec3&) = &Vec3::operator+=;\n')
            objects, paths = {}, {}
            jobs = [(s, ROOT/s, []) for s in (SOURCE, VEHICLE, QUERY, TRANSFORM, SAMPLE, COLLISION)]
            jobs += [('canonical', emission, []), (FIXTURE, ROOT/FIXTURE, ['--extra','/Od'])]
            for index,(key,source,extra) in enumerate(jobs):
                obj=output/('unit%d.obj'%index)
                result=subprocess.run([sys.executable,str(ROOT/'tools/compile.py'),str(source),'-o',str(obj),
                    '--vc6-root',os.environ['VC6_ROOT'],'--profile','vc6_o2_mt',
                    '--include',str(ROOT/'src/krusty2'),*extra],cwd=ROOT,capture_output=True,text=True,
                    encoding='latin1',env={**os.environ,'PYTHONPATH':str(ROOT)})
                self.assertEqual(result.returncode,0,result.stdout+result.stderr)
                objects[key],paths[key]=CoffObject(obj),obj
            for source in (VEHICLE,QUERY,TRANSFORM,SAMPLE):
                self.assertEqual(fingerprint(objects[source],source),baseline[source],source)
            strict=0
            for source,symbol in ((SOURCE,VIEW),(VEHICLE,CANONICAL),(COLLISION,CANONICAL),('canonical',CANONICAL)):
                obj=objects[source];entry=obj.find_symbol(symbol)
                self.assertEqual((entry.function_size,entry.extent_source),(33,'coff_function_aux'))
                body,size,relocs=obj.symbol_extent(entry)
                self.assertEqual((body,size,relocs),(retail.bytes_at_va(0x428060,33),33,[]))
                strict+=1
            provider=objects[SOURCE]
            self.assertEqual({s.name for s in provider.symbols if s.section_number==0},{'__fltused'})
            self.assertEqual({s.name for s in provider.symbols if s.section_number>0 and s.storage_class==2},{VIEW})
            self.assertFalse(any(s.name.startswith('.CRT') or s.characteristics&0xc0
                                 for s in provider.sections if not s.name.startswith('.debug')))
            for source,symbol,va,size,callee,sites in (
                (QUERY,'?LeafPairQuery@@',0x4275f0,736,VIEW,[0x42786a]),
                (VEHICLE,'?Method_00529450@Vehicle@@',0x529450,1415,CANONICAL,[0x52993b,0x5299ae])):
                obj=objects[source];entry=obj.find_symbol(symbol);body,extent,relocs=obj.symbol_extent(entry)
                self.assertEqual((entry.function_size,entry.extent_source),(size,'coff_function_aux'))
                self.assertTrue(compare(retail,obj,symbol,va,size,(ROOT/source).with_suffix('.bindings.json'))['strict_exact'])
                joined=[r for r in relocs if r.type==20 and obj.symbol_by_index[r.symbol_index].name==callee]
                self.assertEqual([r.virtual_address-entry.value for r in joined],[x+1-va for x in sites])
                code=list(decoder.disasm(retail.bytes_at_va(va,size),va))
                self.assertEqual([i.address for i in code if i.mnemonic=='call' and i.op_str=='0x428060'],sites)
                strict+=1
            self.assertFalse(any(s.name==OLD for s in objects[VEHICLE].symbols))
            # Preserve every registered strict body too. Full-object hashes above
            # retain all unadmitted partial bytes/targets without score exceptions.
            for base in ('src/krusty2/bvh','src/krusty2/vehicle','samples/physics/bvh'):
                manifest=ROOT/base/'targets.json'
                for target in json.loads(manifest.read_text()):
                    if not target.get('source') or target.get('expect')!='exact' or not target.get('bindings'):continue
                    source=(manifest.parent/target['source']).resolve().relative_to(ROOT).as_posix()
                    if source not in (VEHICLE,QUERY,SAMPLE):continue
                    result=compare(retail,objects[source],target['candidate_symbol_contains'],int(target['target_va'],0),
                                   target['target_size'],manifest.parent/target['bindings'])
                    self.assertTrue(result['strict_exact'],(source,target['candidate_symbol_contains'],result))
                    strict+=1
            compiler=Path(os.environ['VC6_ROOT']);executable=output/'add.exe'
            result=subprocess.run([str(find_child_ci(find_vc6_bin(compiler),'link.exe')),'/nologo','/subsystem:console',
                '/base:0x100000','/fixed','/opt:ref','/out:'+str(executable),
                '/libpath:'+str(find_child_ci(infer_vc98_root(compiler),'lib')),
                str(paths[SOURCE]),str(paths['canonical']),str(paths[FIXTURE]),'kernel32.lib'],
                cwd=ROOT,capture_output=True,text=True,encoding='latin1')
            self.assertEqual(result.returncode,0,result.stdout+result.stderr)
            result=subprocess.run([str(executable),str(image)],capture_output=True,text=True,timeout=60)
            self.assertEqual(result.returncode,0,result.stdout+result.stderr)
            self.assertEqual(result.stdout.strip(),'Vector add-assign: 34014 alias, FP and partial-fault cases match')
            print('%d strict checks; four complete old objects; three calls; %s'%(strict,result.stdout.strip()))


if __name__=='__main__':
    unittest.main()
