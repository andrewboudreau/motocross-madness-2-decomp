# bmpfile.cpp, Palette8.cpp, FontTexture.cpp, FontTextureManager.cpp

Evidence: the `__FILE__` literals of the four files (xrefs `0x0042416e..
0x004245dc`, `0x004b6ba4..0x004b6e52`, `0x004671a1..0x0046750c`,
`0x004676da`/`0x0046771a`), the `EArray.h` literal (constructor line 53) and
RTTI `Palette8 : BaseObject` (vtable `0x00555aa4`), `FontTexture`
(`0x0055298c`), `CharacterCell` (`0x00552994`) and `Rectangle2D`
(`0x005577fc`). Names are provisional.

- `bmpfile.cpp` (`src/reconstructed/bmpfile.cpp`): read, write, free and
  describe an 8-bit `.bmp` (`0x00424140`, `0x00424380`, `0x004245b0`,
  `0x004245f0`). The file/info headers are pack(2) layouts inferred from
  offsets and stored constants. PCTextureMap.h now includes `bmpfile.h`.
- `Palette8.cpp`: the loader `0x004b6b30`, constructor, destructor and
  deleting wrapper. It uses Quantize's `ColorMapper::UnknownFunction4dddd0`.
- `FontTexture.cpp`: the static manager object at `0x0065b478` and its
  `$E` constructor/atexit chain (`0x00467100..0x0046714f`; the guard byte is
  `0x0065b480`), the FontTexture constructor, destructors and accessors.
  The CharacterCell destructor pair is emitted only in the TU that builds a
  CharacterCell (the loader), so it is checked from
  `samples/render/FontTextureNearMisses.cpp`. Which class owns the static
  object is not established; Game.h's `g_UnknownStatic65b478` names the same
  address.
- `FontTextureManager.cpp`: the manager list's constructors, destructor,
  append, removal, clear and search by name (`0x00467690..0x00467847`).

Exact: 31 calibration cases (4 + 4 + 16 + 7), including FontTexture's
`FindCell` `0x00467340` and the manager's removal `0x00467760` and search
`0x004677f0`.

Source forms that mattered:
- `0x00467340` and `0x004677f0` keep the loop test at the top only as
  `while (1)` with the end test and the match as `break`s; `for` and
  `while (cond)` loops are rotated to test at the bottom.
- `0x00467760` writes `delete node` in both arms of the unlink. VC6 merges
  the two into one tail, and that keeps the delete's `add esp, 4` apart from
  the release call's, as in retail.

Near miss (notes in the sample file): the loader `0x004673d0` (esi/edi
saves placed before the early return in retail).

## Function names

Names given from each function's behaviour (string literals, D3D/DirectDraw
method slots and arguments, callers); the address-derived names they
replace are in Git history. Tier 3 unless the entry says otherwise.

- `0x00467340` `FindCell`
- `0x00467380` `SetCell`
- `0x004673a0` `SetTexture`
- `0x004673c0` `GetTexture`
