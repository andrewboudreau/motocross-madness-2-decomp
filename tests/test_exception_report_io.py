"""Actual reporter/helper/OS-backend linkage and file I/O under native SEH."""
import os
from pathlib import Path
import re
import subprocess
import sys
import tempfile
import unittest

from mcm2tool.coff import CoffObject
from mcm2tool.toolchain import find_child_ci, find_vc6_bin, infer_vc98_root

ROOT = Path(__file__).resolve().parents[1]


@unittest.skipUnless(os.name == 'nt' and os.environ.get('VC6_ROOT'),
                     'requires native Windows and authentic VC6_ROOT')
class ExceptionReportIoTests(unittest.TestCase):
    def test_real_log_nested_seh_and_one_shot(self):
        root = Path(os.environ['VC6_ROOT']).resolve()
        linker = find_child_ci(find_vc6_bin(root), 'link.exe')
        libraries = find_child_ci(infer_vc98_root(root), 'lib')
        with tempfile.TemporaryDirectory(prefix='mcm2-exception-io-') as directory:
            output = Path(directory)
            objects = []
            definitions = {}
            for source in ('samples/game/RecordExceptionInfo.cpp',
                           'samples/game/ThreadStackBase.cpp',
                           'src/reconstructed/ExceptionHandler.cpp',
                           'tests/fixtures/exception_report_io.cpp'):
                obj = output / (Path(source).stem + '.obj')
                result = subprocess.run([
                    sys.executable, str(ROOT / 'tools/compile.py'), str(ROOT / source),
                    '-o', str(obj), '--vc6-root', str(root), '--profile', 'vc6_o2_mt',
                ], cwd=ROOT, capture_output=True, text=True, encoding='latin-1',
                    env={**os.environ, 'PYTHONPATH': str(ROOT)})
                self.assertEqual(result.returncode, 0, result.stdout + result.stderr)
                objects.append(str(obj))
                definitions[Path(source).stem] = {
                    s.name for s in CoffObject(obj).symbols
                    if s.section_number > 0 and s.storage_class == 2 and s.type == 0x20
                }
            expected = {
                'RecordExceptionInfo': '?RecordExceptionInfo@@YAHPAU_EXCEPTION_POINTERS@@PBD@Z',
                'ThreadStackBase': '?ReadCurrentThreadStackBase@@YAPAKXZ',
                'ExceptionHandler': '?RecordModuleList@@YAXPAX@Z',
            }
            for unit, symbol in expected.items():
                self.assertIn(symbol, definitions[unit])
                self.assertNotIn(symbol, definitions['exception_report_io'])

            executable = output / 'exception-io.exe'
            result = subprocess.run([
                str(linker), '/nologo', '/subsystem:console',
                f'/out:{executable}', f'/libpath:{libraries}', *objects,
                'kernel32.lib', 'user32.lib', 'advapi32.lib',
            ], cwd=ROOT, capture_output=True, text=True, encoding='latin-1')
            self.assertEqual(result.returncode, 0, result.stdout + result.stderr)
            log = output / 'errorlog.txt'
            prefix = b'existing native integration log\r\n'
            log.write_bytes(prefix)
            # A different cwd proves the report uses the executable's directory.
            run_directory = output / 'cwd'
            run_directory.mkdir()
            result = subprocess.run([str(executable), str(log)], cwd=run_directory,
                                    capture_output=True, text=True, timeout=30)
            self.assertEqual(result.returncode, 0, result.stdout + result.stderr)
            self.assertIn('Two native exceptions continued to outer handlers; one report,',
                          result.stdout)
            self.assertFalse((run_directory / 'errorlog.txt').exists())
            data = log.read_bytes()
            self.assertTrue(data.startswith(prefix + b'\r\n\r\n\r\n\r\n'))
            report = data.decode('latin-1')
            self.assertEqual(report.count('Exception handler called in '), 1)
            self.assertIn('Exception handler called in native-first-report.\r\n', report)
            self.assertNotIn('native-second-report', report)
            self.assertRegex(report, r'exception-io caused an Access Violation in module '
                             r'exception-io\.exe at [0-9a-f]{4}:[0-9a-f]{8}\.\r\n')
            self.assertRegex(report, r'Error occurred at \d+/\d+/\d+ \d+:\d+:\d+\.\r\n')
            self.assertRegex(report, r'exception-io\.exe, run by [^\r\n]+\.\r\n')
            self.assertRegex(report, r'\d+ processor\(s\), type \d+\.\r\n')
            self.assertRegex(report, r'\d+ MBytes physical memory\.\r\n')
            registers = re.findall(r'^REGISTER (.+)$', result.stdout, re.MULTILINE)
            self.assertEqual(len(registers), 4)
            for line in registers:
                self.assertIn(line + '\r\n', report)
            fault = re.search(r'^FAULT ([0-9a-f]{8})$', result.stdout, re.MULTILINE)
            self.assertIsNotNone(fault)
            self.assertIn('Read from location ' + fault.group(1)
                          + ' caused an access violation.\r\n', report)
            code = re.search(r'^CODE (.+)$', result.stdout, re.MULTILINE)
            stack = re.search(r'^STACK (.+)$', result.stdout, re.MULTILINE)
            self.assertIsNotNone(code)
            self.assertIsNotNone(stack)
            self.assertIn('Bytes at CS:EIP:\r\n' + code.group(1) + '\r\n', report)
            self.assertIn('Stack dump:\r\n' + stack.group(1) + '\r\n', report)
            self.assertNotIn('Exception encountered during stack dump.', report)
            self.assertIn('Module list: names, addresses, sizes, time stamps and file times:', report)
            self.assertRegex(report, r'exception-io\.exe, loaded at 0x[0-9a-f]{8} - '
                             r'\d+ bytes - [0-9a-f]{8} - file date is ')
            self.assertRegex(report.lower(), r'ntdll\.dll, loaded at 0x[0-9a-f]{8} - \d+ bytes')


if __name__ == '__main__':
    unittest.main()
