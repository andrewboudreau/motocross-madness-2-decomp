# VC6 CRT provenance atlas

The exact runtime proof establishes that selected objects from the supplied
VC6 SP3 `LIBCMT.LIB` are present byte-for-byte in retail MCM2. This atlas
extends that proof into a conservative address map useful for decomp triage.

## Acceptance rule

A library function is admitted only when all of the following are true:

1. its COFF object is i386 code;
2. after marking explicit COFF relocation fields, at least **20**
   non-relocation bytes remain;
3. the longest contiguous non-relocation anchor is at least **12 bytes**;
4. the complete body matches when relocation fields are treated as link-time
   values; and
5. that masked body has **exactly one hit** across executable sections of the
   known retail `mcm2.exe`.

Section pseudo-symbols such as `.text` are excluded. Multiple library names
that resolve to the same retail address are preserved as aliases rather than
counted as separate functions.

These thresholds intentionally leave short or ambiguous routines unmapped.

## Measured multithread-runtime atlas

Against the pinned private `LIBCMT.LIB`:

| Measurement | Result |
|---|---:|
| Unique matched retail addresses | **443** |
| Union of matched function bodies | **77,929 bytes** |
| Directly compared non-relocation bytes | **64,081 bytes** |
| Lowest matched address | `0x00534426` |
| Exclusive end of highest matched body | `0x00548b50` |
| Address span | **83,754 bytes** |
| Matched-body coverage inside that span | **93.0451%** |
| Overlapping selected ranges | **0** |

The observed cluster begins immediately after the import thunk at
`0x00534420`; another import thunk begins at `0x00548b50`. That boundary
shape is useful corroboration, but the atlas does **not** promote every byte
between those thunks to CRT ownership.

The remaining span bytes include gaps between admitted rows. They can be
alignment, import thunks, short/ambiguous runtime routines, or other code.
They remain unclassified until independently proven.

## Single-thread negative/control scan

Running the identical conservative scan against the supplied `LIBC.LIB`
finds **352** retail addresses and 61,412 bytes of matched function bodies.
All 352 addresses are also in the `LIBCMT` atlas, while **91 of the 443
LIBCMT addresses have no single-thread match at the same address**.

Many CRT routines are byte-identical between the single- and multithread
libraries, so common matches are expected. This control is not used to claim
that the common rows were linked from `LIBC.LIB`.

The stronger runtime-variant evidence remains the fully relocation-resolved
allocator set documented in `VC6_RUNTIME_PROOF.md`: retail `_free`,
`__msize`, allocation helpers, and their lock/small-block paths match the
multithread objects while the single-thread shapes differ.

## Decomp/provenance impact

This is a large enough identified runtime region that agents should not spend
time reconstructing admitted atlas rows as Rainbow game code.

The safe rule is address-specific:

- **atlas row:** Microsoft VC6 CRT-associated code, backed by matching library
  object bytes;
- **gap inside the surrounding span:** still unknown unless another evidence
  source classifies it;
- **code outside the span:** unaffected by this atlas.

The atlas is provenance evidence, not decomp completion. Runtime bytes do not
become "source reconstructed" merely because Microsoft shipped matching
objects.

## Reproduce

After installing the pinned private bundle:

    make vc6-crt-atlas

or:

    python3 tools/with_private_env.py -- \
      python3 tools/build_vc6_crt_atlas.py

Outputs are written below ignored `work/vc6-crt-atlas/`:

- `atlas.json`: every admitted function address, object member, symbol,
  relocation count, alias set, and match-strength measurements;
- `REPORT.md`: compact summary.

The scanner itself is input-free testable:

    make vc6-crt-atlas-test

No VC6 process or Wine is required for this static atlas. Authentic compiler
execution remains necessary for recovering the original game translation-unit
flag profile.
