# Non-local texture memory probe

`UnknownDisplay::ProbeNonLocalTextureMemory` at `0x004ca5a0` is a complete,
readable candidate in `samples/render/NonLocalTextureMemory.cpp`. It remains
**nonexact**: authentic VC6 SP3 `/O2 /GR /GX /MT /Z7` emits 490 bytes versus
486 retail bytes, with 66 matching positions after resolving relocations.
The original `PCVideoCardNearMisses.cpp` includes the extracted body.

## Extent and source evidence

The reachable retail CFG covers every byte in `[0x004ca5a0, 0x004ca786)`.
Its final `ret 8` starts at `0x004ca783`; the old 484-byte target stopped
inside this three-byte instruction. Ten NOPs follow before the separate
`CreateWindowedSurfaces` entry at `0x004ca790`. The regression decodes all
reachable branches, checks the complete return, padding and next entry, and
requires the candidate's full COFF auxiliary-symbol extent. No bytes are
trimmed to obtain a match.

The debug allocation records line 1185 and the literal retail path
`D:\aardvark\VC\krusty2\PCVideoCard.cpp` at `0x005714f4`. This supports the
original source context; the new filename is an organizational extraction.
The rebuilt diagnostic filename is `NonLocalTextureMemory.cpp`; the private
comparison checks and normalizes this known diagnostic difference. Method
and field names remain reconstructed names.

## Decoded behavior

The method first writes zero through `value`. With a supplied target, it uses
that target without taking ownership. Otherwise it scans modes for the first
640 by 480, 16-bit entry, calls `SetFullscreenDisplayMode(index, 1)`, and
allocates a `0xbc4`-byte `PCRenderTarget`. Initialization receives this display,
`IID_IDirect3DHALDevice`, the current back buffer, `1`, and the current frame
buffer count. An initializer returning null takes the failure return.

The code zeroes a `0x7c`-byte surface descriptor, sets its size, calls
`FillPixelFormat` using target offset `0x28`, then requests a 128 by 128 surface
with flags `0x1007` and caps `0x20005000`. The DirectDraw call is slot 6;
`GetCaps` is surface slot 14 and `Release` is slot 2. Only a zero HRESULT from
both creation and the caps query can produce return value 1, and only when
caps bit `0x20000000` is set. The surface is released after a successful
creation. A temporary render target is deleted through virtual slot 0 on
ordinary completion; a caller-supplied target is never deleted.

The existing unusual failure behavior is preserved. With no supplied target,
a missing compatible mode or failed mode switch leaves the target null and
faults on its format load at address `0x28`. A null `value` faults on the initial
write. A null allocation is passed to the initializer; the initializer's
behavior then determines whether the call returns or faults. No recovery or
replacement value is invented. Constructor failure runs the placement-delete
cleanup. Later dependency exceptions do not gain new RAII cleanup: the
observed allocation/surface state agrees with retail, including retained
resources. These are caller-body observations, not claims about which failures
a real driver or allocator will produce.

## Functional build review

`tests/test_nonlocal_texture_memory_behavior.py` compiles the actual standalone
candidate object with authentic VC6 and links it unchanged into the native
fixture. The fixture executes that body and a hash-pinned private retail copy;
it patches only the five decoded direct body callees, the constructor-unwind
placement delete, and the native CRT exception dispatcher. Both the retail and
candidate one-state constructor cleanup maps are checked. COM calls and virtual
deletion retain their ordinary ABI. The candidate defines exactly one external
function, imports only its enumerated dependencies, and has no CRT initializer
section. Unused scripted virtual methods fail immediately.

**12,912 scenarios pass**, comparing returns, complete display/target/mode/COM
object storage, ordered call arguments and initialized descriptor data,
callback-time state, resource ownership, C++ exceptions and access violations
(including fault kind and accessed address). Coverage includes:

- Supplied or temporary targets; negative/zero mode counts; first, middle,
  last and absent compatible modes; independent width/height/depth rejection.
- Allocation failure; initializer failure with and without dependency cleanup;
  an initializer returning a different target; positive and negative failing
  HRESULTs; a successful surface call returning null; null DirectDraw storage.
- Both caps-bit outcomes with unrelated caps bits, output aliases into mode,
  display and target fields, a null output pointer, and mutating callbacks.
- Throws at mode setup, allocation, construction, initialization, format fill,
  surface creation, caps query, release and target destruction. Constructor
  unwind is also checked independently for placement delete and deallocation.

The descriptor's zeroed pre-format state, final dimensions/flags/caps,
null-output fault, ordinary success/failure results and selected cleanup paths
have independent assertions in addition to the retail differential comparison.
Only initialized callback payload is read: the caps query's incoming stack
storage and the failed surface call's unwritten output are not sampled.

The scripted boundaries are not a real DirectDraw driver, allocator or target
initializer. The finite cases establish behavior at these interfaces, not
universal equivalence for arbitrary invalid pointers, driver reentrancy, or
end-to-end startup, menus or racing. No private retail bytes or executable
reference code are used by the game build.

Existing source providers for the direct dependencies are
`PCVideoCard.cpp` (`SetFullscreenDisplayMode`), `PCRenderTarget.cpp`
(constructor/initializer), `Tgafile.cpp` (`FillPixelFormat`), `MemTag.cpp`
(debug allocation/placement deletion), and `DirectXGuids.cpp` (HAL identifier).
The CRT supplies exception dispatch. This test scripts these boundaries and
therefore does not itself establish their integrated driver behavior.

The complete standalone candidate is explicitly admitted as nonexact in
`config/game_build.json`. The real link removes its missing name without
introducing another; it still fails on other game-owned providers.

```powershell
$env:PYTHONPATH='.'
$env:VC6_ROOT=(Resolve-Path work/vc6sp3-disc).Path
$env:MCM2_EXE=(Resolve-Path work/game/mcm2.exe).Path
python tests/test_nonlocal_texture_memory_behavior.py -v
```
