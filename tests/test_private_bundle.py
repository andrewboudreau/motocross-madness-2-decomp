from __future__ import annotations
import hashlib
import json
from pathlib import Path
import stat
import tempfile
import unittest
from unittest.mock import patch
import zipfile

from mcm2tool.private_bundle import (
    BundleError,
    MANIFEST_PATH,
    PROVENANCE_PATH,
    REQUIRED_FILES,
    install_archive,
    verify_archive,
)

def digest(data: bytes) -> str:
    return hashlib.sha256(data).hexdigest()

class BundleTests(unittest.TestCase):
    def setUp(self):
        self.temp = tempfile.TemporaryDirectory()
        self.addCleanup(self.temp.cleanup)
        self.root = Path(self.temp.name)
        self.game = b'fake-retail-game'
        self.game_hash = digest(self.game)

    def make_bundle(self, extra=None, corrupt=None, unsafe=None, symlink=False):
        payload = {}
        for name in REQUIRED_FILES:
            if name == MANIFEST_PATH:
                continue
            if name == PROVENANCE_PATH:
                payload[name] = json.dumps(
                    {'target_sha256': self.game_hash}
                ).encode()
            elif name == 'work/game/mcm2.exe':
                payload[name] = self.game
            else:
                payload[name] = ('payload:' + name).encode()

        if extra:
            payload.update(extra)
        manifest = {name: digest(data) for name, data in payload.items()}
        path = self.root / 'bundle.zip'
        with zipfile.ZipFile(path, 'w') as z:
            for name, data in payload.items():
                z.writestr(name, data)
            if unsafe:
                z.writestr(unsafe, b'evil')
            if symlink:
                info = zipfile.ZipInfo('toolchains/link')
                info.create_system = 3
                info.external_attr = (stat.S_IFLNK | 0o777) << 16
                z.writestr(info, 'target')
            z.writestr(MANIFEST_PATH, json.dumps(manifest))

        if corrupt:
            manifest[corrupt] = '0' * 64
            with zipfile.ZipFile(path, 'w') as z:
                for name, data in payload.items():
                    z.writestr(name, data)
                z.writestr(MANIFEST_PATH, json.dumps(manifest))
        return path

    def verify(self, path):
        with patch(
            'mcm2tool.private_bundle.EXPECTED_GAME_SHA256', self.game_hash
        ):
            return verify_archive(path)

    def test_valid_manifest_and_install(self):
        path = self.make_bundle()
        result = self.verify(path)
        self.assertEqual(result['target_sha256'], self.game_hash)
        with patch(
            'mcm2tool.private_bundle.EXPECTED_GAME_SHA256', self.game_hash
        ):
            installed = install_archive(
                path, self.root / 'install', result['archive_sha256']
            )
            again = install_archive(
                path, self.root / 'install', result['archive_sha256']
            )
        self.assertEqual(installed.archive_sha256, again.archive_sha256)
        self.assertEqual(
            (installed.root / 'work/game/mcm2.exe').read_bytes(), self.game
        )
        self.assertTrue(
            (installed.root / 'private-inputs-state.json').exists()
        )

    def test_wrong_archive_hash_fails(self):
        path = self.make_bundle()
        with patch(
            'mcm2tool.private_bundle.EXPECTED_GAME_SHA256', self.game_hash
        ):
            with self.assertRaises(BundleError):
                verify_archive(path, '0' * 64)

    def test_payload_hash_mismatch_fails(self):
        path = self.make_bundle(
            corrupt='toolchains/vc6sp3/VC98/BIN/CL.EXE'
        )
        with self.assertRaises(BundleError):
            self.verify(path)

    def test_unlisted_payload_fails(self):
        path = self.make_bundle()
        with zipfile.ZipFile(path, 'a') as z:
            z.writestr('extra.bin', b'x')
        with self.assertRaises(BundleError):
            self.verify(path)

    def test_path_traversal_fails(self):
        path = self.make_bundle(unsafe='../escape.txt')
        with self.assertRaises(BundleError):
            self.verify(path)

    def test_symlink_fails(self):
        path = self.make_bundle(symlink=True)
        with self.assertRaises(BundleError):
            self.verify(path)

    def test_wrong_game_hash_fails(self):
        path = self.make_bundle()
        with self.assertRaises(BundleError):
            verify_archive(path)

    def test_nonempty_destination_refused_without_overwrite(self):
        path = self.make_bundle()
        out = self.root / 'install'
        out.mkdir()
        (out / 'x').write_text('x')
        with patch(
            'mcm2tool.private_bundle.EXPECTED_GAME_SHA256', self.game_hash
        ):
            with self.assertRaises(BundleError):
                install_archive(path, out)

if __name__ == '__main__':
    unittest.main()
