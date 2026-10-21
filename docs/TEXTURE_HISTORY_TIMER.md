# Texture-history timer construction

The genuine existing `UnknownTimer(int)` declaration belongs to three embedded
12-byte histories in `ManagedTextureGroup`. Its complete constructor at
`0x004cb670` is **30 bytes**, through `ret 4` at `0x004cb68b..0x004cb68d`.
The ordinary C++ definition emits all 30 bytes strictly, without flags, aliases,
wrappers or changes to the existing class declarations. Original class and TU
names remain provisional; this does not claim a second distinct retail function.

## Actual owner and data evidence

The complete strict 264-byte TextureMapManager::CreateGroup allocates 0x258 bytes
at 0x5111a5/0x5111aa, passes the allocation unchanged in ECX and calls the complete
996-byte ManagedTextureGroup constructor at 0x5111d2. That constructor passes
5000 to the timer at primary offsets +0x1d4, +0x1e0 and +0x1ec, at calls
0x50c057, 0x50c067 and 0x50c077. The owner writes primary vptr 0x5583c0; its raw
COL/type descriptor identifies ManagedTextureGroup at offset zero. Compiled real
header layouts independently confirm these offsets and total size. The timer
itself is a plain nonvirtual record; no timer RTTI or complete original class
identity is inferred from the shared constructor address.

The constructor zeros +4 before calling ReadClock, writes its returned DWORD to
+0, then copies the interval bits to +8 and returns this. The adjacent complete
Peak/Record bodies at 0x4cb690/28 and 0x4cb6b0/43 independently explain the fields:
+0 is the peak timestamp, +4 the signed sample peak, +8 the unsigned elapsed-time
threshold. Actual owner repacking candidates use these three histories for
texels, elapsed blit time and texture count. Those sample method imports are not
newly supplied or admitted here.

`UnknownPeakHold` already supplies matching 30/28/43-byte bodies. A private alias
trial preserved both texture-group objects under three name mappings, but changed
strict Scene::ReadLight from 2102 to 2065 matching positions. Moving UnknownTimer
verbatim into a narrow header had the same regression. Both header-changing
approaches are rejected. The proposed source instead includes the unchanged
TextureMapManager.h and defines only its existing constructor. The class views
remain separate. Its object contains .drectve40 and .text32 (30-byte function plus
two padding bytes), no data or CRT initialization, and only the supplied ReadClock
import. Four old canonical/sample objects preserve all 692 sections and 3870
relocations exactly, with no mappings or exclusions in the same-path proof.

## Real clock closure and finite native proof

The existing ReadClock at 0x4bfa80 is complete and strict over 29 bytes. It calls
WinMM timeBeginPeriod(1), timeGetTime(), timeEndPeriod(1), then returns the saved
time. The native fixture uses a separately compiled extraction of this actual
source body, strict against retail and identical to the canonical PCGame body
with all named relocations. The new constructor is independently strict. There
is no runtime retail dispatch or fabricated game helper.

The retained test passes **eight complete strict checks**, four whole
old-object pins and two finite native matrices:

- 3,072 constructor calls use actual WinMM functions. Thirty-two page patterns,
  eight interval bit patterns (including 5000, signed limits and all ones), eight
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
  interval, and verifies begin/get/end order and period 1. Success ignores the
  nonzero begin/end status, while 96 cases raise a distinct exception at each of
  the three boundaries and require only the first peak store to have occurred.
  All IAT slots and page protections are restored afterward.

These tests observe prepared raw storage and finite VC6/x86 calls, not the full
owner lifetime, resource packing, scheduler or arbitrary platform behavior.
Real-clock timestamps are not compared bit-for-bit to a separately run retail
clock. Exact constructor/helper code proves those body identities; the native
matrices provide independent storage, order and fault oracles. No arbitrary
receiver aliasing, asynchronous interrupts or exception-resumption context is
claimed. The controlled platform callbacks are not production behavior.

## Portable object regression

The four old private object pins were independently reproduced before correcting
their checkout-path portability. The retained `FILENAMES` tuple enumerates the
four source paths and two exact `ContainerList.h` include spellings: canonical
`src/reconstructed/ContainerList.h` and sample-relative
`samples/render/../../src/reconstructed/ContainerList.h`. These remain distinct
labels; no general path collapsing or filename exclusion is used. Fresh objects
compiled from the published checkout and the private mirror have equal complete
views under this normalization: 692 sections and 3,870 relocations. Three pins
change only because the previously unnormalized header filename and its named
relocation destination become portable; the ProCircuit pin is unchanged.

## Source-build verification

Real build `build-bwfzggry` compiles all 266 translation units and removes
exactly the constructor import from ManagedTextureGroup: **210 -> 209**, with
no new unresolved names. All 265 prior game objects retain 10,713 nondebug
sections and 45,818 relocations, with no name mappings or filename exclusions.
The actual game-command provider has two sections, one relocation to the
existing ReadClock, and the complete 30-byte strict body.

The actual-checkout retained test passes all eight strict checks, four object
pins and both native matrices. Its portable pins enumerate the four source
paths plus the two exact ContainerList.h filename spellings observed in the
canonical and aggregate objects; original private pins were reproduced before
the normalization correction, and all four complete cross-root views agree.
This metadata change does not alter production code.

One new calibration row is strict. The final composed real link is
`build-jwoie6g6`, with 208 unresolved symbols after the separate light lookup
connection. The game does not link; startup, menus and racing are unverified.

Run the retained test with authentic VC6_ROOT and MCM2_EXE, with PYTHONPATH=.;tests:

    python tests/test_texture_history_timer.py -v
