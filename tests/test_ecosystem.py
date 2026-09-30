"""Synthetic verifier tests and execution of our candidate algorithm only."""
from pathlib import Path
from types import SimpleNamespace
import copy
import hashlib
import json
import shutil
import struct
import subprocess
import tempfile
import unittest
from unittest.mock import patch

from test_allocation import fixture_bytes, call
from mcm2tool.pe import PEImage
from mcm2tool.allocation import Insn
from mcm2tool.coff import CoffObject
from mcm2tool.resolved_match import match_object
from tools.review_ecosystem import validate_review, validate_range, render_report

ROOT=Path(__file__).resolve().parents[1]
def sha(b):return hashlib.sha256(b).hexdigest()

class ReviewTests(unittest.TestCase):
    def setUp(self):
        self.temp=tempfile.TemporaryDirectory();self.addCleanup(self.temp.cleanup)
        p=Path(self.temp.name)/'fixture.exe';data=fixture_bytes()
        c=call(0x401000,0x401020);data[0x200:0x205]=c.raw;data[0x205]=0xc3
        data[0x220:0x223]=b'\xff\x01\xc3'
        struct.pack_into('<I',data,0x500,0x401000)
        struct.pack_into('<f',data,0x580,0.5)
        p.write_bytes(data);self.pe=PEImage(p)
        self.instructions={c.va:c,0x401005:Insn(0x401005,b'\xc3','ret','')}
        self.config={'schema_version':1,'input_sha256':sha(data),
          'target':{'va':'0x00401000','size':6,'sha256':sha(data[0x200:0x206]),'class':'EcoSystem','vtable':'0x00402100','slot':0},
          'helper':{'va':'0x00401020','size':3,'sha256':sha(data[0x220:0x223])},
          'blocks':[{'id':'E00','va':'0x00401000','end':'0x00401005','sha256':sha(c.raw)},
                    {'id':'E01','va':'0x00401005','end':'0x00401006','sha256':sha(b'\xc3')}],
          'expected_calls':[{'site':'0x00401000','target':'0x00401020'}],
          'bindings':[{'symbol':'callback','va':'0x00401020','kind':'direct_call'}],
          'literals':[{'va':'0x004020a0','value':'Terrain'}],
          'constants':[{'va':'0x00402180','format':'f','hex':struct.pack('<f',0.5).hex()}]}
        self.cls=SimpleNamespace(name='EcoSystem',vtable_records=[SimpleNamespace(object_offset=0,vtables=[0x402100])])

    def run_review(self,cfg=None):
        with patch('tools.review_ecosystem.parse_rtti',return_value=[self.cls]):
            return validate_review(self.pe,cfg or self.config,self.instructions)

    def test_complete_review_and_no_identity_invention(self):
        d,b=self.run_review();self.assertEqual(d['instruction_count'],2);self.assertEqual(b,{'callback':0x401020})
        self.assertFalse(d['numeric_equivalence_proven']);self.assertFalse(d['original_game_executed'])
        self.assertEqual(d['numeric_constants'][0]['value'],0.5)

    def test_wrong_hash_and_schema_fail(self):
        for k,v in [('input_sha256','0'*64),('schema_version',99)]:
            c=copy.deepcopy(self.config);c[k]=v
            with self.assertRaises(ValueError):self.run_review(c)

    def test_target_or_helper_hash_must_match(self):
        for k in ['target','helper']:
            c=copy.deepcopy(self.config);c[k]['sha256']='0'*64
            with self.assertRaises(ValueError):self.run_review(c)

    def test_partition_gap_duplicate_and_split_rejected(self):
        for mode in ['gap','duplicate','split','incomplete','hash']:
            c=copy.deepcopy(self.config)
            if mode=='gap':c['blocks'][1]['va']='0x00401004'
            if mode=='duplicate':c['blocks'][1]['id']='E00'
            if mode=='split':c['blocks'][0]['end']='0x00401004'
            if mode=='incomplete':c['blocks'].pop()
            if mode=='hash':c['blocks'][0]['sha256']='0'*64
            with self.assertRaises(ValueError):self.run_review(c)

    def test_call_contract_rechecked(self):
        c=copy.deepcopy(self.config);c['expected_calls'][0]['target']='0x00401030'
        with self.assertRaises(ValueError):self.run_review(c)

    def test_unobserved_and_duplicate_binding_fail(self):
        for mode in ['unobserved','duplicate','kind']:
            c=copy.deepcopy(self.config)
            if mode=='unobserved':c['bindings'][0]['va']='0x00401030'
            if mode=='duplicate':c['bindings'].append(dict(c['bindings'][0]))
            if mode=='kind':c['bindings'][0]['kind']='guess'
            with self.assertRaises(ValueError):self.run_review(c)

    def test_literals_and_numeric_bits_rechecked(self):
        for mode in ['literal','constant','format']:
            c=copy.deepcopy(self.config)
            if mode=='literal':c['literals'][0]['value']='Fake'
            if mode=='constant':c['constants'][0]['hex']='00000000'
            if mode=='format':c['constants'][0]['format']='i'
            with self.assertRaises(ValueError):self.run_review(c)

    def test_rtti_and_slot_rechecked(self):
        for k,v in [('class','Fake'),('vtable','0x00402104'),('slot',1)]:
            c=copy.deepcopy(self.config);c['target'][k]=v
            with self.assertRaises(ValueError):self.run_review(c)

    def test_decoded_byte_mismatch_rejected(self):
        self.instructions[0x401005]=Insn(0x401005,b'\x90','ret','')
        with self.assertRaises(ValueError):self.run_review()

    def test_deterministic_review_report(self):
        a,_=self.run_review();b,_=self.run_review()
        self.assertEqual(a,b);self.assertEqual(render_report(a),render_report(b))

    def test_invalid_extent_fails(self):
        for size in [0,-1,65537,True,1.5]:
            spec=dict(self.config['target']);spec['size']=size
            with self.assertRaises(ValueError):validate_range(self.pe,spec)

class CandidateTests(unittest.TestCase):
    @unittest.skipUnless(shutil.which('clang++') or shutil.which('g++'),'native C++ unavailable')
    def test_thirty_three_scenarios_in_two_optimization_modes(self):
        cc=shutil.which('clang++') or shutil.which('g++')
        with tempfile.TemporaryDirectory() as temp:
            for optimize in ['-O0','-O2']:
                exe=Path(temp)/'eco-model'
                subprocess.run([cc,'-std=c++98','-Wall','-Wextra','-pedantic',optimize,'-fno-fast-math','-ffp-contract=off',
                                str(ROOT/'samples/ecosystem/test_pass.cpp'),'-o',str(exe)],check=True,capture_output=True,text=True,timeout=45)
                result=subprocess.run([str(exe)],check=True,capture_output=True,text=True,timeout=10)
                self.assertIn('33 EcoSystem algorithm scenarios passed',result.stdout)

    @unittest.skipUnless(shutil.which('clang-cl'),'clang-cl unavailable')
    def test_i386_candidate_has_only_supported_reviewed_relocations(self):
        cfg=json.loads((ROOT/'config/ecosystem_pass.json').read_text())
        bindings={x['symbol']:int(x['va'],16) for x in cfg['bindings']}
        with tempfile.TemporaryDirectory() as temp:
            for key in ['target','helper']:
                spec=cfg[key];p=Path(temp)/(key+'.obj')
                subprocess.run([shutil.which('clang-cl'),*cfg['compile_flags'],str(ROOT/spec['source']),'/Fo'+str(p)],check=True,capture_output=True,text=True,timeout=45)
                obj=CoffObject(p)
                # Synthetic comparator target, not game bytes or a reported match.
                r=match_object(obj,spec['symbol'],int(spec['va'],16),b'\0',bindings)
                self.assertEqual(r['ignored_bytes'],0)
                if key=='target':self.assertGreater(len(r['relocations_applied']),20)
                else:self.assertEqual(len(r['relocations_applied']),0)

if __name__=='__main__':unittest.main()
