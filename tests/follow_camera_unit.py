"""Validation units for the FollowCamera bodies in src/reconstructed/FollowCamera.cpp.

The native behaviour tests link one FollowCam.cpp function at a time. Each
unit is the canonical file's includes, the file-scope declarations that
function uses (sliced from the canonical file, never copied), and the
definition itself. The file is named FollowCamera.cpp, like the unit the game
build compiles, so its __FILE__ literal ends the same way.
"""
from pathlib import Path

from tools.unit_source import function_definition, standalone_source

ROOT = Path(__file__).resolve().parents[1]
UNIT = ROOT / 'src/reconstructed/FollowCamera.cpp'
UNIT_NAME = 'FollowCamera.cpp'


def _between(text, start, end):
    i = text.index(start)
    return text[i:text.index(end, i)]


def preludes():
    text = UNIT.read_text(encoding='latin-1')
    statics = _between(text, "// FollowCam.cpp's per-file vector", '// D3D_OVERLOADS-style')
    operators = _between(text, '// D3D_OVERLOADS-style', '// 0x00462ee0.')
    controls = '\n\n'.join([function_definition(text, 'FollowCameraTurn'),
                            function_definition(text, 'FollowCameraTilt'),
                            _between(text, "// Slot 45's input queries", '// 0x00463a30.')])
    loader = (_between(text, 'Vec3 CrossProductCall(', '\n') + '\n' +
              _between(text, "// LoadCameraFile's vector helpers", '// 0x004650e0.'))
    return {
        'FollowCamera::FollowCamera': statics,
        'FollowCamera::ProcessCameraInput': operators + controls,
        'FollowCamera::LoadCameraFile': loader,
        'FollowCamera::Tick': statics + operators,
    }


def write_unit(directory, qualified):
    """Write the validation unit for ``qualified`` into ``directory``; return its path."""
    path = Path(directory) / UNIT_NAME
    path.write_text(standalone_source(UNIT, [qualified], preludes()[qualified]), encoding='latin-1')
    return path
