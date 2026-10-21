"""Seven existing D3DIM primary virtual providers through the physics view."""
import hashlib,json,os,struct,subprocess,sys,tempfile,unittest
from pathlib import Path
from unittest.mock import patch
from mcm2tool.coff import CoffObject
from mcm2tool.pe import PEImage
from mcm2tool.rtti import parse_rtti
from mcm2tool.resolved_match import match_object
from mcm2tool.toolchain import find_child_ci,find_vc6_bin,infer_vc98_root
from tools.match import compare
from tools.build_game import ROOT as TOOL_ROOT
import test_eco_quadtree_connections as views
ROOT=Path(__file__).resolve().parents[1]
SOUL='src/krusty2/soultree/SoulTreePhysics.cpp'
PROVIDERS=(0x43f950,0x43fe40,0x4440a0,0x444140,0x4450c0,0x444560,0x444b60)

@unittest.skipUnless(os.name=='nt' and os.environ.get('VC6_ROOT') and os.environ.get('MCM2_EXE'),'requires authentic VC6 and private retail')
class D3dimVtableConnections(unittest.TestCase):
 def test_complete_providers_tables_offsets_and_dispatch(self):
  import capstone
  image=Path(os.environ['MCM2_EXE']);self.assertEqual(hashlib.sha256(image.read_bytes()).hexdigest(),'31fde4cc686a5ee89ef9095b90235325b195596867ecacefe511263e1509b874')
  pe=PEImage(image);word=lambda va:struct.unpack('<I',pe.bytes_at_va(va,4))[0]
  # Raw COLs distinguish the D3DIM primary from both kinds of GameObject.
  for name,table,col,offset in (('D3DIMSoultreeObject',0x5513ec,0x55b2b8,0),('D3DIMSoultreeObject',0x55137c,0x55b248,12),('SoultreePhysicsObject',0x557f54,0x55f168,0),('SoultreePhysicsObject',0x557f28,0x55f118,0x21c),('SoultreePhysicsObject',0x557eb8,0x55f100,0x228),('SoultreePhysicsObject',0x557e48,0x55f0e8,0x4f8)):
   self.assertEqual(word(table-4),col);self.assertEqual(word(col+4),offset)
   self.assertEqual(pe.bytes_at_va(word(col+12)+8,80).split(b'\0')[0],('.?AV'+name+'@@').encode())
  for table in (0x5513ec,0x557f28):self.assertEqual(struct.unpack('<7I',pe.bytes_at_va(table+8,28)),PROVIDERS)
  classes={c.name:c for c in parse_rtti(pe)}
  for cls,base,offset in (('D3DIMSoultreeObject','SoultreeObject',0),('D3DIMSoultreeObject','GameObject',12),('SoultreePhysicsObject','D3DIMSoultreeObject',0x21c),('SoultreePhysicsObject','GameObject',0x228)):
   self.assertTrue(any(b.name==base and b.mdisp==offset and b.pdisp==-1 for b in classes[cls].bases))
  md=capstone.Cs(capstone.CS_ARCH_X86,capstone.CS_MODE_32)
  instructions={i.address:(i.mnemonic,i.op_str)for va,size in ((0x43f160,275),(0x5037c0,194))for i in md.disasm(pe.bytes_at_va(va,size),va)}
  expected={0x43f169:('mov','esi, ecx'),0x43f16c:('call','0x4fb2b0'),0x43f17b:('mov','dword ptr [esi], 0x5513ec'),0x43f183:('mov','dword ptr [esi + 0xc], 0x55137c'),0x43f269:('mov','eax, esi'),0x43f270:('ret','4'),0x5037e0:('mov','esi, ecx'),0x5037f5:('lea','ecx, [esi + 0x4f8]'),0x503817:('call','0x500aa0'),0x503820:('lea','edi, [esi + 0x21c]'),0x503827:('mov','ecx, edi'),0x503829:('call','0x43f160'),0x503831:('mov','dword ptr [esi], 0x557f54'),0x503837:('mov','dword ptr [edi], 0x557f28'),0x50383d:('mov','dword ptr [esi + 0x228], 0x557eb8'),0x50384a:('mov','dword ptr [ecx + esi + 4], 0x557e48'),0x50386e:('mov','dword ptr [esi + 8], eax'),0x503871:('mov','eax, esi'),0x50387f:('ret','8')}
  for va,row in expected.items():self.assertEqual(instructions[va],row,hex(va))
  pins=json.loads((ROOT/'tests/fixtures/d3dim_vtable_objects.json').read_text())
  with tempfile.TemporaryDirectory(prefix='mcm2-d3dim-vtable-')as directory:
   out=Path(directory);compiled={};paths={};vc=Path(os.environ['VC6_ROOT'])
   def compile_source(source,key=None,flag=None):
    key=key or source
    if key in compiled:return compiled[key]
    obj=out/('%d.obj'%len(compiled));cmd=[sys.executable,str(TOOL_ROOT/'tools/compile.py'),str(ROOT/source),'-o',str(obj),'--vc6-root',str(vc),'--profile','vc6_o2_mt','--include',str(ROOT/'src/krusty2'),'--include',str(ROOT/'src/reconstructed')]
    if flag:cmd+=['--extra',flag]
    r=subprocess.run(cmd,capture_output=True,text=True,encoding='latin1',env={**os.environ,'PYTHONPATH':str(TOOL_ROOT)})
    self.assertEqual(r.returncode,0,r.stdout+r.stderr);compiled[key]=CoffObject(obj);paths[key]=obj;return compiled[key]
   for source,pin in pins['objects'].items():
    obj=compile_source(source)
    with patch.object(views,'FILENAMES',tuple(pins['filenames'])):rows=views.whole_object(obj,(ROOT,))
    actual=dict(sections=len(rows),relocations=sum(len(r[-1])for r in rows),sha256=hashlib.sha256(json.dumps(rows,separators=(',',':')).encode()).hexdigest())
    self.assertEqual(actual,pin,source)
   strict=0
   for row in pins['strict']:
    obj=compile_source(row['source']);r=compare(pe,obj,row['symbol'],int(row['target_va'],16),row['target_size'],ROOT/row['bindings'])
    self.assertTrue(r['strict_exact'],(row,r));strict+=1
    if int(row['target_va'],16)in PROVIDERS:
     slot=PROVIDERS.index(int(row['target_va'],16));returns=[i.op_str for i in md.disasm(pe.bytes_at_va(int(row['target_va'],16),row['target_size']),int(row['target_va'],16))if i.mnemonic=='ret']
     self.assertEqual(returns,[['4'],[''],['4'],[''],[''],['4','4'],['8']][slot])
   self.assertEqual(strict,59)
   for source,symbol,va,extent,size,score,digest in pins['partials']:
    binding=ROOT/source.replace('.cpp','.bindings.json');obj=compiled[source];r=compare(pe,obj,symbol,va,extent,binding)
    self.assertEqual((r['candidate_size'],r['strict_match']['matching_positions']),(size,score))
    with patch('mcm2tool.resolved_match.compare_bytes',lambda a,b:{'digest':hashlib.sha256(b).hexdigest()}):resolved=match_object(obj,symbol,va,b'',{k:int(v,0)for k,v in json.loads(binding.read_text()).items()})
    self.assertEqual(resolved['digest'],digest)
   obj=compiled[SOUL];table=obj.find_symbol('??_7SoultreePhysicsObject@@6BQuadTreeObject@@@')
   for slot,(old,new)in enumerate(pins['renames'].items(),2):
    self.assertFalse(any(s.name==old for s in obj.symbols))
    rel=[r for r in obj.relocations if obj.symbol_by_index[r.symbol_index].name==new]
    self.assertEqual([(r.section_number,r.virtual_address-table.value,r.type)for r in rel],[(table.section_number,slot*4,6)])
   # Fixture-only nonvirtual observers test both real declaration views. They
   # do not stand in for provider behavior or execute game hierarchy loading.
   fixture='tests/fixtures/d3dim_physics_dispatch.cpp'
   for key,flag in (('physics','/DPHYSICS_VIEW'),('render','/DRENDER_VIEW'),('observer',None)):compile_source(fixture,key,flag)
   for key,name in (('physics','_PhysicsDispatch'),('render','_RenderDispatch')):
    raw,_,_=compiled[key].symbol_extent(compiled[key].find_symbol(name))
    ins=list(md.disasm(raw,0));indirect=[i.op_str for i in ins if i.mnemonic in ('call','jmp')and '['in i.op_str and ('eax'in i.op_str or 'edx'in i.op_str)]
    for offset in (8,12,16,20,24,28,32):self.assertTrue(any(('+ '+hex(offset)+']')in s or ('+ '+str(offset)+']')in s for s in indirect),(key,offset,indirect))
   exe=out/'dispatch.exe';r=subprocess.run([str(find_child_ci(find_vc6_bin(vc),'link.exe')),'/nologo','/subsystem:console','/opt:ref','/out:'+str(exe),'/libpath:'+str(find_child_ci(infer_vc98_root(vc),'lib')),*(str(paths[k])for k in ('physics','render','observer')),'kernel32.lib'],capture_output=True,text=True,encoding='latin1')
   self.assertEqual(r.returncode,0,r.stdout+r.stderr)
   r=subprocess.run([str(exe)],capture_output=True,text=True,timeout=30);self.assertEqual(r.returncode,0,r.stdout+r.stderr)
   self.assertEqual(r.stdout.strip(),'896 cross-view dispatch cases preserve primary receiver and argument bits')
   print('59 strict checks; nine complete objects; two unchanged partials; seven table entries; '+r.stdout.strip())
if __name__=='__main__':unittest.main()
