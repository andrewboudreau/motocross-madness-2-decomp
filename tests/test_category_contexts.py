"""Input-free category evidence tests. No game or Microsoft SDK is required."""
from pathlib import Path
import copy
import hashlib
import json
import struct
import tempfile
import unittest
from unittest.mock import patch

from mcm2tool.allocation import Insn, PEImage
from mcm2tool.category_contexts import (analyze_categories, branch_target, preserves_stack_argument,
                                recover_argument, successors, validate_build, walk)
from tools.build_category_contexts import pilot_queue, query_snapshot, write_snapshot
from test_allocation import fixture_bytes, call


def push(va, value=0x4020a0):
    return Insn(va, b'\x68'+struct.pack('<I', value), 'push', hex(value))


def recover(rows, site, branch_entries=(), limit=16):
    ins = {i.va: i for i in rows}
    return recover_argument(site, ins, {i.end: i for i in rows}, set(branch_entries),
                            lambda value: {0x4020a0: 'Terrain'}.get(value), limit)


class ArgumentTests(unittest.TestCase):
    def test_adjacent_literal(self):
        self.assertEqual(recover([push(1), call(6, 99)], 6)['label'], 'Terrain')

    def test_move_from_global_can_intervene(self):
        rows = [push(1), Insn(6, b'123456', 'mov', 'ecx,DWORD PTR ds:0x56df04'), call(12, 99)]
        self.assertEqual(recover(rows, 12)['proof_instructions'], ['0x00000001', '0x00000006'])

    def test_safe_positive_stack_store(self):
        rows = [push(1), Insn(6, b'1234', 'mov', 'DWORD PTR [esp+0x14],eax'), call(10, 99)]
        self.assertEqual(recover(rows, 10)['label'], 'Terrain')

    def test_overwrite_argument_blocks(self):
        for dest in ['DWORD PTR [esp]', 'DWORD PTR [esp+0x1]', 'DWORD PTR [esp-0x4]', 'WORD PTR [esp+0x2]']:
            rows = [push(1), Insn(6, b'1234', 'mov', dest+',eax'), call(10, 99)]
            self.assertIsNone(recover(rows, 10)['label'])

    def test_possible_alias_store_blocks(self):
        self.assertFalse(preserves_stack_argument(Insn(1, b'123', 'mov', 'DWORD PTR [eax+0x4],ecx')))

    def test_esp_write_blocks(self):
        for dest in ['esp', 'sp']:
            self.assertFalse(preserves_stack_argument(Insn(1, b'12', 'mov', dest+',eax')))

    def test_push_register_blocks(self):
        rows = [push(1), Insn(6, b'\x50', 'push', 'eax'), call(7, 99)]
        self.assertIsNone(recover(rows, 7)['label'])

    def test_intervening_call_blocks(self):
        self.assertIsNone(recover([push(1), call(6, 90), call(11, 99)], 11)['label'])

    def test_branch_into_middle_blocks(self):
        rows = [push(1), Insn(6, b'12', 'mov', 'ecx,eax'), call(8, 99)]
        self.assertIsNone(recover(rows, 8, [6])['label'])
        self.assertIsNone(recover(rows, 8, [8])['label'])

    def test_branch_target_at_push_is_allowed(self):
        self.assertEqual(recover([push(1), call(6, 99)], 6, [1])['label'], 'Terrain')

    def test_nonstring_stays_unknown(self):
        self.assertIsNone(recover([push(1, 12), call(6, 99)], 6)['label'])

    def test_bounded_lookback(self):
        rows = [push(1), Insn(6, b'12', 'mov', 'ecx,eax'), call(8, 99)]
        self.assertEqual(recover(rows, 8, limit=1)['reason'], 'lookback_limit')

    def test_decode_gap_blocks(self):
        self.assertEqual(recover([push(1), call(8, 99)], 8)['reason'], 'decode_gap')


class TraversalTests(unittest.TestCase):
    def test_branch_target_uses_bytes(self):
        ins = Insn(100, b'\x74\xf6', 'je', 'wrong_label')
        self.assertEqual(branch_target(ins), 92)
        self.assertEqual(branch_target(Insn(100, b'\x0f\x85'+struct.pack('<i', -8), 'jne', 'wrong')), 98)

    def test_calls_do_not_traverse_callee(self):
        self.assertEqual(successors(call(1, 500))[0], [6])

    def test_unknown_branch_stops(self):
        self.assertEqual(successors(Insn(1, b'\xff\xe0', 'jmp', 'eax')), ([], 'unresolved_branch'))

    def test_restore_boundary_stops_before_call(self):
        ins = {1:call(1, 50), 6:call(6, 60), 11:Insn(11, b'\xc3', 'ret', '')}
        reached, stops = walk(1, ins, {1}, {6:'restore'})
        self.assertEqual(reached, [1]); self.assertIn({'va':'0x00000006','reason':'restore'}, stops)

    def test_other_function_boundary_stops(self):
        ins = {1:Insn(1,b'\x90','nop',''), 2:Insn(2,b'\xc3','ret','')}
        self.assertEqual(walk(1,ins,{1,2})[0],[1])

    def test_loop_terminates(self):
        ins = {1:Insn(1,b'\x75\xfe','jne','1'), 3:Insn(3,b'\xc3','ret','')}
        self.assertEqual(walk(1,ins,{1})[0],[1,3])

    def test_instruction_limit_reported(self):
        reached, stops=walk(1,{1:Insn(1,b'\x90','nop',''),2:Insn(2,b'\xc3','ret','')},{1},limit=1)
        self.assertEqual(reached,[1]); self.assertEqual(stops[0]['reason'],'instruction_limit')

    def test_conditional_both_paths_stop_at_select(self):
        ins={1:Insn(1,b'\x74\x06','je','9'),3:call(3,50),8:Insn(8,b'\xc3','ret',''),9:call(9,60)}
        reached,stops=walk(1,ins,{1},{9:'select'})
        self.assertEqual(reached,[1,3,8]); self.assertIn({'va':'0x00000009','reason':'select'},stops)


class AtlasTests(unittest.TestCase):
    def setUp(self):
        self.temp=tempfile.TemporaryDirectory(); self.addCleanup(self.temp.cleanup)
        self.path=Path(self.temp.name)/'fixture.exe'
        data=fixture_bytes()
        for off, val in [(0x4b0,b'EcoSystem\0'),(0x4c0,b'D:\\aardvark\\VC\\krusty2\\Terrain.cpp\0')]:
            data[off:off+len(val)] = val
        data[0x2f0:0x2f2]=b'\xc3\xc3'
        self.path.write_bytes(data); self.pe=PEImage(self.path)
        self.config={'input_sha256':hashlib.sha256(data).hexdigest(),'functions':[
            {'label':'select_category_return_previous','va':'0x004010f0','size':1,'sha256':hashlib.sha256(b'\xc3').hexdigest()},
            {'label':'restore_category_index','va':'0x004010f1','size':1,'sha256':hashlib.sha256(b'\xc3').hexdigest()}]}
        self.rows=[push(0x401000), call(0x401005,0x4010f0), call(0x40100a,0x401080),
                   call(0x40100f,0x4010f1),call(0x401014,0x401090),Insn(0x401019,b'\xc3','ret',''),
                   push(0x401080,0x4020c0),Insn(0x401085,b'\xc3','ret',''),
                   Insn(0x401090,b'\xc3','ret',''),Insn(0x4010f0,b'\xc3','ret',''),Insn(0x4010f1,b'\xc3','ret','')]

    def run_atlas(self, extra=()):
        with patch('mcm2tool.category_contexts.rtti_evidence',return_value=({}, {}, {}, set(extra))):
            r=analyze_categories(self.pe,self.config,{i.va:i for i in self.rows},'synthetic')
        r['pilot_queue']=pilot_queue(r)
        return r

    def test_join_source_without_ownership(self):
        r=self.run_atlas(); link=r['categories'][0]['source_links'][0]
        self.assertEqual(link['evidence'],'one_hop_context_not_ownership')
        self.assertTrue(link['path'].endswith('Terrain.cpp'))
        self.assertEqual(r['summary']['ownership_assignments'],0)
        self.assertTrue(all(c['original_translation_unit'] is None for c in r['candidates']))

    def test_scope_does_not_include_calls_after_restore(self):
        r=self.run_atlas()
        self.assertEqual(r['categories'][0]['one_hop_candidates'],['0x00401080'])

    def test_shared_helper_retains_multiple_contexts(self):
        self.rows += [push(0x401040,0x4020b0),call(0x401045,0x4010f0),call(0x40104a,0x401080),
                      call(0x40104f,0x4010f1),Insn(0x401054,b'\xc3','ret','')]
        r=self.run_atlas([0x401040]); contexts=r['call_contexts'][0]
        self.assertEqual(contexts['observed_contexts'],['EcoSystem','Terrain'])
        self.assertIsNone(contexts['owner_assignment'])

    def test_support_does_not_inherit_category(self):
        self.rows[2]=call(0x40100a,0x4010f1)
        self.assertEqual(self.run_atlas()['call_contexts'],[])

    def test_no_transitive_fanout(self):
        self.rows = [i for i in self.rows if i.va!=0x401085]
        self.rows += [call(0x401085,0x401090),Insn(0x40108a,b'\xc3','ret','')]
        r=self.run_atlas()
        self.assertEqual(r['categories'][0]['one_hop_candidates'],['0x00401080'])

    def test_ambiguous_containing_candidates_preserved(self):
        self.rows += [Insn(0x401030,b'\xeb\xd3','jmp','0x401005')]
        r=self.run_atlas([0x401030])
        self.assertEqual(r['events'][0]['container_status'],'ambiguous')
        self.assertIsNone(r['events'][0]['label'])  # Jump can bypass the push.

    def test_case_insensitive_category_and_exact_source_query(self):
        r=self.run_atlas()
        self.assertEqual(len(query_snapshot(r,category='terrain')),1)
        self.assertEqual(len(query_snapshot(r,source='terrain.CPP')),1)
        self.assertEqual(query_snapshot(r,source='Terra'),[])
        self.assertEqual(query_snapshot(r,category='missing'),[])

    def test_entry_address_query_and_missing_result(self):
        r=self.run_atlas()
        self.assertEqual(len(query_snapshot(r,address=0x401080)),1)
        self.assertEqual(query_snapshot(r,address=0x1234),[])

    def test_wrong_input_hash_fails(self):
        config=copy.deepcopy(self.config);config['input_sha256']='0'*64
        with self.assertRaises(ValueError):validate_build(self.pe,config)

    def test_changed_selector_hash_fails(self):
        config=copy.deepcopy(self.config);config['functions'][0]['sha256']='0'*64
        with self.assertRaises(ValueError):validate_build(self.pe,config)

    def test_duplicate_selector_config_fails(self):
        config=copy.deepcopy(self.config);config['functions'].append(config['functions'][0])
        with self.assertRaises(ValueError):validate_build(self.pe,config)

    def test_navigation_prefers_category_class_and_filename_agreement(self):
        r=self.run_atlas()
        c=next(x for x in r['candidates'] if x['entry_va']=='0x00401000')
        c['source_references']=[{'path':'D:\\test\\Terrain.cpp'}]
        c['vtable_uses']=[{'class':'Terrain'}]
        self.assertEqual(pilot_queue(r)[0]['navigation_tier'],0)
        c['vtable_uses']=[{'class':'Unrelated'}]
        self.assertEqual(pilot_queue(r)[0]['navigation_tier'],1)

    def test_json_and_packet_output_deterministic(self):
        r=self.run_atlas(); a=Path(self.temp.name)/'a'; b=Path(self.temp.name)/'b'
        write_snapshot(r,a);write_snapshot(self.run_atlas(),b)
        names=[str(p.relative_to(a)) for p in a.rglob('*') if p.is_file()]
        self.assertIn('packets/00_Terrain.json',names)
        self.assertTrue(all((a/n).read_bytes()==(b/n).read_bytes() for n in names))
        self.assertEqual(json.loads((a/'work_queue.json').read_text())[0]['owner_assignment'],None)


if __name__=='__main__':unittest.main()
