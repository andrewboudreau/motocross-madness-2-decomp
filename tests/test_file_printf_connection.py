"""FilePrintf callers use the complete existing integer-returning CRT wrapper."""
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
from tools.run_calibration import CASES
from test_edit_box_on_char import extract

ROOT = Path(__file__).resolve().parents[1]
SOURCES = ('src/reconstructed/FileStream.cpp',
           'src/reconstructed/ManagedTextureGroup.cpp',
           'src/reconstructed/EcoSystem.cpp',
           'samples/ecosystem/EcoSystemNearMisses.cpp')
FILENAMES = SOURCES + ('src/reconstructed/ContainerList.h',)
PROVIDER = '?FilePrintf@@YAHPAU_iobuf@@PBDZZ'
RETIRED = '?FilePrintf@@YAXPAU_iobuf@@PBDZZ'


def whole_object(obj, roots=(ROOT,)):
    """Keep all nondebug contents; normalize only the enumerated source paths."""
    sections = [s for s in obj.sections if not s.name.startswith('.debug')]
    prefixes = [str(root).replace('\\', '/') + '/' for root in roots]

    def filename(data):
        value, nul, tail = data.partition(b'\0')
        path = value.decode('latin1').replace('\\', '/')
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
                     'requires authentic native VC6 and private retail')
class FilePrintfConnectionTests(unittest.TestCase):
    def test_complete_objects_call_abi_and_actual_crt_provider(self):
        import capstone
        image_path = Path(os.environ['MCM2_EXE']).resolve()
        self.assertEqual(hashlib.sha256(image_path.read_bytes()).hexdigest(),
                         '31fde4cc686a5ee89ef9095b90235325b195596867ecacefe511263e1509b874')
        image = PEImage(image_path)
        decoder = capstone.Cs(capstone.CS_ARCH_X86, capstone.CS_MODE_32)
        # This is a free cdecl function: FILE*, format, first vararg are at
        # ESP+4/+8/+12. The vfprintf result survives unchanged in EAX.
        instructions = [(i.address, i.mnemonic, i.op_str) for i in
                        decoder.disasm(image.bytes_at_va(0x461d40, 24), 0x461d40)]
        self.assertEqual(instructions, [
            (0x461d40, 'mov', 'ecx, dword ptr [esp + 8]'),
            (0x461d44, 'mov', 'edx, dword ptr [esp + 4]'),
            (0x461d48, 'lea', 'eax, [esp + 0xc]'),
            (0x461d4c, 'push', 'eax'), (0x461d4d, 'push', 'ecx'),
            (0x461d4e, 'push', 'edx'), (0x461d4f, 'call', '0x536b23'),
            (0x461d54, 'add', 'esp, 0xc'), (0x461d57, 'ret', ''),
        ])
        # 0x536b23 is an existing CRT binding, not a PE import thunk. This
        # test proves the game wrapper's complete body and its one call target;
        # native checks below use the linked VC6 CRT, not copied retail CRT.

        call_sites = (
            (SOURCES[1], '?WriteDebugReport@ManagedTextureGroup@@', 0x50c7e0, 224,
             [0x50c808, 0x50c81f, 0x50c85a, 0x50c88f, 0x50c89d]),
            (SOURCES[3], '?WriteListing@EcoSystem@@', 0x458da0, 461,
             [0x458e3d, 0x458f35]),
        )
        for source, symbol, va, size, sites in call_sites:
            code = {i.address: (i.mnemonic, i.op_str) for i in
                    decoder.disasm(image.bytes_at_va(va, size), va)}
            self.assertEqual([at for at, op in code.items() if op == ('call', '0x461d40')], sites)
            for site in sites:
                self.assertIn(code[site+5][0], ('mov', 'lea', 'fld', 'add'))
        # Both caller FILE* values originate in fopen; no object receiver or
        # C++ subobject adjustment is involved. Both listing calls overwrite EAX.
        for va, count, expected in (
            (0x50c7ee, 7, [('call', '0x534ceb'), ('mov', 'edi, eax')]),
            (0x458e1a, 7, [('call', '0x534ceb'), ('mov', 'esi, eax')]),
            (0x458e42, 9, [('mov', 'eax, dword ptr [ebx + 0x55c]'), ('add', 'esp, 0xc')]),
            (0x458f3a, 13, [('mov', 'eax, dword ptr [esp + 0x4c]'),
                           ('mov', 'ecx, dword ptr [ebx + 0x55c]'), ('add', 'esp, 0x34')]),
        ):
            self.assertEqual([(i.mnemonic, i.op_str) for i in decoder.disasm(image.bytes_at_va(va, count), va)], expected)

        pins = json.loads((ROOT/'tests/fixtures/file_printf_objects.json').read_text())
        compiler = Path(os.environ['VC6_ROOT'])
        strict = 0
        with tempfile.TemporaryDirectory(prefix='mcm2-file-printf-') as directory:
            output = Path(directory)
            def compile_source(source, stem, fixture=False):
                path = output/(stem + '.obj')
                command = [sys.executable, str(ROOT/'tools/compile.py'), str(source),
                           '-o', str(path), '--vc6-root', str(compiler), '--profile', 'vc6_o2_mt']
                if fixture:
                    command += ['--include', str(ROOT/'src/reconstructed')]
                result = subprocess.run(command, cwd=ROOT, capture_output=True, text=True,
                                        encoding='latin1', env={**os.environ, 'PYTHONPATH':str(ROOT)})
                self.assertEqual(result.returncode, 0, result.stdout+result.stderr)
                return path, CoffObject(path)
            objects = {}
            for index, source in enumerate(SOURCES):
                _, obj = compile_source(ROOT/source, 'unit%d' % index)
                objects[source] = obj
                rows = whole_object(obj)
                self.assertEqual((len(rows), sum(len(r[3]) for r in rows)),
                                 (pins['objects'][source]['sections'], pins['objects'][source]['relocations']))
                self.assertEqual(hashlib.sha256(json.dumps(rows).encode()).hexdigest(), pins['objects'][source]['sha256'])
                self.assertFalse(any(s.name == RETIRED for s in obj.symbols))
                for case in CASES:
                    if case['source'] != source:
                        continue
                    proof = compare(image, obj, case['symbol'], int(case['target_va'], 0),
                                    case['target_size'], ROOT/case['bindings'])
                    self.assertTrue(proof['strict_exact'], proof)
                    strict += 1
            for source, query, va, size, sites in call_sites:
                obj = objects[source]
                entry = obj.find_symbol(query)
                body, extent, relocations = obj.symbol_extent(entry)
                self.assertEqual((extent, entry.extent_source), (size, 'coff_function_aux'))
                self.assertEqual([(r.virtual_address-entry.value, r.type) for r in relocations
                                  if obj.symbol_by_index[r.symbol_index].name == PROVIDER],
                                 [(site + 1 - va, 20) for site in sites])
            partial = compare(image, objects[SOURCES[3]], '?WriteListing@EcoSystem@@', 0x458da0, 461,
                              ROOT/'samples/ecosystem/EcoSystemNearMisses.bindings.json')
            self.assertEqual((partial['candidate_size'], partial['strict_match']['matching_positions']), (461, 457))
            self.assertFalse(partial['strict_exact'])

            # Extract the unchanged existing provider for native linkage only.
            # Its entire body and named relocations must equal the actual TU.
            source = output/'provider.cpp'
            source.write_text('#include "FilePrintf.h"\n#include <stdarg.h>\n' +
                              extract((ROOT/SOURCES[0]).read_text(), 'int FilePrintf(') + '\n')
            provider_path, provider = compile_source(source, 'provider', True)
            fixture_path, _ = compile_source(ROOT/'tests/fixtures/file_printf_calls.cpp', 'fixture', True)
            def function(obj):
                entry = obj.find_symbol(PROVIDER)
                body, size, relocations = obj.symbol_extent(entry)
                return body, size, [(r.virtual_address-entry.value, r.type,
                                     obj.symbol_by_index[r.symbol_index].name) for r in relocations]
            self.assertEqual(function(provider), function(objects[SOURCES[0]]))
            self.assertEqual(function(provider)[1:], (24, [(16, 20, '_vfprintf')]))
            self.assertEqual({s.name for s in provider.symbols if s.storage_class == 2 and s.section_number == 0}, {'_vfprintf'})
            self.assertEqual({s.name for s in provider.symbols if s.storage_class == 2 and s.section_number > 0}, {PROVIDER})
            self.assertFalse(any(s.raw_size and (s.name.startswith('.CRT') or s.characteristics & 0xc0)
                                 for s in provider.sections if not s.name.startswith('.debug')))
            self.assertTrue(compare(image, provider, PROVIDER, 0x461d40, 24,
                                    ROOT/'src/reconstructed/FileStream.bindings.json')['strict_exact'])
            strict += 1
            executable = output/'calls.exe'
            result = subprocess.run([str(find_child_ci(find_vc6_bin(compiler), 'link.exe')),
                '/nologo', '/subsystem:console', '/out:'+str(executable),
                '/libpath:'+str(find_child_ci(infer_vc98_root(compiler), 'lib')),
                str(provider_path), str(fixture_path)], cwd=ROOT, capture_output=True, text=True, encoding='latin1')
            self.assertEqual(result.returncode, 0, result.stdout+result.stderr)
            result = subprocess.run([str(executable), str(output/'readonly.dat')],
                                    capture_output=True, text=True, timeout=30)
            self.assertEqual(result.returncode, 0, result.stdout+result.stderr)
            self.assertEqual(result.stdout.strip(), 'FilePrintf: 227 real CRT output, return and stream-error cases pass')
        self.assertEqual(strict, 74)
        print('74 strict checks; four complete objects; seven cdecl call sites; ' + result.stdout.strip())


if __name__ == '__main__':
    unittest.main()
