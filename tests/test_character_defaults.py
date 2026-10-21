"""Individual Character defaults retain primary slots and virtual-base boundaries."""
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
from mcm2tool.toolchain import find_child_ci, find_vc6_bin, infer_vc98_root
from tools.match import compare

ROOT = Path(__file__).resolve().parents[1]
SOURCE = 'src/reconstructed/CharacterDefaults.cpp'
MOTION = 'src/krusty2/motion/Motnctrl.cpp'
DERIVED = 'src/krusty2/motion/D3DIMSoultreeMotnctrl.cpp'
FIXTURE = 'tests/fixtures/character_defaults.cpp'
SLOTS = (0, 2, 3, 9, 10)
DERIVED_TARGETS = (0x445a70, 0x445fc0, 0x446110, 0x445ec0, 0x446520)


def symbol(slot, owner='Character'):
    return '?CharacterVirtualSlot%d@%s@@UAEX%s' % (slot, owner, 'H@Z' if slot in (3, 9) else 'XZ')


@unittest.skipUnless(os.name == 'nt' and os.environ.get('VC6_ROOT') and os.environ.get('MCM2_EXE'),
                     'requires authentic native VC6 and private retail')
class CharacterDefaultsTests(unittest.TestCase):
    def test_full_bodies_primary_slots_virtual_bases_and_no_read_behavior(self):
        import capstone
        image = Path(os.environ['MCM2_EXE']).resolve()
        self.assertEqual(hashlib.sha256(image.read_bytes()).hexdigest(),
                         '31fde4cc686a5ee89ef9095b90235325b195596867ecacefe511263e1509b874')
        retail = PEImage(image)
        word = lambda va: struct.unpack('<I', retail.bytes_at_va(va, 4))[0]
        for owner, tables in (('Character', ((0x555318, 0), (0x5552a8, 416))),
                              ('D3DIMSoultreeCharacter', ((0x551530, 0), (0x5514c0, 532)))):
            for table, offset in tables:
                locator = word(table - 4)
                self.assertEqual(word(locator + 4), offset)
                name = ('.?AV' + owner + '@@\0').encode()
                self.assertEqual(retail.bytes_at_va(word(locator + 12) + 8, len(name)), name)
                hierarchy = word(locator + 16)
                count, array = word(hierarchy + 8), word(hierarchy + 12)
                bases = {}
                for pointer in struct.unpack('<' + 'I' * count, retail.bytes_at_va(array, 4 * count)):
                    td, _, mdisp, pdisp, vdisp, _ = struct.unpack('<2I3iI', retail.bytes_at_va(pointer, 24))
                    bases[retail.bytes_at_va(td + 8, 55).split(b'\0')[0]] = (mdisp, pdisp, vdisp)
                self.assertEqual(bases[b'.?AVCharacter@@'], (0, -1, 0))
                self.assertEqual(bases[b'.?AVGameObject@@'], (0, 4, 4))
        for slot, derived in zip(SLOTS, DERIVED_TARGETS):
            self.assertEqual(word(0x555318 + 4 * slot), 0x464e80 if slot in (3, 9) else 0x464e90)
            self.assertEqual(word(0x551530 + 4 * slot), derived)
        self.assertEqual(retail.bytes_at_va(0x464e80, 16), b'\xc2\x04\x00' + b'\x90' * 13)
        self.assertEqual(retail.bytes_at_va(0x464e90, 16), b'\xc3' + b'\x90' * 15)
        decoder = capstone.Cs(capstone.CS_ARCH_X86, capstone.CS_MODE_32)
        constructor = {i.address: (i.mnemonic, i.op_str) for i in
                       decoder.disasm(retail.bytes_at_va(0x4a6780, 335), 0x4a6780)}
        self.assertEqual(constructor[0x4a67af], ('mov', 'dword ptr [ebx], 0x555318'))
        self.assertEqual(constructor[0x4a68cc], ('ret', '8'))
        setup = {i.address: (i.mnemonic, i.op_str) for i in
                 decoder.disasm(retail.bytes_at_va(0x4a6930, 145), 0x4a6930)}
        self.assertEqual(setup[0x4a69a8], ('call', 'dword ptr [edx]'))
        self.assertEqual(setup[0x4a69ae], ('call', 'dword ptr [eax + 8]'))
        with tempfile.TemporaryDirectory(prefix='mcm2-character-defaults-') as directory:
            output = Path(directory)
            objects, paths = {}, {}
            for index, source in enumerate((SOURCE, MOTION, DERIVED, FIXTURE)):
                obj = output / ('unit%d.obj' % index)
                command = [sys.executable, str(ROOT / 'tools/compile.py'), str(ROOT / source),
                           '-o', str(obj), '--vc6-root', os.environ['VC6_ROOT'], '--profile', 'vc6_o2_mt',
                           '--include', str(ROOT / 'src/krusty2')]
                if source == FIXTURE:
                    command += ['--extra', '/Od']
                result = subprocess.run(command, cwd=ROOT, capture_output=True, text=True, encoding='latin1')
                self.assertEqual(result.returncode, 0, result.stdout + result.stderr)
                objects[source], paths[source] = CoffObject(obj), obj
            provider = objects[SOURCE]
            self.assertFalse(any(s.section_number == 0 for s in provider.symbols))
            self.assertEqual({s.name for s in provider.symbols if s.section_number > 0 and s.storage_class == 2},
                             {symbol(slot) for slot in SLOTS})
            self.assertFalse(any(s.name.startswith('.CRT') or s.characteristics & 0xc0
                                 for s in provider.sections if not s.name.startswith('.debug')))
            for slot in SLOTS:
                entry = provider.find_symbol(symbol(slot))
                va, size = (0x464e80, 3) if slot in (3, 9) else (0x464e90, 1)
                self.assertEqual((entry.function_size, entry.extent_source), (size, 'coff_function_aux'))
                self.assertEqual(provider.symbol_extent(entry), (retail.bytes_at_va(va, size), size, []))
                self.assertTrue(compare(retail, provider, symbol(slot), va, size,
                                        ROOT / 'src/reconstructed/CharacterDefaults.bindings.json')['strict_exact'])
            for source, owner, primary in ((MOTION, 'Character', '??_7Character@@6B0@@'),
                                           (DERIVED, 'D3DIMSoultreeCharacter', '??_7D3DIMSoultreeCharacter@@6BCharacter@@@')):
                obj = objects[source]
                table = obj.find_symbol(primary)
                relocs = {r.virtual_address: obj.symbol_by_index[r.symbol_index] for r in obj.relocations
                          if r.section_number == table.section_number}
                for slot in SLOTS:
                    self.assertEqual(relocs[table.value + 4 * slot].name, symbol(slot, owner))
                locator = relocs[table.value - 4]
                section = obj.sections[locator.section_number - 1]
                self.assertEqual(struct.unpack_from('<I', obj.data, section.raw_ptr + locator.value + 4)[0], 0)
            strict = 5
            manifest = ROOT / 'src/krusty2/motion/targets.json'
            for target in json.loads(manifest.read_text()):
                if target.get('source') not in ('Motnctrl.cpp', 'D3DIMSoultreeMotnctrl.cpp') or target.get('expect') != 'exact':
                    continue
                source = 'src/krusty2/motion/' + target['source']
                proof = compare(retail, objects[source], target['candidate_symbol_contains'],
                                int(target['target_va'], 0), target['target_size'], manifest.parent / target['bindings'])
                self.assertTrue(proof['strict_exact'], (source, target['candidate_symbol_contains'], proof))
                strict += 1
            compiler = Path(os.environ['VC6_ROOT'])
            executable = output / 'defaults.exe'
            result = subprocess.run([str(find_child_ci(find_vc6_bin(compiler), 'link.exe')), '/nologo',
                '/subsystem:console', '/out:' + str(executable),
                '/libpath:' + str(find_child_ci(infer_vc98_root(compiler), 'lib')),
                str(paths[SOURCE]), str(paths[FIXTURE]), 'kernel32.lib'], cwd=ROOT,
                capture_output=True, text=True, encoding='latin1')
            self.assertEqual(result.returncode, 0, result.stdout + result.stderr)
            result = subprocess.run([str(executable)], capture_output=True, text=True, timeout=30)
            self.assertEqual(result.returncode, 0, result.stdout + result.stderr)
            self.assertEqual(result.stdout.strip(), 'Character defaults: 4902 complete-storage and no-read calls passed')
            print('%d strict checks; four retail tables and two actual primary tables; %s' %
                  (strict, result.stdout.strip()))


if __name__ == '__main__':
    unittest.main()
