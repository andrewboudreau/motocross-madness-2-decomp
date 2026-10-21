# Soultree transform candidates

These complete methods remain in `samples/physics/helpers/` and are explicitly
included in the functional build through `config/game_build.json`. They are
reviewed nonexact candidates, not additions to the strict-exact census.

| Method | Retail VA | Retail / VC6 bytes | Remaining difference |
| --- | --- | ---: | --- |
| WorldToLocalDirection | `0x004fd710` | 209 / 209 | One x87 multiply loads its operands in the other order |
| SetMatrixIn | `0x004fb8c0` | 1096 / 1134 | Transpose inlining and stack-slot assignment |
| SetAxesIn | `0x004fc050` | 1171 / 1171 | Stack-slot assignment |
| TransposeRotation3x3 | `0x004fefb0` | 41 / 41 | Strict exact helper, unchanged |

The class is confirmed by SoultreeObject RTTI, its two vtable writes and the
`+0/+0xc` base offsets. Attribution of these nonvirtual methods follows the
decoded field offsets and calls to UpdateWorldMatrix within the unit bracket
documented in `src/krusty2/soultree/soultree.cpp`. Method names remain inferred.
The existing physics view receives the complete node pointer without a base
adjustment. Local and world matrices are at `+0xb8/+0xf8`, world validity at
`+0x138` and the parent pointer at `+0x13c`.

WorldToLocalDirection updates the world matrix, copies it, transposes its
rotation and returns the transformed direction through the x86 hidden result
buffer. This uses a transpose, not a general matrix inverse.

SetMatrixIn delegates to SetLocalMatrix when the frame is the node itself.
Otherwise it converts the caller's matrix in place through a nonnull frame,
then expresses that matrix relative to the parent and invalidates the node's
world matrix. Both the caller's input matrix and the node's local matrix may
change. The collision-area matrix product now uses its existing canonical
`CollisionMatrixMultiply` spelling and definition at `0x00436500`; no adapter
or additional matrix implementation is supplied.

SetAxesIn constructs the rotation from the supplied axes, optionally rebuilds
one axis through cross products and normalizes both. The initial cross product
is axisY cross axisZ. For `frame == this`, it multiplies by the previous local
matrix and returns after invalidation. Other frame/parent paths restore the
saved local translation before invalidating. This distinction and the mutation
order are retained from the full decoded retail body.

## Functional build review

`tests/test_soultree_transform_behavior.py` links the actual standalone VC6
candidate objects and executes all three complete methods against the
hash-pinned retail image. It checks their COFF function extents independently,
the strict 41-byte transpose, all external direct-call destinations and the
nine node-cache call sites instrumented by the private fixture. No production
instructions or production linker behavior are changed for the test.

The 86,016 scenarios compare every byte of four node objects, external matrix
and vector storage, result storage, and ordered cache-call events. They vary
null/self/other frames, null/self/other parents, cache-valid flags, rigid and
general matrices, finite fractional values with varied exponents, zero and
parallel axes, and all orthogonalization flag combinations. The hidden return
buffer must also be returned in EAX. Vector results include exact input/output
aliasing and four/eight-byte overlap in both directions; axis inputs include
identical, overlapping and independently selected buffers. Matrix inputs can
overlap local/world storage in each participating node.

UpdateWorldMatrix, InvalidateWorldMatrix and SetLocalMatrix are scripted in
this differential test. The arithmetic dependencies execute the same retail
bodies in both runs; their actual canonical source providers remain covered
by the separate strict physics checks and the real game link. This establishes
the candidate callers' behavior under the tested cache actions and finite
inputs. It does not prove recursive scene-cache integration, universal NaN
payload behavior, or running gameplay. The reference mapping and dependency
substitution exist only in the private test fixture.

```powershell
$env:PYTHONPATH = '.'
$env:VC6_ROOT = (Resolve-Path work/vc6sp3-disc).Path
$env:MCM2_EXE = (Resolve-Path work/game/mcm2.exe).Path
python -m unittest discover -s tests -p test_soultree_transform_behavior.py -v
python tools/run_physics_samples.py --compiler vc6 --vc6-root $env:VC6_ROOT --profile vc6_o2_mt --strict --source samples/physics/helpers/SoultreeMatrix.cpp --source samples/physics/helpers/SoultreeTransform.cpp
python tools/build_game.py -j6
```
