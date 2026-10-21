# Bike network and recorder connections

KrustyBike now calls the existing NetworkInterface, VCRInterface and
EventManager implementations at eight production sites. The included
KrustyBike near-miss aggregate has four additional Send/QueueRecord sites.
These are direct, typed call-boundary conversions; no forwarding implementation,
linker alias, alternate object or replacement storage was added.

| Retired call view | Existing provider | Retail VA / complete extent |
|---|---|---|
| `KbGameCfg::Send` | `NetworkInterface::Send` | `0x004ac830` / 145 bytes |
| `KbRecorder::QueueRecord` | `VCRInterface::QueueRecord` | `0x004e8720` / 240 bytes |
| `KbSession::FindRaceMode` | `EventManager::FindRaceMode` | `0x0045d2b0` / 53 bytes |

All three providers are complete strict VC6 SP3 matches. The isolated source
link compiled 219 translation units and removed these three missing names,
changing 370 unresolved names to 367 without new unresolved names. The game
still does not link; this does not establish network play or replay playback.

## Pointer provenance and argument meaning

`Game::CreateNetworkInterface` (`0x00468ae0`) constructs NetworkInterface and
stores its pointer at Game `+8`. The bike's `KbGame::network` view reads that
same pointer. Its `field_0x10` overlaps NetworkInterface's `isHost` field.
NetworkInterface's primary RTTI record has offset zero. Send's five arguments
retain their types, values and order, with `RET 20`; the final arguments are
the sender and destination identities, not generic flags.

BikeRace constructs KrustyVCR replay/ghost recorders and passes them to racers.
The complete setter at `0x00496e20` stores its pointer argument unchanged at
KrustyBike `+0x13fc`. Retail RTTI places VCRInterface and GameObject at offset
zero in KrustyVCR. QueueRecord's final argument is an unsigned byte count;
changing the declaration from signed int and mutable data to unsigned int and
const data retains the existing 32-bit argument representation and `RET 16`.
In particular, `Fn_004925A0` continues to send a twelve-byte packet and queue
only **one byte** when its recorder branch executes. This observed behavior is
preserved, not reinterpreted as a flag or repaired speculatively.

TrackGame constructs EventManager and stores its primary pointer at `+0x570`.
FindRaceMode reads the existing event-owner pointers in the order supercross,
quarry, baja/enduro, national, then tag, and returns the selected raw pointer.
The bike uses only the established owner fields at `+0xa8` and `+0xdc`.
The returned pointer is explicitly converted to the existing KbPlayer field
view. These boundaries do not identify all of KbPlayer, KbSession or KbGameCfg
with the canonical classes, nor do they establish equivalence of their other
provisional declarations.

## Preservation and native checks

Before/after authentic VC6 objects for KrustyBike, its sample aggregate, Net,
Recorder and EventManager preserve all **829 nondebug sections and 3,594
relocations** under the three explicit external-name substitutions. Raw section
bytes, flags and relocation kinds/addends are unchanged. Defined local targets
are compared by section identity and offset, rather than compiler-generated
label spelling. The normal `WIN32_LEAN_AND_MEAN` include boundary brings in
Recorder's Win32 synchronization types without unrelated OLE default libraries;
the original `.drectve` bytes are preserved too.

The regression checks 311 strict bodies, ten complete partial fingerprints,
all eight production and twelve aggregate direct-call destinations, primary
RTTI offsets, caller/provider field layouts, stack cleanup, and the recorder
setter and one-byte argument instructions. Five older gearbox fingerprints
were refreshed because the newly visible declarations renumbered local labels
and the external names changed. Their function extents, comparison scores,
complete raw section bytes and all relocation section/offset destinations were
proved unchanged; no partial implementation was promoted.

The native fixture extracts the exact three current function texts into
temporary translation units. Both these emitted bodies and the complete
production-TU bodies must independently match the full retail extents before
execution. The fixture also links the actual complete VCR ring source object,
whose registered bodies are checked strictly. Its **4,896 cases** cover:

- 1,728 Send cases: guaranteed/raw selection, sender defaulting, host/client
  relay, target payload, failure/success return values and unchanged storage.
- 3,072 QueueRecord cases: null/stopped/full rings, cursor wrap, zero/one-byte
  and larger payloads, exact capacity, growth and allocation failure, complete
  ring and backing-byte results, event signaling and released locks.
- 96 race-owner cases: every combination of five owner pointers, selection
  priority, returned pointer identity and unchanged owner/game storage.

Transport calls and debug allocation are scripted test dependencies. Locks and
events use real Win32 APIs. No worker runs during these cases, so concurrent
producer/consumer scheduling, real network delivery and full recorder lifecycle
behavior are outside this proof.

```powershell
$env:PYTHONPATH = '.;tests'
$env:VC6_ROOT = (Resolve-Path work/vc6sp3-disc).Path
$env:MCM2_EXE = (Resolve-Path work/game/mcm2.exe).Path
python -m unittest test_bike_network_connections test_vehicle_gearbox_connections -v
```
