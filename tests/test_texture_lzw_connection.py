"""Texture decoding calls the existing LZW provider with unchanged complete objects."""
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
from tools.match import compare
from tools.run_calibration import CASES

ROOT = Path(__file__).resolve().parents[1]
SOURCES = ('src/reconstructed/PCTextureMap.cpp', 'samples/render/PCTextureMapNearMisses.cpp',
           'src/reconstructed/ProCircuitProcs.cpp')
PROVIDER = '?ExpandLzw@@YAXPAEPBXH@Z'
RETIRED = '?ExpandLzw@@YAXPAX0H@Z'


def whole_object(obj):
    """Keep all nondebug contents; normalize only the checkout prefix of source paths."""
    sections = [s for s in obj.sections if not s.name.startswith('.debug')]
    prefix = str(ROOT).replace('\\', '/') + '/'
    # The near-miss aggregate's literal names its included TextureClone.cpp.
    filenames = (*SOURCES, 'samples/render/TextureClone.cpp')

    def filename(data):
        value, nul, tail = data.partition(b'\0')
        path = posixpath.normpath(value.decode('latin1').replace('\\', '/'))
        if nul and path.lower().startswith(prefix.lower()) and path[len(prefix):] in filenames:
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
                     'requires authentic native VC6 and private retail')
class TextureLzwConnectionTests(unittest.TestCase):
    def test_complete_loader_objects_two_calls_and_existing_decoder(self):
        import capstone
        image = Path(os.environ['MCM2_EXE']).resolve()
        self.assertEqual(hashlib.sha256(image.read_bytes()).hexdigest(),
                         '31fde4cc686a5ee89ef9095b90235325b195596867ecacefe511263e1509b874')
        retail = PEImage(image)
        decoder = capstone.Cs(capstone.CS_ARCH_X86, capstone.CS_MODE_32)
        # Both sites push the decoded byte count, input and output. The 16-byte
        # cleanup also removes BytesPerPixel's earlier argument. EAX is ignored.
        code = {i.address: (i.mnemonic, i.op_str) for i in
                decoder.disasm(retail.bytes_at_va(0x4c6080, 2129), 0x4c6080)}
        for va, expected in {
            0x4c63bb: ('call', '0x511740'), 0x4c63c0: ('imul', 'eax, ebp'),
            0x4c63c3: ('imul', 'eax, esi'), 0x4c63ca: ('push', 'eax'),
            0x4c63cf: ('push', 'edx'), 0x4c63d0: ('push', 'eax'),
            0x4c63d1: ('call', '0x4a03d0'), 0x4c63d6: ('add', 'esp, 0x10'),
            0x4c6701: ('call', '0x511740'), 0x4c6706: ('imul', 'eax, esi'),
            0x4c6709: ('imul', 'eax, ebx'), 0x4c6714: ('push', 'eax'),
            0x4c6715: ('push', 'ecx'), 0x4c6716: ('push', 'edx'),
            0x4c6717: ('call', '0x4a03d0'), 0x4c671c: ('add', 'esp, 0x10'),
        }.items():
            self.assertEqual(code[va], expected)
        self.assertEqual(struct.unpack('<I', retail.bytes_at_va(0x555fec + 5*4, 4))[0], 0x4c6080)
        pins = json.loads((ROOT/'tests/fixtures/texture_lzw_objects.json').read_text())
        strict = 0
        with tempfile.TemporaryDirectory(prefix='mcm2-texture-lzw-') as directory:
            objects = {}
            for i, source in enumerate((*SOURCES, 'src/reconstructed/Lzw.cpp')):
                obj = Path(directory)/(str(i)+'.obj')
                run = subprocess.run([sys.executable, str(ROOT/'tools/compile.py'), str(ROOT/source),
                    '-o', str(obj), '--vc6-root', os.environ['VC6_ROOT'], '--profile', 'vc6_o2_mt',
                    '--include', str(ROOT/'src/krusty2'), '--include', str(ROOT/'src/reconstructed')],
                    cwd=ROOT, capture_output=True, text=True, encoding='latin1',
                    env={**os.environ, 'PYTHONPATH': str(ROOT)})
                self.assertEqual(run.returncode, 0, run.stdout+run.stderr)
                objects[source] = coff = CoffObject(obj)
                if source in SOURCES:
                    rows = whole_object(coff)
                    self.assertEqual(len(rows), pins[source]['sections'])
                    self.assertEqual(sum(len(r[3]) for r in rows), pins[source]['relocations'])
                    self.assertEqual(hashlib.sha256(json.dumps(rows).encode()).hexdigest(), pins[source]['sha256'])
                for case in CASES:
                    if case['source'] != source or source == SOURCES[2]:
                        continue
                    proof = compare(retail, coff, case['symbol'], int(case['target_va'], 0),
                                    case['target_size'], ROOT/case['bindings'])
                    self.assertTrue(proof['strict_exact'], proof)
                    strict += 1
            caller = objects[SOURCES[0]]
            entry = caller.find_symbol('?LoadSurface@PCTextureMap@@')
            _, size, relocations = caller.symbol_extent(entry)
            self.assertEqual(size, 2129)
            self.assertFalse(any(s.name == RETIRED for obj in objects.values() for s in obj.symbols))
            self.assertEqual([(r.virtual_address-entry.value, r.type) for r in relocations
                              if caller.symbol_by_index[r.symbol_index].name == PROVIDER],
                             [(0x4c63d2-0x4c6080, 20), (0x4c6718-0x4c6080, 20)])
            provider = objects['src/reconstructed/Lzw.cpp'].find_symbol(PROVIDER)
            self.assertGreater(provider.section_number, 0)
            self.assertEqual(provider.function_size, 522)
        self.assertEqual(strict, 38)
        print('38 strict checks; 498 complete sections including the transitive menu consumer; both texture paths import the real LZW provider')


if __name__ == '__main__':
    unittest.main()
