"""Primary Dust/Steam attachment chains and their actual store-only helpers."""
import hashlib,json,os,struct,subprocess,sys,tempfile,unittest
from pathlib import Path
from unittest.mock import patch
from mcm2tool.coff import CoffObject
from mcm2tool.pe import PEImage
from mcm2tool.rtti import parse_rtti
from mcm2tool.resolved_match import match_object
from mcm2tool.toolchain import find_child_ci,find_vc6_bin,infer_vc98_root
from tools.match import compare
from test_edit_box_on_char import extract
import test_eco_quadtree_connections as views
ROOT=Path(__file__).resolve().parents[1]
SOUL='src/krusty2/soultree/SoulTreePhysics.cpp'

@unittest.skipUnless(os.name=='nt' and os.environ.get('VC6_ROOT') and os.environ.get('MCM2_EXE'),'authentic VC6/private retail required')
class SoultreeAttachmentEmitters(unittest.TestCase):
 def test_primary_attachment_chains_and_actual_store_methods(self):
  import capstone
  image=Path(os.environ['MCM2_EXE']);pe=PEImage(image)
  self.assertEqual(hashlib.sha256(image.read_bytes()).hexdigest(),'31fde4cc686a5ee89ef9095b90235325b195596867ecacefe511263e1509b874')
  pins=json.loads((ROOT/'tests/fixtures/soultree_attachment_emitters_objects.json').read_text())
  with tempfile.TemporaryDirectory(prefix='mcm2-attachment-emitters-')as directory:
   out=Path(directory);compiled={};paths={};vc=Path(os.environ['VC6_ROOT'])
   def compile_source(source):
    if source in compiled:return compiled[source]
    path=Path(source);path=path if path.is_absolute() else ROOT/source
    obj=out/('%d.obj'%len(compiled))
    result=subprocess.run([sys.executable,str(ROOT/'tools/compile.py'),str(path),'-o',str(obj),'--vc6-root',str(vc),'--profile','vc6_o2_mt','--include',str(ROOT/'src/krusty2'),'--include',str(ROOT/'src/reconstructed')],capture_output=True,text=True,encoding='latin1',env={**os.environ,'PYTHONPATH':str(ROOT)})
    self.assertEqual(result.returncode,0,result.stdout+result.stderr)
    compiled[source]=CoffObject(obj);paths[source]=obj;return compiled[source]
   for source,pin in pins['objects'].items():
    obj=compile_source(source)
    with patch.object(views,'FILENAMES',tuple(pins['objects'])):rows=views.whole_object(obj,(ROOT,))
    self.assertEqual(dict(sections=len(rows),relocations=sum(len(r[-1])for r in rows),sha256=hashlib.sha256(json.dumps(rows,separators=(',',':')).encode()).hexdigest()),pin,source)
   strict=0
   for row in pins['strict']:
    self.assertTrue(compare(pe,compiled[row['source']],row['symbol'],int(row['va'],16),row['extent'],ROOT/row['source'].replace('.cpp','.bindings.json'))['strict_exact'],row);strict+=1
   self.assertEqual(strict,59)
   for source,symbol,va,extent,size,score,digest in pins['partials']:
    binding=ROOT/source.replace('.cpp','.bindings.json')
    result=compare(pe,compiled[source],symbol,va,extent,binding)
    self.assertEqual((result['candidate_size'],result['strict_match']['matching_positions']),(size,score))
    with patch('mcm2tool.resolved_match.compare_bytes',lambda a,b:{'digest':hashlib.sha256(b).hexdigest()}):
     resolved=match_object(compiled[source],symbol,va,b'',{k:int(v,0)for k,v in json.loads(binding.read_text()).items()})
    self.assertEqual(resolved['digest'],digest)
   obj=compiled[SOUL]
   for old,new in pins['renames'].items():
    self.assertFalse(any(s.name==old for s in obj.symbols))
    self.assertEqual(sum(r.type==20 and obj.symbol_by_index[r.symbol_index].name==new for r in obj.relocations),2 if 'SetColor' in new else 1)
   md=capstone.Cs(capstone.CS_ARCH_X86,capstone.CS_MODE_32)
   for window in pins['windows']:
    for va,mnemonic,operands in window['instructions']:
     i=next(md.disasm(pe.bytes_at_va(int(va,16),16),int(va,16)));self.assertEqual((i.mnemonic,i.op_str),(mnemonic,operands))
   for address,target in pins['call_sites']:
    i=next(md.disasm(pe.bytes_at_va(int(address,16),5),int(address,16)));self.assertEqual((i.mnemonic,i.op_str),('call',target))
   self.assertEqual(struct.unpack('<4I',pe.bytes_at_va(0x502b40,16)),(0x502ab6,0x502ad1,0x502aec,0x502b07))
   classes={c.name:c for c in parse_rtti(pe)}
   for row in pins['rtti']:
    table=int(row['vtable'],16);col=int(row['col'],16)
    self.assertEqual(struct.unpack('<I',pe.bytes_at_va(table-4,4))[0],col)
    words=struct.unpack('<5I',pe.bytes_at_va(col,20));self.assertEqual(words[1],0)
    self.assertEqual(pe.bytes_at_va(words[3]+8,80).split(b'\0')[0],('.?AV'+row['name']+'@@').encode())
    base='SoultreePhysicsBaseObject' if row['name']in ('Bike','SoultreePhysicsObject') else 'GameObject'
    if row['name']!='SoultreePhysicsBaseObject':self.assertTrue(any(b.name==base and b.mdisp==0 and b.pdisp==-1 for b in classes[row['name']].bases))
    if row['name']in ('Bike','SoultreePhysicsBaseObject','SoultreePhysicsObject'):self.assertEqual(struct.unpack('<I',pe.bytes_at_va(table+37*4,4))[0],0x502a40)
   layout=out/'layout.cpp';layout.write_text('#include "soultree/SoultreePhysicsBaseObject.h"\n#include "soultree/SoultreePhysicsContact.h"\n#include "ParticleEmitters.h"\n#include <stddef.h>\ntypedef char L[(sizeof(SoultreeAttachment)==40 && offsetof(SoultreeAttachment,contactEmitter)==4 && offsetof(SoultreeAttachment,pointEmitter)==0x14 && offsetof(SoultreeAttachment,localOffset)==0x18 && offsetof(SoultreeAttachment,resetTrail)==0x24 && sizeof(Vec3)==sizeof(ParticleVec3))?1:-1];\n')
   compile_source(str(layout))
   # Exact unchanged source extraction isolates the seven dependency-free methods.
   helper=out/'helpers.cpp';text='#include "ParticleEmitters.h"\n'
   for row in pins['providers']:
    method=row['symbol'].split('@')[0][1:];cls=Path(row['source']).stem
    text+=extract((ROOT/row['source']).read_text(),'void '+cls+'::'+method+'(')+'\n'
   helper.write_text(text);native=compile_source(str(helper))
   def body(o,name):
    s=o.find_symbol(name);raw,size,rels=o.symbol_extent(s)
    return raw,[(r.virtual_address-s.value,r.type,o.symbol_by_index[r.symbol_index].name)for r in rels]
   for row in pins['providers']:
    self.assertEqual(body(native,row['symbol']),body(compiled[row['source']],row['symbol']))
    self.assertFalse(body(native,row['symbol'])[1])
    self.assertTrue(compare(pe,native,row['symbol'],int(row['va'],16),row['extent'],ROOT/row['source'].replace('.cpp','.bindings.json'))['strict_exact']);strict+=1
   self.assertEqual([s.name for s in native.symbols if s.storage_class==2 and s.section_number==0],['__fltused'])
   fixture='tests/fixtures/soultree_attachment_emitters.cpp';compile_source(fixture)
   exe=out/'emitters.exe';link=find_child_ci(find_vc6_bin(vc),'link.exe');lib=find_child_ci(infer_vc98_root(vc),'lib')
   result=subprocess.run([str(link),'/nologo','/subsystem:console','/base:0x100000','/fixed','/opt:ref','/out:'+str(exe),'/libpath:'+str(lib),str(paths[str(helper)]),str(paths[fixture]),'kernel32.lib'],capture_output=True,text=True,encoding='latin1')
   self.assertEqual(result.returncode,0,result.stdout+result.stderr)
   result=subprocess.run([str(exe),str(image)],capture_output=True,text=True,timeout=60)
   self.assertEqual(result.returncode,0,result.stdout+result.stderr)
   self.assertEqual(strict,66)
   self.assertEqual(result.stdout.strip(),'Attachment emitters: 130816 cases, 30080 modeled access faults')
   print('%d strict checks; five complete objects; two partials; nine calls; %s'%(strict,result.stdout.strip()))
if __name__=='__main__':unittest.main()
