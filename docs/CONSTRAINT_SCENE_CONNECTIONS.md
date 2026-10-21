# Constraint scene connections

The constraint solver now calls three existing `SoultreeObject` implementations
through its observed primary node pointers. The provisional `ConNode`,
`ConBody`, and `ConVec3` views remain separate; this change does not assert full
class equivalence or alter their layout.

| Previous call view | Existing provider | Retail extent | Production sites |
| --- | --- | --- | --- |
| `ConNode::GetPositionIn` | `SoultreeObject::GetPositionIn` | `0x4fc9a0`, 134 bytes | 2 |
| `ConNode::SetPositionIn` | `SoultreeObject::SetPositionInFrame` | `0x4fc740`, 265 bytes | 2 |
| `ConNode::LocalToWorldPoint` | `SoultreeObject::LocalToWorldPoint` | `0x4fd660`, 168 bytes | 1 |

These providers are emitted by the actual canonical `soultree.cpp` object and
match their complete retail bodies. The setter deliberately uses
`SetPositionInFrame`: the similarly named canonical `SetPositionIn` at
`0x4fc890` translates instead of setting the position. Both getter/setter pairs
in the constraint solver pass a null frame. The third call transforms a probe
point and returns a 12-byte vector through the VC6 hidden result pointer.
The explicit const-reference view keeps that temporary alive through its
assignment into the existing `ConVec3` local.

The receiving pointer has independent producer evidence:

- Bike slot 97 (`0x409420/1516`) finds each of fifteen bones through the rider's
  primary scene node and passes the returned pointer through the existing
  integer argument of `Wrecker::AddProbe`.
- `AddProbe` (`0x532900/210`) forwards that argument unchanged to
  `AddProbePoint` at `0x532924`. The latter stores it into offset `0xc` of a
  32-byte probe record at `0x43c82a`; the solver loads that same field for the
  point-transform receiver at `0x43bb8d`.
- `Wrecker::CreateBodies` (`0x5301c0/995`) passes the character model pointer at
  `+0x1a0` to rigid-body slot 28. The retail `PhysicsBody::SetNode` body
  (`0x4cbdf0/17`) stores the unchanged pointer at `+0x234`, which is the field
  the solver reads for its body-node receiver.

The model RTTI records `SoultreeObject` at primary offset zero and `GameObject`
at secondary offset `0xc`. These scene calls use the former; no secondary-base
adjustment is introduced. The constraint model's own secondary caller pointer
and the remaining provisional body declarations are unchanged.

Before and after the connection, the complete canonical object and its
near-miss aggregate preserve **89 nondebug sections and 271 relocations**:
all bytes, section properties, and relocation destinations match under the
three documented external symbol substitutions. No initializer or storage
section is added. In particular, the full 829-byte solver remains strict.
The aggregate's unadmitted `ApplyContactImpulse` still calls provisional node
methods of its own, so its bindings retain those mappings alongside the new
canonical names. Its complete 2619-byte body and resolved fingerprint remain
unchanged, with 2515 matching positions; it is not promoted.

`tests/test_constraint_scene_connections.py` checks **41 strict caller/producer
bodies**, including both canonical and aggregate copies, and the unchanged
complete partial. It also checks the five direct call destinations, decoded
pointer stores/loads, primary RTTI offsets, and native vector/body/probe
layouts. Nine unchanged node functions plus the existing matrix multiplier
are extracted for execution; each is checked against both its canonical
full-source object and the complete retail extent (**20 strict comparisons**).
The fixture links those actual functions without scripted helper replacements.

The native fixture executes **32,768 cases** over finite integer transforms
with signed orthonormal rotations, root/parent configurations, independent
cache flags, output and input aliases, and guarded storage. It checks all
object and output bytes, ancestor dirty flags, descendant invalidation,
untouched siblings, and setter/getter sequences against independent matrix
arithmetic. The executed getter/setter paths use the caller's null frame;
other frame branches remain covered by complete strict byte proof. Prepared
objects support these bounded method tests, not a claim of full construction,
physics-solver behavior, or working racing.

```powershell
$env:PYTHONPATH='.;tests'
$env:VC6_ROOT=(Resolve-Path work/vc6sp3-disc).Path
$env:MCM2_EXE=(Resolve-Path work/game/mcm2.exe).Path
python -m unittest test_constraint_scene_connections -v
```
