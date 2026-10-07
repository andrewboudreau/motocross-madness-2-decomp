ProjectedShadow and TerrainShadow candidates are preserved in `samples/physics/shadow/`.
D3DIMSoultreeShadow.cpp has 17 strictly verified cases with adjacent bindings.
Shared headers stay here. See [physics validation](../../../docs/PHYSICS_VALIDATION.md).

# ProjectedShadow.cpp (shadow)

Validation: every exact target of `samples/physics/shadow/` passes strict VC6 SP3
comparison with `ProjectedShadow.bindings.json` / `TerrainShadow.bindings.json`
(ProjectedShadow.cpp 19 of 26 targets, TerrainShadow.cpp 4 of 5; the rest are partial).

Evidence
- `__FILE__` string "ProjectedShadow.cpp" at VA 0x571fd8; own xrefs 0x4da745..0x4dacbc
  (new at line 0xaa/0xcd in Init, array growth 0xfe/0x102, 0x115/0x119, vertex buffer 0x131/0x134).
- Bracket 0x4da35c..0x4dc729. The front (0x4da520..0x4da560) is ShadowCamera, not ours;
  ProjectedShadow's ctor starts at 0x4da570. 0x4dc4c0 is Terrain's shared stub (skipped).
- The vector `$E` set 0x4dc4d0..0x4dc60b closes the file (8 targets, strict with
  `ProjectedShadow.bindings.json`): only the ctor reads its zero vector 0x689b48, and
  `.CRT$XCU` lists it after ProCircuitProcs.cpp's set (see docs/INITIALIZERS.md).
- RTTI: ProjectedShadow : GameObject (vptr at +0). Overrides slots 10 (float dt), 12, 13.
- Layout: see the `// +0xNN` comments in ProjectedShadow.h. Size >= 0x134.

Counts: ProjectedShadow.cpp 19 strict exact, 7 partial of 26 targets; D3DIMSoultreeShadow.cpp 17 strict exact.

D3DIMSoultreeShadow.cpp (D3DIMSoultreeShadow : ShadowReceiver : GameObject, vtable 0x55156c; ShadowReceiver 0x5515ec):
ctor 0x446840, Attach 0x4468b0, slot 28 0x446bf0, both deleting dtors (0x446890 is shared with TerrainShadow by
identical code folding, 0x4477a0), ShadowReceiver slot 8 / dtor, and the four ShadowConstVec3 `$E` pairs are exact.
Slot 14 (0x447540, 574 bytes) is exact too; it fills the file's vertex array g_d3dimShadowVertices
(0x581eb8, 3000 32-byte vertices). Open: slots 27, 29 and 30 (inlined 4x4 matrix products, x87 operand
order). The 0x447910 setter is placed with DebugOverlay.cpp (docs/DEBUGOVERLAY.md). Ownership: slot 30 (0x446f40) pushes the file's own
`__FILE__` 0x568cac at 0x447023/0x447043, and the other methods are contiguous members of the same class.

Partial: ctor 84% (prevMinX/Y store scheduling), Init 19.6%, SetLight 6%, ComputeBounds 5.5%
(1761/2020 B), RenderShadow 2.3% (884/1074 B), Present 12.5% (655/1218 B), TintCasterVertices 9% (354/352 B).

Notes
- 0x004dc2b0 iterates the casters (+0x2c/+0x30), not receivers, so it is named TintCasterVertices.
  Retail contains an inline fistp sequence. Its original source mechanism is unproven;
  the tested C++ casts produce __ftol, so it stays partial under the no-asm rule.
- RenderShadow and Present clear with unrolled dword stores through 15/45/30/15-entry jump tables
  (included in the extents); plain loops are used instead.
- DebugMalloc(size, file, line) at 0x4a2e20 uses core/DebugAlloc.h.
- Helper stand-ins (tier 3): ShadowMatrixIdentity 0x4a1410, ShadowMatrixMultiply 0x4a1860,
  ShadowTransformPoints* 0x4a1b00/0x4a1a50, ShadowFillTriangle 0x461e60, caster 0x4433f0/0x445030/0x4fe850/0x4fdab0.

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
