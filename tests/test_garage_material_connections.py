"""Garage material replacement retains the primary model receiver and full caller objects."""
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
from tools.run_calibration import CASES
from test_edit_box_on_char import extract
from test_edit_box_draw import signature
from test_uianim_connections import whole_object, pin

ROOT = Path(__file__).resolve().parents[1]
SOURCES = ('src/reconstructed/DlgProcs.cpp', 'samples/ui/DlgProcsNearMisses.cpp')
RENDER = 'src/reconstructed/D3DIMSoulTree.cpp'
GARAGE = 'src/reconstructed/KrustyUI.cpp'
CHARACTER = 'src/krusty2/motion/D3DIMSoultreeMotnctrl.cpp'
MATERIAL = 'src/reconstructed/SoultreeMaterial.cpp'
PROVIDER = '?ReplaceMaterialTexture@D3DIMSoultreeObject@@QAEHHPBDH@Z'
OLD = '?ReplaceMaterialTexture@UnknownModelTexture@@QAEXHPBDPAPAVTextureMapManager@@@Z'
CALLER = '?RenderPre3D@SPBikeRiderDlg@@UAEHXZ'


@unittest.skipUnless(os.name == 'nt' and os.environ.get('VC6_ROOT') and os.environ.get('MCM2_EXE'),
                     'requires authentic native VC6 and hash-pinned retail')
class GarageMaterialConnectionsTests(unittest.TestCase):
    def test_whole_objects_primary_receiver_and_actual_replacement(self):
        import capstone
        image_path = Path(os.environ['MCM2_EXE']).resolve()
        image = PEImage(image_path)
        self.assertEqual(hashlib.sha256(image.data).hexdigest(),
                         '31fde4cc686a5ee89ef9095b90235325b195596867ecacefe511263e1509b874')
        expected = json.loads((ROOT/'tests/fixtures/garage_material_objects.json').read_text())
        self.assertEqual(expected['renames'], {OLD: PROVIDER})
        decoder = capstone.Cs(capstone.CS_ARCH_X86, capstone.CS_MODE_32)
        decoder.detail = True

        def decode(va, size):
            code = list(decoder.disasm(image.bytes_at_va(va, size), va))
            self.assertEqual(sum(i.size for i in code), size)
            return code

        # Type names/COL and primary/secondary offsets are checked independently
        # of the source spelling of the old plateTexture field.
        classes = {c.name: c for c in parse_rtti(image)}
        model = classes['D3DIMSoultreeObject']
        for table, offset in ((0x5513ec, 0), (0x55137c, 12)):
            locator, = struct.unpack('<I', image.bytes_at_va(table-4, 4))
            _, actual, _, descriptor, _ = struct.unpack('<5I', image.bytes_at_va(locator, 20))
            self.assertEqual(actual, offset)
            self.assertEqual(image.bytes_at_va(descriptor+8, len(b'.?AVD3DIMSoultreeObject@@')), b'.?AVD3DIMSoultreeObject@@')
        for name, offset in (('SoultreeObject', 0), ('GameObject', 12)):
            base = next(b for b in model.bases if b.name == name)
            self.assertEqual((base.mdisp, base.pdisp), (offset, -1))
        character = classes['D3DIMSoultreeCharacter']
        self.assertTrue(any(t.object_offset == 0 and 0x551530 in t.vtables
                            for t in character.vtable_records))
        self.assertEqual(image.bytes_at_va(character.bases[0].type_va+8, len(b'.?AVD3DIMSoultreeCharacter@@')),
                         b'.?AVD3DIMSoultreeCharacter@@')
        self.assertEqual(struct.unpack('<I', image.bytes_at_va(0x551530, 4))[0], 0x445a70)
        code = {i.address: (i.mnemonic, i.op_str) for va, size in
                ((0x498cf0, 3114), (0x445a70, 1021), (0x44ffc0, 257)) for i in decode(va, size)}
        for va, instruction in {
            0x499548: ('push', '0x240'), 0x49956e: ('call', '0x4455b0'),
            0x499577: ('mov', 'dword ptr [ebp + 0x46c], eax'),
            0x4995a7: ('call', 'dword ptr [edx + 0x2c]'),
            0x4996d5: ('push', '0x240'), 0x4996f7: ('call', '0x4455b0'),
            0x499709: ('mov', 'dword ptr [ecx + edi + 0xc0], eax'),
            0x499741: ('call', 'dword ptr [eax + 0x2c]'),
            0x445b29: ('call', '0x43f160'),
            0x445b45: ('mov', 'dword ptr [ebp + 0x1a0], eax'),
            0x445b6a: ('mov', 'eax, dword ptr [ebp + 0x1a0]'),
            0x445b74: ('add', 'eax, 0xc'),
            0x450035: ('mov', 'ecx, dword ptr [eax + edx*8 + 0xc0]'),
            0x45003c: ('mov', 'ecx, dword ptr [ecx + 0x1a0]'),
            0x450042: ('call', '0x444c70'),
            0x450047: ('mov', 'dword ptr [esi + 0x7f84], 0'),
            0x45009e: ('mov', 'eax, dword ptr [edi + 0x46c]'),
            0x4500a6: ('mov', 'ecx, dword ptr [eax + 0x1a0]'),
            0x4500ac: ('call', '0x444c70'),
            0x4500b1: ('mov', 'dword ptr [esi + 0x7f88], 0'),
        }.items(): self.assertEqual(code[va], instruction)
        caller = decode(0x44ffc0, 257)
        self.assertEqual([i.address for i in caller if i.mnemonic == 'call' and i.op_str == '0x444c70'],
                         [0x450042, 0x4500ac])
        # The pointer-valued third argument remains exactly TrackGame+1c. EAX's
        # int return is ignored. The full provider only reads arguments1 and2.
        self.assertEqual([i.address for i in caller if i.mnemonic == 'add' and i.op_str == 'edx, 0x1c'],
                         [0x45001d, 0x450090])
        provider_code = decode(0x444c70, 133)
        self.assertEqual([(i.address, i.op_str) for i in provider_code if i.mnemonic == 'ret'],
                         [(0x444c84, '0xc'), (0x444c9e, '0xc'), (0x444cf2, '0xc')])
        self.assertEqual([(i.address, i.op_str) for i in provider_code if 'esp +' in i.op_str],
                         [(0x444c71, 'ebx, dword ptr [esp + 8]'),
                          (0x444ca2, 'ebp, dword ptr [esp + 0x14]')])
        self.assertEqual([(i.address, i.op_str) for i in provider_code if i.mnemonic == 'call'],
                         [(0x444cc5, '0x534b10'), (0x444ce4, '0x4ff620')])
        with tempfile.TemporaryDirectory(prefix='mcm2-garage-material-') as directory:
            output = Path(directory)
            compiler = Path(os.environ['VC6_ROOT'])

            def compile_source(source, name, includes=()):
                obj = output/(name+'.obj')
                command = [sys.executable, str(ROOT/'tools/compile.py'), str(source), '-o', str(obj),
                           '--vc6-root', str(compiler), '--profile', 'vc6_o2_mt']
                for include in includes: command += ['--include', str(ROOT/include)]
                result = subprocess.run(command, cwd=ROOT, capture_output=True, text=True, encoding='latin1')
                self.assertEqual(result.returncode, 0, result.stdout+result.stderr)
                return obj, CoffObject(obj)

            strict = 0
            for n, source in enumerate(SOURCES):
                _, obj = compile_source(ROOT/source, 'consumer'+str(n))
                self.assertEqual(pin(whole_object(obj)), expected['objects'][source])
                for case in CASES:
                    if case['source'] != source: continue
                    self.assertTrue(compare(image, obj, case['symbol'], int(case['target_va'], 0),
                        case['target_size'], ROOT/Path(source).with_suffix('.bindings.json'))['strict_exact'])
                    strict += 1
                imports = {s.name for s in obj.symbols if s.section_number == 0 and s.storage_class == 2}
                self.assertIn(PROVIDER, imports)
                self.assertNotIn(OLD, imports)
                symbol = obj.find_symbol(CALLER)
                _, size, relocs = obj.symbol_extent(symbol)
                self.assertEqual(size, 257)
                self.assertEqual([(r.virtual_address-symbol.value, r.type) for r in relocs
                                  if obj.symbol_by_index[r.symbol_index].name == PROVIDER], [(131, 20), (237, 20)])
            objects = {}
            for n, source in enumerate((RENDER, GARAGE, CHARACTER, MATERIAL)):
                _, objects[source] = compile_source(ROOT/source, 'provider'+str(n),
                    ('src/krusty2',) if source == CHARACTER else ())
            for source, symbol, va, size in (
                (RENDER, PROVIDER, 0x444c70, 133),
                (RENDER, '??0D3DIMSoultreeObject@@QAE@H@Z', 0x43f160, 275),
                (GARAGE, '?BuildGarageScene@KrustyUI@@QAEXH@Z', 0x498cf0, 3114),
                (CHARACTER, '??0D3DIMSoultreeCharacter@@QAE@H@Z', 0x4455b0, 195),
                (CHARACTER, '?CharacterVirtualSlot0@D3DIMSoultreeCharacter@@UAEXXZ', 0x445a70, 1021),
                (MATERIAL, '?LoadTexture@SoultreeMaterial@@QAEXXZ', 0x4ff620, 948),
            ):
                obj = objects[source]
                self.assertEqual(obj.symbol_extent(obj.find_symbol(symbol))[1], size)
                self.assertTrue(compare(image, obj, symbol, va, size,
                                       ROOT/Path(source).with_suffix('.bindings.json'))['strict_exact'])
                strict += 1
            extracted = output/'replace.cpp'
            extracted.write_text('#include "D3DIMSoulTree.h"\n#include "SoultreeMaterial.h"\n#include <string.h>\n'+
                extract((ROOT/RENDER).read_text(), 'int D3DIMSoultreeObject::ReplaceMaterialTexture(')+'\n')
            native_path, native = compile_source(extracted, 'replace', ('src/reconstructed',))
            self.assertEqual(signature(native, PROVIDER), signature(objects[RENDER], PROVIDER))
            self.assertTrue(compare(image, native, PROVIDER, 0x444c70, 133,
                                   ROOT/Path(RENDER).with_suffix('.bindings.json'))['strict_exact'])
            strict += 1
            self.assertEqual({s.name for s in native.symbols if s.section_number == 0 and s.storage_class == 2},
                             {'_strncpy', '?LoadTexture@SoultreeMaterial@@QAEXXZ'})
            self.assertFalse([s for s in native.sections if not s.name.startswith('.debug')
                              and (s.name.startswith('.CRT') or s.characteristics & 0xc0)])
            fixture_path, _ = compile_source(ROOT/'tests/fixtures/garage_material_compare.cpp',
                                             'fixture', ('src/reconstructed',))
            executable = output/'compare.exe'
            result = subprocess.run([str(find_child_ci(find_vc6_bin(compiler), 'link.exe')),
                '/nologo', '/subsystem:console', '/base:0x100000', '/fixed', '/opt:ref',
                '/out:'+str(executable), '/libpath:'+str(find_child_ci(infer_vc98_root(compiler), 'lib')),
                str(native_path), str(fixture_path), 'kernel32.lib'],
                cwd=ROOT, capture_output=True, text=True, encoding='latin1')
            self.assertEqual(result.returncode, 0, result.stdout+result.stderr)
            result = subprocess.run([str(executable), str(image_path)], capture_output=True, text=True, timeout=30)
            self.assertEqual(result.returncode, 0, result.stdout+result.stderr)
            self.assertEqual(result.stdout.strip(),
                             'ReplaceMaterialTexture: 11589 cases, 69 AV, 1080 scripted throws')
        self.assertEqual(strict, 99)
        print('%d strict checks; two whole objects; two calls; %s' % (strict, result.stdout.strip()))


if __name__ == '__main__':
    unittest.main()
