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
