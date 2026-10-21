# CacheTexture mip copying

`samples/render/CacheTextureNearMisses.cpp` contains the complete readable
`CacheTexture::CopyMipLevels` candidate. The existing standalone file and its
bindings are unchanged. It remains nonexact in `samples/`; no original
translation-unit ownership is established by this grouping.

## Identity, extent and dependencies

The complete retail function begins at `0x005102d0` and occupies **558 bytes**.
Its three `RET 8` instructions are at `0x005104b1`, `0x005104e2` and
`0x005104fb`; two following NOPs precede the ManagedTexture constructor at
`0x00510500`. Every decoded internal branch stays within the complete body.
There is no SEH/EH registration or separate unwind handler in this function.

`CacheTexture::PlaceTextures`, `0x0050fdb0/492`, calls it directly at
`0x0050ff72` after placing an occupant when no deferred placed-texture list
was supplied. This caller is a full strict match. The candidate populates
the occupant's region of a cache page, starting at an appropriate source
mip level, then uploads the changed rectangle or marks the cache dirty.
The behavior names remain provisional.

Authentic VC6 SP3 emits **569 bytes**, with **337 matching positions** against
the complete 558-byte retail body. The existing source and nonmatch are
preserved; no padding, extent reduction or compiler-only source distortion
is used. The retained test validates five direct calls: UpdatePageCoords
and four CRT `__ftol` conversions. It also validates all five indirect
sites: three GetAttachedSurface calls, Blt and virtual Upload.

The direct game helper, `ManagedTexture::UpdatePageCoords` at
`0x00510610/85`, and the inherited virtual Upload implementation,
`PCTextureMap::Upload` at `0x004c7640/385`, already have complete strict
canonical providers. CacheTexture's primary vtable `0x005583d8`, object
offset zero, points to the latter in slot 9. The only other undefined
symbols emitted by the candidate are existing `g_TrackGame` and the normal
CRT `__ftol`/`__fltused` symbols. It has one function, one float constant,
no dynamic initializer and no generated EH code/data.

Private `work/cache-mips27/provider-proof.json` records complete resolved
byte matches for the helper, Upload and PlaceTextures in actual
`build-orqgbi60` objects. The retained test independently recompiles those
three canonical files and checks the same complete functions and the raw
caller instruction. This proves provider availability; it does not imply
that the entire game links or runs.

## Functional build review

`tests/test_cache_texture_mips_behavior.py` compiles the actual whole
candidate at `/O2` and `tests/fixtures/cache_texture_mips_compare.cpp` at
`/Od`. It compares the candidate with the full hash-pinned retail function
in a private validation executable. Only the five decoded direct call
sites are redirected. Production source contains no retail mapping or
dispatch.

The test passes **5,674 scenarios**. It compares complete modeled cache,
occupant, region, group, game, display and surface storage; returned status;
exceptions and fault addresses/operations; FP status; and the ordered
callback transcript. Each boundary snapshot includes the complete modeled
state and FP status. Every attachment caps record is checked as all 16
initialized bytes, and each blit/upload records all four rectangle fields.

The four float-to-integer conversions execute the real VC6 CRT `__ftol`.
The coordinate helper executes the complete current canonical C++ body
under a private renamed method on an empty derived class. Before execution,
all 85 emitted bytes and resolved relocations must match retail. This
validation-only extraction avoids linking unrelated canonical methods into
the fixture; it is not an alternative production implementation.

Coverage includes:

- Texture widths and source sides from 1 through 256, mip-level selection,
  distinct source/destination chain lengths, both coordinate-helper gates,
  source-mip skipping and repeated rectangle halving.
- Fractional, negative, extreme, infinite and quiet-NaN coordinates under
  all four x87 rounding modes with the default masked FP exceptions. Signed
  dimension and counter extremes are compared within the scripted boundary.
- Nonzero success/failure statuses, attachment failure at each tested
  position, null or unwritten output pointers, output writes on failure,
  cyclic chains and aliases between source and destination surfaces.
- Blt failure, attachment failure after a successful blit, ignored Upload
  return values, positive versus nonpositive partial-upload capability,
  callback exceptions, null dependencies and callback changes to subsequent
  receiver/global/object-field reads.

Independent expected-outcome assertions check the full initial rectangle
and its five mip rectangles, correct source/destination identities after
skipping source levels, accumulated texel area, helper-produced coordinates,
reference-count changes and the original whole rectangle passed to Upload.
Other assertions check dirty-flag behavior, early failure, the destination
attachment call that still occurs after source attachment failure, and
exception-time state. No Release call is expected or inserted: references
returned by the scripted attachments remain held, matching observed retail
call behavior.

### Explicit finite-chain contract

The fixture supplies at most ten distinct surfaces per chain. Every scenario
allows **at most twelve successful GetAttachedSurface calls**, then returns
a scripted failure HRESULT on the thirteenth call. This is an observable
external response, not a cutoff inside either tested function. Ordinary
acyclic chains also fail naturally when their configured depth is exhausted.

This bound matters for invalid geometry: a negative destination side can
leave the first loop testing `0 > negative` after the source side has been
shifted to zero. If a cyclic attachment chain keeps succeeding, retail need
not terminate. A dedicated negative-width/cyclic case verifies the precise
thirteenth-call failure, zero return, and absence of blits or upload. The
test makes no termination claim for an endlessly successful such chain.

COM attachment, blit and Upload bodies remain scripted. Surface reference
counts and payload changes model those boundaries; they do not establish
DirectDraw ownership integration or actual pixel copying. The canonical
Upload provider's strict byte proof is separate from this test. Invalid
dimensions are exercised only under the stated finite COM contract, not
asserted safe for real DirectDraw surfaces.

## Preservation and reproduction

No candidate extraction or production source edit was needed.
`work/cache-mips27/check_source.py` independently compiles the candidate from
baseline `eb470e531e943af060ddd08ec40b9ea5accdf505` and the current file.
`source-preservation.json` records the source unchanged after line-ending
normalization, and the complete function plus all three nondebug sections
identical in bytes, flags, sizes and relocation targets/addends. There is
no filename-storage exception and no initializer to exclude.

```powershell
$env:PYTHONPATH = '.'
$env:VC6_ROOT = (Resolve-Path work/vc6sp3-disc).Path
$env:MCM2_EXE = (Resolve-Path work/game/mcm2.exe).Path
python -m unittest discover -s tests -p test_cache_texture_mips_behavior.py -v
```

The candidate is explicitly admitted as `reviewed-nonexact` in
`config/game_build.json`. The isolated source link in `build-8ezxtlff` compiles
216/216 translation units and reduces unresolved names from 378 to 377:
CopyMipLevels disappears and no new missing name appears. All pre-existing
production objects remain unchanged under the separately reviewed collision
and gearbox connections. This link result remains a failure; startup, menus
and racing are unverified.
