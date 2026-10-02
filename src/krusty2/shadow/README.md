# ProjectedShadow.cpp (shadow)

Evidence
- `__FILE__` string "ProjectedShadow.cpp" at VA 0x571fd8; own xrefs 0x4da745..0x4dacbc
  (new at line 0xaa/0xcd in Init, array growth 0xfe/0x102, 0x115/0x119, vertex buffer 0x131/0x134).
- Bracket 0x4da35c..0x4dc729. The front (0x4da520..0x4da560) is ShadowCamera, not ours;
  ProjectedShadow's ctor starts at 0x4da570. 0x4dc4c0 is Terrain's shared stub (skipped).
- RTTI: ProjectedShadow : GameObject (vptr at +0). Overrides slots 10 (float dt), 12, 13.
- Layout: see the `// +0xNN` comments in ProjectedShadow.h. Size >= 0x134.

Counts: 26 exact, 7 partial of 33 targets (D3DIMSoultreeShadow.cpp added).

D3DIMSoultreeShadow.cpp (D3DIMSoultreeShadow : ShadowReceiver : GameObject, vtable 0x55156c; ShadowReceiver 0x5515ec):
ctor 0x446840, Attach 0x4468b0, slot 28 0x446bf0, both deleting dtors (0x446890 is shared with TerrainShadow by
identical code folding, 0x4477a0), ShadowReceiver slot 8 / dtor, and the four ShadowConstVec3 `$E` pairs are exact.
Open: slots 14, 27, 29, 30 and the 0x447910 setter. Ownership: slot 30 (0x446f40) pushes the file's own
`__FILE__` 0x568cac at 0x447023/0x447043, and the other methods are contiguous members of the same class.

Partial: ctor 84% (prevMinX/Y store scheduling), Init 19.6%, SetLight 6%, ComputeBounds 5.5%
(1761/2020 B), RenderShadow 2.3% (884/1074 B), Present 12.5% (655/1218 B), TintCasterVertices 9% (354/352 B).

Notes
- 0x004dc2b0 iterates the casters (+0x2c/+0x30), not receivers, so it is named TintCasterVertices.
  Its fistp helper (fstp/fld through a float temp, then `fistp [local ptr]`) is inline asm in retail,
  the same situation as Terrain QueryGround 0x507c10; the (int) casts produce __ftol, so it stays partial.
- RenderShadow and Present clear with unrolled dword stores through 15/45/30/15-entry jump tables
  (included in the extents); plain loops are used instead.
- The allocator 0x4a2e20 is `DebugMalloc(size, file, line)` in core/DebugAlloc.h.
- Helper stand-ins (tier 3): ShadowMatrixIdentity 0x4a1410, ShadowMatrixMultiply 0x4a1860,
  ShadowTransformPoints* 0x4a1b00/0x4a1a50, ShadowFillTriangle 0x461e60, caster 0x4433f0/0x445030/0x4fe850/0x4fdab0.
