"""Independently identify folded methods used by startup and background rendering."""
import hashlib
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

ROOT = Path(__file__).resolve().parents[1]
PROVIDERS = (
    ('ZoneInterfaceLifetime', '??0UnknownTrackGameObject3410Base@@QAE@XZ', 0x4676a0, 16),
    ('BackgroundImageHooks', '?UnknownVirtualSlot27@BackgroundImage@@UAEHPAUUnknownSurfaceInterface@@@Z', 0x4627f0, 8),
)


@unittest.skipUnless(os.name == 'nt' and os.environ.get('VC6_ROOT') and os.environ.get('MCM2_EXE'),
                     'requires authentic native VC6 and the private retail image')
class StartupDefaultMethodsTests(unittest.TestCase):
    def test_complete_retail_bodies_callers_and_untouched_storage(self):
        import capstone
        path = Path(os.environ['MCM2_EXE']).resolve()
        self.assertEqual(hashlib.sha256(path.read_bytes()).hexdigest(),
                         '31fde4cc686a5ee89ef9095b90235325b195596867ecacefe511263e1509b874')
        retail = PEImage(path)
        decoder = capstone.Cs(capstone.CS_ARCH_X86, capstone.CS_MODE_32)
        decoder.detail = True
        classes = {c.name: c for c in parse_rtti(retail)}
        parent = next(b for b in classes['BackgroundImage'].bases if b.name == 'GameObject')
        self.assertEqual((parent.mdisp, parent.pdisp), (0, -1))
        self.assertEqual(struct.unpack('<I', retail.bytes_at_va(0x5506d8 - 4, 4))[0], 0x55a9a8)
        locator = struct.unpack('<IIIII', retail.bytes_at_va(0x55a9a8, 20))
        self.assertEqual(locator[1], 0)  # Primary object offset.
        type_name = b'.?AVBackgroundImage@@\0'
        self.assertEqual(retail.bytes_at_va(locator[3] + 8, len(type_name)), type_name)
        self.assertEqual(struct.unpack('<I', retail.bytes_at_va(0x5506d8 + 27 * 4, 4))[0], 0x4627f0)
        constructor = list(decoder.disasm(retail.bytes_at_va(0x4676a0, 16), 0x4676a0))
        self.assertEqual([(i.mnemonic, i.op_str) for i in constructor], [
            ('mov', 'eax, ecx'), ('mov', 'dword ptr [eax], 0'),
            ('mov', 'dword ptr [eax + 4], 0'), ('ret', '')])
        hook = list(decoder.disasm(retail.bytes_at_va(0x4627f0, 8), 0x4627f0))
        self.assertEqual([(i.mnemonic, i.op_str) for i in hook], [('mov', 'eax, 1'), ('ret', '4')])
        # Allocation -> constructor -> storage at TrackGame+0x3410, rather
        # than identity inferred from unrelated users of the folded address.
        call = next(decoder.disasm(retail.bytes_at_va(0x5215d3, 5), 0x5215d3))
        self.assertEqual((call.mnemonic, call.operands[0].imm), ('call', 0x4676a0))
        startup = {i.address: (i.mnemonic, i.op_str) for i in
                   decoder.disasm(retail.bytes_at_va(0x5215af, 0x54), 0x5215af)}
        for address, instruction in {
            0x5215af: ('push', '0x32c'),
            0x5215b4: ('call', '0x4a3010'), 0x5215b9: ('mov', 'edi, eax'),
            0x5215d1: ('mov', 'ecx, edi'), 0x5215d8: ('mov', 'ecx, edi'),
            0x5215eb: ('mov', 'dword ptr [esi + 0x3410], ecx'),
            0x5215f3: ('mov', 'eax, dword ptr [esi + 8]'),
            0x5215fe: ('call', '0x4aa350'),
        }.items():
            self.assertEqual(startup[address], instruction)
        render = {i.address: (i.mnemonic, i.op_str) for i in
                  decoder.disasm(retail.bytes_at_va(0x403dc0, 250), 0x403dc0)}
        for address, instruction in {
            0x403dc1: ('mov', 'esi, ecx'),
            0x403e23: ('mov', 'eax, dword ptr [esi + 0x3c]'),
            0x403e26: ('mov', 'edx, dword ptr [esi]'),
            0x403e28: ('push', 'eax'), 0x403e29: ('mov', 'ecx, esi'),
            0x403e2b: ('call', 'dword ptr [edx + 0x6c]'),
            0x403e5c: ('mov', 'eax, dword ptr [esi + 0x18]'),
            0x403e5f: ('mov', 'edx, dword ptr [esi]'),
            0x403e61: ('mov', 'ecx, dword ptr [eax + 0x48]'),
            0x403e64: ('push', 'ecx'), 0x403e65: ('mov', 'ecx, esi'),
            0x403e67: ('call', 'dword ptr [edx + 0x6c]'),
        }.items():
            self.assertEqual(render[address], instruction)
        with tempfile.TemporaryDirectory(prefix='mcm2-startup-defaults-') as directory:
            output = Path(directory)
            sources = ['src/reconstructed/' + p[0] + '.cpp' for p in PROVIDERS]
            sources += ['tests/fixtures/startup_default_methods.cpp']
            objects = []
            for index, source in enumerate(sources):
                obj = output / ('unit%d.obj' % index)
                result = subprocess.run([sys.executable, str(ROOT / 'tools/compile.py'), str(ROOT / source),
                    '-o', str(obj), '--vc6-root', os.environ['VC6_ROOT'], '--profile', 'vc6_o2_mt',
                    '--include', str(ROOT / 'src/krusty2'), '--include', str(ROOT / 'src/reconstructed')],
                    cwd=ROOT, capture_output=True, text=True, encoding='latin1')
                self.assertEqual(result.returncode, 0, result.stdout + result.stderr)
                objects.append(obj)
            for obj, (name, symbol, va, size) in zip(objects, PROVIDERS):
                self.assertTrue(compare(retail, CoffObject(obj), symbol, va, size,
                    ROOT / ('src/reconstructed/' + name + '.bindings.json'))['strict_exact'])
            compiler = Path(os.environ['VC6_ROOT'])
            executable = output / 'defaults.exe'
            result = subprocess.run([str(find_child_ci(find_vc6_bin(compiler), 'link.exe')), '/nologo',
                '/subsystem:console', f'/out:{executable}', f'/libpath:{find_child_ci(infer_vc98_root(compiler), "lib")}',
                *(str(p) for p in objects), 'kernel32.lib'], cwd=ROOT,
                capture_output=True, text=True, encoding='latin1')
            self.assertEqual(result.returncode, 0, result.stdout + result.stderr)
            result = subprocess.run([str(executable)], capture_output=True, text=True, timeout=30)
            self.assertEqual(result.returncode, 0, result.stdout + result.stderr)
            self.assertEqual(result.stdout.strip(),
                'Startup defaults: 256 complete constructor storage and 768 unchanged hook cases passed')
            print('Two complete strict bodies; ' + result.stdout.strip())


if __name__ == '__main__':
    unittest.main()
