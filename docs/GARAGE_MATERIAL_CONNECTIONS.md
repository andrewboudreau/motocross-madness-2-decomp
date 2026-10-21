# Garage material texture connection

`SPBikeRiderDlg::RenderPre3D` now calls the existing
`D3DIMSoultreeObject::ReplaceMaterialTexture` provider for the selected bike and
rider model. The old `UnknownModelTexture` method spelling is retired from the
canonical caller and its aggregate bindings. The provisional record views and
all other interfaces remain unchanged.

The complete 257-byte caller at `0x0044ffc0` calls the complete strict 133-byte
provider at `0x00444c70` from `0x00450042` and `0x004500ac`. Both calls pass
index zero, the same texture-name pointer, and the address of TrackGame's
`+0x1c` texture-context cell. The existing canonical declaration has an unused
integer third parameter; the explicit pointer-to-int cast retains its 32-bit
address. All three provider exits use `RET 12`. The caller ignores the integer
return, just as it ignored the old void declaration. No argument is removed or
reinterpreted as a pointee value.

## Receiver evidence

The old `plateTexture` field name is misleading at this boundary: the value at
character `+0x1a0` is the primary model-node pointer. The garage builder's
complete strict 3,114-byte body allocates `0x240`-byte characters, calls the
strict 195-byte D3DIMSoultreeCharacter constructor, and stores the primary
results at the rider-preview field `+0x46c` or manufacturer-row field `+0xc0`.
The character's model producer is the complete strict 1,021-byte virtual-slot0
body. It calls the strict 275-byte D3DIMSoultreeObject constructor at
`0x00445b29`, then stores returned EAX at character `+0x1a0` at `0x00445b45`.
The separate GameObject attachment path adds `0x0c`; these texture calls do not.

Raw RTTI identifies primary D3DIMSoultreeObject/SoultreeObject at offset zero
and its secondary GameObject at `+0x0c`. The retained test checks both model
COL/type records, the character primary table, constructor/storage instructions,
and the two caller receiver loads. Native compiled layout assertions check the
actual caller's `plateTexture` and manufacturer `uiModel` offsets as well as
canonical material-table/count and material-name/flag fields. This establishes
these pointer boundaries, not whole-class equivalence of provisional views.

## Complete preservation

Authentic VC6 SP3 `vc6_o2_mt` compilation preserves all 420 canonical DlgProcs
nondebug sections and 3,040 relocations, plus all 557 aggregate sections and
4,152 relocations, under exactly one external-name mapping:

```
?ReplaceMaterialTexture@UnknownModelTexture@@QAEXHPBDPAPAVTextureMapManager@@@Z
  -> ?ReplaceMaterialTexture@D3DIMSoultreeObject@@QAEHHPBDH@Z
```

The same-path before/after proof retains every section byte, flag, relocation
kind/addend and defined destination. No filename or partial function is omitted.
Portable retained pins normalize only the source paths already enumerated by
the UIAnim object checker. Its two affected old DlgProcs pins were independently
reproduced from published-baseline objects before advancing them under this
single mapping; its two GUIManager pins are unchanged.

`tests/test_garage_material_connections.py` retains 99 full strict comparisons,
two complete-object pins and two direct call sites. The provider is extracted
unchanged into the native test object and checked against both the complete
production object and retail's entire 133-byte extent. It imports only
`strncpy` and `SoultreeMaterial::LoadTexture`, with no data or CRT initializer.
The complete 948-byte LoadTexture provider is separately strict-checked.

## Native validation scope

The retained fixture executes the actual replacement body on prepared raw model,
material and name storage. It does not construct those classes or execute the
full garage, texture-resource or driver lifetime. Two verified retail direct
calls are redirected privately:

- `0x00444cc5`, originally retail `strncpy` at `0x00534b10`, uses the same native
  VC6 CRT `strncpy` that the source calls. CRT-internal byte identity is not claimed.
- `0x00444ce4`, originally `SoultreeMaterial::LoadTexture` at `0x004ff620`, uses
  an explicitly scripted member boundary. The actual LoadTexture body is not
  executed by this fixture.

Each patch verifies its original opcode and destination. The source replacement
body has no instruction patches. Its member-pointer representation and zero
adjustor are asserted before the reference address is installed. The fixture
compares all three accessible 4-KiB pages, the complete pre-helper snapshot,
helper receiver/count, return value, exception kind/access/address, and final
state between source and reference. Independent oracles also check truncation,
terminator position, unchanged guards and early-return purity.

The finite matrix contains 11,589 cases:

- 11,520 combinations: six signed material counts, five indices including
  prepared negative-index slots, four material flags, eight name lengths
  spanning the 63-byte truncation boundary, four helper behaviors and three
  untouched opaque argument bit patterns. All ordinary cases must complete
  successfully with the independently expected result. The helper modes are
  return, material-flag mutation, a labeled exception, or clearing the model's
  material-table pointer. The 1,080 labeled exceptions preserve the verified
  name write and pre-call state.
- Four invalid model/table/material/name pointer cases and 65 unterminated
  guarded name prefixes. All 69 access violations require independently
  expected read addresses, zero helper calls and unchanged storage.

Input names occupy separate storage; overlapping `strncpy` inputs are not
claimed. The existing `index > materialCount` check, including its equality
case, is preserved. Prepared negative indices remain within the fixture's
backing table; the test does not assert arbitrary invalid-index safety or
portable object-lifetime semantics.

## Reproduction and build result

```powershell
$env:PYTHONPATH = '.;tests'
python tests/test_garage_material_connections.py -v
python tests/test_uianim_connections.py -v
```

Set `VC6_ROOT` and `MCM2_EXE` to the verified private inputs. The new regression
passes 99 strict checks and 11,589 native cases; the existing UIAnim regression
passes 199 strict checks and 3,067 native cases after the precise pin update.
The isolated actual build `build-pejzjcw7` compiles 261/261 translation units and
removes only the obsolete method spelling, reducing unresolved symbols from
222 to 221. All 260 pre-round objects preserve 10,693 nondebug sections and
45,776 relocations under this mapping. The game still does not link; startup,
menus and racing remain unverified.
