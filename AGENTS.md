# Agent workflow

## Objective and evidence

Reconstruct readable C++ that reproduces retail MCM2 x86 under VC6 SP3.

1. Confirmed: literal RTTI/source strings, COL/vtable addresses and offsets,
   target bytes, PE/import metadata, decoded direct instruction behavior.
2. Strong inference: repeated offsets, parsed base relations, canonical compiler
   artifacts, source attribution supported by independent context.
3. Provisional: semantic names, unproven C++ types/signatures and TU ownership
   inferred only from proximity.

Never silently promote inference to confirmed evidence. Preserve multiple and
virtual inheritance, secondary object offsets and adjustor thunks. A type can
have several vtables; always use `object_offset`. Shared tiny addresses alone
do not prove method identity. Prefer UnknownVirtualSlotN/field_0xNN names.

## Work loop

Start with `make status`, [setup](docs/TOOLCHAIN.md) and
[current matches/next targets](docs/VC6_MATCHING.md).

1. Select a target with defensible VA and extent. Inspect RTTI/slot, inheritance,
   source hints, destructor/thunk and vptr-write evidence before naming it.
2. Use `analysis/class_dossiers.json` as a join view, then trace claims back to
   `rtti_classes.json`, `vtables.json`, `vtable_overrides.json`,
   `class_layout_hints.json`, `deleting_destructors.json`, `vtable_thunks.json`,
   `vtable_write_xrefs.json` and `source_xrefs.json`.
3. Keep uncertain candidates in `samples/`. Reconstructed class source belongs
   in `src/reconstructed/`; use original TU paths only with strong ownership
   evidence. Filename-only skeletons belong in ignored `generated/`.
4. Compile with authentic VC6 SP3 and compare all bytes with relocations resolved.
   Clang is an ABI/code-shape check, not the historical compiler authority.
5. Calibrate flags before distorting readable source. No inline assembly, naked
   functions, copied machine-code arrays, .byte directives or matching-only
   linker tricks. Do not trim function extents to the requested target length.
6. Regenerate the function manifest/dossiers/queue after adding candidates.
   The legacy queue does not ingest VC6 reports; consult the actual matrix.
7. Run the affected checks and publish completed, validated slices to main.
   Fetch remote changes first and preserve unrelated local work.

```bash
python3 tools/find_class.py UIControl
python3 tools/discover_easy_targets.py --class UIControl
python3 tools/nearest_source.py 0x4703c0
make static-check test
make vc6-gate VC6_ROOT="$VC6_ROOT"
make vc6-profile-matrix
```

Do not hand-maintain trivial generated accessor probes. `make easy-smoke-vc6`
regenerates and checks them; a failure is compiler/profile evidence first.

## Category and library evidence

Read [CATEGORIES.md](docs/CATEGORIES.md) and [PROVENANCE.md](docs/PROVENANCE.md).
Categories describe accounting contexts, not exclusive source/class ownership.
Inspect literal selectors, source-reference instructions and non-inherited
primary RTTI slots. Do not propagate labels to callees, sibling methods, derived
classes or whole files. Mixed-category orchestration remains many-to-many.
External references are context until independently checked against retail.

## Private inputs and verification

Never commit VC6 binaries/headers/libraries, game data, generated objects,
signed bundle URLs or tokens. Use the verified installer and wrapper in
[TOOLCHAIN.md](docs/TOOLCHAIN.md). A static fingerprint is not compiler execution:
readiness requires authentic CL.EXE to emit nonempty i386 COFF. Keep readiness,
byte matching, source quality and library identity as separate claims.

## Repository hygiene

Current docs record facts, remaining uncertainty and reproduction commands.
Remove superseded stage reports, solved speculation and duplicate candidates;
Git history holds the chronology. Route checks through canonical source after
promotion. Retain regression tests that protect matching/evidence behavior.
Generated reports stay in ignored analysis/work directories and must not rewrite
tracked documentation. Do not delete private inputs as routine build cleanup.
