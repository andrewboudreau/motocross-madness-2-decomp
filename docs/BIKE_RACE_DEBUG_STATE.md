# Race debug state

`src/reconstructed/BikeRaceDebugState.cpp` supplies twelve existing writable
objects consumed by `BikeRace`. It uses the existing `Vector3` declaration from
`MatrixUtil.h`. Original translation-unit ownership is not established; the
new reconstructed filename groups the evidenced storage.

| Existing object | Retail VA | Size | Initial state |
| --- | --- | --- | --- |
| `g_ObstacleDebugLeftLine` | `0x5779d8` | 24 | zero, file-backed |
| `g_ObstacleDebugUpperHit` | `0x577a00` | 4 | zero, file-backed |
| `g_ObstacleDebugRightLine` | `0x577a18` | 24 | zero, file-backed |
| `g_ObstacleDebugLine` | `0x577a30` | 24 | zero, file-backed |
| `g_ObstacleDebugLowerLine` | `0x577a48` | 24 | zero, file-backed |
| `g_ObstacleDebugHit` | `0x578da8` | 4 | loader zero-fill |
| `g_ObstacleDebugLowerHit` | `0x578dac` | 4 | loader zero-fill |
| `g_PathTargetDebugLine` | `0x578dc0` | 24 | loader zero-fill |
| `g_ObstacleDebugRightHit` | `0x578dd8` | 4 | loader zero-fill |
| `g_ObstacleDebugUpperLine` | `0x578de0` | 24 | loader zero-fill |
| `g_ObstacleDebugLeftHit` | `0x578df8` | 4 | loader zero-fill |
| `g_LodDebugSelectedRow` | `0x567a88` | 4 | integer `2`, file-backed |

All retail locations are in writable `.data`; its raw-backed range ends at
`0x578000`, while the virtual range extends through `0x68c4a0`. The regression
checks actual image bytes where present and PE loader zero-fill where absent.
The six line objects are **arrays of two vectors**, each containing six float
words. Their existing declarations in `BikeRace.h` already say `Vector3[2]`.
VC6's external array spelling includes `PAUVector3`; a linker diagnostic may
render that as a pointer. A four-byte pointer definition would not supply the
required storage.

Retail producers at `0x415640/6112` and `0x40d200/4464` write components at
byte offsets `0,4,8,12,16,20` of each line. The first producer clears all five
flags after `xor eax,eax` at `0x415c09`. BikeRace's debug drawing method at
`0x423410/885` reads the flags and passes each line's base and base-plus-12
addresses to the drawing operation. `Tick` at `0x41d2b0/5980` and
`OnControlPress` at `0x41f5e0/2644` read and update the selected row.
These instructions establish the storage boundary independently of the
unadmitted AI candidates in `BikeAINearMisses.cpp`. Those candidates retain
their provisional `Vec3[2]` views and are not promoted by this change.
The already supplied gate debug arrays in `RaceStatus.cpp` are separate.

The real `Vector3` type has an empty default constructor. Native VC6 emits six
`.CRT$XCU` entries for these definitions, each pointing to a five-byte jump to
an empty one-byte initializer. The test checks these complete routes and
links the unchanged provider into a normal CRT executable, which observes
zero arrays/flags and selected row `2` at entry to `main`. Compiler-local
initializer names and the emitted unused array-constructor iterator are not
assigned original retail identities. Scalar objects may be followed by
alignment padding outside their four-byte extents; no original inter-object
layout is claimed for the new object file.

`tests/test_bike_race_debug_state.py` verifies all twelve native exports,
writable storage, six exact 24-byte array allocations, disjoint native and
retail ranges, direct consumer relocation addends `0/12`, and decoded retail
producer/consumer accesses. The fixture includes the actual `BikeRace.h`
declarations, checks `Vector3` size/component offsets and both element
addresses, then changes each of 42 words through eight bit patterns while
checking every other word: **336 mutations**, including signed-zero and NaN
payloads without floating-point conversion. This tests actual initialized
storage and independent writes, not AI simulation or rendering.

The focused run also retains **80 strict caller bodies and 10 complete partial
fingerprints** across `BikeRace.cpp` and its near-miss aggregate. Existing
production files and headers are unchanged by this provider addition.

Reproduce in PowerShell with the private compiler and image available:

```powershell
$env:PYTHONPATH='.;tests'
$env:VC6_ROOT=(Resolve-Path work/vc6sp3-disc).Path
$env:MCM2_EXE=(Resolve-Path work/game/mcm2.exe).Path
python -m unittest test_bike_race_debug_state -v
```

The isolated real link `work/game-build/build-meo57h1y` compiles 228 translation
units and reduces unresolved symbols from **335 to 323**, removing exactly the
twelve data names with no new missing names. The source-built game still does
not link or run.
