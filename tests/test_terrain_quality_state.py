"""Literal writable quality tables and the actual strict Terrain selector."""
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
from mcm2tool.toolchain import find_child_ci, find_vc6_bin, infer_vc98_root
from tools.build_game import ROOT as TOOL_ROOT
from tools.match import compare
from test_garage_shadow_connections import function_text
import test_eco_quadtree_connections as views

ROOT = Path(__file__).resolve().parents[1]
STATE = 'src/reconstructed/TerrainQualityState.cpp'
TERRAIN = 'src/krusty2/broadphase/Terrain.cpp'
FILENAMES = (TERRAIN,)
ARRAYS = (('g_terrainQualityHardware', 0x574920,
           '9d22cdb07404e3fd89585a8a6e80bd3b839d5743098b9d931300c5890c660707'),
          ('g_terrainQualitySoftware', 0x5749c0,
           'c34dd66e2418231ac0c0a492d80c5a03c3322f9cb7bb1a2194d39da6025eaf90'))


def whole(obj):
    with patch.object(views, 'FILENAMES', FILENAMES):
        return views.whole_object(obj, (ROOT,))


def pin(obj):
    rows = whole(obj)
    return dict(sections=len(rows), relocations=sum(len(r[-1]) for r in rows),
                sha256=hashlib.sha256(json.dumps(rows, separators=(',', ':')).encode()).hexdigest())


def body(obj, name):
    symbol = obj.find_symbol(name)
    data, size, relocations = obj.symbol_extent(symbol)
    return data, size, [(r.virtual_address-symbol.value, r.type,
                        obj.symbol_by_index[r.symbol_index].name) for r in relocations]


@unittest.skipUnless(os.name == 'nt' and os.environ.get('VC6_ROOT') and os.environ.get('MCM2_EXE'),
                     'requires authentic native VC6 and pinned retail')
class TerrainQualityStateTests(unittest.TestCase):
    def test_complete_data_and_actual_selector(self):
        import capstone
        pe = PEImage(Path(os.environ['MCM2_EXE']))
        self.assertEqual(hashlib.sha256(pe.data).hexdigest(),
                         '31fde4cc686a5ee89ef9095b90235325b195596867ecacefe511263e1509b874')
        md = capstone.Cs(capstone.CS_ARCH_X86, capstone.CS_MODE_32)
        md.detail = True
        instructions = {}
        for va, size in ((0x5059d0, 2116), (0x507930, 37), (0x507960, 129)):
            decoded = list(md.disasm(pe.bytes_at_va(va, size), va))
            self.assertEqual(sum(i.size for i in decoded), size)
            instructions.update({i.address: (i.mnemonic, i.op_str) for i in decoded})
        for va, expected in {
            0x505a2a: ('mov', 'eax, dword ptr [ecx + 0x2d0]'),
            0x505a30: ('mov', 'dword ptr [0x68a32c], 0x574920'),
            0x505a3a: ('test', 'eax, eax'), 0x505a3c: ('jne', '0x505a48'),
            0x505a3e: ('mov', 'dword ptr [0x68a32c], 0x5749c0'),
            0x505a52: ('call', '0x507960'),
            0x507967: ('mov', 'dword ptr [esi + 0xbf0], eax'),
            0x507973: ('shl', 'eax, 4'),
            0x507976: ('mov', 'eax, dword ptr [eax + ecx + 0xc]'),
            0x507988: ('mov', 'eax, dword ptr [edx + ecx]'),
            0x50798e: ('call', '0x507930'),
            0x5079a3: ('mov', 'edx, dword ptr [ecx + 0x94]'),
            0x5079ba: ('mov', 'edx, dword ptr [eax + ecx + 4]'),
            0x5079d3: ('mov', 'edx, dword ptr [eax + ecx + 8]'),
            0x5079de: ('ret', '4'),
        }.items():
            self.assertEqual(instructions[va], expected, hex(va))
        bindings = json.loads((ROOT/STATE).with_suffix('.bindings.json').read_text())
        self.assertEqual(bindings, {'?'+name+'@@3PAUTerrainQualityEntry@@A': hex(va)
                                    for name, va, _ in ARRAYS})
        with tempfile.TemporaryDirectory(prefix='mcm2-terrain-quality-') as directory:
            out = Path(directory)
            objects, paths = {}, {}
            def compile_source(source, label):
                source = Path(source)
                if not source.is_absolute(): source = ROOT/source
                dest = out/(label+'.obj')
                run = subprocess.run([sys.executable, str(TOOL_ROOT/'tools/compile.py'), str(source),
                    '-o', str(dest), '--vc6-root', os.environ['VC6_ROOT'], '--profile', 'vc6_o2_mt',
                    '--include', str(ROOT/'src/krusty2')], cwd=TOOL_ROOT,
                    env={**os.environ, 'PYTHONPATH': str(TOOL_ROOT)}, capture_output=True, text=True)
                self.assertEqual(run.returncode, 0, run.stdout+run.stderr)
                objects[label], paths[label] = CoffObject(dest), dest
                return objects[label]
            state = compile_source(STATE, 'state')
            terrain = compile_source(TERRAIN, 'terrain')
            recorded = json.loads((ROOT/'tests/fixtures/terrain_quality_objects.json').read_text())
            self.assertEqual(pin(terrain), recorded['objects'][TERRAIN])
            sections = [s for s in state.sections if not s.name.startswith('.debug')]
            self.assertEqual([(s.name, s.raw_size, s.characteristics) for s in sections],
                             [('.drectve', 40, 0x100a00), ('.data', 320, 0xc0400040)])
            self.assertFalse([r for r in state.relocations
                              if not state.section(r.section_number).name.startswith('.debug')])
            exports = {s.name: s for s in state.symbols if s.storage_class == 2}
            self.assertEqual(set(exports), set(bindings))
            for index, (name, va, digest) in enumerate(ARRAYS):
                symbol = exports['?'+name+'@@3PAUTerrainQualityEntry@@A']
                section = state.section(symbol.section_number)
                self.assertEqual((section.name, symbol.value), ('.data', index*160))
                raw = state.data[section.raw_ptr+symbol.value:section.raw_ptr+symbol.value+160]
                self.assertEqual(raw, pe.bytes_at_va(va, 160))
                self.assertEqual(hashlib.sha256(raw).hexdigest(), digest)
                original = pe.section_for_rva(va-pe.image_base)
                self.assertEqual((original.name, original.characteristics), ('.data', 0xc0000040))
                self.assertLessEqual(va-pe.image_base-original.virtual_address+160, original.raw_size)
                self.assertEqual(sum(terrain.symbol_by_index[r.symbol_index].name == symbol.name
                                     for r in terrain.relocations
                                     if not terrain.section(r.section_number).name.startswith('.debug')), 1)
            strict = 0
            for case in json.loads((ROOT/'src/krusty2/broadphase/targets.json').read_text()):
                if case['source'] != 'Terrain.cpp' or case.get('expect') != 'exact': continue
                result = compare(pe, terrain, case['candidate_symbol_contains'],
                                 int(case['target_va'], 0), case['target_size'],
                                 (ROOT/TERRAIN).with_suffix('.bindings.json'))
                self.assertTrue(result['strict_exact'], case)
                strict += 1
            source = (ROOT/TERRAIN).read_text()
            consumer = out/'consumer.cpp'
            consumer.write_text('#include "'+(ROOT/'src/krusty2/broadphase/Terrain.h').as_posix()+'"\n'
                +'TerrainQualityEntry* g_pTerrainQualityTable;\nint g_terrainQualityValue;\n'
                +function_text(source, 'void Terrain::SetField0xbec(')+'\n'
                +function_text(source, 'void Terrain::SelectQuality(')+'\n')
            actual = compile_source(consumer, 'consumer')
            for name, va, size in (('SetField0xbec', 0x507930, 37), ('SelectQuality', 0x507960, 129)):
                self.assertEqual(body(actual, name), body(terrain, name))
                self.assertTrue(compare(pe, actual, name, va, size,
                    (ROOT/TERRAIN).with_suffix('.bindings.json'))['strict_exact'])
                strict += 1
            compile_source('tests/fixtures/terrain_quality_state.cpp', 'fixture')
            vc6 = Path(os.environ['VC6_ROOT'])
            binary = out/'native.exe'
            run = subprocess.run([str(find_child_ci(find_vc6_bin(vc6), 'link.exe')), '/nologo',
                '/subsystem:console', '/out:'+str(binary),
                '/libpath:'+str(find_child_ci(infer_vc98_root(vc6), 'lib')),
                *(str(paths[k]) for k in ('state', 'consumer', 'fixture')), 'kernel32.lib'],
                capture_output=True, text=True)
            self.assertEqual(run.returncode, 0, run.stdout+run.stderr)
            run = subprocess.run([str(binary)], capture_output=True, text=True, timeout=30)
            self.assertEqual(run.returncode, 0, run.stdout+run.stderr)
            self.assertEqual(run.stdout.strip(),
                'Terrain quality: 5130 cases, 8 independently modeled access faults')
            self.assertEqual(strict, 24)
            print('%d strict checks; complete unchanged Terrain object; %s' % (strict, run.stdout.strip()))


if __name__ == '__main__': unittest.main()
