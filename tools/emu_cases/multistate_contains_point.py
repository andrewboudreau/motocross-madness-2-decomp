"""UIMultiState::ContainsPoint 0x00478e10, the opaque-pixel hit test (tools/emu_diff.py case).

The inputs of ``button_contains_point``: retail reads the same offsets and slots (screen
area +0x2c passed to PtInRect directly, owner dialog +0xb8, slot 48, surface +0x70).
"""
from tools.emu_cases.button_contains_point import BINDINGS, cases, setup  # noqa: F401

RETAIL_VA = 0x00478e10
SYMBOL = '?ContainsPoint@UIMultiState@@UAEHUtagPOINT@@H@Z'
SOURCE = 'samples/ui/ControlHitTest.cpp'
