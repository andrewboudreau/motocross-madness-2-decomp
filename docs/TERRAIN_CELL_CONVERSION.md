# Terrain cell conversion

Terrain::QueryGround now selects cells using the active x87 rounding mode.
The old integer cast truncated: with nearest rounding, a scaled coordinate of
1.25 became cell zero instead of retail's cell one. Retail subtracts 0.5,
stores binary32, then executes signed 32-bit FISTP at `0x00507c86` and
`0x00507ca0`. The source repair preserves that cell-selection behavior under
masked floating-point exceptions; it does not establish full ground sampling.

The ordinary C++ helper decodes the stored float and applies nearest/ties-even,
downward, upward or toward-zero rounding. It handles signed zero, subnormal
values, signed 32-bit overflow and NaNs. A volatile signed 64-bit conversion
through the authentic CRT supplies the sticky exception flags; a quiet NaN
supplies invalid for values outside the signed 32-bit range. The control word
is restored by the CRT. Unmasked exception timing and restoration on a trap
remain unverified.

Private VC6 `/QIfist` calibration emitted a 64-bit FISTP for ordinary integer
casts, including pointer and volatile variants. Its 32-bit overflow behavior
differs from retail, so that flag was not adopted. No assembly, copied code or
retail dispatch is used in production.

## Native evidence and limits

`tests/test_terrain_cell_conversion.py` passes **278,411 native cases** against
the hash-pinned retail image. It checks all four rounding modes and three x87
precision modes, six scale factors, edge values and 200,000 deterministic
raw-bit inputs. Cases include boundary neighbors, ties, subnormals, infinities,
NaNs and signed 32-bit limits, optional normals and absent height fields.
Independent expected outcomes protect the 1.25 regression, tie-to-even cases
and positive-infinity rejection.

The fixture executes the actual complete compiled QueryGround and helper
bodies, requiring their bytes and named relocations to equal those extracted
from the production object. It compares state, the requested cell indices,
normal request, floating-point status and control word. Its first
GetCellCorners callback stops execution before interpolation. Later helpers
fail immediately if reached. Thus height interpolation, normal/material
selection and complete QueryGround behavior remain unverified.

QueryGround remains nonexact: **3255 emitted bytes** against **2963 retail
bytes**, plus a **370-byte** local conversion helper without a claimed retail
identity. The former candidate was 3321 bytes. Unresolved helper bindings
prevent a complete strict comparison. Its non-gating relocation-masked score
changes from 65/2711 to 54/2707 matching positions; this is an intentional
behavior repair, not a byte-preservation claim for that partial function.
All 22 previously strict Terrain targets remain strict.

Load's 2117-byte body is also unchanged. The additional local helper renumbers
its EH relocation target from `$L4895` to `$L4924`, both section 29 at offset
182. The complete object comparison confirms identical bytes and relocation
destinations before the named fingerprint is refreshed. The affected terrain
regression passes on rerun. Subsequent [sampling-provider connections](TERRAIN_SAMPLING_CONNECTIONS.md)
preserve both bodies, refreshing named fingerprints for canonical callees and
Load's next compiler-local EH label.

The adjacent GroundFogableObject constructor now has a canonical source
definition at the confirmed `0x004aae20`: its complete **nine bytes** are strict
exact. Terrain and EcoSystem retain the shared four-byte base at `+0x2c`.
The field name and reconstructed source grouping remain provisional. The
isolated real build `build-5te86usb` compiles **206/206 units**, removing only
that missing constructor: **477 to 476**, with no new unresolved names.

```powershell
$env:PYTHONPATH = '.'
$env:VC6_ROOT = (Resolve-Path work/vc6sp3-disc).Path
$env:MCM2_EXE = (Resolve-Path work/game/mcm2.exe).Path
python tests/test_terrain_cell_conversion.py -v
```
