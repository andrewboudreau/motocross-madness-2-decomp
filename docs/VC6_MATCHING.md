# VC6 SP3 matching checks — 2026-09-29 and 2026-09-30

The private VC6 SP3 compiler now runs natively on Windows. No further compiler
download is needed for this matching pass. The target executable has SHA-256
`31fde4cc686a5ee89ef9095b90235325b195596867ecacefe511263e1509b874`.

## Results

Using the existing readable candidates and `vc6_o2_ml_g6`:

| Corpus | Result | Scope of the comparison |
| --- | --- | --- |
| Hand-written smoke samples | 19/19 exact | Complete function bodies; zero masked relocation bytes |
| Generated easy probes | 39/39 strict exact | 37 without relocations; two global-load probes resolve all address bytes |
| Calibration | 10/16 pass | Seven without relocations; constructor, destructor core and deleting wrapper resolve all relocation bytes |

These are function-level results, not a whole-game rebuild. Semantic names and
translation-unit ownership retain their existing evidence tiers. The masked
comparisons in calibration do not prove relocation destinations. The main gate
now requires SP3 identity and successful compiler execution, manual matches with
no masked bytes, and strict generated-probe results. Calibration remains a
separate, partially matching corpus.

The two global-load addresses were re-decoded from the current executable and
bound to the generated external symbols before applying the COFF relocations:

| Probe VA | Observed load address | Strict result |
| --- | --- | --- |
| `0x0040c880` | `0x00550774` | 7/7 bytes, zero ignored |
| `0x0052a5b0` | `0x00550484` | 9/9 bytes, zero ignored |

These bindings establish the operands of these two generated probes, not the
original global variable names or library identity. Stale address/return-pop
evidence and unexpected external symbols fail closed. Legacy masked metrics
remain in the report for comparison, but cannot accept a generated probe.

After updating to main `e464030`, the private installer verified the pinned ZIP
and its 1,463 payload files into a cache outside the checkout. The new runner
passed its real native CL readiness compilation and full gate with the local
matching changes. A negative integration check changed one generated probe's
address evidence: readiness remained true, strict matching failed, and the
`--full-gate` process exited 1. Restoring the evidence restored a passing gate.

The ten passing calibration candidates are the BaseObject constructor, destructor core and deleting wrapper,
UIStatic slot 30, UIMultiState slots 34–37, and FollowCamera slots 63 and 70.
Default-profile failures are BaseObject Release;
UIControl slots 61/62; and FollowCamera slots 69/71/72. A mismatch
alone does not establish a different compiler: source shape and flags remain
under investigation. Linux/Wine execution was not tested in this pass.
Without `/G6`, both `/O2 /ML` and `/O2 /MT` reach 14/16; only FollowCamera
slots 69 and 71 remain nonmatching.

## BaseObject follow-up — 2026-09-30

The constructor now assigns the field in its body:

```cpp
BaseObject::BaseObject() { refCount = 1; }
BaseObject::~BaseObject() {}
```

VC6 emits the vptr store before the field store with this form, matching the
observed order at `0x00405120`. The initializer-list form emitted the stores in
the reverse order. This changes no known behavior for the scalar field, and
does not establish the spelling of the original source.

Both `samples/base_object/BaseObjectSpecialMembers.cpp` and the existing promoted
`src/reconstructed/BaseObject.cpp` were compiled independently. The constructor
matches 16/16 bytes and the destructor core at `0x00405150` matches 7/7 bytes,
with zero ignored bytes after binding `??_7BaseObject@@6B@` to `0x005507c0`.
The binding is supported by BaseObject's primary RTTI/vtable record
(`object_offset = 0`) and the decoded vptr stores in
`analysis/vtable_write_xrefs.json`. The scalar deleting-destructor evidence also
identifies `0x00405150` as its direct destructor target.

The reusable bindings are in `samples/base_object/special_member_bindings.json`.
`tools/match.py --bindings` now applies the relocations and uses the strict
comparison for its exit status. A deliberately incorrect vtable binding was
rejected for both independently compiled sources even though the legacy masked
comparison reported success. Calibration and profile summaries prefer the
strict result when one is available.

```powershell
python tools/compile.py samples/base_object/BaseObjectSpecialMembers.cpp -o work/base-special.obj --compiler vc6 --vc6-root $env:VC6_ROOT
python tools/match.py --exe work/game/mcm2.exe --target-va 0x00405120 --target-size 16 --obj work/base-special.obj --symbol '??0BaseObject' --bindings samples/base_object/special_member_bindings.json --json
python tools/match.py --exe work/game/mcm2.exe --target-va 0x00405150 --target-size 7 --obj work/base-special.obj --symbol '??1BaseObject' --bindings samples/base_object/special_member_bindings.json --json
```

### Deleting wrapper and Release

The generated scalar deleting destructor has no COFF auxiliary function-size
record, but its VC6 `S_GPROC32_ST` CodeView record independently supplies a
30-byte length. Its paired SECREL/SECTION relocations identify the exact COFF
function symbol. The parser now reads that length; the target size and trailing
NOPs never determine the boundary. This resolves the previous 32-byte-section
versus 30-byte-function mismatch without changing C++ or compiler output.

The wrapper at `0x00405130` matches 30/30 bytes after resolving its destructor
call to `0x00405150` and its delete call to `0x004a30c0`. These destinations are
decoded directly in `analysis/deleting_destructors.json`. Independently changing
either binding makes the comparison fail for both the sample and complete
BaseObject source.

Release now keeps the result in a local across virtual deletion:

```cpp
int BaseObject::Release() {
    int remaining = refCount;
    if (remaining != 0) {
        remaining = --refCount;
        if (remaining == 0)
            delete this;
    }
    return remaining;
}
```

This matches all 32 bytes at `0x00405170` under `/O2` without `/G6`, with no
relocations. The zero-count path returns zero; a nonzero count is decremented
and the saved result survives deletion. Method/member spellings and signedness
remain provisional. The prior nested return/constant-zero form emitted a
different epilogue. `/G6` still produces a different instruction schedule.

All six bodies emitted from `src/reconstructed/BaseObject.cpp` were independently
matched under `vc6_o2_ml`: constructor 16 bytes, wrapper 30, destructor core 7,
AddRef 8, Release 32, and GetRefCount 4. All comparisons ignore zero bytes.
Paired compilations with and without `/Z7` produced identical complete code
sections and relocation destinations for both tested CPU profiles.

```powershell
python tools/compile.py src/reconstructed/BaseObject.cpp -o work/base-object-full.obj --compiler vc6 --vc6-root $env:VC6_ROOT --profile vc6_o2_ml
python tools/match.py --exe work/game/mcm2.exe --target-va 0x00405130 --target-size 30 --obj work/base-object-full.obj --symbol '??_GBaseObject' --bindings samples/base_object/special_member_bindings.json --json
python tools/match.py --exe work/game/mcm2.exe --target-va 0x00405170 --target-size 32 --obj work/base-object-full.obj --symbol 'Release@BaseObject' --bindings samples/base_object/special_member_bindings.json --json
```

The full six-profile matrix preserves 39/39 strict generated and 19/19 manual
matches for every `/O2` profile. Both `/ML` and `/MT` reach 14/16 calibration
matches without `/G6`, versus 10/16 with `/G6`. This is profile evidence, not
proof of original per-translation-unit flags, so the default is unchanged.
Private reports are under `work/baseobject-calibration/next-slices/`:
`all-baseobject-proof.json`, `wrong-wrapper-bindings.json`, and
`profile-results/matrix.json`. The two remaining best-profile failures are
FollowCamera slots 69 and 71.

Validation: native SP3 readiness and the full gate passed; the unit suite passed
204 tests with 5 skips, and static checks passed. Negative tests cover mismatched
call bindings, conflicting/truncated CodeView records, invalid extents, and
target-independent length handling. `clang-cl` was unavailable in this Windows
session, so the earlier clang baseline was not re-executed. The regenerated
legacy manifest does not ingest these VC6 reports; the explicit proof files
above remain the source for this pass's VC6 results.

## What simple matching looks like

The existing BaseObject probe expresses the directly observed field behavior:

```cpp
int BaseObject::AddRef() {
    return ++refCount;
}

int BaseObject::GetRefCount() {
    return refCount;
}
```

RTTI confirms the class; the field is at `this+4`. The method and member names
remain semantic inferences. Both methods match without relocation masking:

| Candidate | Target VA | Compiler and target bytes |
| --- | --- | --- |
| AddRef | `0x00405160` | `8b 41 04 40 89 41 04 c3` |
| GetRefCount | `0x00401940` | `8b 41 04 c3` |

For AddRef these instructions load the field, increment EAX, write it back, and
return the new value. No C++ changes were needed to obtain these matches.

## Function lengths and alignment

VC6 places these 8-byte and 4-byte functions in 16-byte COFF sections, with
trailing NOP alignment. The previous parser compared the whole section against
the function and reported a length mismatch despite matching instruction bytes.

VC6 profiles include `/Z7`, which emits function-definition auxiliary and
CodeView records. The parser uses auxiliary `TotalSize`, defined by the
[Microsoft PE/COFF specification](https://learn.microsoft.com/en-us/windows/win32/debug/pe-format#auxiliary-format-1-function-definitions).
For generated functions lacking that record, it also supports legacy VC6
`S_LPROC32_ST`/`S_GPROC32_ST` procedure lengths, using the layout in Microsoft's
[CodeView definitions](https://github.com/microsoft/microsoft-pdb/blob/master/include/cvinfo.h).
The debug record must have paired i386 SECREL/SECTION relocations to the same
symbol, zero address addends, and a unique exact COFF function definition.
Display names are not used for identity. Conflicting metadata, zero lengths,
out-of-bounds extents and truncated records are rejected. Unsupported debug
generations or associations supply no length; objects without supported
metadata retain their section/next-symbol extent. NOPs are never guessed away,
and the target length does not determine the candidate length.

Paired compilations with and without `/Z7` of 12 sample source files had identical
code sections, including padding, and identical relocation destinations. Local
compiler label names were compared by section and offset because debug metadata
can renumber them. `/Z7` is measurement metadata, not evidence that the game used
that flag. For example, FollowCamera slot 71 has a 192-byte declared extent
including its jump table; it is not accepted by clipping to a 171-byte target.
Retail has the same table after the code, so its calibration target is now the
full 192 bytes (see below).

## FollowCamera slots 69 and 71 — 2026-09-30 (Linux/Wine)

These were the two calibration targets still failing without `/G6` after the
BaseObject follow-up above.

**Slot 69 (`0x466a80`, 65 bytes): source shape.** Retail forms the `+0x2A8`
cache address before calling slot 57 and copies the three dwords straight from
the returned hidden buffer; there is no named temporary:

```cpp
void FollowCamera::UnknownVirtualSlot69() {
    CameraValue12* cached = reinterpret_cast<CameraValue12*>(reinterpret_cast<char*>(this) + 0x2A8);
    *cached = UnknownVirtualSlot57(0);
    UnknownVirtualSlot43(*cached);
}
```

This matches 65/65 bytes (no relocations) under `vc6_o2_ml`; `/G6` gives 55%.
The previous `CameraValue12 value = ...; *cached = value;` form kept the copy
in three extra registers and saved `ebx`.

**Slot 71 (`0x466e50`): target extent.** The code already matched. VC6's
declared extent is 192 bytes: 171 code bytes, one alignment NOP, and a
5-entry jump table. Retail has the same layout; every table entry
(`0x466e77`, `0x466e83`, `0x466e8f`, `0x466e9b`, `0x466eb3`) and the table
reference at +35 resolve to the retail addresses when each `$L` label is placed
at `0x466e50 + label offset`, and all other bytes are equal. The calibration
target size is therefore 192, not 171. (The comparison runner still masks
these six relocations; the label resolution above was checked separately.)

With both changes, `tools/run_calibration.py`:

| Profile | Calibration exact |
| --- | ---: |
| `vc6_o2_ml_g6` | 11/16 |
| `vc6_o2_ml` | 16/16 |
| `vc6_o2_mt` | 16/16 |

The generated (39/39) and manual (19/19) corpora are unchanged in both profiles
(`tools/vc6_profile_matrix.py`). The five `/G6` misses — Release, UIControl
61/62, FollowCamera 69/72 — differ only in instruction selection and
scheduling; explicit `/G5` behaves like VC6's default, and `/O1` fails the same
targets. No calibrated target prefers `/G6`.

`vc6_o2_mt` gives the same 16/16. On this evidence the `tools/compile.py` VC6
default is now `vc6_o2_mt` (see `VC6_PROFILE_MATRIX.md`); the earlier "default
is unchanged" note above predates the slot 69/71 results.

## Reproduce

From a checkout with the private input bundle extracted (see TOOLCHAIN.md), in
PowerShell:

```powershell
$env:PYTHONPATH = '.'
$env:VC6_ROOT = (Resolve-Path 'toolchains/vc6sp3').Path
python tools/probe_vc6.py --vc6-root $env:VC6_ROOT
python tools/compile.py samples/base_object/BaseObject.cpp -o work/base-object.obj --compiler vc6 --vc6-root $env:VC6_ROOT
python tools/match.py --exe work/game/mcm2.exe --target-va 0x00405160 --target-size 8 --obj work/base-object.obj --symbol AddRef@BaseObject --json
python tools/match.py --exe work/game/mcm2.exe --target-va 0x00401940 --target-size 4 --obj work/base-object.obj --symbol GetRefCount@BaseObject --json
python -m unittest discover -s tests -p test_coff.py -v
```

After generating analysis, run `tools/vc6_gate.py --vc6-root $env:VC6_ROOT` for
both smoke corpora and calibration. Local evidence from this pass is stored in
`work/vc6-simple-matching/gate.json` and `metadata-codegen-check.json`; those
generated/private-input reports are not committed. The subsequent strict pass
is recorded in `work/vc6-simple-matching/strict-gate.json`; private-runner
readiness/full-gate and negative-check reports are under `work/private-runner-*.json`.

## Documentation review

- README, TOOLCHAIN, ROADMAP, RECONSTRUCTION_STATUS and AGENTS now point to the
  executed SP3 checks instead of treating compiler acquisition as the next task.
- FIRST_PASS_RESULTS is retained as a historical v0.5 snapshot, with its older
  counts explicitly marked. CHANGELOG remains historical by design.
- Allocation, provenance, category and class-model documents still describe
  separate evidence workflows. Their dated non-execution statements concern
  those earlier passes; these function tests do not establish CRT attribution,
  source ownership, Linux execution, or GitHub CI results for them.

No evidence document was deleted solely because the compiler became available.
