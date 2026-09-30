# Reconstruction status

Current compiler evidence: [VC6 SP3 matching check](VC6_MATCHING.md).
Native Windows SP3 now passes 19/19 manual samples; the larger calibration set
remains partially matching. The earlier clang results below are retained.

## BaseObject — promoted

Confirmed binary evidence:

- RTTI class name `BaseObject`
- primary vtable `0x005507c0`
- 32-bit field at `this+0x04`
- constructor `0x00405120` writes the BaseObject vptr and initializes the field to 1
- constructor-body assignment now gives a 16/16 strict VC6 SP3 match with the vtable relocation resolved
- scalar deleting destructor `0x00405130` wraps destructor core `0x00405150`
- destructor core matches 7/7 bytes with its vtable relocation resolved
- generated wrapper matches 30/30 bytes with both call relocations resolved; its independent length comes from VC6 CodeView metadata
- `0x00405160` (provisional `AddRef`) is an 8/8 exact clang/MSVC-ABI and VC6 SP3 match
- `0x00405170` (provisional `Release`) matches 32/32 bytes under VC6 SP3 `/O2` without `/G6`
- `0x00401940` (provisional `GetRefCount`) is a 4/4 exact clang/MSVC-ABI and VC6 SP3 match

The names `AddRef`, `Release`, `GetRefCount`, and `refCount` are semantic/provisional. The class identity, layout, vtable, and machine behavior are evidence-backed.

All six bodies emitted from `src/reconstructed/BaseObject.cpp` match under
`vc6_o2_ml`, with zero ignored bytes. `/ML` and `/MT` both pass the calibration
cases without `/G6`; these matches do not establish original project settings,
source-file ownership, or a whole-executable match.

## FollowCamera — active reconstruction

Slots 64–67 are exact preset-store methods. Slots 70/72 expose state-machine and cyclic-state-list behavior.

Slot 69 now has a strong MSVC ABI interpretation: virtual slot 57 returns a 12-byte aggregate by value via the hidden return-buffer convention, the result is copied to `this+0x2A8`, and virtual slot 43 consumes that cached value by reference. The 12-byte semantic type remains intentionally unnamed.
