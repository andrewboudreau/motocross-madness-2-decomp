# Texture rectangle copying

`PCTextureMap::CopyRectTo` is implemented as a complete nonexact candidate in
[TextureRectCopy.cpp](../samples/render/TextureRectCopy.cpp). The aggregate
`PCTextureMapNearMisses.cpp` includes that source at the original body position.
Original TU ownership remains unproven.

## Extent, ABI and dependencies

The complete retail body occupies **0x004c7b40 through 0x004c7e2d**, inclusive:
**750 bytes**. Its returns at 0x004c7b9f and 0x004c7e2b both pop 24 argument
bytes; two alignment NOPs follow the final return. All decoded branches stay
inside the body. There is no EH setup and no direct helper call. The five COM
call sites are:

| Site | Surface slot | Operation |
| --- | --- | --- |
| 0x004c7b87 | 7, +0x1c | Destination `BltFast` |
| 0x004c7bd5 | 25, +0x64 | Source `Lock` |
| 0x004c7c07 | 25, +0x64 | Destination `Lock` |
| 0x004c7dfb | 32, +0x80 | Destination `Unlock` |
| 0x004c7e13 | 32, +0x80 | Source `Unlock` |

The surface descriptions are 124 bytes. Both begin zeroed with their size set,
and the lock arguments are `(null, description, 0x811, null)`. The retained
test checks native description/texture offsets and all five decoded sites.
The candidate has no undefined COFF symbols, code relocations, initializers,
EH data, storage definitions or other emitted functions.

`BackgroundImage::DrawImageRect` is the current production caller. Its complete
719-byte body at 0x00404700 remains strict, including six calls at 0x0040474d,
0x0040476e, 0x004048a9, 0x004048c7, 0x00404918 and 0x00404974. The test checks
those original call destinations independently. The known `GameCursor` call
passes a null table, taking the ordinary `BltFast` path. Address 0x005777c8 is
a source rectangle, not evidence of a palette lookup table.

## Preserved pixel and failure behavior

A null table forwards coordinates, surface, rectangle and flags to `BltFast`.
Any nonzero HRESULT yields zero; zero yields one. With a table, the method
locks both surfaces, reads the rectangle after both callbacks, and processes
pixels only when unsigned x/y are below the destination dimensions. Clipping
uses the observed inclusive bounds and subtracts **two**, not one, at the
right/bottom edge. There is no additional clipping to source storage.

For an 8-bit destination, each byte becomes
`table[sourceByte * 256 + destinationByte]`. Aliased surfaces, pixel buffers
and lookup storage retain their sequential read/write effects. The format
test is exactly `bitCount == 8`; every other value takes the WORD path.

In that path, retail zero-extends the destination WORD into ESI and then reads
a byte through that numeric address:

```text
004c7d8f  xor esi,esi
004c7d93  mov si,word ptr [eax]
004c7d96  mov dx,word ptr [edi+eax]
004c7d9f  mov bl,byte ptr [esi]
```

This is an access to **0x00000000..0x0000ffff**, not an omitted base address.
The source retains it. If that read were accessible, the source WORD would
select a 256-entry table block, and the resulting byte would be zero-extended
into the destination WORD. Successful execution of that low-memory-dependent
pixel operation is not claimed by the current native proof.

The initial source row uses signed `pitch * top / 2`, truncating toward zero:
retail emits `cdq; sub eax,edx; sar eax,1`. The initial destination row uses
unsigned y and a logical shift for division by two. Subsequent row increments
divide both signed pitches by two with truncation toward zero. Odd and
negative pitches are included in the native comparisons.

Failure handling is deliberately unchanged. Destination-lock failure leaves
the successful source lock held. Destination-unlock failure skips source
unlock. Pixel access violations and callback exceptions leave acquired locks
held; there is no invented cleanup or recovery. The final source unlock reads
the texture's current `systemSurface` again, so callback pointer mutations
remain observable. No AddRef or Release is introduced.

## Access-order correction and preservation

The prior sample emitted **755 bytes / 257 matching resolved positions**. It
loaded the source WORD before the destination WORD. An independent native case
with source address 1 and destination address 2 observed a source fault at 1
from the candidate versus a destination fault at 2 from retail. Ordinary
separate local assignments did not constrain VC6's scheduling.

The correction uses two explicit volatile WORD reads, destination first, to
preserve the observed access/fault order. It does not change the object layout,
pixel values, pointer arithmetic or subsequent low-address read. This is a
behavioral requirement demonstrated by the failing case, not a stack-homing
or byte-score technique. The candidate is now **775 bytes / 364 matching
positions**, and remains nonexact. Its complete raw/resolved body hash changes
from `6b429187dafd18f3021ce8f7027f8f7734e3ee7fdcb890768b1791777fbd925d` to
`86776f81371b828a977dedfc5d00a1ad279e1c8a46e38623adfc6b8d65943596`.

Private preservation checks against baseline `94c364f0` establish that the
initial extraction retained all three aggregate functions and seven nondebug
sections. After the correction, only CopyRectTo and its text section change.
The other two complete functions and six sections, including Clone's EH
funclets, EH metadata, filename storage and CreateSurface, retain their full
bytes, section flags and named relocations. No filename exception, extent
trimming or omitted cleanup block is used. Reproduction scripts and reports
are in ignored `work/rect-copy29/check_extraction.py`, `source-preservation.json`,
`check_correction.py` and `correction-proof.json`. The extraction script
reconstructs the intermediate unchanged body from the original baseline;
the correction script verifies the current source explicitly.

## Functional build review

The isolated authentic source build `work/game-build/build-oxgukjco/report.json`
compiles 220/220 translation units and removes only this unresolved symbol:
**367 → 366**. All prior production objects remain unchanged under the three
separately reviewed KrustyBike provider substitutions for this composition.

The reviewed candidate is admitted individually as **nonexact source**. The
native test compiles the actual standalone source separately with authentic
VC6 SP3 `/O2`, checks its full emitted extent and imports, and requires the
aggregate to emit identical candidate bytes. The private validation executable
maps the hash-pinned retail image inside its own reserved storage; **no retail
instructions are patched**. Both implementations call the same scripted COM
surface vtables. Production never maps or executes retail code.

**72,899 scenarios** compare complete initialized buffers, lookup storage,
texture state, lock counts, ordered callbacks and their arguments, return
values, C++ exceptions and access-violation type/address. Independent expected
outcomes check ordinary pixel results, untouched bytes, overlapping buffers,
lookup-storage aliasing, one-pixel and empty clipping at the destination edge,
HRESULT failures and held locks. The test also covers null-table forwarding,
small/reversed/negative rectangle coordinates, unsigned x/y extremes, zero/
odd/negative pitches, repeated surface identities, callback changes to the
rectangle, descriptions and source pointer, and exceptions at each scripted
boundary on representative paths.

Before low-address cases, the executable verifies a minimum application address
of at least 0x10000 and checks that every page below it is uncommitted. All
**65,536 destination WORD values** are tested individually for the actual
non-8-bit read fault, with both locks retained. Additional non-8 formats,
signed/odd pitches and invalid source/destination buffers are compared. These
are real native faults; no low-memory read emulation or successful 16-bit pixel
claim is substituted for them.

The proof is bounded to finite supplied pixel buffers and scripted COM
contracts. It does not establish arbitrary overflowing loops or corrupt
descriptors safe, actual DirectDraw behavior, live device rendering, successful
non-8-bit table processing on historical low-memory mappings, or end-to-end
cursor/menu rendering. The useful null-table and 8-bit paths are exercised,
but link/startup/gameplay remain separate completion gates.

With native `VC6_ROOT` and the private `MCM2_EXE` configured:

```powershell
$env:PYTHONPATH = '.'
python -m unittest discover -s tests -p test_texture_rect_copy_behavior.py -v
```
