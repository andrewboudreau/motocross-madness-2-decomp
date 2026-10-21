"""Full FollowCamera Tick candidate with actual FastSqrt and scripted boundaries."""
import os
import json
from pathlib import Path
import struct
import subprocess
import sys
import tempfile
import unittest

from mcm2tool.coff import CoffObject
from mcm2tool.pe import PEImage
from mcm2tool.toolchain import find_child_ci, find_vc6_bin, infer_vc98_root, sha256_file
from tools.match import compare

ROOT = Path(__file__).resolve().parents[1]
SYMBOL = '?Tick@FollowCamera@@UAEHM@Z'


@unittest.skipUnless(os.name == 'nt' and os.environ.get('VC6_ROOT') and os.environ.get('MCM2_EXE'),
                     'requires native VC6 and private retail image')
class FollowCameraTickBehaviorTests(unittest.TestCase):
    def test_complete_camera_state_callbacks_allocations_and_floats(self):
        import capstone
        reference = Path(os.environ['MCM2_EXE']).resolve()
        self.assertEqual(sha256_file(reference),
                         '31fde4cc686a5ee89ef9095b90235325b195596867ecacefe511263e1509b874')
        image = PEImage(reference)
        decoder = capstone.Cs(capstone.CS_ARCH_X86, capstone.CS_MODE_32)
        decoder.detail = True
        instructions = list(decoder.disasm(image.bytes_at_va(0x465c20, 3672), 0x465c20))
        self.assertEqual(sum(i.size for i in instructions), 3672)
        self.assertEqual(bytes(instructions[-1].bytes), bytes.fromhex('c20400'))
        self.assertEqual(image.bytes_at_va(0x466a78, 8), b'\x90' * 8)
        self.assertEqual(struct.unpack('<I', image.bytes_at_va(0x55282c+10*4, 4))[0], 0x465c20)
        boundaries = {i.address for i in instructions}
        calls, slots = {}, set()
        for i in instructions:
            if i.group(capstone.CS_GRP_JUMP):
                self.assertEqual(i.operands[0].type, capstone.x86.X86_OP_IMM)
                self.assertIn(i.operands[0].imm, boundaries)
            if i.mnemonic == 'call':
                if i.operands[0].type == capstone.x86.X86_OP_IMM:
                    self.assertEqual(i.bytes[0], 0xe8)
                    calls[i.address] = i.operands[0].imm
                else:
                    self.assertEqual(i.operands[0].type, capstone.x86.X86_OP_MEM)
                    slots.add(i.operands[0].mem.disp//4)
        self.assertEqual(calls, {0x465d03:0x507c10, 0x465d56:0x53304c, 0x465d71:0x53304c,
            0x465de3:0x42e9b0, 0x465e50:0x507c10, 0x465fc1:0x460b50,
            0x46608b:0x4a3010, 0x4660d2:0x4a3010, 0x46610a:0x4a3010,
            0x466151:0x4a3010, 0x466189:0x4a3010, 0x4661c1:0x4a3010,
            0x466305:0x460b50, 0x46671b:0x42e9b0, 0x466a2b:0x42e930, 0x466a37:0x42e690})
        self.assertEqual(slots, {3,29,33,36,41,42,43,44,45,48,49,51,52,53,54,55,56,57,59,67,68,74,75})
        self.assertFalse(any('fs:' in i.op_str for i in instructions))
        # Complete original initializer bodies initialize the reference axes.
        for start, target in ((0x466f70,0x65b458),(0x467000,0x65b468)):
            decoded = list(decoder.disasm(image.bytes_at_va(start,60),start))
            self.assertEqual(decoded[-1].mnemonic,'ret')
            self.assertFalse(any(i.group(capstone.CS_GRP_CALL) or i.group(capstone.CS_GRP_JUMP) for i in decoded))
            stores = [i.operands[0].mem.disp for i in decoded if i.mnemonic=='mov'
                      and i.operands[0].type==capstone.x86.X86_OP_MEM and not i.operands[0].mem.base]
            self.assertEqual(stores,[target,target+4,target+8])
        compiler = Path(os.environ['VC6_ROOT']).resolve()
        linker = find_child_ci(find_vc6_bin(compiler), 'link.exe')
        libraries = find_child_ci(infer_vc98_root(compiler), 'lib')
        with tempfile.TemporaryDirectory(prefix='mcm2-follow-camera-tick-') as directory:
            output = Path(directory)
            objects = []
            compiled = {}
            for source in ('samples/camera/FollowCameraTick.cpp', 'samples/camera/FollowCameraNearMisses.cpp',
                           'src/reconstructed/FastMath.cpp',
                           'tests/fixtures/follow_camera_tick_compare.cpp'):
                obj = output / (Path(source).stem + '.obj')
                result = subprocess.run([sys.executable, str(ROOT / 'tools/compile.py'), str(ROOT / source),
                    '-o', str(obj), '--vc6-root', str(compiler), '--profile', 'vc6_o2_mt',
                    '--include', str(ROOT / 'src/krusty2'),
                    # VC6's optimizer ICEs on the fixture's large main callback
                    # table/loop. Candidate and real math providers retain /O2.
                    *(['--extra','/Od'] if source.startswith('tests/') else [])], cwd=ROOT, capture_output=True,
                    text=True, encoding='latin-1', env={**os.environ, 'PYTHONPATH': str(ROOT)})
                self.assertEqual(result.returncode, 0, result.stdout + result.stderr)
                if not source.endswith('FollowCameraNearMisses.cpp'):
                    objects.append(str(obj))
                candidate = CoffObject(obj)
                compiled[source] = candidate
                if source.endswith('FollowCameraTick.cpp'):
                    entry = candidate.find_symbol(SYMBOL)
                    self.assertEqual(entry.extent_source, 'coff_function_aux')
                    self.assertEqual(entry.function_size, 3672)
                    report = compare(image, candidate, SYMBOL, 0x465c20, 3672,
                                     ROOT / 'samples/camera/FollowCameraTick.bindings.json')
                    self.assertEqual(report['strict_match']['matching_positions'],3659)
                    self.assertEqual([m['offset'] for m in report['strict_match']['mismatches']],
                                     [331,332,333,334,907,908,909,910,911,912,913,914,915])
                    binding_path = ROOT/'samples/camera/FollowCameraTick.bindings.json'
                    bindings = json.loads(binding_path.read_text())
                    for name, address, size in (('_$E2',0x466f60,5),('_$E1',0x466f70,60),
                                                ('_$E5',0x466ff0,5),('_$E4',0x467000,60)):
                        entry = candidate.find_symbol(name)
                        self.assertEqual(entry.extent_source,'codeview_proc')
                        self.assertEqual(candidate.symbol_extent(entry)[1],size)
                        self.assertTrue(compare(image,candidate,name,address,size,binding_path)['strict_exact'])
                    crt = [s for s in candidate.sections if s.name=='.CRT$XCU']
                    self.assertEqual(len(crt),1)
                    self.assertEqual(crt[0].raw_size,8)
                    self.assertEqual([candidate.symbol_by_index[r.symbol_index].name for r in candidate.relocations
                                      if r.section_number==crt[0].index],['_$E2','_$E5'])
                    for entry in candidate.symbols:
                        if entry.name.startswith('__real@4@'):
                            section = candidate.section(entry.section_number)
                            self.assertFalse(section.characteristics & 0x80000000)
                            start = section.raw_ptr+entry.value
                            self.assertEqual(candidate.data[start:start+4],image.bytes_at_va(int(bindings[entry.name],0),4))
                elif source.endswith('FastMath.cpp'):
                    for symbol, address in (('?FastSqrt@@YAMM@Z',0x460b50),('?InitFastSqrtTable@@YAXXZ',0x460ae0)):
                        size = candidate.find_symbol(symbol).function_size
                        report = compare(image,candidate,symbol,address,size,ROOT/'src/reconstructed/FastMath.bindings.json')
                        self.assertTrue(report['strict_exact'],report)
            def complete(coff):
                entry = coff.find_symbol(SYMBOL)
                body,size,relocations = coff.symbol_extent(entry)
                return body,size,[(r.virtual_address-entry.value,r.type,coff.symbol_by_index[r.symbol_index].name)
                                  for r in relocations]
            self.assertEqual(complete(compiled['samples/camera/FollowCameraTick.cpp']),
                             complete(compiled['samples/camera/FollowCameraNearMisses.cpp']))
            patch_file = output / 'calls.bin'
            patch_file.write_bytes(b''.join(struct.pack('<II',a,b) for a,b in calls.items()))
            executable = output / 'camera.exe'
            result = subprocess.run([str(linker), '/nologo', '/subsystem:console', '/base:0x100000',
                '/fixed', '/opt:ref', f'/out:{executable}', f'/libpath:{libraries}',
                *objects, 'kernel32.lib'], cwd=ROOT, capture_output=True, text=True, encoding='latin-1')
            self.assertEqual(result.returncode, 0, result.stdout + result.stderr)
            result = subprocess.run([str(executable),str(reference),str(patch_file)],capture_output=True,
                                    text=True,timeout=180)
            self.assertEqual(result.returncode,0,result.stdout+result.stderr)
            self.assertIn('FollowCameraTick: 674335 full-state, callback, allocation and floating-point scenarios match',result.stdout)
            print(result.stdout.strip())


if __name__ == '__main__':
    unittest.main()
