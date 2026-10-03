import unittest

from tools.static_check import absolute_includes


class IncludePathTests(unittest.TestCase):
    def test_checkout_specific_headers_are_rejected(self):
        for path in ('C:/tmp/other/Quadtree.h', r'C:\other\Quadtree.h',
                     '/workspace/other/header.h', r'\\server\share\header.h'):
            with self.subTest(path=path):
                self.assertEqual(absolute_includes('#include "' + path + '"'), [path])

    def test_project_and_system_includes_are_allowed(self):
        text = '#include "Quadtree.h"\n#include "../../reconstructed/BlockAllocator.h"\n#include <string.h>'
        self.assertEqual(absolute_includes(text), [])
