# Texture-history timers

`ManagedTextureGroup` embeds three 12-byte `PeakHold` histories (TextureMapManager.h).
PeakHold.cpp supplies the bodies: the constructor at `0x004cb670` (30 bytes,
through `ret 4` at `0x004cb68b..0x004cb68d`), `Peak` at `0x004cb690` (28) and
`Record` at `0x004cb6b0` (43). All three are strict. The original class and TU
names remain provisional; PeakHold is descriptive.

## Actual owner and data evidence

The complete strict 264-byte TextureMapManager::CreateGroup allocates 0x258 bytes
at 0x5111a5/0x5111aa, passes the allocation unchanged in ECX and calls the complete
996-byte ManagedTextureGroup constructor at 0x5111d2. That constructor passes
5000 to the timer at primary offsets +0x1d4, +0x1e0 and +0x1ec, at calls
0x50c057, 0x50c067 and 0x50c077. The owner writes primary vptr 0x5583c0; its raw
COL/type descriptor identifies ManagedTextureGroup at offset zero. Compiled real
header layouts independently confirm these offsets and total size. The timer
itself is a plain nonvirtual record; no timer RTTI is inferred from the shared
constructor address.

The constructor zeros +4 before calling ReadClock, writes its returned DWORD to
+0, then copies the hold bits to +8 and returns this. Peak and Record explain the
fields: +0 is the peak timestamp (`peakTime`), +4 the signed peak (`peak`), +8
the unsigned hold (`holdTime`). The histories record texels, elapsed blit time
and texture count.

## History of the fold

TextureMapManager.h used to declare its own `UnknownTimer(int)` view, with
TextureHistoryTimer.cpp defining only the constructor and link aliases joining
`RecordSample`/`LargestRecentSample` to PeakHold's bodies. An alias trial and a
verbatim move of the view into a narrow header both moved strict Scene::ReadLight
(2102 to 2065). Typing the histories `PeakHold` with TextureHistoryTimer.cpp
dropped moved Terrain::Render3D and BikeRace::CreateRacers out of their
declaration-count windows when done alone (2026-11-25).

On 2026-11-28 the fold landed together with the OptionProcs.h settings fold and
the soultree-surface fold (docs/NAMING.md, "Three deferred views retired"): the
histories are `PeakHold`, TextureMapManager.h includes PeakHold.h, and the
duplicate provider and its calibration row are gone. Every object is identical
to the previous build apart from symbol names, label counters and COMDAT order,
and all six windows hold.

## Real clock closure and finite native proof

The existing ReadClock at 0x4bfa80 is complete and strict over 29 bytes. It calls
WinMM timeBeginPeriod(1), timeGetTime(), timeEndPeriod(1), then returns the saved
time. The native fixture uses a separately compiled extraction of this actual
source body, strict against retail and identical to the canonical PCGame body
with all named relocations.

The retained test passes **seven complete strict checks** (the three PeakHold
bodies, both ReadClock copies, the ManagedTextureGroup constructor and
CreateGroup), checks that the constructor calls `??0PeakHold@@QAE@I@Z` at the
three history offsets, pins three whole objects and runs two finite native
matrices against PeakHold.cpp's object:

- 3,072 constructor calls use actual WinMM functions. Thirty-two page patterns,
  eight hold bit patterns (including 5000, signed limits and all ones), eight
  aligned/unaligned accessible addresses and four fault addresses are exercised.
  Successful timestamps must fall in the actual surrounding timeGetTime interval;
  the full prepared page and all three fields are checked independently.
- 1,024 of those calls require exact write AVs. The +4-first failure leaves the
  page untouched; when only the first eight bytes are available, +4 and +0 must
  be written before the +8 fault. Address 1 is used for the low-address failure.
  A null placement-new expression can skip constructor execution in VC6, so it
  is deliberately not used as a claim about constructor-null behavior.
- 128 additional cases replace only this private executable's three WinMM IAT
  entries with explicit platform callbacks. The actual ReadClock instructions
  remain unchanged. Each callback requires peak zero and original timestamp/
  hold, and verifies begin/get/end order and period 1. Success ignores the
  nonzero begin/end status, while 96 cases raise a distinct exception at each of
  the three boundaries and require only the first peak store to have occurred.
  All IAT slots and page protections are restored afterward.

These tests observe prepared raw storage and finite VC6/x86 calls, not the full
owner lifetime, resource packing, scheduler or arbitrary platform behavior.
Real-clock timestamps are not compared bit-for-bit to a separately run retail
clock. The controlled platform callbacks are not production behavior.

## Portable object regression

The retained `FILENAMES` tuple enumerates the source paths and two exact
`ContainerList.h` include spellings: canonical `src/reconstructed/ContainerList.h`
and sample-relative `samples/render/../../src/reconstructed/ContainerList.h`.
These remain distinct labels; no general path collapsing or filename exclusion
is used.

Run the retained test with authentic VC6_ROOT and MCM2_EXE, with PYTHONPATH=.;tests:

    python tests/test_texture_history_timer.py -v
