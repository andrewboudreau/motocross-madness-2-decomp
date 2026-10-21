"""The vehicle's existing binding owner reaches the primary InputDevice getter."""
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
from mcm2tool.rtti import parse_rtti
from mcm2tool.toolchain import find_child_ci, find_vc6_bin, infer_vc98_root
from tools.match import compare
import test_eco_quadtree_connections as views

ROOT = Path(__file__).resolve().parents[1]
GETTER = '?IsDeviceSubtype@InputDevice@@QAEHH@Z'
OLD = '?Method_004897E0@VehicleAxisSource@@QAEHH@Z'
VEHICLE = 'src/krusty2/vehicle/Vehicle.cpp'
PROVIDER = 'src/reconstructed/InputDevice.cpp'


@unittest.skipUnless(os.name == 'nt' and os.environ.get('VC6_ROOT') and os.environ.get('MCM2_EXE'),
                     'authentic VC6 and private retail required')
class VehicleInputSubtypeTests(unittest.TestCase):
    def test_complete_objects_primary_binding_chain_and_actual_getter(self):
        import capstone
        image = Path(os.environ['MCM2_EXE'])
        self.assertEqual(hashlib.sha256(image.read_bytes()).hexdigest(),
                         '31fde4cc686a5ee89ef9095b90235325b195596867ecacefe511263e1509b874')
        pe = PEImage(image)
        pins = json.loads((ROOT/'tests/fixtures/vehicle_input_objects.json').read_text())
        with tempfile.TemporaryDirectory(prefix='mcm2-input-subtype-') as directory:
            out = Path(directory)
            compiled, paths = {}, {}
            compiler = Path(os.environ['VC6_ROOT'])

            def compile_source(source):
                if source in compiled:
                    return compiled[source]
                path = Path(source)
                if not path.is_absolute():
                    path = ROOT/source
                obj = out/('%d.obj' % len(compiled))
                run = subprocess.run([sys.executable, str(ROOT/'tools/compile.py'), str(path), '-o', str(obj),
                    '--vc6-root', str(compiler), '--profile', 'vc6_o2_mt',
                    '--include', str(ROOT/'src/krusty2'), '--include', str(ROOT/'src/reconstructed')],
                    capture_output=True, text=True, encoding='latin1', env={**os.environ, 'PYTHONPATH': str(ROOT)})
                self.assertEqual(run.returncode, 0, run.stdout+run.stderr)
                compiled[source], paths[source] = CoffObject(obj), obj
                return compiled[source]

            for source, pin in pins['objects'].items():
                obj = compile_source(source)
                with patch.object(views, 'FILENAMES', tuple(pins['filename_paths'])):
                    rows = views.whole_object(obj, (ROOT,))
                self.assertEqual(dict(sections=len(rows), relocations=sum(len(row[-1]) for row in rows),
                    sha256=hashlib.sha256(json.dumps(rows, separators=(',', ':')).encode()).hexdigest()), pin, source)
            strict = 0
            for row in pins['strict_cases']:
                self.assertTrue(compare(pe, compile_source(row['source']), row['symbol'],
                    int(row['va'], 16), row['extent'], ROOT/row['bindings'])['strict_exact'], row)
                strict += 1
            self.assertEqual(strict, 233)
            caller = compiled[VEHICLE]
            self.assertFalse(any(s.name == OLD for s in caller.symbols))
            call = caller.find_symbol('?UnknownVirtualSlot63@Vehicle@@')
            body, size, relocs = caller.symbol_extent(call)
            self.assertEqual(size, 209)
            self.assertEqual([(r.virtual_address-call.value, r.type) for r in relocs
                if caller.symbol_by_index[r.symbol_index].name == GETTER], [(116, 20)])
            getter = compiled[PROVIDER].find_symbol(GETTER)
            raw, size, relocs = compiled[PROVIDER].symbol_extent(getter)
            self.assertEqual(size, 17)
            self.assertFalse(relocs)
            self.assertEqual(raw[-3:], b'\xc2\x04\x00')

            decoder = capstone.Cs(capstone.CS_ARCH_X86, capstone.CS_MODE_32)
            for address, mnemonic, operands in pins['instructions']:
                instruction = next(decoder.disasm(pe.bytes_at_va(int(address,16), 16), int(address,16)))
                self.assertEqual((instruction.mnemonic, instruction.op_str), (mnemonic, operands))

            def pushed_argument(push, call):
                code = list(decoder.disasm(pe.bytes_at_va(push,call-push),push))
                self.assertEqual(sum(x.size for x in code),call-push)
                self.assertEqual(code[0].mnemonic,'push')
                displacement = 0
                for instruction in code[1:]:
                    if instruction.mnemonic == 'push':
                        displacement += 4
                    elif instruction.mnemonic == 'sub' and instruction.op_str.startswith('esp, '):
                        displacement += int(instruction.op_str[5:],0)
                    else:
                        self.assertNotIn(instruction.mnemonic,('pop','call','ret'))
                        self.assertFalse(instruction.op_str.startswith('esp, '))
                return displacement+4

            self.assertEqual([pushed_argument(p,0x490061) for p in (0x48ff56,0x48ff55,0x48ff49)],
                             [0x8c,0x90,0x94])
            self.assertEqual([pushed_argument(p,0x407b21) for p in (0x407a35,0x407a2d,0x407a25)],
                             [0x84,0x88,0x8c])
            def incoming_argument(entry, load):
                code = list(decoder.disasm(pe.bytes_at_va(entry,load-entry+8),entry))
                depth, depths, branches = 0, {}, []
                for instruction in code:
                    depths[instruction.address] = depth
                    if instruction.address == load:
                        self.assertTrue(instruction.op_str.endswith(']'))
                        offset = int(instruction.op_str.split('[esp + ')[1][:-1],0)
                        for origin, target, branch_depth in branches:
                            self.assertIn(target,depths)
                            self.assertEqual(depths[target],branch_depth,(hex(origin),hex(target)))
                        return offset-depth
                    if instruction.mnemonic == 'push':
                        depth += 4
                    elif instruction.mnemonic == 'sub' and instruction.op_str.startswith('esp, '):
                        depth += int(instruction.op_str[5:],0)
                    else:
                        self.assertNotIn(instruction.mnemonic,('pop','call','ret'))
                        self.assertFalse(instruction.op_str.startswith('esp, '))
                        if instruction.mnemonic.startswith('j'):
                            branches.append((instruction.address,int(instruction.op_str,0),depth))
                self.fail('argument load not decoded')

            self.assertEqual([incoming_argument(0x4079c0,p) for p in (0x407a0e,0x407a16,0x407a1e)],
                             [0x94,0x90,0x8c])
            self.assertEqual([incoming_argument(0x525e20,p) for p in (0x525e3c,0x525e49,0x525e57)],
                             [0x88,0x84,0x8c])
            classes = {c.name:c for c in parse_rtti(pe)}
            for row in pins['rtti']:
                col, table, type_va = (int(row[k],16) for k in ('col','vtable','type'))
                self.assertEqual(struct.unpack('<I',pe.bytes_at_va(table-4,4))[0],col)
                words = struct.unpack('<5I',pe.bytes_at_va(col,20))
                self.assertEqual((words[1],words[3]),(0,type_va))
                self.assertEqual(pe.bytes_at_va(type_va+8,80).split(b'\0')[0],
                                 ('.?AV'+row['name']+'@@').encode())
                self.assertTrue(any(b.name=='InputDevice' and b.mdisp==0 and b.pdisp==-1
                                    for b in classes[row['name']].bases))
            layout = out/'layout.cpp'
            layout.write_text('#include "vehicle/Vehicle.h"\n#include "InputDevice.h"\n'
                '#include "ControlInterface.h"\n#include <stddef.h>\n'
                'typedef char L[(offsetof(VehicleAxis,axisSource)==0 && '
                'offsetof(UnknownControlBinding,owner)==0 && offsetof(VehicleAxis,axisValue)==0x24 && '
                'offsetof(UnknownControlBinding,currentValue)==0x24 && offsetof(VehicleAxisSource,kindCode)==0xc && '
                'offsetof(Vehicle,steerAxis)==0x4f8 && offsetof(Vehicle,leanAxis)==0x4fc && '
                'offsetof(Vehicle,throttleAxis)==0x500)?1:-1];\n')
            compile_source(str(layout))
            track = out/'track.cpp'
            track.write_text('#include "TrackGame.h"\n#include <stddef.h>\n'
                'typedef char L[(offsetof(TrackGame,steerBinding)==0x3344 && '
                'offsetof(TrackGame,leanBinding)==0x3380 && offsetof(TrackGame,throttleBinding)==0x33bc && '
                'offsetof(ControlInterface,activeJoystick)==0xc && offsetof(ControlInterface,mouse)==0x30 && '
                'offsetof(ControlInterface,keyboard)==0x34)?1:-1];\n')
            compile_source(str(track))
            fixture = 'tests/fixtures/vehicle_input_subtype_compare.cpp'
            compile_source(fixture)
            exe = out/'subtype.exe'
            link = find_child_ci(find_vc6_bin(compiler),'link.exe')
            library = find_child_ci(infer_vc98_root(compiler),'lib')
            run = subprocess.run([str(link),'/nologo','/subsystem:console','/base:0x100000','/fixed',
                '/opt:ref','/out:'+str(exe),'/libpath:'+str(library),str(paths[PROVIDER]),str(paths[fixture]),
                'kernel32.lib'],capture_output=True,text=True,encoding='latin1')
            self.assertEqual(run.returncode,0,run.stdout+run.stderr)
            run = subprocess.run([str(exe),str(image)],capture_output=True,text=True,timeout=30)
            self.assertEqual(run.returncode,0,run.stdout+run.stderr)
            self.assertEqual(run.stdout.strip(),'Input subtype: 11264 cases, 3072 modeled read faults')
        print('233 strict checks; ten complete objects; one caller join; '+run.stdout.strip())


if __name__ == '__main__':
    unittest.main()
