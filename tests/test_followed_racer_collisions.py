"""Primary followed-racer notifications and complete actual ignore-list closure."""
import hashlib,json,os,struct,subprocess,sys,tempfile,unittest
from pathlib import Path
from unittest.mock import patch
from mcm2tool.coff import CoffObject
from mcm2tool.pe import PEImage
from mcm2tool.rtti import parse_rtti
from mcm2tool.resolved_match import match_object
from mcm2tool.toolchain import find_child_ci,find_vc6_bin,infer_vc98_root
from tools.match import compare
from tools.run_calibration import CASES
from test_collision_point_consumers import definition
from test_eco_quadtree_connections import whole_object
ROOT=Path(__file__).resolve().parents[1]
SOURCES=('src/reconstructed/FollowRacer.cpp','samples/race/FollowRacerNearMisses.cpp','samples/race/FollowedRacerPicker.cpp')
GROUPS={
 'bike':('src/krusty2/vehicle/KrustyBike.cpp','#include "vehicle/KrustyBikeCore.h"\n#include "collision/CollisionObject.h"\n#include "Wrecker.h"\n',
         [('KrustyBike::Fn_00496F90','?Fn_00496F90@KrustyBike',0x496f90,981)]),
 'collision':('src/krusty2/collision/CollisionObject.cpp','#include "collision/CollisionObject.h"\n#include "core/DebugAlloc.h"\n',
              [('CollisionObject::AddIgnoredOwner','?AddIgnoredOwner@CollisionObject',0x439410,118)]),
 'allocator':('src/reconstructed/MemTag.cpp','#include <malloc.h>\n#include <stdlib.h>\n#include <string.h>\n#include "MemTag.h"\n#include "DebugAlloc.h"\nextern int g_UnclaimedBytes;\nvoid* DebugMemcpy(void*,const void*,unsigned int,const char*,int);\n',
              [('DebugRealloc','?DebugRealloc@@',0x4a2ec0,256),('DebugMemcpy','?DebugMemcpy@@',0x4a3120,35)]),
 'announce':(SOURCES[0],'#include "BikeRace.h"\n#include "TrackGame.h"\n#include "vehicle/KrustyBikeCore.h"\n',
             [('TrackGameViewOwner::AnnounceFollowedRacer','?AnnounceFollowedRacer@',0x4a9f90,113)]),
 'iterator':('src/reconstructed/BikeRace.cpp','#include "BikeRaceCore.h"\n#include "TrackGame.h"\n',
             [('BikeRace::NextBike','?NextBike@BikeRace',0x4204e0,162)])}

@unittest.skipUnless(os.name=='nt' and os.environ.get('VC6_ROOT') and os.environ.get('MCM2_EXE'),'requires native VC6 and private retail')
class FollowedRacerCollisionTests(unittest.TestCase):
 def test_primary_producers_complete_callers_and_actual_closure(self):
  import capstone
  image=Path(os.environ['MCM2_EXE']);self.assertEqual(hashlib.sha256(image.read_bytes()).hexdigest(),'31fde4cc686a5ee89ef9095b90235325b195596867ecacefe511263e1509b874')
  pe=PEImage(image);md=capstone.Cs(capstone.CS_ARCH_X86,capstone.CS_MODE_32)
  def decoded(va,size):return {i.address:(i.mnemonic,i.op_str)for i in md.disasm(pe.bytes_at_va(va,size),va)}
  word=lambda va:struct.unpack('<I',pe.bytes_at_va(va,4))[0]
  for table,col,offset in ((0x554548,0x55d078,0),(0x554514,0x55d008,0x21c),(0x5544a4,0x55cff0,0x160c)):
   self.assertEqual(word(table-4),col);self.assertEqual((word(col+4),word(col+12)),(offset,0x56d0a8))
   self.assertEqual(pe.bytes_at_va(word(col+12)+8,32).split(b'\0')[0],b'.?AVKrustyBike@@')
  classes={c.name:c for c in parse_rtti(pe)}
  for name,table,col in (('Tire',0x558578,0x55f5e0),('ConstraintMethodCollisionModel',0x551260,0x55b0f8),('CollisionObject',0x5511b8,0x55aff0)):
   self.assertEqual(word(table-4),col);self.assertEqual(word(col+4),0)
   self.assertEqual(pe.bytes_at_va(word(col+12)+8,80).split(b'\0')[0],('.?AV'+name+'@@').encode())
   if name!='CollisionObject':self.assertTrue(any(b.name=='CollisionObject' and b.mdisp==0 and b.pdisp==-1 for b in classes[name].bases))
  # Wheel array retains the Tire primary pointer; the separate contact-list
  # conversion adds +b8 only after the primary array store. Wrecker +38 is a
  # primary ConstraintMethodCollisionModel, not its GameObject+0c subobject.
  producers={}
  for va,size in ((0x4079c0,6745),(0x525d30,160),(0x512f10,1165),(0x5301c0,995),(0x43b8d0,121),(0x48fc80,5384)):producers.update(decoded(va,size))
  for address,value in {
   0x4084d7:('call','0x512f10'),0x4084dc:('mov','esi, eax'),0x408547:('push','esi'),0x408548:('call','0x525d30'),0x525d47:('mov','eax, dword ptr [esp + 8]'),0x525d70:('mov','edx, dword ptr [esi + 0x53c]'),0x525d78:('mov','dword ptr [edx + ecx*4], eax'),0x525d7d:('add','eax, 0xb8'),
   0x4086ff:('mov','eax, dword ptr [ebp + 0x53c]'),0x408705:('mov','eax, dword ptr [eax + ecx*4]'),0x408708:('mov','dword ptr [ebp + 0x5f0], eax'),0x40871d:('mov','eax, dword ptr [ebp + 0x53c]'),0x408725:('mov','eax, dword ptr [eax + ecx*4]'),0x408728:('mov','dword ptr [ebp + 0x5f4], eax'),
   0x513120:('mov','dword ptr [esi], 0x558578'),0x51312a:('mov','dword ptr [esi + 0xc], 0x558508'),0x513135:('mov','dword ptr [esi + 0xb8], 0x5584fc'),
   0x530304:('call','0x43b8d0'),0x530313:('mov','dword ptr [ebx + 0x38], eax'),0x43b8d5:('mov','esi, ecx'),0x43b8d8:('call','0x431e70'),0x43b920:('mov','dword ptr [esi], 0x551260'),0x43b926:('mov','dword ptr [esi + 0xc], 0x5511f0'),0x43b943:('mov','eax, esi'),
   0x490717:('call','0x431e70'),0x490766:('mov','dword ptr [ebp + 0x1414], eax'),0x4907de:('call','0x431e70'),0x4907f8:('mov','dword ptr [ebp + 0x1418], eax'),
  }.items():self.assertEqual(producers[address],value)
  self.assertEqual(word(0x554548+28*4),0x48da50);self.assertEqual(word(0x5544a4+10*4),0x497ca0)
  self.assertEqual(decoded(0x497ca0,8),{0x497ca0:('sub','ecx, dword ptr [ecx - 4]'),0x497ca3:('jmp','0x4977a0')})
  # Allocation -> original constructor ECX -> primary return -> each race slot.
  create=decoded(0x419970,13428);constructor=decoded(0x48fa60,532)
  self.assertEqual(constructor[0x48fa80],('mov','esi, ecx'));self.assertEqual(constructor[0x48fc5f],('mov','eax, esi'));self.assertEqual(constructor[0x48fc71],('ret','8'))
  sites=(0x419e09,0x41a275,0x41a78d,0x41acb8,0x41b1a8,0x41b496,0x41b862,0x41bfcd,0x41c7af)
  self.assertEqual(tuple(a for a,v in create.items()if v==('call','0x48fa60')),sites)
  instructions=list(create.items())
  for address in sites:
   n=next(n for n,r in enumerate(instructions)if r[0]==address);window=[v for _,v in instructions[n-22:n]]
   self.assertIn(('push','0x1638'),window);self.assertIn(('call','0x4a3010'),window);self.assertIn(('mov','ecx, eax'),window[-4:])
  for address,operands in {0x419e12:'dword ptr [ebp + 0x38], eax',0x41a27e:'edx, dword ptr [ebp + 0x40]',0x41a28e:'dword ptr [edx + ebx*4 - 4], eax',0x41a79b:'ecx, dword ptr [ebp + 0x3c]',0x41a7a3:'dword ptr [ecx + esi*4], eax',0x41acc1:'edx, dword ptr [ebp + 0x3c]',0x41acca:'dword ptr [edx + ebx*4], eax',0x41b1ad:'edi, eax',0x41b1b3:'ecx, dword ptr [ebp + 0x3c]',0x41b1bc:'dword ptr [ecx + ebx*4], edi',0x41b4b3:'dword ptr [ebp + 0x38], eax',0x41b86b:'edx, dword ptr [ebp + 0x40]',0x41b873:'dword ptr [edx], eax',0x41bfd6:'ecx, dword ptr [ebp + 0x40]',0x41bfe1:'dword ptr [ecx + ebx*4], eax',0x41c7b4:'edi, eax',0x41c7ba:'ecx, dword ptr [ebp + 0x3c]',0x41c7c3:'dword ptr [ecx + ebx*4], edi'}.items():self.assertEqual(create[address],('mov',operands))
  raw={}
  for va,size in ((0x48da50,416),(0x4977a0,1153),(0x4a9d20,340),(0x4a9e80,272),(0x4a9f90,113),(0x496f90,981)) :raw.update(decoded(va,size))
  # All direct switch calls pass a real KrustyBike primary (including the
  # explicit virtual-GameObject-to-primary subtraction in Tick), never +21c.
  expected={0x48da51:('mov','esi, ecx'),0x48db0c:('push','1'),0x48db0e:('push','0'),0x48db10:('push','esi'),0x48db13:('call','0x4a9e80'),0x4977ae:('mov','esi, ecx'),0x49797b:('lea','edx, [esi - 0x160c]'),0x497983:('push','edx'),0x497984:('push','0'),0x497986:('call','0x4a9e80'),
   0x4a9e81:('mov','ebp, dword ptr [esp + 0xc]'),0x4a9efb:('mov','esi, dword ptr [esp + 0x10]'),0x4a9f3d:('push','ebp'),0x4a9f3e:('push','esi'),0x4a9f41:('call','0x4a9f90'),0x4a9f84:('mov','dword ptr [edi + 0xa8], esi'),
   0x4a9d54:('call','0x4204e0'),0x4a9d65:('mov','ebx, eax'),0x4a9d70:('call','0x4204e0'),0x4a9d80:('mov','ecx, dword ptr [eax + 0x38]'),0x4a9d83:('mov','dword ptr [edi + 0xa8], ecx'),0x4a9d90:('mov','dword ptr [edi + 0xa8], ebx'),0x4a9dc1:('mov','esi, dword ptr [esi + 0x3c]'),0x4a9deb:('mov','eax, dword ptr [esi + eax*4]'),0x4a9dee:('mov','dword ptr [edi + 0xa8], eax'),
   0x4a9faa:('mov','esi, eax'),0x4a9fc2:('push','esi'),0x4a9fc3:('mov','ecx, edi'),0x4a9fc5:('call','0x496f90'),0x4a9fd2:('push','ebx'),0x4a9fd3:('mov','ecx, esi'),0x4a9fd5:('call','0x496f90'),0x4a9fde:('push','edi'),0x4a9fdf:('mov','ecx, esi'),0x4a9fe1:('call','0x496f90'),0x4a9fee:('call','0x4204e0'),0x4a9ff3:('mov','esi, eax'),0x4a9ffe:('ret','8')}
  for address,value in expected.items():self.assertEqual(raw[address],value)
  self.assertEqual(sum(v==('call','0x439410')for a,v in raw.items()if 0x496f90<=a<0x497365),50)
  pins=json.loads((ROOT/'tests/fixtures/followed_racer_collisions_objects.json').read_text());strict=0
  with tempfile.TemporaryDirectory(prefix='mcm2-follow-collisions-')as temp:
   out=Path(temp);objects={};links=[]
   def compile(source,label):
    path=out/(label+'.obj');r=subprocess.run([sys.executable,str(Path.cwd()/'tools/compile.py'),str(source),'-o',str(path),'--vc6-root',os.environ['VC6_ROOT'],'--profile','vc6_o2_mt','--include',str(ROOT/'src/krusty2'),'--include',str(ROOT/'src/reconstructed')],capture_output=True,text=True,encoding='latin1',env={**os.environ,'PYTHONPATH':str(Path.cwd())})
    self.assertEqual(r.returncode,0,r.stdout+r.stderr);return path,CoffObject(path)
   for i,source in enumerate(SOURCES):
    _,obj=compile(ROOT/source,'caller'+str(i));objects[source]=obj;rows=whole_object(obj,())
    self.assertEqual(dict(sections=len(rows),relocations=sum(len(r[-1])for r in rows),sha256=hashlib.sha256(json.dumps(rows).encode()).hexdigest()),pins['objects'][source])
   for source in SOURCES[:2]:
    for row in CASES:
     if row['source']!=SOURCES[0]:continue
     r=compare(pe,objects[source],row['symbol'],int(row['target_va'],0),row['target_size'],(ROOT/source).with_suffix('.bindings.json'));self.assertTrue(r['strict_exact'],r);strict+=1
   for row in pins['partials']:
    obj=objects[SOURCES[1]];r=compare(pe,obj,row['symbol'],row['va'],row['retail_size'],(ROOT/SOURCES[1]).with_suffix('.bindings.json'))
    body,size,_=obj.symbol_extent(obj.find_symbol(row['symbol']));self.assertEqual((size,r['strict_match']['matching_positions'],hashlib.sha256(body).hexdigest()),(row['size'],row['score'],row['raw_sha256']))
    bindings={k:int(v,0)for k,v in json.loads((ROOT/SOURCES[1]).with_suffix('.bindings.json').read_text()).items()}
    with patch('mcm2tool.resolved_match.compare_bytes',side_effect=lambda expected,actual:{'digest':hashlib.sha256(actual).hexdigest()}):
     resolved=match_object(obj,row['symbol'],row['va'],b'',bindings)
    self.assertEqual(resolved['digest'],row['resolved_sha256'])
   for name,(source,header,bodies)in GROUPS.items():
    if source not in objects:_,objects[source]=compile(ROOT/source,'full-'+name)
    extracted=out/Path(source).name;extracted.write_text(header+'\n'.join(definition((ROOT/source).read_text(),fn)for fn,*_ in bodies)+'\n')
    path,obj=compile(extracted,'extract-'+name);links.append(path)
    for _,symbol,va,size in bodies:
     for checked in (objects[source],obj):
      entry=checked.find_symbol(symbol);self.assertEqual((entry.function_size,entry.extent_source),(size,'coff_function_aux'))
      r=compare(pe,checked,symbol,va,size,(ROOT/source).with_suffix('.bindings.json'));self.assertTrue(r['strict_exact'],r);strict+=1
   for symbol,va,size in [('?UnknownVirtualSlot28@KrustyBike',0x48da50,416),('?Tick@KrustyBike@@UAEHM@Z',0x4977a0,1153)]:
    source=GROUPS['bike'][0];r=compare(pe,objects[source],symbol,va,size,(ROOT/source).with_suffix('.bindings.json'));self.assertTrue(r['strict_exact'],r);strict+=1
   joined=next(iter(pins['renames'].values()))
   for source in SOURCES[:2]:
    obj=objects[source];self.assertEqual(sum(r.type==20 and obj.symbol_by_index[r.symbol_index].name==joined for r in obj.relocations),3)
    self.assertFalse(any(s.name in pins['renames']for s in obj.symbols))
   fixture,_=compile(ROOT/'tests/fixtures/followed_racer_collisions.cpp','fixture');compiler=Path(os.environ['VC6_ROOT']);exe=out/'native.exe'
   r=subprocess.run([str(find_child_ci(find_vc6_bin(compiler),'link.exe')),'/nologo','/subsystem:console','/opt:ref','/out:'+str(exe),'/libpath:'+str(find_child_ci(infer_vc98_root(compiler),'lib')),*map(str,links),str(fixture),'kernel32.lib'],capture_output=True,text=True,encoding='latin1');self.assertEqual(r.returncode,0,r.stdout+r.stderr)
   r=subprocess.run([str(exe)],capture_output=True,text=True,timeout=60);self.assertEqual(r.returncode,0,r.stdout+r.stderr);self.assertEqual(r.stdout.strip(),'20432 actual notification cases, 10960 independently modeled AVs')
   self.assertEqual(strict,20);print('Followed-racer collisions:',strict,'strict; three whole objects; two unchanged partials;',r.stdout.strip())
if __name__=='__main__':unittest.main()
