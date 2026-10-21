"""Six clipper declarations share a pointer import without inventing target storage."""
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
from tools.match import compare
from tools.run_calibration import CASES

ROOT = Path(__file__).resolve().parents[1]
CANONICAL = '?g_visibilityClipper@@3PAVVisibilityClipper@@A'
SOURCES = (
    'src/reconstructed/CubeDraw.cpp',
    'src/reconstructed/FollowCamera.cpp',
    'src/reconstructed/TrackOverlay.cpp',
    'src/reconstructed/EcoSystem.cpp',
    'src/reconstructed/Griddraw.cpp',
    'src/krusty2/shadow/D3DIMSoultreeShadow.cpp',
    'src/krusty2/visibility/VisibilityQuadTree.cpp',
    'samples/render/CubeDrawNearMisses.cpp',
    'samples/camera/FollowCameraNearMisses.cpp',
    'samples/track/TrackOverlayNearMisses.cpp',
    'samples/ecosystem/EcoSystemNearMisses.cpp',
    'samples/render/GriddrawNearMisses.cpp',
    'samples/physics/shadow/D3DIMSoultreeShadowNearMisses.cpp',
    'samples/physics/visibility/VisibilityQuadTreeTraversal.cpp',
    'src/krusty2/broadphase/Terrain.cpp',
)
FILENAMES = SOURCES + ('samples/camera/FollowCameraTick.cpp',
                       'samples/camera/FollowCameraLoadFile.cpp')

def view(obj, normalize_paths=()):
    """Complete sections and destinations, with known source paths made portable."""
    sections = [section for section in obj.sections if not section.name.startswith('.debug')]
    prefixes = [str(path).replace('\\', '/') + '/' for path in normalize_paths]

    def filename(data):
        value, separator, tail = data.partition(b'\0')
        text = value.decode('latin1').replace('\\', '/')
        for prefix in prefixes:
            if separator and text.lower().startswith(prefix.lower()) and text[len(prefix):] in FILENAMES:
                suffix = text[len(prefix):]
                return ('source:' + suffix).encode() + separator + tail, suffix
        return data, None

    def symbol_name(symbol):
        if symbol.name.startswith('??_C@') and symbol.section_number > 0:
            section = obj.sections[symbol.section_number - 1]
            data = obj.data[section.raw_ptr + symbol.value:section.raw_ptr + section.raw_size]
            normalized, suffix = filename(data)
            if suffix:
                return normalized.split(b'\0')[0].decode()
        return symbol.name

    keys, anonymous = {}, {}
    for section in sections:
        labels = tuple(sorted((symbol_name(symbol), symbol.value) for symbol in obj.symbols
                              if symbol.section_number == section.index and
                              (symbol.storage_class == 2 or symbol.type == 32)))
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

    rows, filenames = [], []
    for section in sections:
        data = (obj.data[section.raw_ptr:section.raw_ptr + section.raw_size]
                if section.raw_ptr else bytes(section.raw_size))
        data, suffix = filename(data)
        if suffix:
            filenames.append((keys[section.index], suffix))
        relocations = [(relocation.virtual_address, relocation.type,
                        target(obj.symbol_by_index[relocation.symbol_index]))
                       for relocation in obj.relocations if relocation.section_number == section.index]
        rows.append((keys[section.index], section.characteristics, data.hex(), relocations))
    return sorted(rows, key=lambda row: repr(row[0])), filenames


@unittest.skipUnless(os.name == 'nt' and os.environ.get('VC6_ROOT') and os.environ.get('MCM2_EXE'),
                     'requires authentic VC6 and the private retail image')
class ClipperGlobalConnectionsTests(unittest.TestCase):
    def test_shared_import_complete_objects_and_retail_pointer_reads(self):
        import capstone
        image = Path(os.environ['MCM2_EXE']).resolve()
        self.assertEqual(hashlib.sha256(image.read_bytes()).hexdigest(),
                         '31fde4cc686a5ee89ef9095b90235325b195596867ecacefe511263e1509b874')
        pe = PEImage(image)
        pins = json.loads((ROOT/'tests/fixtures/clipper_global_objects.json').read_text())
        references = json.loads((ROOT/'tests/fixtures/clipper_global_reads.json').read_text())
        decoder = capstone.Cs(capstone.CS_ARCH_X86, capstone.CS_MODE_32)
        # Search every byte, rather than stopping a linear sweep at inline data.
        offsets = []
        start = 0
        needle = struct.pack('<I', 0x575a98)
        while True:
            offset = pe.data.find(needle, start)
            if offset < 0: break
            offsets.append(pe.offset_to_va(offset))
            start = offset + 1
        self.assertEqual(offsets, [int(row['instruction'], 0) + 2 for row in references])
        self.assertEqual(len(offsets), 46)
        for row in references:
            address = int(row['instruction'], 0)
            self.assertEqual(pe.bytes_at_va(address, 6), b'\x8b\x0d' + needle)
            instruction = next(decoder.disasm(pe.bytes_at_va(address, 6), address))
            self.assertEqual((instruction.mnemonic, instruction.op_str),
                             ('mov', 'ecx, dword ptr [0x575a98]'))
            call_address, target = row['next_call']
            call_address = int(call_address, 0)
            instruction = next(decoder.disasm(pe.bytes_at_va(call_address, 5), call_address))
            self.assertEqual((instruction.mnemonic, instruction.op_str), ('call', target))
        self.assertEqual(pe.bytes_at_va(0x575a98, 4), struct.pack('<I', 0x68acd4))
        section = pe.section_for_rva(0x575a98 - pe.image_base)
        self.assertEqual(section.characteristics, 0xc0000040)
        target_section = pe.section_for_rva(0x68acd4 - pe.image_base)
        self.assertGreaterEqual(0x68acd4, pe.image_base + target_section.virtual_address + target_section.raw_size)
        self.assertLess(0x68acd4, pe.image_base + target_section.virtual_address + target_section.virtual_size)
        # These no-op initializer bytes do not encode a receiver or target size.
        # They remain evidence limits, not a basis for inventing singleton storage.
        self.assertEqual(pe.bytes_at_va(0x566570, 4), struct.pack('<I', 0x52f080))
        self.assertEqual(pe.bytes_at_va(0x52f080, 5), b'\xe9\x0b\0\0\0')
        self.assertEqual(pe.bytes_at_va(0x52f090, 1), b'\xc3')
        self.assertEqual(pe.directories[5], (0, 0))
        compiler = Path(os.environ['VC6_ROOT']).resolve()
        strict = 0
        with tempfile.TemporaryDirectory(prefix='mcm2-clipper-global-') as directory:
            objects = {}
            for index, source in enumerate(SOURCES):
                output = Path(directory)/('%d.obj' % index)
                result = subprocess.run([sys.executable, str(ROOT/'tools/compile.py'), str(ROOT/source),
                    '-o', str(output), '--vc6-root', str(compiler), '--profile', 'vc6_o2_mt',
                    '--include', str(ROOT/'src/krusty2')], cwd=ROOT, capture_output=True,
                    text=True, encoding='latin1', env={**os.environ, 'PYTHONPATH': str(ROOT)})
                self.assertEqual(result.returncode, 0, result.stdout + result.stderr)
                obj = objects[source] = CoffObject(output)
                rows, filenames = view(obj, (ROOT,))
                actual = dict(sections=len(rows), relocations=sum(len(row[3]) for row in rows),
                              sha256=hashlib.sha256(json.dumps(rows).encode()).hexdigest())
                self.assertEqual(actual, pins['objects'][source])
                self.assertTrue(all(suffix in FILENAMES for _, suffix in filenames))
                self.assertFalse(set(pins['renames']) & {symbol.name for symbol in obj.symbols})
                occurrences = [symbol for symbol in obj.symbols if symbol.name == CANONICAL]
                self.assertTrue(all(symbol.section_number == 0 and symbol.value == 0 for symbol in occurrences))
                imports = [r for r in obj.relocations
                           if not obj.sections[r.section_number - 1].name.startswith('.debug')
                           and obj.symbol_by_index[r.symbol_index].name == CANONICAL]
                self.assertEqual(len(imports), pins['pointer_imports'][source])
                for relocation in imports:
                    self.assertEqual(relocation.type, 6)  # Existing absolute DWORD load.
                    sec = obj.sections[relocation.section_number - 1]
                    payload = obj.data[sec.raw_ptr:sec.raw_ptr + sec.raw_size]
                    offset = relocation.virtual_address
                    self.assertEqual(payload[offset-2:offset+4], b'\x8b\x0d\0\0\0\0')
            cases = [(c['source'], c['symbol'], int(c['target_va'], 0), c['target_size'], ROOT/c['bindings'])
                     for c in CASES if c['source'] in SOURCES]
            for path in (ROOT/'src/krusty2').rglob('targets.json'):
                for target in json.loads(path.read_text()):
                    if not target.get('source') or target.get('expect') != 'exact': continue
                    source = (path.parent/target['source']).resolve().relative_to(ROOT).as_posix()
                    if source in SOURCES:
                        cases.append((source, target['candidate_symbol_contains'], int(target['target_va'], 0),
                                      target['target_size'], path.parent/target['bindings']))
            for source, symbol, va, size, bindings in dict.fromkeys(cases):
                report = compare(pe, objects[source], symbol, va, size, bindings)
                self.assertTrue(report['strict_exact'], (source, symbol, report))
                strict += 1
            # The actual frozen PCCamera emits one strict implicit destructor COMDAT.
            self.assertEqual(sum(row['sections'] for row in pins['objects'].values()), 1036)
            # The corrected camera loader adds its explicit first-cross call relocation.
            self.assertEqual(sum(row['relocations'] for row in pins['objects'].values()), 4714)
        print('Clipper global:', strict, 'strict checks; 15 whole objects; 46 retail pointer loads; storage remains unresolved')


if __name__ == '__main__': unittest.main()
