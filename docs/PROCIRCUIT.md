# ProCircuit.cpp and ProCircuitProcs.cpp

These are the pro circuit career and its dialogs: `src/reconstructed/ProCircuit.*`
and `ProCircuitProcs.*`. Helper names follow their bodies (`FreeSchedule`,
`LoadSaved`, `Save`, `AdvanceAfterRace`, `PickComputerRacers`, `OpenPage`,
...); the rest are provisional.

**ProCircuit.cpp** (`0x004d3480..0x004d4b1f`). Evidence: the `__FILE__`
literal at `0x005718ac` (xrefs `0x004d34a1..0x004d45c2`); the unit closes
with its kVec3 `$E` set `0x004d49e0..0x004d4b1b` (vectors
`0x00689ab8..0x00689af7`, which no ProCircuit code reads). The object is the career at TrackGame+0x3444
(0x12e5 bytes; ProCircuitProcs allocates that size). It is named `ProCircuit` after the unit (its constructor pushes that
`__FILE__` literal), and TrackGame.h now includes
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

Exact (35):
- The eight `$E`.
- Three qsort comparators.
- The state-dialog opener `0x004d4ba0`.
- The slot 29 of PCStartupDlg, PCNewDlg, PCCentralDlg, PCCentralNextDlg,
  PCCentralStandingsDlg, PCLastRaceDlg, PCFailedDlg, PCFinishedDlg,
  PCBonusTrackDlg, PCNewEventDlg and PCCompleteDlg. PCBonusTrackDlg's
  (`0x004d9cd0`) declares its text buffer in each case that uses it.
  PCNewEventDlg's (`0x004d9fd0`) /GX prologue loads `mov edx,[esp+4]` between the fs:[0] load and `push -1`;
  `mcm2tool/resolved_match.py` recognises that shape so its handler
  `0x0054d257` binds.
- Their helpers.
- PCCentralBikeRiderDlg's slots 13, 26 and 31, its plate painter and rider
  store. Slots 13 and 26 share their bodies with the SP/MP bike dialogs'
  vtables.

Source forms needed:
- Clamped copies use a `sizeof`-based macro.
- Retail ORs only the low byte of TrackGame+0x1ee8.
- Inline helpers keep the CircuitNNN.pc search loop unrotated.

Near misses, in `ProCircuitProcs.cpp` in retail order with `// Not exact`
status comments ([forms tried](#near-misses-forms-tried)). They replaced the
game build's four bring-up copies (`Group0_PCLastRaceDlg.cpp`,
`Group5_PCBailoutDlg.cpp`, `Group4_DlgPCBunny.cpp`, `Group3_BikeRiderDlgPC.cpp`):
- The race payout `0x004d8c40` (427/883).
- The slot 29 of PCBailoutDlg `0x004d8fc0` (836/882) and PCBunnyDlg
  `0x004d9860` (1068/1122): prologue scheduling differs.
- PCCentralBikeRiderDlg slot 10 `0x004d72c0`, 1237 of 1291 bytes: only
  the scheduling of the by-value vector copies for the camera call differs.

Exact in the unit but not registered: `0x004d59a0` (its reload order flips
with unrelated declarations) and `0x004d6fc0`.

Also exact: PCCentralBikeRiderDlg slot 29 `0x004d6770` and `0x004d7900`.
The `UnknownProCircuitSkinned` view is the same object as DlgProcs.h's
`DialogSoultreeObjectView`; one should eventually replace the other.

**`$E` attribution.** A shared-header vector set is emitted after its
unit's functions (see CUBE.md). `.CRT$XCU` lists `0x004d3340`, `0x004d49e0`,
`0x004da3e0` and then `0x004dc4d0` in a row, and their vectors sit in
ascending `.bss` in that order. Each set's vectors are read only by the code
before it: `0x004d321a` (gearbox), none (ProCircuit), `0x004d6aba` and
`0x004d73df`/`0x004d7546` (ProCircuitProcs), and `0x004da591` (ProjectedShadow.cpp,
which owns the `0x004dc4d0` set; see [INITIALIZERS](INITIALIZERS.md)).

## Near misses: forms tried

Carried over from the header of the former `samples/ui/ProCircuitProcsNearMisses.cpp` when its four bodies moved into ProCircuitProcs.cpp.

```text
Near misses for ProCircuitProcs.cpp (canonical file:
src/reconstructed/ProCircuitProcs.cpp, included below for its types and
its matched functions). Check: compile this file and compare each function
with ProCircuitProcsNearMisses.bindings.json.

PCLastRaceDlg::PayOutRace (0x004d8c40, 883 bytes): the same
  flow, calls and x87 code; in the per-racer update retail loads +0x0c
  before +0x08 for `money += stunt - medical - repairs - fee +
  winnings` and then adds +0x0c into +0x24 in memory, VC6 here loads +0x08
  first. Reassociating or splitting the expression compiles the same
  (all 120 orders of the five terms give one object); reordering the
  points/paid/+0x30 statements, a winnings local, an AddStats inline
  helper (pointer or reference), a by-value net helper or a stats pointer
  are worse. The stats field types do not matter: unsigned int or long
  for +0x04, +0x08, +0x0c or +0x14 alone, or for every field of
  ProCircuitStats (ProCircuit.h), all give 427. Retail keeps stunt
  (ecx) and repairs (edx) and reloads medical for the +0x24 update; VC6
  here keeps medical and repairs and reloads stunt. A `const int&` alias
  of medical, repairs, stunt or winnings used in the money expression,
  in its +0x18 accumulation or both, and a stats reference for the money
  expression, give the same or more instruction diffs (12-25).
PCBailoutDlg::HandleEvent (0x004d8fc0, 882 bytes, 836 match) and
PCBunnyDlg::HandleEvent (0x004d9860, 1122 bytes, 1068 match): the
  init case reads the fee and the cash into locals and stores the fee
  member last; that gives retail's ecx/edx choices from the first
  FindControl call on (750 and 225 with the fee member stored first, as
  retail does: its fee goes to ecx with `mov edx, ecx; mov ecx, esi;
  sub edx, eax`, and every fee-first form restores the old registers;
  all fee/cash store orders, chained and embedded assignments were
  swept). What remains: retail loads the
  message kind into ecx after moving `this` to esi and before loading
  TrackGame+0x3444 (the reverse here), and picks ecx/edx the other way for
  the TrackGame/module temporaries. Local order, scopes and a `kind` copy do
  not move it, nor do per-case circuit locals, a `TrackGame* game` local,
  a const/register circuit, an inline circuit getter or a circuit macro
  (exact PCNewEventDlg slot 29 has the same source shape and retail's
  order). g_TrackGame->proCircuit read in every case, a leading `kind`
  local and `self = this` are worse or the same, and no typedef count
  (0..63) ahead of the TU makes 0x004d8c40, 0x004d8fc0 or 0x004d9860
  exact, so it is not a symbol-arena tie-break.
  Session 25 for 0x004d8fc0: 0..7 dead initialised locals and seven
  more init-store forms stay at 836 (fee-first member forms 750, the
  others 101).
  Session 14 for 0x004d9860, base 1068: a `switch (circuit = ..., kind)`
  comma form, an event pointer copy, `event[0].kind`, `char text` ahead
  of the circuit, a const circuit with `switch (int kind = ...)` and a
  series local are 1068. A leading `kind` local, also with a TrackGame
  local, keeps kind in eax and moves TrackGame to ecx (108). The fee
  stored first, before or after the cash load, gives retail's first store
  but not its registers (135-141); `int fee = entryFee = ...` chains and
  member-only arithmetic give 225. Loading debt after fee or cash gives
  1048-1054.
  Inline setters for 0x004d8fc0's fee, cash and shortfall stores (one
  three-store helper given the loads or the locals, per-field setters,
  a cash-and-shortfall setter) are all worse than the locals form in the
  same standalone unit (485 or less against its 586).
PCCentralBikeRiderDlg::Tick (0x004d72c0, 1291 bytes, 1237
  match): as SPBikeRiderDlg slot 10 (DlgProcs.cpp), only the
  scheduling of the by-value Vector3 copies for the camera call differs.
  kVec3YAxis is this unit's own copy at 0x00689b28 (ProCircuitProcs.cpp's
  `$E` vector, as in the canonical bindings); the sample had bound
  ProCircuit.cpp's 0x00689ae8 (1125).
  Session 19: code-free separators between the copies (`if (0) {}`, `if (0)`
  around a statement, a zero-trip for, `while (0)`, `switch (0)`, a switch
  on a member) all stay 1237.

0x004d8fc0 prologue, unchanged at 836 (2026-11; 0x004d9860 shares it):
the circuit as `ProCircuit*&`, `ProCircuit* const&`, a `ProCircuit**` slot
or `*&`, assigned after its declaration, a TrackGame local or
`TrackGame*&`, the event kind through an inline accessor or
`(*event).kind`, and a comma `kind = (circuit = ..., event->kind)`.
Worse: the fee stored first, and game/kind/circuit declaration orders
with an int, unsigned or const kind (836, or 91/883 with the kind in eax
and TrackGame reloaded).
2026-11, second pass. Retail scan: of the dialogs' HandleEvent methods
only 0x004d8fc0, 0x004d9860 and exact PCCompleteDlg 0x004da1b0 load the
kind into ecx, and in 0x004da1b0 that is because the g_TrackGame
temporary (eax) is still live when the kind is loaded (its init case
calls g_TrackGame->proCircuit->Save first); exact PCNewEventDlg
0x004d9fd0 has the kind in eax as the candidates here do. So retail keeps
a g_TrackGame-derived value live across the kind load in these two as
well, and the fee/cash rotation (series eax, fee ecx stored first, cash
eax, shortfall in edx) follows from it. No source form found that does
it, all 836 unless noted: the cases in command-first order, a default
arm, if/else chains for the switch (72-86), `switch (event->kind - 1)`
(68), an unsigned or cast kind; the circuit through `CircuitOf(game)`,
a `TrackGame*&` or a parameterless inline getter, the kind through an
inline accessor, each against six init-store forms (72 combinations:
836, 750 for the member-first stores `entryFee = ...; playerCash = ...;
cashShortfall = entryFee - playerCash;`, 99-101 for the others);
g_TrackGame->proCircuit written out at any one of the three init reads
(836; 750 or 86 with member-first stores); a `game` local kept live into
the init case (89: TrackGame in ecx, kind in eax); the kind read before
the circuit (91/883). A throwaway extra store or test in the first block
never moved the kind to ecx either.
```
