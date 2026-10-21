# Bike text-message implementation connections

The bike's lap, finish and stunt notices now call existing message, overlay,
event-manager and resource-string implementations. Four obsolete external
names are removed at 18 production call sites. The isolated authentic VC6
source link compiles 220 translation units and changes **366 to 362 unresolved
names**, with no added unresolved names. This connects existing source; it
does not establish a working game or visible race notices.

| Old view | Existing definition | Retail VA / complete bytes |
|---|---|---|
| `KbMessage` constructor | `UnknownMessage(const char*, float)` | `0x0051b200` / 68 |
| `KbMsgSink::ReplaceHeadMessage` | `TextQueueOverlay::ReplaceHeadMessage(UnknownMessage*)` | `0x0051b540` / 146 |
| `KbSession::FindTextQueue` | `EventManager::FindTextQueue()` | `0x0045d340` / 78 |
| `KbGame::GetStringText` | `TrackGame::LoadResourceString(int, char*, int)` | `0x00521970` / 189 |

These complete provider bodies match strictly, with relocations resolved.
The complete production callers are KrustyBike slot 49 at `0x00496790`
(1,400 bytes, 12 sites) and `Fn_00495FF0` (895 bytes, six sites). The included
near-miss aggregate uses the same connections; its existing partial status
and full fingerprints are retained. No method implementation, thunk, wrapper
or alternate storage was added.

## Receiver and lifetime evidence

The shared `g_TrackGame` is already a TrackGame pointer. Its resource loader
reads the existing resource-instance member and calls LoadStringA. The former
view declared a void return; the canonical method returns int. These callers
ignore the result, retain the three argument values and order, and preserve
the full emitted caller bytes. EventManager is the object already constructed
and stored at TrackGame `+0x570`.

FindTextQueue selects the first present event owner in the existing priority
order and returns its `+0x6c` pointer, even when that pointer is null. The overlay
creation path calls TextQueueOverlay's constructor at `0x004e0af8`, stores the
primary result at owner `+0x6c`, calls Setup at `0x004e0b28`, and stores Setup's
returned pointer there again. TextQueueOverlay's RTTI places GameObject at
offset zero. The connection does not establish complete identity of all
provisional owner or sink declarations.

Each temporary message is independently heap allocated with the same `0x8c`
size and original source/line arguments. UnknownMessage has a 128-byte text
array, duration at `+0x80`, elapsed duration at `+0x84`, and next pointer at
`+0x88`; it has no virtual destructor. The old KbMessage declaration was opaque
byte storage. No equivalence of embedded-object alignment is asserted: these
particular allocations already supply suitably aligned storage.

ReplaceHeadMessage copies the supplied message and retains the old queue tail
before releasing the previous head. The caller then deletes its temporary.
The canonical copy constructor at `0x0051b250` (74 bytes, also strict) copies
the text and duration, resets elapsed time, and initially clears next. No
ownership transfer of the supplied temporary is introduced. Existing
allocation-failure behavior is preserved: an empty queue can remain null;
replacement of a nonempty queue faults when the failed allocation is written
at address `0x88`, leaving its old head unreleased.

## Complete preservation and bounded native execution

Private before/after authentic VC6 objects cover KrustyBike, its near-miss
aggregate and all three provider translation units: **878 nondebug sections
and 4,165 relocations**. Every raw section byte, flag and relocation
kind/addend/destination is unchanged under exactly the four external-name
substitutions. This includes the allocation and EH paths and source-file
diagnostic storage. No header extraction or provider change was required.

The focused regression checks 301 strict bodies, ten complete partial
fingerprints, all 18 production and 18 aggregate call destinations, primary
RTTI relationships, field layouts and the overlay creation/storage path.
Four older gearbox metadata digests were refreshed after comparing their old
values against the private baseline and proving all new relocation targets.
Only external spellings and compiler-local label names changed; extents,
comparison scores and complete section bytes did not change. The prior bike
network and gearbox native regressions also pass on the composed headers.

The native test extracts the exact current function text for the four providers
and the message copy constructor. Temporary-source location directives retain
the actual production `__FILE__` allocator argument. Each extracted body and
the corresponding full production-TU body must independently match the complete
retail extent before the test executes it. The test scripts only allocation
and the named LoadStringA import cell; no candidate instructions are patched.

Its **4,704 native cases** comprise:

- 1,024 message constructions with guarded full storage, text lengths through
  127 bytes, and duration bit patterns including negative zero and a quiet NaN.
- 2,048 queue replacements, including supplied-message aliasing with the head
  or tail, preserved tail links, full guarded storage, allocation failure and
  the observed access-violation kind/address.
- 96 event-owner selections, including an earlier owner with a null queue
  ahead of a later owner with a nonnull queue.
- 1,536 resource loads, including missing instance, failed lookup, truncation,
  capacity zero/one and unchanged surrounding storage and receiver state.

This is bounded single-threaded behavior evidence. It does not test real
resource bundles, rendering or oversized input to the existing unbounded
`strcpy` calls. Allocation failure is observed rather than repaired.

```powershell
$env:PYTHONPATH = '.;tests'
$env:VC6_ROOT = (Resolve-Path work/vc6sp3-disc).Path
$env:MCM2_EXE = (Resolve-Path work/game/mcm2.exe).Path
python -m unittest test_bike_text_connections test_bike_network_connections test_vehicle_gearbox_connections -v
```
