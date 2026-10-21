# PCTextureMap texture cloning

`samples/render/TextureClone.cpp` contains the complete existing readable
`PCTextureMap::Clone` candidate. `PCTextureMapNearMisses.cpp` includes that file
at the former body position. It remains a nonexact candidate in `samples/`;
original translation-unit ownership is unproven.

## Retail identity and complete extent

The pinned retail image has primary PCTextureMap vtable `0x00555fec`, object
offset zero, with slot 6 pointing to `0x004c71c0`. The complete body is **600
bytes**, through its only `RET` at `0x004c7417`. Eight following NOPs precede
`RestoreTextureSurface` at `0x004c7420`. All internal branch targets resolve
to decoded instructions within the complete extent.

The two allocation EH states use FuncInfo `0x005629b0` and unwind map
`0x005629d0`. Their separate cleanup functions are `0x0054cd00/26` and
`0x0054cd1a/26`, carrying allocation lines 976 and 986 respectively. The
complete handler is `0x0054cd34/10`. The native test validates these records,
six main direct call sites, two cleanup calls and the handler's CRT tail
jump, plus all six indirect call sites. The latter are surface Lock and
Unlock, texture CreateSurface, two ApplyColorKey sites and deleting slot 0.

Authentic VC6 SP3 emits **590 bytes**, with **333 matching positions** against
the full 600-byte retail body. Neither the extent nor the matching score is
changed to hide the nonmatch. The unchanged source retains the original
candidate's register-allocation differences.

## Dependency and ownership evidence

The current source build already supplies all six direct game dependencies.
The private `work/texture-clone26/provider-proof.json` verifies complete bytes
and resolved relocations in actual `build-ure841up` objects:

| Provider | Retail VA | Full bytes |
| --- | --- | ---: |
| Debug allocation operator new | `0x004a3010` | 70 |
| Debug placement delete | `0x004a3060` | 96 |
| PCTextureMap constructor | `0x004c5f00` | 46 |
| ManagedTexture constructor | `0x00510500` | 123 |
| ManagedTextureGroup::AddTexture | `0x0050c6c0` | 145 |
| PixelFormatBytesPerPixel | `0x00511970` | 78 |

Failure deletion also has existing complete strict providers. The primary
tables point to PCTextureMap's scalar deleting wrapper `0x004c5f30/30` and
ManagedTexture's `0x00510580/30`. Their direct destructor calls resolve to
`0x004c5f50/227` and `0x005105a0/110`, and plain delete to `0x004a30c0/96`.
The focused test recompiles both canonical class files, checks the complete
constructors/destructors/wrappers, and checks the vtable relocations through
VC6's generated vector-deleting weak external to the scalar fallback.
These are ordinary compiler-generated references, without source aliases.

ApplyColorKey is an existing strict `0x004c81d0/457` provider. CreateSurface
has the separately [reviewed nonexact provider](TEXTURE_SURFACE_CREATE.md).
The native Clone fixture scripts these virtual boundaries and deletion; it
does not execute the real constructor, container registration, allocator or
destructor chains. Provider byte proof and scripted behavior proof are
separate evidence. The existing `g_TrackGame` storage and standard CRT EH
support complete the candidate's imports.

## Functional build review

The source build admits this standalone candidate as `reviewed-nonexact`.
Real build `build-hcm18cpo` compiles 215/215 translation units and reduces
unresolved names from 387 to 386 against `build-jkbvgguc`: only Clone is
removed, with no new missing names. The game still does not link or run.

`tests/test_texture_clone_behavior.py` compiles the actual standalone candidate
at `/O2` and the private fixture at `/Od`. The fixture compares it against the
complete hash-pinned retail body and both retail allocation cleanup functions.
Only independently decoded direct dependency sites are redirected; virtual
calls use scripted receiver tables. Retail mapping and redirection exist only
in this private validation executable.

The test passes **135,464 scenarios**. It compares returned identity, completion
or exception, access-violation address and operation, all modeled object and
buffer bytes, allocation/registration/deletion state, and ordered callback
arguments with a complete state snapshot at each boundary. Descriptor storage
is checked as 124 initialized bytes. Diagnostic filenames are checked against
the expected retail/source basenames before normalization; allocation line
numbers remain observable.

Coverage includes:

- Cached and ordinary allocation, distinct manager/group identities, absent
  source surfaces, null allocation and constructor exceptions through both EH
  cleanup states.
- Lock/CreateSurface/Unlock status combinations, ignored ApplyColorKey return
  values, null dependencies, and exceptions at every reached callback. A
  CreateSurface exception preserves the observed outstanding allocation and
  locked surface; the fixture does not invent failure cleanup.
- All 65,536 packed keys for each of 555 and 565, all 256 palette keys,
  caps-derived flags, valid and unknown formats, signed pitches including
  integer extremes, and width/height/minimum arguments including zero,
  negative and extreme values passed to the scripted creation boundary.
- Callback changes to the global game, manager/group, surface, dimensions,
  format, palette/key flags and destination vtable. Distinct receiver and
  payload identities expose cached arguments versus subsequent reloads.
  Deliberate allocator/source alias cases are bounded fault probes, not a
  claim that ordinary allocation returns an existing live object.

Independent assertions check allocation size and diagnostic line, selected
manager and group registration, every CreateSurface argument, source payload,
success return identity, failed-copy deletion and constructor-unwind behavior.
An arithmetic color-key oracle checks the full packed-key sweep and palette
RGB ordering. PixelFormatBytesPerPixel executes a validation-only renamed
extraction of the complete current canonical C++ body; its full 78 emitted
bytes and relocations must remain strict before execution.

The finite script does not establish DirectDraw driver behavior, actual
allocator/container/refcount lifecycle integration, pixel conversion or a
working source-built capture path. Negative dimensions are forwarded to a
scripted callee; this does not validate those inputs through real surface
creation. Palette indices stay within their 256-entry storage. Other than
the strict pixel-size helper, dependencies remain scripted boundaries.

## Extraction preservation and reproduction

`work/texture-clone26/check_extraction.py` compiles the `799f6e9d` aggregate
and the new include-based aggregate. `extraction-proof.json` records all
three complete public function bodies and all six remaining nondebug
sections, including EH code/data and relocation targets/addends, unchanged.
The sole storage exception is the checked `__FILE__` string: the private
baseline filename stands in for the former aggregate filename, while the
extracted body names `TextureClone.cpp`. The string itself and relocations
to it are explicitly identified; unrelated strings are not normalized.
No initializer or additional executable body is introduced. The retained
test also checks standalone versus aggregate body, cleanup/handler code,
FuncInfo/unwind-map and filename storage in full.

```powershell
$env:PYTHONPATH = '.'
$env:VC6_ROOT = (Resolve-Path work/vc6sp3-disc).Path
$env:MCM2_EXE = (Resolve-Path work/game/mcm2.exe).Path
python -m unittest discover -s tests -p test_texture_clone_behavior.py -v
```

Before admission, `build-ure841up` reported Clone unresolved from CacheTexture,
ManagedTexture and PCTextureMap: one unique missing name in three translation
units. The final link delta and exact input hashes are recorded in
`work/game-build/round26-link-evidence.json`.
