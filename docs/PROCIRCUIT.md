# ProCircuit.cpp and ProCircuitProcs.cpp

These are the pro circuit career and its dialogs: `src/reconstructed/ProCircuit.*`
and `ProCircuitProcs.*`. Names are provisional.

**ProCircuit.cpp** (`0x004d3340..0x004d49dd`). Evidence: the `__FILE__`
literal at `0x005718ac` (xrefs `0x004d34a1..0x004d45c2`); the unit opens
with its kVec3 `$E` set. The object is the career at TrackGame+0x3444
(0x12e5 bytes; ProCircuitProcs allocates that size). It keeps the existing
name `UnknownTrackGameObject3444`, and TrackGame.h now includes
ProCircuit.h. Its fields sit at odd offsets, so it is byte-packed. The
first 0x1225 bytes are what the load and save functions read and write.
The gap `0x004d1d20..0x004d3340` after Pixtrans.cpp is not attributed.

Exact (17): the constructor (PCTables.pb, PCNames.txt, PCSched.pb), the
destructor, schedule free and read, new career, load, save, advance,
racer pick and the eight `$E`.

**ProCircuitProcs.cpp** (`0x004d49e0..0x004da3db`). Evidence: the
`__FILE__` literal at `0x005719e0` (xrefs up to `0x004da35c`); the unit
opens with its own `$E` set. The next `$E` set, at `0x004da3e0`, opens the
ShadowCamera/ProjectedShadow unit. The slot-29 procedures belong to the
RTTI PC*Dlg vtables.

Exact (31):
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

Not attempted: PCCentralBikeRiderDlg slots 29 and 10 and `0x004d7900`.
The `UnknownProCircuitSkinned` view is the same object as DlgProcs.h's
`UnknownModelTexture`; one should eventually replace the other.

**Open attribution question.** The RaceStatus/Pixtrans pass suggests that
the kVec3 `$E` sets are attributed one unit off:
- **`0x004d3340`:** belongs to an unnamed gearbox/RPM unit at
  `0x004d2940..0x004d333f` ("Unable to find optimum shift range for
  GEAR=%d"). That unit's code reads its zero vector `0x00689a88`.
- **`0x004d49e0`:** belongs to ProCircuit.cpp.
- **`0x004da3e0`:** belongs to ProCircuitProcs.cpp. `0x004d6abb` reads
  `0x00689b08`.

If that holds, ProCircuit.cpp starts after `0x004d347f`, and the 16
ProCircuit/ProCircuitProcs `$E` cases need re-pointing. Point2D's inline
methods sit at `0x004d28b0..0x004d2936`. This is not yet acted on.
