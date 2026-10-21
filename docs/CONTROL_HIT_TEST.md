# Menu control pixel hit tests

## Identity and complete bodies

`UIButton::ContainsPoint(POINT,int)` and
`UIMultiState::ContainsPoint(POINT,int)` supply primary virtual slot 28 of
retail tables `0x00552ce0` and `0x00553210`, respectively. RTTI binds both
primary tables to offset zero. The canonical GameUi object demands both named
providers. The existing strict `UIControl::HitTest` at `0x004709f0` (122 bytes)
invokes slot 28 at `0x00470a50`; its whole source body remains strict.
The method names and original translation-unit ownership remain reconstructed.

The complete UIButton body is `0x004734c0..0x00473621` (**354 bytes**), including
three `ret 12` instructions at `0x004735ff`, `0x0047360e` and `0x0047361f`.
Fourteen NOPs follow. UIMultiState spans `0x00478e10..0x00478f58` (**329 bytes**),
with returns at `0x00478f36`, `0x00478f45` and `0x00478f56`, followed by seven
NOPs. All decoded branch destinations remain inside their respective complete
bodies. Neither body registers an EH frame or allocates memory. Earlier sample
headlines used the candidate sizes as denominators; those did not describe the
complete retail extents.

Both bodies cache the global target's pixel format, perform the Windows
rectangle test, scale local coordinates, obtain state texture -1, lock the
system surface, compare its pixel with the descriptor color key, then unlock.
Only a zero Lock result succeeds. The state argument is unused. UIButton copies
the rectangle before the API call; UIMultiState passes its member rectangle and
reloads its origin afterward. The latter distinction matters under mutation.
The descriptor's size is the only initialized field supplied to Lock.

The 8-bit branch reads one byte and compares the low key byte. **Every non-8
format reads a WORD**, even formats whose byte-count helper returns three or
four. Signed pitch arithmetic uses the original integer division. Unknown
formats can produce a divide-by-zero fault after a successful lock. Unlock's
result does not change the computed opacity; a fault or thrown callback does
not introduce cleanup absent in retail. The texture's surface is reloaded for
Unlock, so a callback can change the release receiver.

## Functional build review

`samples/ui/ControlHitTest.cpp` contains the two unchanged complete readable
candidates. VC6 SP3 `/O2 /GR /GX /MT` emits **328 bytes with 256 matching
positions** for UIButton and **303 bytes with 231 matching positions** for
UIMultiState, against the full 354/329-byte retail bodies. Neither is strict.
Register allocation and return-block placement differ. The source stays in
`samples/`; this is bounded functional evidence, not original TU ownership or
a claim of byte identity.

The aggregate `GameUiNearMisses.cpp` includes this same source rather than
maintaining duplicate bodies. A private before/after VC6 proof against
`7156bd98` preserves all **350 emitted functions and 732 nondebug sections**,
including data, initializers, EH information, complete bytes and named
relocations. The sole excluded storage is the independently checked
`__FILE__` string for the private baseline filename versus the original
aggregate filename. Neither extracted body uses that string. Local compiler
label spellings are normalized to section identity plus offset. Reproduction
and the exact report are retained in ignored
`work/ui-hit33/check_extraction.py` and `extraction-proof.json`.
An independent same-path comparison with the saved pre-extraction audit object
also preserves **all 733 nondebug sections and 3,912 relocations**, with no
filename exception: `work/ui-hit33/review_same_path.py` and
`same-path-preservation.json`. Defined relocation destinations are compared by
section identity and offset.

The standalone's only external requirements are the existing
`PixelFormatBytesPerPixel`, `g_TrackGame`, USER32 `PtInRect`, and CRT `__ftol` /
`__fltused`. There are no constructors, EH funclets, or initialization sections
in this object. The retained test compiles and proves these complete canonical
helpers strict, then executes source-derived copies in the native fixture:

| Helper | Retail VA | Complete bytes |
| --- | --- | ---: |
| UIControl::GetStateTexture | `0x00471fa0` | 36 |
| UIMultiState::GetStateTexture | `0x00478810` | 66 |
| UIAnim::GetCurrentTexture | `0x00472f90` | 20 |
| PixelFormatBytesPerPixel | `0x00511970` | 78 |

The first two copies have private nonvirtual names to isolate their existing
bodies; each copy independently passes the same strict full-body comparison.
The fixture also verifies all 39 bytes of the linked authentic CRT `__ftol`
against retail before using that implementation. This proves conversion code
identity rather than replacing it with host-language casts.

## Native evidence and limits

`tests/test_control_hit_test_behavior.py` builds the actual standalone candidate
as a separate object and compares it with the hash-pinned retail image in
**2,780 native scenarios**. It checks both complete CFGs, all six direct calls,
eight indirect call sites, primary-vtable roles, the complete caller, source
layouts, provider/import closure, and unchanged candidate/aggregate output.

The fixture observes PtInRect through a private executable IAT hook while
calling the real Windows API. The mapped retail IAT uses the same observer.
The six retail direct call sites are validated before private replacement:
four execute the identical authentic CRT converter, and two execute an observer
that calls the real strict pixel-format helper. Virtual GetStateTexture uses
an observer around the actual strict texture/animation helper chain. COM Lock
and Unlock are scripted ABI boundaries; only initialized descriptor input is
observed. Retail mapping and all instrumentation are validation-only.

Each boundary and final result compare whole control, dialog, game, target,
texture, animation, frame, state-table, pixel-buffer and surface storage, global
pointer identity, callback order/arguments/results, initialized descriptors,
floating-point status and exception information. Cases include rectangle edges,
transparent/opaque pixels, distinct byte/WORD keys, seven formats, nine signed
pitches, source/control-storage aliasing, nonuniform pixels with independent
coordinate/stride oracles, scale/rounding combinations, zero/infinite/NaN scales,
Lock/Unlock status values, null dependencies, sound frames, selected/focused
state, callback mutations and throws at every reached boundary. Independent
assertions cover successful opacity, rectangle rejection, lock failure,
ignored unlock failure, invalid pixel reads and divide-by-zero while a lock
remains held.

These are finite scripted surfaces and source-helper integration tests. They
do not exercise a real DirectDraw surface, actual GUI lifetime, concurrent
mutation, complete menu navigation or physical display/input behavior. No
cleanup or bounds validation is added to production to make these cases pass.

The isolated real link `work/game-build/build-rl5ao1p1` compiles all 227
translation units and removes exactly these two missing names: 337 to 335,
with no added unresolved symbols. Source, header and configuration hashes
bind the report to this composition; the game still does not link or run.

```powershell
$env:VC6_ROOT = (Resolve-Path work/vc6sp3-disc).Path
$env:MCM2_EXE = (Resolve-Path work/game/mcm2.exe).Path
$env:PYTHONPATH = '.'
python -m unittest discover -s tests -p test_control_hit_test_behavior.py -v
```
