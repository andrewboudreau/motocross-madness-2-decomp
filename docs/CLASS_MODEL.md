# Class and source evidence

The known retail build has 252 RTTI descriptors, 249 logical class hierarchies
and 271 vtables, including 22 secondary tables. A type can have several Complete
Object Locators. Always use `object_offset`; virtual-base `pdisp`/`vdisp` values
and adjustor thunks are part of the layout.

| Class | Known base/subobject evidence |
|---|---|
| CollisionObject | QuadTreeObject, GraphicsTest; tables at +0, +12 |
| SoultreeObject | QuadTreeObject, GameObject; tables at +0, +12 |
| Tire | CollisionObject, MovingPart, CollisionPoint; +0, +12, +184 |
| Vehicle | Tables at +0, +540, +1472; virtual-base evidence also present |
| Bike | Tables at +0, +540, +1848; virtual-base evidence also present |

## Generated evidence

`make analyze` or the [native setup commands](TOOLCHAIN.md) produce:

| File under `analysis/` | Use |
|---|---|
| `rtti_classes.json`, `vtables.json` | Identities, bases and subobject tables |
| `vtable_overrides.json` | Inherited/overridden/introduced primary slots |
| `class_layout_hints.json` | Observed member offsets and widths |
| `deleting_destructors.json` | 145 recognized wrappers and destructor targets |
| `vtable_thunks.json` | 28 recognized this-adjustments |
| `vtable_write_xrefs.json` | 492 decoded vptr writes; ctor/dtor leads |
| `source_manifest.json`, `source_xrefs.json` | Literal source names and reference candidates |
| `class_dossiers.json`, `CLASS_DOSSIERS.md` | Joined evidence and source hints |
| `function_manifest.json`, `work_queue.json`, `WORK_QUEUE.md` | Candidate inventory and priority |

Dossiers/rankings are convenience views. Trace claims back to instructions,
RTTI or strings. The legacy queue's clang validation labels do not include the
VC6 matrix; see [matching status](VC6_MATCHING.md).

## Reconstruction rules

- Primary override comparisons do not establish secondary-table ownership.
- A vptr write can occur in construction, destruction or a member subobject.
  Pair it with wrapper/core evidence before naming a special member.
- A dword load proves width, not int/pointer/handle/enum identity or signedness.
- Shared tiny addresses can reflect inheritance or linker folding.
- The 107 observed cpp and four header names are embedded-name evidence, not
  a complete original project. Nearby source references do not prove TU ranges.

BaseObject's matched declaration is under `src/reconstructed/`. UIControl,
UIMultiState, physics and camera candidates retain offset-based names in
`samples/` until stronger type/ownership evidence exists. Filename skeletons
are generated under `generated/`, never counted as completed source.

```bash
python3 tools/find_class.py UIControl
python3 tools/discover_easy_targets.py --class UIControl
python3 tools/nearest_source.py 0x4703c0
make class-evidence msvc-artifacts manifest
```
