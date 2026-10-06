import importlib.util
import unittest
import tempfile
from pathlib import Path


ROOT = Path(__file__).resolve().parents[1]
SPEC = importlib.util.spec_from_file_location(
    "decompilation_progress", ROOT / "tools" / "decompilation_progress.py"
)
MODULE = importlib.util.module_from_spec(SPEC)
assert SPEC.loader is not None
SPEC.loader.exec_module(MODULE)


class DecompilationProgressTests(unittest.TestCase):
    def test_repository_counts_both_canonical_trees_and_excludes_samples(self):
        with tempfile.TemporaryDirectory() as directory:
            root = Path(directory)
            for name in ('src/reconstructed/A.cpp', 'src/krusty2/nested/B.cpp',
                         'src/krusty2/core/B.h', 'samples/C.cpp', 'generated/D.cpp'):
                p = root / name
                p.parent.mkdir(parents=True, exist_ok=True)
                p.write_text('// one line\n', encoding='utf-8')
            self.assertEqual(MODULE.repository_counts(root),
                             {'cpp_files': 2, 'header_files': 1, 'source_lines': 3})

    def test_render_labels_selected_target_percentage(self):
        config = {
            "as_of": "2000-01-01",
            "retail_sha256": "abc",
            "retail_text_bytes": 1048576,
            "retail_source_paths": 100,
            "rtti_types": 20,
            "vc6_selected_targets": 4,
            "vc6_exact_targets": 3,
            "vc6_case_count": 5,
            "vc6_profile": "vc6_o2_mt",
            "reviewed_commit": "abc",
            "notes": "Test note.",
        }
        text = MODULE.render(
            config, {"cpp_files": 2, "header_files": 3, "source_lines": 40}
        )
        self.assertIn("3 of 4 selected function targets (75.0%)", text)
        self.assertIn("not 75.0% of MCM2", text)
        self.assertIn("1.00 MiB", text)
        self.assertIn("5 cases", text)
        self.assertNotIn("Reconstructed files / retail", text)
        for exact, selected in ((1, 0), (5, 4), (-1, 4)):
            with self.subTest(exact=exact, selected=selected):
                config.update(vc6_exact_targets=exact, vc6_selected_targets=selected)
                with self.assertRaises(ValueError):
                    MODULE.render(config, {})

    def test_staleness_tracks_canonical_source_changes(self):
        config = (ROOT / "config" / "decompilation_progress.json").read_text(encoding="utf-8")
        with tempfile.TemporaryDirectory() as directory:
            root = Path(directory)
            for name, text in (("config/decompilation_progress.json", config),
                               ("src/reconstructed/A.cpp", "// one line\n")):
                p = root / name
                p.parent.mkdir(parents=True, exist_ok=True)
                p.write_text(text, encoding="utf-8")
            page = root / "docs" / "DECOMPILATION_PROGRESS.md"
            page.parent.mkdir(parents=True)
            self.assertIn("+# Decompilation progress", MODULE.staleness(root))
            page.write_text(MODULE.expected_document(root), encoding="utf-8")
            self.assertIsNone(MODULE.staleness(root))
            (root / "src" / "reconstructed" / "B.h").write_text("// header\n", encoding="utf-8")
            diff = MODULE.staleness(root)
            self.assertIn("-| Canonical reconstructed headers | **0**", diff)
            self.assertIn("+| Canonical reconstructed headers | **1**", diff)


if __name__ == "__main__":
    unittest.main()
