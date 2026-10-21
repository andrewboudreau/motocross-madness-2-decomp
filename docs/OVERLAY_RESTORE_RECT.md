# Overlay rectangle restoration

`Overlay::RestoreRect` copies a rectangle from the texture at receiver `+0x30`
into the texture at `+0x2c`. Stats and chat HUD drawing use it before redrawing
content. The readable candidate remains in
`samples/render/OverlayNearMisses.cpp`; original translation-unit ownership and
semantic field names are not established by this implementation.

## Complete target and access-order repair

The complete retail body begins at `0x004b6710` and ends at `0x004b687c`,
**365 bytes**, including both `RET 4` instructions at `0x004b686e` and
`0x004b687a`. Three following NOPs precede the next entry at `0x004b6880`.
The prior 361-byte note omitted the complete final return. The retained test
checks every decoded instruction, internal branch destination, both returns,
three direct calls and four indirect calls. This body has no SEH registration
or unwind cleanup.

The old candidate emitted **350 bytes with 63 matching resolved positions**
against the complete 365-byte target. Its initial rectangle reads were top,
left, bottom, right. Retail reads left, top, right, bottom. This is observable:
a rectangle pointer of 1 faults reading address 1 in retail and address 5 in
the old candidate. With only left and top readable, the private old control
faults four bytes beyond the retail fault address.

The candidate now explicitly reads the four initial fields through a volatile
view in retail order, then computes height and width from those values. The
qualification is an **explicit reconstructed access-order model**, not proof
that the original rectangle type was volatile. An ordinary-local control with
the same four statements but no volatile qualifier emits the exact old
350-byte body and relocations. The later top and left rereads after the lock
callbacks remain unchanged.

The repaired candidate emits **365 bytes with 77 matching resolved positions**.
Equal extent does not make it byte-exact. Its complete function bytes and
relocations are tested; no extent trimming, helper replacement, special
compiler options or assembly is used. The standalone object has only two
nondebug sections: `.drectve` 40 bytes and `.text` 368 bytes, the latter including
alignment padding. It has one public function, three relocations to the
existing `PixelFormatBytesPerPixel`, and no data or CRT initializer. There is
no other aggregate body to duplicate or alter.

## Identity, callers and real helper boundary

Raw COL/type-descriptor names and primary table offsets are checked for
`Overlay` (`0x555a28`), `StatsOverlay` (`0x558680`), `ChatOverlay` (`0x558854`)
and `PCTextureMap` (`0x555fec`). The parsed base relations place Overlay at
primary offset zero in the two HUD classes, and TextureMap at primary offset
zero in PCTextureMap. This supports the receiver boundary; it does not prove
all provisional field names or that every TextureMap instance has this backend.

Four complete strict bodies supply independent producer evidence:

| Body | Retail VA | Bytes |
| --- | --- | ---: |
| PCTextureMap constructor | `0x4c5f00` | 46 |
| Overlay constructor | `0x4b5e20` | 120 |
| Overlay::Attach | `0x4b5f50` | 583 |
| StatsOverlay::LoadPanel | `0x5194b0` | 976 |

The constructors write their respective primary vptrs. Overlay initializes
its two texture fields; Attach stores its supplied TextureMap pointer at
`+0x2c`. LoadPanel stores the `TerrainLoadTexture` result at `+0x30`, invokes
that texture's slot 6, then stores the returned clone at `+0x2c`. The test
checks the actual call/store instructions as well as the complete bodies.
These methods are compiled and compared, not executed by the native fixture.

Seven complete strict TrackOverlay functions contain eight direct calls to
`0x4b6710`: StatsOverlay DrawViewModePanel (`0x51a302`), DrawRacePanel
(`0x51a8dc`), DrawTagStandings (`0x51ad4e`), and ChatOverlay RenderPre3D
(`0x51e2a0`), RedrawChat (`0x51de23`), RedrawNameLine (`0x51e84a`) and
RedrawNameTags (`0x51e949`, `0x51e971`). Both the complete caller comparisons
and their COFF relocation destinations are checked. The fixture does not
execute those complete HUD callers.

The native fixture executes unchanged readable source for:

| Helper | Retail VA | Bytes |
| --- | --- | ---: |
| PCTextureMap::LockSurface | `0x4c79e0` | 99 |
| PCTextureMap::UnlockSurface | `0x4c7a50` | 25 |
| PixelFormatBytesPerPixel | `0x511970` | 78 |

Each extracted helper is independently strict against retail and identical to
its canonical source object in complete bytes and relocations. Raw
PCTextureMap slots 13 and 14 identify the two texture helpers. Native layout
checks use actual headers: Overlay `0x11c`, PCTextureMap `0x80`, rectangle 16,
surface descriptor 124, the observed texture fields and four-byte Overlay
member-function pointer representation.

For differential execution, both paths invoke the same actual source texture
helpers through typed validation-only method wrappers. Only the reference's
three verified direct PixelFormat calls are redirected to the same source
helper. Each patch checks the original E8 opcode and target first. This is a
shared strict-helper comparison, not an untouched retail helper graph. The
surface Lock/Unlock backend is scripted; the copies are the actual VC6-emitted
`memcpy` sequences, without a replacement copier.

## Finite native validation

`tests/test_overlay_restore_rect.py` compiles the production sample with
ordinary VC6 `/O2` and **no extra project include directories**, then compares
its complete object to the actual game build command. The fixture uses `/Od`;
all candidate/helper code retains the normal production profile. Retail is
private, hash-pinned and mapped only in the validation executable.

The test requires **17 strict comparisons and 12,970 native scenarios**:

| Cases | Coverage |
| ---: | --- |
| 12,800 | Eight formats (8, 555, 565, 888, 1555, 4444, 8888, unsupported 0), texture widths 1–8, both copy branches, each pitch in -33/-17/0/17/33, heights 0–3; left/top vary -1–1 |
| 32 | Both branches, every source/destination failed-HRESULT and successful-null-output mask |
| 2 | Shared source/destination buffer base, including row-offset overlap from different pitches |
| 72 | Source-read or destination-write guard fault after 0–17 accessible bytes, both branches |
| 44 | Eleven mutations at each of four reached COM boundaries |
| 4 | Scripted exception at each reached COM boundary |
| 12 | Row branch heights -3 through 0 with unlock status 0, 1 or failing HRESULT `0x80004005` |
| 1 | Rectangle pointer 1, requiring the exact read fault and no callbacks |
| 3 | Only the first 4, 8 or 12 rectangle bytes accessible |

Ordinary cases require normal completion independently of the differential
comparison. A bytewise oracle checks every destination byte and unchanged
source bytes for separate buffers. It preserves the observed row-branch
initial offset based on **source pitch for both buffers**; the whole-width
branch instead applies each buffer's own pitch to top and copies one
contiguous block. No corrective graphics interpretation is substituted.

Both texture locks happen even if the first fails. Independent failure oracles
require return zero, no unlocks, and the expected held-lock counters, including
the successfully acquired other surface. Incoming descriptors must contain
size 124 and otherwise zero bytes, matching the actual helper. Scripted
Lock writes its outputs even when returning the failing HRESULT, but the
actual helper ignores those outputs on failure. Successful-null-output cases
are intentional finite backend contracts, not claims about a real driver.

Mutation cases alter rectangle coordinates, texture pointers, surface owners,
format, returned pitch/pointer or a returned null pointer. Every requested
boundary must be reached. Some writes at unlock have no later observable
consumer; they are not claimed to alter the result. Independent ownership
oracles verify that field reloads can reroute both unlocks to one surface.
Those deliberately inconsistent backend states can produce negative held
counters; they model observation of the supplied script, not valid DirectDraw
resource management. Exceptions and copy faults must retain the observed
partial state and held resources; this body performs no unwinding unlock.

Comparisons include the entire prepared receiver and texture storage, both
pixel/guard buffers, rectangle, surface counters, ordered callback arguments,
all 124 incoming/output descriptor bytes, snapshots before/after callbacks,
return/completion and access-fault kind/address. Guard cases independently
check the exact page fault and no unlock. Their independent copy-prefix oracle
checks the accessible prefix only: completed DWORDs and unchanged remaining
accessible bytes. Differential snapshots compare the complete prepared
destination storage as well.

The current harness also runs the ordinary-local negative-control object. It
must pass the preceding 12,966 cases and then exit at the explicit pointer-1
oracle with retail read AV1 versus candidate read AV5. It does not reach the
last three partial-rectangle cases. The earlier private smaller harness's
partial-rectangle failure is separate evidence, not a claim that it exercised
the expanded matrix.

Objects and surfaces are prepared raw storage; constructors, texture lifetime,
real COM interfaces, graphics memory, asynchronous callbacks and complete HUD
rendering are outside this test. The two overlap cases establish finite
VC6-emitted behavior, not portable overlapping-`memcpy` semantics. Finite
negative-height row cases do not establish arbitrary negative widths, enormous
unsigned copy counts, whole-width negative heights or unrestricted addresses.
No floating-point arithmetic occurs in this method.

### Functional build review

The candidate remains explicitly reviewed nonexact source. The isolated build
`work/game-build/build-jd7lf5es` compiled **254/254** translation units and
changed **244 to 243** unresolved names, removing only
`?RestoreRect@Overlay@@QAEHPBUUnknownOverlayRect@@@Z`, with no new names. The
independent recompilation confirms that the actual candidate object's two
sections and three relocations match the retained compile. This supplies the missing method;
the game still does not link, and this is not a working-menu or runtime claim.

Reproduce the retained test with private inputs:

```powershell
$env:VC6_ROOT=(Resolve-Path work/vc6sp3-disc).Path
$env:MCM2_EXE=(Resolve-Path work/game/mcm2.exe).Path
$env:PYTHONPATH='.;tests'
python -m unittest test_overlay_restore_rect -v
```

Private initial controls and decoded reports remain in
`work/connections48/overlay/`; the retained test supplies the current source,
full extent, helper, caller, producer, negative-control and native evidence.
