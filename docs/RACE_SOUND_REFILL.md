# RaceSound stream refill

`samples/race/RaceSoundRefill.cpp` supplies the complete inferred
`RaceSound::RefillStream` operation at `0x004e4d10`. It fills the engine stream's
previously played half from the current sample, looping that sample or choosing
the next idle, power-band or sputter sample. The source remains nonexact and in
`samples/`; the standalone file does not establish original translation-unit
ownership.

The original aggregate `samples/race/RaceSoundNearMisses.cpp` includes this one
body. The body itself is unchanged. Its complete VC6 `/O2 /GR /GX /MT` function
is **2,038 bytes**, with **806 matching positions** against the complete
**2,032-byte** retail function. No additional compiler flag is required.

## Identity, extent and dependencies

The complete decoded body ends at `0x004e54ff`; it also returns at
`0x004e50c3`. The next function starts at `0x004e5500`. All branch targets remain
inside the function. There are eleven direct calls, no indirect calls, no local
EH registration and no floating-point instructions. The full extent includes
both half-buffer paths and all three random-selection arms in each path.

The primary RaceSound table is `0x00557710`, with COL `0x0055eb78`, object
offset zero and the literal RTTI name `.?AVRaceSound@@`. Its slot 10 is the
632-byte `Tick` at `0x004e3730`. That strict function calls the strict
2,348-byte `UpdateRacerSounds` (`0x004e39b0`) at call site `0x004e3808`;
the latter calls this refill
operation at `0x004e4013`. The operation name is inferred from its behavior and
caller. The literal `racesnd.cpp` source context is documented in
[RACESOUND.md](RACESOUND.md).

| Dependency | Retail address | Complete size | Evidence |
| --- | --- | --- | --- |
| `Sound::GetBufferPosition` | `0x004bcd40` | 57 | Strict canonical PCAudio source |
| `Sound::LockBuffer` | `0x004bd020` | 85 | Strict canonical PCAudio source |
| `Sound::UnlockBuffer` | `0x004bd080` | 56 | Strict canonical PCAudio source |
| `rand` | `0x00534c1b` | CRT boundary | Existing CRT import; scripted in the native comparison |

The candidate imports only these four names. There is one position query, two
lock sites, two unlock sites and six `rand` sites. Copies are inline machine
instructions generated from the unchanged `memcpy` expressions; the fixture
does not replace the copies.

The channel pointer is at RaceSound `+0x434`, and the sample width is at
`+0x1298`. A channel is `0x54` bytes. Its current sample pointer, byte count and
consumed offset determine whether a half can be copied directly or must wrap.
The source preserves the retail asymmetry: both paths upload
`22050 * bytesPerSample`, but the first-half path passes only
`11025 * bytesPerSample` as the first region's unlock length. The second-half
path passes the full upload length. Return values from position, lock and unlock
are ignored by the caller, as in retail.

## Preservation

A same-path aggregate object was captured before extraction. After replacing
only the definition with its include, all **106 nondebug sections and 579
relocations** are unchanged: complete bytes, section flags, initializer data,
relocation types, destinations and addends. There are no exclusions or filename
exceptions in that private before/after comparison. The private evidence is
`work/race-refill42/aggregate-proof.json`, reproduced by
`work/race-refill42/prove.py` from `before.obj` and `after.obj`.

The retained regression pins that complete aggregate object. For portability,
its fingerprint can replace only the checkout prefix of the recognized
`src/reconstructed/RaceSound.cpp` filename literal; it does not normalize code
or other payload. All 26 existing canonical calibration functions are checked
strict in both canonical and aggregate objects. The standalone refill's whole
function and relocations equal the aggregate's definition. A separate compile
through `tools.build_game.compile_source`, using the current game profile and
include inventory, must preserve every nondebug section of the standalone
object too.

## Functional build review

`tests/test_race_sound_refill.py` compiles the actual standalone candidate as a
separate object and executes it against the hash-pinned retail function. The
fixture is `tests/fixtures/race_sound_refill_compare.cpp`. The three Sound
methods are extracted verbatim from canonical source into a temporary validation
translation unit; each complete body is strict against retail and identical to
its canonical object's body and relocations. Both executions call those actual
methods. Their DirectSound virtual calls are scripted at slots 4, 11 and 19.
`GetBufferPosition` also follows the actual `g_TrackGame->soundInterface` chain.

The test checks all eleven original retail call sites and all six candidate
`rand` relocations before the private runtime redirects. Only those dependency
operands change. The complete refill logic, inline copies and state writes run
intact. No retail mapping or dependency replacement is part of the game source.

The retained test passes **58 strict checks and 4,782 native scenarios**:

- 3,780 combinations of sample widths 1/2, both halves and unsupported half
  values, six next-sample kinds, seven availability cases, and each of the
  `5512`, `11025` and `16537` cursor thresholds times the sample width, including
  the byte immediately below and above each threshold.
- 224 COM-result and buffer-alias combinations, including failure results with
  defined output values and safe bounded overlaps.
- 288 callback-mutation scenarios and 32 scripted boundary exceptions. Changes
  include channel replacement, sample width, sample kind, offset, engine/count,
  Sound/buffer identity and sample pointers. A requested boundary that is not
  reached makes no mutation or exception; these counts are scenarios, not a
  claim that every requested action occurred.
- 48 invalid-pointer scenarios, six zero-count signed-division faults, and 72
  progressive access faults at source, working-buffer or upload-buffer page
  boundaries. The latter vary valid prefixes from zero through 127 bytes.
- 256 further deterministic combinations with varied initial bytes, random
  results, engine/count selections, statuses and aliases.
- Four independently calculated ordinary/wrap byte oracles and 72 independently
  calculated random-selection oracles. These assert no exception, exact sample
  pointer/count/offset/kind transitions, buffer contents, lock state and the
  differing unlock lengths. The selection oracles also exercise ignored COM
  failures.

Every run compares all bytes of the prepared RaceSound, channel, Sound, audio,
game-prefix, source, working and upload storage, including guards. Every scripted
boundary records arguments, receiver identity, the full prepared world and
lock accounting. Fault comparisons retain the access kind/address and partial
memory changes. Divide-by-zero and progressive-copy faults must retain the
scripted acquired lock, with no invented cleanup. The method has no local
cleanup to run when a boundary exception propagates.

The reference image SHA-256 is
`31fde4cc686a5ee89ef9095b90235325b195596867ecacefe511263e1509b874`.

The isolated actual source build `work/game-build/build-puygm1gk` compiled all
241 translation units and reduced unresolved symbols from 278 to 277. Only
`RaceSound::RefillStream` was removed; no new unresolved names appeared. The
240 prior production objects preserve all 10,603 nondebug sections and 45,556
relocations under the four separately reviewed motion-name connections. This
addition closes the refill provider; the game still does not link.

### Limits

This is a finite comparison of compiled behavior over prepared raw storage;
constructors, object lifetime, real DirectSound playback and scheduling are not
exercised. `rand` results and COM behavior are scripted, including failure cases
whose output values remain defined. The lock accounting describes that script,
not all behavior of an actual device on error.

Initial inactive audio or a null DirectSound buffer can make the actual position
helper return without writing the cursor. The caller then reads uninitialized
stack storage. Those unwritten-output paths are excluded from deterministic
behavior claims; production locals are not initialized to conceal this fact.
Changing the buffer to null after a successful cursor callback is covered.

Sample widths are bounded to 1 and 2, arrays/counts and copies are prepared to
fit the fixture except for the intentional faults, and observed overlap behavior
does not establish a portable C++ `memcpy` overlap contract. The fixture starts
with masked floating-point exceptions and compares post-call control/status;
there is no floating-point arithmetic in the decoded refill. Full register
contexts, arbitrary memory, resumable exceptions, universal behavior and a
running source-built game are not claimed.

```powershell
$env:PYTHONPATH = '.'
$env:VC6_ROOT = (Resolve-Path work/vc6sp3-disc).Path
$env:MCM2_EXE = (Resolve-Path work/game/mcm2.exe).Path
python -m unittest discover -s tests -p test_race_sound_refill.py -v
```
