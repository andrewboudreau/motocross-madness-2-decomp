# EcoSystem reconstruction

The shared C++98 candidate implements the normal top-level body at `0x0045aad0`.
Its callees, floating-point equivalence and historical byte matching remain
unfinished; it is not a running game implementation.

## Deliverable

- `samples/ecosystem/EcoSystemPass.h`: a C++98 algorithm covering every reviewed
  top-level branch, iteration, list update, and category/timer exit.
- `EcoSystemProbe.cpp`: a 32-bit MSVC-ABI adapter using explicit field offsets,
  typed helper declarations and external globals; it compiles the same algorithm.
- `EpochProbe.cpp`: an exact clang helper from that call chain.
- `test_pass.cpp`: 33 native scenarios against the shared algorithm.
- `config/ecosystem_pass.json` and `tools/review_ecosystem.py`: hash-bound review
  blocks, literal/constant checks, RTTI slot validation, call/binding checks,
  and strict resolved-relocation comparison.

All reconstructed class/member/helper names are provisional. `RecordPrefix`,
`DefinitionPrefix`, and `FramePrefix` assert only observed offset prefixes,
not original classes, inheritance, full sizes, or original field names.

## Reviewed input and coverage

EXE SHA-256:
`31fde4cc686a5ee89ef9095b90235325b195596867ecacefe511263e1509b874`.

The target is EcoSystem primary vtable `0x00552508`, slot 12. Its reviewed body
is `0x0045aad0..0x0045add3` (exclusive end): **771 bytes, 214 instructions**.
Ten contiguous review blocks cover that whole interval. The verifier checks
all block hashes and all **17 direct-call sites** before comparing candidates.
This is top-level review coverage, not a game-wide decomp completion metric.

## What the loop does

After the existing enable/ready gates, it optionally services a small
age/accounting object at `this+0x59c`. It obtains a total and an output count,
compares the latter as signed against `0x40000`, conditionally invokes the
trim-like helper with `total - output`, and then advances a dword at offset zero.
The record pointer is reloaded around these calls. The helper's actual body
at `0x00401040` increments a dword; it does not clear a buffer.

The routine publishes `frame + 0xac` to global `0x0059af00`, where the frame
pointer is reached through `[this+0x18] -> [+8]`. It derives two stored float
values from frame member `+0x1c0`, using these exact stored constants:

| Global | Stored representation | Value |
|---|---|---:|
| `0x00551090` | float32 | 65535.0 |
| `0x00551094` | float32 | 1.5259021893143654e-05 |
| `0x00550d18` | float64 | 0.5 |

It resets both work-list counts, resets the external iterator, and repeatedly
requests records. No linked-list layout or iterator stride is invented.
For each record:

1. Compare byte `record+0x08` with byte `this+0x599`; skip other groups.
2. Initialize two values from the low bit at `record+0x16` and byte `+0x13`,
   then let helper `0x00456890` update them through pointers.
3. Read byte `record+0x12` *after that helper* and select a definition from the
   pointer table at `this+0x58`.
4. When the returned mode is zero, obtain two metrics, form a bound and a
   three-float position, and call the geometry-test-like helper `0x0052fbb0`.
   A rejected result skips record application and both work lists.
5. Call `0x00456a10`, then re-read the record state/flag bytes to decide which
   work lists receive the record. The cached definition pointer is retained.
6. Request the next record; when exhausted, record elapsed time and restore
   the previous category.

Classification, record application, geometry testing, traversal, allocation,
and category helpers remain normal external call boundaries with reviewed
addresses. Their internals are not replaced by an invented semantic shortcut.

## Record helper near miss

`samples/ecosystem/EcoRecordNearMisses.cpp` holds a VC6 candidate for the
step 2 helper `0x00456890` (368 of 369 bytes). It writes the two caller
values: 1 and 0xff when the record lies beyond the current distance band
(`0x0059af14[EcoSystem+0x5c0]`, outer then inner `int`), first per axis and
then radially through `0x00460b50`; otherwise 0 and a 0..255 fade between
the band's inner and outer distances (0 inside it or without definition
`+0x1fc`). Only one load's scheduling differs.

## Packed fields and the geometry branch

The observed record prefix contains unsigned 16-bit components at `+0x0c`,
`+0x0e`, and `+0x10`. They are scaled using a value at
`[global 0x0059aebc] + 0x5a8`, not blindly substituted with `this+0x5a8`.
The position's middle component adds half of the second metric. That vertical
lift is distinct from the float-rounded bound used by the geometry predicate.

The x87 compare/test sequence selects the second bound for less-than, equal,
**or unordered** results. The source uses `if (!(bound > lift))`, rather than a
plain `<` comparison or an unspecified `max` helper. Native tests include NaN
cases. They do not emulate x87 exceptions, precision-control settings, or all
extended-precision intermediate values.

The native model uses a wider accumulator type and explicit float stores; the
MSVC-ABI candidate currently uses double. These choices preserve reviewed
numeric intent but do do not prove retail floating-point equivalence. Modern
compiler options and long-double behavior are not evidence of VC6 settings.

## Two work lists, and a subtle growth rule

| Role in this caller | Pointer | Count | Capacity-like field | Requested growth |
|---|---|---|---|---|
| List A, updated record state byte nonzero | `+0x3c` | `+0x5a0` | `+0x5b0` | 100 pointer slots / 400 bytes |
| List B, low flag bit below signed definition `+0x1d8` | `+0x40` | `+0x5a4` | `+0x5b4` | 20 pointer slots / 80 bytes |

Growth is attempted on **count == capacity**, not count >= capacity. The
capacity-like value is increased **only when the returned pointer differs from
the old pointer**, not whenever reallocation succeeds. The calls carry
`EcoSystem.cpp` and original line arguments `0x8dd` (2269) and `0x8ea` (2282).
Those are binary call arguments, not current repository line numbers.

The code stores the returned pointer and subsequently appends without a newly
introduced null/failure guard. That behavior is preserved, not recommended for
new application code. In-place reallocation leaves the capacity-like field
unchanged in this caller. This is a static observation, not a demonstrated
runtime bug. Tests use real backing arrays larger than their simulated
capacities so they can check the branch safely without claiming allocator safety.

## Category lifetime behavior

The global-disabled path returns before selecting. The missing-ready-member
path returns after selection and the first timer sample without a local
restore. The full path stores the elapsed 32-bit difference and restores.
The algorithm does not insert an RAII restore on the early path.

## Match state

Local clang-cl 17, i686 MSVC ABI:

| Target | Retail bytes | Candidate bytes | Fully compared matches | Applied relocations | Result |
|---|---:|---:|---:|---:|---|
| EcoSystem slot 12, `0x0045aad0` | 771 | 782 | **34/782** | 33 | **Not exact** |
| Age/generation increment helper, `0x00401040` | 3 | 3 | **3/3** | 0 | **Exact** |

The larger routine remains nonmatching. Its result
is calibration data, not a new large-function match. Every supported relocation
is applied through the explicit 26-symbol binding map; no bytes are ignored.
The helper is ordinary `++tick` C++, not inline assembly or a machine-code array.
Original helper class/method names and return-type spelling remain provisional.

The probe forces the shared template into the member-function wrapper so the
comparison does not accidentally compare a tiny delegation stub with the whole
retail routine. `/GS-` avoids modern security-cookie helpers; `/arch:IA32` selects
a baseline appropriate for the old x87 code. These are **modern probe settings**,
not claimed original compiler flags. This candidate has not yet been matched with VC6.

## Reproduce

```bash
make ecosystem           # Review the extracted owned binary, without compiling
make ecosystem-probes    # Also build and strictly compare both C++ candidates
make ecosystem-test      # Synthetic checks and native algorithm tests
```

Direct invocation from any working directory:

```bash
python3 /path/to/repo/tools/review_ecosystem.py \
  --exe /private/game/mcm2.exe --compile-probes
```

Generated JSON and report go to ignored `work/ecosystem/`. The report retains
input/config/tool hashes, all reviewed instructions, block-to-algorithm labels,
call sites, constants, normal-exit witnesses, bindings and actual match results.

## Next targets

The two record helpers at `0x00456890` and `0x00456a10` now have constrained
call contracts and downstream state uses. Reconstructing them should clarify
whether the provisional mode/state values are visibility, detail-level, or other
states; those names are not promoted yet. Terrain acquisition-site tracing
remains a separate next slice. Neither requires inventing source ownership from
the category label alone.

## Primary ABI/precision references

- https://learn.microsoft.com/en-us/cpp/cpp/thiscall
- https://learn.microsoft.com/en-us/cpp/cpp/data-type-ranges
- https://learn.microsoft.com/en-us/cpp/build/reference/fp-specify-floating-point-behavior

These document general/current ABI and floating-point distinctions, not the
original game compiler settings. Game-specific evidence is the reviewed binary.
