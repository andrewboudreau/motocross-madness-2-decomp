# overlay.cpp, and the end of Camera.cpp

## overlay.cpp

`src/reconstructed/Overlay.cpp` and `OverlayIconService.h`, with the class in
`Overlay.h`. Evidence: the overlay.cpp `__FILE__` xref at `0x004b60a2` (line
107) and the Overlay vtable at `0x00555a28`. Two header literals also fall
inside the range: OverlayIconService.h at `0x004b605d` (line 90, an inline
allocation) and ContainerList.h at `0x004b6ab0` (line 59, its inlined Init).

Extent: `0x004b5e20..0x004b6b20`; Palette8.cpp follows at `0x004b6b30`.
OptionProcs.cpp runs to `0x004b5a5d`: the ConfirmRestoreDlg and
OptControlsDlg vtables point there. `0x004b5a60` and `0x004b5d00` are math
helpers with about 20 callers each and no strings, and they are not
attributed. Overlay's COL follows their constant at `0x00555a20`, so
overlay.cpp cannot be ruled out for them.

Exact: 10 calibration cases. These are the Overlay constructor, the
destructor and its deleting wrapper, the setup `0x004b5f50`, slot 14
(draw), the quad layout and clip `0x004b6380`, the colour-key texel copy
`0x004b6880`, and OverlayIconService's constructor and destructors. Retail
calls the OverlayIconService constructor out of line from this unit. Its
own file is not attested, and the header says so.

Source forms needed:
- The constructor clears +0xf8 with `memset`.
- The destructor tests `int count = Release(); if (count == 0)`.
- The setup reads `((RenderTarget*)field_0x18)->field_0x28` directly.
- The texel copy walks the rows with `s++`/`d++` pointers.

Overlay.h changes:
- `0x004b6710` returns int, so TrackOverlay.bindings.json now uses its new
  mangled name.
- +0x34 is a TextureMap and +0x3c an OverlayIconService.
- `0x004b6880` takes its colour as `unsigned short`: ChatOverlay `0x0051e910`
  builds the 4444 grey in 16-bit registers and pushes it unextended.

Near miss (`samples/render/OverlayNearMisses.cpp`): `0x004b6710`. Its
instructions are the same, but the registers are allocated differently.

## Camera.cpp, the remaining functions

Camera.cpp ends at `0x0042f38b`, before CarProcedural.cpp. Its last three
functions are now exact:
- `0x0042f0e0` sets the viewport x and width. The height is the width
  times +0x1b8, which is now typed float.
- `0x0042f190` sets the viewport. Retail stores the width before y.
- `0x0042f210` returns the viewport rectangle.

So are its eight kVec3 `$E` (`0x0042f250..0x0042f38b`). That is 11 more
calibration cases. The kVec3 statics sit at the end of Camera.cpp because
the `$E` code follows every Camera function.
