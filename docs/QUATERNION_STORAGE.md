# Shared quaternion fallback storage

`src/reconstructed/QuatState.cpp` supplies the existing missing
`g_QuatIdentity` symbol as sixteen zero-initialized writable bytes. The symbol
name is provisional and retained for source compatibility. It does **not**
establish a mathematical identity quaternion or original translation-unit
ownership. The earlier `{1,0,0,0}` and runtime-initializer comments were
unsupported and are corrected without changing consumer code.

## Retail storage evidence

The range `0x006850b0..0x006850bf` lies wholly beyond the file-backed portion of
the writable `.data` section and inside its virtual extent. Consequently the
loader initially supplies sixteen zero bytes. This is distinct from the
question of later writes.

The pinned retail image has stripped relocations and an empty base-relocation
directory. No claim of a relocation-table proof is made. A byte-by-byte search
of every file-backed section for every address inside this sixteen-byte range
finds the eight actual component operands below and one overlapping instruction
byte sequence. The latter, at `0x0044015e`, happens to encode `0x006850b2` while
crossing a LEA and two PUSH instructions in the complete 1,964-byte ReadLods
body; it is not an address operand or a stored pointer.

| Consumer | Complete retail extent | Component-read instructions |
| --- | --- | --- |
| QuatNormalize | `0x004a1d60`, 173 bytes | `0x004a1da0`, `0x004a1da6`, `0x004a1dac`, `0x004a1db2` |
| PhysicsRigidBody::ResetState | `0x004cc500`, 256 bytes | `0x004cc5b8`, `0x004cc5c5`, `0x004cc5ce`, `0x004cc5d7` |

Each instruction is a read of one DWORD at offsets 0, 4, 8 or 12. No direct
write or address-taking operand for this storage was found in this audit.
The nearby address-producing immediates target the separate MemTagStack at
`0x006850c0`. The four preceding CRT initializer entries, indices 179 through
182 at `0x004a2210`, `0x004a2260`, `0x004a22b0`, and `0x004a2300`, initialize
only the existing vector objects at `0x00685070`, `0x00685080`, `0x00685090`,
and `0x006850a0`. Their complete strict bodies write twelve bytes each, ending
at `0x006850ab`. Entry 183 initializes MemTagStack at `0x006850c0`.

This supports a loader-zero provider with **no discovered initializer in the
bounded audit**. It does not prove immutability, exhaustive absence of indirect
writes, or the meaning of every operation that could form an address at runtime.
The provider therefore remains writable. Derivative-history reset is also
consistent with zero values, but that semantic inference is not used to replace
the concrete PE evidence.

## Readable provider and unchanged consumers

The provider uses the same ordinary four-float POD shape already declared
independently by QuatUtil.cpp and Math3D.h, with no constructor or initializer
expression. It introduces no new type hierarchy or complete-class identity
claim. The existing scalar-first interpretation is inferred from the arithmetic;
the retained layout probe separately checks the four offsets and size 16.

Authentic VC6 emits one 16-byte `.bss` section with characteristics `0xc0400080`
and the usual 40-byte compiler directive section. The sole external definition
is `?g_QuatIdentity@@3UQuat@@A` at BSS offset zero. There are no nondebug
relocations, imports, functions, read-only payloads or CRT startup entries.
Debug-symbol relocations are not runtime dependencies.

Only comments change in the existing QuatUtil.cpp and Math3D.h. The old complete
QuatUtil, PhysicsRigidBody sample and canonical Math3D objects retain all
**87 nondebug sections and 213 relocations**, including flags, storage,
initializers, code and relocation destinations, with no symbol mappings or
path/section exclusions. Original strict and partial bodies are preserved.

## Actual-consumer native validation

The test compiles the complete actual QuatUtil.cpp and the new storage provider
as separate objects. It extracts the actual ResetState source body with its
genuine PhysicsBody header and compares the complete emitted body and named
relocations to the original complete sample. The source is not replaced by
an implementation of expected behavior. ResetState remains a sample consumer;
this storage change does not admit its whole translation unit.

Fourteen strict checks cover all eleven QuatUtil functions and initializer
bodies, the complete 256-byte ResetState, and the extracted consumer's complete
zero-vector initializer wrapper and body. That local vector's source initializer
runs through the normal CRT startup table. The private retail reference receives
its separately verified zero-vector value at `0x00689a00`. No game-helper code,
CRT entry point or consumer instruction is patched.

The reference runs complete pinned retail consumer bodies. The candidate runs
the actual emitted source functions. Both use their own initially zero shared
quaternion cells. Two initial scenarios exercise normalization and reset before
any deliberate fixture write to either quaternion. The subsequent cases write
the same four raw patterns into the actual source global and the reference
cell, then check the resulting values. This proves that the consumers observe
the shared linked storage, rather than independent constants or folded zeros.
The test mutations do not assert that retail normally performs those writes.

The final matrix contains **11,134 scenarios**:

| Matrix | Cases | Independent expected behavior |
| --- | ---: | --- |
| Initial normalization and derivative reset | 2 | Loader-zero fallback and derivative values before fixture mutation |
| Signed-zero fallback, four global patterns and thirteen overlapping output positions | 9,984 | Exact raw global copy, hidden-output return pointer and unchanged surrounding bytes |
| Nonzero single-component powers of two | 864 | Exact signed unit component and zero remaining components |
| Whole rigid-body reset over four global patterns and 32 byte patterns | 128 | Accumulator/history clearing, initial-velocity copies, two shared quaternion copies and every other byte unchanged |
| Guarded reset with four patterns and aligned writable sizes `0x210..0x2a8` in steps of four | 156 | Complete page, exact written prefix, first fault address/access kind and completion classification |

The normalization matrix spans all twelve combinations of masked exceptions,
24/53/64-bit x87 precision and nearest/down/up/chop rounding. Powers range from
`2^-60` to `2^60` in steps of fifteen exponents. This is not a proof over arbitrary
nonzero floating-point quaternions, unmasked exceptions or full x87 fault
context. Raw NaN/infinity patterns appear only in the copied fallback storage;
they are not used as normalization arithmetic inputs.

The guarded reset cases include **148 independently modeled access violations**.
The minimum available prefix contains all initial-velocity input fields.
The expected store sequence is written separately from the source body and
compared against the complete 4 KiB page. Receivers are prepared storage, not
constructed live game objects. Neither constructor/destructor lifecycle nor a
whole rigid-body simulation is claimed.

The fixture checks the explicit 32-bit hidden-output convention for the
normalizer and a four-byte member-entry representation for the single-inheritance
reset consumer. A negative control builds the conventional `{1,0,0,0}` value;
the same executable rejects it at the initial loader-zero assertion with exit 2.

## Reproduction

With authentic `VC6_ROOT` and pinned `MCM2_EXE` configured:

```powershell
$env:PYTHONPATH='.;tests'
python tests/test_quaternion_storage.py
```

The retail SHA-256 is
`31fde4cc686a5ee89ef9095b90235325b195596867ecacefe511263e1509b874`.

### Functional build review

Actual build `work/game-build/build-gpaauzfi` compiles 273/273 sources and removes
exactly the missing quaternion global: **184 -> 183** unresolved names, none
added. All 272 previous objects preserve their complete 10,864 nondebug sections
and 46,268 relocations without symbol mappings or filename exclusions. The new
object has two nondebug sections, including compiler directives, and no
relocations. Its only storage is the sixteen-byte writable BSS payload.

The current-source retained test passes. Global calibration and physics checks
preserve every previous extent, score and strict result. No build configuration,
compiler flag or existing calibration row changes. Evidence is in
`work/game-build/round63-quaternion-link-proof.json` and
`connections-round63-quaternion-preservation.json`.

The game still does not link; startup, menus and racing remain unverified.
The existing symbol spelling and the absence of a discovered initializer must
not be restated as a conventional identity value or global immutability.
