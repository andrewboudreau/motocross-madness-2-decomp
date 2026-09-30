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
- scalar deleting destructor `0x00405130` wraps destructor core `0x00405150`
- `0x00405160` (provisional `AddRef`) is an 8/8 exact clang/MSVC-ABI and VC6 SP3 match
- `0x00405170` (provisional `Release`) remains a VC6 calibration target
- `0x00401940` (provisional `GetRefCount`) is a 4/4 exact clang/MSVC-ABI and VC6 SP3 match

The names `AddRef`, `Release`, `GetRefCount`, and `refCount` are semantic/provisional. The class identity, layout, vtable, and machine behavior are evidence-backed.

## FollowCamera — active reconstruction

Slots 64–67 are exact preset-store methods. Slots 70/72 expose state-machine and cyclic-state-list behavior.

Slot 69 now has a strong MSVC ABI interpretation: virtual slot 57 returns a 12-byte aggregate by value via the hidden return-buffer convention, the result is copied to `this+0x2A8`, and virtual slot 43 consumes that cached value by reference. The 12-byte semantic type remains intentionally unnamed.
