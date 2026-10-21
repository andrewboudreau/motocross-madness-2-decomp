# KrustyBike scalar state

`src/reconstructed/KrustyBikeState.cpp` supplies twelve writable scalars already
used by canonical `vehicle/KrustyBike.cpp`. This data-only grouping does not
claim original translation-unit ownership. The existing declarations in
`KrustyBikeTypes.h` are unchanged. The provider needs only built-in scalar types;
including that class header would introduce unrelated per-TU vector constants
and their dynamic initializers.

## Storage and runtime writes

Values below are confirmed from the executable pinned by SHA-256
`31fde4cc686a5ee89ef9095b90235325b195596867ecacefe511263e1509b874`.
All reside in retail `.data` with characteristics `0xc0000040` (read/write).
Addresses through `0x56cb60` have file-backed four-byte values. The two addresses
at `0x67c3a0` and `0x67c3ac` are beyond the section's raw bytes, within its virtual
size, and therefore loader-zeroed. Names remain inferred from decoded usage and
configuration literals; scalar widths and write destinations are confirmed.

| Symbol suffix (`g_kb`) | VA | Initial value | Decoded runtime write |
| --- | --- | --- | --- |
| LatencyHiding | `0x56cb3c` | int 1 | `0x490a55` |
| RateLimiting | `0x56cb40` | int 1 | `0x490a6b` |
| AllowWarping | `0x56cb44` | int 1 | `0x490a81` |
| UseExtrapLimit | `0x56cb48` | int 1 | `0x490aad` |
| ExtrapLimit | `0x56cb4c` | float 0.5 | `0x490adb` |
| LatencyThreshold | `0x56cb50` | float 0.5 | `0x490b04` |
| WarpThreshold | `0x56cb54` | float 0.2 (`cd cc 4c 3e`) | `0x490b29` |
| UseTimeReceived | `0x56cb58` | int 1 | `0x490b3a` |
| AllowNegative | `0x56cb5c` | int 1 | `0x490b50` |
| Interpolate | `0x56cb60` | int 1 | `0x490b60` |
| UseLatencyThreshold | `0x67c3a0` | int 0, zero-fill | `0x490a97` |
| PadActive | `0x67c3ac` | int 0, zero-fill | `0x4964a0`, `0x4964cc` |

The first eleven receive configuration values in the existing complete
`KrustyBike::Load` at `0x48fc80` (5,384-byte extent including its jump table).
Its three float writes use `fstp dword ptr`, following the configuration result
conversion to seconds. The file's ExtrapLimit is **0.5f**, whereas the loader's
100 ms fallback becomes **0.1f** at that later phase. The other defaults are
500 ms and 200 ms. The provider preserves the initial bytes; it neither runs nor
replaces those existing runtime configuration reads. Slot 94 at `0x4963f0`
(236 bytes) sets the pad-active latch to one and clears it to zero.

The unrelated `g_kbZeroVec` needs a separate connection to already emitted
vector storage. StallThreshold and StallHoldSec are declared nearby but are not
missing imports of the current source build. None is added here.

## Verification

```powershell
$env:VC6_ROOT = (Resolve-Path work/vc6sp3-disc).Path
$env:MCM2_EXE = (Resolve-Path work/game/mcm2.exe).Path
python -m unittest discover -s tests -p test_krusty_bike_state.py -v
```

Authentic VC6 SP3 produces exactly twelve public definitions: 40 initialized
bytes and 8 BSS bytes, with writable/readable section attributes and the expected
four-byte extent for each scalar. There are no instructions, CRT initializer
entries, external imports or runtime/data relocations (ordinary debug symbol
relocations remain). The test compares all stored bytes and zero-fill provenance,
checks each independently decoded write destination, verifies the actual caller
COFF imports consume every provider, and retains all **76 complete strict
KrustyBike comparisons**. These include the loader and pad-latch methods.

The isolated real build compiled 196/196 translation units and reduced unresolved
names from 554 to 542: exactly these twelve were removed and none added. This
closes storage dependencies; the source-built game still does not link.
