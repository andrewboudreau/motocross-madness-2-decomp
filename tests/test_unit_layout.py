"""Unit layout: LINK-style placement of one object's sections against retail.

The synthetic object holds no compiler or retail bytes: two EH functions, a
pick-any COMDAT bound elsewhere, their .text$x stubs and .xdata$x tables. The
VC6 cases (enabled by VC6_ROOT and MCM2_EXE) recompile two retail units and
require them to lay out byte-identically.
"""
from __future__ import annotations

import contextlib
import io
import json
import os
from pathlib import Path
import struct
import sys
import tempfile
import unittest

ROOT = Path(__file__).resolve().parents[1]
sys.path.insert(0, str(ROOT))
sys.path.insert(0, str(ROOT / 'tools'))

from mcm2tool.coff import CoffObject  # noqa: E402
import unit_layout  # noqa: E402

TEXT = 0x60501020       # code, COMDAT, align 16, execute/read
DATA8 = 0x40401040      # initialized data, COMDAT, align 8, read
DIR32, REL32 = 0x0006, 0x0014


def build_object(path: Path) -> None:
    """Sections: 1 fA (nodup), 2 fB (pick-any), 3 .text$x(1), 4 .xdata$x(1),
    5 fC (nodup, calls fA and fB), 6 .text$x(5), 7 .xdata$x(5)."""
    strings = bytearray(4)

    def name8(name: str) -> bytes:
        raw = name.encode()
        if len(raw) <= 8:
            return raw.ljust(8, b'\0')
        strings.extend(raw + b'\0')
        return struct.pack('<II', 0, len(strings) - len(raw) - 1)

    # (name, characteristics, data, selection, associated, relocations[(offset, symbol, type)])
    sections = [
        ('.text', TEXT, bytes.fromhex('558bec6aff68') + b'\0' * 4 + b'\xc3' + b'\x90' * 5, 1, 0,
         [(6, 'LA', DIR32)]),
        ('.text', TEXT, bytes.fromhex('8b4104c3') + b'\x90' * 12, 2, 0, []),
        ('.text$x', TEXT, b'\xb8' + b'\0' * 4 + b'\xe9' + b'\0' * 4, 5, 1,
         [(1, 'TA', DIR32), (6, '___CxxFrameHandler', REL32)]),
        ('.xdata$x', DATA8, bytes.fromhex('20059319') + b'\0' * 4, 5, 1, [(4, 'LA', DIR32)]),
        ('.text', TEXT, b'\xe8' + b'\0' * 4 + b'\xe8' + b'\0' * 4 + b'\xc3' + b'\x90' * 5, 1, 0,
         [(1, 'fA', REL32), (6, 'fB', REL32)]),
        ('.text$x', TEXT, b'\xb8' + b'\0' * 4 + b'\xe9' + b'\0' * 4, 5, 5,
         [(1, 'TC', DIR32), (6, '___CxxFrameHandler', REL32)]),
        ('.xdata$x', DATA8, bytes.fromhex('20059319') + b'\0' * 4, 5, 5, [(4, 'LC', DIR32)]),
    ]
    defined = {'fA': (1, 2), 'fB': (2, 2), 'LA': (3, 6), 'TA': (4, 6), 'fC': (5, 2), 'LC': (6, 6), 'TC': (7, 6)}
    symbols, index = bytearray(), {}
    for number, (name, _, data, selection, associated, relocs) in enumerate(sections, 1):
        symbols += name8(name) + struct.pack('<IhHBB', 0, number, 0, 3, 1)
        symbols += struct.pack('<IHHIHB3x', len(data), len(relocs), 0, 0, associated, selection)
        for sym, (section, storage) in defined.items():
            if section == number:
                index[sym] = len(symbols) // 18
                symbols += name8(sym) + struct.pack('<IhHBB', 0, number, 0x20 if storage == 2 else 0, storage, 0)
    index['___CxxFrameHandler'] = len(symbols) // 18
    symbols += name8('___CxxFrameHandler') + struct.pack('<IhHBB', 0, 0, 0x20, 2, 0)

    header_size = 20 + 40 * len(sections)
    body, headers, offset = bytearray(), bytearray(), header_size
    for name, characteristics, data, _, _, relocs in sections:
        raw_ptr = offset
        body += data
        offset += len(data)
        reloc_ptr = offset if relocs else 0
        for at, sym, kind in relocs:
            body += struct.pack('<IIH', at, index[sym], kind)
            offset += 10
        headers += name8(name) + struct.pack('<IIIIIIHHI', 0, 0, len(data), raw_ptr, reloc_ptr, 0,
                                             len(relocs), 0, characteristics)
    struct.pack_into('<I', strings, 0, len(strings))
    header = struct.pack('<HHIIIHH', 0x14c, len(sections), 0, offset, len(symbols) // 18, 0, 0)
    path.write_bytes(header + headers + body + symbols + strings)


BINDINGS = {'fA': 0x401000, 'fB': 0x402000, 'fA$ehhandler': 0x500000, '___CxxFrameHandler': 0x700000}


def retail_image(fill=0xCC) -> dict[int, int]:
    image = {}

    def put(va, data):
        image.update({va + i: b for i, b in enumerate(data)})

    rel = lambda target, at: struct.pack('<i', target - (at + 4))
    put(0x401000, bytes.fromhex('558bec6aff68') + struct.pack('<I', 0x500000) + b'\xc3' + b'\x90' * 5)
    put(0x401010, b'\xe8' + rel(0x401000, 0x401011) + b'\xe8' + rel(0x402000, 0x401016) + b'\xc3' + b'\x90' * 5)
    put(0x500000, b'\xb8' + struct.pack('<I', 0x600000) + b'\xe9' + rel(0x700000, 0x500006))
    put(0x50000a, bytes([fill]) * 6)
    put(0x500010, b'\xb8' + struct.pack('<I', 0x600008) + b'\xe9' + rel(0x700000, 0x500016))
    put(0x600000, bytes.fromhex('20059319') + struct.pack('<I', 0x500000))
    put(0x600008, bytes.fromhex('20059319') + struct.pack('<I', 0x500010))
    return image


def reader(image):
    return lambda va, size: bytes(image.get(va + i, 0) for i in range(size))


class SyntheticLayoutTest(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        cls.tmp = tempfile.TemporaryDirectory()
        path = Path(cls.tmp.name) / 'unit.obj'
        build_object(path)
        cls.obj = CoffObject(path)

    @classmethod
    def tearDownClass(cls):
        cls.tmp.cleanup()

    def layout(self, image=None, bindings=BINDINGS, **kwargs):
        return unit_layout.run(self.obj, dict(bindings), reader(image or retail_image()),
                               'auto', 'auto', 'auto', fallback={}, **kwargs)

    def test_comdat_records(self):
        info = unit_layout.comdat_info(self.obj)
        self.assertEqual(info[2], (2, 0))
        self.assertEqual(info[6], (5, 5))

    def test_strict_identical_layout(self):
        report = self.layout()
        groups = report['groups']
        self.assertEqual(groups['.text']['verdict'], 'strict_identical')
        self.assertEqual((groups['.text']['start'], groups['.text']['end']), ('0x00401000', '0x00401020'))
        self.assertEqual(groups['.text$x']['verdict'], 'strict_identical')
        self.assertEqual(groups['.text$x']['fill_bytes_checked'], 6)
        self.assertEqual(groups['.xdata$x']['start'], '0x00600000')
        self.assertEqual(groups['.xdata$x']['verdict'], 'strict_identical')
        elsewhere = [r for r in report['sections'] if r['status'] == 'elsewhere']
        self.assertEqual([r['functions'] for r in elsewhere], [['fB']])
        self.assertEqual(report['functions_offset_agree'], 1)        # fA; fC is unbound
        self.assertEqual(report['functions_without_binding'], 1)
        self.assertEqual(report['functions_offset_disagree'], [])

    def test_link_fill_is_checked(self):
        report = self.layout(retail_image(fill=0x90))
        self.assertEqual(report['groups']['.text$x']['fill_bytes_differing'], 6)
        self.assertEqual(report['groups']['.text$x']['verdict'], 'differs')

    def test_byte_difference_is_reported(self):
        image = retail_image()
        image[0x401014] ^= 1
        report = self.layout(image)
        self.assertEqual(report['groups']['.text']['verdict'], 'differs')
        row = next(r for r in report['sections'] if r.get('va') == '0x00401010')
        self.assertEqual(row['first_differences'], ['0x00401014'])

    def test_unresolved_relocations_are_masked(self):
        bindings = {k: v for k, v in BINDINGS.items() if k != '___CxxFrameHandler'}
        report = self.layout(bindings=bindings)
        self.assertEqual(report['groups']['.text$x']['verdict'], 'masked_identical')
        self.assertEqual(report['groups']['.text$x']['masked_bytes'], 8)

    def test_order_disagreement(self):
        bindings = dict(BINDINGS, fC=0x401000, fA=0x401010)
        report = unit_layout.run(self.obj, bindings, reader(retail_image()), 0x401000, fallback={})
        self.assertEqual(sorted(report['functions_offset_disagree']), ['fA', 'fC'])

    def test_drop_takes_associated_sections(self):
        report = self.layout(drop={'fC'})
        dropped = {r['section'] for r in report['sections'] if r['status'] == 'dropped'}
        self.assertEqual(dropped, {5, 6, 7})
        self.assertEqual(report['groups']['.text$x']['sections'], 1)


VC6_ROOT = os.environ.get('VC6_ROOT')
MCM2_EXE = os.environ.get('MCM2_EXE', str(ROOT / 'work/game/mcm2.exe'))


@unittest.skipUnless(VC6_ROOT and Path(VC6_ROOT).is_dir() and Path(MCM2_EXE).is_file(),
                     'needs VC6_ROOT and the retail MCM2_EXE')
class RetailUnitLayoutTest(unittest.TestCase):
    """AgeManager.cpp opens retail .text at 0x00401000; ArcadeObject.cpp follows."""

    def check(self, name, extra=()):
        with tempfile.TemporaryDirectory() as tmp:
            out = Path(tmp) / 'report.json'
            with contextlib.redirect_stdout(io.StringIO()):
                code = unit_layout.main(['--source', str(ROOT / 'src/reconstructed' / name), '--exe', MCM2_EXE,
                                         '--start', 'auto', '--text-x-start', 'auto', '--xdata-start', 'auto',
                                         '--vc6-root', VC6_ROOT, '--json', str(out), *extra])
            return code, json.loads(out.read_text())

    def test_age_manager(self):
        code, report = self.check('AgeManager.cpp')
        self.assertEqual(code, 0)
        self.assertEqual(report['groups']['.text']['start'], '0x00401000')
        self.assertEqual(report['groups']['.text']['end'], '0x00401260')
        self.assertEqual(report['groups']['.text']['verdict'], 'strict_identical')

    def test_arcade_object(self):
        code, report = self.check('ArcadeObject.cpp')
        self.assertEqual(code, 0)
        for group in ('.text', '.text$x', '.xdata$x'):
            self.assertEqual(report['groups'][group]['verdict'], 'strict_identical', group)
        self.assertEqual(report['groups']['.text$x']['start'], '0x005495d0')


if __name__ == '__main__':
    unittest.main()
