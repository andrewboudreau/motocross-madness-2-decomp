"""Validation units for single bodies of src/reconstructed/TrackOverlay.cpp.

The native behaviour tests link one TrackOverlay.cpp function at a time. Each
unit is the canonical file's includes, the file-scope declarations that
function uses (sliced from the canonical file, never copied), and the
definition itself. The file is named TrackOverlay.cpp, like the unit the game
build compiles, so its __FILE__ literal ends the same way.
"""
from pathlib import Path

from tools.unit_source import standalone_source

ROOT = Path(__file__).resolve().parents[1]
UNIT = ROOT / 'src/reconstructed/TrackOverlay.cpp'
UNIT_NAME = 'TrackOverlay.cpp'


def _between(text, start, end):
    i = text.index(start)
    return text[i:text.index(end, i)]


def preludes():
    text = UNIT.read_text(encoding='latin-1')
    imports = _between(text, '// GDI32 and USER32 imports', '// 0x0057513c:')
    flag = _between(text, '// 0x0068a444: when set', '\n\n')
    return {
        'StatsOverlay::DrawStandings': imports,
        'ChatOverlay::DrawNameTag': imports + flag + '\n',
    }


def write_unit(directory, qualified):
    """Write the validation unit for ``qualified`` into ``directory``; return its path."""
    Path(directory).mkdir(parents=True, exist_ok=True)
    path = Path(directory) / UNIT_NAME
    path.write_text(standalone_source(UNIT, [qualified], preludes()[qualified]), encoding='latin-1')
    return path
