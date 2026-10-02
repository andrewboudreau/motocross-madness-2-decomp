import os
from pathlib import Path
import subprocess
import unittest
from unittest.mock import patch

from mcm2tool.vc6_runtime import executable_command, runner_kind, windows_path


class Vc6RuntimeTests(unittest.TestCase):
    def test_auto_prefers_wibo_even_when_wine_is_installed(self):
        with patch('mcm2tool.vc6_runtime.os.name', 'posix'), \
             patch.dict(os.environ, {'VC6_RUNNER': 'auto'}), \
             patch('mcm2tool.vc6_runtime.shutil.which', return_value='/bin/wibo'):
            self.assertEqual(runner_kind(), 'wibo')

    def test_explicit_wine_remains_available(self):
        with patch('mcm2tool.vc6_runtime.os.name', 'posix'), \
             patch.dict(os.environ, {'VC6_RUNNER': 'wine'}):
            self.assertEqual(runner_kind(), 'wine')

    def test_invalid_runner_is_rejected(self):
        with patch('mcm2tool.vc6_runtime.os.name', 'posix'), \
             patch.dict(os.environ, {'VC6_RUNNER': 'clang'}):
            with self.assertRaises(ValueError):
                runner_kind()

    def test_native_windows_does_not_need_a_host_runner(self):
        with patch('mcm2tool.vc6_runtime.os.name', 'nt'), \
             patch.dict(os.environ, {'VC6_RUNNER': 'wibo'}):
            self.assertEqual(runner_kind(), 'windows')

    def test_wibo_loads_host_executable_without_invoking_winepath(self):
        program = Path('private tree/VC98/BIN/CL.EXE').resolve()
        with patch('mcm2tool.vc6_runtime.shutil.which', return_value='/bin/wibo') as which, \
             patch('mcm2tool.vc6_runtime.subprocess.run') as run:
            self.assertEqual(executable_command(program, 'wibo'), ['/bin/wibo', str(program)])
            which.assert_called_once_with('wibo')
            run.assert_not_called()

    def test_guest_arguments_use_wibo_path_conversion(self):
        path = Path('source with spaces.cpp').resolve()
        result = subprocess.CompletedProcess([], 0, 'Z:\\source with spaces.cpp\n')
        with patch('mcm2tool.vc6_runtime.shutil.which', return_value='/bin/wibo'), \
             patch('mcm2tool.vc6_runtime.subprocess.run', return_value=result) as run:
            self.assertEqual(windows_path(path, 'wibo'), 'Z:\\source with spaces.cpp')
            self.assertEqual(run.call_args.args[0], ['/bin/wibo', 'path', '-w', str(path)])
            self.assertTrue(run.call_args.kwargs['check'])

    def test_missing_wibo_fails_instead_of_using_another_compiler(self):
        program = Path('CL.EXE')
        with patch('mcm2tool.vc6_runtime.shutil.which', return_value=None):
            with self.assertRaises(FileNotFoundError):
                executable_command(program, 'wibo')
