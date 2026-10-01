# FollowCamera reconstruction

`FollowCamera : PCCamera` introduces slots 33–75; slots 33, 35, 39, 41, 42,
50, 51, 57 and 74 are `_purecall` in its vtable and are declared pure.
VehicleCamera, BikeCamera and KrustyBikeCamera inherit many entries. FollowCam.cpp is a source-file candidate
supported by name overlap and nearby references, not a proven TU assignment.
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

FollowCamera's members (+0x220 onward) are declared in `FollowCamera.h`; the
slot 63–67 presets store floats (for example 0x40490fdb = π, 0x42aa0000 =
85.0) written as float literals. The remaining FollowCamera functions
(destructor `0x00463120`, slots 10, 23, 36, 38, 40, 43–49 and 73) are not yet
reconstructed. Enum names and the 12-byte aggregate's type remain
provisional.

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

Slot 69 calls virtual slot 57 with a hidden stack return buffer for a 12-byte
aggregate, copies three dwords into +0x2a8/+0x2ac/+0x2b0, then passes the cache
to slot 43. Retail forms the cache address before the call and copies straight
from the returned buffer, so the candidate assigns the call result directly
(`field_0x2a8 = UnknownVirtualSlot57(0);`); a named temporary kept the copy in
extra registers. Size alone does not establish a vector type.

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
| +0x2a8 | 12-byte cached aggregate |
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

The owner is the object at Camera+0x18; its +0x50 holds a COM-style interface
(`this` on the stack). Method 11 taking kind 1/2/3 and a 64-byte block is
consistent with `IDirect3DDevice7::SetTransform` for world/view/projection,
which would make +0x2c/+0xac/+0x6c the world/view/projection matrices. That is
inference from call shape only; names stay neutral until the interface is
identified from creation/import evidence.

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

