"""Full-extent transform proof, actual caller imports, and native alias behavior."""
from __future__ import annotations
import json
import os
from pathlib import Path
import struct
import subprocess
import sys
import tempfile
import unittest
from mcm2tool.coff import CoffObject
from mcm2tool.pe import PEImage
from mcm2tool.toolchain import find_child_ci, find_vc6_bin, infer_vc98_root, sha256_file
from tools.match import compare

ROOT = Path(__file__).resolve().parents[1]
EXE = Path(os.environ.get('MCM2_EXE', ROOT / 'work/game/mcm2.exe'))
MOVE = '?MoveBox@@YAXPAUVec3@@0PBUMatrix4@@@Z'
INVERSE = '?InvertRigidTransform@@YAXPAUMatrix4@@@Z'

@unittest.skipUnless(os.environ.get('VC6_ROOT') and EXE.is_file(),
                     'requires VC6_ROOT and private retail image')
class CollisionTransformProviders(unittest.TestCase):
    def test_bodies_callers_dependency_closure_and_aliases(self):
        self.assertEqual(sha256_file(EXE),
            '31fde4cc686a5ee89ef9095b90235325b195596867ecacefe511263e1509b874')
        retail = PEImage(EXE)
        for target, sites in {0x428db0: (0x426b92,0x4335cb,0x435b9c,0x435d68),
                              0x42e2b0: (0x4399be,0x439a47)}.items():
            for site in sites:
                call = retail.bytes_at_va(site,5)
                self.assertEqual(call[0],0xe8)
                self.assertEqual(site+5+struct.unpack('<i',call[1:])[0],target)
        sources = {
            'CollisionTransforms':'src/reconstructed/CollisionTransforms.cpp',
            'CollisionObject':'src/krusty2/collision/CollisionObject.cpp',
            'BoundingBoxTreeQuery':'src/krusty2/bvh/BoundingBoxTreeQuery.cpp',
        }
        with tempfile.TemporaryDirectory(prefix='mcm2-transforms-') as tmp:
            objects = {}
            def compile_source(source, name):
                path = Path(tmp)/(name+'.obj')
                result = subprocess.run([sys.executable,str(ROOT/'tools/compile.py'),
                    str(ROOT/source),'-o',str(path),'--vc6-root',os.environ['VC6_ROOT'],
                    '--profile','vc6_o2_mt','--include',str(ROOT/'src/krusty2')],
                    cwd=ROOT,capture_output=True,text=True,encoding='latin-1',
                    env={**os.environ,'PYTHONPATH':str(ROOT)})
                self.assertEqual(result.returncode,0,result.stdout+result.stderr)
                return path
            for name, source in sources.items():
                objects[name]=CoffObject(compile_source(source,name))
            provider=objects['CollisionTransforms']
            definitions={s.name for s in provider.symbols if s.storage_class==2 and s.section_number>0}
            self.assertTrue({MOVE,INVERSE}<=definitions)
            self.assertEqual({s.name for s in provider.symbols if s.section_number==0},{'__fltused'})
            constants=[s for s in provider.symbols if s.name.startswith('_kVec3')]
            self.assertEqual(len(constants),4)
            self.assertTrue(all(s.storage_class==3 for s in constants))
            self.assertEqual([(s.raw_size,s.reloc_count) for s in provider.sections
                              if s.name=='.CRT$XCU'],[(16,4)])
            matched=set()
            for target_file in ('samples/physics/bvh/targets.json',
                                'src/krusty2/bvh/targets.json','src/krusty2/collision/targets.json'):
                path=ROOT/target_file
                for target in json.loads(path.read_text()):
                    source=(path.parent/target['source']).resolve()
                    name=source.stem
                    if name not in objects or target.get('expect','exact')!='exact': continue
                    self.assertEqual(source,ROOT/sources[name])
                    report=compare(retail,objects[name],target['candidate_symbol_contains'],
                        int(target['target_va'],0),target['target_size'],path.parent/target['bindings'])
                    self.assertTrue(report['strict_exact'],report)
                    self.assertEqual(report['candidate_size'],target['target_size'])
                    matched.add((name,int(target['target_va'],0)))
            self.assertTrue({('CollisionTransforms',0x428db0),('CollisionTransforms',0x42e2b0)}<=matched)
            def imports(obj):
                return {obj.symbol_by_index[r.symbol_index].name for r in obj.relocations
                        if obj.symbol_by_index[r.symbol_index].section_number==0}
            self.assertTrue({MOVE,INVERSE}<=imports(objects['CollisionObject']))
            self.assertIn(MOVE,imports(objects['BoundingBoxTreeQuery']))
            self.assertFalse(any('CollisionInvertRigidCall' in s for s in imports(objects['CollisionObject'])))
            if os.name=='nt':
                fixture=compile_source('tests/fixtures/collision_transforms.cpp','fixture')
                compiler=Path(os.environ['VC6_ROOT'])
                linker=find_child_ci(find_vc6_bin(compiler),'link.exe')
                libraries=find_child_ci(infer_vc98_root(compiler),'lib')
                executable=Path(tmp)/'transforms.exe'
                result=subprocess.run([str(linker),'/nologo','/subsystem:console',
                    '/out:'+str(executable),'/libpath:'+str(libraries),
                    str(Path(tmp)/'CollisionTransforms.obj'),str(fixture)],
                    capture_output=True,text=True,encoding='latin-1')
                self.assertEqual(result.returncode,0,result.stdout+result.stderr)
                result=subprocess.run([str(executable)],capture_output=True,text=True,timeout=30)
                self.assertEqual(result.returncode,0,result.stdout+result.stderr)
                self.assertIn('Collision transforms: 229 geometry and alias cases passed',result.stdout)

if __name__=='__main__': unittest.main()
