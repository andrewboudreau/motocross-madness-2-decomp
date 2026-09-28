# Agent work queue

Mechanically ranked from the function manifest. Priority is a convenience heuristic, not evidence. Calibration targets are intentionally ranked highly because VC6 can resolve compiler-shape questions.

## Next targets

| Rank | VA | Size | Kind/status | Stability | Classes | Nearest source hint |
|---:|---|---:|---|---|---|---|
| 1 | `0x00478fe0` | 3 | return_zero | medium | UIDDLStatic, UIProgressBar, UIStatic | D:\aardvark\VC\krusty2\gameui.cpp (0x75) |
| 2 | `0x00470410` | 9 | sub_i32_fields | medium | UIButton, UIControl, UIDDLButton | D:\aardvark\VC\krusty2\gameui.cpp (0x124) |
| 3 | `0x00470420` | 9 | sub_i32_fields | medium | UIButton, UIControl, UIDDLButton | D:\aardvark\VC\krusty2\gameui.cpp (0x114) |
| 4 | `0x00405150` | 7 | BaseObject::~BaseObject destructor core | — | — | D:\aardvark\VC\krusty2\BackgroundImage.cpp (0x1000) |
| 5 | `0x00478280` | 20 | get_indexed_i32_stride32 | medium | UIMultiState, UIRadioButton | D:\aardvark\VC\krusty2\gameui.cpp (0xfc) |
| 6 | `0x004782a0` | 20 | get_indexed_i32_stride32 | medium | UIMultiState, UIRadioButton | D:\aardvark\VC\krusty2\gameui.cpp (0xdc) |
| 7 | `0x004782c0` | 20 | address_of_indexed_stride32 | medium | UIMultiState, UIRadioButton | D:\aardvark\VC\krusty2\gameui.cpp (0xbc) |
| 8 | `0x00478260` | 20 | get_indexed_i32_stride32 | medium | UIMultiState, UIRadioButton | D:\aardvark\VC\krusty2\gameui.cpp (0x11c) |
| 9 | `0x00405120` | 16 | BaseObject::BaseObject constructor | — | — | D:\aardvark\VC\krusty2\BackgroundImage.cpp (0xfd0) |
| 10 | `0x00405130` | 30 | BaseObject scalar deleting destructor | — | — | D:\aardvark\VC\krusty2\BackgroundImage.cpp (0xfe0) |
| 11 | `0x004aa190` | 3 | return_zero | medium | ShadowReceiver, SoultreePhysicsBaseObject, SoultreePhysicsCharacter | D:\aardvark\VC\krusty2\MSZoneInterface.cpp (0x1dc) |
| 12 | `0x004dc610` | 5 | return_u16_minus_one | medium | CollisionObject, ConstraintMethodCollisionModel, D3DIMSoultreeObject | D:\aardvark\VC\krusty2\Quadtree.cpp (0x119) |
| 13 | `0x00507920` | 5 | return_zero | medium | Bike, SoultreePhysicsBaseObject, SoultreePhysicsCharacter | D:\aardvark\VC\krusty2\Terrain.cpp (0x14d) |
| 14 | `0x004dc4c0` | 5 | return_zero | medium | ProjectedShadow, StatsOverlay, Terrain | D:\aardvark\VC\krusty2\Quadtree.cpp (0x269) |
| 15 | `0x00405170` | 32 | BaseObject::Release | — | — | D:\aardvark\VC\krusty2\BackgroundImage.cpp (0x1020) |
| 16 | `0x004da550` | 5 | return_zero | medium | DrawableGridNode, ShadowCamera | D:\aardvark\VC\krusty2\ProCircuitProcs.cpp (0x1f4) |
| 17 | `0x004da560` | 5 | return_zero | medium | ShadowCamera | D:\aardvark\VC\krusty2\ProjectedShadow.cpp (0x1e5) |
| 18 | `0x00510750` | 5 | return_zero | medium | ManagedTexture | D:\aardvark\VC\krusty2\ContainerList.h (0x2ae) |
| 19 | `0x00510980` | 14 | test_i32_nonzero | medium | ManagedTexture | D:\aardvark\VC\krusty2\ContainerList.h (0x7e) |
| 20 | `0x004806e0` | 5 | return_zero | medium | DrawableGridNode | D:\aardvark\VC\krusty2\Griddraw.cpp (0xb4d) |
| 21 | `0x004c7470` | 11 | test_i32_nonzero | medium | CacheTexture, PCTextureMap | D:\aardvark\VC\krusty2\PCTexMap.cpp (0x217) |

## Validated clang/MSVC-ABI plumbing samples

- `0x00401940` — 4 B — get_i32 — ArcadeObject, ArrowManager, AuralScape
- `0x00405160` — 8 B — clang-exact — BaseObject
- `0x0040c880` — 7 B — return_float_global — Bike, Vehicle
- `0x0040c890` — 7 B — get_i32 — Bike, KrustyBike, Vehicle
- `0x0040cac0` — 18 B — write_arg_i32_const_return_const — Bike, Vehicle
- `0x0040cae0` — 13 B — copy_i32_field — Bike, Vehicle
- `0x0040cba0` — 7 B — get_float — Bike, KrustyBike
- `0x00434ce0` — 8 B — return_constant — CollisionObject, ConstraintMethodCollisionModel, D3DIMSoultreeObject
- `0x0044d710` — 1 B — return_void — ConnectionInfoType, ConstraintMethodCollisionModel, DrawableGridNode
- `0x004627f0` — 8 B — return_constant — BackgroundImage, Fog
- `0x00464e80` — 3 B — return_void_pop — BaseQuarryEvent, BikeCamera, Character
- `0x00464e90` — 1 B — return_void — BaseQuarryEvent, Bike, BikeCamera
- `0x00467ae0` — 6 B — return_constant — BaseQuarryEvent, BikeCamera, Camera
- `0x00468c90` — 6 B — return_constant — Game, PCGame, ShadowReceiver
- `0x004703c0` — 7 B — get_i32 — UIButton, UIControl, UIDDLButton
- `0x004703d0` — 7 B — get_i32 — UIButton, UIControl, UIDDLButton
- `0x004703e0` — 7 B — get_i32 — UIButton, UIControl, UIDDLButton
- `0x004703f0` — 7 B — address_of_field — UIButton, UIControl, UIDDLButton
- `0x00470400` — 10 B — set_i32_arg — UIButton, UIControl, UIDDLButton
- `0x00470a70` — 13 B — set_i32_arg — UIButton, UIControl, UIDDLButton
- `0x00478520` — 20 B — set_i32_const_and_arg — UIMultiState, UIRadioButton
- `0x00479220` — 20 B — set_i32_const_and_arg — UIDDLListBox, UIDropDownList, UIListBox
- `0x0047b100` — 9 B — get_i32_ignore_args — UIProgressBar
- `0x004806f0` — 3 B — return_void_pop — DrawableGridNode, RenderTarget
- `0x004a6ba0` — 3 B — return_void_pop — Character, Vehicle
- `0x004cbdf0` — 17 B — set_i32_arg_if_nonzero — PhysicsBody, PhysicsRigidBody
- `0x004cc0a0` — 13 B — set_i32_arg — PhysicsBody, PhysicsRigidBody
- `0x004da540` — 8 B — return_constant — ArcadeObject, ArrowManager, AuralScape
- `0x004de580` — 3 B — return_void_pop — BaseQuarryEvent, NationalRace
- `0x004f9a50` — 10 B — set_i32_arg — SelectiveGravityModel
- `0x005289b0` — 7 B — get_float — Vehicle
- `0x00529270` — 11 B — set_i32_const — Vehicle
- `0x0052a520` — 10 B — get_nested_float — Vehicle
- `0x0052a5b0` — 9 B — return_float_global — Vehicle
- `0x0052cec0` — 10 B — get_nested_float — VehicleCamera
