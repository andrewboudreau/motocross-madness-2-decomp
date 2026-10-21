# Menu texture color scan

## Identity and complete extent

`TextureHasColor(TextureMap*,int)` is the missing cdecl provider at
`0x0047b490`. The strict file-loading UIFrame constructor calls it at
`0x00472ac9`; the strict stream-loading UIFrame method calls it at
`0x00472cb9`. Both pass the newly created PCTextureMap and `0x00ff00ff`, then
conditionally invoke ApplyColorKey. This is a menu image-loading dependency.
The semantic name and original translation-unit ownership remain reconstructed.

The complete retail body is **220 bytes**, `0x0047b490..0x0047b56b` inclusive.
Its two returns are at `0x0047b564` and `0x0047b56b`. All direct branches stay
within that extent; four NOPs precede the next function at `0x0047b570`.
There are exactly two calls, both virtual: texture slot 13 at `0x0047b4a8`
and slot 14 at `0x0047b558`. There is no EH registration, direct helper call,
global access, allocation or switch table. The historical 120/220 sample
headline uses the correct full retail extent in this case.

## Functional build review

`samples/ui/TextureHasColor.cpp` contains the unchanged complete readable
candidate. Authentic VC6 SP3 `/O2 /GR /GX /MT` emits **220 bytes with 120 matching
positions** against the full retail body. It remains nonexact, with register
allocation, OR ordering and loop/return placement differences. The source stays
in `samples/`; bounded native behavior evidence does not establish original
source ownership or byte identity.

`GameUiNearMisses.cpp` includes the same body at its original position. The
saved same-path aggregate from baseline `9e92f134` and the extracted aggregate
have identical **733 nondebug sections and 3,912 relocations**, including
complete function bytes, data, initialization, EH and filename storage. There
are no filename exceptions. Defined relocation destinations are compared by
section identity plus offset, and undefined destinations by name. The ignored
proof is retained in `work/texture-color34/check_extraction.py` and
`same-path-preservation.json`, with `aggregate-before.obj` / `aggregate-after.obj`.
The standalone and aggregate versions of this function also have identical
complete bytes and relocations. The standalone has no external imports and no
initialization or EH sections.

The actual PCTextureMap primary RTTI table at `0x00555fec`, object offset zero,
selects the existing canonical lock and unlock providers. The retained test
checks those retail entries and their emitted source-vtable relocations, and
reproves the complete providers and both callers strict:

| Provider or caller | Retail VA | Complete bytes |
| --- | --- | ---: |
| PCTextureMap::LockSurface | `0x004c79e0` | 99 |
| PCTextureMap::UnlockSurface | `0x004c7a50` | 25 |
| UIFrame file constructor | `0x00472960` | 566 |
| UIFrame::LoadFromStream | `0x00472bc0` | 315 |

The native fixture executes source-derived copies of the two actual helpers,
with private nonvirtual names solely to isolate their bodies. Each copy must
independently pass the same full strict comparison before it is linked. The
candidate and mapped retail receive the same source helper addresses in their
texture vtables. Their lower-level DirectDraw COM calls are scripted; there
are no replacement production providers or retail execution in the game build.

## Preserved behavior

LockSurface is called with null rectangle, null pitch output and flags `0x11`,
before checking format or dimensions. A null returned pixel pointer returns
zero without Unlock. The real helper zeroes its entire 124-byte descriptor,
sets its size, and calls COM Lock. Any nonzero COM status produces a null
result. A successful COM lock that supplies null pixels also follows the
no-Unlock path; the fixture preserves that held-lock outcome.

After Lock, the function loads the current texture format. Format 565 packs
RGB to 5/6/5; format 555 packs to 5/5/5. Other formats skip pixel reads. A packed
key of zero also skips every pixel read, including when the image contains
black. Positive dimensions are scanned as tightly packed WORD rows: the
function intentionally supplies no pitch output. Each row stops at its first
matching pixel, but finding a match **does not stop later rows**. Nonpositive
height or width produces no pixel reads. The original signed dimension and
pointer arithmetic are retained without added bounds checks.

UnlockSurface is called after a nonnull Lock result even when no scan occurs.
It converts the COM result to logical success; a false helper result forces
TextureHasColor to return zero even after a match. The actual helper reloads
the system-surface pointer, so replacement during Lock can change the Unlock
receiver. There is no synthesized cleanup after a pixel fault or thrown COM
callback. The source does not change these behaviors to satisfy the fixture.

## Native evidence and limits

`tests/test_texture_has_color_behavior.py` compiles the actual standalone
candidate as a separate VC6 object and compares it against hash-pinned retail
in **131,899 native scenarios**. It also checks the complete CFG, both call
sites, primary slot identity, caller direct-call targets, relevant layouts,
provider strictness, import closure and the unchanged aggregate function.
The mapped retail function's code is not patched; its two virtual boundaries
execute the verified source lock/unlock helpers.

The fixture observes the COM calls, including receiver, arguments, complete
initialized input descriptor, returned descriptor and HRESULT. Whole texture,
pixel-buffer, surface/lock-count and accessible guard-page storage is compared
at each boundary and at completion, together with callback order, result,
exception kind and fault address. Lock and Unlock failures, positive and
negative statuses, null dependencies, null/invalid pixels, format and dimension
changes, pixel mutation, surface replacement and exceptions are exercised.

Independent oracles cover every possible 16-bit pixel value against an
independently expanded representative RGB key for each of 555 and 565, plus
signed/high-bit and channel-truncation keys. Bounded dimension grids cover
negative/zero sizes and first/middle/last/absent matches. A distinct descriptor
pitch proves that the scan uses tightly packed rows. Controlled aliasing with
texture storage is included. Guarded cases distinguish an early inner-loop
match from continued outer-loop traversal: a first-row match followed by an
inaccessible later row still faults with the lock held and no Unlock. Invalid
pixels are not dereferenced for unsupported formats, packed-zero keys or
nonpositive sizes. Independent assertions also check suppressed matches on
Unlock failure and lock state after each scripted thrown callback.

These tests use finite buffers, bounded dimensions and scripted COM surfaces.
They do not establish actual DirectDraw behavior, concurrent texture mutation,
complete resource loading or working menus. Exhausting 16-bit pixel values is
not an exhaustive test of every 24-bit RGB key or arbitrary pointer/dimension
combination. Private retail mapping is validation-only.

```powershell
$env:VC6_ROOT = (Resolve-Path work/vc6sp3-disc).Path
$env:MCM2_EXE = (Resolve-Path work/game/mcm2.exe).Path
$env:PYTHONPATH = '.'
python -m unittest discover -s tests -p test_texture_has_color_behavior.py -v
```

The isolated real link `work/game-build/build-cx_80xxq` compiles 229 translation
units and removes exactly `TextureHasColor`: **321 to 320 unresolved symbols**,
with no new missing names. Source, header and configuration hashes bind this
result to the reviewed composition. The source-built game still does not link.
