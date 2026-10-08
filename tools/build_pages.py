#!/usr/bin/env python3
"""Render docs/*.md and README.md into a static HTML site for GitHub Pages.

    python3 tools/build_pages.py --out _site

Links between documents become links between the generated pages; links to
repository files outside docs/ (sources, headers, AGENTS.md) point at the
file on GitHub. Rendering is CommonMark with GitHub tables, like GitHub's own
view; needs `pip install markdown-it-py mdit-py-plugins`.
"""
from __future__ import annotations

import argparse
import html
import posixpath
import re
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
REPO_URL = 'https://github.com/andrewboudreau/motocross-madness-2-decomp'

# Landing-page groups; every other document is listed under "Units".
GROUPS = [
    ('Status', ['DECOMPILATION_PROGRESS', 'VC6_MATCHING', 'NEAR_MISS_INDEX',
                'UNATTRIBUTED', 'INITIALIZERS', 'PHYSICS_VALIDATION']),
    ('How VC6 SP3 compiles this code', ['VC6_FRAME_LAYOUT', 'VC6_OPERAND_ORDER',
                                        'VC6_INLINE_BUDGET', 'VC6_CRT_ATLAS']),
    ('Project', ['TOOLCHAIN', 'PROVENANCE', 'CATEGORIES', 'CATEGORY_PILOTS',
                 'CLASS_MODEL', 'REFERENCES']),
]

CSS = """
:root { --fg:#1f2328; --muted:#59636e; --bg:#ffffff; --code:#f6f8fa; --line:#d1d9e0; --link:#0969da; }
@media (prefers-color-scheme: dark) {
  :root { --fg:#e6edf3; --muted:#9198a1; --bg:#0d1117; --code:#151b23; --line:#3d444d; --link:#4493f8; }
}
* { box-sizing: border-box; }
body { margin:0; background:var(--bg); color:var(--fg);
       font:16px/1.6 -apple-system,BlinkMacSystemFont,"Segoe UI",Helvetica,Arial,sans-serif; }
header { border-bottom:1px solid var(--line); padding:12px 16px; }
header a { color:var(--fg); font-weight:600; text-decoration:none; }
main { max-width:980px; margin:0 auto; padding:16px 16px 64px; }
a { color:var(--link); }
h1, h2, h3 { line-height:1.25; }
h1 { border-bottom:1px solid var(--line); padding-bottom:.3em; }
code, pre { font-family:ui-monospace,SFMono-Regular,Menlo,Consolas,monospace; font-size:85%; }
code { background:var(--code); padding:.15em .35em; border-radius:4px; }
pre { background:var(--code); padding:12px; border-radius:6px; overflow-x:auto; }
pre code { background:none; padding:0; font-size:100%; }
table { border-collapse:collapse; display:block; overflow-x:auto; max-width:100%; }
th, td { border:1px solid var(--line); padding:4px 10px; vertical-align:top; }
th { background:var(--code); }
.muted { color:var(--muted); font-size:90%; }
ul.docs { columns:3 220px; padding-left:1.2em; }
"""

LINK = re.compile(r'(\]\()([^)\s]+)(\))')


def page_name(stem: str) -> str:
    return 'index.html' if stem == 'README' else stem + '.html'


def rewrite_links(text: str, source: str, doc_stems: set[str]) -> str:
    """Point .md links at generated pages and other repo paths at GitHub."""
    base_dir = posixpath.dirname(source)

    def fix(match: re.Match) -> str:
        target = match.group(2)
        if re.match(r'^[a-z]+:', target) or target.startswith('#'):
            return match.group(0)
        path, _, anchor = target.partition('#')
        resolved = posixpath.normpath(posixpath.join(base_dir, path))
        stem = posixpath.splitext(posixpath.basename(resolved))[0]
        if resolved.endswith('.md') and (resolved == 'README.md' or
                                         (resolved.startswith('docs/') and stem in doc_stems)):
            new = page_name(stem) + ('#' + anchor if anchor else '')
        elif (ROOT / resolved).exists():
            kind = 'tree' if (ROOT / resolved).is_dir() else 'blob'
            new = f'{REPO_URL}/{kind}/main/{resolved}' + ('#' + anchor if anchor else '')
        else:
            new = target
        return match.group(1) + new + match.group(3)

    return LINK.sub(fix, text)


def render(md_text: str) -> str:
    from markdown_it import MarkdownIt
    from mdit_py_plugins.anchors import anchors_plugin
    md = (MarkdownIt('commonmark', {'html': True}).enable('table').enable('strikethrough')
          .use(anchors_plugin, max_level=4))
    return md.render(md_text)


def wrap(title: str, body: str, source: str) -> str:
    edit = f'{REPO_URL}/blob/main/{source}'
    return f"""<!doctype html>
<html lang="en"><head><meta charset="utf-8">
<meta name="viewport" content="width=device-width, initial-scale=1">
<title>{html.escape(title)} · MCM2 decompilation</title>
<style>{CSS}</style></head>
<body><header><a href="index.html">Motocross Madness 2 decompilation</a></header>
<main>{body}
<p class="muted">Source: <a href="{edit}">{html.escape(source)}</a></p></main></body></html>
"""


def title_of(md_text: str, fallback: str) -> str:
    for line in md_text.splitlines():
        if line.startswith('# '):
            return line[2:].strip()
    return fallback


def build(out: Path) -> int:
    docs = sorted((ROOT / 'docs').glob('*.md'))
    stems = {p.stem for p in docs}
    out.mkdir(parents=True, exist_ok=True)
    titles = {}
    for path in docs:
        text = path.read_text(encoding='utf-8')
        source = f'docs/{path.name}'
        titles[path.stem] = title_of(text, path.stem)
        body = render(rewrite_links(text, source, stems))
        (out / page_name(path.stem)).write_text(wrap(titles[path.stem], body, source), encoding='utf-8')

    readme = (ROOT / 'README.md').read_text(encoding='utf-8')
    body = render(rewrite_links(readme, 'README.md', stems))
    listed = set()
    sections = []
    for heading, group in GROUPS:
        items = [s for s in group if s in stems]
        listed.update(items)
        sections.append((heading, items))
    sections.append(('Units', sorted(stems - listed)))
    body += '<h2 id="documents">Documents</h2>'
    for heading, items in sections:
        links = ''.join(f'<li><a href="{page_name(s)}">{html.escape(titles[s])}</a></li>' for s in items)
        body += f'<h3>{html.escape(heading)}</h3><ul class="docs">{links}</ul>'
    (out / 'index.html').write_text(wrap('Overview', body, 'README.md'), encoding='utf-8')
    (out / '.nojekyll').write_text('', encoding='utf-8')
    return len(docs) + 1


def main() -> None:
    ap = argparse.ArgumentParser(description=__doc__.splitlines()[0])
    ap.add_argument('--out', default='_site')
    a = ap.parse_args()
    print(f'wrote {build(Path(a.out))} pages to {a.out}')


if __name__ == '__main__':
    main()
