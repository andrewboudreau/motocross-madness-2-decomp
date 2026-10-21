# Bike rider-node connections

Bike and KrustyBike now call three existing SoultreeObject implementations
through the rider's model pointer. The `BikeA1A0` partial view remains in place
for its other fields and methods; explicit casts occur only at these calls.
No wrapper, alias implementation, new provider or header change was introduced.

| Retired call view | Existing provider | Evidence status |
| --- | --- | --- |
| `BikeA1A0::FindByName(const char*)` | `SoultreeObject::FindByName`, 0x004fdae0 | Full 84 bytes strict exact |
| `BikeA1A0::GetSubtreeBounds(Vec3*, Vec3*)` | `SoultreeObject::GetSubtreeBounds`, 0x004fe850 | Full 74 bytes strict exact |
| `BikeA1A0::SetMatrixIn(int, Matrix4*)` | `SoultreeObject::SetMatrixIn`, 0x004fb8c0 | Previously admitted reviewed nonexact body; 1134 candidate bytes versus 1096 retail |

The first two providers belong to the existing soultree.cpp reconstruction.
SetMatrixIn remains in `samples/physics/helpers/SoultreeMatrix.cpp` with its
existing explicit functional-build admission. This connection does not upgrade
its byte-match status or establish its original source layout.

## Receiver, return and parameter evidence

Bike's loader calls the D3DIMSoultreeCharacter constructor at 0x004455b0
(direct call 0x00407c6c), then stores the returned primary pointer at Bike+0x5c4
(0x00407c84). The existing character declaration holds its model node at
+0x1a0, the same offset used by `BikeA5C4::c_0x1a0` and
`BikeA34::r_0x1a0`. The character's node-building methods allocate a rendered
Soultree model and retain its primary pointer in that field.

Retail D3DIMSoultreeObject RTTI has primary object offset zero and a distinct
GameObject subobject at +0x0c (COLs 0x0055b2b8 and 0x0055b248). The selected
methods require the primary node pointer, not a conversion to GameObject.
Native layout checks preserve the partial view's child pointer at +0x140,
matching `SoultreeObject::firstChild`, and the local matrix at +0xb8.
The casts add no receiver adjustment; complete caller comparisons preserve
the supplying and consuming instructions.

`FindByName` returns a node pointer in EAX, recursively searching siblings
before children. Bike's bone-attachment sequence still carries that pointer
through its existing 32-bit integer parameter to `Wrecker::AddProbe`; an explicit
pointer-to-int cast replaces the old incorrectly integer-typed return. The
native regression requires equal pointer/int widths. No bone-index conversion
or new ownership behavior is introduced.

`GetSubtreeBounds` retains its two writable 12-byte Vec3 outputs, updating the
cached subtree bounds when dirty. SetMatrixIn retains its writable 64-byte
Matrix4 parameter. Every converted call supplies zero for the first parameter;
the canonical declaration identifies this as a null reference-frame pointer,
with the same stack value as the old integer zero. The existing calls' matrix
storage and lifetimes are unchanged. The corresponding KrustyBike near-miss
sample uses the same canonical call and binding.

## Verification

With native `VC6_ROOT` and the hash-pinned private `MCM2_EXE` configured:

```powershell
$env:PYTHONPATH = '.'
python tests/test_bike_node_connections.py -v
python tests/test_soultree_transform_behavior.py -v
```

The focused connection regression checks actual caller/provider COFF objects,
canonical direct-call imports and absence of the three retired names. It
preserves **189 strict cases** (51 Bike, 76 KrustyBike and 62 soultree), the
exact **41-byte** transpose helper, and complete relocation-resolved hashes of
**15 registered nonexact bodies**: seven Bike methods, six KrustyBike sample
methods and the two existing matrix methods. Partial hashes guard preservation;
they do not claim retail identity. Native layout and compile-only receiver
probes accompany raw retail constructor/RTTI checks.

The existing full standalone transform differential test was rerun and passed
**86,016** state and alias scenarios, including SetMatrixIn. Its existing
limitation remains: node-cache dependencies are scripted and shared retail math
is privately executed for both sides; this is not a whole scene-graph or racing
integration test.

The isolated real source build compiled **200/200** translation units and
reduced unresolved names **519 to 516**, exactly these three names, with none
added. The game still does not link.

## Deferred candidates

The render-specific `SetUseLighting` and `ClearScreenRects` view calls remain
separate pending compatible render/physics declarations. The canonical
Character methods at 0x004a8bf0 and 0x004a8c50 are themselves still missing, so
renaming their rider-view callers would not supply implementations. Scale at
0x004fd340 also lacks an admitted provider; its existing nonexact sample was
not promoted by this slice. QueryGround's known rounding defect is outside
this work and remains unchanged.
