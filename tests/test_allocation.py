"""Allocation/relocation tests use synthetic bytes and candidate C++ only."""
from pathlib import Path
import hashlib
import json
import shutil
import struct
import subprocess
import tempfile
import unittest
from unittest.mock import patch

from mcm2tool.pe import PEImage, PEFormatError
from mcm2tool.allocation import (Insn, analyze, ascii_at, category_literals, compare_resolved_bodies,
                                direct_call_target, iat_symbols, imported_target,
                                parse_objdump, reachable_imports, read_va, reviewed_body)
from mcm2tool.resolved_match import apply_relocations, compare_bytes, RelocationError

ROOT = Path(__file__).resolve().parents[1]


def fixture_bytes():
    data = bytearray(0xa00)
    data[:2] = b'MZ'
    struct.pack_into('<I', data, 0x3c, 0x80)
    data[0x80:0x84] = b'PE\0\0'
    struct.pack_into('<HHIIIHH', data, 0x84, 0x14c, 2, 0, 0, 0, 0xe0, 0x102)
    oh = 0x98
    struct.pack_into('<H', data, oh, 0x10b)
    for off, value in [(16,0x1000),(28,0x400000),(32,0x1000),(36,0x200),(56,0x3000),(60,0x200),(92,16)]:
        struct.pack_into('<I', data, oh+off, value)
    struct.pack_into('<II', data, oh+104, 0x2000, 40)
    for i,(name,va,rs,ro,flags) in enumerate([(b'.text',0x1000,0x200,0x200,0x60000020), (b'.rdata',0x2000,0x600,0x400,0x40000040)]):
        at=oh+0xe0+i*40
        data[at:at+len(name)] = name
        struct.pack_into('<IIIIIIHHI', data, at+8, rs,va,rs,ro,0,0,0,0,flags)
    struct.pack_into('<5I', data, 0x400, 0x2040,0,0,0x2060,0x2050)
    struct.pack_into('<3I', data, 0x440, 0x2080,0x80000009,0)
    struct.pack_into('<3I', data, 0x450, 0x2080,0x80000009,0)
    for at,value in [(0x460,b'KERNEL32.dll\0'), (0x482,b'HeapFree\0'), (0x4a0,b'Terrain\0')]:
        data[at:at+len(value)] = value
    data[0x200]=0xc3
    return data


def call(va, target):
    return Insn(va, b'\xe8'+struct.pack('<i', target-(va+5)), 'call', hex(target))


class AllocationTests(unittest.TestCase):
    def setUp(self):
        self.temp=tempfile.TemporaryDirectory(); self.addCleanup(self.temp.cleanup)
        self.path=Path(self.temp.name)/'fixture.exe'
        self.data=fixture_bytes(); self.pe=self.load()

    def load(self):
        self.path.write_bytes(self.data); return PEImage(self.path)

    def test_import_names_and_ordinals(self):
        rows=iat_symbols(self.pe)
        self.assertEqual(rows[0x402050]['name'],'HeapFree')
        self.assertEqual(rows[0x402054]['ordinal'],9)
        self.assertEqual(rows[0x402050]['module'],'KERNEL32.dll')

    def test_bound_iat_keeps_name_unknown(self):
        struct.pack_into('<II',self.data,0x400,0,1)
        struct.pack_into('<I',self.data,0x450,0x70000000)
        row=iat_symbols(self.load())[0x402050]
        self.assertIsNone(row['name']); self.assertIsNone(row['ordinal'])
        self.assertEqual(row['resolution'],'bound_iat_without_lookup')

    def test_import_terminator_required(self):
        struct.pack_into('<I',self.data,0x98+108,20)
        with self.assertRaises(PEFormatError): iat_symbols(self.load())

    def test_import_unbound_fallback(self):
        struct.pack_into('<I',self.data,0x400,0)
        self.assertEqual(iat_symbols(self.load())[0x402050]['name'],'HeapFree')

    def test_read_does_not_cross_section(self):
        with self.assertRaises(PEFormatError): read_va(self.pe,0x4011ff,2)
        with self.assertRaises(PEFormatError): read_va(self.pe,0x401000,-1)

    def test_ascii_requires_printable_terminated_string(self):
        self.assertEqual(ascii_at(self.pe,0x4020a0),'Terrain')
        self.assertIsNone(ascii_at(self.pe,0x4020a0,3))
        self.assertIsNone(ascii_at(self.pe,0x799999))

    def test_disassembly_readback(self):
        rows=parse_objdump('  401000:\tc3\tret\n',self.pe)
        self.assertEqual(rows[0x401000].raw,b'\xc3')
        with self.assertRaises(PEFormatError): parse_objdump('  401000:\t90\tnop\n',self.pe)

    def test_direct_calls_use_bytes_not_pretty_labels(self):
        row=call(0x1000,0x2000)
        self.assertEqual(direct_call_target(Insn(row.va,row.raw,'call','wrong_label')),0x2000)
        self.assertIsNone(direct_call_target(Insn(0,b'\x90','call','0x2000')))

    def test_iat_operand_uses_bytes(self):
        ins=Insn(0x401000,b'\xff\x15'+struct.pack('<I',0x402050),'call','label')
        self.assertEqual(imported_target(ins,iat_symbols(self.pe))['name'],'HeapFree')

    def test_reviewed_ranges_reject_gap_or_midinstruction(self):
        ins={1:Insn(1,b'\x90','nop',''),3:Insn(3,b'\xc3','ret','')}
        with self.assertRaises(ValueError): reviewed_body(ins,1,4)
        with self.assertRaises(ValueError): reviewed_body({1:call(1,10)},1,4)

    def test_reviewed_range_requires_end_transfer(self):
        with self.assertRaises(ValueError): reviewed_body({1:Insn(1,b'\x90','nop','')},1,2)

    def test_same_destination_is_not_blind_masking(self):
        a=[call(0x1000,0x9000),Insn(0x1005,b'\xc3','ret','')]
        b=[call(0x2000,0x9000),Insn(0x2005,b'\xc3','ret','')]
        r=compare_resolved_bodies(a,b)
        self.assertFalse(r['raw_bytes_equal']); self.assertTrue(r['equal_with_resolved_transfers'])
        self.assertFalse(r['library_identity_established'])

    def test_different_call_target_fails(self):
        self.assertFalse(compare_resolved_bodies([call(0x1000,0x9000)],[call(0x2000,0xa000)])['equal_with_resolved_transfers'])

    def test_internal_call_requires_same_relative_target(self):
        a=[call(0x1000,0x1005),Insn(0x1005,b'\xc3','ret','')]
        b=[call(0x2000,0x2005),Insn(0x2005,b'\xc3','ret','')]
        self.assertTrue(compare_resolved_bodies(a,b)['equal_with_resolved_transfers'])

    def test_changed_nonrelocation_or_extent_fails(self):
        a=[Insn(1,b'\xb8\x01\0\0\0','mov','eax,1'),Insn(6,b'\xc3','ret','')]
        b=[Insn(1,b'\xb8\x02\0\0\0','mov','eax,2'),Insn(6,b'\xc3','ret','')]
        self.assertFalse(compare_resolved_bodies(a,b)['equal_with_resolved_transfers'])
        self.assertFalse(compare_resolved_bodies(a,b[:-1])['equal_with_resolved_transfers'])
        self.assertFalse(compare_resolved_bodies([],[])['equal_with_resolved_transfers'])

    def test_category_local_argument(self):
        ins={1:Insn(1,b'\x68'+struct.pack('<I',0x4020a0),'push','x'),6:call(6,0x9999)}
        self.assertEqual(category_literals(ins,0x9999,self.pe)[0]['label'],'Terrain')

    def test_category_does_not_scan_through_call_or_stack_change(self):
        ins={1:Insn(1,b'\x68'+struct.pack('<I',0x4020a0),'push','x'),6:call(6,0x8888),11:call(11,0x9999)}
        self.assertEqual(category_literals(ins,0x9999,self.pe),[])
        ins={1:Insn(1,b'\x68'+struct.pack('<I',0x4020a0),'push','x'),6:Insn(6,b'\x50','push','eax'),7:call(7,0x9999)}
        self.assertEqual(category_literals(ins,0x9999,self.pe),[])

    def test_reachable_imports_handle_cycles(self):
        rows=reachable_imports(1,{1:[2],2:[1,3]},{3:[{'name':'HeapFree'}]})
        self.assertEqual(rows[0]['path'],['0x00000001','0x00000002','0x00000003'])
        self.assertEqual(reachable_imports(1,{1:[2],2:[3]},{3:[{'name':'HeapFree'}]},max_depth=1),[])

    def test_analyzer_fails_on_unknown_input(self):
        with self.assertRaises(ValueError): analyze(self.pe,{'input_sha256':'0'*64},{})

    def test_analyzer_fails_on_changed_reviewed_body(self):
        cfg={'input_sha256':hashlib.sha256(self.pe.data).hexdigest(),
             'functions':[{'va':'0x00401000','size':1,'sha256':'0'*64}]}
        with self.assertRaises(ValueError): analyze(self.pe,cfg,{0x401000:Insn(0x401000,b'\xc3','ret','')})

    def test_static_analysis_never_promotes_library_identity(self):
        cfg={'input_sha256':hashlib.sha256(self.pe.data).hexdigest(),
             'functions':[{'va':'0x00401000','size':1,'sha256':hashlib.sha256(b'\xc3').hexdigest()}],
             'same_body_groups':[],'globals':[],'category_select_va':'0x00401000'}
        with patch('mcm2tool.allocation.parse_rtti',return_value=[]):
            r=analyze(self.pe,cfg,{0x401000:Insn(0x401000,b'\xc3','ret','')})
        self.assertEqual(r['functions'][0]['library_identity'],'unverified')
        self.assertIsNone(r['functions'][0]['original_source_unit'])
        self.assertEqual(r['summary']['library_signatures_matched'],0)


class ResolvedMatchTests(unittest.TestCase):
    def rel(self,kind=6,offset=0,symbol='target'):
        return {'type':kind,'offset':offset,'symbol':symbol}

    def test_dir32_includes_addend(self):
        out,audit=apply_relocations(struct.pack('<I',4),[self.rel()],{'target':0x500000},0x400000)
        self.assertEqual(struct.unpack('<I',out)[0],0x500004); self.assertEqual(len(audit),1)

    def test_rel32_includes_place_and_addend(self):
        out,_=apply_relocations(b'\xe8'+struct.pack('<I',3),[self.rel(0x14,1)],{'target':0x500000},0x400000)
        self.assertEqual(struct.unpack_from('<I',out,1)[0],0x500000+3-0x400005)

    def test_backward_rel32_wrap(self):
        out,_=apply_relocations(bytes(4),[self.rel(0x14)],{'target':0x300000},0x400000)
        self.assertEqual(struct.unpack('<i',out)[0],0x300000-0x400004)

    def test_unknown_symbol_fails_closed(self):
        with self.assertRaises(RelocationError): apply_relocations(bytes(4),[self.rel()],{},0x400000)

    def test_unsupported_kind_fails_closed(self):
        with self.assertRaises(RelocationError): apply_relocations(bytes(4),[self.rel(0x999)],{'target':1},0x400000)

    def test_overlapping_and_truncated_fixups_rejected(self):
        for rels in [[self.rel(offset=-1)],[self.rel(offset=1)],[self.rel(),self.rel()]]:
            with self.assertRaises(RelocationError): apply_relocations(bytes(4),rels,{'target':1},0x400000)

    def test_out_of_range_address_rejected(self):
        with self.assertRaises(RelocationError): apply_relocations(bytes(4),[],{},0xffffffff)
        with self.assertRaises(RelocationError): apply_relocations(bytes(4),[self.rel()],{'target':-1},0x400000)

    def test_absolute_padding_relocation_ignores_no_bytes(self):
        out,audit=apply_relocations(b'abcd',[self.rel(0)],{},0x400000)
        self.assertEqual(out,b'abcd'); self.assertEqual(audit,[])

    def test_size_mismatch_is_not_100_percent(self):
        r=compare_bytes(b'abc',b'abcd')
        self.assertFalse(r['strict_exact']); self.assertEqual(r['match_percent'],75)
        self.assertEqual(r['ignored_bytes'],0)

    def test_empty_functions_never_exact(self):
        self.assertFalse(compare_bytes(b'',b'')['strict_exact'])

    def test_wrong_bound_callee_breaks_exact_comparison(self):
        ref,_=apply_relocations(bytes(4),[self.rel(0x14)],{'target':0x500000},0x400000)
        wrong,_=apply_relocations(bytes(4),[self.rel(0x14)],{'target':0x500001},0x400000)
        self.assertFalse(compare_bytes(ref,wrong)['strict_exact'])


class NativeCandidateTests(unittest.TestCase):
    @unittest.skipUnless(shutil.which('clang++') or shutil.which('g++'),'native C++ compiler unavailable')
    def test_fourteen_candidate_behavior_scenarios(self):
        cc=shutil.which('clang++') or shutil.which('g++')
        with tempfile.TemporaryDirectory() as temp:
            exe=Path(temp)/'accounting-model'
            subprocess.run([cc,'-std=c++98','-Wall','-Wextra','-pedantic',
                            str(ROOT/'samples/allocation/test_accounting.cpp'),'-o',str(exe)],
                           check=True,capture_output=True,text=True,timeout=60)
            run=subprocess.run([str(exe)],check=True,capture_output=True,text=True,timeout=10)
            self.assertIn('14 accounting candidate scenarios passed',run.stdout)


if __name__ == '__main__':
    unittest.main()
