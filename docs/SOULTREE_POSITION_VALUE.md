# By-value Soultree position providers

Two existing by-value call views now have complete readable implementations:
`SoultreeObject::SetPositionVec3(Vec3)` in SoultreePositionValue.cpp and
`D3DIMSoultreeObject::SetPositionVec3(Vector3)` in D3DIMPositionValue.cpp.
Both match the same complete **38-byte** retail body at `0x004fc630` under
authentic VC6 SP3 `vc6_o2_mt`, with resolved relocations. These are two source
call views of one retail implementation, not two recovered retail functions.
Original overload spelling and translation-unit ownership remain unproven.

Each actual body copies its three argument words to the node's translation at
`+0xe8/+0xec/+0xf0`, then calls the existing InvalidateWorldMatrix provider.
Neither forwards through the other setter. Their only external dependencies
are that real cache helper and the compiler's `__fltused` marker. The
physics-view translation unit also emits Math3D's ordinary private vector
storage and initializers, accounted below; the rendering-view translation unit
emits neither. The existing scalar SetPosition(float,float,float) body at the
same retail address remains unchanged.

All existing callers and bindings remain unchanged. The rendering
SoultreeObject declaration gains only the actual, already-implemented
`InvalidateWorldMatrix()` prototype beside UpdateWorldMatrix. It changes no
virtual slot, inheritance relation or object layout.

## Receiver and value argument

The two production call sites retain their twelve-byte by-value contract:

| Caller | Complete strict extent | Retail call |
|---|---:|---:|
| D3DIMSoultreeCharacter::CharacterVirtualSlot4, `0x00446210` | 235 bytes | `0x00446291` |
| LightEmitter::SetPosition, `0x0049e150` | 141 bytes | `0x0049e1a0` |

The motion controller's nodeNames rows have a 64-byte stride and a primary
node pointer at row `+0x34`. FillNodeNames (`0x00445460`, 321 bytes) stores its
incoming node pointer there at `0x00445479` and follows child/sibling links.
Slot4 reads that row pointer and copies all three words from the pose position
to the outgoing argument area. Its existing SoultreeObject call is untouched.

LightEmitter::Create (`0x0049e230`, complete 348-byte extent including jump-table
data) allocates `0x2d8` bytes, calls the D3DIMSoultreeObject constructor at
`0x0049e29b`, and stores its primary result in LightEmitter `+0x98` at
`0x0049e2af`. SetPosition reloads that same pointer into ECX. The independently
strict 275-byte constructor at `0x0043f160` writes both object tables:

- Primary table `0x005513ec`, COL `0x0055b2b8`, object offset zero.
- GameObject table `0x0055137c`, COL `0x0055b248`, object offset 12.

The raw type name is `.?AVD3DIMSoultreeObject@@`. D3DIMSoultreeObject,
SoultreeObject and QuadTreeObject have PMD `(0,-1,0)`; GameObject and BaseObject
have `(12,-1,0)`. The loading call's separate GameObject result feeds
AppendChild, rather than replacing the stored primary pointer.

Independent compiler layout checks retain both vectors' twelve-byte size and
component offsets `0/4/8`, model size, and node translation/cache offsets.
The complete caller bodies retain argument snapshots and primary receivers.
These checks do not equate every provisional field or establish original
header ownership.

## Complete preservation

Scalar-argument rewrites change the strict motion caller from 235 to 223 bytes
and the light caller from 141 to 133 bytes, so they are rejected. A by-value
base-declaration join preserves those callers but changes the strict Scene
light reader; that join is also rejected. The final implementation retains
both existing call declarations and supplies their actual bodies, sharing only
the genuine cache helper. No caller changes or matching-only declarations are
retained, and no strict target or partial fingerprint is weakened.

The retained same-path before/after proof covers:

| Object | Nondebug sections | Relocations |
|---|---:|---:|
| LightEmitter | 57 | 172 |
| D3DIMSoultreeMotnctrl | 62 | 273 |
| SceneManagerNearMisses | 96 | 484 |
| BikeRaceNearMisses | 206 | 1,929 |
| SceneManager | 272 | 1,273 |
| Total | **693** | **4,131** |

Every section payload, relocation location, kind, addend and resolved
destination is preserved with **no symbol substitutions**. Both aggregates
retain their complete partial bodies. Private evidence is in
`work/connections43/final-proof/`.

Portable retained pins recognize only complete source-filename strings and
normalize checkout prefixes. Public RTTI COMDATs are keyed and sorted by
actual exported identity because VC6 can emit them in a different order for a
longer checkout path. Their full bytes and relocation destinations remain
checked; anonymous sections retain kind and ordinal. No instruction or data
payload is omitted. Parent integration separately compares all old production
objects on canonical paths to cover the remaining header consumers.

## Header storage and startup

SoultreePositionValue includes Math3D.h for the existing physics types. Under
VC6 it therefore emits four additional **private** per-TU vector objects:
zero, X axis, Y axis and Z axis. They occupy 60 bytes of writable, zero-filled
BSS, including alignment gaps: zero at offset 0, Z at 16, X at 32 and Y at 48.
They are not exports or aliases for any existing game-global vectors.

The object has 12 nondebug sections: compiler directives, the setter, four
5-byte initializer jump thunks, four 60-byte initializer bodies, BSS, and a
16-byte `.CRT$XCU` table. Its four startup entries target the corresponding
local thunks, whose bodies initialize the three components at offsets 0, 4
and 8 within each private vector. The retained test checks all eight complete
initializer bodies and relocations against the same header emissions in the
canonical soultree object, whose eight retail targets are independently strict.
It also pins BSS size and writable flags, local symbol ownership/offsets, all
CRT entries, and each initializer's exact local destination and addends.

This is additional reconstructed-TU storage and startup work. Matching the
shared header emissions does not establish an original retail TU or assign
these new private objects retail addresses. The setter and cache helpers do
not read these vectors; native setter comparisons do not claim such use.
D3DIMPositionValue has only two nondebug sections, compiler directives and its
setter body, with no storage or startup entries. Production sources remain
unchanged by these accounting checks.

## Native validation

`test_soultree_position_value` passes **121 strict comparisons**, all five
complete-object pins, and **158,212 native scenarios for each setter**. It
checks all registered LightEmitter, SceneManager and D3DIMSoultreeMotnctrl
strict bodies, both new setters, the unchanged scalar setter, the model
constructor, and these actual cache helpers:

| Helper | Retail address | Complete extent |
|---|---:|---:|
| InvalidateWorldMatrix | `0x004fb850` | 45 bytes |
| InvalidateSiblingChain | `0x004fb880` | 50 bytes |
| MarkSubtreeDirty | `0x004fdab0` | 38 bytes |

Native execution links both actual new source objects with unchanged helper
definitions extracted from canonical soultree source. All three helpers have
identical complete bytes and relocation records to their production-TU
counterparts and independently match retail. A separate validation-only caller
compiled with the actual rendering declaration invokes the rendering setter;
its sole external dependency is that setter. The reference executes the four
unmodified retail bodies. There are no patched dependency calls, scripted
helpers, substituted constants or retail instructions in candidate code.

The matrix comprises:

- **147,456 normal scenarios:** every seven-node cache-valid mask, sixteen
  raw-bit seeds, six independent/exact-overlap/shifted-overlap input views,
  all four x87 rounding modes and all three precision modes.
- **10,752 protected-page scenarios:** receiver, parent, child or sibling at
  21 accessible-tail boundaries, sixteen bit seeds and eight cache masks.
- **Four explicit outcome scenarios:** correct translation and ancestor/child
  invalidation; unchanged target sibling; already-stale subtree rules; and
  first/second components stored before a protected third-component write.

Raw bits include signed zeros, subnormals, finite extremes, infinities, quiet
NaNs and signaling NaNs. For each scenario, both source setters separately
match the retail result. The test compares all seven prepared nodes and guard
bytes, input storage, accessible fault page, exception kind/address and final
FP status/control. Every normal case also independently checks the exact
argument words in the destination and absence of new FP status flags.

These are prepared raw node views, without constructors, model loading or a
running scene. FP exceptions are masked; transient register/stack context,
unmasked traps and arbitrary cyclic or malformed hierarchies are outside the
finite claim. The fixture invokes the setters, not complete LightEmitter or
character-pose playback; those callers have complete strict and whole-object
proofs. Original source grouping and whole-game functionality remain separate
questions.

The existing two unresolved call-view names gain actual providers. Final real
link counts and full composition checks are recorded by integration in
[GAME_BUILD.md](GAME_BUILD.md); rejected declaration experiments are not the
admitted path. Linking, startup and gameplay remain incomplete.

```powershell
$env:PYTHONPATH='.;tests'
$env:VC6_ROOT=(Resolve-Path work/vc6sp3-disc).Path
$env:MCM2_EXE=(Resolve-Path work/game/mcm2.exe).Path
python -m unittest test_soultree_position_value -v
```
