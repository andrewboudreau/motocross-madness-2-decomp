# MCM2 provenance map — first static pass

Input: `mcm2.exe`

SHA-256: `31fde4cc686a5ee89ef9095b90235325b195596867ecacefe511263e1509b874`

**This is an evidence inventory, not a recovered original C++ project or a decomp completion percentage.**

## External implementation boundaries

| Module | Import slots | Package availability | Family / evidence |
|---|---:|---|---|
| `advapi32.dll` | 9 | not_in_supplied_package | windows |
| `blade.dll` | 2 | packaged | microsoft_blade_rasterizer_candidate |
| `d3drm.dll` | 1 | not_in_supplied_package | directx |
| `dinput.dll` | 1 | not_in_supplied_package | directx |
| `dplayx.dll` | 1 | not_in_supplied_package | directx |
| `dsound.dll` | 2 | not_in_supplied_package | directx |
| `gdi32.dll` | 18 | not_in_supplied_package | windows |
| `imm32.dll` | 11 | not_in_supplied_package | windows |
| `kernel32.dll` | 121 | not_in_supplied_package | windows |
| `msvfw32.dll` | 2 | not_in_supplied_package | windows |
| `ole32.dll` | 3 | not_in_supplied_package | windows |
| `shell32.dll` | 1 | not_in_supplied_package | windows |
| `user32.dll` | 54 | not_in_supplied_package | windows |
| `winmm.dll` | 8 | not_in_supplied_package | windows |
| `wsock32.dll` | 10 | not_in_supplied_package | windows |

IAT addresses in imports.json belong to this executable. They are not runtime addresses inside the DLLs.

### Blade correction

`blade.dll` contains the identity string **Microsoft Blade Software Rasterizer** at file offset `0x0003313c`.
MCM2 imports: `DirectDrawEnumerateA`, `DirectDrawCreateEx`.

Treat Blade as a separate rasterizer component with Microsoft self-identification, not as a proven Rainbow engine DLL.

Named d3drm imports: `D3DRMVectorRotate`. A vector-helper import alone does not establish use of the Retained Mode renderer.

### Other packaged DLLs

- `DSETUP.DLL`: packaged, but not in the EXE's normal import table. It may be dynamically loaded or installer/resource support; no role is assumed.
- `EBUEula.dll`: packaged, but not in the EXE's normal import table. It may be dynamically loaded or installer/resource support; no role is assumed.
- `lang.dll`: packaged, but not in the EXE's normal import table. It may be dynamically loaded or installer/resource support; no role is assumed.
- `SETUPENU.DLL`: packaged, but not in the EXE's normal import table. It may be dynamically loaded or installer/resource support; no role is assumed.
- `uilang.dll`: packaged, but not in the EXE's normal import table. It may be dynamically loaded or installer/resource support; no role is assumed.

## Inside the executable

- 3921 candidate entry addresses, seeded from RTTI vtables, direct calls, compiler artifacts, IAT jump stubs, and the PE entry point.
- 249 parsed RTTI class records; 271 concrete vtables.
- 111 embedded source/header paths; 107 referenced by instructions reached from the current candidates.

| Ownership evidence | Candidate entries |
|---|---:|
| linker_glue | 33 |
| rainbow_project_associated | 460 |
| rainbow_project_candidate | 321 |
| unknown | 3107 |

These are evidence labels, not mutually complete original-library allocations. Class/filename hints are not confirmed authorship.

| Code role (independent of ownership) | Candidate entries |
|---|---:|
| import_thunk | 33 |
| operator_delete_candidate | 1 |
| ordinary_or_unknown | 3714 |
| scalar_deleting_destructor | 145 |
| this_adjustor_thunk | 28 |

Canonical deleting destructors and adjustor thunks should be reproduced through C++ declarations and the compiler. They are not discarded from the matching scope.

### Runtime candidate

`0x004a30c0` is the common delete target of 145 recognized wrappers. The operator-delete role is a strong hypothesis; Microsoft CRT identity remains **unverified** until library/signature comparison.

## Coverage accounting

Executable-section file-backed virtual bytes: **1368518**.
Union of instructions reached from candidate entries: **1135469** bytes.
Bytes not reached from the current candidates: **233049**.

The remainder may include undiscovered code, alignment, inline data, switch tables or decoding gaps. The reached set can also contain false-positive code candidates. Neither number is an authorship or decomp-progress percentage.

**Exact original translation-unit ranges established: 0.** Source paths are anchors and lower-bound filename evidence, not a complete project file list.

## Reproducibility and limitations

Decoder: `GNU objdump (GNU Binutils for Debian) 2.44`. All decoded instruction bytes were checked against the supplied image.

The report is regenerated directly from the binary; it does not trust cached smoke-test percentages, dossiers, or old analysis files.

- Candidate entry points are not an exhaustive or symbol-proven function inventory.
- Decoded ranges are reachable instructions, not original function extents or translation-unit ranges.
- A source string reference can come from a header or inlined code; it does not prove a complete original translation unit.
- Calling an imported API does not make the caller Microsoft code.
- Compiler-generated class glue can still be associated with the Rainbow project.
- An IAT slot is local pointer storage, not a resolved DLL function address.
- Indirect calls, COM methods, delay imports and LoadLibrary/GetProcAddress targets are not fully resolved.
- No static-library signature corpus has been applied; runtime candidates are not confirmed Microsoft CRT functions.
- DLL identity strings are self-reported metadata, not authenticated vendor signatures.
- No decomp completion percentage or total Rainbow function count is established.

## Next evidence to collect

1. Compare runtime candidates with privately supplied VC6 library objects, checking relocations and callees rather than just masking them.
2. Recover additional function/CFG boundaries with a disassembler project and switch-table analysis.
3. Resolve COM interface identities and dynamic DLL loads; preserve unresolved dispatches until evidence exists.
4. Promote source ownership only where independent evidence agrees. Preserve shared code and multiple possible source origins.

## Format references

- Microsoft PE/COFF: https://learn.microsoft.com/en-us/windows/win32/debug/pe-format
- COM interface layout: https://learn.microsoft.com/en-us/windows/win32/com/interface-pointers-and-interfaces
- Link inputs: https://learn.microsoft.com/en-us/cpp/build/reference/link-input-files
