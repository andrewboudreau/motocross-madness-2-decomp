# VisibilityQuadTree.cpp

Validation: all 16 required targets pass strict VC6 SP3 comparison with every
relocation resolved by `VisibilityQuadTree.bindings.json`. Traverse remains a
partial candidate; the separate helper samples are not covered by that claim.

- `__FILE__` string at 0x575a3c, xref 0x52d4c9 (node factory, `new(__FILE__, 0x4f)`).
- Bracket 0x52d22a..0x5300f8. The front (0x52d240, 0x52d250, 0x52d2f0) is VideoCard;
  0x52ff90/0x5300a0/0x5300c0 are Wrecker; 0x52ff00/0x52ff20 are not VisibilityQuadTree.
- `VisibilityQuadTree : QuadTree (+0), GameObject (+0x874)`; primary vtable 0x558dfc,
  secondary (GameObject shape) 0x558d8c. `VisibilityQuadTreeNode : QuadTreeNode`, vtable 0x558e08.
- 16 exact (see targets.json), Traverse 0x52d610 partial (72.7%, 3026 vs 3027 bytes, call-site arg scheduling). Samples: 3 exact, 3 partial (VisProjectPoint, VisCullQuad, VisSphereInFrustum).
- Not done: 0x52f570 (box test, VisibilityClipper thiscall, 1358 bytes), 0x52fdc0, 0x52f190 (argument layout still unclear).
- The verified debug walk uses a typed renderer pointer at camera+0x18. The partial
  projection probe has a separate provisional `VisProjectionRecord`; its matrix-prefix
  hypothesis is not asserted as part of VisibilityCamera.

## Relocation evidence

Bindings distinguish observed addresses from provisional semantic names:

- RTTI/COL records identify QuadTree's primary table `0x0055763c`,
  VisibilityQuadTree's primary table `0x00558dfc` at object offset zero and
  GameObject table `0x00558d8c` at offset `0x874`, and the node table
  `0x00558e08`. The constructor's stores at `0x0052d35f`, `0x0052d37d`
  and `0x0052d383` preserve those different subobjects.
- The constructor stores `this` to `0x0068aba4` at `0x0052d389`; the destructor
  clears that same global at `0x0052d425`. CollisionObject reads it for the
  persistent broad-phase tree. Query instead installs its current traversal
  at `0x00689b78` (`0x0052d579`), also used by QuadTreeNode allocation.
  These must be separate C++ objects: `g_collisionQuadTree` and `g_pQuadTree`.
  The original candidate incorrectly used the latter name for both.
- The constructor initializes sixteen `0x20`-byte records starting at
  `0x0068a774`; its +8 cursor runs from `0x0068a77c` to `0x0068a97c`.
  Slot 23 toggles the two distinct flags at `0x0068ab9c` and `0x0068aba0`
  and copies `0x220` bytes to `0x0068a968`. Slot 12 reads that snapshot,
  stores elapsed ticks at `0x0068ab88`, and updates the record at `0x0068ab90`.
  These roles, rather than proximity, support the provisional global names.
- Literal contents were checked at `0x00575a3c` (the full retail source path),
  `0x0056a544` (`QuadTree`), `0x0056a5ac` (`Memory %d`), and `0x0056a594`
  (`PrepareGeometry %d %d`). They are address-specific bindings, not masks.
- Calls to GameObject's constructor, destructor and slot 8 agree with the
  canonical GameObject evidence and its primary RTTI table. QuadTree's
  Reset, Init, EndQuery and node lifetime calls agree with the separately
  reconstructed broad-phase targets. Local calls and the secondary deleting
  thunk agree with `targets.json`; the thunk adjusts `this` by `0x874`.
- The compiler EH stubs at `0x0054f108`, `0x0054f128`, and `0x0054f154`
  load FuncInfo pointers `0x005645d0`, `0x005645f8`, and `0x00564620`
  respectively, then jump to `0x0053471a`. Each is paired with the decoded
  SEH registration prologue of its constructor, destructor or node factory.
- The clock helper `0x004bfa80` calls the imported timer API. The statistic
  helpers `0x004cb6b0`/`0x004cb690` update/read its timestamp and retained
  value. The line writers `0x00447fa0`/`0x00447f40` operate on the log's
  line indices; `0x004a2d20` looks up a category string. Their callers and
  decoded data accesses support the signatures shown in the source; names
  remain provisional. `__ftol` is the VC6 CRT conversion routine.
- DebugDraw reads the inverse scale at `0x00689b74`, the clipper pointer
  at `0x00575a98`, and calls the box test at `0x0052f570`. DrawBox uses the
  vertex-buffer pointer at `0x0068a768`. These are supported by the direct
  calls and accesses documented in the header, not by a whole-file label.

Reproduce after private setup:

```bash
python tools/run_physics_samples.py --strict --root src/krusty2/visibility \
  --vc6-root "$VC6_ROOT" --exe "$MCM2_EXE"
```

The partial Traverse target is still reported, including its full compiler
extent. It is not trimmed to retail's length or counted as exact.
