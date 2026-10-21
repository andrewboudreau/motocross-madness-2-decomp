"""Five visibility call joins retain full objects and execute actual helper closure."""
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
from test_collision_point_consumers import definition

ROOT = Path(__file__).resolve().parents[1]
SOURCES = (
    'src/reconstructed/CubeDraw.cpp',
    'src/reconstructed/FollowCamera.cpp',
    'src/reconstructed/TrackOverlay.cpp',
    'samples/render/CubeDrawNearMisses.cpp',
    'samples/camera/FollowCameraNearMisses.cpp',
    'samples/track/TrackOverlayNearMisses.cpp',
    'src/krusty2/visibility/VisibilityQuadTree.cpp',
)
PROVIDER = SOURCES[-1]
FILENAMES = SOURCES + ('samples/camera/FollowCameraTick.cpp',
                       'samples/camera/FollowCameraLoadFile.cpp')
METHODS = [('TransformVectors', 0x52f0a0, 156), ('TestDot', 0x52f140, 72),
           ('ProjectPoint', 0x52f340, 389), ('CullQuad', 0x52f4d0, 155),
           ('CullPolygon', 0x52fac0, 234)]

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


def signature(obj, query):
    entry = obj.find_symbol(query)
    body, size, relocations = obj.symbol_extent(entry)
    return body, size, [(r.virtual_address - entry.value, r.type,
                         obj.symbol_by_index[r.symbol_index].name) for r in relocations]


@unittest.skipUnless(os.name == 'nt' and os.environ.get('VC6_ROOT') and os.environ.get('MCM2_EXE'),
                     'requires authentic native VC6 and private retail')
class VisibilityConnectionsTests(unittest.TestCase):
    def test_complete_objects_camera_boundary_and_native_helpers(self):
        import capstone
        image = Path(os.environ['MCM2_EXE']).resolve()
        self.assertEqual(hashlib.sha256(image.read_bytes()).hexdigest(),
                         '31fde4cc686a5ee89ef9095b90235325b195596867ecacefe511263e1509b874')
        pe = PEImage(image)
        word = lambda va: struct.unpack('<I', pe.bytes_at_va(va, 4))[0]
        classes = {c.name: c for c in parse_rtti(pe)}
        for owner in ('Camera', 'PCCamera', 'FollowCamera'):
            cls = classes[owner]
            col = cls.complete_object_locator_va
            self.assertEqual(word(col + 4), 0)
            self.assertEqual(pe.bytes_at_va(word(col + 12) + 8, 40).split(b'\0')[0],
                             ('.?AV' + owner + '@@').encode())
            for table in cls.vtables:
                self.assertEqual(word(table - 4), col)
            if owner != 'Camera':
                relation = next(b for b in cls.bases if b.name == 'Camera')
                self.assertEqual((relation.mdisp, relation.pdisp), (0, -1))
        decoder = capstone.Cs(capstone.CS_ARCH_X86, capstone.CS_MODE_32)
        ranges = [(0x43e330, 398), (0x4636e0, 780), (0x5190e0, 654),
                  (0x42e340, 402), (0x4bed80, 25), (0x4e8cf0, 49), (0x42f190, 114)]
        decoded = {i.address: (i.mnemonic, i.op_str) for va, size in ranges
                   for i in decoder.disasm(pe.bytes_at_va(va, size), va)}
        for va, expected in {
            0x4bed88: ('call', '0x42e340'),
            0x4bed8d: ('mov', 'dword ptr [esi], 0x555da4'),
            0x4e8cf0: ('mov', 'eax, dword ptr [esp + 4]'),
            0x4e8cf6: ('mov', 'dword ptr [ecx + 8], eax'),
            0x42e388: ('mov', 'dword ptr [ebx + 0x16c], 0x429a0000'),
            0x42e3ac: ('mov', 'dword ptr [ebx + 0x1bc], ecx'),
            0x42f1a6: ('mov', 'dword ptr [eax + 0x1a8], edx'),
            0x42f1c0: ('mov', 'dword ptr [eax + 0x1ac], edi'),
            0x43e3bd: ('mov', 'eax, dword ptr [edx + 8]'),
            0x43e3b7: ('mov', 'ecx, dword ptr [0x575a98]'),
            0x43e42b: ('mov', 'ecx, dword ptr [0x575a98]'),
            0x43e468: ('mov', 'ecx, dword ptr [0x575a98]'),
            0x463731: ('lea', 'ebp, [esi + 0xec]'),
            0x463744: ('mov', 'ecx, dword ptr [0x575a98]'),
            0x5192e9: ('mov', 'eax, dword ptr [edx + 8]'),
            0x5192f2: ('mov', 'ecx, dword ptr [0x575a98]'),
            0x5192f8: ('lea', 'edx, [eax + 0xec]'),
        }.items():
            self.assertEqual(decoded[va], expected)
        sites = {0x43e3c1: 0x52f140, 0x43e43b: 0x52f0a0, 0x43e46e: 0x52f4d0,
                 0x463754: 0x52f340, 0x519300: 0x52f340}
        for va, target in sites.items():
            self.assertEqual(decoded[va], ('call', hex(target)))
        pins = json.loads((ROOT/'tests/fixtures/visibility_connections_objects.json').read_text())
        compiler = Path(os.environ['VC6_ROOT']).resolve()
        linker = find_child_ci(find_vc6_bin(compiler), 'link.exe')
        libraries = find_child_ci(infer_vc98_root(compiler), 'lib')
        strict = 0
        with tempfile.TemporaryDirectory(prefix='mcm2-visibility-') as directory:
            out = Path(directory)
            def compile_source(source, name):
                path = out/(name + '.obj')
                result = subprocess.run([sys.executable, str(ROOT/'tools/compile.py'), str(source),
                    '-o', str(path), '--vc6-root', str(compiler), '--profile', 'vc6_o2_mt',
                    '--include', str(ROOT/'src/krusty2'), '--include', str(ROOT/'src/reconstructed')],
                    cwd=ROOT, capture_output=True, text=True, encoding='latin1',
                    env={**os.environ, 'PYTHONPATH': str(ROOT)})
                self.assertEqual(result.returncode, 0, result.stdout + result.stderr)
                return path, CoffObject(path)
            objects = {}
            for index, source in enumerate(SOURCES + ('src/reconstructed/Camera.cpp',
                    'src/reconstructed/PCCamera.cpp', 'src/reconstructed/RenderTarget.cpp')):
                _, obj = compile_source(ROOT/source, 'full%d' % index)
                objects[source] = obj
                if source in pins['objects']:
                    rows, filenames = view(obj, (ROOT,))
                    actual = dict(sections=len(rows), relocations=sum(len(r[3]) for r in rows),
                                  sha256=hashlib.sha256(json.dumps(rows).encode()).hexdigest())
                    self.assertEqual(actual, pins['objects'][source])
                    self.assertTrue(all(suffix in FILENAMES for _, suffix in filenames))
            for case in CASES:
                if case['source'] not in objects or case['source'] not in SOURCES[:3]:
                    continue
                report = compare(pe, objects[case['source']], case['symbol'],
                                 int(case['target_va'], 0), case['target_size'], ROOT/case['bindings'])
                self.assertTrue(report['strict_exact'], report)
                strict += 1
            for target in json.loads((ROOT/'src/krusty2/visibility/targets.json').read_text()):
                if target['expect'] != 'exact': continue
                report = compare(pe, objects[PROVIDER], target['candidate_symbol_contains'],
                                 int(target['target_va'], 0), target['target_size'],
                                 ROOT/'src/krusty2/visibility'/target['bindings'])
                self.assertTrue(report['strict_exact'], report)
                strict += 1
            for source, symbol, va, size in [
                ('src/reconstructed/Camera.cpp', '??0Camera@@', 0x42e340, 402),
                ('src/reconstructed/Camera.cpp', '?SetViewport@Camera', 0x42f190, 114),
                ('src/reconstructed/PCCamera.cpp', '??0PCCamera', 0x4bed80, 25),
                ('src/reconstructed/RenderTarget.cpp', '?SetCamera@RenderTarget', 0x4e8cf0, 49),
            ]:
                bindings = 'src/reconstructed/Camera.bindings.json' if 'PCCamera' in source else str(Path(source).with_suffix('.bindings.json'))
                report = compare(pe, objects[source], symbol, va, size, ROOT/bindings)
                self.assertTrue(report['strict_exact'], report)
                strict += 1
            joined = set(pins['renames'].values())
            observed_sites = {}
            for source, symbol, va in [(SOURCES[0], '?MarkVisibleCells@DrawableCube', 0x43e330),
                    (SOURCES[1], '?UnknownVirtualSlot38@FollowCamera', 0x4636e0),
                    (SOURCES[2], '?UnknownVirtualSlot14@NameOverlay', 0x5190e0)]:
                bindings = json.loads((ROOT/source).with_suffix('.bindings.json').read_text())
                for offset, kind, name in signature(objects[source], symbol)[2]:
                    if name in joined:
                        self.assertEqual(kind, 0x14)
                        observed_sites[va + offset - 1] = int(bindings[name], 0)
            self.assertEqual(observed_sites, sites)
            extracted = out/'visibility.cpp'
            source = (ROOT/PROVIDER).read_text()
            extracted.write_text('#include "visibility/VisibilityQuadTree.h"\n' +
                '\n'.join(definition(source, 'VisibilityClipper::' + name) for name, _, _ in METHODS))
            provider_path, provider = compile_source(extracted, 'provider')
            for name, va, size in METHODS:
                symbol = '?' + name + '@VisibilityClipper'
                for obj in (provider, objects[PROVIDER]):
                    report = compare(pe, obj, symbol, va, size, (ROOT/PROVIDER).with_suffix('.bindings.json'))
                    self.assertTrue(report['strict_exact'], report)
                    strict += 1
                self.assertEqual(signature(provider, symbol), signature(objects[PROVIDER], symbol))
            self.assertEqual({s.name for s in provider.symbols if s.storage_class == 2 and s.section_number == 0},
                             {'__fltused'})
            self.assertFalse(any(s.name in ('.data', '.bss', '.CRT$XCU') and s.raw_size for s in provider.sections))
            # Verify the actual Camera declaration independently from the provisional
            # visibility record, including the real protected-field offsets.
            layout = out/'layout.cpp'
            layout.write_text('#include "Camera.h"\n#include <stddef.h>\n'
                'class CameraLayout : public Camera { public: static int Check(){return '
                'sizeof(Camera)==0x220 && sizeof(Vector3)==12 && sizeof(Matrix4)==64 && '
                'offsetof(CameraLayout,screenMatrix)==0xec && offsetof(CameraLayout,fieldOfView)==0x16c && '
                'offsetof(CameraLayout,viewport)==0x1a0 && offsetof(CameraLayout,heightWidthRatio)==0x1b8 && '
                'offsetof(CameraLayout,nearPlane)==0x1bc;} };\n'
                'extern "C" int VisibilityCameraLayout(){return CameraLayout::Check();}\n')
            layout_path, _ = compile_source(layout, 'layout')
            fixture, _ = compile_source(ROOT/'tests/fixtures/visibility_connections_compare.cpp', 'fixture')
            executable = out/'visibility.exe'
            result = subprocess.run([str(linker), '/nologo', '/subsystem:console', '/base:0x100000',
                '/fixed', '/opt:ref', '/out:' + str(executable), '/libpath:' + str(libraries),
                str(provider_path), str(layout_path), str(fixture), 'kernel32.lib'],
                capture_output=True, text=True, encoding='latin1')
            self.assertEqual(result.returncode, 0, result.stdout + result.stderr)
            result = subprocess.run([str(executable), str(image)], capture_output=True, text=True, timeout=120)
            self.assertEqual(result.returncode, 0, result.stdout + result.stderr)
            self.assertIn('66845 visibility scenarios match', result.stdout)
            print('Visibility connections:', strict, 'strict checks; seven whole objects;', result.stdout.strip())


if __name__ == '__main__': unittest.main()
