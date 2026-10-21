# Global bit reader and source LZW integration

`GR_BitString::ReadBits(int)` is reconstructed as ordinary C++ in
[`samples/render/BitStringRead.cpp`](../samples/render/BitStringRead.cpp).
The complete retail body is **125 bytes**, `0x00423ef0..0x00423f6c`, including
`RET 4`; the following three NOP bytes precede the next function. VC6 SP3
emits **157 bytes**, with **8 relocation-resolved matching positions**. This
is a nonexact candidate, not a matching source promotion or recovered original
translation unit. Its retail frame, BSWAP and local-return sequence suggest
original inline assembly, but that attribution is inference. The reconstruction
uses neither assembly nor copied code, and production never maps retail code.

## Identity and complete behavior

The raw primary RTTI table at `0x00550f50`, COL `0x0055ae08` with object offset
zero, names `.?AVGR_BitString@@`. The actual constructor at `0x00423d50` writes
that table. `ReadBits` is nonvirtual; its receiver is unused. The complete
strict LZW decoder calls it at `0x004a041a`, `0x004a043b`, `0x004a0461` and
`0x004a05a3`. The first read discards a 32-bit prefix; later calls use the
current dictionary code width. The retained test decodes the complete reader
CFG and proves every reachable byte, branch and final return independently of
the candidate's extent.

The reader uses shared state already defined by canonical `GR_BitString.cpp`:

| Storage | Retail VA | Evidence |
| --- | --- | --- |
| Low-mask array, 33 DWORDs | `0x005681a0` | All 132 bytes match `(1<<n)-1`, n=0..32 |
| Pointer to that array | `0x00568224` | Retail value and actual COFF relocation |
| Current big-endian word | `0x00578e90` | Writable loader-zero storage |
| Bits consumed in current word | `0x00578e94` | Writable loader-zero storage |
| Current input-word cursor | `0x00578e98` | Writable loader-zero storage |

No storage or constructor is replaced. The canonical unit has no CRT
initializer. A comment-only reader-status update preserves its complete
**20 nondebug sections and 30 relocations**, including public symbol offsets,
section flags and all bytes. The retained whole-object pin comes from the
pre-comment object; private equality evidence is
`work/connections46/readbits/comment-proof.json`, with no filename exceptions.

After loading the selected mask, the reader adds count to used bits with
32-bit wrap and uses a signed comparison against 32. Below the boundary it
publishes the new count and extracts high-order bits. At or beyond the boundary
it publishes the remainder **before** dereferencing the next input word. It
then byte-swaps that word and publishes cursor/word. Even an exact-boundary
read eagerly dereferences the following word. The constructor resets this
shared state, so constructing another object redirects reads through the first.

Retail loads the old word before its indexed mask read; the candidate source
orders these reads differently. The finite alias/fault observations below
agree, but this is not a concurrent-access, asynchronous mutation, or complete
fault-register equivalence claim. No zero-count shortcut or bounds guard is
added that would alter retail's reads.

## Functional build review

The standalone candidate has only the four existing canonical global imports;
it introduces no new allocator, thunk or forwarding wrapper. Its complete
nondebug object is also compiled with `tools.build_game.compile_source` and
compared with the object exercised by the native test. Ordinary VC6
`vc6_o2_mt` flags are used, without pragmas or per-source special options.
Admission is individually reviewed nonexact source; linkage and full game
startup/menu/racing completion remain separate checks.

[`tests/test_bitstring_read.py`](../tests/test_bitstring_read.py) checks **42
complete strict bodies/initializers** across canonical `GR_BitString.cpp`,
`Lzw.cpp` and `MemTag.cpp`. This includes the actual 113-byte constructor,
28-byte destructor, 71-byte dictionary reset, 106-byte bank allocator and
522-byte decoder. The source-only decoder links complete actual units, including
real allocation tracking and the native VC6 CRT. The already-reviewed bit
shift pair is linked for the actual class's vtable closure but is not invoked
by these reader/decoder cases. Its separate validation remains
[documented independently](BITSTRING_SHIFTS.md).

### Native reader comparison

The retained reader fixture executes the entire emitted candidate and
unmodified hash-pinned retail reader/constructor: **205,632 cases**.

| Matrix | Cases |
| --- | ---: |
| 32 patterns, used0..31, count0..32, four byte alignments | 135,168 |
| Five mask/state/lookahead alias forms, eight patterns, all used/count | 42,240 |
| Invalid pointers and zero through three readable bytes before a guard page | 16,896 |
| Null/address1 receiver, all used/count | 2,112 |
| 384 real-constructor runs, 24 chained reads each, second object reset midway | 9,216 |

An independent MSB-first per-bit oracle checks ordinary and chained return
values plus cursor/word/used state. Comparisons include full prepared input,
object bytes and readable guard-page storage, completion, AV code, access kind
and address, and shared state. Valid alias cases must complete without an
exception; invalid mask cases must read-fault; invalid lookahead pointers must
remain unaccessed below the boundary. Boundary-fault assertions independently
require remainder publication with the old word retained. These assertions
prevent matching unwanted faults from passing as successful behavior.

Native and reference objects/storage have different addresses. Only known
pointer-valued identities for word/used/mask storage are normalized; numeric
returns are never changed. Constructor cases verify retail vptr `0x00550f50`
and stable native vptr, then normalize that prepared object field before reader
execution. The reader does not inspect the vptr. No retail instruction, call,
CRT boundary, or global address is patched.

### Source-only LZW integration

[`lzw_source_decode.cpp`](../tests/fixtures/lzw_source_decode.cpp) executes the
real decoder, reader, constructor/destructor, dictionary reset/allocation and
`MemTag.cpp` allocation helpers on **2,051 packed streams**. No retail function,
replacement allocator, stub or redirected call participates in this executable.

The independent encoder emits a discarded 32-bit prefix, literals, EOS256,
flush258, width-bump257, existing dictionary codes and the next-code KwKwK case.
Its oracle stores complete forward strings, independently of the decoder's
parent-linked reverse stack. Three explicit examples include empty output,
`AAAAAA`, and `ABABBACCC`. A further 256 seeds × eight modes cover all literal
bytes, repeated prior-code use, widths9..15, two flush positions, and repeated
calls with previously allocated banks.

Every run compares exact output and all trailing/leading guards, unchanged
input, dictionary width/nextcode/bump state, and eager reader cursor/word/used.
It checks all **137 × 256 dictionary entries**, including the 49 entries beyond
the reset loop's 35,023-entry bound and each entry's three padding bytes.
Independent expected parent/character state persists across flushes and calls,
as it does in retail's reset behavior. Bank addresses must remain stable after
the initial allocation; actual allocator accounting must rise by exactly
`137*256*12` bytes once, then remain unchanged. The native test uses the real
tracker initialized by canonical CRT startup. Dictionary banks intentionally
remain owned by the decoder for subsequent calls.

The packed streams are bounded successful inputs, with readable eager
lookahead padding. They do not validate malformed/cyclic dictionaries,
arbitrary code-width escalation, input/output exhaustion, allocation failure,
or constructor faults. Reader counts outside0..32 and initial used counts
outside0..31 are unclaimed. CPU register/stack context and fault resumption are
not compared; the prepared x86 alias cases are not a general C++ arbitrary-
pointer or concurrency guarantee. No actual compressed texture asset, renderer,
menu session or complete game run is claimed by these tests.

## Reproduce

```powershell
$env:VC6_ROOT=(Resolve-Path work/vc6sp3-disc).Path
$env:MCM2_EXE=(Resolve-Path work/game/mcm2.exe).Path
python -m unittest discover -s tests -p test_bitstring_read.py -v
```

The retail image is private and pinned to SHA256
`31fde4cc686a5ee89ef9095b90235325b195596867ecacefe511263e1509b874`.
Only the native fixture drivers use `/Od`; every candidate and actual helper
object uses the authentic optimized game profile.
