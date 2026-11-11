# VC6 runtime identity and address atlas

The pinned SP3 LIBCMT.LIB contains object code linked into the known retail EXE.
This identifies runtime implementation and toolchain family; it does not recover
all game compiler flags or count as reconstructed game C++.

## Fully resolved allocator proof

| Symbol | Retail VA | Bytes | Applied relocations |
|---|---|---:|---:|
| `_malloc` | `0x0053789d` | 18 | 2 |
| `__nh_malloc` | `0x005378af` | 44 | 2 |
| `__heap_alloc` | `0x005378db` | 78 | 6 |
| `_free` | `0x00537929` | 72 | 7 |
| `__msize` | `0x005351f0` | 69 | 6 |

All **281 bytes** match after applying 23 relocations, with none ignored.
Single-thread LIBC.LIB has different `_free`/`__msize` shapes (47/41 bytes versus
retail's 72/69), lacking the observed multithread paths. Supporting small-block
functions and `__callnewh` match outside relocation fields; that is weaker evidence.

## Broader atlas

The scanner admits i386 functions with at least 20 non-relocation bytes, a
contiguous anchor of at least 12 bytes, a full masked-body match and exactly
one hit in executable sections. Aliases are retained; short/ambiguous functions
remain unassigned.

The recorded LIBCMT scan has **443 unique addresses**, 77,929 body bytes and
64,081 directly compared non-relocation bytes across `0x00534426..0x00548b50`.
This is masked provenance evidence, not 443 fully relocation-resolved matches.
The LIBC control finds 352 of those addresses; shared implementations do not
prove single-thread linkage.

Use admitted rows to avoid reconstructing CRT code as game source. Application
accounting wrappers remain in scope.

## Span attribution

LIBCMT is built with function-level linking, so a member is many COMDAT code
sections. Short or byte-identical sections never become unique atlas rows.
`attribute_span` (`mcm2tool/crt_atlas.py`, run by `make vc6-crt-atlas`)
places whole COFF code sections of `LIBCMT.LIB` over the span. Every placement
is a full relocation-masked match at a free address, and each one records its
evidence tier:

| Tier | Kind | Rule |
|---:|---|---|
| 1 | `atlas_section` | the section holding an atlas row matches whole at the address the row implies |
| 1 | `relocation_named` | a rel32/dir32 field in already placed library code decodes to the section's symbol at this address |
| 2 | `member_adjacent` / `gap_fill` | the section matches inside a gap, and no reference names it. The member is already linked (`member_adjacent`) or is not (`gap_fill`). The section needs a resolving relocation, 16 compared bytes, or 4 compared bytes and no relocations |
| 3 | either kind | weaker than that; accepted only because the section fills its gap exactly |
| 2 | `alignment_padding` | int3 run before a placed section that ends on that section's COFF alignment |

A candidate is rejected when one of its own relocations decodes to an address
that contradicts a symbol address already known.

The atlas rows leave 5,825 bytes of gaps in `0x00534426..0x00548b50`.
`rebuild_coverage.py` counted 5,813 of them, because it painted 12 of them as
two import thunks. All 5,825 bytes except one are LIBCMT.LIB sections:

| Tier | Kind | Ranges | Bytes |
|---:|---|---:|---:|
| 1 | `relocation_named` | 76 | 4,124 |
| 1 | `atlas_section` (the part outside the row) | 10 | 572 |
| 2 | `member_adjacent` | 30 | 695 |
| 2 | `gap_fill` | 5 | 165 |
| 2 | `alignment_padding` | 34 | 254 |
| 3 | `member_adjacent` | 2 | 14 |
| - | unexplained | 1 | 1 |

Notes on individual sections:

- The largest tier-1 gaps are `memmove.obj` `_memmove`
  (`0x00536d00..0x00537035`) and `memcpy.obj` `_memcpy`
  (`0x0053b0e0..0x0053b415`). Both are 821-byte sections with identical bytes,
  so only the relocations that name them tell them apart.
- `undname.obj` contributes 541 bytes and `rtti.obj` 154.
- The `_fread`/`_fwrite`/`_fseek`/`_ftell` wrappers come from their members,
  and so do `__lseek`/`__read`/`__write` (101 bytes each).
- Confirmed correction: `0x0053e039` (`GetOEMCP`) and `0x0053e04e` (`GetACP`)
  are not import thunks. They are tail-call `jmp [IAT]` instructions inside
  the static `_getSystemCP` of `mbctype.obj` (`0x0053e01f..0x0053e069`, tier 1).
  The masked match covers both.
- Tier 3: `fpinit.obj` `__fpclear` (`0x00534650`, a 1-byte `ret`) and
  `stdexcpt.obj` `?what@exception@@UBEPBDXZ` (`0x0053de2b`, 13 bytes).
- Alternatives: three placements have byte-identical alternatives, which the
  JSON lists.
  - Two are tier 2: `??_G__non_rtti_object` at `0x00535493` against
    `??_Gbad_cast`, `??_Gbad_typeid` and `??_Gexception`, and
    `?length@pcharNode` at `0x005438e6` against `?length@DNameStatusNode`.
    For these the member is certain, but which of the identical sections sits
    at the address is inferred.
  - One is tier 1: `__amsg_exit` is defined by both `wincrt0.obj` and
    `wwincrt0.obj`. The member that was already linked (`wincrt0.obj`) is the
    one chosen.
- Unexplained: one `0xcc` at `0x0054550b`. It sits between `rmtmp.obj`
  `__rmtmp` (ends `0x0054550b`) and `wincmdln.obj` `__wincmdln` (starts
  `0x0054550c`). Both sections declare 1-byte alignment, so neither section
  explains this byte, and no rule above does.

The run used LIBCMT.LIB alone. No other library (OLDNAMES, LIBCP, DINPUT,
DXGUID, import libraries) is needed to explain the span, apart from the one
byte above.

## Code after the span: `0x00548b56..0x005495d0`

These 2,682 bytes lie between the KERNEL32 `RtlUnwind` thunk
(`0x00548b50`) and the EH tail. They are 10 bytes of int3, then code at
`0x00548b60..0x005495c5`, then 11 bytes of int3.

What is confirmed by decoding:

- The code is two functions, `0x00548b60..0x00549038` and
  `0x00549038..0x005495c5`.
- Both are perspective-correct texture span loops. One reads 8-bit texels and
  the other 16-bit texels (`word [esi*2]`).
- They step in subdivisions of 8 pixels (`shr ecx,3`, `and eax,7`). The u/v
  steps are converted with `fistp`, using a 65536.0 scale.
- They step the texel address with `add ecx,[dUfrac]; sbb ebp,ebp; add ebx,edx;
  adc esi,[ebp*4+0x575af0]`.
- They switch the FPU to single precision (`and eax,0xfffffcff` around
  `fnstcw`/`fldcw`).
- No call, jump or pointer to either function was found.

What is inferred:

- The prologue `push ebp; mov ebp,esp; add esp,-0x28; push edi; push esi; push
  ebx` and the matching epilogue are the shape MASM produces for
  `PROC USES edi esi ebx` with `LOCAL`. VC6 would use `sub esp` (strong
  inference).
- The functions' constants (`0x00575adc..0x00575b1c`: 65536.0f, 1.0f, the
  step table) sit in `.data`. They come right after the last game object's
  data (`wrecker.cpp`) and before LIBCMT's `.?AVtype_info@@`. The object was
  therefore linked after the game objects and before the libraries. Its code
  landing after LIBCMT points to a separately named code section of a game
  assembly module (strong inference, provider unidentified).

Libraries checked, with zero hits: every code section of every `.LIB` and
`.OBJ` under `VC98/` was searched for an unmasked run of 12 bytes or more.
That is 718 files and 38,135 sections, including LIBC/LIBCMT/MSVCRT, LIBCP,
LIBCPMT, OLDNAMES, MFC, DINPUT, DXGUID and all import libraries. A copy of
another VC6 tree (`MSVC600-master`, 293 files, 36,196 sections) also gave zero
hits. The bundle has no `ML.EXE` and no DirectX SDK.

`rebuild_coverage.py` now starts the EH tail at the first unwind funclet that
the tail stubs' FuncInfo (magic `0x19930520`) names. FuncInfo `0x0055fd08`
(maxState 2) of the first stub names `0x005495d0` and `0x005495e4`, so the 40
bytes `0x005495d0..0x005495f8` are `.text$x` (confirmed). They are no longer
part of this block.

## Reproduction

```bash
python3 tools/with_private_env.py -- python3 tools/verify_vc6_runtime.py
make vc6-crt-atlas          # atlas rows plus span_attribution, about 10 s
make vc6-crt-atlas-test     # synthetic tests, no private inputs
```

Outputs: `work/vc6-runtime-proof.json` and `work/vc6-crt-atlas/atlas.json` plus
reports, with input/library hashes. The `span_attribution` key of `atlas.json`
holds every placement (member, symbols, kind, tier, evidence and alternatives),
the padding and the unexplained ranges. `tools/rebuild_coverage.py` reads it.
Static comparison requires neither Wine nor execution of game code.
