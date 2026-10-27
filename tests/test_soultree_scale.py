"""Complete nonexact scale, actual callers/cache helpers, and native matrix state."""
import hashlib
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
from mcm2tool.toolchain import find_child_ci, find_vc6_bin, infer_vc98_root
from tools.match import compare
from tools.build_game import compile_source as build_compile_source
from test_steering_axis_from_points import signature, complete_object, definition

ROOT = Path(__file__).resolve().parents[1]
SOURCE = 'samples/render/D3DIMScale.cpp'
AGGREGATE = 'samples/physics/helpers/SoultreeBounds.cpp'
TREE = 'src/krusty2/soultree/soultree.cpp'
RENDER = 'src/reconstructed/D3DIMSoulTree.cpp'
CALLERS = ('src/reconstructed/ArcadeObject.cpp', 'src/reconstructed/LightEmitter.cpp')
SYMBOL = '?Scale@D3DIMSoultreeObject@@QAEXMMM@Z'
HELPERS = (('InvalidateWorldMatrix',0x4fb850,45),('InvalidateSiblingChain',0x4fb880,50),('MarkSubtreeDirty',0x4fdab0,38))


@unittest.skipUnless(os.name == 'nt' and os.environ.get('VC6_ROOT') and os.environ.get('MCM2_EXE'),
                     'requires native VC6 and hash-pinned private retail')
class SoultreeScaleTests(unittest.TestCase):
    def test_complete_scale_callers_helpers_and_native_matrix_state(self):
        import capstone
        from tools.run_calibration import CASES
        image = Path(os.environ['MCM2_EXE']).resolve()
        self.assertEqual(hashlib.sha256(image.read_bytes()).hexdigest(),
                         '31fde4cc686a5ee89ef9095b90235325b195596867ecacefe511263e1509b874')
        retail = PEImage(image)
        decoder = capstone.Cs(capstone.CS_ARCH_X86,capstone.CS_MODE_32)
        decoder.detail = True
        def decode(va,size,table=False):
            rows = list(decoder.disasm(retail.bytes_at_va(va,size),va))
            if not table:self.assertEqual(sum(i.size for i in rows),size)
            return rows
        body = decode(0x4fd340,630)
        self.assertEqual([(i.address,i.op_str) for i in body if i.mnemonic=='ret'],[(0x4fd5b3,'0xc')])
        self.assertEqual([(i.address,i.op_str) for i in body if i.mnemonic=='call'],[(0x4fd5a5,'0x4fb850')])
        self.assertEqual(retail.bytes_at_va(0x4fd5b6,10),b'\x90'*10)
        stores = [i for i in body if i.mnemonic=='fstp']
        self.assertEqual([i.op_str for i in stores],['dword ptr [edx]']+
            ['dword ptr [edx + %s]'%hex(i) if i>=10 else 'dword ptr [edx + %d]'%i for i in list(range(4,48,4))+[60]])
        at = {i.address:(i.mnemonic,i.op_str) for i in body}
        for va,ins in ((0x4fd35c,('rep stosd','dword ptr es:[edi], eax')),
                      (0x4fd36d,('rep movsd','dword ptr es:[edi], dword ptr [esi]'))):
            self.assertEqual(at[va],ins)
        # The constructor returns the primary model pointer; both callers store
        # and later reload it without the distinct +12 GameObject adjustment.
        instructions = {}
        for va,size,table in ((0x401310,468,False),(0x49e230,348,True),(0x43f160,275,False)):
            instructions.update({i.address:(i.mnemonic,i.op_str) for i in decode(va,size,table)})
        for va,ins in {
            0x40135d:('call','0x43f160'),0x401373:('mov','dword ptr [esi + 0x2c], eax'),
            0x401459:('mov','ecx, dword ptr [esi + 0x2c]'),0x401469:('push','eax'),
            0x40146a:('push','eax'),0x40146b:('push','eax'),0x40146c:('call','0x4fd340'),
            0x49e29b:('call','0x43f160'),0x49e2af:('mov','dword ptr [esi + 0x98], eax'),
            0x49e2d2:('mov','ecx, dword ptr [esi + 0x98]'),
            0x49e2dc:('push','0x3ecccccd'),0x49e2e1:('push','0x3ecccccd'),0x49e2e6:('push','0x3ecccccd'),
            0x49e2eb:('call','0x4fd340'),0x43f17b:('mov','dword ptr [esi], 0x5513ec'),
            0x43f183:('mov','dword ptr [esi + 0xc], 0x55137c'),0x43f269:('mov','eax, esi'),
        }.items():self.assertEqual(instructions[va],ins)
        for table,col,offset in ((0x5513ec,0x55b2b8,0),(0x55137c,0x55b248,12)):
            self.assertEqual(struct.unpack('<I',retail.bytes_at_va(table-4,4))[0],col)
            _,actual,_,td,_ = struct.unpack('<5I',retail.bytes_at_va(col,20))
            self.assertEqual(actual,offset)
            self.assertEqual(retail.bytes_at_va(td+8,40).split(b'\0')[0],b'.?AVD3DIMSoultreeObject@@')
        compiler = Path(os.environ['VC6_ROOT']).resolve()
        with tempfile.TemporaryDirectory(prefix='mcm2-scale-') as directory:
            output = Path(directory)
            def compile_source(source,name):
                path = output/(name+'.obj')
                run = subprocess.run([sys.executable,str(ROOT/'tools/compile.py'),str(source),'-o',str(path),
                    '--vc6-root',str(compiler),'--profile','vc6_o2_mt','--include',str(ROOT/'src/krusty2'),
                    '--include',str(ROOT/'src/reconstructed')],cwd=ROOT,capture_output=True,text=True,
                    encoding='latin1',env={**os.environ,'PYTHONPATH':str(ROOT)})
                self.assertEqual(run.returncode,0,run.stdout+run.stderr)
                return path,CoffObject(path)
            paths,objects = {},{}
            for i,source in enumerate((SOURCE,AGGREGATE,TREE,RENDER,*CALLERS)):
                paths[source],objects[source] = compile_source(ROOT/source,'unit%d'%i)
            candidate,aggregate = objects[SOURCE],objects[AGGREGATE]
            self.assertEqual(signature(candidate,SYMBOL),signature(aggregate,'?Scale@SoultreeObject'))
            symbol = candidate.find_symbol(SYMBOL)
            self.assertEqual((symbol.function_size,symbol.extent_source),(630,'coff_function_aux'))
            proof = compare(retail,candidate,SYMBOL,0x4fd340,630,(ROOT/SOURCE).with_suffix('.bindings.json'))
            self.assertFalse(proof['strict_exact']);self.assertEqual(proof['strict_match']['matching_positions'],618)
            self.assertEqual({s.name for s in candidate.symbols if s.section_number==0},
                             {'__fltused','?InvalidateWorldMatrix@SoultreeObject@@QAEXXZ'})
            self.assertEqual({s.name for s in candidate.symbols if s.storage_class==2 and s.section_number>0},{SYMBOL})
            self.assertEqual([(s.name,s.raw_size) for s in candidate.sections if not s.name.startswith('.debug')],
                             [('.drectve',40),('.text',640)])
            rows = complete_object(aggregate)
            self.assertEqual((len(rows),sum(len(r[3]) for r in rows)),(12,21))
            self.assertEqual(hashlib.sha256(json.dumps(rows).encode()).hexdigest(),'b23936459e562ea3ee9c40901297f7bc39b4324e8f9765482659db7f9a7b62b1')
            spec = json.loads((ROOT/'config/game_build.json').read_text())
            built = build_compile_source(ROOT/SOURCE,output/'game-command',compiler,
                [ROOT/p for p in spec['includes']],spec['profile'],{**os.environ,'PYTHONPATH':str(ROOT)})
            self.assertTrue(built['compiled'],Path(built['log']).read_text())
            self.assertEqual(complete_object(candidate),complete_object(CoffObject(Path(built['object']))))
            strict = 0
            for case in (c for c in CASES if c['source'] in CALLERS):
                proof = compare(retail,objects[case['source']],case['symbol'],int(case['target_va'],0),
                                case['target_size'],ROOT/case['bindings'])
                self.assertTrue(proof['strict_exact'],proof);strict += 1
            for source,query,address,call in ((CALLERS[0],'?Create@ArcadeObject',0x401310,0x40146c),
                                              (CALLERS[1],'?Create@LightEmitter',0x49e230,0x49e2eb)):
                obj = objects[source];s=obj.find_symbol(query);data,_,relocs=obj.symbol_extent(s)
                calls = [r for r in relocs if obj.symbol_by_index[r.symbol_index].name==SYMBOL]
                self.assertEqual([(r.type,address+r.virtual_address-s.value-1) for r in calls],[(20,call)])
                self.assertEqual(struct.unpack_from('<I',data,calls[0].virtual_address-s.value)[0],0)
            proof = compare(retail,objects[RENDER],'??0D3DIMSoultreeObject@@',0x43f160,275,(ROOT/RENDER).with_suffix('.bindings.json'))
            self.assertTrue(proof['strict_exact'],proof);strict += 1
            text = (ROOT/TREE).read_text();helper_source=output/'helpers.cpp'
            helper_source.write_text('#include "math/Math3D.h"\n'+'\n'.join(definition(text,'void SoultreeObject::'+n+'(') for n,_,_ in HELPERS)+'\n')
            helper_path,helpers=compile_source(helper_source,'helpers')
            for name,va,size in HELPERS:
                query='?'+name+'@SoultreeObject'
                self.assertEqual(signature(helpers,query),signature(objects[TREE],query))
                for obj in (helpers,objects[TREE]):
                    proof=compare(retail,obj,query,va,size,(ROOT/TREE).with_suffix('.bindings.json'))
                    self.assertTrue(proof['strict_exact'],proof);strict += 1
            fixture,_=compile_source(ROOT/'tests/fixtures/soultree_scale_compare.cpp','fixture')
            executable=output/'scale.exe'
            run=subprocess.run([str(find_child_ci(find_vc6_bin(compiler),'link.exe')),'/nologo','/subsystem:console',
                '/base:0x100000','/fixed','/out:'+str(executable),
                '/libpath:'+str(find_child_ci(infer_vc98_root(compiler),'lib')),str(paths[SOURCE]),str(helper_path),
                str(fixture),'kernel32.lib'],capture_output=True,text=True,encoding='latin1')
            self.assertEqual(run.returncode,0,run.stdout+run.stderr)
            run=subprocess.run([str(executable),str(image)],capture_output=True,text=True,timeout=90)
            self.assertEqual(run.returncode,0,run.stdout+run.stderr)
            self.assertIn('Soultree scale: 99827 complete-state, cache, FP and fault cases match',run.stdout)
            print('%d strict caller/helper checks; complete nonexact630B scale; '%strict+run.stdout.strip())


if __name__=='__main__':
    unittest.main()
