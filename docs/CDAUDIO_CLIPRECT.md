# CDAudio and ClipRectangle

Canonical reconstruction: `src/reconstructed/CDAudio.*` and
`src/reconstructed/ClipRectangle.*`. Neither unit has RTTI, a vtable or a
`__FILE__` literal. Both file names, all class names and all member names are
ours (tier 3).

## Extent

The two units fill the gap `0x00430ff0..0x004318d0` between CarProcedural.cpp
(last `__FILE__` xref `0x0042fb52`, its `$E` pairs end at `0x00430feb`) and
CollisionCharacter.cpp (`__FILE__` xref `0x00431a12`).
- `0x00430ff0` is the out-of-line copy of the TextureMap.h inline
  end-of-stream test. It is covered through its callers.
- CDAudio is `0x00431050..0x004312a8`.
- ClipRectangle is `0x004312b0..0x004318cf`. It opens with its own `$E` set,
  `.CRT$XCU` 57.
- Strong inference: the split between the two units. Nothing calls across
  it, and the clipper's static instance and `$E` set start a new unit's
  pattern.

## CDAudio (9 exact)

TrackGame.cpp line 343 allocates 0x10 bytes. It runs the constructor
`0x00431050`, then calls `0x004310c0` and stores the object at
TrackGame+0x3340. TrackGame's destructor deletes it, and `0x00521a40` calls
`0x004310e0` on it. TrackGame's source and bindings now use these names.

Confirmed from decoded calls:
- The constructor picks the aux device whose `wTechnology` is
  `AUXCAPS_CDAUDIO`.
- `0x004311e0` opens the MCI device type `"cdaudio"` (literal `0x00568460`)
  and sets the TMSF time format.
- The volume functions scale percentages through `auxGetVolume` and
  `auxSetVolume`.
- The close function restores the saved volume, then sends `MCI_CLOSE`.

Source forms that mattered:
- The volume conversions go through a named `float fraction`.
- `Open` clears its `MCI_OPEN_PARMS` with `memset`.
- Both of `Open`'s failure paths `goto` a single `return 0`.

## ClipRectangle (10 exact)

This is an axis-aligned rectangle in the x/y plane. It has a
Cohen-Sutherland segment clipper and a polygon clipper that inserts the
rectangle's corners where the polygon leaves through one edge and comes back
through another. A polygon that winds around the rectangle without entering
it becomes the rectangle itself.

The static instance lives at `0x00579730`. Its only references are its `$E`
functions. The `.data` pointer `0x0056852c` is statically initialised to the
instance. The only users of the pointer are D3DIMSoultreeShadow (`0x00446f7d`,
`0x004472a5`) and TerrainShadow (`0x00509b0d`, `0x00509e86`). They call
`Set` (`0x00431350`) and `ClipPolygon` (`0x00431680`).

The table `0x00568500` (11 ints) maps an outcode to a region index 1..8
around the rectangle. The impossible left+right outcodes map to -1.

Exact:
- the four `$E` functions; the destructor is empty and folds into the shared
  `ret` at `0x00464e90`;
- the constructor `0x004312f0`;
- `Set`;
- the outcode `0x004313d0`;
- the edge intersection `0x00431430`;
- the segment clipper `0x004314e0` (408 bytes);
- the polygon clipper `0x00431680` (591 bytes).

Source forms that mattered:
- The corner type is a plain struct. A point type with a user constructor
  makes VC6 emit a vector constructor iterator.
- The constructor never stores `top`, which is a retail quirk. Every corner
  component is assigned explicitly.
- `Set` writes the corners as chained assignments
  (`corners[0].x = corners[3].x = left`).
- `ClipPolygon` needs four forms:
  - Its counters start as `outCount = winding = lastExit = 0`, before the
    closing `points[count] = points[0]`.
  - Both wrapped exits `break` to a single `return outCount`. Separate returns
    give VC6's known-zero epilogue.
  - The fully enclosed case is handled inside the `i == count` test.
  - The emitted points go through a `ClipPoint* dst = &out[outCount++]`
    temporary. Plain `out[outCount++] = p` reads the source after advancing
    the index (521/583).

## Reproduce

```bash
python3 tools/compile.py --compiler vc6 --vc6-root "$VC6_ROOT" src/reconstructed/ClipRectangle.cpp -o work/ClipRectangle.obj
python3 tools/run_calibration.py --exe "$MCM2_EXE" --compiler vc6 --vc6-root "$VC6_ROOT"
```
