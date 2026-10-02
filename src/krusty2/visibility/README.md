# VisibilityQuadTree.cpp

Validation: counts labeled "exact" below are historical relocation-masked
diagnostics, not strict acceptance. Use `tools/run_physics_samples.py --strict`
with reviewed bindings before accepting these candidates.

- `__FILE__` string at 0x575a3c, xref 0x52d4c9 (node factory, `new(__FILE__, 0x4f)`).
- Bracket 0x52d22a..0x5300f8. The front (0x52d240, 0x52d250, 0x52d2f0) is VideoCard;
  0x52ff90/0x5300a0/0x5300c0 are Wrecker; 0x52ff00/0x52ff20 are not VisibilityQuadTree.
- `VisibilityQuadTree : QuadTree (+0), GameObject (+0x874)`; primary vtable 0x558dfc,
  secondary (GameObject shape) 0x558d8c. `VisibilityQuadTreeNode : QuadTreeNode`, vtable 0x558e08.
- 16 exact (see targets.json), Traverse 0x52d610 partial (72.7%, 3026 vs 3027 bytes, call-site arg scheduling). Samples: 3 exact, 3 partial (VisProjectPoint, VisCullQuad, VisSphereInFrustum).
- Not done: 0x52f570 (box test, VisibilityClipper thiscall, 1358 bytes), 0x52fdc0, 0x52f190 (argument layout still unclear).
- Camera layout conflict: DrawBox receives camera+0x18 as renderer, contradicting the header's matrix-at-+0 assumption.
