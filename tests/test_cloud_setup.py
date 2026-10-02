"""Exercise startup orchestration without network, Wine or proprietary inputs."""
import json
import os
from pathlib import Path
import shutil
import subprocess
import tempfile
import unittest

ROOT = Path(__file__).resolve().parents[1]


@unittest.skipIf(os.name == 'nt' or not shutil.which('bash'), 'requires POSIX bash')
class CloudSetupTests(unittest.TestCase):
    def setUp(self):
        self.temp = tempfile.TemporaryDirectory(prefix='mcm2-cloud-test-')
        self.addCleanup(self.temp.cleanup)
        self.root = Path(self.temp.name) / 'checkout'
        self.private = Path(self.temp.name) / 'private $cache with spaces'
        for folder in ('tools', 'config', 'bin', 'analysis'):
            (self.root / folder).mkdir(parents=True, exist_ok=True)
        shutil.copyfile(ROOT / 'tools/cloud_setup.sh', self.root / 'tools/cloud_setup.sh')
        shutil.copyfile(ROOT / 'tools/with_private_env.py', self.root / 'tools/with_private_env.py')
        (self.root / 'config/private_bundle_expected.json').write_text(
            json.dumps({'archive_sha256': 'a' * 64}))
        self.env = {k: v for k, v in os.environ.items()
                    if not k.startswith(('MCM2_', 'VC6_', 'WINE'))}
        self.env.update(HOME=str(Path(self.temp.name) / 'home'),
                        VC6_RUNNER='wine',
                        MCM2_PRIVATE_ROOT=str(self.private),
                        PATH=str(self.root / 'bin') + os.pathsep + os.environ['PATH'])
        for name in ('wine', 'winepath', 'objdump', 'g++', 'clang-cl'):
            self.command(name, 'exit 0')
        self.command('curl', 'echo unexpected-download >&2; exit 9')
        self.command('make', 'echo analysis >> calls; mkdir -p analysis; exit "${ANALYSIS_EXIT:-0}"')
        (self.root / 'tools/vc6_acceptance.py').write_text(
            'import os\nfrom pathlib import Path\n'
            'with Path("calls").open("a") as f: f.write("compiler\\n")\n'
            'raise SystemExit(int(os.environ.get("ACCEPTANCE_EXIT", "0")))\n')

    def command(self, name, body):
        path = self.root / 'bin' / name
        path.write_text('#!/usr/bin/env bash\n' + body + '\n')
        path.chmod(0o755)

    def installed(self, archive_sha=None):
        for name in ('work/game/mcm2.exe', 'toolchains/vc6sp3/VC98/BIN/CL.EXE'):
            path = self.private / name
            path.parent.mkdir(parents=True, exist_ok=True)
            path.touch()
        (self.private / 'private-inputs-state.json').write_text(json.dumps({
            'archive_sha256': archive_sha or 'a' * 64,
            'unrelated': 'a' * 64,
        }))
        (self.private / ('.accepted-wine-' + 'a' * 64)).touch()
        (self.root / 'analysis/.cloud-analyze-ok').touch()

    def run_setup(self, *args):
        return subprocess.run(['bash', str(self.root / 'tools/cloud_setup.sh'),
                               '--no-apt', *args], cwd=self.temp.name,
                              env=self.env, text=True, capture_output=True)

    def test_missing_inputs_fail_strict_but_preserve_best_effort_hook(self):
        self.assertNotEqual(self.run_setup('--strict').returncode, 0)
        self.assertEqual(self.run_setup().returncode, 0)

    def test_cached_startup_rechecks_execution_and_analysis_without_download(self):
        self.installed()
        for _ in range(2):
            result = self.run_setup('--strict')
            self.assertEqual(result.returncode, 0, result.stderr)
        self.assertEqual((self.root / 'calls').read_text().splitlines(),
                         ['compiler', 'analysis', 'compiler', 'analysis'])
        # The generated exports must preserve spaces and literal shell metacharacters.
        result = subprocess.run(['bash', '-c',
                                 'source work/cloud-env.sh; printf "%s" "$MCM2_EXE"'],
                                cwd=self.root, env=self.env, capture_output=True, text=True)
        self.assertEqual(result.stdout, str(self.private / 'work/game/mcm2.exe'))

    def test_stale_accepted_marker_does_not_hide_compiler_failure(self):
        self.installed()
        self.env['ACCEPTANCE_EXIT'] = '4'
        result = self.run_setup('--strict')
        self.assertNotEqual(result.returncode, 0)
        self.assertIn('VC6 acceptance failed', result.stderr)
        self.assertEqual((self.root / 'calls').read_text(), 'compiler\n')

    def test_analysis_failure_propagates(self):
        self.installed()
        self.env['ANALYSIS_EXIT'] = '2'
        result = self.run_setup('--strict')
        self.assertNotEqual(result.returncode, 0)
        self.assertIn('make analyze failed', result.stderr)

    def test_wrong_marker_hash_is_not_an_installed_bundle(self):
        self.installed(archive_sha='b' * 64)
        result = self.run_setup('--strict')
        self.assertNotEqual(result.returncode, 0)
        self.assertIn('MCM2_PRIVATE_BUNDLE_URL is not set', result.stderr)
        self.assertFalse((self.root / 'calls').exists())

    def test_download_failure_does_not_leak_url(self):
        secret = 'https://example.invalid/bundle.zip?private-token=DO-NOT-PRINT'
        self.env['MCM2_PRIVATE_BUNDLE_URL'] = secret
        result = self.run_setup('--strict')
        self.assertNotEqual(result.returncode, 0)
        self.assertIn('could not download', result.stderr)
        self.assertNotIn(secret, result.stdout + result.stderr)
        self.assertNotIn(secret, (self.root / 'work/cloud-env.sh').read_text())

    def test_missing_env_file_argument_is_usage_error(self):
        result = self.run_setup('--env-file')
        self.assertEqual(result.returncode, 2)
        self.assertIn('requires a path', result.stderr)

    def test_cached_startup_requires_pinned_wibo_even_if_wine_exists(self):
        self.installed()
        self.env['VC6_RUNNER'] = 'wibo'
        result = self.run_setup('--strict')
        self.assertNotEqual(result.returncode, 0)
        self.assertIn('pinned wibo missing or changed', result.stderr)
        self.assertFalse((self.root / 'calls').exists())


if __name__ == '__main__':
    unittest.main()
