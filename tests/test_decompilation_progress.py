import importlib.util
import unittest
from pathlib import Path


ROOT = Path(__file__).resolve().parents[1]
SPEC = importlib.util.spec_from_file_location(
    "decompilation_progress", ROOT / "tools" / "decompilation_progress.py"
)
MODULE = importlib.util.module_from_spec(SPEC)
assert SPEC.loader is not None
SPEC.loader.exec_module(MODULE)


class DecompilationProgressTests(unittest.TestCase):
    def test_repository_counts_only_canonical_cpp_and_headers(self):
        counts = MODULE.repository_counts(ROOT)
        self.assertGreater(counts["cpp_files"], 0)
        self.assertGreater(counts["header_files"], 0)
        self.assertGreater(counts["source_lines"], 0)

    def test_render_labels_selected_target_percentage(self):
        config = {
            "as_of": "2000-01-01",
            "retail_sha256": "abc",
            "retail_text_bytes": 1048576,
            "retail_source_paths": 100,
            "rtti_types": 20,
            "vc6_selected_targets": 4,
            "vc6_exact_targets": 3,
            "notes": "Test note.",
        }
        text = MODULE.render(
            config, {"cpp_files": 2, "header_files": 3, "source_lines": 40}
        )
        self.assertIn("3 of 4 selected function targets (75.0%)", text)
        self.assertIn("not 75.0% of MCM2", text)
        self.assertIn("1.00 MiB", text)


if __name__ == "__main__":
    unittest.main()
