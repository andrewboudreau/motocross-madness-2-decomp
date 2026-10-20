from pathlib import Path
import subprocess
import tempfile
from types import SimpleNamespace
import unittest
from unittest.mock import patch

from tools.build_game import link_game, response_file, run_command, source_files, valid_object


class GameBuildTests(unittest.TestCase):
    def test_data_only_providers_are_valid_but_empty_and_wrong_machine_objects_are_not(self):
        with tempfile.TemporaryDirectory() as tmp:
            path = Path(tmp) / 'globals.obj'
            path.write_bytes(b'object validated by COFF parser')
            data = SimpleNamespace(machine=0x14c, symbols=[], sections=[
                SimpleNamespace(raw_size=16, characteristics=0x40)])
            with patch('tools.build_game.CoffObject', return_value=data):
                self.assertTrue(valid_object(path))
                data.machine = 0x8664
                self.assertFalse(valid_object(path))
                data.machine = 0x14c
                data.sections = []
                self.assertFalse(valid_object(path))
                data.symbols = [SimpleNamespace(storage_class=2, section_number=0, value=4)]
                self.assertTrue(valid_object(path))

    def test_source_discovery_includes_both_trees_and_preserves_duplicate_basenames(self):
        with tempfile.TemporaryDirectory() as tmp:
            root = Path(tmp)
            for name in ('src/reconstructed/Bike.cpp', 'src/krusty2/Bike.cpp',
                         'src/reconstructed/Main.cpp', 'samples/probe.cpp'):
                path = root / name
                path.parent.mkdir(parents=True, exist_ok=True)
                path.write_text('')
            sources = source_files(root, {'source_roots': ['src']})
            self.assertEqual([p.relative_to(root).as_posix() for p in sources], [
                'src/krusty2/Bike.cpp', 'src/reconstructed/Bike.cpp',
                'src/reconstructed/Main.cpp'])

    def test_missing_sources_fail(self):
        with tempfile.TemporaryDirectory() as tmp:
            with self.assertRaisesRegex(ValueError, r'no C\+\+ sources'):
                source_files(Path(tmp), {'source_roots': ['missing']})

    def test_failed_link_does_not_accept_existing_executable(self):
        with tempfile.TemporaryDirectory() as tmp:
            root = Path(tmp)
            (root / 'mcm2.exe').write_bytes(b'stale output')
            with patch('tools.build_game.find_vc6_bin', return_value=root), \
                 patch('tools.build_game.find_child_ci', return_value=root / 'link.exe'), \
                 patch('tools.build_game.executable_command', return_value=['link.exe']), \
                 patch('tools.build_game.windows_path', side_effect=lambda p, r: str(p)), \
                 patch('tools.build_game.run_command', return_value={'returncode': 1120}):
                result = link_game(root, [], [], root, 'windows', {})
            self.assertFalse(result['linked'])
            self.assertIsNone(result['executable'])
            response = (root / 'link.rsp').read_text()
            self.assertIn('/SUBSYSTEM:WINDOWS', response)
            self.assertNotIn('/FORCE', response)
            self.assertNotIn('/ENTRY', response)

    def test_response_file_quotes_paths_and_rejects_argument_injection(self):
        with tempfile.TemporaryDirectory() as tmp:
            path = Path(tmp) / 'link.rsp'
            response_file(path, ['path with spaces/Main.obj', '/SUBSYSTEM:WINDOWS'])
            self.assertEqual(path.read_text(),
                             '"path with spaces/Main.obj"\n"/SUBSYSTEM:WINDOWS"\n')
            with self.assertRaises(ValueError):
                response_file(path, ['input.obj\n/FORCE'])

    def test_process_failures_keep_diagnostics(self):
        with tempfile.TemporaryDirectory() as tmp:
            root = Path(tmp)
            failure = subprocess.CompletedProcess([], 2, 'source.cpp\n', 'error C2001\n')
            with patch('tools.build_game.subprocess.run', return_value=failure):
                result = run_command(['cl'], root, {}, root / 'compile.log')
            self.assertEqual(result['returncode'], 2)
            self.assertIn('error C2001', (root / 'compile.log').read_text())
            with patch('tools.build_game.subprocess.run', side_effect=OSError('cannot start')):
                result = run_command(['cl'], root, {}, root / 'start.log')
            self.assertEqual(result['returncode'], -1)
            self.assertIn('cannot start', (root / 'start.log').read_text())


if __name__ == '__main__':
    unittest.main()
