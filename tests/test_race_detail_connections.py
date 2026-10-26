"""Race-view detail calls preserve full objects and execute the actual model route."""
import hashlib
import json
import os
from pathlib import Path
import struct
import subprocess
import sys
import tempfile
import unittest
from unittest.mock import patch

from mcm2tool.coff import CoffObject
from mcm2tool.pe import PEImage
from mcm2tool.rtti import parse_rtti
from mcm2tool.toolchain import find_child_ci, find_vc6_bin, infer_vc98_root
from tools.build_game import ROOT as TOOL_ROOT
from tools.match import compare
from tools.run_calibration import CASES
from test_garage_shadow_connections import function_text
import test_eco_quadtree_connections as whole

ROOT=Path(__file__).resolve().parents[1]
CALLER='src/reconstructed/QuarryStuntEvent.cpp'
SAMPLE='samples/game/QuarryStuntEventNearMisses.cpp'
RACE='src/reconstructed/BikeRace.cpp'
MODEL='src/reconstructed/D3DIMSoulTree.cpp'
STATE='src/reconstructed/GraphicsState.cpp'
OLD='?SetDetailLevel@UnknownKrustyBikeView@@QAEXH@Z'
NEW='?SetDetailLevel@BikeRace@@QAEXH@Z'
PROVIDERS=((RACE,NEW,0x423790,240,'void BikeRace::SetDetailLevel('),
 (MODEL,'?ApplyLodSettings@D3DIMSoultreeObject@@QAEXH@Z',0x4451e0,81,'void D3DIMSoultreeObject::ApplyLodSettings('),
 (MODEL,'?SetLodThreshold@D3DIMSoultreeObject@@QAEXM@Z',0x444d60,19,'void D3DIMSoultreeObject::SetLodThreshold('),
 (MODEL,'?SetUseLighting@D3DIMSoultreeObject@@QAEXH@Z',0x4444c0,23,'void D3DIMSoultreeObject::SetUseLighting('))
FILENAMES=(CALLER,SAMPLE)

def object_pin(obj):
 with patch.object(whole,'FILENAMES',FILENAMES):rows=whole.whole_object(obj,(ROOT,))
 return dict(sections=len(rows),relocations=sum(len(r[-1])for r in rows),sha256=hashlib.sha256(json.dumps(rows,separators=(',',':')).encode()).hexdigest())

def function(obj,symbol):
 body,size,rels=obj.symbol_extent(obj.find_symbol(symbol))
 return body,[(r.virtual_address,r.type,obj.symbol_by_index[r.symbol_index].name)for r in rels]

@unittest.skipUnless(os.name=='nt' and os.environ.get('VC6_ROOT') and os.environ.get('MCM2_EXE'),'requires authentic VC6 and retail image')
class RaceDetailConnections(unittest.TestCase):
 def test_complete_receivers_calls_and_actual_route(self):
  import capstone
  exe=Path(os.environ['MCM2_EXE'])
  self.assertEqual(hashlib.sha256(exe.read_bytes()).hexdigest(),'31fde4cc686a5ee89ef9095b90235325b195596867ecacefe511263e1509b874')
  image=PEImage(exe);md=capstone.Cs(capstone.CS_ARCH_X86,capstone.CS_MODE_32)
  def decode(va,size):
   ins=list(md.disasm(image.bytes_at_va(va,size),va));self.assertEqual(sum(i.size for i in ins),size);return ins
  # Literal RTTI proves primary BikeRace, not equivalence of its provisional view.
  col=struct.unpack('<5I',image.bytes_at_va(0x55ac68,20))
  self.assertEqual(col,(0,0,0,0x567ab0,0x55ac58))
  self.assertEqual(struct.unpack('<I',image.bytes_at_va(0x550d24-4,4))[0],0x55ac68)
  self.assertEqual(image.bytes_at_va(col[3]+8,32).split(b'\0')[0],b'.?AVBikeRace@@')
  classes={c.name:c for c in parse_rtti(image)}
  self.assertEqual({v.object_offset for v in classes['BikeRace'].vtable_records},{0})
  for base in classes['BikeRace'].bases:self.assertEqual((base.mdisp,base.pdisp),(0,-1))
  listing={i.address:(i.mnemonic,i.op_str)for va,size in ((0x4de590,7952),(0x417c30,632),(0x417ed0,6745),(0x4e0ce0,1980),(0x423790,240))for i in decode(va,size)}
  for va,value in {
   0x4df9db:('push','0x400'),0x4df9e0:('call','0x4a3010'),0x4df9fd:('mov','ecx, eax'),
   0x417c5a:('mov','dword ptr [esi], 0x550d24'),0x417ef3:('mov','ebp, ecx'),0x41990c:('mov','eax, ebp'),0x419926:('ret','0x24'),
   0x4df9ff:('call','0x417c30'),0x4dfa08:('mov','dword ptr [ebp + 0x34], eax'),
   0x4dfabb:('mov','ecx, dword ptr [ebp + 0x34]'),0x4dfabf:('call','0x417ed0'),
   0x4dfac6:('mov','dword ptr [ebp + 0x34], eax'),
   0x4e128a:('mov','ecx, dword ptr [esi + 0x34]'),0x4e128d:('push','edi'),0x4e128e:('call','0x423790'),
   0x4e139b:('mov','ecx, dword ptr [esi + 0x34]'),0x4e139e:('push','edi'),0x4e139f:('call','0x423790'),
   0x423798:('mov','edi, ecx'),0x4237bb:('mov','ebx, dword ptr [esp + 0x10]'),
   0x4237c0:('mov','ecx, dword ptr [ecx + 0x3bc]'),0x4237cf:('mov','eax, dword ptr [edx + 0x5c4]'),
   0x4237d5:('mov','ecx, dword ptr [eax + 0x1a0]'),0x42387d:('ret','4')}.items():self.assertEqual(listing[va],value,hex(va))
  with tempfile.TemporaryDirectory(prefix='mcm2-race-detail-')as directory:
   out=Path(directory);objects={};paths={}
   def compile(source,label,includes=True):
    source=Path(source);source=source if source.is_absolute()else ROOT/source
    dest=out/(label+'.obj');args=[sys.executable,str(TOOL_ROOT/'tools/compile.py'),str(source),'-o',str(dest),'--vc6-root',os.environ['VC6_ROOT'],'--profile','vc6_o2_mt']
    if includes:args+=['--include',str(ROOT/'src/krusty2'),'--include',str(ROOT/'src/reconstructed')]
    run=subprocess.run(args,cwd=TOOL_ROOT,capture_output=True,text=True,env={**os.environ,'PYTHONPATH':str(TOOL_ROOT)})
    self.assertEqual(run.returncode,0,run.stdout+run.stderr);objects[label]=CoffObject(dest);paths[label]=dest;return objects[label]
   for source in (CALLER,SAMPLE,RACE,MODEL,STATE):compile(source,source.replace('/','_'),source not in (CALLER,SAMPLE))
   def obj(source):return objects[source.replace('/','_')]
   pins=json.loads((ROOT/'tests/fixtures/race_detail_objects.json').read_text())['objects']
   for source in (CALLER,SAMPLE):self.assertEqual(object_pin(obj(source)),pins[source])
   strict=0
   for c in CASES:
    if c['source']!=CALLER:continue
    self.assertTrue(compare(image,obj(CALLER),c['symbol'],int(c['target_va'],0),c['target_size'],ROOT/c['bindings'])['strict_exact'],c);strict+=1
   self.assertTrue(compare(image,obj(RACE),'??0BikeRace@@QAE@H@Z',0x417c30,632,(ROOT/RACE).with_suffix('.bindings.json'))['strict_exact']);strict+=1
   caller=obj(CALLER);names=[caller.symbol_by_index[r.symbol_index].name for r in caller.relocations]
   self.assertNotIn(OLD,names);self.assertEqual(names.count(NEW),2)
   # Complete extraction is byte/relocation identical before native execution.
   for label,source in (('race',RACE),('model',MODEL)):
    path=out/(label+'.cpp')
    prefix='#include "'+(ROOT/'src/reconstructed/BikeRace.h').as_posix()+'"\n#include "'+(ROOT/'src/reconstructed/D3DIMSoulTree.h').as_posix()+'"\n'
    path.write_text(prefix+'\n'.join(function_text((ROOT/source).read_text(),signature)for src,_,_,_,signature in PROVIDERS if src==source)+'\n')
    compile(path,label)
   for source,symbol,va,size,_ in PROVIDERS:
    label='race'if source==RACE else'model'
    self.assertEqual(function(objects[label],symbol),function(obj(source),symbol))
    for candidate in (obj(source),objects[label]):
     self.assertEqual(candidate.find_symbol(symbol).function_size,size)
     self.assertTrue(compare(image,candidate,symbol,va,size,(ROOT/source).with_suffix('.bindings.json'))['strict_exact'],symbol);strict+=1
   storage=obj(STATE)
   for name,va in (('?g_NonBladeLodSettings@@',0x574428),('?g_BladeLodSettings@@',0x5744c8)):
    s=storage.find_symbol(name);sec=storage.section(s.section_number)
    self.assertEqual(storage.data[sec.raw_ptr+s.value:sec.raw_ptr+s.value+160],image.bytes_at_va(va,160))
    self.assertTrue(sec.characteristics&0x80000000)
   active=storage.find_symbol('?g_ActiveLodSettings@@');sec=storage.section(active.section_number)
   self.assertTrue(sec.characteristics&0x80);self.assertTrue(sec.characteristics&0x80000000)
   compile('tests/fixtures/race_detail_compare.cpp','fixture')
   vc6=Path(os.environ['VC6_ROOT']);binary=out/'probe.exe'
   run=subprocess.run([str(find_child_ci(find_vc6_bin(vc6),'link.exe')),'/nologo','/subsystem:console','/out:'+str(binary),'/libpath:'+str(find_child_ci(infer_vc98_root(vc6),'lib')),*(str(paths[k])for k in ('race','model',STATE.replace('/','_'),'fixture')),'kernel32.lib'],cwd=TOOL_ROOT,capture_output=True,text=True)
   self.assertEqual(run.returncode,0,run.stdout+run.stderr)
   run=subprocess.run([str(binary)],capture_output=True,text=True,timeout=30);self.assertEqual(run.returncode,0,run.stdout+run.stderr)
   self.assertEqual(run.stdout.strip(),'Race detail: 4520 cases, 200 independently modeled write faults')
   self.assertEqual(strict,35)
   print('%d strict checks; two whole objects; %s'%(strict,run.stdout.strip()))

if __name__=='__main__':unittest.main()
