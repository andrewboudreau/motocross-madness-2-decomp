"""Input-free lifetime witnesses and native C++ behavior models."""
from pathlib import Path
from types import SimpleNamespace
import hashlib
import shutil
import subprocess
import struct
import tempfile
import unittest
from unittest.mock import patch

from mcm2tool.allocation import Insn
from mcm2tool.category_lifetimes import normal_exit_paths
from tools.review_category_pilots import validate_range, build_review

ROOT=Path(__file__).resolve().parents[1]
SELECT=0x1000
RESTORE=0x2000

def call(va,target):return Insn(va,b'\xe8'+struct.pack('<i',target-va-5),'call',hex(target))
def ret(va):return Insn(va,b'\xc3','ret','')
def state_rows(rows,**kwargs):return normal_exit_paths(rows,SELECT,RESTORE,**kwargs)

class LifetimeTests(unittest.TestCase):
    def test_no_selection(self):
        r=state_rows([ret(1)])
        self.assertEqual(r['normal_returns'][0]['local_history'],'not_selected')

    def test_select_and_restore(self):
        r=state_rows([call(1,SELECT),call(6,RESTORE),ret(11)])
        self.assertEqual(r['normal_returns'][0]['local_history'],'local_restore_after_select')
        self.assertEqual(r['normal_returns'][0]['witness_instructions'],['0x00000001','0x00000006','0x0000000b'])

    def test_selected_without_local_restore(self):
        r=state_rows([call(1,SELECT),ret(6)])
        self.assertEqual(r['normal_returns'][0]['local_history'],'selected_without_local_restore')
        self.assertFalse(r['callee_effects_modeled'])

    def test_unknown_call_cannot_become_known_restore(self):
        r=state_rows([call(1,SELECT),call(6,0x3000),ret(11)])
        self.assertEqual(r['normal_returns'][0]['local_history'],'selected_without_local_restore')

    def test_restore_without_select_is_distinct(self):
        self.assertEqual(state_rows([call(1,RESTORE),ret(6)])['normal_returns'][0]['local_history'],'local_restore_without_select')

    def test_reselect_after_restore_requires_another_restore(self):
        r=state_rows([call(1,SELECT),call(6,RESTORE),call(11,SELECT),ret(16)])
        self.assertEqual(r['normal_returns'][0]['local_history'],'selected_without_local_restore')

    def test_does_not_claim_saved_value_equivalence(self):
        self.assertFalse(state_rows([call(1,SELECT),call(6,RESTORE),ret(11)])['restore_argument_equivalence_proven_by_this_analysis'])

    def test_one_return_can_have_two_histories(self):
        # JZ directly to RET can bypass the restore call.
        rows=[call(1,SELECT),Insn(6,b'\x74\x05','je','0xd'),call(8,RESTORE),ret(13)]
        r=state_rows(rows)
        self.assertEqual({x['local_history'] for x in r['normal_returns']},{'selected_without_local_restore','local_restore_after_select'})
        self.assertFalse(r['branch_predicates_solved'])

    def test_bypassed_select_retains_not_selected_history(self):
        rows=[Insn(1,b'\x74\x05','je','0x8'),call(3,SELECT),ret(8)]
        self.assertEqual({x['local_history'] for x in state_rows(rows)['normal_returns']},{'not_selected','selected_without_local_restore'})

    def test_loop_terminates_without_counting_as_new_function(self):
        rows=[call(1,SELECT),Insn(6,b'\x75\xf9','jne','0x1'),call(8,RESTORE),ret(13)]
        r=state_rows(rows)
        self.assertEqual(len(r['normal_returns']),1)
        self.assertLess(r['visited_abstract_states'],20)

    def test_unresolved_indirect_jump_blocks_completion(self):
        r=state_rows([Insn(1,b'\xff\xe0','jmp','eax')])
        self.assertEqual(r['normal_returns'],[])
        self.assertEqual(r['unresolved_or_abnormal_stops'][0]['reason'],'unresolved_branch')

    def test_indirect_call_is_not_traversed(self):
        rows=[call(1,SELECT),Insn(6,b'\xff\xd0','call','eax'),call(8,RESTORE),ret(13)]
        r=state_rows(rows)
        self.assertEqual(r['normal_returns'][0]['local_history'],'local_restore_after_select')
        self.assertFalse(r['callee_effects_modeled'])

    def test_tail_branch_outside_range_is_explicit(self):
        r=state_rows([Insn(1,b'\xeb\x7f','jmp','0x82')])
        self.assertEqual(r['unresolved_or_abnormal_stops'][0]['reason'],'outside_reviewed_range')
        self.assertEqual(r['normal_returns'],[])

    def test_trap_is_not_normal_return(self):
        r=state_rows([Insn(1,b'\xcc','int3','')])
        self.assertEqual(r['normal_returns'],[])

    def test_rep_ret(self):
        r=state_rows([Insn(1,b'\xf3\xc3','repz','ret')])
        self.assertEqual(r['normal_returns'][0]['return_va'],'0x00000001')

    def test_state_limit_reported(self):
        r=state_rows([call(1,SELECT),ret(6)],limit=1)
        self.assertEqual(r['normal_returns'],[])
        self.assertEqual(r['unresolved_or_abnormal_stops'][0]['reason'],'state_limit')

    def test_empty_body_and_conflicting_boundaries_rejected(self):
        with self.assertRaises(ValueError):state_rows([])
        with self.assertRaises(ValueError):normal_exit_paths([ret(1)],SELECT,SELECT)
        with self.assertRaises(ValueError):state_rows([ret(1)],limit=0)

    def test_duplicate_instructions_rejected(self):
        with self.assertRaises(ValueError):state_rows([ret(1),ret(1)])

    def test_decode_gap_rejected(self):
        with self.assertRaises(ValueError):state_rows([call(1,SELECT),ret(7)])

    def test_determinism(self):
        rows=[call(1,SELECT),call(6,RESTORE),ret(11)]
        self.assertEqual(state_rows(rows),state_rows(rows))

    def test_exception_edges_never_claimed(self):
        self.assertFalse(state_rows([ret(1)])['exception_edges_modeled'])

class ReviewValidationTests(unittest.TestCase):
    def test_hash_checked_before_rtti_or_disassembly_use(self):
        pe=SimpleNamespace(machine=0x14c,data=b'fixture')
        with self.assertRaises(ValueError):build_review(pe,{'schema_version':1,'input_sha256':'0'*64},{})

    def test_machine_and_schema_fail_closed(self):
        pe=SimpleNamespace(machine=0x8664,data=b'fixture')
        cfg={'schema_version':1,'input_sha256':hashlib.sha256(pe.data).hexdigest()}
        with self.assertRaises(ValueError):build_review(pe,cfg,{})
        pe.machine=0x14c;cfg['schema_version']=99
        with self.assertRaises(ValueError):build_review(pe,cfg,{})

    def test_reviewed_range_hash(self):
        spec={'va':'0x00401000','size':1,'sha256':hashlib.sha256(b'\xc3').hexdigest()}
        with patch('tools.review_category_pilots.read_va',return_value=b'\xc3'):validate_range(None,spec)
        with patch('tools.review_category_pilots.read_va',return_value=b'\x90'):
            with self.assertRaises(ValueError):validate_range(None,spec)

    def test_invalid_range_size_rejected(self):
        for size in [0,-1,65537,True,1.5]:
            with self.assertRaises(ValueError):validate_range(None,{'va':'0x00401000','size':size,'sha256':'0'*64})

class NativeModels(unittest.TestCase):
    @unittest.skipUnless(shutil.which('clang++') or shutil.which('g++'),'native C++ compiler unavailable')
    def test_terrain_native_behavior_scenarios(self):
        compiler=shutil.which('clang++') or shutil.which('g++')
        with tempfile.TemporaryDirectory() as temp:
            exe=Path(temp)/'pilots'
            subprocess.run([compiler,'-std=c++98','-Wall','-Wextra','-pedantic',str(ROOT/'samples/category_pilots/test_models.cpp'),'-o',str(exe)],check=True,capture_output=True,text=True,timeout=45)
            r=subprocess.run([str(exe)],check=True,capture_output=True,text=True,timeout=10)
            self.assertIn('15 native pilot model scenarios passed',r.stdout)

if __name__=='__main__':unittest.main()
