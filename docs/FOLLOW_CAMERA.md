# FollowCamera reconstruction

`FollowCamera : PCCamera` introduces slots 63–72. VehicleCamera, BikeCamera and
KrustyBikeCamera inherit many entries. FollowCam.cpp is a source-file candidate
supported by name overlap and nearby references, not a proven TU assignment.
Canonical candidate: `samples/camera/FollowCameraProbe.cpp`.

| Slot | Retail VA | Bytes | VC6 SP3 status / behavior |
|---:|---|---:|---|
| 63 | `0x00466c90` | 21 | Exact; zeros +0x22c, +0x234, +0x220 |
| 64 | `0x00466cb0` | 31 | Exact preset stores |
| 65 | `0x00466cd0` | 31 | Exact preset stores |
| 66 | `0x00466cf0` | 41 | Exact preset stores |
| 67 | `0x00466d20` | 41 | Exact preset stores |
| 68 | `0x00466d50` | — | Bounded float-like update; no matching candidate |
| 69 | `0x00466a80` | 65 | Exact without `/G6`; slot 57 result assigned straight into +0x2a8 |
| 70 | `0x00467040` | 101 | Exact mode/state save and restore |
| 71 | `0x00466e50` | 192 | Exact; 171 code bytes, one NOP, 5-entry jump table |
| 72 | `0x00466fb0` | 62 | Exact cyclic advance without `/G6` |

These are function matches, not a completed class. Fields, enum names and the
12-byte aggregate's semantic type remain provisional.

## Notes on slots 69 and 71

Slot 69 calls virtual slot 57 with a hidden stack return buffer for a 12-byte
aggregate, copies three dwords into +0x2a8/+0x2ac/+0x2b0, then passes the cache
to slot 43. Retail forms the cache address before the call and copies straight
from the returned buffer, so the candidate assigns the call result directly
(`*cached = UnknownVirtualSlot57(0);`); a named temporary kept the copy in extra
registers. Size alone does not establish a vector type.

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
| +0x220, +0x22c, +0x234 | Preset parameters |
| +0x244, +0x248 | Current/saved state candidates |
| +0x24c | Saved copy of +0x258 |
| +0x258 | Float-like parameter, bounded around 10–70 in slot 68 |
| +0x268 | Low input byte stored as a dword |
| +0x26c | Reset on enabling temporary state |
| +0x2a8 | 12-byte cached aggregate |
| +0x2c4, +0x2c8, +0x2cc | Snapshot of the three preset parameters |
| +0x2f0 | State-specific saved copy of +0x258 |
| +0x30c, +0x310, +0x314 | Cyclic index, count, inline dword table |

Slot 70 saves the previous state/parameter before entering literal state 5 and
restores them on disable. Slot 72 increments/wraps the index, selects a table
value, stores it at +0x244 and calls slot 71. Reproduce with the
[calibration/profile commands](VC6_MATCHING.md).
