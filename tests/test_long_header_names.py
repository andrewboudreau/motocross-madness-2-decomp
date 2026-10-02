"""Long-name VC98 header copies restored from the bundle's 8.3 names."""
from pathlib import Path
import tempfile
import unittest

from mcm2tool.private_bundle import LONG_HEADER_NAMES, restore_long_header_names


class LongHeaderNameTests(unittest.TestCase):
    def test_copies_missing_and_keeps_existing(self):
        with tempfile.TemporaryDirectory() as td:
            include = Path(td) / 'VC98' / 'INCLUDE'
            include.mkdir(parents=True)
            (include / 'XCEPTION').write_bytes(b'exception header')
            (include / 'ALGRITHM').write_bytes(b'algorithm header')
            (include / 'ALGORITHM').write_bytes(b'already installed')
            self.assertEqual(restore_long_header_names(Path(td)), ['EXCEPTION'])
            self.assertEqual((include / 'EXCEPTION').read_bytes(), b'exception header')
            self.assertEqual((include / 'ALGORITHM').read_bytes(), b'already installed')
            self.assertEqual(restore_long_header_names(Path(td)), [])

    def test_mapping_is_the_vc6_setup_set(self):
        self.assertEqual(LONG_HEADER_NAMES['XCEPTION'], 'EXCEPTION')
        self.assertEqual(len(LONG_HEADER_NAMES), 6)


if __name__ == '__main__':
    unittest.main()
