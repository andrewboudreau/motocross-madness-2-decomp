# bmpfile.cpp, Palette8.cpp, FontTexture.cpp, FontTextureManager.cpp

Evidence: the `__FILE__` literals of the four files (xrefs `0x0042416e..
0x004245dc`, `0x004b6ba4..0x004b6e52`, `0x004671a1..0x0046750c`,
`0x004676da`/`0x0046771a`), the `EArray.h` literal (constructor line 53) and
RTTI `Palette8 : BaseObject` (vtable `0x00555aa4`), `FontTexture`
(`0x0055298c`), `CharacterCell` (`0x00552994`) and `Rectangle2D`
(`0x005577fc`). Names are provisional.

- `bmpfile.cpp` (`src/reconstructed/bmpfile.cpp`): read, write, free and
  describe an 8-bit `.bmp` (`0x00424140`, `0x00424380`, `0x004245b0`,
  `0x004245f0`: `ReadBitmap`, `WriteBitmap`, `FreeBitmap` and
  `DescribeBitmap8`; EcoSystem.cpp's own declarations keep the address names). The file/info headers are pack(2) layouts inferred from
  offsets and stored constants. PCTextureMap.h now includes `bmpfile.h`.
- `Palette8.cpp`: the loader `0x004b6b30`, constructor, destructor and
  deleting wrapper. It uses Quantize's `ColorMapper::LoadShared`.
- `FontTexture.cpp`: the static manager object at `0x0065b478` and its
  `$E` constructor/atexit chain (`0x00467100..0x0046714f`; the guard byte is
  `0x0065b480`), the FontTexture constructor, destructors, accessors and the
  loader `0x004673d0`. The CharacterCell destructor pair is emitted only in
  the TU that builds a CharacterCell (the loader). Which class owns the static
  object is not established; Game.h's `g_UnknownStatic65b478` names the same
  address.
- `FontTextureManager.cpp`: the manager list's constructors, destructor,
  append, removal, clear and search by name (`0x00467690..0x00467847`).

Exact: 32 calibration cases (4 + 4 + 17 + 7), including FontTexture's
`FindCell` `0x00467340` and loader `0x004673d0` and the manager's removal
`0x00467760` and search `0x004677f0`.

## Font rectangle index storage

`src/reconstructed/Rectangle2DState.cpp` supplies the four writable DWORDs
already declared in `Rectangle2D.h`. The zero-valued cell at `0x0057972c`
lies in the zero-filled tail of the retail image's writable `.data` section;
`0x00568468`, `0x0056846c` and `0x00568470` contain 1, 2 and 3. The new
object has four bytes of `.bss` and twelve bytes of `.data`, with no code,
dynamic initializer, relocation or external dependency. Original names,
aggregate membership and translation-unit ownership remain unknown.

An exhaustive scan for each encoded absolute DWORD finds exactly one load:
`0x00467547`, `0x00467540`, `0x00467532` and `0x0046752c`, respectively.
The loader uses those values to select four floats from the record read by
`UnknownTextureStream::Read`, then passes them to the actual Rectangle2D
constructor at `0x004e8ad0`. This proves these direct references, not absence
of arbitrary indirect writes. It does not justify changing the cells to
read-only constants or inlining their values into callers.

`tests/test_graphics_state.py` verifies all sixteen initial bytes, individual
symbol extents, writable storage, absence of initialization code/dependencies,
and the actual indexed-load/constructor sequence against retail. The actual
game objects retain all 22 strict FontTexture and Rectangle2D cases. The real
link removes exactly these four names, **250 -> 246**, without adding another
missing symbol. Existing production objects remain unchanged. These checks
do not establish loading real font assets or integrated menu rendering.

```powershell
$env:PYTHONPATH='.;tests'
$env:VC6_ROOT=(Resolve-Path work/vc6sp3-disc).Path
$env:MCM2_EXE=(Resolve-Path work/game/mcm2.exe).Path
python -m unittest test_graphics_state -v
python tools/build_game.py -j6
```

Source forms that mattered:
- `0x00467340` and `0x004677f0` keep the loop test at the top only as
  `while (1)` with the end test and the match as `break`s; `for` and
  `while (cond)` loops are rotated to test at the bottom.
- `0x00467760` writes `delete node` in both arms of the unlink. VC6 merges
  the two into one tail, and that keeps the delete's `add esp, 4` apart from
  the release call's, as in retail.
- `0x004673d0` writes the lookup hit and the load path as an if/else that
  share one `return font;`. With an early `return node->font;` VC6 delays
  the esi/edi saves past that return (retail saves all four registers
  first); a block around the cell loop's `code`/`rect` locals moves their
  stack slots.

## Function names

Names given from each function's behaviour (string literals, D3D/DirectDraw
method slots and arguments, callers); the address-derived names they
replace are in Git history. Tier 3 unless the entry says otherwise.

- `0x00467340` `FindCell`
- `0x00467380` `SetCell`
- `0x004673a0` `SetTexture`
- `0x004673c0` `GetTexture`
