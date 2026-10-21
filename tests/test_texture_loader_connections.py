"""Check actual texture/material provider imports and unchanged full callers."""
from __future__ import annotations
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
from mcm2tool.toolchain import sha256_file
from tools.match import compare
from tools.run_calibration import CASES
ROOT=Path(__file__).resolve().parents[1]
EXE=Path(os.environ.get('MCM2_EXE',ROOT/'work/game/mcm2.exe'))
LOADER='?TerrainLoadTexture@@YAPAVTextureMap@@PAVTextureMapManager@@PBDHPAVPalette8@@HHHPAUUnknownTextureFormatChoice@@HIHH@Z'
MATERIAL={'??0SoultreeMaterial@@QAE@H@Z',
 '?Attach@SoultreeMaterial@@QAEPAV1@PAVRenderTarget@@PBUSoultreeTextureOptions@@PAVTextureMapManager@@H@Z',
 '?CopyFrom@SoultreeMaterial@@QAEXPBV1@@Z'}
@unittest.skipUnless(os.environ.get('VC6_ROOT') and EXE.is_file(),'requires VC6_ROOT and private retail image')
class TextureLoaderConnections(unittest.TestCase):
    def test_shared_signatures_and_complete_caller_proof(self):
        self.assertEqual(sha256_file(EXE),'31fde4cc686a5ee89ef9095b90235325b195596867ecacefe511263e1509b874')
        retail=PEImage(EXE)
        self.assertEqual(retail.bytes_at_va(0x445cd0,5),bytes.fromhex('68d0000000'))
        for site,target in ((0x445cf4,0x4fefe0),(0x445d46,0x4ff0b0),(0x445d85,0x5000f0)):
            call=retail.bytes_at_va(site,5)
            self.assertEqual(call[0],0xe8)
            self.assertEqual(site+5+struct.unpack('<i',call[1:])[0],target)
        sources={name:'src/reconstructed/'+name+'.cpp' for name in
          ('TextureMap','SoultreeMaterial','BikeNumberPainter','FontTexture','TrackOverlay','EcoSystem','SceneManager')}
        sources.update({'Particles':'src/krusty2/effects/Particles.cpp',
            'Terrain':'src/krusty2/broadphase/Terrain.cpp',
            'motion':'src/krusty2/motion/D3DIMSoultreeMotnctrl.cpp',
            'layout':'tests/fixtures/texture_loader_layout.cpp'})
        with tempfile.TemporaryDirectory(prefix='mcm2-texture-connections-') as temp:
            objects={}
            for name,source in sources.items():
                obj=Path(temp)/(name+'.obj')
                result=subprocess.run([sys.executable,str(ROOT/'tools/compile.py'),str(ROOT/source),
                    '-o',str(obj),'--vc6-root',os.environ['VC6_ROOT'],'--profile','vc6_o2_mt',
                    '--include',str(ROOT/'src/krusty2'),'--include',str(ROOT/'src/reconstructed')],
                    cwd=ROOT,capture_output=True,text=True,encoding='latin-1',
                    env={**os.environ,'PYTHONPATH':str(ROOT)})
                self.assertEqual(result.returncode,0,result.stdout+result.stderr)
                objects[name]=CoffObject(obj)
            def imports(obj):
                return {obj.symbol_by_index[r.symbol_index].name for r in obj.relocations
                    if obj.symbol_by_index[r.symbol_index].section_number==0}
            def definitions(obj):
                return {s.name for s in obj.symbols if s.section_number>0 and s.storage_class==2}
            self.assertIn(LOADER,definitions(objects['TextureMap']))
            for name in ('SoultreeMaterial','BikeNumberPainter','FontTexture','TrackOverlay','EcoSystem','Particles','Terrain'):
                calls=imports(objects[name])
                self.assertIn(LOADER,calls,name)
                self.assertEqual({s for s in calls if s.startswith('?TerrainLoadTexture@@')},{LOADER})
            self.assertTrue(MATERIAL<=definitions(objects['SoultreeMaterial']) & imports(objects['motion']))
            self.assertFalse(any('SceneItem@@' in s for s in imports(objects['motion'])))
            source_names={source:name for name,source in sources.items()}
            matched=set()
            for case in CASES:
                if case['source'] not in source_names:continue
                name=source_names[case['source']]
                result=compare(retail,objects[name],case['symbol'],int(case['target_va'],0),
                    case['target_size'],ROOT/case['bindings'])
                self.assertTrue(result['strict_exact'],result)
                matched.add((name,int(case['target_va'],0)))
            self.assertTrue({('TextureMap',0x50a590),('SoultreeMaterial',0x4fefe0),
                ('SoultreeMaterial',0x4ff0b0),('SoultreeMaterial',0x5000f0),
                ('SceneManager',0x4eb570)}<=matched)
            self.assertGreaterEqual(len(matched),236)
            physics_matches=set()
            for target_file in ('src/krusty2/effects/targets.json','src/krusty2/broadphase/targets.json','src/krusty2/motion/targets.json'):
                path=ROOT/target_file
                for case in json.loads(path.read_text()):
                    source=(path.parent/case['source']).resolve().relative_to(ROOT).as_posix()
                    if source not in source_names or case.get('expect','exact')!='exact':continue
                    result=compare(retail,objects[source_names[source]],case['candidate_symbol_contains'],
                        int(case['target_va'],0),case['target_size'],path.parent/case['bindings'])
                    self.assertTrue(result['strict_exact'],result)
                    physics_matches.add((source,int(case['target_va'],0)))
            self.assertGreaterEqual(len(physics_matches),55)

if __name__=='__main__':unittest.main()
