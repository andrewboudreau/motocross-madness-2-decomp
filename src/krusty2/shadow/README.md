Candidate implementations for this area are preserved in `samples/physics/shadow/`.
Shared headers stay here. See [physics validation](../../../docs/PHYSICS_VALIDATION.md).

# ProjectedShadow.cpp (shadow)

Validation: counts labeled "exact" below are historical relocation-masked
diagnostics, not strict acceptance. Use `tools/run_physics_samples.py --strict`
with reviewed bindings before accepting these candidates.

Evidence
- `__FILE__` string "ProjectedShadow.cpp" at VA 0x571fd8; own xrefs 0x4da745..0x4dacbc
  (new at line 0xaa/0xcd in Init, array growth 0xfe/0x102, 0x115/0x119, vertex buffer 0x131/0x134).
- Bracket 0x4da35c..0x4dc729. The front (0x4da520..0x4da560) is ShadowCamera, not ours;
  ProjectedShadow's ctor starts at 0x4da570. 0x4dc4c0 is Terrain's shared stub (skipped).
- RTTI: ProjectedShadow : GameObject (vptr at +0). Overrides slots 10 (float dt), 12, 13.
- Layout: see the `// +0xNN` comments in ProjectedShadow.h. Size >= 0x134.

Counts: 11 exact, 7 partial of 18 targets.
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
