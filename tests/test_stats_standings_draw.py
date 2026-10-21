"""Complete standings body, real source/CRT helpers, and finite surface contracts."""
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
from mcm2tool.coff_archive import get_archive_member
from mcm2tool.pe import PEImage
from mcm2tool.rtti import parse_rtti
from mcm2tool.toolchain import find_child_ci, find_vc6_bin, infer_vc98_root
from tools.match import compare
from tools.build_game import compile_source as game_compile
from test_edit_box_on_char import extract
from test_edit_box_draw import signature
from test_chat_name_tag_draw import object_view

ROOT = Path(__file__).resolve().parents[1]
SOURCE = ROOT/'samples/track/StatsStandingsDraw.cpp'
AGGREGATE = ROOT/'samples/track/TrackOverlayNearMisses.cpp'
SYMBOL = '?DrawStandings@StatsOverlay@@QAEHXZ'
ITERATOR = '?NextBike@BikeRace@@QAEPAVUnknownBikeRaceRacer@@PAH@Z'
OLD_ITERATOR = '?NextBike@UnknownKrustyBikeView@@QAEPAUUnknownEventRacer@@PAH@Z'
DIRECT = {0x519a68:0x521970, 0x519a9f:0x518640, 0x519ab8:0x534aaf,
          0x519acb:0x4204e0, 0x519afb:0x4204e0, 0x519b11:0x534426,
          0x519bb2:0x534aaf, 0x519c33:0x534aaf, 0x519ca7:0x4b6710,
          0x519dec:0x4b6880}
INDIRECT = {0x519cbe:'dword ptr [edx + 0x44]', 0x519cd0:'dword ptr [0x550070]',
            0x519cdd:'dword ptr [0x550074]', 0x519ced:'dword ptr [0x550080]',
            0x519cff:'dword ptr [0x550078]', 0x519d35:'dword ptr [0x550084]',
            0x519d72:'dword ptr [0x5502e8]', 0x519db4:'dword ptr [0x550078]',
            0x519dc9:'dword ptr [ecx + 0x68]', 0x519dfa:'dword ptr [eax + 0x24]'}
# Each extraction is checked against both complete retail and the actual full TU.
HELPER_GROUPS = [
    ('iteration', '#include "BikeRaceCore.h"\n#include "TrackGame.h"\n',
     [('BikeRace', 'UnknownBikeRaceRacer* BikeRace::NextBike(', 0x4204e0, 162)]),
    ('format', '#include <stdio.h>\n#include "Track.h"\n',
     [('Track', 'void FormatMinutesSeconds(', 0x518640, 67)]),
    ('comparator', '#include "TrackOverlay.h"\n#include "EventManager.h"\n',
     [('TrackOverlay', 'int CompareScoresDescending(', 0x5199f0, 43)]),
    ('resource', '#include <stdio.h>\n#include <string.h>\n#include "TrackGame.h"\n'
     'extern "C" __declspec(dllimport) int __stdcall LoadStringA(void*,unsigned int,char*,int);\n',
     [('TrackGame', 'int TrackGame::LoadResourceString(', 0x521970, 189)]),
    ('surface', '#include <string.h>\n#include "PCTextureMap.h"\n#include "PCRenderTarget.h"\n'
     '#include "TrackGame.h"\n#include "Tgafile.h"\n#include "Overlay.h"\n',
     [('PCTextureMap', 'void* PCTextureMap::LockSurface(', 0x4c79e0, 99),
      ('PCTextureMap', 'int PCTextureMap::UnlockSurface(', 0x4c7a50, 25),
      ('PCTextureMap', 'int PCTextureMap::Upload(', 0x4c7640, 385),
      ('Tgafile', 'int PixelFormatBytesPerPixel(', 0x511970, 78),
      ('Overlay', 'int Overlay::TintRows(', 0x4b6880, 484)])]
PRODUCERS = [
    ('TrackOverlay', '??0StatsOverlay@@', 0x519370, 133),
    ('TrackOverlay', '?LoadPanel@StatsOverlay@@', 0x5194b0, 976),
    ('TrackOverlay', '?Redraw@StatsOverlay@@', 0x5198a0, 96),
    ('TrackOverlay', '?RenderPre3D@StatsOverlay@@', 0x519940, 61),
    ('PCTextureMap', '??0PCTextureMap@@', 0x4c5f00, 46),
    ('Overlay', '??0Overlay@@', 0x4b5e20, 120),
    ('Overlay', '?Attach@Overlay@@', 0x4b5f50, 583)]


def record(obj, portable=False):
    rows = object_view(obj, portable=portable)
    return dict(sections=len(rows), relocations=sum(len(r[3]) for r in rows),
                sha256=hashlib.sha256(json.dumps(rows,separators=(',',':')).encode()).hexdigest())


@unittest.skipUnless(os.name == 'nt' and os.environ.get('VC6_ROOT') and os.environ.get('MCM2_EXE'),
                     'requires authentic native VC6 and private retail')
class StatsStandingsTests(unittest.TestCase):
    def test_complete_standings_real_helpers_and_finite_surfaces(self):
        import capstone
        image_path = Path(os.environ['MCM2_EXE']).resolve()
        self.assertEqual(hashlib.sha256(image_path.read_bytes()).hexdigest(),
                         '31fde4cc686a5ee89ef9095b90235325b195596867ecacefe511263e1509b874')
        image = PEImage(image_path)
        decoder = capstone.Cs(capstone.CS_ARCH_X86,capstone.CS_MODE_32)
        decoder.detail = True
        def decoded(va,size): return list(decoder.disasm(image.bytes_at_va(va,size),va))
        body = decoded(0x519a20,1002)
        self.assertEqual(sum(i.size for i in body),1002)
        self.assertEqual([(i.address,bytes(i.bytes)) for i in body if i.mnemonic=='ret'],
                         [(0x519c99,b'\xc3'),(0x519dd9,b'\xc3'),(0x519e09,b'\xc3')])
        self.assertEqual(image.bytes_at_va(0x519e0a,6),b'\x90'*6)
        self.assertEqual({i.address:i.operands[0].imm for i in body if i.mnemonic=='call' and
                          i.operands[0].type==capstone.x86.X86_OP_IMM},DIRECT)
        self.assertEqual({i.address:i.op_str for i in body if i.mnemonic=='call' and
                          i.operands[0].type!=capstone.x86.X86_OP_IMM},INDIRECT)
        starts={i.address for i in body}
        for instruction in body:
            if instruction.group(capstone.CS_GRP_JUMP):
                self.assertIn(instruction.operands[0].imm,starts)
        self.assertFalse(any('fs:' in i.op_str for i in body))
        classes={c.name:c for c in parse_rtti(image)}
        self.assertEqual([(r.object_offset,r.vtables) for r in classes['StatsOverlay'].vtable_records],
                         [(0,[0x558680])])
        for name,table in [('StatsOverlay',0x558680),('BikeRace',0x5510d4),('PCTextureMap',0x555fec)]:
            # Use the independently parsed table for BikeRace; raw COL/name/PMDs
            # below prevent inferring identity solely from a shared method VA.
            if name=='BikeRace':table=classes[name].vtable_records[0].vtables[0]
            locator=struct.unpack('<I',image.bytes_at_va(table-4,4))[0]
            words=struct.unpack('<5I',image.bytes_at_va(locator,20))
            self.assertEqual(words[1],0)
            self.assertEqual(image.bytes_at_va(words[3]+8,64).split(b'\0')[0],('.?AV'+name+'@@').encode())
            self.assertTrue(all(b.mdisp==0 and b.pdisp==-1 for b in classes[name].bases))
        self.assertEqual(struct.unpack('<I',image.bytes_at_va(0x55867c,4))[0],0x55f6e8)
        self.assertEqual(struct.unpack('<2i',image.bytes_at_va(0x5546ec,8)),(-4,0x1608))
        col=struct.unpack('<5I',image.bytes_at_va(0x55cff0,20))
        self.assertEqual(col[1],0x160c)
        self.assertEqual(image.bytes_at_va(col[3]+8,32).split(b'\0')[0],b'.?AVKrustyBike@@')
        instructions={i.address:(i.mnemonic,i.op_str) for va,size in
                      [(0x519370,133),(0x5198a0,96),(0x4e07c0,1049),(0x48fa60,532),(0x519a20,1002)]
                      for i in decoded(va,size)}
        for va,mnemonic,operand in [
            (0x5193a8,'mov','dword ptr [esi], 0x558680'),
            (0x5198b7,'call','0x519a20'),
            (0x4e0a39,'push','0x9b8'),(0x4e0a5d,'call','0x519370'),(0x4e0a9c,'call','0x5194b0'),
            (0x48fa8e,'mov','dword ptr [esi + 4], 0x5546ec'),
            (0x48fa97,'lea','ecx, [esi + 0x160c]'),
            (0x519a3e,'mov','eax, dword ptr [ecx + 0x55c]'),
            (0x519a44,'mov','ebp, dword ptr [eax + 0x34]'),
            (0x519ad8,'mov','ecx, dword ptr [eax + 4]'),
            (0x519adb,'mov','edx, dword ptr [ecx + 4]'),
            (0x519ade,'test','byte ptr [edx + eax + 0x29], 1'),
            (0x519ae5,'mov','ecx, dword ptr [eax + 0x768]')]:
            self.assertEqual(instructions[va],(mnemonic,operand))
        self.assertEqual(struct.unpack('<I',image.bytes_at_va(0x555fec+9*4,4))[0],0x4c7640)
        self.assertEqual(struct.unpack('<2I',image.bytes_at_va(0x555fec+13*4,8)),(0x4c79e0,0x4c7a50))
        compiler=Path(os.environ['VC6_ROOT'])
        linker=find_child_ci(find_vc6_bin(compiler),'link.exe')
        libraries=find_child_ci(infer_vc98_root(compiler),'lib')
        with tempfile.TemporaryDirectory(prefix='mcm2-stats-standings-') as directory:
            output=Path(directory)
            def compile_source(source,name,fixture=False):
                path=output/(name+'.obj')
                includes=[] if name in ('candidate','aggregate','restore') else [
                    '--include',str(ROOT/'src/reconstructed'),'--include',str(ROOT/'src/krusty2')]
                result=subprocess.run([sys.executable,str(ROOT/'tools/compile.py'),str(source),'-o',str(path),
                    '--vc6-root',str(compiler),'--profile','vc6_o2_mt',*includes,
                    *(['--extra','/Od'] if fixture else [])],cwd=ROOT,capture_output=True,
                    text=True,encoding='latin1',env={**os.environ,'PYTHONPATH':str(ROOT)})
                self.assertEqual(result.returncode,0,result.stdout+result.stderr)
                return path,CoffObject(path)
            candidate_path,candidate=compile_source(SOURCE,'candidate')
            _,aggregate=compile_source(AGGREGATE,'aggregate')
            units={unit for _,_,items in HELPER_GROUPS for unit,_,_,_ in items}
            canonical={unit:compile_source(ROOT/f'src/reconstructed/{unit}.cpp',unit)[1] for unit in units}
            pins=json.loads((ROOT/'tests/fixtures/stats_standings_objects.json').read_text())
            self.assertEqual(record(candidate),pins['candidate'])
            self.assertEqual(record(aggregate,True),pins['aggregate'])
            self.assertEqual(record(canonical['TrackOverlay'],True),pins['canonical'])
            self.assertEqual(signature(candidate,SYMBOL),signature(aggregate,SYMBOL))
            result=compare(image,candidate,SYMBOL,0x519a20,1002,SOURCE.with_suffix('.bindings.json'))
            self.assertEqual((result['candidate_size'],result['strict_match']['matching_positions'],result['strict_exact']),
                             (1004,605,False))
            entry=candidate.find_symbol(SYMBOL)
            _,_,relocs=candidate.symbol_extent(entry)
            self.assertEqual([r.virtual_address-entry.value for r in relocs
                              if candidate.symbol_by_index[r.symbol_index].name==ITERATOR],[172,220])
            self.assertNotIn(OLD_ITERATOR,[s.name for s in candidate.symbols])
            # The other aggregate caller keeps its independently provisional view.
            self.assertIn('?NextBike@UnknownChatView@@QAEPAUUnknownChatRacer@@PAH@Z',
                          [s.name for s in aggregate.symbols])
            helper_paths=[];strict=0
            for name,header,items in HELPER_GROUPS:
                source=output/(name+'.cpp')
                source.write_text(header+'\n'.join(extract((ROOT/f'src/reconstructed/{unit}.cpp').read_text(),sig)
                                                  for unit,sig,_,_ in items)+'\n')
                path,obj=compile_source(source,name);helper_paths.append(path)
                for unit,sig,va,size in items:
                    short=sig.split(' ')[-1].rstrip('(')
                    symbol='?'+'@'.join(reversed(short.split('::')))+'@@'
                    for compared in (obj,canonical[unit]):
                        report=compare(image,compared,symbol,va,size,ROOT/f'src/reconstructed/{unit}.bindings.json')
                        self.assertTrue(report['strict_exact'],report);strict+=1
                    self.assertEqual(signature(obj,symbol),signature(canonical[unit],symbol))
            for unit,symbol,va,size in PRODUCERS:
                report=compare(image,canonical[unit],symbol,va,size,ROOT/f'src/reconstructed/{unit}.bindings.json')
                self.assertTrue(report['strict_exact'],report);strict+=1
            restore_path,restore=compile_source(ROOT/'samples/render/OverlayNearMisses.cpp','restore')
            report=compare(image,restore,'?RestoreRect@Overlay@@',0x4b6710,365,
                           ROOT/'samples/render/OverlayNearMisses.bindings.json')
            self.assertEqual((report['candidate_size'],report['strict_match']['matching_positions']), (365,77))
            # The source graph uses this already-reviewed nonexact helper. Both
            # whole-method runs share it; its own larger regression is separate.
            bindings=output/'crt.bindings.json'
            bindings.write_text(json.dumps({'_qsort':'0x534426','_shortsort':'0x53457a','_swap':'0x5345c8',
                                            '__output':'0x539359','__flsbuf':'0x539241'}))
            archive=find_child_ci(libraries,'libcmt.lib')
            for member,items in [
                ('build/intel/mt_obj/qsort.obj',[('_qsort',0x534426,340),('_shortsort',0x53457a,78),('_swap',0x5345c8,44)]),
                ('build/intel/mt_obj/sprintf.obj',[('_sprintf',0x534aaf,82)]),
                ('../build/intel/mt_obj/ftol.obj',[('__ftol',0x534a88,39)])]:
                path=output/Path(member).name;path.write_bytes(get_archive_member(archive,member));obj=CoffObject(path)
                for symbol,va,size in items:
                    report=compare(image,obj,symbol,va,size,bindings)
                    self.assertTrue(report['strict_exact'],report);strict+=1
            layout=output/'layout.cpp'
            layout.write_text('#include <stddef.h>\n#include "TrackOverlay.h"\n#include "TrackGame.h"\n'
                '#include "BikeRaceCore.h"\n#include "PCTextureMap.h"\n#include "Display.h"\n'
                'typedef char Layout[(sizeof(StatsOverlay)==0x9b8 && sizeof(int(StatsOverlay::*)())==4 && '
                'sizeof(BikeRace)==0x400 && sizeof(PCTextureMap)==0x80 && sizeof(UnknownEventScore)==8 && '
                'offsetof(StatsOverlay,largeFont)==0x11c && offsetof(StatsOverlay,rowRects)==0x130 && '
                'offsetof(StatsOverlay,drawnRowText)==0x1b0 && offsetof(StatsOverlay,rowText)==0x5b0 && '
                'offsetof(StatsOverlay,rowExtent)==0x9b0 && offsetof(TrackGame,quarryEvent)==0x55c && '
                'offsetof(TrackGameViewOwner,raceView)==0x34 && offsetof(BikeRace,localRacer)==0x38 && '
                'offsetof(UnknownEventRacer,racerName)==0x5e0 && offsetof(UnknownEventRacer,score)==0x768 && '
                'offsetof(Overlay,overlayTexture)==0x2c && offsetof(Overlay,backingTexture)==0x30 && '
                'offsetof(Overlay,sharedTexture)==0x34 && offsetof(Overlay,sourceRect)==0xf8 && '
                'offsetof(TextureMap,textureWidth)==0x14 && offsetof(TextureMap,mapFormat)==0x20 && '
                'offsetof(TextureMap,usesColorKey)==0x30 && offsetof(PCTextureMap,systemSurface)==0x70 && '
                'offsetof(PCTextureMap,textureSurface)==0x74 && '
                'offsetof(UnknownDisplay,partialTextureUploadResult)==0x5bc)?1:-1];\n')
            # UnknownEventScore lives with the event callback declarations.
            layout.write_text('#include "EventManager.h"\n'+layout.read_text())
            compile_source(layout,'layout')
            spec=json.loads((ROOT/'config/game_build.json').read_text())
            built=game_compile(SOURCE,output/'game-command',compiler,[ROOT/p for p in spec['includes']],
                               spec['profile'],{**os.environ,'PYTHONPATH':str(ROOT)})
            self.assertTrue(built['compiled'],built)
            self.assertEqual(object_view(candidate),object_view(CoffObject(Path(built['object']))))
            fixture_path,_=compile_source(ROOT/'tests/fixtures/stats_standings_compare.cpp','fixture',True)
            executable=output/'stats.exe'
            result=subprocess.run([str(linker),'/nologo','/subsystem:console','/base:0x100000','/fixed',
                '/opt:ref',f'/out:{executable}',f'/libpath:{libraries}',str(candidate_path),str(fixture_path),
                *map(str,helper_paths),str(restore_path),'kernel32.lib','user32.lib','gdi32.lib'],
                cwd=ROOT,capture_output=True,text=True,encoding='latin1')
            self.assertEqual(result.returncode,0,result.stdout+result.stderr)
            result=subprocess.run([str(executable),str(image_path)],cwd=ROOT,capture_output=True,text=True,timeout=120)
            self.assertEqual(result.returncode,0,result.stdout+result.stderr)
            self.assertIn('Stats DrawStandings: 3228 cases match, 78 AV, 26 throws',result.stdout)
            self.assertEqual(strict,30)
            print(f'Stats DrawStandings: {strict} strict game/CRT comparisons; full aggregate and canonical pins; '+result.stdout.strip())


if __name__=='__main__':
    unittest.main()
