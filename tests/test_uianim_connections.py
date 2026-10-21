"""Two UIAnim call boundaries preserve whole objects and the actual texture getter."""
import hashlib
import json
import os
import posixpath
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

ROOT = Path(__file__).resolve().parents[1]
SOURCES = ('src/reconstructed/GUIManager.cpp', 'src/reconstructed/DlgProcs.cpp',
           'samples/ui/GUIManagerNearMisses.cpp', 'samples/ui/DlgProcsNearMisses.cpp')
FILENAMES = SOURCES + ('src/reconstructed/ContainerList.h', 'samples/ui/GuiScreenTexture.cpp')
GETTER = '?GetCurrentTexture@UIAnim@@QAEPAVTextureMap@@XZ'
ADVANCE = '?AdvancePastSounds@UIAnim@@QAEPAVTextureMap@@XZ'
RENAMES = {'?GetCurrentTexture@UnknownDialogImage@@QAEHXZ': GETTER,
           '?AdvancePastSounds@UnknownCursorAnimation@@QAEHXZ': ADVANCE}


def pin(rows):
    return dict(sections=len(rows), relocations=sum(len(row[3]) for row in rows),
                sha256=hashlib.sha256(json.dumps(rows).encode()).hexdigest())


def whole_object(obj, roots=(ROOT,)):
    """Keep all nondebug contents; normalize only the enumerated source paths."""
    sections = [s for s in obj.sections if not s.name.startswith('.debug')]
    prefixes = [str(root).replace('\\', '/') + '/' for root in roots]

    def filename(data):
        value, nul, tail = data.partition(b'\0')
        path = posixpath.normpath(value.decode('latin1').replace('\\', '/'))
        for prefix in prefixes:
            if nul and path.lower().startswith(prefix.lower()) and path[len(prefix):] in FILENAMES:
                return ('source:' + path[len(prefix):]).encode() + nul + tail
        return data

    def name(symbol):
        if symbol.name.startswith('??_C@') and symbol.section_number > 0:
            section = obj.section(symbol.section_number)
            data = obj.data[section.raw_ptr + symbol.value:section.raw_ptr + section.raw_size]
            normalized = filename(data)
            if normalized != data:
                return normalized.split(b'\0')[0].decode()
        return symbol.name

    keys, anonymous = {}, {}
    for section in sections:
        labels = tuple(sorted((name(s), s.value) for s in obj.symbols
                       if s.section_number == section.index and (s.storage_class == 2 or s.type == 32)))
        if labels:
            keys[section.index] = ('named', labels)
        else:
            ordinal = anonymous.get(section.name, 0)
            anonymous[section.name] = ordinal + 1
            keys[section.index] = ('anonymous', section.name, ordinal)
    assert len(set(keys.values())) == len(keys)

    def target(symbol):
        if symbol.section_number > 0:
            return ('defined', keys[symbol.section_number], symbol.value)
        return ('external', symbol.name, symbol.value, symbol.section_number)

    rows = []
    for section in sections:
        data = obj.data[section.raw_ptr:section.raw_ptr + section.raw_size] if section.raw_ptr else bytes(section.raw_size)
        relocations = [(r.virtual_address, r.type, target(obj.symbol_by_index[r.symbol_index]))
                       for r in obj.relocations if r.section_number == section.index]
        rows.append((keys[section.index], section.characteristics, filename(data).hex(), relocations))
    return sorted(rows, key=lambda row: repr(row[0]))


@unittest.skipUnless(os.name == 'nt' and os.environ.get('VC6_ROOT') and os.environ.get('MCM2_EXE'),
                     'requires authentic VC6 and hash-pinned private retail')
class UIAnimConnectionsTests(unittest.TestCase):
    def test_full_objects_receiver_chain_and_actual_getter(self):
        import capstone
        image_path = Path(os.environ['MCM2_EXE']).resolve()
        image = PEImage(image_path)
        self.assertEqual(hashlib.sha256(image.data).hexdigest(),
                         '31fde4cc686a5ee89ef9095b90235325b195596867ecacefe511263e1509b874')
        expected = json.loads((ROOT/'tests/fixtures/uianim_connection_objects.json').read_text())
        self.assertEqual(expected['renames'], RENAMES)
        decoder = capstone.Cs(capstone.CS_ARCH_X86, capstone.CS_MODE_32)
        decoder.detail = True
        def decode(address, size):
            code = list(decoder.disasm(image.bytes_at_va(address, size), address))
            self.assertEqual(sum(i.size for i in code), size)
            return code
        def instruction(address, size, mnemonic, operands):
            code = decode(address, size)
            self.assertEqual([(i.mnemonic, i.op_str) for i in code], [(mnemonic, operands)])
        classes = {c.name: c for c in parse_rtti(image)}
        anim = classes['UIAnim']
        self.assertEqual([(t.complete_object_locator_va, t.object_offset, t.vtables)
                          for t in anim.vtable_records], [(0x55c410, 0, [0x552ccc])])
        self.assertEqual([(b.name, b.mdisp, b.pdisp) for b in anim.bases],
                         [('UIAnim', 0, -1), ('BaseObject', 0, -1)])
        self.assertEqual(image.bytes_at_va(anim.bases[0].type_va+8, 14), b'.?AVUIAnim@@\0\0')
        # Actual allocation -> primary constructor -> image table -> named section.
        for args in (
            (0x46ba94, 5, 'push', '0xf8'),
            (0x46ba99, 5, 'call', '0x4a3010'),
            (0x46baba, 2, 'mov', 'ecx, eax'),
            (0x46babc, 5, 'call', '0x472e30'),
            (0x46bad5, 7, 'mov', 'dword ptr [ebp + ecx*4 + 0x18c], eax'),
            (0x46bc49, 7, 'mov', 'eax, dword ptr [ebp + edx*4 + 0x18c]'),
            (0x46bc50, 6, 'mov', 'dword ptr [ecx + 0xa00], eax'),
            (0x472e51, 6, 'mov', 'dword ptr [esi], 0x552ccc'),
            (0x46e9f6, 7, 'mov', 'eax, dword ptr [edi + eax*4 + 0xa00]'),
            # Both default animation and per-control MouseAnim preserve that pointer.
            (0x46b545, 5, 'call', '0x46e9a0'),
            (0x46b54a, 6, 'mov', 'dword ptr [ebp + 0x95c], eax'),
            (0x46da6f, 7, 'lea', 'eax, [esp + 0x2004]'),
            (0x46da81, 5, 'push', '0x56b838'),
            (0x46da8d, 5, 'call', '0x4b7ec0'),
            (0x46e55d, 7, 'lea', 'eax, [esp + 0x2004]'),
            (0x46e567, 5, 'call', '0x46e9a0'),
            (0x46e56c, 6, 'mov', 'dword ptr [ebx + 0x180], eax'),
            (0x46f033, 6, 'mov', 'eax, dword ptr [edi + 0x180]'),
            (0x46f03d, 6, 'mov', 'eax, dword ptr [esi + 0x95c]'),
            (0x46f043, 1, 'push', 'eax'),
            (0x46f044, 5, 'call', '0x487fb0'),
            (0x488000, 5, 'call', '0x485150'),
            (0x485156, 3, 'mov', 'dword ptr [ecx + 0x5c], eax'),
            (0x485173, 3, 'mov', 'ecx, dword ptr [esi + 0x5c]'),
            (0x48517a, 5, 'call', '0x4730b0'),
            (0x48517f, 3, 'mov', 'dword ptr [esi + 0x38], eax'),
            # Loading dialog: named UIAnim -> raw EAX texture -> existing bar API.
            (0x4511b0, 5, 'push', '0x569d94'),
            (0x4511b7, 5, 'call', '0x46e9a0'),
            (0x4511bd, 2, 'mov', 'ecx, eax'),
            (0x4511bf, 5, 'call', '0x472f90'),
            (0x4511c4, 1, 'push', 'eax'),
            (0x4511c7, 5, 'call', '0x47b3d0'),
        ): instruction(*args)
        self.assertEqual(image.bytes_at_va(0x569d94, 8), b'BlueBar\0')
        self.assertEqual(image.bytes_at_va(0x56b838, 10), b'MouseAnim\0')
        with tempfile.TemporaryDirectory(prefix='mcm2-uianim-') as directory:
            output = Path(directory)
            compiler = Path(os.environ['VC6_ROOT'])
            def compile_source(source, name, includes=False):
                path = output/(name+'.obj')
                command = [sys.executable, str(ROOT/'tools/compile.py'), str(source),
                           '-o', str(path), '--vc6-root', str(compiler), '--profile', 'vc6_o2_mt']
                if includes:
                    command += ['--include', str(ROOT/'src/reconstructed')]
                run = subprocess.run(command, cwd=ROOT, capture_output=True, text=True, encoding='latin1')
                self.assertEqual(run.returncode, 0, run.stdout+run.stderr)
                return path, CoffObject(path)
            strict = sections = relocations = 0
            for n, source in enumerate(SOURCES):
                _, obj = compile_source(ROOT/source, 'consumer'+str(n))
                rows = whole_object(obj)
                self.assertEqual(pin(rows), expected['objects'][source])
                sections += len(rows)
                relocations += sum(len(row[3]) for row in rows)
                for case in CASES:
                    if case['source'] != source:
                        continue
                    result = compare(image, obj, case['symbol'], int(case['target_va'], 0),
                                     case['target_size'], ROOT/Path(source).with_suffix('.bindings.json'))
                    self.assertTrue(result['strict_exact'], result)
                    strict += 1
                wanted = ADVANCE if n in (0, 2) else GETTER
                imports = {s.name for s in obj.symbols if s.storage_class == 2 and s.section_number == 0}
                self.assertIn(wanted, imports)
                self.assertFalse(imports & set(RENAMES))
                caller = '?RenderPost3D@GUICursor@@UAEHXZ' if n in (0, 2) else '?HandleEvent@LoadingDlg@@UAEXPAUUnknownDialogEvent@@@Z'
                entry = obj.find_symbol(caller)
                _, _, calls = obj.symbol_extent(entry)
                self.assertEqual([(r.virtual_address-entry.value, r.type) for r in calls
                                  if obj.symbol_by_index[r.symbol_index].name == wanted],
                                 [(11 if n in (0, 2) else 416, 20)])
            self.assertEqual((strict, sections, relocations), (193, 1387, 8896))
            _, provider = compile_source(ROOT/'src/reconstructed/GameUi.cpp', 'provider')
            for symbol, address, size in (
                ('??0UIAnim@@QAE@PAX0@Z', 0x472e30, 82),
                (GETTER, 0x472f90, 20), (ADVANCE, 0x4730b0, 46),
                ('?Advance@UIAnim@@QAEPAVUIFrame@@XZ', 0x472fe0, 200),
                ('?FindSectionObject@UIDialog@@QAEPAVSound@@PBD@Z', 0x46e9a0, 109),
            ):
                self.assertTrue(compare(image, provider, symbol, address, size,
                    ROOT/'src/reconstructed/GameUi.bindings.json')['strict_exact'])
                strict += 1
            source = output/'getter.cpp'
            source.write_text('#include "GameUi.h"\n' + extract(
                (ROOT/'src/reconstructed/GameUi.cpp').read_text(),
                'TextureMap* UIAnim::GetCurrentTexture(') + '\n')
            getter_path, getter = compile_source(source, 'getter', True)
            self.assertEqual(signature(getter, GETTER), signature(provider, GETTER))
            self.assertTrue(compare(image, getter, GETTER, 0x472f90, 20,
                ROOT/'src/reconstructed/GameUi.bindings.json')['strict_exact'])
            strict += 1
            self.assertFalse([s for s in getter.symbols if s.storage_class == 2 and s.section_number == 0])
            self.assertFalse([s for s in getter.sections if not s.name.startswith('.debug')
                              and (s.name.startswith('.CRT') or s.characteristics & 0xc0)])
            fixture_path, _ = compile_source(ROOT/'tests/fixtures/uianim_getter_compare.cpp', 'fixture', True)
            executable = output/'getter.exe'
            result = subprocess.run([str(find_child_ci(find_vc6_bin(compiler), 'link.exe')),
                '/nologo', '/subsystem:console', '/base:0x100000', '/fixed', '/opt:ref',
                '/out:'+str(executable), '/libpath:'+str(find_child_ci(infer_vc98_root(compiler), 'lib')),
                str(getter_path), str(fixture_path), 'kernel32.lib'],
                cwd=ROOT, capture_output=True, text=True, encoding='latin1')
            self.assertEqual(result.returncode, 0, result.stdout+result.stderr)
            result = subprocess.run([str(executable), str(image_path)], capture_output=True, text=True, timeout=30)
            self.assertEqual(result.returncode, 0, result.stdout+result.stderr)
            self.assertEqual(result.stdout.strip(),
                'UIAnim GetCurrentTexture: 3067 cases, 41 independently checked AV; no storage writes')
            self.assertEqual(strict, 199)
        print('199 strict checks; four whole objects; two retail calls; '+result.stdout.strip())


if __name__ == '__main__':
    unittest.main()
