"""Pages build: slide decks under docs/presentations assemble in deck order."""
from pathlib import Path
import json
import re
import tempfile
import unittest

from tools.build_pages import build_presentation, build_presentations

ROOT = Path(__file__).resolve().parents[1]
DECKS = sorted((ROOT / 'docs' / 'presentations').glob('*/deck.json'))


class PresentationTests(unittest.TestCase):
    def test_every_deck_lists_existing_slides_once(self):
        self.assertTrue(DECKS)
        for deck_json in DECKS:
            order = json.loads(deck_json.read_text(encoding='utf-8'))['order']
            files = {p.stem for p in (deck_json.parent / 'slides').glob('*.html')}
            self.assertEqual(len(order), len(set(order)), deck_json)
            self.assertEqual(set(order), files, deck_json)

    def test_slides_appear_in_deck_order(self):
        for deck_json in DECKS:
            order = json.loads(deck_json.read_text(encoding='utf-8'))['order']
            page, count = build_presentation(deck_json.parent)
            self.assertEqual(count, len(order))
            self.assertEqual(re.findall(r'<section id="([^"]+)"', page), order)
            self.assertNotIn('{{', page.split('<style>')[0])

    def test_decks_are_written_under_presentations(self):
        with tempfile.TemporaryDirectory() as tmp:
            decks = build_presentations(Path(tmp))
            for name, _title, _count in decks:
                self.assertTrue((Path(tmp) / 'presentations' / name / 'index.html').is_file())
            self.assertEqual(len(decks), len(DECKS))


if __name__ == '__main__':
    unittest.main()
