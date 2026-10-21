"""Complete FollowCamera construction and explicit finite partial-write repair."""
import hashlib
import json
import os
from pathlib import Path
import re
import struct
import subprocess
import sys
import tempfile
import unittest
from unittest.mock import patch
from mcm2tool.coff import CoffObject
from mcm2tool.pe import PEImage
from mcm2tool.toolchain import find_child_ci, find_vc6_bin, infer_vc98_root
from tools.match import compare
from tools.build_game import ROOT as TOOL_ROOT, compile_source as game_compile
from test_edit_box_on_char import extract
import test_eco_quadtree_connections as views

ROOT = Path(__file__).resolve().parents[1]
SOURCE = 'samples/camera/FollowCameraConstructor.cpp'
AGGREGATE = 'samples/camera/FollowCameraNearMisses.cpp'
SYMBOL = '??0FollowCamera@@QAE@H@Z'
FILENAMES = (AGGREGATE, 'samples/camera/FollowCameraLoadFile.cpp',
             'samples/camera/FollowCameraTick.cpp', 'samples/camera/FollowCameraControls.cpp')
ALIASES = {'_FixtureMalloc': '_malloc', '_FixtureFree': '_free', '_FixtureMsize': '__msize'}


def function(obj, query):
    entry = obj.find_symbol(query)
    raw, size, rels = obj.symbol_extent(entry)
    def target(symbol, seen=()):
        if symbol.section_number == entry.section_number:
            return ('self', symbol.value-entry.value)
        if symbol.section_number > 0 and (symbol.name.startswith(('$L', '$T')) or
                symbol.name == obj.section(symbol.section_number).name):
            if symbol.section_number in seen:
                return ('backref', seen.index(symbol.section_number), symbol.value)
            section = obj.section(symbol.section_number)
            return ('local', section.name, section.characteristics,
                    obj.data[section.raw_ptr:section.raw_ptr+section.raw_size].hex(), symbol.value,
                    [(r.virtual_address, r.type, target(obj.symbol_by_index[r.symbol_index], seen+(section.index,)))
                     for r in obj.relocations if r.section_number == section.index])
        return ALIASES.get(symbol.name, symbol.name)
    return raw.hex(), size, [(r.virtual_address-entry.value, r.type, target(obj.symbol_by_index[r.symbol_index])) for r in rels]


def whole(obj, root=ROOT):
    with patch.object(views, 'FILENAMES', FILENAMES):
        return views.whole_object(obj, (root,))


def canonical_aliases(value):
    if isinstance(value, str):
        return ALIASES.get(value, value)
    if isinstance(value, (tuple, list)):
        return tuple(canonical_aliases(v) for v in value)
    return value


@unittest.skipUnless(os.name == 'nt' and os.environ.get('VC6_ROOT') and os.environ.get('MCM2_EXE'),
                     'authentic native VC6 and pinned retail required')
class FollowCameraConstructorTests(unittest.TestCase):
    def test_complete_constructor_actual_initialization_and_unwind(self):
        import capstone
        retail = Path(os.environ['MCM2_EXE']).resolve()
        self.assertEqual(hashlib.sha256(retail.read_bytes()).hexdigest(),
                         '31fde4cc686a5ee89ef9095b90235325b195596867ecacefe511263e1509b874')
        image = PEImage(retail)
        decoder = capstone.Cs(capstone.CS_ARCH_X86, capstone.CS_MODE_32)
        def decode(va, size):
            code = list(decoder.disasm(image.bytes_at_va(va, size), va))
            self.assertEqual(sum(i.size for i in code), size)
            return {i.address: (i.mnemonic, i.op_str) for i in code}
        body = decode(0x462ee0, 564)
        self.assertEqual({va: op[1] for va, op in body.items() if op[0] == 'call'},
                         {0x462f03: '0x4bed80', 0x46302e: '0x466e50'})
        self.assertEqual([va for va, op in body.items() if op[0] == 'ret'], [0x463111])
        self.assertEqual(body[0x463111], ('ret', '4'))
        self.assertEqual(image.bytes_at_va(0x463114, 12), b'\x90'*12)
        for va, op in {0x462f0a: ('mov', 'dword ptr [esi], 0x55282c'),
                       0x462f10: ('mov', 'byte ptr [esi + 0x275], bl'),
                       0x462f23: ('mov', 'dword ptr [esp + 0x18], ebx'),
                       0x462f81: ('mov', 'dword ptr [esi + 0x2e4], ebx'),
                       0x462f87: ('mov', 'byte ptr [esi + 0x276], bl'),
                       0x463000: ('mov', 'byte ptr [esi + 0x274], bl'),
                       0x463006: ('mov', 'dword ptr [esi + 0x2dc], ebx')}.items():
            self.assertEqual(body[va], op)
        self.assertEqual(decode(0x54ab60, 18), {
            0x54ab60: ('mov', 'ecx, dword ptr [ebp - 0x10]'), 0x54ab63: ('jmp', '0x4624d0'),
            0x54ab68: ('mov', 'eax, 0x560ca8'), 0x54ab6d: ('jmp', '0x53471a')})
        self.assertEqual(struct.unpack('<8I', image.bytes_at_va(0x560ca8, 32)),
                         (0x19930520, 1, 0x560cc8, 0, 0, 0, 0, 0))
        self.assertEqual(struct.unpack('<2I', image.bytes_at_va(0x560cc8, 8)), (0xffffffff, 0x54ab60))
        classes = ['VehicleCamera', 'FollowCamera', 'PCCamera', 'Camera', 'GameObject', 'BaseObject']
        for index, table in enumerate((0x558c30, 0x55282c, 0x555da4, 0x550f78, 0x552a2c, 0x5507c0)):
            col = struct.unpack('<I', image.bytes_at_va(table-4, 4))[0]
            words = struct.unpack('<5I', image.bytes_at_va(col, 20))
            self.assertEqual(words[1], 0)
            self.assertEqual(image.bytes_at_va(words[3]+8, 80).split(b'\0')[0], ('.?AV'+classes[index]+'@@').encode())
            hierarchy = struct.unpack('<4I', image.bytes_at_va(words[4], 16))
            self.assertEqual(hierarchy[2], len(classes)-index)
            for j, name in enumerate(classes[index:]):
                pointer = struct.unpack('<I', image.bytes_at_va(hierarchy[3]+4*j, 4))[0]
                base = struct.unpack('<6I', image.bytes_at_va(pointer, 24))
                self.assertEqual(base[2:5], (0, 0xffffffff, 0))
                self.assertEqual(image.bytes_at_va(base[0]+8, 80).split(b'\0')[0], ('.?AV'+name+'@@').encode())
        caller = decode(0x52b920, 137)
        for va, op in {0x52b920: ('mov', 'eax, dword ptr [esp + 4]'),
                       0x52b925: ('mov', 'esi, ecx'), 0x52b927: ('push', 'eax'),
                       0x52b928: ('call', '0x462ee0'),
                       0x52b92f: ('mov', 'dword ptr [esi], 0x558c30')}.items():
            self.assertEqual(caller[va], op)

        with tempfile.TemporaryDirectory(prefix='mcm2-camera-constructor-') as directory:
            out = Path(directory)
            vc6 = Path(os.environ['VC6_ROOT']).resolve()
            env = {**os.environ, 'PYTHONPATH': str(TOOL_ROOT)}
            def compile_source(source, name, includes=True):
                path = out/(name+'.obj')
                command = [sys.executable, str(TOOL_ROOT/'tools/compile.py'), str(source), '-o', str(path),
                           '--vc6-root', str(vc6), '--profile', 'vc6_o2_mt']
                if includes: command += ['--include', str(ROOT/'src/krusty2'), '--include', str(ROOT/'src/reconstructed')]
                run = subprocess.run(command, capture_output=True, text=True, env=env)
                self.assertEqual(run.returncode, 0, run.stdout+run.stderr)
                return path, CoffObject(path)
            def includes(text, parent):
                return re.sub(r'#include "([^"]+)"', lambda m: '#include "'+str((parent/m[1]).resolve()).replace('\\', '/')+'"', text)
            candidate_path, candidate = compile_source(ROOT/SOURCE, 'candidate', False)
            aggregate_path, aggregate = compile_source(ROOT/AGGREGATE, 'aggregate', False)
            original_text = (ROOT/AGGREGATE).read_text()
            original_source = out/'original.cpp'
            original_source.write_text(includes(original_text[:original_text.index('FollowCamera::FollowCamera(int flags)')]+
                extract(original_text, 'FollowCamera::FollowCamera(int flags)')+'\n', (ROOT/AGGREGATE).parent))
            original_path, original = compile_source(original_source, 'original')
            for obj, binding, size, matching in (
                (candidate, ROOT/SOURCE.replace('.cpp', '.bindings.json'), 569, 100),
                (original, ROOT/AGGREGATE.replace('.cpp', '.bindings.json'), 564, 511),
                (aggregate, ROOT/AGGREGATE.replace('.cpp', '.bindings.json'), 564, 511)):
                proof = compare(image, obj, SYMBOL, 0x462ee0, 564, binding)
                self.assertEqual((proof['candidate_size'], proof['strict_match']['matching_positions']), (size, matching))
                self.assertFalse(proof['strict_exact'])
            self.assertEqual(function(original, SYMBOL), function(aggregate, SYMBOL))
            pins = json.loads((ROOT/'tests/fixtures/follow_camera_constructor_objects.json').read_text())
            rows = whole(aggregate)
            actual = dict(sections=len(rows), relocations=sum(len(r[-1]) for r in rows),
                          sha256=hashlib.sha256(json.dumps(rows, separators=(',', ':')).encode()).hexdigest())
            self.assertEqual(actual, pins['aggregate'])
            candidate_rows = whole(candidate)
            self.assertEqual(dict(sections=len(candidate_rows),
                relocations=sum(len(row[-1]) for row in candidate_rows),
                sha256=hashlib.sha256(json.dumps(candidate_rows, separators=(',', ':')).encode()).hexdigest()),
                pins['candidate'])
            spec = json.loads((TOOL_ROOT/'config/game_build.json').read_text())
            built = game_compile(ROOT/SOURCE, out/'game-command', vc6, [TOOL_ROOT/i for i in spec['includes']], spec['profile'], env)
            self.assertTrue(built['compiled'], built)
            self.assertEqual(whole(candidate), whole(CoffObject(Path(built['object']))))

            keep = {'Camera': {'Camera', '~Camera'}, 'PCCamera': {'PCCamera'},
                    'FollowCamera': {'UnknownVirtualSlot58', 'UnknownVirtualSlot59', 'UnknownVirtualSlot60',
                        'UnknownVirtualSlot61', 'UnknownVirtualSlot62', 'UnknownVirtualSlot63', 'UnknownVirtualSlot64',
                        'UnknownVirtualSlot65', 'UnknownVirtualSlot66', 'UnknownVirtualSlot67', 'UnknownVirtualSlot71'}}
            native = {}; canonical = {}; paths = [candidate_path]
            for name, retained in keep.items():
                source = ROOT/'src/reconstructed'/(name+'.cpp')
                _, canonical[name] = compile_source(source, 'canonical-'+name)
                text = source.read_text()
                edits = []
                pattern = re.compile(r'^[^/\n]*?\b'+name+r'::(~?\w+)\([^;\n]*\)[^;{\n]*\{', re.M)
                for match in pattern.finditer(text):
                    if match[1] in retained: continue
                    start = text.index('{', match.start()); depth = 1; end = start+1
                    while depth:
                        depth += (text[end] == '{')-(text[end] == '}'); end += 1
                    edits.append((start, end))
                for start, end in reversed(edits): text = text[:start]+'{ FixtureUnvisited(); }'+text[end:]
                source_path = out/(name+'-init.cpp')
                source_path.write_text('extern "C" __declspec(noreturn) void FixtureUnvisited();\n'+includes(text, source.parent))
                path, native[name] = compile_source(source_path, name+'-init'); paths.append(path)
            for name in ('GameObject', 'BaseObject', 'MemTag', 'MatrixUtil', 'GraphicsState', 'FastMath', 'VectorCalls'):
                path, native[name] = compile_source(ROOT/'src/reconstructed'/(name+'.cpp'), name)
                canonical[name] = native[name]; paths.append(path)
            _, canonical['VehicleCamera'] = compile_source(ROOT/'src/reconstructed/VehicleCamera.cpp', 'VehicleCamera')
            boundary_source = out/'MemTagBoundary.cpp'
            boundary_source.write_text('#define malloc FixtureMalloc\n#define free FixtureFree\n#define _msize FixtureMsize\n#include "'+str(ROOT/'src/reconstructed/MemTag.cpp').replace('\\', '/')+'"\n')
            boundary_path, boundary = compile_source(boundary_source, 'MemTagBoundary')
            self.assertEqual(canonical_aliases(whole(boundary)), canonical_aliases(whole(native['MemTag'])))
            paths[paths.index(out/'MemTag.obj')] = boundary_path
            strict = 0
            checks = [('Camera','??0Camera@@QAE@H@Z',0x42e340,402),('Camera','??1Camera@@UAE@XZ',0x42f020,37),
                ('PCCamera','??0PCCamera@@QAE@H@Z',0x4bed80,25),('PCCamera','??1PCCamera@@UAE@XZ',0x4624d0,5),
                ('GameObject','??0GameObject@@QAE@H@Z',0x468ca0,149),('GameObject','??1GameObject@@UAE@XZ',0x468d60,97),
                ('GameObject','?AppendClassName@GameObject',0x469ce0,201),('BaseObject','??0BaseObject',0x405120,16),
                ('BaseObject','??1BaseObject',0x405150,7),('MemTag','?DebugMalloc@@',0x4a2e20,57),
                ('MemTag','?DebugFree@@',0x4a2e60,96),('MemTag','?DebugRealloc@@',0x4a2ec0,256),
                ('MemTag','?DebugMemcpy@@',0x4a3120,35),('MatrixUtil','?IdentityMatrix@@',0x4a1410,71),
                ('VehicleCamera','??0VehicleCamera@@QAE@H@Z',0x52b920,137)]
            for unit, symbol, va, size in checks:
                obj = native.get(unit, canonical[unit])
                binding = ROOT/'src/reconstructed'/(unit+'.bindings.json')
                if unit == 'PCCamera': binding = ROOT/'src/reconstructed/Camera.bindings.json'
                self.assertTrue(compare(image, obj, symbol, va, size, binding)['strict_exact'], (unit, symbol))
                self.assertEqual(function(obj, symbol), function(canonical[unit], symbol))
                if unit == 'MemTag': self.assertEqual(function(obj, symbol), function(boundary, symbol))
                strict += 1
            for slot, va, size in ((58,0x464e90,1),(61,0x464e90,1),(66,0x466cf0,41),(71,0x466e50,192)):
                symbol = '?UnknownVirtualSlot%d@FollowCamera@@UAEX'%slot+('H@Z' if slot==71 else 'XZ')
                self.assertEqual(struct.unpack('<I', image.bytes_at_va(0x55282c+4*slot,4))[0], va)
                self.assertTrue(compare(image, native['FollowCamera'], symbol, va, size, ROOT/'src/reconstructed/FollowCamera.bindings.json')['strict_exact'])
                self.assertEqual(function(native['FollowCamera'], symbol), function(canonical['FollowCamera'], symbol)); strict += 1
            for name, va, size in (('_$E2',0x466f10,5),('_$E1',0x466f20,60),('_$E5',0x466f60,5),('_$E4',0x466f70,60),
                                   ('_$E8',0x466ff0,5),('_$E7',0x467000,60),('_$E11',0x4670b0,5),('_$E10',0x4670c0,60)):
                self.assertTrue(compare(image, candidate, name, va, size, ROOT/SOURCE.replace('.cpp','.bindings.json'))['strict_exact'])
                self.assertEqual(function(candidate,name),function(aggregate,name)); strict += 1
            crt = [s for s in candidate.sections if s.name=='.CRT$XCU']; self.assertEqual(len(crt),1)
            self.assertEqual(crt[0].raw_size,16)
            self.assertEqual([candidate.symbol_by_index[r.symbol_index].name for r in candidate.relocations if r.section_number==crt[0].index],['_$E2','_$E5','_$E8','_$E11'])
            for symbol, offset in (('_kVec3Zero',0),('_kVec3XAxis',32),('_kVec3YAxis',48),('_kVec3ZAxis',16)):
                entry = candidate.find_symbol(symbol); section = candidate.section(entry.section_number)
                self.assertEqual((section.name, section.raw_size, entry.value),('.bss',60,offset)); self.assertTrue(section.characteristics&0x80000000)
            fixture_path, fixture = compile_source(ROOT/'tests/fixtures/follow_camera_constructor_compare.cpp','fixture')
            self.assertTrue({'_malloc','_free','__msize'}.issubset({s.name for s in fixture.symbols if s.storage_class==2 and s.section_number==0}))
            paths.append(fixture_path)
            linker = find_child_ci(find_vc6_bin(vc6),'link.exe'); library = find_child_ci(infer_vc98_root(vc6),'lib')
            def execute(first, name):
                executable = out/(name+'.exe'); mapping_path = out/(name+'.map')
                command = [str(linker),'/nologo','/subsystem:console','/base:0x100000','/fixed','/opt:ref',
                           '/include:'+SYMBOL,'/map:'+str(mapping_path),'/out:'+str(executable),'/libpath:'+str(library),
                           str(first),*[str(p) for p in paths[1:]],'kernel32.lib']
                result = subprocess.run(command,capture_output=True,text=True);self.assertEqual(result.returncode,0,result.stdout+result.stderr)
                mapping = {m[1]:m[2] for m in re.finditer(r'^\s+\S+:\S+\s+(\S+)\s+([0-9a-fA-F]{8})\s',mapping_path.read_text(),re.M)}
                names = [SYMBOL,'??0PCCamera@@QAE@H@Z','?UnknownVirtualSlot71@FollowCamera@@UAEXH@Z',
                         '?UnknownVirtualSlot58@FollowCamera@@UAEXXZ','?UnknownVirtualSlot61@FollowCamera@@UAEXXZ',
                         '?UnknownVirtualSlot66@FollowCamera@@UAEXXZ','??_7FollowCamera@@6B@','??1PCCamera@@UAE@XZ','??_7BaseObject@@6B@']
                return subprocess.run([str(executable),str(retail),*[mapping[n] for n in names]],capture_output=True,text=True,timeout=30)
            result = execute(candidate_path,'corrected')
            self.assertEqual(result.returncode,0,result.stdout+result.stderr)
            self.assertIn('FollowCamera constructor: 3072 normal, 4284 guarded (3504 AV), 36 allocation cases',result.stdout)
            control = execute(original_path,'original-control')
            self.assertEqual(control.returncode,36,control.stdout+control.stderr)
            self.assertIn('Guard oracle available2c4',control.stdout)
            self.assertEqual(strict,27)
            print('27 complete strict checks; unchanged aggregate; 7392 native cases; original guard-order control rejected')


if __name__ == '__main__':
    unittest.main()
