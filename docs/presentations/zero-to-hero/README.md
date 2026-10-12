# Motocross Madness 2: Zero to Hero

A 23-slide walkthrough of MCM2's design and architecture, built from this repo's `docs/`.

- **View:** [andrewboudreau.github.io/motocross-madness-2-decomp/presentations/zero-to-hero/](https://andrewboudreau.github.io/motocross-madness-2-decomp/presentations/zero-to-hero/) (← / → or click to navigate, `N` toggles speaker notes, `#N` in the URL jumps to a slide). Locally, `python tools/build_pages.py --out _site` writes it to `_site/presentations/zero-to-hero/index.html`.
- **Source:** `deck.json` (title, slide order, sections, fonts) and one `slides/<id>.html` per slide, each a single 1920×1080 `<section>` with inline styles and an `<aside>` of speaker notes. The page around the slides is `../viewer.html`, shared by every deck.

Sections: Zero (game and binary) · Architecture · Objects and memory · The world · Bike and rider · Game layer · Hero (the decomp).

Figures reflect `DECOMPILATION_PROGRESS.md` (2026-10-11) and `REBUILD_GAPS.md` (2026-10-10). Names are mostly reconstruction names; the notes flag inferences.
