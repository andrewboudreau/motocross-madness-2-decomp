"""Genuine particle-emitter construction and primary Spark setter connections."""
import hashlib,json,os,struct,subprocess,sys,tempfile,unittest
from pathlib import Path
from unittest.mock import patch
from mcm2tool.coff import CoffObject
from mcm2tool.pe import PEImage
from mcm2tool.rtti import parse_rtti
from mcm2tool.toolchain import find_child_ci,find_vc6_bin,infer_vc98_root
from tools.match import compare
from test_edit_box_on_char import extract
import test_eco_quadtree_connections as views
ROOT=Path(__file__).resolve().parents[1]
@unittest.skipUnless(os.name=='nt' and os.environ.get('VC6_ROOT') and os.environ.get('MCM2_EXE'),'authentic VC6/private retail required')
class ParticleEmitterConnections(unittest.TestCase):
 def test_complete_classes_callers_and_actual_setter(self):
  pe=PEImage(os.environ['MCM2_EXE'])
  self.assertEqual(hashlib.sha256(Path(os.environ['MCM2_EXE']).read_bytes()).hexdigest(),'31fde4cc686a5ee89ef9095b90235325b195596867ecacefe511263e1509b874')
  pins=json.loads((ROOT/'tests/fixtures/particle_emitter_objects.json').read_text())
  targets=json.loads((ROOT/'samples/physics/effects/targets.json').read_text())
  with tempfile.TemporaryDirectory(prefix='mcm2-emitters-')as tmp:
   out=Path(tmp);compiled={};paths={};vc=Path(os.environ['VC6_ROOT'])
   def compile_source(source,name=None):
    if source in compiled:return compiled[source]
    path=Path(source);path=path if path.is_absolute() else ROOT/source
    obj=out/((name or str(len(compiled)))+'.obj')
    result=subprocess.run([sys.executable,str(ROOT/'tools/compile.py'),str(path),'-o',str(obj),'--vc6-root',str(vc),'--profile','vc6_o2_mt','--include',str(ROOT/'src/krusty2'),'--include',str(ROOT/'src/reconstructed')],capture_output=True,text=True,encoding='latin1',env={**os.environ,'PYTHONPATH':str(ROOT)})
    self.assertEqual(result.returncode,0,result.stdout+result.stderr)
    compiled[source]=CoffObject(obj);paths[source]=obj;return compiled[source]
   for source,pin in pins['objects'].items():
    o=compile_source(source)
    with patch.object(views,'FILENAMES',tuple(pins['objects'])):rows=views.whole_object(o,(ROOT,))
    self.assertEqual(dict(sections=len(rows),relocations=sum(len(x[-1])for x in rows),sha256=hashlib.sha256(json.dumps(rows,separators=(',',':')).encode()).hexdigest()),pin,source)
   strict=0
   for row in targets:
    source=str((ROOT/'samples/physics/effects'/row['source']).resolve().relative_to(ROOT)).replace('\\','/')
    if source not in pins['objects']:continue
    self.assertTrue(compare(pe,compiled[source],row['candidate_symbol_contains'],int(row['target_va'],16),row['target_size'],ROOT/'samples/physics/effects'/row['bindings'])['strict_exact'],row)
    strict+=1
   self.assertEqual(strict,33) # Four admitted classes plus unchanged DirtSpray header consumer.
   for row in pins['caller_strict']:
    self.assertTrue(compare(pe,compiled[row['source']],row['symbol'],int(row['target_va'],16),row['target_size'],ROOT/row['bindings'])['strict_exact'],row)
    strict+=1
   self.assertEqual(len(pins['caller_strict']),130)
   classes={c.name:c for c in parse_rtti(pe)}
   for row in pins['rtti']:
    table=int(row['vtable'],16);col=int(row['col'],16)
    self.assertEqual(struct.unpack('<I',pe.bytes_at_va(table-4,4))[0],col)
    words=struct.unpack('<5I',pe.bytes_at_va(col,20));self.assertEqual(words[1],0)
    self.assertEqual(pe.bytes_at_va(words[3]+8,80).split(b'\0')[0],('.?AV'+row['class']+'@@').encode())
    self.assertTrue(any(b.name=='GameObject' and b.mdisp==0 and b.pdisp==-1 for b in classes[row['class']].bases))
   # The existing inherited methods retain their complete independently registered extents.
   for table in pins['inherited_vtables']:
    rtti=next(x for x in pins['rtti']if x['class']==table['class']);va=int(rtti['vtable'],16)
    obj=compiled['src/reconstructed/'+table['class']+'.cpp'];vptr=obj.find_symbol('??_7'+table['class']+'@@6B@')
    rels={r.virtual_address-vptr.value:obj.symbol_by_index[r.symbol_index].name for r in obj.relocations if r.section_number==vptr.section_number and r.virtual_address>=vptr.value}
    self.assertEqual(len(rels),28)
    # VC6 emits a weak vector-deleting spelling referring to its scalar body.
    self.assertEqual(rels[0],'??_E'+table['class']+'@@UAEPAXI@Z')
    weak=next(x for x in obj.symbols if x.name==rels[0])
    self.assertEqual((weak.storage_class,weak.aux_count),(105,1))
    fallback,search=struct.unpack_from('<II',obj.data,obj.sym_ptr+(weak.index+1)*18)
    self.assertEqual(search,2)
    self.assertEqual(obj.symbol_by_index[fallback].name,'??_G'+table['class']+'@@UAEPAXI@Z')
    for slot,prefix in ((0,'??_G'),(10,'?Tick@'),(27,'?UnknownVirtualSlot27@')):
     name=prefix+table['class']
     target=next(x for x in targets if Path(x['source']).name==table['class']+'.cpp' and x['candidate_symbol_contains']==name)
     self.assertEqual(struct.unpack('<I',pe.bytes_at_va(va+slot*4,4))[0],int(target['target_va'],16))
     if slot:self.assertTrue(rels[slot*4].startswith(name+'@@'))
    for row in table['slots']:
     self.assertEqual(rels[row['slot']*4],row['symbol'])
     self.assertEqual(struct.unpack('<I',pe.bytes_at_va(va+row['slot']*4,4))[0],int(row['va'],16))
     provider=compile_source(row['source']);binding=ROOT/row['source'].replace('.cpp','.bindings.json')
     self.assertTrue(compare(pe,provider,row['symbol'],int(row['va'],16),row['extent'],binding)['strict_exact'],row)
     strict+=1
   for source,symbol,va,size in (
    ('src/reconstructed/GameObject.cpp','??0GameObject@@QAE@H@Z',0x468ca0,149),
    ('src/reconstructed/GameObject.cpp','??1GameObject@@UAE@XZ',0x468d60,97),
    ('src/reconstructed/FastMath.cpp','?FastSqrt@@YAMM@Z',0x460b50,85),
    ('src/krusty2/effects/Particles.cpp','?AddParticle@ParticleManager',0x4baa50,158)):
    provider=compile_source(source)
    self.assertTrue(compare(pe,provider,symbol,va,size,ROOT/source.replace('.cpp','.bindings.json'))['strict_exact'],symbol)
    strict+=1
   # Exact producer instructions include allocator sizes, ctor ECX, returned-field stores,
   # and primary slot27 dispatch. No whole LoadBike strict claim is made.
   import capstone
   md=capstone.Cs(capstone.CS_ARCH_X86,capstone.CS_MODE_32)
   for window in pins['producer_windows']+pins['setter_windows']+pins['constructor_evidence']:
    for address,mnemonic,operands in window['instructions']:
     instruction=next(md.disasm(pe.bytes_at_va(int(address,16),16),int(address,16)))
     self.assertEqual((instruction.mnemonic,instruction.op_str),(mnemonic,operands))
   calls={}
   for source in ('src/krusty2/vehicle/Bike.cpp','src/krusty2/vehicle/Vehicle.cpp'):
    obj=compiled[source]
    for old,new in pins['renames'].items():
     self.assertFalse(any(s.name==old for s in obj.symbols))
     calls[new]=calls.get(new,0)+sum(r.type==20 and obj.symbol_by_index[r.symbol_index].name==new for r in obj.relocations)
   self.assertEqual(sorted(calls.values()),[1,1,1,1,3])
   layout=out/'layout.cpp'
   layout.write_text('#include "vehicle/Bike.h"\n#include "ParticleEmitters.h"\n#include <stddef.h>\n'
    'typedef char L[(sizeof(BikeDustEmitter)==sizeof(DustParticleEmitter) && sizeof(DustParticleEmitter)==0x80 && '
    'sizeof(BikeDirtChunkEmitter)==sizeof(DirtChunkParticleEmitter) && sizeof(DirtChunkParticleEmitter)==0x608 && '
    'sizeof(BikeSteamEmitter)==sizeof(SteamParticleEmitter) && sizeof(SteamParticleEmitter)==0x47c && '
    'sizeof(VehicleSparkEmitter)==sizeof(SparkParticleEmitter) && sizeof(SparkParticleEmitter)==0x60c && '
    'offsetof(VehicleSparkEmitter,burstPending)==offsetof(SparkParticleEmitter,burstPending) && '
    'offsetof(VehicleSparkEmitter,launchVelocity)==offsetof(SparkParticleEmitter,launchVelocity) && sizeof(Vec3)==sizeof(ParticleVec3))?1:-1];\n')
   compile_source(str(layout),'layout')
   helper=out/'helper.cpp';helper.write_text('#include "ParticleEmitters.h"\n'+extract((ROOT/'src/reconstructed/SparkParticleEmitter.cpp').read_text(),'void SparkParticleEmitter::SetPosition(')+'\n')
   obj=compile_source(str(helper),'helper');symbol='?SetPosition@SparkParticleEmitter@@QAEXUParticleVec3@@@Z'
   def body(o):
    s=o.find_symbol(symbol);raw,size,rels=o.symbol_extent(s);return raw,[(r.virtual_address-s.value,r.type,o.symbol_by_index[r.symbol_index].name)for r in rels]
   self.assertEqual(body(obj),body(compiled['src/reconstructed/SparkParticleEmitter.cpp']))
   self.assertTrue(compare(pe,obj,symbol,0x4b9dc0,49,ROOT/'src/reconstructed/SparkParticleEmitter.bindings.json')['strict_exact']);strict+=1
   self.assertFalse(body(obj)[1]);self.assertFalse([s for s in obj.symbols if s.storage_class==2 and s.section_number==0])
   fixture='tests/fixtures/particle_emitter_position_compare.cpp';compile_source(fixture,'fixture')
   exe=out/'emitter.exe';link=find_child_ci(find_vc6_bin(vc),'link.exe');lib=find_child_ci(infer_vc98_root(vc),'lib')
   result=subprocess.run([str(link),'/nologo','/subsystem:console','/base:0x100000','/fixed','/opt:ref','/out:'+str(exe),'/libpath:'+str(lib),str(paths[str(helper)]),str(paths[fixture]),'kernel32.lib'],capture_output=True,text=True,encoding='latin1')
   self.assertEqual(result.returncode,0,result.stdout+result.stderr)
   result=subprocess.run([str(exe),os.environ['MCM2_EXE']],capture_output=True,text=True,timeout=30)
   self.assertEqual(result.returncode,0,result.stdout+result.stderr)
   self.assertEqual(result.stdout.strip(),'Emitter position: 3200 cases, 1024 modeled access faults')
   self.assertEqual(strict,268)
  print('268 strict comparisons; seven whole objects; seven joined calls; '+result.stdout.strip())
if __name__=='__main__':unittest.main()
