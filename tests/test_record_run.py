import json
import os
from pathlib import Path
import tempfile
import time
import unittest

from tools.build_game import source_revision
from tools.record_run import ROOT, record, sha256_file

LOG = """\
20:03:50.952 mcm2 runtime d3drm.dll loaded into C:\\run\\mcm2.exe (Windows reports 6.2 build 9200)
20:03:50.960 executable sha256 {old} (UNKNOWN build; hooks stay off)
20:04:00.000 ExitProcess(1) called from 0x0053b38e
20:55:30.979 mcm2 runtime d3drm.dll loaded into C:\\run\\mcm2.exe (Windows reports 6.2 build 9200)
20:55:30.990 executable sha256 {new} (UNKNOWN build; hooks stay off)
20:55:30.991 recorded a portable full install at C:\\run
20:55:31.000 first D3DRMVectorRotate call (camera is running)
20:59:00.918 ExitProcess(1) called from 0x0053b38e
20:59:00.921 unloading after 44134 D3DRMVectorRotate calls
"""


class RecordRunTests(unittest.TestCase):
    def test_last_session_build_and_frames(self):
        with tempfile.TemporaryDirectory() as tmp:
            run, builds = Path(tmp) / 'run', Path(tmp) / 'builds'
            (run / 'frames').mkdir(parents=True)
            exe = run / 'mcm2.exe'
            exe.write_bytes(b'MZ source build')
            sha = sha256_file(exe)
            build = builds / 'build-x'
            build.mkdir(parents=True)
            (build / 'report.json').write_text(json.dumps(
                {'link': {'sha256': sha}, 'source_revision': {'commit': 'abc', 'dirty': False}}))
            (builds / 'build-y').mkdir()
            (builds / 'build-y' / 'report.json').write_text(json.dumps({'link': {'sha256': '0' * 64}}))
            log = run / 'mcm2_runtime.log'
            log.write_text(LOG.format(old='1' * 64, new=sha))
            day = time.mktime(time.strptime('2026-11-05 21:00:00', '%Y-%m-%d %H:%M:%S'))
            os.utime(log, (day, day))
            for name, when in (('frame_old.bmp', day - 3600), ('frame_005s.bmp', day - 120)):
                (run / 'frames' / name).write_bytes(b'BM')
                os.utime(run / 'frames' / name, (when, when))
            errors = run / 'errorlog.txt'
            errors.write_text('Read from location 00000000 caused an access violation.\n')
            os.utime(errors, (day - 3600, day - 3600))

            result = record(run, builds)

        self.assertEqual(result['build']['commit'], 'abc')
        self.assertTrue(result['build']['directory'].endswith('build-x'))
        session = result['session']
        self.assertEqual(session['start'], '20:55:30.979')
        self.assertEqual(session['date'], '2026-11-05T20:55:30')
        self.assertTrue(session['ran_this_exe'])
        self.assertEqual(session['hooks'], 'on')  # forced: it says "stay off" but installed
        self.assertTrue(session['camera_ran'])
        self.assertEqual(session['last_line'], 'unloading after 44134 D3DRMVectorRotate calls')
        self.assertEqual(result['frames']['names'], ['frame_005s.bmp'])
        self.assertEqual(result['errorlog']['access_violations'], 1)
        self.assertFalse(result['errorlog']['written_during_session'])

    def test_source_revision_names_a_commit(self):
        revision = source_revision(ROOT)
        if revision['commit'] is not None:
            self.assertRegex(revision['commit'], r'^[0-9a-f]{40}$')
            self.assertIsInstance(revision['dirty'], bool)


if __name__ == '__main__':
    unittest.main()
