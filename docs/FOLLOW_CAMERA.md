# FollowCamera reconstruction

`FollowCamera : PCCamera` introduces slots 33–75; slots 33, 35, 39, 41, 42,
50, 51, 57 and 74 are `_purecall` in its vtable and are declared pure.
VehicleCamera, BikeCamera and KrustyBikeCamera inherit many entries.

The initializer at `0x00463140`, which sits between the constructor and the
destructor, cites `D:\aardvark\VC\krusty2\FollowCam.cpp` through `__FILE__`
when it allocates the +0x27c values. That is literal evidence for the file
name of this region.

Canonical source: `src/reconstructed/FollowCamera.h` and `.cpp`, built on
`PCCamera`, `Camera` and `GameObject` in the same directory.

| Slot | Retail VA | Bytes | VC6 SP3 status / behavior |
|---:|---|---:|---|
| 63 | `0x00466c90` | 21 | Exact; zeros +0x22c, +0x234, +0x220 |
| 64 | `0x00466cb0` | 31 | Exact preset stores |
| 65 | `0x00466cd0` | 31 | Exact preset stores |
| 66 | `0x00466cf0` | 41 | Exact preset stores |
| 67 | `0x00466d20` | 41 | Exact preset stores |
| 68 | `0x00466d50` | 241 | Exact without `/G6`, all relocations resolved; distance-derived +0x258 clamped to [10, 70] |
| 69 | `0x00466a80` | 65 | Exact without `/G6`; slot 57 result assigned straight into +0x2a8 |
| 70 | `0x00467040` | 101 | Exact mode/state save and restore |
| 71 | `0x00466e50` | 192 | Exact; 171 code bytes, one NOP, 5-entry jump table |
| 72 | `0x00466fb0` | 62 | Exact cyclic advance without `/G6` |
| 34 | `0x00464a40` | 63 | Exact; returns +0x2b4 with y + 3 (`Vector3` by hidden pointer) |
| 37 | `0x00464a10` | 37 | Exact; returns +0x2b4 (named local copy) |
| 52 | `0x00464e80` | 3 | Exact empty body (`ret 4`) |
| 53, 54, 58–62 | `0x00464e90` | 1 | Exact shared empty body |
| 55, 56 | `0x00464ea0`, `0x00464ec0` | 23 each | Exact; slot 5 of the interface at (`0x0056e26c` object)->+0x14->+0x34 with 0x38 / 0x2a |
| 75 | `0x00404fc0` | 25 | Exact; state +0x244 is 5 or 2 |

| 40 | `0x004639f0` | 60 | Exact; with a non-empty +0x2e4 table, entry 0 takes the target, then slot 38 |
| 43, 44 | `0x00464ee0`, `0x00464f70` | 144 each | Exact; feed a vector into +0x27c..+0x284 / +0x288..+0x290 values (inlined `Set`, rate 0.25 or 0.3 by subject+0xbe8) |
| 48 | `0x00464a80` | 175 | Exact; follow point by state (5: cache or slot 34, 7: raised target, else slot 35 or 37) |
| 73 | `0x00463620` | 192 | Exact; eases +0x220 toward a height-dependent target via the +0x298 value, minimum 20 |
| `0x00463450` | `0x00463450` | 201 | Exact; writes entry `index` of the +0x2e4 table of 44-byte records |
| Destructor core | `0x00463350` | 245 | Exact; null-checked `delete[]` of the +0x2e4 table and `delete` of the eight +0x27c..+0x298 values, then `~Camera` |
| Scalar deleting destructor | `0x00463120` | 30 | Exact canonical wrapper |

FollowCamera's members (+0x220 to +0x343) are declared in `FollowCamera.h`
from the constructor's stores. +0x2a8 is assigned from a `Vector3` global, so
the old 12-byte aggregate is typed `Vector3`. The slot 63–67 presets store
floats (for example 0x40490fdb = π, 0x42aa0000 = 85.0) written as float
literals.

Members whose roles repeat directly across matched bodies now use semantic
names: `cameraState` and its saved copy, the target/cache vectors, the owned
point table, and the cyclic state list. `VehicleCamera`, `BikeCamera`, and
`KrustyBikeCamera` likewise name their repeatedly dereferenced vehicle, bike,
and view pointers. These names describe observed use; they do not establish the
original source identifiers or the pointed-to runtime classes.

Two near misses are kept in `samples/camera/FollowCameraNearMisses.cpp`:

- Slot 36 (`0x00465000`, 209 bytes) differs by 12 bytes. Retail ends its
  disabled path with one zero register for the +0x277 store and the return,
  which no source form tried so far reproduces.
- The constructor (`0x00462ee0`, 564 bytes) has every value and offset right,
  and its virtual call to slot 71 binds statically to `0x00466e50`. About 141
  bytes differ: VC6 schedules the +0x274/+0x2dc zero stores into a load-delay
  slot where retail used +0x2e4/+0x276. Moving those statements does not fix
  it.

Slots 10, 23, 38, 45–47 and 49 and the initializer `0x00463140` are not yet
reconstructed.

Two shapes are evidence for helper functions in the original. The 20-byte
values at +0x27c..+0x298 have an inline `Set` (the rate is skipped when it is
FLT_MAX) and an inline `Update` that eases toward a target by
min(dt, rate) / rate. Slot 73 and `Update` also need a minimum written as an
inline function: a ternary or a `__min`-style macro reloads the chosen operand,
whereas retail keeps it on the FPU stack. Enum names remain provisional.

## Notes on slots 68, 69 and 71

Slot 68 copies the 12-byte float triple at +0x2b4, adds 3.0 to its second
component, takes the absolute differences between +0x170/+0x178 and the
triple's first/third components, and calls `0x00460b50` (a cdecl
`float(float)` that returns 0 for 0 and otherwise approximates a square root
by halving the exponent and looking up a table) on the sum of their squares.
+0x258 becomes `((200 - r) / 180) * 60 + 10`, clamped to [10, 70], and the
triple is passed by value to slot 29. `src/reconstructed/FollowCamera.bindings.json`
binds each VC6 float literal to a retail constant whose value was checked
against the literal, plus the direct call. Names, the triple's type and the
helper's identity remain provisional. Writing `/ 180.0f * 60.0f` without the
inner parentheses lets VC6 fold the two constants into one multiply, which
retail does not do.

Slot 69 calls virtual slot 57 with a hidden stack return buffer for a
`Vector3`, copies three dwords into +0x2a8/+0x2ac/+0x2b0, then passes the cache
to slot 43. Retail forms the cache address before the call and copies straight
from the returned buffer, so the candidate assigns the call result directly
(`cachedTarget = UnknownVirtualSlot57(0);`); a named temporary kept the copy in
extra registers.

Slot 71 stores the input at +0x244, calls slot 58, dispatches states 0–4 to
slots 66/65/64/63/60, then snapshots +0x220/+0x22c/+0x234 into
+0x2c4/+0x2c8/+0x2cc before slot 61. State 3 restores +0x258 from +0x2f0;
state 4 saves it. The code already matched; the function's extent includes one
alignment NOP and the 5-entry jump table. Retail has the same layout: all five
entries (`0x00466e77`–`0x00466eb3`) and the table reference resolve to the
retail addresses when each label is placed at its function offset.

## Field behavior

| Offset | Observed role |
|---|---|
| Camera +0x170 (x, z) | Compared with the +0x2b4 triple in slot 68 |
| +0x220, +0x22c, +0x234 | Preset parameters |
| +0x244, +0x248 | Current/saved state candidates |
| +0x24c | Saved copy of +0x258 |
| +0x258 | Float-like parameter, bounded around 10–70 in slot 68 |
| +0x268 | Low input byte stored as a dword |
| +0x26c | Reset on enabling temporary state |
| +0x2a8 | Cached slot 57 `Vector3` |
| +0x2b4 | 12-byte float triple copied by slot 68 and passed to slot 29 |
| +0x2c4, +0x2c8, +0x2cc | Snapshot of the three preset parameters |
| +0x2f0 | State-specific saved copy of +0x258 |
| +0x30c, +0x310, +0x314 | Cyclic index, count, inline dword table |

Slot 70 saves the previous state/parameter before entering literal state 5 and
restores them on disable. Slot 72 increments/wraps the index, selects a table
value, stores it at +0x244 and calls slot 71. Reproduce with the
[calibration/profile commands](VC6_MATCHING.md).

## Camera, PCCamera and ShadowCamera

RTTI: `PCCamera : Camera : GameObject : BaseObject`; `ShadowCamera : PCCamera`.
Canonical source: `src/reconstructed/Camera.{h,cpp}`, `PCCamera.{h,cpp}` and
`ShadowCamera.{h,cpp}`; call bindings in
`src/reconstructed/Camera.bindings.json` come from the GameObject/Camera vtables
and decoded direct calls. All twenty-six bodies are strict exact under the
default profile with zero ignored bytes. Camera's members (+0x2c..+0x21f) are
declared in `Camera.h`; the class ends at +0x220, where FollowCamera's fields
begin.

| Class/slot | Retail VA | Bytes | Behavior |
|---|---|---:|---|
| Camera constructor | `0x0042e340` | 402 | `/GX` frame; GameObject constructor; scalar defaults; `IdentityMatrix()` ([matrix helpers](MATRIX_UTIL.md)) into +0x2c; vectors (0,0,1), (0,1,0), (0,0,0) at +0x17c/+0x188/+0x170; +0x214/+0x208 copied from +0x17c/+0x170 |
| Camera 0x42e8e0 | `0x0042e8e0` | 75 | If +0x1c0/+0x1bc differ from the `.rdata` limits: `0x0042e960(limits)` then slot 28; returns 1 |
| Camera 0x42e960 | `0x0042e960` | 79 | +0x1bc = max(argument, 1.0 limit), +0x1c0 = min(argument, 100000.0 limit) |
| Camera 4 | `0x00499ad0` | 5 | Tail jump to GameObject slot 4 (address shared by folding) |
| Camera 8 | `0x0042e500` | 65 | GameObject slot 8; if `0x0042e550` fails, `Release` and return 0; else slot 28, owner notify on +0x25 bit 0, return `this` |
| Camera 14 | `0x00467ae0` | 6 | Returns 1 (shared body) |
| Camera 23 | `0x0042f090` | 71 | If GameObject slot 23 finds nothing: cdecl `0x0043caa0(0xb7, 0, a, 0x80000000)`, then slot 27 and return 1 |
| ShadowCamera constructor | `0x004da520` | 25 | `PCCamera(flags)`, then the ShadowCamera vptr |
| ShadowCamera 19, 22 | `0x004da550`, `0x004da560` | 5 each | Return 0 |
| PCCamera 27 | `0x004beed0` | 19 | Owner helper `0x004c5d00`, then `0x00468880` on the object at `0x0056e26c`; Camera's slot 27 is `_purecall` |
| Camera destructor core | `0x0042f020` | 37 | Camera vptr; `fclose(+0x1e4)` when set; GameObject destructor `0x00468d60` |
| Camera scalar deleting destructor | `0x0042e4e0` | 30 | Canonical wrapper (slot 0) |
| PCCamera constructor | `0x004bed80` | 25 | `Camera(flags)` (`0x0042e340`), then the PCCamera vptr |
| PCCamera destructor | `0x004624d0` | 5 | Compiler-generated: tail jump to `~Camera`, no PCCamera vptr store |
| PCCamera scalar deleting destructor | `0x004beda0` | 30 | Shared with ShadowCamera slot 0 |
| Camera 5 | `0x0042f050` | 32 | GameObject slot 5, owner helper `0x004e8cf0(this)`, +0x1d0 = owner+0x14 + 1 |
| Camera 13 | `0x0042e630` | 92 | If +0x1cc: rectangle from x/y/width/height at +0x1a0 to owner slot 12; then `0x0042e8e0`; returns 1 |
| Camera 18 | `0x0042f070` | 28 | GameObject slot 18, +0x1d0 = owner+0x14 + 1, returns 1 |
| Camera 30/31/32 | `0x0042edd0`/`0x0042edf0`/`0x0042ee10` | 26/29/26 | Copy a 64-byte block into +0x2c/+0xac/+0x6c, return 1 |
| PCCamera 13 | `0x004bee80` | 65 | Camera 13; if owner+0x08 is this camera, interface method 11 with kinds 2 (+0xac) and 3 (+0x6c) |
| PCCamera 30/31/32 | `0x004bedc0`/`0x004bee00`/`0x004bee40` | 54 each | Camera version, then optional interface method 11 with kind 1/2/3 |

The owner is the object at Camera+0x18, a `RenderTarget`; for PCCamera it is a
`PCRenderTarget` whose +0x50 holds a COM-style device (`this` on the stack; see
[RenderTarget](RENDER_TARGET.md)). Method 11 taking kind 1/2/3 and a 64-byte
block is consistent with `IDirect3DDevice7::SetTransform` for
world/view/projection. The members at +0x2c/+0xac/+0x6c are therefore named
`worldMatrix`/`viewMatrix`/`projectionMatrix`. These semantic names remain
provisional inference from the call shape; identifying the interface from
creation/import evidence would be needed to confirm them.

Destructor evidence. `analysis/deleting_destructors.json` pairs each wrapper
with its destructor core. Camera's vptr is written in its constructor
(`0x0042e36e`) and destructor (`0x0042f029`); PCCamera's only in its
constructor (`0x004bed8d`). That, the tail-jump body and the wrapper shared
with ShadowCamera fit a compiler-generated PCCamera destructor, so
`PCCamera.h` declares none. `0x00534c3d` is LIBCMT's multithreaded `fclose`
(`tools/build_vc6_crt_atlas.py`: `mt_obj\fclose.obj`, 37/37 compared bytes),
so Camera+0x1e4 is a `FILE*`. GameObject's constructor (`0x00468ca0`) cites
`D:\aardvark\VC\krusty2\gameobj.cpp` via `__FILE__` and uses only the low bit
of its argument; the `int flags` parameter type is provisional.

Constructor evidence. The two `.rdata` floats loaded into +0x1bc/+0x1c0
(`0x00550f6c` = 1.0, `0x00550f70` = 100000.0) sit just before Camera's
vtable and are loaded from memory, not as immediates, so they are declared as
external `const float` objects. The matrix type needs an empty user-declared
default constructor (as D3DMATRIX has under `D3D_OVERLOADS`) for VC6 to copy
the returned identity straight into +0x2c. Retail builds only three vector
temporaries and reuses the registers holding +0x17c's and +0x170's
components for +0x214 and +0x208, which is what member copies compile to;
constructing fresh vectors there adds stack temporaries.


## VehicleCamera and BikeCamera

RTTI: `BikeCamera : VehicleCamera : FollowCamera`. Canonical source:
`src/reconstructed/VehicleCamera.{h,cpp}` and `BikeCamera.{h,cpp}` with their
bindings files. Neither translation unit is established. VehicleCamera.cpp
overlaps with Vehicle.cpp by name only; BikeCamera's code sits near BikeAI.cpp
references. All bodies below are strict exact under the default profile.

| Function | Retail VA | Bytes | Behavior |
|---|---|---:|---|
| VehicleCamera constructor | `0x0052b920` | 137 | `FollowCamera(flags)`; +0x390 = true; +0x398/+0x3a4 from the `.bss` vector `0x0068a728` |
| VehicleCamera destructor / wrapper | `0x0052b9d0` / `0x0052b9b0` | 11 / 30 | Explicit empty destructor (vptr store, jump to `~FollowCamera`) |
| VehicleCamera 33 | `0x0052ba30` | 142 | Tracked point: vehicle +0x64 in vehicle mode, else target +0x384 (+0x224) or +0x388 (+0x40), else the global default |
| VehicleCamera 42 | `0x0052cc00` | 122 | +0x308 = fov/zoom ratio × vehicle +0x43c × 0.3 (0.4 in state 3), 0 otherwise |
| VehicleCamera 39 | `0x0052cbd0` | 41 | Returns vehicle +0x45c; +0x23c += π when vehicle +0x464 is set |
| VehicleCamera 50 | `0x0052cb80` | 73 | Vehicle part position 0, raised by 5 |
| VehicleCamera 51 | `0x0052cec0` | 10 | Vehicle +0x48 |
| VehicleCamera 67 | `0x0052bfa0` | 76 | Preset: distance 17 (`.rdata` `0x00558d60`) in vehicle mode, else 60 |
| VehicleCamera 72 | `0x0052bff0` | 62 | Hold while vehicle +0x444; else FollowCamera slot 72 or reset to state 0 |
| VehicleCamera 74 | `0x00417490` | 31 | Inline in the header; vehicle mode and vehicle +0x444 |
| VehicleCamera 75 | `0x0052d010` | 58 | FollowCamera slot 75, else not vehicle mode or vehicle +0x444 clear |
| BikeCamera constructor | `0x00416e20` | 35 | `VehicleCamera(flags)`; +0x3b0 = 0 |
| BikeCamera destructor / wrapper | `0x00416e70` / `0x00416e50` | 11 / 30 | Explicit empty destructor |
| BikeCamera 40 | `0x00416ed0` | 211 | Table entries 0 (rider head) and 1 (target) with values 2.5/4 or 5/5, then slot 38 |
| BikeCamera 37 | `0x00416fb0` | 153 | 38% of the way from the rider's `"Head"` part to the target point |
| BikeCamera 50 | `0x00417290` | 77 | The rider's `"Head"` position |
| BikeCamera 51 | `0x00417340` | 13 | Bike +0x58 × +0x48 |
| BikeCamera 53, 54 | `0x004172e0`, `0x00417310` | 45 each | `0x004444e0` / `0x004fdb50` on the bike part and the rider |
| BikeCamera 75 | `0x004174b0` | 72 | FollowCamera slot 75, mode and bike state tests |

Several shapes record source structure:

- **FollowCamera slot 75 is inline.** Its body is inlined into both
  overrides, and its out-of-line copy sits far away at `0x00404fc0`.
  VehicleCamera slot 74 is also inline, emitted next to BikeCamera code.
- **VehicleCamera slot 75's helper.** It needs a two-return inline helper:
  only that makes VC6 materialise the `sete` result before testing it.
- **VehicleCamera slot 74's local.** It needs a named `bool` local for
  `setne al` to land straight in the return register.
- **BikeCamera's "Head" lookup.** The lookup is its own statement. Nesting it
  inside the position call makes VC6 push the outer call's arguments first.

## KrustyBikeCamera

RTTI: `KrustyBikeCamera : BikeCamera`. Canonical source:
`src/reconstructed/KrustyBikeCamera.{h,cpp}`. KrustyBike.cpp is a candidate
TU (name overlap, nearby references). The camera keeps its state and presets
in the object behind the global pointer `0x0056e26c` (declared in
`TrackGame.h`), so they survive between cameras. All bodies below
are strict exact.

| Function | Retail VA | Bytes | Behavior |
|---|---|---:|---|
| Constructor | `0x00497cb0` | 170 | `/GX` frame; slot 62 (restore state) and slot 71 bound statically; resets and copies |
| Destructor / wrapper | `0x00497d80` / `0x00497d60` | 11 / 30 | Explicit empty destructor |
| 23 | `0x00497df0` | 38 | Returns 0 while global +0x3430 is set, else FollowCamera slot 23 |
| 42 | `0x00498130` | 148 | +0x308 = fov/zoom ratio × bike +0x43c × 0.42 (0.55 in state 3), 0 otherwise |
| 48 | `0x004985b0` | 249 | FollowCamera slot 48 plus the raw target while view +0x3f8/+0x3f9 are set |
| 52 | `0x00497fa0` | 217 | Keeps a point 3.5 above the subject's ground probe (easing +0x22c), capped at 400 in global modes 3 and 4 |
| 55 | `0x00498080` | 18 | Global +0x14 virtual slot 2 with (0x0b, 0x3f) |
| 56 | `0x004980a0` | 134 | Input 0x0a test; outside state 7 also bike +0x108, axis < -2 and not +0x735 in vehicle mode |
| 58 | `0x004982c0` | 116 | Unless state 6, shows string 0x13b9 + state for 1.5 s through the global +0x570 object |
| 59, 60 | `0x004981d0`, `0x00498230` | 96 / 69 | Save / restore presets in global +0x2934..+0x2940 |
| 61, 62 | `0x004982a0`, `0x00498280` | 19 / 18 | Save / restore the state in global +0x2930 |

Slot 58's frame holds a 0x8c-byte message object and a `char[260]` text
buffer (MAX_PATH), although it asks for at most 0x80 characters.

Slot 10 (`0x00497e20`, 352 bytes plus a 6-entry jump table) is a near miss in
`samples/camera/KrustyBikeCameraNearMisses.cpp`, with 35 of 376 bytes
differing. It picks its view from the global's +0x558..+0x568 objects by
+0x2d74 (case order 2, 3, 0, 1/5, 4 in the code). While the view is available
it drives FollowCamera slots 46 and 47 from it, using file-scope statics at
`0x0067c3e8` and `0x0067c3f4` (no initialisation guard). Only the register
choice in its second `0x004210f0` call differs.

## FollowCam.cpp extent and the second wave

FollowCam.cpp runs from `0x00462ee0` to `0x004670fb`, and every FollowCam.cpp
`__FILE__` xref falls inside it. The code before it is FastMath, the text
stream and Fog/FogOff/FogOn, with its own vector set (`.CRT$XCU` 109-112).
FontTexture.cpp's `$E` follows at `0x00467100`.

The unit's own kVec3 set (`.CRT$XCU` 113-116, `0x00466f10..0x004670fb`) is
interleaved with slots 72 and 70. Its vectors `0x0065b438..0x0065b468` are
read only by FollowCam code, so they are now file statics.

This wave adds 13 calibration cases: the point-table append `0x00463520`,
set subject `0x00463600`, slot 38 (point-table steering, with its jump
table), slot 49 (camera position), slot 23 (controls) and the eight `$E`.

Slot 40 takes a float, because slot 38 loads its argument as one. So the
FollowCamera and BikeCamera slot 40 cases now use the `@@UAEXM@Z` names.
The 0x344 matrix belongs to FollowCamera, not VehicleCamera.

Near misses (`samples/camera/FollowCameraNearMisses.cpp`):
- The constructor and slot 36, both already known.
- Init `0x00463140`: an `offset + points` operand order.
- Slot 46 `0x004654e0`: two late `fsubp`.
- Slot 47 `0x00465720`.
- The CAMERA-file loader `0x004650e0`: retail keeps cross products in memory.
- Slot 45 `0x00463a30`: joystick pointer reloads.

Slot 10 `0x00465c20` (about 3.6 KB) is not attempted.
