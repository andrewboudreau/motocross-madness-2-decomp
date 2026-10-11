ProjectedShadow.cpp and D3DIMSoultreeShadow.cpp are here; TerrainShadow candidates are preserved in
`samples/physics/shadow/`. Shared headers stay here. See [physics validation](../../../docs/PHYSICS_VALIDATION.md).

# ProjectedShadow.cpp (shadow)

Extent: `0x004da570..0x004dc610` (ShadowCamera's methods `0x004da520..0x004da560` precede it; the
`or ax, 0xffff` stub `0x004dc610` shared by seven other vtables follows). 21 targets of
`src/krusty2/shadow/targets.json` pass strict VC6 SP3 comparison with `ProjectedShadow.bindings.json`.

Evidence
- `__FILE__` string "ProjectedShadow.cpp" at VA 0x571fd8; own xrefs 0x4da745..0x4dacbc
  (new at line 0xaa/0xcd in Init, array growth 0xfe/0x102, 0x115/0x119, vertex buffer 0x131/0x134).
- RTTI: ProjectedShadow : GameObject (vptr at +0, vtable 0x5575c8). Overrides slots 10 (float dt), 12, 13
  and 23; slot 23 is the `xor eax, eax; ret 8` stub 0x4dc4c0 between slot 13 and the `$E` set, which
  identical-code folding shares with Terrain and StatsOverlay (registered with `broadphase/Terrain.cpp`).
- The vector `$E` set 0x4dc4d0..0x4dc60b closes the file: only the ctor reads its zero vector 0x689b48, and
  `.CRT$XCU` lists it after ProCircuitProcs.cpp's set (see docs/INITIALIZERS.md).
- Layout: see the `// +0xNN` comments in ProjectedShadow.h. Size >= 0x134.

Functions (all in the extent)
- Exact in `ProjectedShadow.cpp`: ctor 0x4da570, scalar deleting dtor 0x4da6c0, dtor 0x4da6e0, AddCaster
  0x4dab00, ClearCasters 0x4dab90, AddReceiver 0x4daba0, ResizeVertexBuffer 0x4dac50, SetShadowStrength
  0x4dace0, SetShadowGrey 0x4dae30, SetLight 0x4dae50, slots 10/12/13 (0x4dafc0, 0x4db000, 0x4dc410) and the
  eight `$E` functions 0x4dc4d0..0x4dc5d0.
- Near misses in `samples/physics/shadow/ProjectedShadowNearMisses.cpp` (includes the canonical file):
  Init 0x4da7b0, ComputeBounds 0x4db0d0, Present 0x4dbd30 (scores in docs/NEAR_MISS_INDEX.md).
- Excluded (inline asm): RenderShadow 0x4db8c0 and TintCasterVertices 0x4dc2b0 round with bare
  `fistp dword ptr [eax]` in an ebp frame (0x4dbb5c/0x4dbb8b, 0x4dc351/0x4dc36c); their C++ candidates use
  `(int)` casts and stay registered partials.

Notes
- The ctor 0x4da570: the previous-bounds fields are written per axis (min x, max x, min y, max y) after
  receiverCapacity; written as max pair then min pair, VC6 hoists the 0x7fffffff stores.
- SetLight takes (pos, dir, mode): KrustyUI 0x4998c0 and QuarryStuntEvent 0x4e00e2 push the mode first.
  The direction goes through an inline Vec3Normalize-shaped helper, and one `mode = newMode` store after the
  if/else chain keeps newMode in ebx as retail does.
- Present has one row loop per format; its three jump tables are expansions of a
  Duff's-device fill of the 16-bit rows (`FillShortsUnrolled` in the near-miss file); RenderShadow's clear
  has a fourth table of the same shape.
- 0x004dc2b0 iterates the casters (+0x2c/+0x30), not receivers, so it is named TintCasterVertices.
- DebugMalloc(size, file, line) at 0x4a2e20 uses core/DebugAlloc.h.
- Helper stand-ins (tier 3): ShadowMatrixIdentity 0x4a1410, ShadowMatrixMultiply 0x4a1860,
  ShadowTransformPoints* 0x4a1b00/0x4a1a50, ShadowFillTriangle 0x461e60, caster 0x4433f0/0x445030/0x4fe850/0x4fdab0.

# D3DIMSoultreeShadow.cpp (shadow)

D3DIMSoultreeShadow.cpp has 18 strictly verified cases with adjacent bindings.
D3DIMSoultreeShadow.cpp (D3DIMSoultreeShadow : ShadowReceiver : GameObject, vtable 0x55156c; ShadowReceiver 0x5515ec):
ctor 0x446840, Attach 0x4468b0, slot 28 0x446bf0, both deleting dtors (0x446890 is shared with TerrainShadow by
identical code folding, 0x4477a0), ShadowReceiver slot 8 / dtor, and the four ShadowConstVec3 `$E` pairs are exact.
Slot 14 (0x447540, 574 bytes) is exact too; it fills the file's vertex array g_d3dimShadowVertices
(0x581eb8, 3000 32-byte vertices). Slot 27 0x4468f0 (767 B) is exact: the caster-bounds box test
(0x52f570) against the shadow camera after an inlined 4x4 product, whose helper reads b through a
second reference in _24 so that VC6 orders the row-2 sums as retail. Near misses
(samples/physics/shadow/D3DIMSoultreeShadowNearMisses.cpp): slot 29 0x446c30 (771 B, 730/771), the same
test against the render target's camera; only the product's x87 term order differs. Slot 30 0x446f40 (1524 B,
180/1485) builds the receiver vertices from the caster's current LOD meshes; retail rounds the clipped points
with an inline __asm fld/fistp helper (ebp frame), so it stays partial under the no-asm rule. The 0x447910 setter is placed with DebugOverlay.cpp (docs/DEBUGOVERLAY.md). Ownership: slot 30 (0x446f40) pushes the file's own
`__FILE__` 0x568cac at 0x447023/0x447043, and the other methods are contiguous members of the same class.

## D3DIMSoultreeShadow binding evidence

RTTI independently identifies the primary tables `0x0055156c` for
D3DIMSoultreeShadow and `0x005515ec` for ShadowReceiver, both at object offset
zero. The constructor calls the independently reconstructed GameObject ctor,
installs its table, zeroes seven fields, and fills 3,000 shorts at `0x005995b8`:
`mov [eax],cx; add eax,2; inc ecx; cmp eax,0x0059ad28`.

Attach calls GameObject slot 8 and ProjectedShadow::AddReceiver at `0x004daba0`.
The latter's body grows and appends to its receiver list at +`0x128`/+`0x12c`;
its own debug allocation references ProjectedShadow.cpp. Slot 28 reads the
texture size and stores the reciprocal texel value at `0x0057efa4`; the
`0.5f` and `1.0f` constants were checked at `0x005507f4` and `0x00550748`.

The deleting wrapper at `0x00446890` calls the 11-byte generated core at
`0x00508b70`, which writes the ShadowReceiver table and tail-calls GameObject's
destructor. This is the same teardown shape also emitted at `0x004477c0`.
The RTTI-backed wrapper and decoded vptr/call behavior support the binding;
a shared tiny address alone is not used to establish exclusive method identity.

Four initializer thunks select the bodies at `0x004477e0`, `0x00447830`,
`0x00447880`, `0x004478d0`. Their decoded three-component writes establish the
zero/X/Y/Z constants at `0x0057efa8`, `0x0057efb8`, `0x0057efc8`, `0x0057ef98`.
All sixteen cases compare complete compiler extents after applying relocations.
The other declared rendering overrides remain unreconstructed.
