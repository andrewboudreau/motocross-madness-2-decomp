from __future__ import annotations
from pathlib import Path
import tempfile
import unittest

from mcm2tool.coff_archive import (
    CoffArchiveError,
    get_archive_member,
    read_archive,
)


def header(name: str, size: int) -> bytes:
    fields = [
        name.encode().ljust(16),
        b'0'.ljust(12),
        b'0'.ljust(6),
        b'0'.ljust(6),
        b'100644'.ljust(8),
        str(size).encode().ljust(10),
        b'\x60\n',
    ]
    return b''.join(fields)


def archive(parts: list[tuple[str, bytes]]) -> bytes:
    out = bytearray(b'!<arch>\n')
    for name, data in parts:
        out += header(name, len(data)) + data
        if len(data) & 1:
            out += b'\n'
    return bytes(out)


class ArchiveTests(unittest.TestCase):
    def setUp(self):
        self.temp = tempfile.TemporaryDirectory()
        self.addCleanup(self.temp.cleanup)
        self.path = Path(self.temp.name) / 'x.lib'

    def test_short_and_long_member_names(self):
        long_names = (
            b'build\\intel\\mt_obj\\free.obj/\n'
            b'build\\intel\\mt_obj\\msize.obj/\n'
        )
        second = long_names.find(b'build\\intel\\mt_obj\\msize.obj')
        self.path.write_bytes(
            archive(
                [
                    ('//', long_names),
                    ('/0', b'FREE'),
                    (f'/{second}', b'MSIZE'),
                    ('tiny.obj/', b'TINY'),
                ]
            )
        )
        rows = read_archive(self.path)
        self.assertEqual(
            [row.name for row in rows],
            [
                'build\\intel\\mt_obj\\free.obj',
                'build\\intel\\mt_obj\\msize.obj',
                'tiny.obj',
            ],
        )
        self.assertEqual(
            get_archive_member(
                self.path, 'build/intel/mt_obj/free.obj'
            ),
            b'FREE',
        )
        self.assertEqual(
            get_archive_member(
                self.path, 'BUILD\\INTEL\\MT_OBJ\\MSIZE.OBJ'
            ),
            b'MSIZE',
        )

    def test_wrong_magic_and_truncation_fail(self):
        self.path.write_bytes(b'nope')
        with self.assertRaises(CoffArchiveError):
            read_archive(self.path)
        self.path.write_bytes(b'!<arch>\n' + b'x' * 10)
        with self.assertRaises(CoffArchiveError):
            read_archive(self.path)

    def test_missing_member_fails_closed(self):
        self.path.write_bytes(archive([('a.obj/', b'A')]))
        with self.assertRaises(CoffArchiveError):
            get_archive_member(self.path, 'b.obj')


if __name__ == '__main__':
    unittest.main()
