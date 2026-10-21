# Palette lookup-table getter connections

Pixtrans now calls the existing `ColorMapper::GetIndexFrom555` and
`ColorMapper::GetIndexFrom565` providers through the verified primary pointer.
The public `UnknownTexturePalette*` parameters and provisional prefix view stay
unchanged. Four canonical call sites and two sample call sites use explicit
receiver casts; no wrapper, alias, duplicate provider, storage or compiler flag
was added.

| Existing provider | Retail extent | Result |
| --- | --- | --- |
| `ColorMapper::GetIndexFrom555` | `0x004de280`, 7 bytes | Receiver + `0x710` |
| `ColorMapper::GetIndexFrom565` | `0x004de290`, 7 bytes | Receiver + `0x8710` |

Both complete functions in `Quantize.cpp` are strict VC6 matches. Their shared
accessor shape is insufficient evidence of receiver identity by itself.

## Receiver and argument evidence

The raw `ColorMapper` primary vtable `0x557658` points to COL `0x55eaf0`, with
object offset zero and literal type name `.?AVColorMapper@@`. `Palette8` has
primary vtable `0x555aa4`, COL `0x55ded0`, offset zero and its own literal type
name. Their base metadata places `BaseObject` at zero; the actual allocation
and propagation chain establishes which pointer reaches these calls:

* Strict `ColorMapper::LoadShared` (`0x4dddd0`, 201 bytes) allocates `0x18b10`
  bytes and calls the strict 689-byte constructor at `0x4ddf40`. The constructor
  writes the primary vptr `0x557658`.
* Strict `Palette8` construction (`0x4b6c00`, 630 bytes) stores the unchanged
  result of `LoadShared` at receiver `+0x708`.
* Strict `TerrainLoadTexture` (`0x50a590`, 1,442 bytes) loads that same field
  into its local pointer and passes it as argument eight to texture virtual
  slots four and five. The receiver and palette pointer receive no adjustment.
* The primary `PCTextureMap` table routes those slots to `CreateSurface` and
  `LoadSurface`. Retail `CreateSurface` stores argument eight at receiver
  `+0x2c`; its admitted implementation remains reviewed nonexact. This raw
  store observation is separate from the strict claims.
* Strict `PCTextureMap::LoadSurface` (`0x4c6080`, 2,129 bytes) forwards the
  incoming palette as argument ten to `ConvertPixels`. Its pending
  `PixelFormatBytesPerPixel` cdecl argument explains the stack offset and
  52-byte cleanup; it is not another palette field or a base adjustment.
* Strict `ConvertPixels` (`0x4d1d20`, 1,964 bytes) forwards that argument as
  argument eight to each indexed converter. The converter loads it directly
  into ECX before calling the getter.

The retained test checks these raw instructions, complete strict bodies,
primary COL/type bytes and the actual header layouts. `ColorMapper` has the
full `0x18b10` allocation, including both lookup tables. The provisional
`UnknownTexturePalette` view describes only a prefix through `0x8710`; this
change does not claim complete type equivalence or justify allocating that
smaller view as a `ColorMapper`.

## Complete caller preservation

| Caller | Retail getter calls |
| --- | --- |
| `Convert24To8`, `0x4d0aa0` / 232 bytes | `0x4d0afa`, 555 |
| `Convert565To8`, `0x4d0b90` / 171 bytes | `0x4d0be4`, 565 |
| `Convert555To8`, `0x4d0c40` / 242 bytes | `0x4d0cac`, `0x4d0ce1`, 555 |
| Sample `Halve8`, `0x4cee30` | `0x4cee54`, 555 |
| Sample `DitherConvert`, `0x4cf2a0` | `0x4cf2c2`, 555 |

The three canonical callers remain complete strict matches. The sample
receivers come from their unchanged seventh and eleventh arguments. All four
sample function bodies remain nonexact and unadmitted; their complete emitted
extents, contents and relocation destinations are protected by the whole-object
pin rather than selected matching windows.

Before and after compilation at the same private source paths preserves all
**56 nondebug sections and 153 relocations** across the canonical and sample
objects under exactly two external-name substitutions:

```
?GetIndexFrom555@UnknownTexturePalette@@QAEPAEXZ
    -> ?GetIndexFrom555@ColorMapper@@QAEPAEXZ
?GetIndexFrom565@UnknownTexturePalette@@QAEPAEXZ
    -> ?GetIndexFrom565@ColorMapper@@QAEPAEXZ
```

That proof has no filename exclusions. The retained portable pins normalize
only the two enumerated source filename literals and their relocation names,
so the same proof runs in another checkout. All other bytes, section flags,
external names and defined relocation section/offset destinations remain exact.
The include is confined to the two implementation files, after `TextureMap.h`.

The isolated real build compiles **254/254** translation units and reduces
unresolved names **243 to 241**, removing only the two obsolete getter names.
All 254 prior game objects preserve **10,671 sections and 45,720 relocations**
under those substitutions, with no filename exclusions.

## Native validation

`tests/test_palette_getter_connections.py` passes **103 strict checks**, both
whole-object pins, all six call sites and **8,139 native cases**, including
**22 access-fault comparisons**.

The test extracts the unchanged text of the actual two getters and scalar
`Convert24To8` / `Convert565To8` functions into a temporary native TU. Every
complete body is independently strict and identical in bytes and named
relocations to its actual canonical TU. The extracted object has no storage or
dynamic initializer. Its only unresolved game dependency, `DitherConvert`, is
an explicit test-only fail-fast boundary; every exercised call uses dither zero.
No missing converter is replaced with a successful stub.

Native execution compares these source bodies with the corresponding complete
functions and getters in a hash-pinned private retail mapping. The reference
instructions and call destinations are not patched. The matrix contains:

* 13 getter checks: both table offsets at six raw receiver bit patterns,
  including null and address wrap, plus prepared-storage pointer checks.
* 8,064 scalar cases: eight deterministic byte seeds, both formats, seven
  widths, four heights, three positive/zero/negative stride choices and six
  output placements. Placements include source overlap in both directions and
  writable lookup-table overlap. An independent sequential byte/index oracle
  checks the whole prepared arena, return value and absence of exceptions.
* 60 guard cases: ten invalid or partially accessible source, destination and
  table placements for each format, with positive size, zero width and zero
  height. Source/reference outcomes compare the access kind/address and all
  accessible memory, including writes before a fault.
* Two additional fault oracles: exactly one output byte is written before the
  next byte reaches a guard page, and simultaneous invalid operands fault on
  the source read first.

The independent oracle uses byte arithmetic, not a call to either tested
converter. The complete source arena and guard-page prefix are compared, so
unexpected input/table writes are visible. Only access violations are caught;
ordinary cases must complete and return one. These functions use integer
operations, not floating-point conversion or rounding helpers.

Raw getter bit patterns test the observed VC6 ABI and wrapping address
calculation; they do not establish legal C++ object lifetimes. Native storage is
prepared directly without constructors. The finite matrix does not exercise
resource loading, real texture backends, dither behavior, arbitrary malformed
sizes or full startup/racing. `Convert555To8Fast` remains a genuine missing
dependency; neither that path nor the partial halver/ditherer is presented as a
working native implementation by this regression.

```powershell
$env:PYTHONPATH='.;tests'
$env:VC6_ROOT=(Resolve-Path work/vc6sp3-disc).Path
$env:MCM2_EXE=(Resolve-Path work/game/mcm2.exe).Path
python -m unittest test_palette_getter_connections -v
```
