"""Scene restart keeps complete callers and executes the existing motion providers."""
from collections import Counter
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
SCENE = 'src/reconstructed/SceneManager.cpp'
SAMPLE = 'samples/track/SceneManagerNearMisses.cpp'
MOTION = 'src/krusty2/motion/Motnctrl.cpp'
FIXTURE = 'tests/fixtures/scene_motion_compare.cpp'
RENAMES = {
    '?SetMotion@UnknownSceneObject@@QAEXPAX@Z': '?SetMotion@Character@@QAEXPAUMotion@@@Z',
    '?AdvanceMotion@UnknownSceneObject@@QAEHMHH@Z': '?AdvanceMotion@Character@@QAEHMHH@Z',
}


def object_fingerprint(obj, root, renames=None):
    """Retain every nondebug byte and destination, normalizing only file prefixes.

    Local labels resolve to the unchanged nondebug section and exact offset.
    A complete __FILE__ string and its generated string symbol are checkout-relative;
    no arbitrary strings, executable bytes or relocation addends are replaced.
    """
    renames = renames or {}
    prefix = str(root).replace('\\', '/') + '/'
    def filename(data):
        value, separator, tail = data.partition(b'\0')
        name = value.decode('latin1').replace('\\', '/')
        if separator and name.lower().startswith(prefix.lower()):
            suffix = name[len(prefix):]
            if suffix in (SCENE, SAMPLE, 'src/reconstructed/ContainerList.h'):
                return ('source:' + suffix).encode() + separator + tail
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
    return {'sections': len(rows), 'relocations': sum(len(row[4]) for row in rows),
            'sha256': hashlib.sha256(json.dumps(rows).encode()).hexdigest()}


@unittest.skipUnless(os.name == 'nt' and os.environ.get('VC6_ROOT') and os.environ.get('MCM2_EXE'),
                     'requires authentic native VC6 and the private retail image')
class SceneMotionConnectionsTests(unittest.TestCase):
    def test_complete_callers_producers_and_native_restart(self):
        import capstone
        from tools.run_calibration import CASES
        image = Path(os.environ['MCM2_EXE']).resolve()
        self.assertEqual(hashlib.sha256(image.read_bytes()).hexdigest(),
                         '31fde4cc686a5ee89ef9095b90235325b195596867ecacefe511263e1509b874')
        retail = PEImage(image)
        word = lambda va: struct.unpack('<I', retail.bytes_at_va(va, 4))[0]
        for table, col, offset in ((0x55110c, 0x55af38, 0), (0x55109c, 0x55aeb0, 0x268)):
            self.assertEqual(word(table - 4), col)
            self.assertEqual(word(col + 4), offset)
            self.assertEqual(retail.bytes_at_va(word(col + 12) + 8, 40).split(b'\0')[0],
                             b'.?AVCollisionCharacter@@')
        character = next(c for c in parse_rtti(retail) if c.name == 'CollisionCharacter')
        self.assertEqual([(b.name, b.mdisp, b.pdisp, b.vdisp) for b in character.bases],
                         [('CollisionCharacter', 0, -1, 0), ('D3DIMSoultreeCharacter', 0, -1, 0),
                          ('Character', 0, -1, 0), ('GameObject', 0, 4, 4), ('BaseObject', 0, 4, 4)])
        decoder = capstone.Cs(capstone.CS_ARCH_X86, capstone.CS_MODE_32)
        def decoded(va, size):
            instructions = list(decoder.disasm(retail.bytes_at_va(va, size), va))
            self.assertEqual(sum(i.size for i in instructions), size)
            return {i.address: (i.mnemonic, i.op_str) for i in instructions}
        producer = decoded(0x4edfe0, 5335)
        # The animation reader stores the constructor's primary result in entry+4,
        # then obtains the actual Motion pointer from that same receiver.
        for va, expected in {
            0x4ee6de: ('call', '0x4318d0'),
            0x4ee704: ('mov', 'dword ptr [ecx + ebx + 4], eax'),
            0x4ee726: ('call', '0x4319c0'),
            0x4eeaee: ('call', '0x4a6b30'),
            0x4eeb00: ('mov', 'dword ptr [edx + esi*4], eax'),
        }.items():
            self.assertEqual(producer[va], expected)
        caller = decoded(0x4eb040, 280)
        for va, expected in {
            0x4eb08e: ('mov', 'edi, dword ptr [eax + 4]'),
            0x4eb0ee: ('push', 'ecx'), 0x4eb0ef: ('mov', 'ecx, edi'),
            0x4eb0f1: ('call', '0x4a8b40'), 0x4eb0fa: ('push', '0'),
            0x4eb0fc: ('push', '0'), 0x4eb0fe: ('push', 'edx'),
            0x4eb0ff: ('mov', 'ecx, edi'),
            0x4eb101: ('mov', 'dword ptr [edi + 0x10], 0'),
            0x4eb108: ('call', '0x4a6bb0'),
            0x4eb117: ('mov', 'dword ptr [edx + esi + 0x18], eax'),
        }.items():
            self.assertEqual(caller[va], expected)
        baseline = json.loads((ROOT/'tests/fixtures/scene_motion_objects.json').read_text())['objects']
        with tempfile.TemporaryDirectory(prefix='mcm2-scene-motion-') as directory:
            output = Path(directory)
            extracted = {
                'restart': '#include "SceneManager.h"\n#include "motion/D3DIMSoultreeCharacter.h"\n' +
                    extract(ROOT/SCENE, 'int Scene::RestartEntry('),
                'motion': '#include "motion/Motnctrl.h"\n' + '\n'.join(extract(ROOT/MOTION, start) for start in (
                    'int AdvanceLooping(', 'inline void ClampFloat(', 'inline float SmoothStep(',
                    'inline void BlendPose(', 'int Character::AdvanceMotion(', 'void Character::SetMotion(')),
            }
            math = ['src/reconstructed/' + name + '.cpp' for name in
                    ('VectorCalls', 'CrossProduct', 'Math3D', 'FastMath')]
            jobs = [(source, ROOT/source) for source in (SCENE, SAMPLE, MOTION, *math, FIXTURE)]
            for key, value in extracted.items():
                path = output/(key+'.cpp')
                path.write_text(value+'\n')
                jobs.append((key, path))
            paths, objects = {}, {}
            for index, (key, source) in enumerate(jobs):
                obj = output/('unit%d.obj' % index)
                result = subprocess.run([sys.executable, str(ROOT/'tools/compile.py'), str(source), '-o', str(obj),
                    '--vc6-root', os.environ['VC6_ROOT'], '--profile', 'vc6_o2_mt',
                    '--include', str(ROOT/'src/krusty2'), '--include', str(ROOT/'src/reconstructed')],
                    cwd=ROOT, capture_output=True, text=True, encoding='latin1',
                    env={**os.environ, 'PYTHONPATH': str(ROOT)})
                self.assertEqual(result.returncode, 0, result.stdout+result.stderr)
                paths[key], objects[key] = obj, CoffObject(obj)
            for source in (SCENE, SAMPLE):
                self.assertEqual(object_fingerprint(objects[source], ROOT), baseline[source], source)
            imports = {s.name for s in objects[SCENE].symbols if s.section_number == 0}
            self.assertFalse(set(RENAMES) & imports)
            self.assertTrue(set(RENAMES.values()) <= imports)
            strict = 0
            cases = [c for c in CASES if c['source'] == SCENE]
            self.assertEqual(len(cases), 51)
            for case in cases:
                result = compare(retail, objects[SCENE], case['symbol'], int(case['target_va'], 0),
                                 case['target_size'], ROOT/case['bindings'])
                self.assertTrue(result['strict_exact'], result)
                self.assertEqual(result['candidate_size'], case['target_size'])
                strict += 1
            for original, key, symbol, va, size in (
                (SCENE, 'restart', '?RestartEntry@Scene@@', 0x4eb040, 280),
                (MOTION, 'motion', '?SetMotion@Character@@', 0x4a8b40, 27),
                (MOTION, 'motion', '?AdvanceMotion@Character@@', 0x4a6bb0, 1293),
                (MOTION, 'motion', '?AdvanceLooping@@', 0x4a56d0, 121),
            ):
                for source in (original, key):
                    result = compare(retail, objects[source], symbol, va, size,
                                     (ROOT/original).with_suffix('.bindings.json'))
                    self.assertTrue(result['strict_exact'], result)
                    self.assertEqual(result['candidate_size'], size)
                    strict += 1
                self.assertEqual(complete(objects[original], symbol), complete(objects[key], symbol))
            entry = objects[SCENE].find_symbol('?RestartEntry@Scene@@')
            _, _, relocs = objects[SCENE].symbol_extent(entry)
            joined = [(0x4eb040+r.virtual_address-entry.value-1,
                       objects[SCENE].symbol_by_index[r.symbol_index].name)
                      for r in relocs if r.type == 20 and
                      objects[SCENE].symbol_by_index[r.symbol_index].name in RENAMES.values()]
            self.assertEqual(joined, list(zip((0x4eb0f1, 0x4eb108), RENAMES.values())))
            # This is the sole reference patch: original AdvanceLooping's CRT
            # conversion calls the authentic VC6 _ftol in the test executable.
            self.assertEqual(decoded(0x4a56d0, 121)[0x4a56ed], ('call', '0x534a88'))
            compiler = Path(os.environ['VC6_ROOT'])
            executable = output/'scene-motion.exe'
            result = subprocess.run([str(find_child_ci(find_vc6_bin(compiler), 'link.exe')), '/nologo',
                '/subsystem:console', '/base:0x100000', '/fixed', '/out:'+str(executable),
                '/libpath:'+str(find_child_ci(infer_vc98_root(compiler), 'lib')),
                *(str(paths[key]) for key in ('restart', 'motion', *math, FIXTURE)), 'kernel32.lib'],
                cwd=ROOT, capture_output=True, text=True, encoding='latin1')
            self.assertEqual(result.returncode, 0, result.stdout+result.stderr)
            result = subprocess.run([str(executable), str(image)], capture_output=True, text=True, timeout=60)
            self.assertEqual(result.returncode, 0, result.stdout+result.stderr)
            self.assertEqual(result.stdout.strip(), 'Scene motion: 62215 state, frame, callback and fault cases passed')
            print('%d strict checks; two complete objects; two primary calls; %s' % (strict, result.stdout.strip()))


if __name__ == '__main__':
    unittest.main()
