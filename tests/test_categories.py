"""Synthetic category joins; no game/SDK inputs or network access required."""
from pathlib import Path
from types import SimpleNamespace
import hashlib
import struct
import tempfile
import unittest
from unittest.mock import patch

from test_allocation import fixture_bytes, call
from mcm2tool.pe import PEImage
from mcm2tool.allocation import Insn
from mcm2tool.categories import (CategoryError, branch_entries, build_categories,
    candidate_ranges, decode_sources, number, primary_class_uses, recover_argument,
    range_body, render_report, safe_argument_gap, validate_inputs)


class CategoryTests(unittest.TestCase):
    def setUp(self):
        self.temp = tempfile.TemporaryDirectory(); self.addCleanup(self.temp.cleanup)
        self.path = Path(self.temp.name)/'fixture.exe'
        self.data = fixture_bytes()
        for off, text in [(0x4b0, b'UI\0'), (0x4c0, b'D:\\aardvark\\VC\\krusty2\\Loader.cpp\0')]:
            self.data[off:off+len(text)] = text
        self.data[0x300] = 0xc3
        self.selector = 0x401100
        self.refresh()

    def refresh(self):
        self.path.write_bytes(self.data); self.pe = PEImage(self.path)
        digest = hashlib.sha256(self.pe.data).hexdigest()
        self.config = {'input_sha256': digest, 'category_select_va': hex(self.selector),
                       'functions': [{'va': hex(self.selector), 'size': 1, 'sha256': hashlib.sha256(b'\xc3').hexdigest()}]}
        self.snapshot = {'schema_version': 1, 'input': {'sha256': digest, 'preferred_image_base': '0x00400000'}, 'functions': []}

    def push(self, va=0x401000, pointer=0x4020a0):
        return Insn(va, b'\x68'+struct.pack('<I', pointer), 'push', hex(pointer))

    def recover(self, rows, incoming=(), **kw):
        return recover_argument(rows[-1], {r.end:r for r in rows}, set(incoming), self.pe, **kw)

    def install(self, rows, ranges=None):
        for i in rows:
            off = self.pe.va_to_offset(i.va); self.data[off:off+len(i.raw)] = i.raw
        self.refresh()
        self.snapshot['functions'] = [{'entry_va': hx, 'decoded_ranges': [[hex(a),hex(b)] for a,b in rs]}
                                      for hx,rs in (ranges or [('0x00401000', [(rows[0].va, rows[-1].end)])])]
        return {i.va:i for i in rows}

    def build(self, rows, ranges=None):
        ins = self.install(rows, ranges)
        with patch('mcm2tool.categories.primary_class_uses', return_value={}):
            return build_categories(self.pe,self.config,self.snapshot,ins)

    def test_immediate_argument(self):
        r=self.recover([self.push(),call(0x401005,self.selector)])
        self.assertEqual(r['label'],'Terrain'); self.assertEqual(r['intervening_instructions'],[])

    def test_skip_stack_local_write_not_argument(self):
        mov=Insn(0x401005,b'\x89\x5c\x24\x04','mov','DWORD PTR [esp+0x4],ebx')
        r=self.recover([self.push(),mov,call(mov.end,self.selector)])
        self.assertEqual(r['label'],'Terrain'); self.assertEqual(r['intervening_instructions'],['0x00401005'])

    def test_top_argument_write_rejected(self):
        for operand in ['DWORD PTR [esp],ebx','DWORD PTR [esp+0x0],ebx','DWORD PTR [esp+0x3],ebx']:
            self.assertFalse(safe_argument_gap(Insn(1,b'123','mov',operand)))

    def test_unknown_alias_write_rejected(self):
        mov=Insn(0x401005,b'\x89\x08','mov','DWORD PTR [eax],ecx')
        self.assertIsNone(self.recover([self.push(),mov,call(mov.end,self.selector)])['label'])

    def test_ebp_store_not_assumed_safe(self):
        self.assertFalse(safe_argument_gap(Insn(1,b'123','mov','DWORD PTR [ebp+0x8],ebx')))

    def test_register_and_memory_read_moves_allowed(self):
        for operand in ['eax,ecx','ecx,DWORD PTR ds:0x402000','edx,DWORD PTR [eax+0x38]']:
            self.assertTrue(safe_argument_gap(Insn(1,b'123','mov',operand)))

    def test_esp_write_and_stack_change_rejected(self):
        for op,args in [('mov','esp,eax'),('add','esp,0x4'),('pop','eax'),('lea','esp,[esp+0x4]')]:
            self.assertFalse(safe_argument_gap(Insn(1,b'123',op,args)))

    def test_register_push_unresolved(self):
        self.assertIsNone(self.recover([Insn(0x401000,b'\x52','push','edx'),call(0x401001,self.selector)])['label'])

    def test_non_string_pointer_unresolved(self):
        self.assertIsNone(self.recover([self.push(pointer=0x700000),call(0x401005,self.selector)])['label'])

    def test_does_not_cross_call(self):
        r=self.recover([self.push(),call(0x401005,0x401080),call(0x40100a,self.selector)])
        self.assertIsNone(r['label'])

    def test_incoming_branch_at_call_or_gap_rejected(self):
        mov=Insn(0x401005,b'\x8b\xc8','mov','ecx,eax')
        for address in [mov.va,mov.end]:
            r=self.recover([self.push(),mov,call(mov.end,self.selector)],[address])
            self.assertEqual(r['reason'],'incoming_branch_can_bypass_argument')

    def test_branch_into_push_is_allowed(self):
        r=self.recover([self.push(),call(0x401005,self.selector)],[0x401000])
        self.assertEqual(r['label'],'Terrain')

    def test_backward_bounds(self):
        mov=Insn(0x401005,b'\x8b\xc8','mov','ecx,eax')
        rows=[self.push(),mov,call(mov.end,self.selector)]
        self.assertEqual(self.recover(rows,max_instructions=1)['reason'],'backward_instruction_limit')
        self.assertEqual(self.recover(rows,max_bytes=1)['reason'],'backward_byte_limit')

    def test_branch_target_collection(self):
        ins=[Insn(1,b'12','jne','0x402000'),Insn(3,b'12','jmp','eax'),call(5,0x9999)]
        self.assertEqual(branch_entries({i.va:i for i in ins}),{0x402000})

    def test_input_hash_schema_base_and_selector_fail_closed(self):
        validate_inputs(self.pe,self.config,self.snapshot)
        for what in ['hash','schema','base','selector','range','missing']:
            import copy
            c,p=copy.deepcopy(self.config),copy.deepcopy(self.snapshot)
            if what=='hash':p['input']['sha256']='0'*64
            elif what=='schema':p['schema_version']=99
            elif what=='base':p['input']['preferred_image_base']='0x500000'
            elif what=='selector':c['functions'][0]['sha256']='0'*64
            elif what=='range':c['functions'][0]['size']=-1
            elif what=='missing':c['functions']=[]
            with self.assertRaises(CategoryError):validate_inputs(self.pe,c,p)

    def test_address_validation(self):
        self.assertEqual(number('0x401000'),0x401000)
        for value in [True,-1,0x100000000,3.5]:
            with self.assertRaises(CategoryError):number(value)

    def test_range_gap_or_midinstruction_rejected(self):
        i=call(0x401000,self.selector)
        for ranges in [[(i.va,i.end+1)],[(i.va,i.end-1)]]:
            with self.assertRaises(CategoryError):range_body(ranges,{i.va:i})

    def test_duplicate_candidate_and_overlap_rejected(self):
        row={'entry_va':'0x401000','decoded_ranges':[['0x401000','0x401001']]}
        ins={0x401000:Insn(0x401000,b'\xc3','ret','')}
        with self.assertRaises(CategoryError):candidate_ranges({'functions':[row,row]},ins)
        row['decoded_ranges']=[['0x401000','0x401005'],['0x401000','0x401004']]
        with self.assertRaises(CategoryError):candidate_ranges({'functions':[row]},ins)

    def test_fresh_source_reference_not_cached_source_claim(self):
        rows=[self.push(),call(0x401005,self.selector),self.push(0x40100a,0x4020c0),Insn(0x40100f,b'\xc3','ret','')]
        ins=self.install(rows)
        self.snapshot['functions'][0]['source_references']=[{'path':'Forged.cpp'}]
        with patch('mcm2tool.categories.primary_class_uses',return_value={}):
            out=build_categories(self.pe,self.config,self.snapshot,ins)
        edge=out['source_category_edges'][0]
        self.assertEqual(edge['source_path'],'D:\\aardvark\\VC\\krusty2\\Loader.cpp')
        self.assertFalse(edge['exclusive_ownership']);self.assertIsNone(out['candidates'][0]['original_translation_unit'])

    def test_multiple_categories_preserved(self):
        rows=[self.push(),call(0x401005,self.selector),self.push(0x40100a,0x4020b0),call(0x40100f,self.selector),Insn(0x401014,b'\xc3','ret','')]
        r=self.build(rows)
        self.assertEqual(r['candidates'][0]['category_labels'],['Terrain','UI'])
        self.assertEqual(r['summary']['mixed_category_candidates'],1)
        self.assertIsNone(r['candidates'][0]['exclusive_category'])

    def test_no_propagation_to_callee(self):
        rows=[self.push(),call(0x401005,self.selector),call(0x40100a,0x401050),Insn(0x40100f,b'\xc3','ret',''),Insn(0x401050,b'\xc3','ret','')]
        r=self.build(rows,[('0x401000',[(0x401000,0x401010)]),('0x401050',[(0x401050,0x401051)])])
        self.assertEqual(len(r['candidates']),1)
        self.assertEqual(r['summary']['callee_category_propagations'],0)

    def test_ambiguous_containers_do_not_create_source_edges(self):
        rows=[self.push(),call(0x401005,self.selector),self.push(0x40100a,0x4020c0),Insn(0x40100f,b'\xc3','ret','')]
        r=self.build(rows,[('0x400fff',[(0x401000,0x401010)]),('0x401000',[(0x401000,0x401010)])])
        self.assertEqual(r['call_sites'][0]['container_status'],'ambiguous_candidates')
        self.assertEqual(r['source_category_edges'],[])

    def test_unmapped_site_retained(self):
        rows=[self.push(),call(0x401005,self.selector)]
        ins=self.install(rows);self.snapshot['functions']=[]
        with patch('mcm2tool.categories.primary_class_uses',return_value={}):
            out=build_categories(self.pe,self.config,self.snapshot,ins)
        self.assertEqual(out['call_sites'][0]['container_status'],'unmapped')
        self.assertEqual(out['call_sites'][0]['label'],'Terrain')

    def test_calls_rechecked_against_image(self):
        rows=[self.push(),call(0x401005,self.selector)]
        ins=self.install(rows);self.data[0x206]=0;self.refresh()
        with patch('mcm2tool.categories.primary_class_uses',return_value={}):
            with self.assertRaises(CategoryError):build_categories(self.pe,self.config,self.snapshot,ins)

    def test_no_category_from_source_filename(self):
        rows=[Insn(0x401000,b'\x52','push','edx'),call(0x401001,self.selector),self.push(0x401006,0x4020c0),Insn(0x40100b,b'\xc3','ret','')]
        out=self.build(rows)
        self.assertEqual(out['summary']['literal_labeled_sites'],0)
        self.assertEqual(out['source_category_edges'],[])
        self.assertEqual(out['candidates'][0]['review_lane'],'unresolved_category_argument')

    def test_report_and_repeated_join_deterministic(self):
        rows=[self.push(),call(0x401005,self.selector),Insn(0x40100a,b'\xc3','ret','')]
        a=self.build(rows);b=self.build(rows)
        self.assertEqual(a,b);self.assertEqual(render_report(a),render_report(b))
        self.assertIn('not exclusive source ownership',render_report(a))

    def test_inherited_class_use_not_duplicated(self):
        def cls(name,vt,base=None):
            return SimpleNamespace(name=name,direct_bases=[base] if base else [],
                bases=[SimpleNamespace(name=name)] + ([SimpleNamespace(name=base,mdisp=0,pdisp=-1)] if base else []),
                vtable_records=[SimpleNamespace(object_offset=0,vtables=[vt])])
        struct.pack_into('<2I',self.data,0x500,0x401000,0)
        struct.pack_into('<2I',self.data,0x520,0x401000,0)
        pe=self.load_pe()
        with patch('mcm2tool.categories.parse_rtti',return_value=[cls('Parent',0x402100),cls('Child',0x402120,'Parent')]):
            out=primary_class_uses(pe)
        self.assertEqual([r['class'] for r in out[0x401000]],['Parent'])

    def load_pe(self):
        self.path.write_bytes(self.data);return PEImage(self.path)

    def test_secondary_vtable_ignored(self):
        cls=SimpleNamespace(name='Child',direct_bases=[],bases=[],
            vtable_records=[SimpleNamespace(object_offset=12,vtables=[0x402100])])
        with patch('mcm2tool.categories.parse_rtti',return_value=[cls]):
            self.assertEqual(dict(primary_class_uses(self.pe)),{})


if __name__=='__main__':unittest.main()
