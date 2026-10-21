"""Two existing by-value position views retain callers and share actual cache helpers."""
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
from mcm2tool.rtti import parse_rtti
from mcm2tool.toolchain import find_child_ci, find_vc6_bin, infer_vc98_root
from tools.match import compare
from test_bike_text_connections import extract
from test_character_motion_start import complete

ROOT = Path(__file__).resolve().parents[1]
SOURCE = 'src/reconstructed/SoultreePositionValue.cpp'
RENDER_SOURCE = 'src/reconstructed/D3DIMPositionValue.cpp'
LIGHT = 'src/reconstructed/LightEmitter.cpp'
SCENE = 'src/reconstructed/SceneManager.cpp'
MOTION = 'src/krusty2/motion/D3DIMSoultreeMotnctrl.cpp'
TREE = 'src/krusty2/soultree/soultree.cpp'
RENDER = 'src/reconstructed/D3DIMSoulTree.cpp'
SAMPLES = ('samples/track/SceneManagerNearMisses.cpp', 'samples/race/BikeRaceNearMisses.cpp')
VIEW = '?SetPositionVec3@SoultreeObject@@QAEXUVec3@@@Z'
OLD = '?SetPositionVec3@D3DIMSoultreeObject@@QAEXUVector3@@@Z'
FIXTURE = 'tests/fixtures/soultree_position_compare.cpp'
HELPERS = (('InvalidateWorldMatrix', 0x4fb850, 45), ('InvalidateSiblingChain', 0x4fb880, 50),
           ('MarkSubtreeDirty', 0x4fdab0, 38))


def fingerprint(obj, root, renames=None):
    """Complete sections and destinations; only recognized __FILE__ roots vary."""
    renames = renames or {}
    prefix = str(root).replace('\\', '/') + '/'
    filenames = (LIGHT, MOTION, SCENE, *SAMPLES, 'src/reconstructed/ContainerList.h',
                 'samples/race/../../src/reconstructed/BikeRace.h',
                 'samples/race/../../src/reconstructed/BikeRace.cpp')
    def filename(data):
        value, separator, tail = data.partition(b'\0')
        name = value.decode('latin1').replace('\\', '/')
        if separator and name.lower().startswith(prefix.lower()) and name[len(prefix):] in filenames:
            return ('source:' + name[len(prefix):]).encode() + separator + tail
        return data
    def name(symbol):
        if symbol.name.startswith('??_C@') and symbol.section_number > 0:
            section = obj.sections[symbol.section_number - 1]
            data = obj.data[section.raw_ptr + symbol.value:section.raw_ptr + section.raw_size]
            normalized = filename(data)
            if normalized != data:
                return normalized.split(b'\0')[0].decode()
        return symbol.name
    sections = [s for s in obj.sections if not s.name.startswith('.debug')]
    keys = {s.index: tuple(sorted(name(t) for t in obj.symbols
              if t.section_number == s.index and t.storage_class == 2))
              or ('section', n, s.name) for n, s in enumerate(sections)}
    def target(symbol):
        if symbol.section_number > 0:
            return ('defined', keys[symbol.section_number], symbol.value)
        return ('external', renames.get(symbol.name, symbol.name), symbol.value, symbol.section_number)
    rows = []
    for section in sections:
        data = (obj.data[section.raw_ptr:section.raw_ptr + section.raw_size]
                if section.raw_ptr else bytes(section.raw_size))
        relocs = [(r.virtual_address, r.type, target(obj.symbol_by_index[r.symbol_index]))
                  for r in obj.relocations if r.section_number == section.index]
        rows.append((keys[section.index], section.name, section.characteristics, filename(data).hex(), relocs))
    # VC6 can order public RTTI COMDATs differently for a longer checkout path.
    # Their names, complete payloads and all destination identities remain exact;
    # anonymous sections retain their original ordinal and kind in their key.
    rows.sort(key=lambda row: repr(row[0]))
    return {'sections': len(rows), 'relocations': sum(len(row[4]) for row in rows),
            'sha256': hashlib.sha256(json.dumps(rows).encode()).hexdigest()}


@unittest.skipUnless(os.name == 'nt' and os.environ.get('VC6_ROOT') and os.environ.get('MCM2_EXE'),
                     'requires authentic VC6 and the private retail image')
class SoultreePositionValueTests(unittest.TestCase):
    def test_full_provider_callers_and_native_cache_hierarchy(self):
        import capstone
        from tools.run_calibration import CASES
        image = Path(os.environ['MCM2_EXE']).resolve()
        self.assertEqual(hashlib.sha256(image.read_bytes()).hexdigest(),
                         '31fde4cc686a5ee89ef9095b90235325b195596867ecacefe511263e1509b874')
        retail = PEImage(image)
        word = lambda va: struct.unpack('<I', retail.bytes_at_va(va, 4))[0]
        for table, col, offset in ((0x5513ec, 0x55b2b8, 0), (0x55137c, 0x55b248, 12)):
            self.assertEqual(word(table-4), col)
            self.assertEqual(word(col+4), offset)
            self.assertEqual(retail.bytes_at_va(word(col+12)+8, 40).split(b'\0')[0],
                             b'.?AVD3DIMSoultreeObject@@')
        model = next(c for c in parse_rtti(retail) if c.name == 'D3DIMSoultreeObject')
        self.assertEqual([(b.name,b.mdisp,b.pdisp,b.vdisp) for b in model.bases],
            [('D3DIMSoultreeObject',0,-1,0),('SoultreeObject',0,-1,0),('QuadTreeObject',0,-1,0),
             ('GameObject',12,-1,0),('BaseObject',12,-1,0)])
        decoder = capstone.Cs(capstone.CS_ARCH_X86, capstone.CS_MODE_32)
        def decoded(va, size):
            ins = list(decoder.disasm(retail.bytes_at_va(va,size),va))
            # Create's complete 348-byte extent includes its jump-table data;
            # complete strict comparison below still checks that entire extent.
            if va != 0x49e230:
                self.assertEqual(sum(i.size for i in ins), size)
            return {i.address:(i.mnemonic,i.op_str) for i in ins}
        instructions = {}
        for va,size in ((0x43f160,275),(0x49e230,348),(0x49e150,141),(0x445460,321),(0x446210,235)):
            instructions.update(decoded(va,size))
        for va, expected in {
            0x43f17b:('mov','dword ptr [esi], 0x5513ec'),
            0x43f183:('mov','dword ptr [esi + 0xc], 0x55137c'),0x43f269:('mov','eax, esi'),
            0x49e27a:('push','0x2d8'),0x49e29b:('call','0x43f160'),
            0x49e2af:('mov','dword ptr [esi + 0x98], eax'),
            0x49e17f:('mov','ecx, dword ptr [esi + 0x98]'),0x49e1a0:('call','0x4fc630'),
            0x445467:('mov','ebx, dword ptr [esp + 0x18]'),
            0x445470:('shl','esi, 6'),0x445479:('mov','dword ptr [ebp + 0x34], ebx'),
            0x44622f:('mov','eax, dword ptr [edi + 0x188]'),0x44623c:('shl','edx, 6'),
            0x446243:('mov','ebx, dword ptr [eax + 0x34]'),0x446291:('call','0x4fc630'),
        }.items():
            self.assertEqual(instructions[va], expected)
        setter = decoded(0x4fc630,38)
        self.assertEqual(list(setter.values()), [
            ('mov','eax, dword ptr [esp + 4]'),('mov','edx, dword ptr [esp + 8]'),
            ('mov','dword ptr [ecx + 0xe8], eax'),('mov','eax, dword ptr [esp + 0xc]'),
            ('mov','dword ptr [ecx + 0xec], edx'),('mov','dword ptr [ecx + 0xf0], eax'),
            ('call','0x4fb850'),('ret','0xc')])
        baseline = json.loads((ROOT/'tests/fixtures/soultree_position_objects.json').read_text())['objects']
        with tempfile.TemporaryDirectory(prefix='mcm2-node-position-') as directory:
            output = Path(directory)
            helper = output/'helpers.cpp'
            helper.write_text('#include "math/Math3D.h"\n#include "core/SoultreeObject.h"\n' +
                '\n'.join(extract(ROOT/TREE,'void SoultreeObject::'+name+'(') for name,_,_ in HELPERS))
            layout = output/'render-layout.cpp'
            layout.write_text('#include <stddef.h>\n#include "D3DIMSoulTree.h"\n#include "../krusty2/math/Vec3.h"\n'
                'typedef char ModelSize[(sizeof(D3DIMSoultreeObject)==0x2d8)?1:-1];\n'
                'typedef char Translation[(offsetof(SoultreeObject,localMatrix)==0xb8)?1:-1];\n'
                'typedef char Vectors[(sizeof(Vec3)==12 && sizeof(Vector3)==12 && '
                'offsetof(Vec3,x)==0 && offsetof(Vec3,y)==4 && offsetof(Vec3,z)==8 && '
                'offsetof(Vector3,x)==0 && offsetof(Vector3,y)==4 && offsetof(Vector3,z)==8)?1:-1];\n'
                'extern "C" void RenderingPositionBoundary(void* node,const void* position) {\n'
                ' reinterpret_cast<D3DIMSoultreeObject*>(node)->SetPositionVec3(*reinterpret_cast<const Vector3*>(position));\n}\n')
            jobs = [(s,ROOT/s) for s in (SOURCE,RENDER_SOURCE,LIGHT,SCENE,MOTION,TREE,RENDER,*SAMPLES,FIXTURE)]
            jobs += [('helpers',helper),('layout',layout)]
            paths,objects = {},{}
            for index,(key,source) in enumerate(jobs):
                obj = output/('unit%d.obj'%index)
                result = subprocess.run([sys.executable,str(ROOT/'tools/compile.py'),str(source),'-o',str(obj),
                    '--vc6-root',os.environ['VC6_ROOT'],'--profile','vc6_o2_mt',
                    '--include',str(ROOT/'src/krusty2'),'--include',str(ROOT/'src/reconstructed')],
                    cwd=ROOT,capture_output=True,text=True,encoding='latin1',env={**os.environ,'PYTHONPATH':str(ROOT)})
                self.assertEqual(result.returncode,0,result.stdout+result.stderr)
                paths[key],objects[key] = obj,CoffObject(obj)
            for source in (LIGHT,MOTION,*SAMPLES,SCENE):
                self.assertEqual(fingerprint(objects[source],ROOT),baseline[source],source)
            for source,symbol in ((SOURCE,VIEW),(RENDER_SOURCE,OLD)):
                self.assertEqual({s.name for s in objects[source].symbols if s.storage_class==2 and s.section_number==0},
                                 {'__fltused','?InvalidateWorldMatrix@SoultreeObject@@QAEXXZ'})
                self.assertEqual({s.name for s in objects[source].symbols if s.storage_class==2 and
                                  s.section_number>0 and s.type==32},{symbol})
            self.assertEqual({s.name for s in objects['layout'].symbols if s.storage_class==2 and s.section_number==0},
                             {OLD})
            strict = 0
            # Math3D.h contributes private per-TU vectors and their ordinary CRT
            # initializers. They are additional storage, not recovered retail-TU
            # ownership. Pin the full bodies, actual local destinations and CRT
            # chain instead of counting only the public setter symbol.
            provider = objects[SOURCE]
            initializers = (
                ('_$E2', '_$E1', '_kVec3Zero', 0),
                ('_$E5', '_$E4', '_kVec3XAxis', 32),
                ('_$E8', '_$E7', '_kVec3YAxis', 48),
                ('_$E11', '_$E10', '_kVec3ZAxis', 16),
            )
            names = {name for thunk,body,_,_ in initializers for name in (thunk,body)}
            self.assertEqual({s.name for s in provider.symbols if s.type==32 and s.section_number>0},
                             names|{VIEW})
            sections = [s for s in provider.sections if not s.name.startswith('.debug')]
            self.assertEqual(len(sections),12)
            self.assertEqual({s.name for s in sections},{'.drectve','.text','.bss','.CRT$XCU'})
            bss = next(s for s in sections if s.name=='.bss')
            self.assertEqual((bss.raw_size,bss.raw_ptr,bss.characteristics),(60,0,0xc0400080))
            self.assertEqual({(s.name,s.value,s.storage_class) for s in provider.symbols
                              if s.section_number==bss.index and s.name!='.bss'},
                             {(name,offset,3) for _,_,name,offset in initializers})
            crt = next(s for s in sections if s.name=='.CRT$XCU')
            self.assertEqual((crt.raw_size,crt.characteristics),(16,0xc0300040))
            self.assertEqual(provider.data[crt.raw_ptr:crt.raw_ptr+16],bytes(16))
            self.assertEqual([(r.virtual_address,r.type,provider.symbol_by_index[r.symbol_index].name)
                              for r in provider.relocations if r.section_number==crt.index],
                             [(4*i,6,row[0]) for i,row in enumerate(initializers)])
            tree_targets = json.loads((ROOT/'src/krusty2/soultree/targets.json').read_text())
            for thunk,body,vector,_ in initializers:
                for name,size in ((thunk,5),(body,60)):
                    self.assertEqual(complete(provider,name),complete(objects[TREE],name))
                    self.assertEqual(complete(provider,name)[1],size)
                    target = next(t for t in tree_targets if t.get('source')=='soultree.cpp' and
                                  t.get('candidate_symbol_contains')==name)
                    proof = compare(retail,objects[TREE],name,int(target['target_va'],0),size,
                                    (ROOT/TREE).with_suffix('.bindings.json'))
                    self.assertTrue(proof['strict_exact'],proof)
                    strict += 1
                entry = provider.find_symbol(body)
                data,_,relocs = provider.symbol_extent(entry)
                self.assertEqual([(r.virtual_address-entry.value,r.type,
                                   provider.symbol_by_index[r.symbol_index].name,
                                   struct.unpack_from('<I',data,r.virtual_address-entry.value)[0]) for r in relocs],
                                 [(40,6,vector,0),(46,6,vector,4),(52,6,vector,8)])
            rendering = objects[RENDER_SOURCE]
            self.assertEqual([(s.name,s.raw_size) for s in rendering.sections if not s.name.startswith('.debug')],
                             [('.drectve',40),('.text',48)])
            self.assertEqual({s.name for s in rendering.symbols if s.type==32 and s.section_number>0},{OLD})
            for case in (c for c in CASES if c['source'] in (LIGHT,SCENE)):
                proof = compare(retail,objects[case['source']],case['symbol'],int(case['target_va'],0),
                                case['target_size'],ROOT/case['bindings'])
                self.assertTrue(proof['strict_exact'],proof);strict += 1
            targets = json.loads((ROOT/'src/krusty2/motion/targets.json').read_text())
            for target in (t for t in targets if t.get('source')=='D3DIMSoultreeMotnctrl.cpp' and t.get('expect')=='exact'):
                proof = compare(retail,objects[MOTION],target['candidate_symbol_contains'],int(target['target_va'],0),
                                target['target_size'],ROOT/'src/krusty2/motion'/target['bindings'])
                self.assertTrue(proof['strict_exact'],proof);strict += 1
            for source,symbol,va,size in ((SOURCE,VIEW,0x4fc630,38),
                (RENDER_SOURCE,OLD,0x4fc630,38),
                (TREE,'?SetPosition@SoultreeObject@@QAEXMMM@Z',0x4fc630,38),
                (RENDER,'??0D3DIMSoultreeObject@@',0x43f160,275)):
                proof = compare(retail,objects[source],symbol,va,size,(ROOT/source).with_suffix('.bindings.json'))
                self.assertTrue(proof['strict_exact'],proof);self.assertEqual(proof['candidate_size'],size);strict += 1
            for name,va,size in HELPERS:
                symbol='?'+name+'@SoultreeObject@@'
                for source in (TREE,'helpers'):
                    proof=compare(retail,objects[source],symbol,va,size,(ROOT/TREE).with_suffix('.bindings.json'))
                    self.assertTrue(proof['strict_exact'],proof);self.assertEqual(proof['candidate_size'],size);strict += 1
                self.assertEqual(complete(objects[TREE],symbol),complete(objects['helpers'],symbol))
            for source,symbol,site,va,destination in ((LIGHT,'?SetPosition@LightEmitter@@',0x49e1a0,0x49e150,OLD),
                (MOTION,'?CharacterVirtualSlot4@D3DIMSoultreeCharacter@@',0x446291,0x446210,VIEW)):
                entry=objects[source].find_symbol(symbol)
                _,_,relocs=objects[source].symbol_extent(entry)
                calls=[r for r in relocs if objects[source].symbol_by_index[r.symbol_index].name==destination]
                self.assertEqual([(r.type,va+r.virtual_address-entry.value-1) for r in calls],[(20,site)])
            compiler=Path(os.environ['VC6_ROOT']);executable=output/'position.exe'
            result=subprocess.run([str(find_child_ci(find_vc6_bin(compiler),'link.exe')),'/nologo','/subsystem:console',
                '/base:0x100000','/fixed','/out:'+str(executable),
                '/libpath:'+str(find_child_ci(infer_vc98_root(compiler),'lib')),
                str(paths[SOURCE]),str(paths[RENDER_SOURCE]),str(paths['helpers']),str(paths['layout']),str(paths[FIXTURE]),'kernel32.lib'],
                cwd=ROOT,capture_output=True,text=True,encoding='latin1')
            self.assertEqual(result.returncode,0,result.stdout+result.stderr)
            result=subprocess.run([str(executable),str(image)],capture_output=True,text=True,timeout=60)
            self.assertEqual(result.returncode,0,result.stdout+result.stderr)
            self.assertEqual(result.stdout.strip(),'Soultree position: 158212 raw-bit, cache and partial-fault cases passed')
            print('%d strict checks; five complete caller/aggregate objects; both setters; %s'%(strict,result.stdout.strip()))


if __name__=='__main__':
    unittest.main()
