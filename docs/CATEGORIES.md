# Allocation categories and source navigation

Categories identify accounting contexts, not exclusive source/class ownership.
Keep a many-to-many relationship. A label does not establish a filename or
justify moving source files.

| Command | Generated output | Purpose |
|---|---|---|
| `make categories` | `analysis/categories/` | Selector anchors and source/RTTI associations |
| `make category-contexts` | `analysis/category_contexts/` | One-hop context, reverse source lookup and category packets |

Both reproduce 52 literal arguments among 55 selector calls across 15 labels.
The allocation analyzer's narrower recognizer reports 45; these have different
scopes. All outputs are generated, hash-bound and ignored by Git.

```bash
python3 tools/build_categories.py --query Terrain
python3 tools/build_categories.py --query QuarryStuntEvent.cpp
python3 tools/build_category_contexts.py --category Terrain
python3 tools/build_category_contexts.py --source EcoSystem.cpp
python3 tools/build_category_contexts.py --address 0x005079f0
```

Use `--exe`, `--out` and the anchor tool's `--provenance` for other paths.
Queries read existing snapshots; absence is not an ownership conclusion.
Live analysis requires GNU objdump.

## Useful anchors

| Candidate | Evidence / interpretation |
|---|---|
| `0x0042f600` | Collision, CarProcedural.cpp and primary virtual use |
| `0x0045aad0` | EcoSystem.cpp and slot 12; [candidate](ECOSYSTEM.md) |
| `0x005079f0` | Terrain source/vptr clues; confirmed [destructor core](CATEGORY_PILOTS.md) |
| `0x004857f0`, `0x00485830`, `0x00485870` | UI selections and GUIManager slots |
| `0x004de590` | QuarryStuntEvent.cpp orchestration selecting twelve categories |
| `0x00521050` | trkgame.cpp orchestration selecting Audio, Scene, Startup, UI |

One-hop clues include TextureMapManager.cpp, AuralScape.cpp, PCAudio.cpp and
krustyui.cpp. Shared callees retain every observed context. Do not propagate
labels to callees' callees, whole files, sibling methods or derived classes.

## Unresolved evidence

Arguments at `0x00401771`, `0x004dfa95` and `0x00500fca` remain unresolved.
Recognizers stop at unsupported writes, stack changes, calls, branch entries
and decoding gaps rather than substituting nearby strings.

Candidate bounds are heuristic. One-hop reachability before a local category
boundary does not prove runtime state: unknown callees may change it; exception
and indirect-call effects are unresolved. Primary-slot evidence excludes unchanged
inheritance; secondary tables are not flattened. [Lifetime checks](CATEGORY_PILOTS.md)
independently refine selected candidates beyond this navigation map.
