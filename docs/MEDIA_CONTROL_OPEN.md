# Opening a movie stream

`samples/gameui/MediaControlNearMisses.cpp` contains the complete readable
`MediaControl::Open` candidate. The source and bindings are unchanged from
`883a92cc`; the file already contains only this operation. `GameUi.cpp`
needs this missing provider for video controls. `MediaControl` is an RTTI
class; the method name and original translation-unit ownership remain
provisional.

## Extent and dependency evidence

The full retail body is **646 bytes**, `0x004a2560..0x004a27e6`, including
both `ret 0x10` tails at `0x004a27c2` and `0x004a27e3`. Ten NOPs precede
the next function. Every direct branch targets a decoded instruction in
that complete body. There are two direct game calls and seventeen indirect
calls: two Win32/COM import calls, fourteen stream/surface interface calls,
and the receiver's virtual Release call. Their exact sites and slot operands
are checked by the test. There is no local EH registration or cleanup
funclet.

VC6 SP3 `/O2` emits **656 bytes**, with **53 matching positions** after all
relocations are resolved. This is explicitly nonexact; the complete body
is tested without shortening its extent. The standalone object defines
only Open, has no CRT initializer and imports only its two game helpers,
two APIs and seven GUID constants.

The two direct helpers, `GameObject::SetRenderContext` (12 bytes) and
`MediaControl::IsDirectShowInstalled` (51 bytes), are strict in the actual
source-build objects from `build-hbhhwx9h`. Six GUIDs resolve through the
existing VC6 SDK libraries, and IID_IDirectDrawSurface4 through the canonical
`DirectXGuids.cpp` provider. The test links those actual providers and checks
all seven complete 16-byte values against retail, both in the linked PE and
at execution, including readonly section attributes. It does not define
test-only substitute GUID values.

The unchanged source check compiles the baseline and current source with
current headers. Both complete nondebug sections, including all code and
relocation destinations/types/addends, are identical. Source-text comparison
normalizes only Git's LF versus the working tree's CRLF; there are no source
edits or filename-storage exceptions. Private reproduction/evidence:
`work/game-build/check_media_open_source.py` and
`work/game-build/media-open-source-proof.json`.

## Functional build review

The functional build explicitly admits this standalone source as
`reviewed-nonexact`. Fresh build `build-kioe61ys` compiles **211/211** source
units and reduces unresolved names from **439 to 438** against
`build-hbhhwx9h`: only `MediaControl::Open` is removed, with no new missing
names. The game still does not link or run; this closes the movie-opening
provider without claiming integrated playback. The isolated comparison is
recorded in `work/game-build/media-open-link-delta.json`.

Open assigns the render context, checks DirectShow availability, creates a
multimedia stream, initializes it, adds video and optional audio, converts
the filename and opens it, then obtains the video and DirectDraw stream
interfaces. It reads the video format, creates the sample surface and stream
sample, obtains the surface interface, and creates a frame surface using the
current render surface's description. On success it stores the frame
rectangle and returns itself. Failure dispatches the receiver's virtual
Release and returns null.

The instructions distinguish operations that require HRESULT zero from
those accepting any nonnegative HRESULT. The audio-stream call also accepts
`0x80040256`; neighbouring failure codes take the failure path. The extra
multimedia-stream AddRef is retained. Callback/owner fields and intermediate
interface outputs are stored at the same points as retail; no automatic
cleanup absent from the body is added.

`tests/test_media_control_open_behavior.py` runs **11,125 native scenarios**
against the complete hash-pinned retail function and the actual separately
compiled `/O2` candidate. The independent fixture uses `/Od`. Only the two
audited direct calls in the private reference mapping and the two import
operands in the private executable are redirected. The compiled candidate
object is not rewritten. No retail dispatch or fixture enters the production
build.

All 0x80 receiver bytes, two render targets and displays, twenty distinct
interface objects and both filename stores are compared at every dependency
boundary and after execution. Ordered transcripts retain receiver and output
pointer identities, GUID identity, incoming output values, HRESULT-dependent
state, modeled reference counts, complete 108/124-byte descriptors, the full
initialized 260-character wide path, return values and exception/fault data.

Coverage includes:

- Ten HRESULT values at every result boundary, including zero, positive
  success values, signed extremes, the accepted no-audio value and adjacent
  failures. DirectShow absence and ignored AddRef/conversion return values
  are also checked.
- Five interface alias patterns, distinct output banks, existing versus null
  member outputs, success with null or unwritten outputs, and output writes
  on failure. These distinguish reloads and cached receiver/interface values.
- Varied dimensions including zero and signed extremes, paths from zero to
  259 bytes, and exact descriptor initialization and subsequent field updates.
- Exceptions at every boundary in four representative traces, and mutations
  of render context, display ownership, stream/interface fields, dimensions,
  frame surface and callback fields. Null target, display, DirectDraw and
  render-surface paths compare actual access violations.
- Independent expected outcomes for successful opening, missing DirectShow,
  accepted/rejected audio errors and positive HRESULT handling. Success must
  perform eight output writes and one additional AddRef, retain the supplied
  callback/owner, and pass the expected frame flags, caps and dimensions
  while preserving the other descriptor bytes.

This is a finite scripted boundary test. It does not create real DirectShow
streams, decode media, execute DirectDraw drivers, or prove live movie
playback. The virtual Release boundary records dispatch and models a remaining
reference; downstream destructor/resource teardown is not executed. The
format and surface interfaces supply deterministic complete descriptors.
The character-conversion boundary writes a complete initialized path even
when returning zero, so that case proves only the ignored return value with
written output. Actual conversion failure leaving an uninitialized local
path is outside the compared input contract. COM output failures and C++
exceptions are deliberately scripted; no arbitrary COM implementation or
invalid-pointer equivalence is claimed.

```powershell
$env:VC6_ROOT = (Resolve-Path work/vc6sp3-disc).Path
$env:MCM2_EXE = (Resolve-Path work/game/mcm2.exe).Path
python -m unittest discover -s tests -p test_media_control_open_behavior.py -v
```
