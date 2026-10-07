# MediaControl and MovingPart

Canonical reconstruction: `src/reconstructed/MediaControl.*` and
`src/reconstructed/MovingPart.*`. The gap is `0x004a2350..0x004a2ac0`,
between the Math3D vector set (`.CRT$XCU` 179-182, `0x004a2210..0x004a2340`)
and the allocation accounting unit (`0x004a2ac0`). No `__FILE__` literal
reaches it, so every file name is ours (tier 3).

## MovingPart (2 exact)

- `0x004a2350` `GetMatrixRow` (66 bytes, cdecl, hidden result) returns
  `m[row][0..2]`. MorphBastardModifier and Bike call it.
- `0x004a23a0` is the `MovingPart` constructor (`ret 0xc`). It finds a named
  node below a root through `SoultreeObject::FindByName` (`0x004fdae0`),
  copies the node's local matrix to `+0` (`0x004fca60`), and stores the
  node at `+0x40`, its third argument at `+0x44` and zero at `+0x48`.
  MovingPart is the non-polymorphic base of Tire (mdisp `0x17c`) and of the
  Shock family (mdisp 4); see `samples/physics/tire/Tire.h`.
- Open: whether these two belong to the Math3D unit before them, to
  MediaControl's unit, or to a unit of their own. Nothing calls across the
  boundaries.

Source form that mattered: `GetMatrixRow` copies the three components into
a local one by one. Returning `*(Vector3*)m->m[row]` copies straight to the
hidden result, and a `Vector3(x, y, z)` temporary goes through the x87.

## MediaControl (16 of 17 exact)

RTTI `.?AVMediaControl@@` (vtable `0x005551c0`), a GameObject that plays a
movie through DirectShow multimedia streams; see `MediaControl.h` for the
GUIDs and interface slots.

- `0x004a2900` `Restart` (50 bytes) matches with a `do { ... break; ... }
  while (0)` body. That form places the shared `return 0` before `return 1`
  (`jl` to it, then `jge` over it). The `&&`, nested-if, early-return,
  `goto`, ternary and result-flag forms all put `return 1` first.
- Near miss: `0x004a2560` (open, 646 bytes) in
  `samples/gameui/MediaControlNearMisses.cpp`. Every call, argument and
  branch target matches. Retail allocates one more callee-saved register
  (a zero in `ebx`, `&field_0x60` in `ebp`), which shifts every frame
  offset by four.

## Reproduce

```bash
python3 tools/compile.py --compiler vc6 --vc6-root "$VC6_ROOT" src/reconstructed/MovingPart.cpp -o work/MovingPart.obj
python3 tools/match.py --exe "$MCM2_EXE" --target-va 0x004a23a0 --target-size 48 --obj work/MovingPart.obj --symbol '??0MovingPart' --bindings src/reconstructed/MovingPart.bindings.json
python3 tools/run_calibration.py --exe "$MCM2_EXE" --compiler vc6 --vc6-root "$VC6_ROOT"
```
