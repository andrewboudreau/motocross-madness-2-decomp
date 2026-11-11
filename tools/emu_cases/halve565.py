"""Halve565 0x004cea10, the 2x2 box filter of 565 textures (tools/emu_diff.py case).

The inputs of ``halve555`` (all three filter paths) against the 565 entry point.
"""
from tools.emu_cases.halve555 import cases, setup  # noqa: F401

RETAIL_VA = 0x004cea10
SYMBOL = '?Halve565@@YAHPAX0HHHHHG@Z'
SOURCE = 'src/bringup/Group0_Pixtrans.cpp'
FPCWS = (0x027f,)   # integer only
