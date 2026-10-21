# FollowCamera camera-file loader

`FollowCamera::LoadCameraFile` supplies the flyby camera resource path reached by
`KrustyBikeCamera::SetupFlyby`. The readable standalone candidate remains in
`samples/camera/FollowCameraLoadFile.cpp`: its behavior has finite native evidence,
but it is not byte-exact and does not establish original translation-unit ownership.

## Identity and complete extent

The complete retail method is **0x004650e0, 1,014 bytes**. Its final `ret 4` spans
0x4654d3..0x4654d5; the following ten NOPs precede the next function at 0x4654e0.
The retained regression decodes all instructions, all 18 direct calls and every
branch destination. No indirect call occurs in this method.

The independently strict 81-byte `KrustyBikeCamera::SetupFlyby` at 0x497d90 calls
`SetupBikeCamera` first. On success it pushes the literal `FlybyCam.vue`, restores
its saved primary receiver to ECX, and calls 0x4650e0. Raw COL/type names and base
PMDs for FollowCamera and KrustyBikeCamera corroborate the zero-offset inherited
receiver. This is stronger than identifying a method from address proximity.
The test compiles the actual headers: FollowCamera size 0x384, frameCount+0x334,
frames+0x338, trailing-values pointer+0x33c, Matrix4 size 64, stream size 0x134,
and four-byte LoadCameraFile member pointer. Semantic field names remain inferred.

The loader counts lines with a case-insensitive six-byte `CAMERA` prefix, allocates
64 bytes per matrix and four bytes per trailing value, seeks to the stream's slice
start, and parses each matching line in a second pass. It constructs a basis from
two input vectors, reads input coordinates in x/z/y order, stores the position and
trailing scalar, then destroys the temporary stream. Successfully replacing old
frame/value pointers does not free those old arrays. The source preserves this
retail behavior rather than introducing an ownership policy.

## Two observed behavior corrections

The old aggregate candidate emitted 952 bytes with 519 resolved matching positions
against the full 1,014-byte target. Connecting its local Vector3 math values to the
existing Vec3 normalization/cross providers preserved every old raw instruction
byte and call offset under those two proved math-name changes.

That candidate nevertheless retained extra x87 precision in its first inline cross
product. Retail stores the three first-cross components to float at 0x4652e0,
0x46530d and 0x46532e, then reloads those values for the second cross. The retained
old-math negative control reproduces seed 2's second matrix m[0][0]: retail
`0xbd180a5c`, old candidate `0xbd180a57`. The correction calls the existing strict
`CrossProductCall(upHint, direction)` to materialize the three float components.
The math-only candidate emitted 976 bytes with 439 matching positions.

A progressive output guard exposed another difference: when only the first matrix
float is writable, retail next faults at frame+4, while the math-only candidate
faults at frame+8. The final source explicitly models retail's 16 matrix-store
order through a volatile output view. **This is reconstructed observable access
ordering, not evidence that the original matrix type was volatile.** Both the
fault address and every readable output-page byte are checked independently.
Normal successful matrix values remain unchanged.

The final body is **980 bytes with 429 resolved matching positions**, still compared
against all 1,014 retail bytes. The score decrease is disclosed and justified by
observed behavior; no extent is shortened and no match-score source contortion,
assembly, machine-code embedding or production retail dispatch is used. The test
retains both negative controls and requires their exact known failures under the
current harness. A failing control stops early; it does not execute every case.

## Source helpers, CRT and exception boundaries

The native test executes actual source stream construction (77 bytes) and destruction (15 bytes),
Open (957 bytes), FindArchive (57 bytes), IsValidFile (209 bytes), SetBufferSize (39 bytes), Tell (63 bytes), ReadByte (224 bytes)
and ReadLine (229 bytes). Complete extracted bodies equal their full canonical objects,
including relocation destinations, and are strict against retail. ResourceManager
FindItem (133 bytes), IsEncoded (20 bytes) and ReadXorDecoded (115 bytes) are also strict linked closure;
the plain-file/null-resource-manager cases do not exercise their archive/encoding
branches. AddArchive and WriteHeader are explicit fail-fast *unreached* fixture
boundaries; no executed game helper returns a scripted result.

The graph uses the already reviewed nonexact stream CheckHeader (176 versus 186 bytes)
and Seek (730 versus 702 bytes). Their complete source bytes and named relocations equal
the existing admitted sample. Both loader runs share these helpers; this is not an
untouched retail stream graph. Vec3Normalize (150 bytes), CrossProductCall (91 bytes),
FastInvSqrt (107 bytes) and its table initializer (74 bytes) execute as strict actual source.
The table is initialized by its real initializer before loading.

Real native VC6 file I/O and `sscanf`/`_strnicmp` execute on a temporary private
file. Full 52-byte sscanf and 257-byte strnicmp bodies are relocation-strict against
retail; their complete linked native bodies and destinations are checked in each
executable. This identifies those bodies, not the entire CRT parser, locale,
allocator or filesystem implementation. Both reference and candidate use the same
actual native CRT. The private reference alone redirects verified direct calls to
these source/CRT helpers and native EH runtime; the candidate remains separately
compiled and unpatched. Constructor/destructor adapters invoke the actual source
members and do not replace their behavior.

Retail has one constructor unwind state, no try blocks. FuncInfo 0x560cf8 points to
the 26-byte action at 0x54aba0, which calls diagnostic delete with the allocated
stream, FollowCam.cpp and line 0x3c7. Handler 0x54abba is 10 bytes and jumps to
`__CxxFrameHandler`. Progressive constructor guards execute this actual cleanup:
partial initialized bytes are preserved, diagnostic delete runs once, allocation
is released, and the camera remains unchanged. Later allocation faults/throws
have no stream-cleanup state and retain the open stream. The fixture snapshots
that state before closing the FILE solely for test isolation.

Debug allocation is the explicit scripted boundary: four deterministic 8,192-byte
blocks, selected NULL/C++ failures, and guard-page destinations. Allocation line
arguments and complete source/retail diagnostic filenames are validated before
normalizing their different path identities. Allocation/delete event snapshots
record ownership fields; final snapshots cover all prepared camera/heap/page and
preexisting-buffer bytes. They are not a transcript at every internal instruction.

## Finite native matrix

The retained test passes **1,551 cases**:

- 1,000 deterministic moderate finite files with zero through four CAMERA records.
- 12 exact unit-basis/position/scalar oracles across three x87 precision settings
  and four rounding modes, plus 384 varied files across those same 12 settings.
- One successful overwrite of preexisting frame/value buffers; three Open failures
  (missing/empty/null path), plus one failed Open preserving preexisting buffers.
- Three NULL allocations and three C++ allocation throws, one at each allocation.
- 97 progressive guards: 78 constructor boundaries, 17 matrix boundaries and two
  trailing-value boundaries. The 94 faulting cases assert write address and full
  readable-page prefix independently; the three full-size cases complete normally.
- 44 well-formed text-shape cases: case-insensitive prefixes, skipped/blank lines,
  CRLF/LF/no final newline, and long ignored suffixes across ReadLine boundaries.
- Three reached allocator mutations that alter frameCount or the matrix destination;
  independent oracles verify the subsequent allocation size, pointer and writes.

Ordinary cases require completion with no exception. Independent oracles check
frame count, exact unit basis, position (1.25, 2.75, -3.5), trailing value 4.5, zero/one matrix
slots, allocation sizes/order, ownership and preservation of old buffers. The
three NULL-allocation cases independently assert AV and ownership, while their
precise fault addresses are compared differentially. Progressive guards provide
the stronger independent access-address and partial-write oracles. The same two
runs compare all prepared storage, exception kind/access/address, final x87 status
and control word without float tolerance.

All floating-point exceptions are masked. Random components are moderate finite
decimals (approximately [-327, +327]); this does not prove arbitrary raw bits,
NaNs/infinities, overflow, unmasked exceptions, full exception register context or
resumption. Each CAMERA record supplies all ten parsed floats. Malformed/truncated
records with unwritten sscanf outputs are excluded. The second pass sees a fixed
file and bounded record count; concurrent file changes are not modeled. Archive,
encoded and write paths are excluded. Prepared raw camera storage does not prove
full camera construction, integrated flyby startup, menus or rendered frames.

## Aggregate and compiler artifacts

The standalone compiles with relative includes and no project include paths.
Its whole output is independently identical under the actual game compiler command.
It has nine nondebug sections and 37 relocations, including the normal emitted
48-byte vector-constructor iterator COMDAT and constructor EH metadata. The
compiler iterator is unchanged from the original aggregate, not claimed as a
new recovered retail method. No additional initializer or global runtime storage
is introduced by the standalone candidate.

Same-path original/corrected aggregate proof accounts for all 66 nondebug sections.
63 sections and 302 relocations remain identical, including all other methods,
static axis initializer bodies, 60-byte BSS, 16-byte CRT list, RTTI/vtables and data.
Three changes are explicit: the corrected loader section/padding, its saved-stream
EH displacement (-396 to -420), and validated diagnostic filename storage. The EH
state graph and cleanup action are otherwise unchanged. Generated label names are
compared through actual destinations, not ignored. Retained full-object pins include
public/static symbol offsets, flags and relocation destinations; only verified
filename identities are made portable. Original TU ownership is not established.

The existing clipper and visibility regressions also pin the complete camera
aggregate. Their published old fingerprints were independently reproduced before
updating only those two records for the three reviewed changes above. Each test
explicitly recognizes the new loader filename for path portability; all other
object records remain unchanged. The complete aggregate now has 337 relocations,
up from 336 because the first cross product calls the existing source helper.

## Functional build review

Independent source review and native reruns accepted this candidate for explicit
reviewed-nonexact functional-build admission. Isolated build `build-rmu8z7mx`
compiles 262/262 translation units and reduces unresolved names from 220 to 219:
only LoadCameraFile is removed, with no additions. All 261 prior production objects
(10,695 sections and 45,776 relocations) are preserved under the one independently
verified ProCircuit name connection. Independent recompilation reproduces the
new loader object's nine nondebug sections and 37 relocations under the actual
game command and retained pin. Native behavior evidence is finite; the function
remains nonexact, and the source-built game still has unrelated unresolved providers.

Reproduce from repository root with private inputs:

```powershell
$env:VC6_ROOT = "$PWD/work/vc6sp3-disc"
$env:MCM2_EXE = "$PWD/work/game/mcm2.exe"
$env:PYTHONPATH = '.;tests'
python -m unittest discover -s tests -p test_follow_camera_load_file.py -v
```
