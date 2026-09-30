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
| 69 | `0x00466a80` | 65 | Aggregate-return/cache candidate; cache address now kept live across the return-buffer call; native rerun pending |
| 70 | `0x00467040` | 101 | Exact mode/state save and restore |
| 71 | `0x00466e50` | 171 code / 192 extent | Dispatcher candidate now uses the complete code+NOP+switch-table extent; native rerun pending |
| 72 | `0x00466fb0` | 62 | Exact cyclic advance without `/G6` |

These are function matches, not a completed class. Fields, enum names and the
12-byte aggregate's semantic type remain provisional.

## Remaining targets

Slot 69 calls virtual slot 57 with a hidden stack return buffer for a 12-byte
aggregate, copies three dwords into +0x2a8/+0x2ac/+0x2b0, then passes the cache
to slot 43. The retail body computes the +0x2a8 cache address before the virtual
return-buffer call, keeps it live in EDI, and reuses it as the slot-43 argument.
The candidate now expresses that source order directly. The ABI interpretation
is strong; an authentic VC6 rerun is still required before promoting the new
source shape to an exact match. Size alone does not establish a vector type.

Slot 71 stores the input at +0x244, calls slot 58, dispatches states 0–4 to
slots 66/65/64/63/60, then snapshots +0x220/+0x22c/+0x234 into
+0x2c4/+0x2c8/+0x2cc before slot 61. State 3 restores +0x258 from +0x2f0;
state 4 saves it. Retail loads all three snapshot sources before beginning the
stores, so the candidate now keeps those values in explicit locals.

The previous 171-byte calibration target stopped at the RET. Retail has one NOP
at 0x00466efb followed by a five-entry absolute switch table at
0x00466efc..0x00466f0f; the next routine begins at 0x00466f10. VC6's independent
procedure metadata reports a 192-byte candidate extent, so calibration now
compares that complete 192-byte compiler-owned extent instead of clipping it.
Strict relocation matching may auto-resolve a relocation only when its final
S+A remains inside this independently measured function extent. Other symbols
still require explicit reviewed bindings. The resulting slot-71 match remains
pending an authentic VC6 rerun.

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
