# ProCircuit.cpp and ProCircuitProcs.cpp

These are the pro circuit career and its dialogs: `src/reconstructed/ProCircuit.*`
and `ProCircuitProcs.*`. Helper names follow their bodies (`FreeSchedule`,
`LoadSaved`, `Save`, `AdvanceAfterRace`, `PickComputerRacers`, `OpenPage`,
...); the rest are provisional.

**ProCircuit.cpp** (`0x004d3480..0x004d4b1f`). Evidence: the `__FILE__`
literal at `0x005718ac` (xrefs `0x004d34a1..0x004d45c2`); the unit closes
with its kVec3 `$E` set `0x004d49e0..0x004d4b1b` (vectors
`0x00689ab8..0x00689af7`, which no ProCircuit code reads). The object is the career at TrackGame+0x3444
(0x12e5 bytes; ProCircuitProcs allocates that size). It keeps the existing
name `UnknownTrackGameObject3444`, and TrackGame.h now includes
ProCircuit.h. Its fields sit at odd offsets, so it is byte-packed. The
first 0x1225 bytes are what the load and save functions read and write.
Before it:
- `0x004d28b0..0x004d2936`: Point2D's out-of-line methods
  (`src/reconstructed/Point2D.cpp`, TU name unattested).
- `0x004d2940..0x004d347f`: the engine gearbox/RPM unit
  (`src/reconstructed/GearRatios.cpp`, TU name unattested): strings "Gear
  Ratios invalid in %s", "Rpm Limits (%d,%d) invalid in %s" and "Unable to
  find optimum shift range for GEAR=%d", constructed by Vehicle.cpp
  (`0x005260df`). Its code `0x004d321a` reads `0x00689a88`, the zero vector
  of the `$E` set `0x004d3340..0x004d347b` that closes it.

Exact (17): the constructor (PCTables.pb, PCNames.txt, PCSched.pb), the
destructor, schedule free and read, new career, load, save, advance,
racer pick and the eight `$E`.

**ProCircuitProcs.cpp** (`0x004d4b20..0x004da51f`). Evidence: the
`__FILE__` literal at `0x005719e0` (xrefs up to `0x004da35c`); the unit
closes with its own `$E` set `0x004da3e0..0x004da51b`, whose vectors
`0x00689b08` and `0x00689b28` are read at `0x004d6aba` and `0x004d73df`. The slot-29 procedures belong to the
RTTI PC*Dlg vtables.

Exact (33):
- The eight `$E`.
- Three qsort comparators.
- The state-dialog opener `0x004d4ba0`.
- The slot 29 of PCStartupDlg, PCNewDlg, PCCentralDlg, PCCentralNextDlg,
  PCCentralStandingsDlg, PCLastRaceDlg, PCFailedDlg, PCFinishedDlg and
  PCCompleteDlg.
- Their helpers.
- PCCentralBikeRiderDlg's slots 13, 26 and 31, its plate painter and rider
  store. Slots 13 and 26 share their bodies with the SP/MP bike dialogs'
  vtables.

Source forms needed:
- Clamped copies use a `sizeof`-based macro.
- Retail ORs only the low byte of TrackGame+0x1ee8.
- Inline helpers keep the CircuitNNN.pc search loop unrotated.

Near misses (`samples/ui/ProCircuitProcsNearMisses.cpp`):
- `0x004d59a0`: exact in the sample TU, but its reload order flips with
  unrelated declarations, so it is not registered.
- The race payout `0x004d8c40`.
- The slot 29 of PCBailoutDlg, PCBunnyDlg and PCBonusTrackDlg: prologue
  scheduling differs.
- `0x004d6fc0`: one register differs.
- PCNewEventDlg slot 29 `0x004d9fd0`: byte-exact once its exception handler
  `0x0054d257` is bound. The resolver does not yet recognise that
  `mov edx,[esp+4]` EH prologue shape.

- PCCentralBikeRiderDlg slot 10 `0x004d72c0`, 1233 of 1291 bytes: only
  the scheduling of the by-value vector copies for the camera call differs.

Also exact: PCCentralBikeRiderDlg slot 29 `0x004d6770` and `0x004d7900`.
The `UnknownProCircuitSkinned` view is the same object as DlgProcs.h's
`UnknownModelTexture`; one should eventually replace the other.

**`$E` attribution.** A shared-header vector set is emitted after its
unit's functions (see CUBE.md). `.CRT$XCU` lists `0x004d3340`, `0x004d49e0`,
`0x004da3e0` and then `0x004dc4d0` in a row, and their vectors sit in
ascending `.bss` in that order. Each set's vectors are read only by the code
before it: `0x004d321a` (gearbox), none (ProCircuit), `0x004d6aba` and
`0x004d73df`/`0x004d7546` (ProCircuitProcs), and `0x004da591` (ProjectedShadow.cpp,
which owns the `0x004dc4d0` set; see [INITIALIZERS](INITIALIZERS.md)).
