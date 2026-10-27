"""Private retained prototype: actual vegetation getters and unchanged query consumers."""
import hashlib,json,os,struct,subprocess,sys,tempfile,unittest
from pathlib import Path
from unittest.mock import patch
ROOT=Path(__file__).resolve().parents[1]
sys.path.insert(0,str(ROOT))
from mcm2tool.coff import CoffObject
from mcm2tool.pe import PEImage
from mcm2tool.rtti import parse_rtti
from mcm2tool.toolchain import find_child_ci,find_vc6_bin,infer_vc98_root
from tools.match import compare
from tools.run_calibration import CASES
import test_eco_quadtree_connections as object_view
SOURCES=('src/krusty2/collision/CollisionObject.cpp','samples/physics/collision/CollisionObjectNearMisses.cpp',
         'src/reconstructed/EcoSystem.cpp','samples/ecosystem/EcoSystemNearMisses.cpp')
RENAMES={'?GetObjectCount@CollisionVegetation@@QAEHXZ':'?GetCollisionCount@Vegetation@@QAEHXZ',
         '?GetRadius@CollisionVegetation@@QAEMXZ':'?GetRadius@Vegetation@@QAEMXZ',
         '?g_vegetationGrid@@3PAUCollisionVegetationGrid@@A':'?g_EcoSystem@@3PAVEcoSystem@@A'}
GETTERS=(('int Vegetation::GetCollisionCount(', '?GetCollisionCount@Vegetation@@QAEHXZ',0x457080,22),
         ('float Vegetation::GetRadius(', '?GetRadius@Vegetation@@QAEMXZ',0x457230,24),
         ('float UnknownEcoDefinition::RadiusForParameter(', '?RadiusForParameter@UnknownEcoDefinition@@QAEME@Z',0x455ff0,95))
def extract(text,prefix):
    start=text.index(prefix);pos=text.index('{',start);depth=1;end=pos+1
    while depth:
        depth+=(text[end]=='{')-(text[end]=='}');end+=1
    return text[start:end]
def signature(obj,name):
    s=obj.find_symbol(name);raw,size,rels=obj.symbol_extent(s)
    return raw.hex(),size,[(r.virtual_address-s.value,r.type,obj.symbol_by_index[r.symbol_index].name)for r in rels]
@unittest.skipUnless(os.name=='nt' and os.environ.get('VC6_ROOT') and os.environ.get('MCM2_EXE'),'authentic native VC6 and retail required')
class VegetationConnectionsTests(unittest.TestCase):
    def test_complete_objects_primary_receivers_and_actual_getters(self):
        import capstone
        exe=Path(os.environ['MCM2_EXE']);image=PEImage(exe)
        self.assertEqual(hashlib.sha256(exe.read_bytes()).hexdigest(),'31fde4cc686a5ee89ef9095b90235325b195596867ecacefe511263e1509b874')
        decoder=capstone.Cs(capstone.CS_ARCH_X86,capstone.CS_MODE_32)
        def decode(va,size):
            code=list(decoder.disasm(image.bytes_at_va(va,size),va));self.assertEqual(sum(i.size for i in code),size)
            return {i.address:(i.mnemonic,i.op_str)for i in code}
        classes={c.name:c for c in parse_rtti(image)}
        for name,table,col in [('Vegetation',0x5524f8,0x55bef0),('EcoSystem',0x552508,0x55bf60)]:
            actual_col=struct.unpack('<I',image.bytes_at_va(table-4,4))[0]
            self.assertEqual(actual_col,col)
            words=struct.unpack('<5I',image.bytes_at_va(actual_col,20));self.assertEqual(words[1],0)
            self.assertEqual(image.bytes_at_va(words[3]+8,80).split(b'\0')[0],('.?AV'+name+'@@').encode())
        self.assertTrue(any(b.name=='QuadTreeObject' and b.mdisp==0 and b.pdisp==-1 for b in classes['Vegetation'].bases))
        self.assertEqual(image.bytes_at_va(0x5678f8,11),b'Vegetation\0')
        op={}
        for va,size in ((0x438e70,1102),(0x431e70,346),(0x456650,58),(0x4567a0,53),(0x457250,180),(0x457330,335),(0x4594d0,859),(0x4598d0,455)):
            op.update(decode(va,size))
        expected={
            0x431ee6:('mov','ecx, dword ptr [0x575744]'),0x459639:('mov','ecx, dword ptr [0x575744]'),
            0x431eec:('push','0x5678f8'),0x431ef1:('call','0x521ea0'),0x431ef6:('mov','byte ptr [0x568611], al'),
            0x45963f:('push','0x5678f8'),0x459644:('call','0x521ea0'),0x45964c:('mov','byte ptr [esi + 0x599], al'),
            0x45665c:('mov','dword ptr [eax], 0x5524f8'),0x45667a:('mov','ecx, dword ptr [0x59aebc]'),
            0x456680:('mov','dl, byte ptr [ecx + 0x599]'),0x456686:('mov','byte ptr [eax + 8], dl'),
            0x4567ac:('lea','edx, [ecx + 0xc]'),0x4567b1:('mov','dword ptr [edx], esi'),0x4567b8:('mov','word ptr [edx + 4], ax'),
            0x45994e:('mov','edx, dword ptr [esi + 0x38]'),0x459954:('mov','ecx, ebp'),0x459956:('add','ecx, edx'),0x459958:('call','0x4567a0'),
            0x459a23:('add','ecx, dword ptr [esi + 0x38]'),0x459a26:('push','ecx'),0x459a2d:('call','0x4dcac0'),
            0x438f10:('call','0x4dd540'),0x438f15:('mov','edi, eax'),0x438f20:('mov','al, byte ptr [edi + 8]'),
            0x438ffa:('cmp','al, byte ptr [0x568611]'),0x439000:('jne','0x439127'),
            0x439006:('mov','eax, dword ptr [0x59aebc]'),0x43900d:('mov','dx, word ptr [edi + 0xc]'),0x439017:('mov','cx, word ptr [edi + 0x10]'),
            0x439025:('fmul','dword ptr [eax + 0x5a8]'),0x439033:('fmul','dword ptr [eax + 0x5a8]'),
            0x439023:('mov','ecx, edi'),0x43903d:('call','0x457230'),0x439042:('fstp','st(0)'),0x439044:('mov','ecx, edi'),0x439046:('call','0x457230'),
            0x439065:('mov','ecx, edi'),0x439069:('call','0x457080'),0x439117:('mov','ecx, edi'),0x43911a:('call','0x457080'),
            0x439079:('call','0x4570a0'),0x4572e0:('mov','dword ptr [0x59aebc], esi'),0x4572e6:('mov','dword ptr [esi + 0x5a8], eax'),
            0x457455:('mov','dword ptr [0x59aebc], eax')}
        for va,value in expected.items():self.assertEqual(op[va],value,hex(va))
        pins=json.loads((ROOT/'tests/fixtures/vegetation_connections_objects.json').read_text());self.assertEqual(pins['renames'],RENAMES)
        vc=Path(os.environ['VC6_ROOT'])
        with tempfile.TemporaryDirectory(prefix='mcm2-vegetation-')as directory:
            output=Path(directory);objects={}
            def compile_source(source,name):
                path=output/(name+'.obj')
                r=subprocess.run([sys.executable,str(ROOT/'tools/compile.py'),str(source),'-o',str(path),'--vc6-root',str(vc),'--profile','vc6_o2_mt',
                    '--include',str(ROOT/'src/krusty2'),'--include',str(ROOT/'src/reconstructed')],cwd=ROOT,capture_output=True,text=True,encoding='latin1',env={**os.environ,'PYTHONPATH':str(ROOT)})
                self.assertEqual(r.returncode,0,r.stdout+r.stderr);return path,CoffObject(path)
            for n,source in enumerate(SOURCES):
                _,obj=compile_source(ROOT/source,'whole'+str(n));objects[source]=obj
                with patch.object(object_view,'FILENAMES',SOURCES):
                    view=object_view.whole_object(obj,(ROOT,))
                pin={'sections':len(view),'relocations':sum(len(r[-1])for r in view),'sha256':hashlib.sha256(json.dumps(view,separators=(',',':')).encode()).hexdigest()}
                self.assertEqual(pin,pins['objects'][source])
            cases=[(c['source'],c['symbol'],int(c['target_va'],0),c['target_size'],ROOT/c['bindings'])for c in CASES if c['source']in SOURCES]
            for parent in ('src/krusty2','samples/physics'):
                for manifest in (ROOT/parent).rglob('targets.json'):
                    for c in json.loads(manifest.read_text()):
                        if 'source'not in c or c.get('expect')=='partial' or not c.get('bindings'):continue
                        source=(manifest.parent/c['source']).resolve().relative_to(ROOT).as_posix()
                        if source in SOURCES:cases.append((source,c['candidate_symbol_contains'],int(c['target_va'],0),c['target_size'],manifest.parent/c['bindings']))
            cases=list(dict.fromkeys(cases));strict=0
            required={(0x438e70,1102),(0x431e70,346),(0x456650,58),(0x4567a0,53),(0x457080,22),(0x457230,24),(0x455ff0,95),(0x457250,180),(0x457330,335),(0x4594d0,859),(0x4598d0,455)}
            self.assertTrue(required<={(va,size)for _,_,va,size,_ in cases})
            for source,symbol,va,size,bindings in cases:
                self.assertTrue(compare(image,objects[source],symbol,va,size,bindings)['strict_exact'],(source,symbol));strict+=1
            # The later placement slice separately connects the genuine primary receiver.
            for source in SOURCES[:2]:
                obj=objects[source];imports={s.name for s in obj.symbols if s.storage_class==2 and s.section_number==0}
                self.assertFalse(imports&set(RENAMES));self.assertTrue(set(RENAMES.values())<=imports)
                self.assertNotIn('?GetObject@CollisionVegetation@@QAEPAVCollisionObject@@H@Z',imports)
                self.assertIn('?GetCollisionObject@Vegetation@@QAEPAVCollisionObject@@H@Z',imports)
                symbol=obj.find_symbol('QueryCollisions@CollisionObject');_,size,rels=obj.symbol_extent(symbol);self.assertEqual(size,1102)
                calls=[(r.virtual_address-symbol.value,obj.symbol_by_index[r.symbol_index].name)for r in rels if obj.symbol_by_index[r.symbol_index].name in RENAMES.values()]
                self.assertEqual(calls,[(407,'?g_EcoSystem@@3PAVEcoSystem@@A'),(462,'?GetRadius@Vegetation@@QAEMXZ'),(471,'?GetRadius@Vegetation@@QAEMXZ'),(506,'?GetCollisionCount@Vegetation@@QAEHXZ'),(683,'?GetCollisionCount@Vegetation@@QAEHXZ')])
            text=(ROOT/SOURCES[2]).read_text();extracted=output/'getters.cpp'
            self.assertIn('EcoSystem* g_EcoSystem;',text)
            extracted.write_text('#include "EcoSystem.h"\nEcoSystem* g_EcoSystem;\n'+'\n'.join(extract(text,prefix)for prefix,_,_,_ in GETTERS)+'\n')
            native_path,native=compile_source(extracted,'getters')
            for _,symbol,va,size in GETTERS:
                self.assertEqual(signature(native,symbol),signature(objects[SOURCES[2]],symbol))
                self.assertTrue(compare(image,native,symbol,va,size,ROOT/'src/reconstructed/EcoSystem.bindings.json')['strict_exact']);strict+=1
            self.assertEqual({s.name for s in native.symbols if s.section_number==0 and s.storage_class==2},{'__fltused'})
            glob=native.find_symbol('?g_EcoSystem@@3PAVEcoSystem@@A');self.assertEqual(glob.value,0)
            section=native.section(glob.section_number);self.assertEqual((section.name,section.raw_size),('.bss',4))
            self.assertFalse([s for s in native.sections if s.name.startswith('.CRT')])
            fixture,_=compile_source(ROOT/'tests/fixtures/vegetation_getters.cpp','fixture')
            exeout=output/'getters.exe';r=subprocess.run([str(find_child_ci(find_vc6_bin(vc),'link.exe')),'/nologo','/subsystem:console','/opt:ref','/out:'+str(exeout),
                '/libpath:'+str(find_child_ci(infer_vc98_root(vc),'lib')),str(native_path),str(fixture),'kernel32.lib'],capture_output=True,text=True,encoding='latin1')
            self.assertEqual(r.returncode,0,r.stdout+r.stderr)
            r=subprocess.run([str(exeout)],capture_output=True,text=True,timeout=45);self.assertEqual(r.returncode,0,r.stdout+r.stderr)
            self.assertEqual(r.stdout.strip(),'Vegetation getters: 65807 cases, 15 independent read AV')
        self.assertEqual(strict,120)
        print('%d strict; four complete objects; ten joined loads/calls; %s'%(strict,r.stdout.strip()))
if __name__=='__main__':unittest.main()
