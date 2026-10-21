"""Existing texture timer view, complete constructor and actual clock closure."""
from pathlib import Path
import sys,subprocess,os,json,hashlib,tempfile,unittest,struct
from unittest.mock import patch
from test_edit_box_on_char import extract
import test_eco_quadtree_connections as views
from mcm2tool.coff import CoffObject
from mcm2tool.pe import PEImage
from mcm2tool.toolchain import find_child_ci,find_vc6_bin,infer_vc98_root
from tools.match import compare
ROOT=Path(__file__).resolve().parents[1]
FILENAMES=(
 'src/reconstructed/ManagedTextureGroup.cpp',
 'samples/render/ManagedTextureGroupNearMisses.cpp',
 'src/reconstructed/SceneManager.cpp',
 'src/reconstructed/ProCircuitProcs.cpp',
 'src/reconstructed/ContainerList.h',
 'samples/render/../../src/reconstructed/ContainerList.h',
)
@unittest.skipUnless(os.name=='nt' and os.environ.get('VC6_ROOT') and os.environ.get('MCM2_EXE'),'authentic VC6/private retail required')
class TextureHistoryTimerTests(unittest.TestCase):
 def test_actual_constructor_clock_order_and_faults(self):
  with tempfile.TemporaryDirectory(prefix='mcm2-texture-timer-') as directory:
   R=T=ROOT;P=Path(directory);VC=Path(os.environ['VC6_ROOT'])
   self.assertEqual(hashlib.sha256(Path(os.environ['MCM2_EXE']).read_bytes()).hexdigest(),'31fde4cc686a5ee89ef9095b90235325b195596867ecacefe511263e1509b874')
   clock=extract((R/'src/reconstructed/PCGame.cpp').read_text(),'unsigned int ReadClock()')
   (P/'clock.cpp').write_text('#include <windows.h>\n#include <mmsystem.h>\n'+clock+'\n')
   items=[('provider',R/'src/reconstructed/TextureHistoryTimer.cpp'),('peak',R/'src/reconstructed/PeakHold.cpp'),('pcgame',R/'src/reconstructed/PCGame.cpp'),('clock',P/'clock.cpp'),('fixture',R/'tests/fixtures/texture_history_timer.cpp'),('managed',R/'src/reconstructed/ManagedTextureGroup.cpp'),('manager',R/'src/reconstructed/TextureMapManager.cpp')];objects={}
   for name,source in items:
    cmd=[sys.executable,str(R/'tools/compile.py'),str(source),'-o',str(P/(name+'-native.obj')),'--vc6-root',str(VC),'--profile','vc6_o2_mt']
    if name=='fixture':cmd+=['--include',str(T/'src/reconstructed'),'--extra','/Od']
    result=subprocess.run(cmd,capture_output=True,text=True,encoding='latin1',env={**os.environ,'PYTHONPATH':str(ROOT)});(P/(name+'-native.log')).write_text(result.stdout+result.stderr);assert result.returncode==0,result.stdout+result.stderr;objects[name]=CoffObject(P/(name+'-native.obj'))
   image=PEImage(Path(os.environ['MCM2_EXE']));proofs=[]
   import capstone
   md=capstone.Cs(capstone.CS_ARCH_X86,capstone.CS_MODE_32)
   decoded={}
   for va,size in [(0x50bed0,996),(0x511180,264),(0x4cb670,30)]:
    instructions=list(md.disasm(image.bytes_at_va(va,size),va));self.assertEqual(sum(i.size for i in instructions),size)
    decoded.update((i.address,(i.mnemonic,i.op_str))for i in instructions)
   expected={0x5111a5:('push','0x258'),0x5111aa:('call','0x4a3010'),
    0x5111d0:('mov','ecx, eax'),0x5111d2:('call','0x50bed0'),
    0x50c047:('push','0x1388'),0x50c04c:('lea','ecx, [esi + 0x1d4]'),0x50c057:('call','0x4cb670'),
    0x50c05c:('push','0x1388'),0x50c061:('lea','ecx, [esi + 0x1e0]'),0x50c067:('call','0x4cb670'),
    0x50c06c:('push','0x1388'),0x50c071:('lea','ecx, [esi + 0x1ec]'),0x50c077:('call','0x4cb670'),
    0x50c09b:('mov','dword ptr [esi], 0x5583c0'),
    0x4cb673:('mov','dword ptr [esi + 4], 0'),0x4cb67a:('call','0x4bfa80'),
    0x4cb67f:('mov','dword ptr [esi], eax'),0x4cb685:('mov','dword ptr [esi + 8], eax'),0x4cb68b:('ret','4')}
   for address,row in expected.items():self.assertEqual(decoded[address],row)
   col=struct.unpack('<I',image.bytes_at_va(0x5583bc,4))[0]
   words=struct.unpack('<5I',image.bytes_at_va(col,20));self.assertEqual(words[1],0)
   self.assertEqual(image.bytes_at_va(words[3]+8,80).split(b'\0')[0],b'.?AVManagedTextureGroup@@')
   for name,symbol,va,size,binding in [('provider','??0UnknownTimer@@QAE@H@Z',0x4cb670,30,T/'src/reconstructed/TextureHistoryTimer.bindings.json'),('peak','??0UnknownPeakHold',0x4cb670,30,R/'src/reconstructed/PeakHold.bindings.json'),('peak','?Peak@UnknownPeakHold',0x4cb690,28,R/'src/reconstructed/PeakHold.bindings.json'),('peak','?Record@UnknownPeakHold',0x4cb6b0,43,R/'src/reconstructed/PeakHold.bindings.json'),('pcgame','?ReadClock@@YAIXZ',0x4bfa80,29,R/'src/reconstructed/PCGame.bindings.json'),('clock','?ReadClock@@YAIXZ',0x4bfa80,29,R/'src/reconstructed/PCGame.bindings.json'),('managed','??0ManagedTextureGroup',0x50bed0,996,R/'src/reconstructed/ManagedTextureGroup.bindings.json'),('manager','?CreateGroup@TextureMapManager',0x511180,264,R/'src/reconstructed/TextureMapManager.bindings.json')]:
    proof=compare(image,objects[name],symbol,va,size,binding);assert proof['strict_exact'],proof;proofs.append(dict(object=name,symbol=symbol,va=hex(va),size=size,strict=True))
   def function(obj,symbol):
    s=obj.find_symbol(symbol);raw,size,rels=obj.symbol_extent(s);return raw.hex(),[(r.virtual_address-s.value,r.type,obj.symbol_by_index[r.symbol_index].name)for r in rels]
   assert function(objects['clock'],'?ReadClock@@YAIXZ')==function(objects['pcgame'],'?ReadClock@@YAIXZ')
   o=objects['provider'];assert [(s.name,s.raw_size)for s in o.sections if not s.name.startswith('.debug')]==[('.drectve',40),('.text',32)]
   assert [s.name for s in o.symbols if s.storage_class==2 and s.section_number==0]==['?ReadClock@@YAIXZ']
   linker=find_child_ci(find_vc6_bin(VC),'link.exe');lib=find_child_ci(infer_vc98_root(VC),'lib');exe=P/'timer.exe'
   result=subprocess.run([str(linker),'/nologo','/subsystem:console','/out:'+str(exe),'/libpath:'+str(lib),str(P/'provider-native.obj'),str(P/'clock-native.obj'),str(P/'fixture-native.obj'),'kernel32.lib','winmm.lib'],capture_output=True,text=True,encoding='latin1',env={**os.environ,'PYTHONPATH':str(ROOT)});assert result.returncode==0,result.stdout+result.stderr
   result=subprocess.run([str(exe)],capture_output=True,text=True,timeout=30);print(result.stdout+result.stderr);assert result.returncode==0,result.returncode;assert result.stdout.strip()=='Texture timer: 3072 actual clock cases, 1024 exact write AV; 128 platform boundary cases, 96 throws'

   self.assertEqual(len(proofs),8)
   self.assertEqual([(r.virtual_address-objects['managed'].find_symbol('??0ManagedTextureGroup').value,r.type) for r in objects['managed'].symbol_extent(objects['managed'].find_symbol('??0ManagedTextureGroup'))[2] if objects['managed'].symbol_by_index[r.symbol_index].name=='??0UnknownTimer@@QAE@H@Z'],[(392,20),(408,20),(424,20)])
   pins=json.loads((ROOT/'tests/fixtures/texture_history_timer_objects.json').read_text())['objects']
   for index,source in enumerate(pins):
    objpath=P/('whole%d.obj'%index)
    run=subprocess.run([sys.executable,str(ROOT/'tools/compile.py'),str(ROOT/source),'-o',str(objpath),'--vc6-root',str(VC),'--profile','vc6_o2_mt','--include',str(ROOT/'src/krusty2')],capture_output=True,text=True,encoding='latin1',env={**os.environ,'PYTHONPATH':str(ROOT)})
    self.assertEqual(run.returncode,0,run.stdout+run.stderr)
    with patch.object(views,'FILENAMES',FILENAMES):rows=views.whole_object(CoffObject(objpath),(ROOT,))
    actual=dict(sections=len(rows),relocations=sum(len(z[-1])for z in rows),sha256=hashlib.sha256(json.dumps(rows,separators=(',',':')).encode()).hexdigest())
    self.assertEqual(actual,pins[source])
   print('8 complete strict checks; four old objects; real WinMM and controlled platform boundaries')

if __name__=="__main__":unittest.main()
