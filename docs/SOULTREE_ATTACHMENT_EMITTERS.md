# Soultree attachment emitter connections

Nine existing calls in SoulTreePhysics.cpp use the canonical DustParticleEmitter and SteamParticleEmitter declarations. Seven obsolete method names disappear; the attachment records and provisional prefix views remain. No constructor, virtual dispatch, storage or emitter Tick implementation changes. Types 2 and 3 and the mixed impact-sink receivers remain outside this connection.

## Identity and call boundaries

Bike::LoadBike allocates 0x80 bytes, calls the Dust constructor at 0x4b8a00, and stores its returned primary pointer at Bike +0x728. The type-1 registration at 0x408a69 reloads that pointer and invokes primary slot37 without adjustment. The Steam path allocates 0x47c bytes, calls 0x4b9f40, stores the result at +0x730, and registers that pointer as type4 at 0x408ae8. Raw primary COLs and the nonvirtual SoultreePhysicsBaseObject base at zero establish the Bike dispatch boundary; the separate virtual GameObject base is not used as that receiver.

Scene's independent Dust path allocates 0x80 bytes and calls the same constructor at 0x4ed70e. The returned pointer stays in EDI and is passed as type1 at 0x4ed774. Its owner is constructed at 0x4ed2d6 by the complete 194-byte SoultreePhysicsObject constructor. Raw primary RTTI and slot37 establish the same attachment dispatch; its separate rendering subobject is not substituted for the physics primary pointer. These finite observed producer chains do not prove every possible external attachment producer.

Slot37 at 0x502a40 is checked across its complete 272 bytes, including the 16-byte switch table. It appends 40-byte records; type1 copies the emitter argument into record +4, and type4 copies it into +0x14, with no adjustment. Type4 also copies the local offset and clears the anchor. The actual header layout and table destinations are independently asserted.

The complete slot18 at 0x502420/193 bytes and slot21 at 0x5024f0/1116 bytes remain strict. The nine actual direct targets and their surrounding receiver/argument instructions are retained. The Dust position fourth argument changes from an integer literal zero declaration to the genuine float parameter with 0.0f; the emitted word remains zero. The two color byte parameters receive existing in-range constants 0x40, 0x80 or 0xff; all twelve stack argument bytes and cleanup remain unchanged. The 12-byte vector by-value copies preserve both component bits and temporary lifetimes.

## Complete providers and preservation

| Provider | Retail entry | Complete bytes |
| --- | --- | --- |
| Dust SetPosition | 0x4b8d90 | 58 |
| Dust SetColor | 0x4b8dd0 | 31 |
| Steam SetPosition | 0x4ba2c0 | 49 |
| Steam Restart | 0x4ba300 | 18 |
| Steam Burst | 0x4ba320 | 19 |
| Steam StopEmitting | 0x4ba340 | 30 |
| Steam SetColor | 0x4ba360 | 34 |

These existing game-owned bodies are all strict and have no calls or relocations. The separate native extraction contains unchanged complete source definitions, dual-compared against their canonical objects and retail; its only external marker is __fltused. There are no scripted game helpers or retail call patches.

Same-path before/after comparison preserves the complete caller's 128 nondebug sections and 625 relocations under exactly seven method-name substitutions, excluding no filename sections. Retained pins additionally cover both providers, Bike and the Scene aggregate: five objects, 520 sections and 2,324 relocations. Private/actual-root compilations agree under explicitly enumerated source filenames. All 50 existing strict physics rows remain strict, and the two complete resolved partial fingerprints remain unchanged. The retained total is 66 strict checks, including the two constructors and seven dual-checked native methods.

The prior-pin inventory includes Python literals as well as JSON fixtures. Both existing model-render Soultree partial pins were independently reproduced before and after the joins. No prior pin updates are required. Existing model-render, contact-consumer and Soultree method/storage suites pass in the actual checkout.

## Native scope

The fixture independently models every ordered DWORD read/write, including previous-position copies, the extra Dust intensity store, Steam Restart's initial read before its writes, Burst's three stores, StopEmitting's leading clear, and packed byte colors. Each role executes on the same prepared guarded page, compares all 4 KiB, and requires either successful completion or the exact independently predicted read/write access fault and address. The retail PE is hash-pinned and receives no instruction changes.

There are 130,816 finite cases, with 30,080 modeled access faults: 64 input/page patterns, seven methods, writable and read-only ordinary receivers, null, and every aligned accessible-prefix boundary through 0x480. The first input patterns include the actual caller colors and zero intensity; remaining values retain arbitrary DWORD bits. The methods copy those bits rather than evaluating floating-point arithmetic. No split DWORD access is exercised.

This is prepared-storage method evidence, not constructed emitter lifetimes, a full slot18/slot21 simulation, particle generation or gameplay execution. It does not establish portable out-of-object pointer arithmetic or complete equivalence of provisional view types. RTTI, construction, strict callers and exact object preservation support the narrow connection separately.

Reproduce with authentic VC6 SP3 and the private retail image configured as VC6_ROOT and MCM2_EXE:

```
python -m unittest tests.test_soultree_attachment_emitters -v
```

Original emitter translation-unit ownership and several semantic names remain provisional. The genuine canonical declarations are reused without claiming a newly recovered original TU.

## Source-build verification

The isolated actual build compiles 270/270 translation units and removes exactly
the seven obsolete method names (202 to 195), with none added. All old objects
preserve 10,816 nondebug sections and 46,075 relocations under those names,
without filename exclusions. Calibration remains 3,023/3,034 strict and physics
1,086/1,162 strict, with every prior full extent and score unchanged.
`work/game-build/round60-soultree-link-proof.json` and
`connections-round60-soultree-preservation.json` record this isolated gate.

The later Character blend connection requires one Bike fingerprint transition
in this suite. The old value was reproduced from the actual Soultree link
object, and the replacement is proved by its sole rider-call name change.
All other fixture records remain unchanged. Both retained suites pass in the
combined build; the game still does not link or have runtime verification.
