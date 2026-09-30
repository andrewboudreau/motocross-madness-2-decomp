# VC6 compile-profile matrix

The private-bundle proof establishes the VC6 SP3 / build-8447 toolchain family,
and the runtime proof establishes that retail MCM2 contains object code from the
supplied multithread `LIBCMT.LIB`. The remaining compiler question is therefore
mostly **project/translation-unit flags and source shape**, not compiler identity.

`/MT` is now the leading runtime-library hypothesis because the linked runtime
is `LIBCMT`, but that fact alone does not prove every game translation unit was
compiled with `/MT`: linker inputs can be overridden and projects can mix object
settings. The matrix tests the hypotheses instead of promoting that inference.

## Matrix

The configured VC6 profiles are read from `config/compile_profiles.json` and,
by default, every profile whose name begins with `vc6_` is executed. Each profile
runs three independent corpora:

- generated high-confidence easy probes;
- hand-written smoke/source candidates;
- compiler-shape calibration cases such as BaseObject special members,
  UIControl subtraction slots, UIMultiState SIB addressing, and FollowCamera.

Run it after the private toolchain acceptance gate:

    make private-ready
    make vc6-profile-matrix

or directly through the private-environment wrapper:

    python3 tools/with_private_env.py -- \
      python3 tools/vc6_profile_matrix.py

Restrict a diagnostic run to one or more profiles with repeated `--profile`:

    python3 tools/vc6_profile_matrix.py \
      --profile vc6_o2_mt_g6 \
      --profile vc6_o2_mt

The runner never falls back to clang. A mismatch is retained as evidence rather
than treated as an environment failure. A compiler invocation whose output is
not parseable as the expected JSON is recorded as an execution error.

Generated private outputs:

- `work/vc6-profile-matrix/matrix.json`: flags, raw results and summaries;
- `work/vc6-profile-matrix/REPORT.md`: compact comparison table.

## Interpretation

The report uses an exact-count tuple `(easy, manual, calibration)` only to make
high-signal differences visible. A profile with the highest tuple is **not**
automatically declared the original project setting. Exact function matches,
consistent improvements across compiler-sensitive calibration cases, and other
binary evidence should agree before promoting flags to confirmed status.

Once a profile is strongly established, per-translation-unit exceptions can be
investigated separately instead of contorting readable source to compensate for
the wrong compiler switches.

## Native Windows follow-up, 2026-09-30

After the BaseObject constructor/Release source changes and CodeView extent
support for the generated deleting wrapper, the full matrix produced:

| Profile | Strict generated | Manual | Calibration |
| --- | --- | --- | --- |
| `vc6_o2_ml_g6` | 39/39 | 19/19 | 10/16 |
| `vc6_o2_mt_g6` | 39/39 | 19/19 | 10/16 |
| `vc6_o2_ml` | 39/39 | 19/19 | 14/16 |
| `vc6_o2_mt` | 39/39 | 19/19 | 14/16 |
| `vc6_o1_ml` | 31/39 | 13/19 | 8/16 |
| `vc6_o1_mt` | 31/39 | 13/19 | 8/16 |

All `/O2` profiles resolve the BaseObject vtable and deleting-wrapper call
addresses. Removing `/G6` also matches BaseObject Release, UIControl slots 61/62
and FollowCamera slot 72. `/ML` and `/MT` produce equal counts within each tested
CPU/optimization profile, so the improvement cannot be attributed to runtime
selection. FollowCamera slots 69 and 71 remain nonmatching under both leading
profiles. The default remains unchanged pending broader per-TU evidence. See
`VC6_MATCHING.md`.

### Default profile — 2026-09-30

With FollowCamera slot 69's source shape and slot 71's 192-byte extent
(`VC6_MATCHING.md`), `vc6_o2_ml` and `vc6_o2_mt` reach 16/16 calibration while
`/G6` stays at 11/16; no calibrated target prefers `/G6`. The `tools/compile.py`
default (and so the gate) is now `vc6_o2_mt`: `/ML` and `/MT` emit identical code
for every tested target, and `/MT` is the runtime supported by the LIBCMT proof.
This is the best-supported working hypothesis, not a confirmed per-TU project
setting; pass `--profile` to test others, and keep treating a new mismatch as
evidence about flags or source shape.

Summaries prefer `strict_exact` when present. A wrong relocation binding cannot
be counted as exact merely because the legacy masked comparison passes.
