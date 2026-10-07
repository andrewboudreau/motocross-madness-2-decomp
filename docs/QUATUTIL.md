# Quaternion helpers (MatrixUtil..PhysicsRigidBody gap)

Canonical reconstruction: `src/reconstructed/QuatUtil.cpp`. The range is
`0x004a1d10..0x004a2350`. No `__FILE__` literal or RTTI reaches it, so the
file name is ours (tier 3). `0x004a1d10` and `0x004a1f90` were already
matched in `samples/physics/common/Math3D.cpp` and stay there. Whether they
belong to the same unit as the functions below is open.

## Matched (11 of 11 unregistered, strict exact)

- `0x004a1d60` QuatNormalize (173 bytes). Returns `q / |q|`, or the
  identity at `0x006850b0` when `|q|^2 == 0`.
- `0x004a1e10` QuatToMatrix (381 bytes). The row-vector rotation matrix of
  `q`, scaled by `2 / |q|^2` so that `q` need not be normalized.
- `0x004a2040` QuatFromMatrix (462 bytes). Picks the largest of `w^2`,
  `x^2`, `y^2`, `z^2` from the trace terms, takes its root and scales the
  other three by `0.25 / root`.
- `0x004a2210..0x004a2350`: the unit's own copy of the four `Vector3`
  constants (XCU entries 179-182, globals `0x00685070..0x006850ac`). Each
  has a 5-byte `jmp` thunk (`$E2`, `$E5`, `$E8`, `$E11`) and a 60-byte body
  (`$E1`, `$E4`, `$E7`, `$E10`). Nothing reads these copies.

PhysicsRigidBody (`0x004cc6c4`, `0x004cc960`, `0x004cc98f`) calls the
three quaternion functions. The names and the `Quat` layout (scalar first)
are inferred from the arithmetic, not from symbols.

Source forms that mattered:

- `Quat` is a POD with no constructors. With constructors, QuatNormalize's
  result components land in other registers.
- QuatNormalize uses one `result` local assigned by fields. Separate
  `return` statements split the epilogue.
- In QuatToMatrix the products are declared in the order `w*`, then the
  cross terms (`xy`, `xz`, `yz`), then the squares. Declaring the squares
  before the cross terms gives the same instructions with different stack
  slots.

## Reproduce

```bash
python3 tools/compile.py --compiler vc6 --vc6-root "$VC6_ROOT" src/reconstructed/QuatUtil.cpp -o work/QuatUtil.obj
python3 tools/match.py --exe "$MCM2_EXE" --target-va 0x004a1e10 --target-size 381 --obj work/QuatUtil.obj --symbol '?QuatToMatrix@@YA?AUMatrix4@@ABUQuat@@@Z' --bindings src/reconstructed/QuatUtil.bindings.json
python3 tools/run_calibration.py --exe "$MCM2_EXE" --compiler vc6 --vc6-root "$VC6_ROOT"
```
