# Placement area queries (ObjectPicker..ObjectPlacement gap)

Canonical reconstruction: `src/reconstructed/PlacementQueries.cpp`. The gap
is `0x004b08ec..0x004b0df0`, after ObjectPicker.cpp's last function and
before ObjectPlacement.cpp's first `__FILE__` reference (`0x004b0fca`).
`0x004b08ec..0x004b08ef` is padding after the preceding `ret`. No literal
ties the three functions to either file, so the file name is ours
(tier 3). ObjectPlacement `0x004b0df0` calls all three. 0x004b0ac0 also has
a caller at `0x00491dc6`.

## Matched (3 of 3, strict exact)

- `0x004b08f0` (455 bytes, cdecl). Whether a sphere around a point touches
  a collision object or a vegetation cell in the collision quadtree
  (`0x0068aba4`). It skips `self`, objects whose `+0x64` owner type equals
  either of two arguments, and objects whose ignore list (`+0x74` mode,
  `+0x78` list, `+0x7c` count) rules `self` out. Then it calls
  `SphereTouchesObject` (`0x004394f0`). It returns the matching enable flag
  (argument 6 or 7) or 0. Like the CollisionObject constructor, it caches
  the `"CollisionObject"`/`"Vegetation"` type ids. Its copies are at
  `0x0056eeb0`/`0x0056eeb1`, guarded by the byte at `0x00688774`.
- `0x004b0ac0` (177 bytes, cdecl). Builds the square `(x, z, size)` scaled
  by `scale * 256`, writes its bounds to four optional outputs and returns
  whether the point lies strictly inside it in x and z.
- `0x004b0b80` (619 bytes, cdecl, void). ObjectPlacement's tail call. When
  the position is outside the square, it moves x and z to a
  `scale * 1.2333` margin inside the edge near `center`. Near an edge it
  probes the ground (`0x00507c10`). Where the normal's y is below `0.707`
  it pushes the position `scale * 10` inward. It then writes
  `target - position`. Arguments 1, 6 and 7 are unused. No path sets eax
  deliberately, so it is declared `void`. The sample's reading that
  ObjectPlacement returns this call's result is therefore doubtful.

The sample `samples/physics/contact/ObjectPlacement.cpp` still declares
these as `Fn_4b08f0`/`Fn_4b0ac0`/`Fn_4b0b80`.

Source forms that mattered:

- The type ids are `unsigned char`. Retail sets the sentinel with
  `mov bl, 0xff`. A signed `char` -1 gives `or bl, 0xff`.
- The query loop is a `for` over `NextObject()`. A `do`/`while` behind an
  `if` inverts the loop exit and moves the epilogues.
- In `0x004b0ac0` the size term is written out twice
  (`x0 + size * scale * 256.0f`). A shared `extent` local changes which
  value stays on the x87 stack.
- In `0x004b0b80` the margin test is `center->x < margin + minX`, which
  gives retail's `fcom`. The probe point is a block-scope copy while the
  normal is a function-scope local, which reproduces the frame layout.

## Reproduce

```bash
python3 tools/compile.py --compiler vc6 --vc6-root "$VC6_ROOT" src/reconstructed/PlacementQueries.cpp -o work/PlacementQueries.obj
python3 tools/match.py --exe "$MCM2_EXE" --target-va 0x004b0b80 --target-size 619 --obj work/PlacementQueries.obj --symbol '?ClampToPlacementSquare@@YAXPAVCollisionObject@@PAVPlacementProbe@@PBUCollisionVec3@@MHHH2PAU3@3@Z' --bindings src/reconstructed/PlacementQueries.bindings.json
python3 tools/run_calibration.py --exe "$MCM2_EXE" --compiler vc6 --vc6-root "$VC6_ROOT"
```
