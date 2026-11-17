# CacheTexture mip copying

`CacheTexture::CopyMipLevels` is in
[CacheTexture.cpp](../src/reconstructed/CacheTexture.cpp) after `RemoveTexture`
`0x005102b0`, in retail order, with a `// Not exact` status comment.
CacheTexture.cpp holds part of the retail `TextureCache.cpp` unit
([ORIGINAL_UNITS.md](ORIGINAL_UNITS.md)). The behaviour test links a
validation unit holding only this definition.

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

```powershell
$env:PYTHONPATH = '.'
$env:VC6_ROOT = (Resolve-Path work/vc6sp3-disc).Path
$env:MCM2_EXE = (Resolve-Path work/game/mcm2.exe).Path
python -m unittest discover -s tests -p test_cache_texture_mips_behavior.py -v
```

The game build links this body from CacheTexture.cpp. Startup, menus and
racing with it are covered only by the per-function checks above.

## Forms tried

CacheTexture::CopyMipLevels (0x005102d0, 558 bytes; ours 569): blits the
occupant's mip levels into its region. Retail places the shared `return 0`
block between the second loop and the overlay check (its loop ends
`jle; jmp top`). Both failures as one early-failure test after the loops
(the Blt result kept in `result` and `break`, then `if (result) return 0;`
in place of the else arm) gives that block order: 16 -> 5 differing
instructions (s14). Left: VC6 keeps `result` in a frame slot and tests
it again at the loop's normal exit, where retail jumps straight to the
overlay check. Unchanged at 5: `(result = Blt) != 0`, `int result`,
`!= 0`. Worse: a separate Blt variable (16-17), `!result` in the loop
condition (38-57), the early test without the Blt in `result` (16), the
first loop returning directly (18). The tries below were made on the
earlier form with the Blt `return 0` in the loop (16 diffs). The
halvings are written sourceSide, side, levelWidth, left, right, top,
bottom (the best of all 5040 orders, 450 -> 462 of 558, which matches
retail's loads).
Tried (s14, 450 at the old shift order): the Blt failure as `break`
(the else's `return 0` still goes last), the else without braces,
`goto failed` with `failed:` placed right after the loop and the
overlay check behind `upload:` (450-454), `if (sourceSide > 0)` around
a do/while or a bottom-tested for(;;) (293), a top-tested `for (;;)`
and a trailing `continue` (450). With `if (result) { return 0; } else`
VC6 keeps the `return 0` in place, so it is not always moved to the end.
Tried (s10, all unchanged at 450/558 or worse): the Blt failure as an
else arm or a goto into the else, `for (;;)`-style and for-increment
loops, the inner break as an if/else, the first loop returning 0
directly, an early `if (result) return 0;` (397), a do/while (293),
the Blt result stored and tested after the loop (338) and `goto failed`
to a label at the end or in an `if (result)` block (450-454). This is the
known `jle exit; jmp top` loop-bottom shape that VC6 has not reproduced.
Also 462 (s14, current order): the Blt `return 0` braced, with and
without an unbraced else, the Blt result in a named local, and `!= 0`.
Also 462 at the current order (s14 corpus pass): `goto failed` with
`failed: return 0;` after the loop and the overlay check behind
`upload:`, `failed:` inside the else arm, and the Blt `return 0` as an
if arm with the rest of the body in its else. `if (result) { return 0;
}` ahead of the loop (braced or not) gives 545 bytes. In the exact
loop-bottom examples (docs/VC6_PATTERN_CORPUS.md) the early return is
the body's last test; here it is the first.
s20 (409/462 strict, both worse than the current form): `if (result) return 0;`
between the loops with the Blt failure as an in-loop `return 0` (545 bytes,
the return block lands after the first loop); a trailing `continue` or
`return 0` at the loop-body end (same); and the whole second loop plus the
upload inside `if (result == 0) { ... return 1; } return 0;` (462).
s21 (all 466/558 strict, 9 differing instructions; VC6 moves the shared
`return 0` to the end and keeps a plain `jg top` latch): `if (result)
goto failed;` after the first loop (unbraced, braced, `!= 0`) with the
second loop as `while` and `failed: return 0;` in its Blt arm or `if
(Blt) goto failed;`; a `for (;;) { if (sourceSide <= 0) break; ... }`
with the label in the Blt arm; tails with no extra code, `if (0) {
failed: return 0; }`, or `goto upload; failed: return 0; upload:`. More
matching bytes than the kept form but more differing instructions.
s22. Retail's shape: the first loop's failure and the test after it both
jump to one `return 0` block placed right after the second loop, whose
latch is `jle upload; jmp top` with no `result` store or test, and the
second loop's GetAttachedSurface failures go to the upload. Tried:
- a constant failure flag (`if (Blt) { result = 1; break; }` or an `ok`
  flag, then `if (result) return 0;`) in a `while`: 466, 9 instructions.
  VC6 threads the flag away (no store, no test) but puts the `return 0`
  after the upload arm's `ret` and keeps `jg top`.
- `if (!result) { while { if (Blt) goto failed; ... } } else { failed:
  return 0; }`, and the body as `if (!Blt) { ...; continue; } return 0;`:
  462, 9 instructions.
- `if (sourceSide > 0) do { ... } while (sourceSide > 0);` with the Blt
  failure as `return 0`, as `result = Blt; if (result) return 0;`, as an
  if/else body, with the constant flag, as a bottom-tested `for (;;)`,
  or with `sourceResult`/`destinationResult` zeroed at the loop top:
  294/586 (the loop head is copied to the back edge); with the first
  loop's `if (result) return 0;` ahead of it 250/590; zeroed locals plus
  `result = Blt; break` 277/614.
k-scan (s22): compiled behind 0..63 prepended typedefs, 0x005102d0 has the
same code at every count, so its difference is not a declaration-count
effect (docs/VC6_OPERAND_ORDER.md section 2).
