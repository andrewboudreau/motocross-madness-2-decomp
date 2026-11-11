"""GR_BitString::ShiftBitsTowardBack 0x00423b70, sub-word shift of a bit string (tools/emu_diff.py case).

The inputs of ``bitstring_shift_front`` against the other direction.
"""
from tools.emu_cases.bitstring_shift_front import cases, setup  # noqa: F401

RETAIL_VA = 0x00423b70
SYMBOL = '?ShiftBitsTowardBack@GR_BitString@@QAEXH@Z'
SOURCE = 'samples/render/BitStringShifts.cpp'
FPCWS = (0x027f,)   # integer only
