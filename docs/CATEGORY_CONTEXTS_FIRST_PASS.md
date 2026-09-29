# Memory categories: source-navigation atlas

Input SHA-256: `31fde4cc686a5ee89ef9095b90235325b195596867ecacefe511263e1509b874`

**Category context is not source ownership. No files were moved or renamed.**

Resolved 52 of 55 category-selection call arguments into 15 literal categories.
The graph links 29 source/header filenames and 196 one-hop candidates; 18 candidates appear in multiple category contexts.

| Category | Literal call sites | Containing candidates | One-hop candidates | Source/header links |
|---|---:|---:|---:|---|
| 3DObjects | 4 | 1 | 5 | ArcadeObject.cpp, QuarryStuntEvent.cpp |
| Audio | 4 | 3 | 11 | AuralScape.cpp, ContainerList.h, PCAudio.cpp, QuarryStuntEvent.cpp, bikerace.cpp, racesnd.cpp, trkgame.cpp |
| BikeRace | 3 | 2 | 14 | QuarryStuntEvent.cpp, RaceStatus.cpp, TrackOverlay.cpp, bikerace.cpp |
| Collision | 8 | 7 | 21 | ArcadeObject.cpp, CarProcedural.cpp, CollisionCharacter.cpp, CollisionObject.cpp, EcoSystem.cpp, KrustyBike.cpp, QuarryStuntEvent.cpp, SceneManager.cpp |
| DebugOverlay | 1 | 1 | 3 | Game.cpp |
| EcoSystem | 5 | 5 | 41 | EcoSystem.cpp, QuarryStuntEvent.cpp, SceneManager.cpp, Tgafile.cpp |
| Particles | 2 | 1 | 1 | QuarryStuntEvent.cpp |
| QuadTree | 3 | 3 | 4 | QuarryStuntEvent.cpp |
| Scene | 4 | 4 | 43 | CollisionCharacter.cpp, CollisionPoint.cpp, ContainerList.h, Parameterblocks.cpp, QuarryStuntEvent.cpp, SceneManager.cpp, trkgame.cpp |
| Shadow | 3 | 1 | 10 | ProjectedShadow.cpp, QuarryStuntEvent.cpp, SceneManager.cpp |
| Sky | 1 | 1 | 13 | QuarryStuntEvent.cpp, SceneManager.cpp |
| Startup | 2 | 2 | 11 | trkgame.cpp |
| Terrain | 4 | 4 | 27 | AgeManager.cpp, Griddraw.cpp, QuarryStuntEvent.cpp, SceneManager.cpp, Terrain.cpp |
| TextureCache | 1 | 1 | 1 | QuarryStuntEvent.cpp, TextureMapManager.cpp |
| UI | 7 | 6 | 28 | Net.cpp, TrackRecord.cpp, krustyui.cpp, trkgame.cpp, uiinfo.cpp |

The last column combines two explicitly separate evidence classes: a path referenced in the same CFG candidate as the selector, or a path referenced in a one-hop callee before a local category boundary. It is not a directory reorganization plan.

## Strong starting points

- `0x0045aad0`: labels EcoSystem; source anchors EcoSystem.cpp; class clues EcoSystem.
- `0x005079f0`: labels Terrain; source anchors Terrain.cpp; class clues Terrain.

These are review leads, not recovered function names or exact original TU assignments. Constructor/destructor vptr stores can also initialize members. A containing candidate may select many categories.

## Unresolved selection arguments

- `0x00401771`: dynamic_argument_or_unmodelled_write; not filled from nearby text.
- `0x004dfa95`: dynamic_argument_or_unmodelled_write; not filled from nearby text.
- `0x00500fca`: dynamic_argument_or_unmodelled_write; not filled from nearby text.

## Shared context example

- `0x00469190`: 3DObjects, Audio, Collision, DebugOverlay, Scene, Shadow, Sky, Terrain, UI. Owner remains unassigned.
- `0x00534aaf`: 3DObjects, EcoSystem, QuadTree, Scene, Sky, Terrain. Owner remains unassigned.
- `0x004e9cd0`: Collision, EcoSystem, Scene, Sky, Terrain. Owner remains unassigned.
- `0x00460d60`: Collision, EcoSystem, Scene, Sky. Owner remains unassigned.
- `0x00460d10`: Collision, EcoSystem, Scene. Owner remains unassigned.

## Evidence safeguards

- Categories name memory-accounting contexts, not necessarily original source folders or modules.
- Candidate function boundaries come from linear disassembly and bounded CFG traversal; they are not exhaustive.
- Source paths are decoded immediate references, including possible headers/inlined code; no original TU is assigned.
- One-hop calls are reachable before a local select/restore boundary, not proof of the runtime category at the callee.
- Unknown callees can change global accounting state; exception edges, jump tables and indirect callees are not resolved.
- Multiple category contexts are preserved, not collapsed into a single owner or propagated transitively.
- Vptr stores are class construction/destruction/member clues, not proof that the containing function is a class method.
- No source files are moved or renamed, and no new byte-match or CRT-identity claim is made by this pass.

Decoder: `GNU objdump (GNU Binutils for Debian) 2.44`. The input and reviewed select/restore bodies are hash-checked. The tool hashes are retained in context_map.json.

## Technical references

- Microsoft x86 thiscall: https://learn.microsoft.com/en-us/cpp/cpp/thiscall
- Predefined macros, including __FILE__: https://learn.microsoft.com/en-us/cpp/preprocessor/predefined-macros
