# Bit-string sub-word shifts

## Functional build review

The two sub-word shifts used by `GR_BitString`'s whole-word wrappers sit in
[GR_BitString.cpp](../src/reconstructed/GR_BitString.cpp) between those wrappers
and the constructor, in retail order. They are not exact. Names are
provisional; the unit's file name is not recovered (no `__FILE__`).

| Method | Retail VA | Complete retail bytes | Candidate bytes | Resolved matching positions |
|---|---:|---:|---:|---:|
| `ShiftBitsTowardFront(int)` | `0x004239c0` | 420 | 420 | 343 |
| `ShiftBitsTowardBack(int)` | `0x00423b70` | 442 | 442 | 387 |

The test compiles a validation unit holding only the two definitions
(`tools/unit_source.py`) and requires its bodies and relocations to equal the
whole unit's; the unit's nine calibration cases stay strict. The regression
pins the two complete nonexact scores.

## Forms tried

### ShiftBitsTowardFront (`0x004239c0`)

0x004239c0 (343/420): retail keeps `this` in ebp and `count` in ebx and
computes `32 - count` into edi before the first word; VC6 here swaps the
two and computes it later. `src = (unsigned int*)data + 1;` gives retail's
allocation and order (7 differing instructions) but re-reads `data` where
retail forms `lea edx, [esi + 4]`, so it stays `dst + 1`. Unchanged at 343:
every order of the words/dst/carryShift assignments (declared at the top or
assigned as in ShiftTowardFront), for/while(--i)/for(;--i;) loops,
`register` on any subset of the locals, `const int count`, a `count` copy,
the first word in its own local or block, `src = dst; *src++ = ...`, the
src step in the for header, swap-macro spellings (operands and `+`), a
`fill` local or swapped fill arms (worse), and `32 - count` written out
for any subset of the carryShift uses (worse), and 0..8 dead initialised
locals.

### ShiftBitsTowardBack (`0x00423b70`)

0x00423b70 (387/442): retail computes `src = dst - 1` (lea esi, [edx - 4]),
steps it forward with `dst` after the last word (add esi, 4) and
pre-decrements it at the loop top; the bottom-decrement loop gave 93/438.
Left: retail keeps `src` in esi and `dst` in edx (VC6 here the other way)
and two swap-macro ORs end in the other register. Unchanged at 387: `src`
assigned before or after the first read, `src++` and `dst++` as separate
statements or a comma expression, `*--src` in the zero test, while(--i)
and a split `*--dst` in the 0xffffffff arm. Worse: `src = dst - 1` after
the store (83), src assigned from `data` (59), while(--words) (154),
swapped fill arms (367), the first word read through `src[1]` (45),
a mask local in the 0xffffffff arm (204), a count-up loop (155),
`32 - count` in place of carryShift (15) and `src` re-derived from
`data` (17); `&data[words - 1]` and a separate `>>= count` stay 387.
Also 387: 0..8 dead initialised locals, `i != 0`/`--i` and `for (i =
words; --i;)` loops, `--src`, `!*src`, a split `dst--` in the 0xffffffff
arm and the head's `dst++`/`src++` in either order. `i > 0` is 386, `dst--`
hoisted ahead of the value test 155 and ahead of the fill arms 343.
Wave 32: whenever VC6 folds `src = dst - 1; src++` away (src defined after
`dst++`, `src++` ahead of the store, `src = data + words - 2; dst = src +
1`) src gets esi and dst edx as retail and the loop matches (149-151 of 158
instructions, 437-439 bytes), but the head then lacks retail's unfolded
`lea esi, [edx - 4]` .. `add edx, 4` / `add esi, 4`; every form that keeps
the pair unfolded gives dst esi. Also 387: the swap macro with its terms
reversed, `<< 24` forms or casts, an inline swap taking a reference or a
pointer, a `w = *--src` local, the head steps in the for initialiser
(`dst++, src++`; `src++, dst++` 6/443), `dst[0] =`. Worse: `dst[-1] =` 378,
an inline swap by value 170, `i` assigned before the first store 122, a
post-decrement mirror of the loop 60/417, an indexed `w[i]` loop 33/428, a
`last` temporary 13.

## Identity, extent and dependencies

Raw RTTI names `GR_BitString`, with COL `0x0055ae08`, primary table
`0x00550f50` and object offset zero. Table slot 2 reaches the signed shift
dispatcher at `0x00423e90`. Its existing strict whole-word helpers at
`0x004238c0` and `0x00423940` reach the two new bodies. The constructor at
`0x00423d50` and the existing header establish the 32-byte receiver layout:
bit count at +4, data pointer at +8, fill word at +0x18.

Complete recursive retail CFG traversal reaches every byte of each stated
extent. The front returns at `0x00423b48` and `0x00423b61`; the back returns
at `0x00423d0c` and `0x00423d27`. Each return pops four bytes. Stopping at the
first return would omit a fill branch. Neither body has calls or an EH handler.

The only undefined symbol in the standalone candidate is
`?g_WordHighBitMasks@@3PAIA`, already supplied by canonical
`src/reconstructed/GR_BitString.cpp`. Its complete 33-DWORD table at
`0x00568098` matches retail and a separate big-endian mask oracle. Its emitted
section is writable initialized data; no new storage or CRT initializer is
introduced by the standalone source.

The wrappers process complete 32-bit words first and call the sub-word bodies
only for positive remainders from 1 through 31. Slot 2 skips null data or zero
bit count. A positive bit count is rounded up to a whole word; the sub-word
operations shift that rounded storage, rather than clipping the final byte to
the nominal bit count. Zero fill clears the incoming edge; any nonzero fill
selects the all-one edge branch. Whole-word fill still uses its actual DWORD.
The constructor normally supplies zero or -1.

## Native validation

`tests/test_bitstring_shifts.py` compiles the actual standalone source and actual
canonical `GR_BitString.cpp` separately with authentic VC6 `/O2`. Only the
validation harness uses `/Od`. Its private, hash-pinned retail mapping executes
the complete retail methods and wrappers. It redirects no retail calls, patches
no candidate objects, and mocks no math, masks, allocation or external API.
Prepared raw receiver storage deliberately bypasses unrelated constructors.

The retained fixture runs **422,838 cases**:

- 412,672 direct cases: both directions, 32 deterministic byte patterns,
  13 positive bit counts spanning partial and complete words through 256 bits,
  four fill values, four byte alignments, and every count from 1 through 31.
  Every successful result is also checked against an independent bit-by-bit
  big-endian oracle.
- 9,984 integration cases execute both actual whole-word wrappers and the
  actual signed virtual-slot body, with counts including 0, 31, 32, 33, 63,
  64, 65, 95, 128 and 257. The independent bit oracle applies here too.
- 132 direct fault/alias cases cover null receivers/data, progressively readable
  DWORDs at the upper guard, a lower guard reached during backwards traversal,
  truncated receivers, and data overlapping the receiver or its fill field.
- 48 cases independently require the slot body's null-data/zero-count early
  exits to succeed without exceptions.
- Two additional independent access-fault oracles require the expected read
  fault address and the exact buffer word already changed before the fault,
  once in each traversal direction.

Every differential case compares the full 1,024-byte prepared storage region,
the entire writable guard page, exception code, access kind and fault address.
Fault tests compare partial memory effects; full register context, instruction
pointer, exception resumption and external exception handlers are not claimed.
These are integer-only bodies; no floating-point contract is introduced.

The finite successful domain uses prepared positive-length buffers. Direct
zero/negative bit counts and arbitrary out-of-range shift arguments can create
invalid indexing or unbounded loops in retail and are not accepted successful
inputs. No real game session, compressed-texture decoding or all bit-string
operations are validated by this fixture. The separate complete `ReadBits`
candidate and source-only LZW integration have their own
[bounded validation](BITSTRING_READ.md).

## Reproduce

```powershell
$env:PYTHONPATH='.'
$env:VC6_ROOT=(Resolve-Path work/vc6sp3-disc).Path
$env:MCM2_EXE=(Resolve-Path work/game/mcm2.exe).Path
python -m unittest discover -s tests -p test_bitstring_shifts.py -v
```

Private extraction evidence is under `work/provider40`: `bitstring.obj` is the
same-path pre-extraction aggregate, `aggregate-after.obj` is the extracted
aggregate, and `aggregate-preservation.json` records the identical normalized
whole-object fingerprint. `cfg.py`/`cfg.json` and the two `*-cfg.txt` files retain
the complete retail traversal; `*-standalone.json` records the fresh comparisons.
These private artifacts are not inputs required by the retained regression.
