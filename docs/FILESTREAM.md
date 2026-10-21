# File stream helpers

Source: `src/reconstructed/FileStream.cpp` (the exact methods) and
`samples/io/FileStream.cpp` (the near misses), each with its bindings. The
class is `UnknownTextureStream`, declared in `src/reconstructed/FileStream.h`
and included by `TextureMap.h`.

## Evidence

- The code runs from `0x00460d10` to `0x00461d57`.
  - Before it: the FastMath helpers.
  - After it: the shadow fill code at `0x00461d60` and `0x00461e60`, which
    is inline assembly. `0x00461d60` (to `0x00461e58`, `ret 0xc`) has an
    ebp frame and two floor-division blocks that reload their operands
    from homed stack slots into fixed esi/edi and clear edx with
    `xor edx, edx` (not `cdq`) before `idiv`.
- There is no `__FILE__` literal and no RTTI. The file name and every
  member name are ours (tier 3).
- Callers include Track, SceneManager, Motnctrl, CarProcedural,
  ResourceManager and TGAFile.
- A stream reads either:
  - a file directly, or
  - a slice of an inner stream at `+0x1c`. The slice starts at `+0x130` and
    its length is at `+0x04`.
- Encoded files start with "FAOE". Their bytes go through a running key at
  `+0x01`, seeded from the file name.

## Exact (13), `src/reconstructed/FileStream.cpp`

| VA | Size (bytes) |
|---|---|
| `0x00460d10` | 77 |
| `0x00460d60` | 15 |
| `0x00460d70` | 57 |
| `0x00460e70` | 209 |
| `0x00460f50` | 957 |
| `0x00461310` | 39 |
| `0x00461600` | 63 |
| `0x00461980` | 224 |
| `0x00461a60` | 56 |
| `0x00461aa0` | 229 |
| `0x00461cb0` | 107 |
| `0x00461d20` | 30 |
| `0x00461d40` | 24 |

Source forms that mattered:
- **`0x00460d10`:** the constructor names the buffer pointer
  (`char* buffer = field_0x21;`) after the header bytes and before the
  argument store; only then does VC6 load the argument at entry (edx) and
  keep the buffer address in ecx for both pointer stores.
- **`0x00460e70`:** the NULL test guards the rest, so its message is
  emitted last.
- **`0x00460f50`:** three forms mattered.
  - Each branch (read, write) declares its own message buffer. With one
    buffer for the function, VC6 cross-jumps the write branch's
    header-failure and "filename is NULL!" blocks into their read-branch
    twins; retail keeps them apart.
  - The key loop reads the byte into a `char` first
    (`char c = *p++;`), as in `0x00461b90`.
  - Retail clears only bytes 1..7 of the access string before strncpy
    overwrites it (`memset(access + 1, 0, sizeof(access) - 1)`).
- **`0x00461980`:** the decode reads the byte into a `char` temporary and
  the key straight from the field: `c = *p; decoded = key ^ c; key += c;
  *p = decoded;`. That is the one spelling under which VC6 loads the key
  first and copies it, as retail does; naming the key in a temporary makes
  it copy the byte instead.
- **`0x00461aa0`:** two forms mattered.
  - `#pragma inline_depth(0)` keeps the end-of-stream test `0x00430ff0` out
    of line.
  - `count < size - 1` replaces a decremented size.
- **`0x00461cb0`:** returns `_fstat`'s result, not void. `TextureMap.h` and
  `SceneManager.bindings.json` now use the `int` signature.

## Near misses (5), `samples/io/FileStream.cpp`

The notes are in the source header.

| VA | Retail bytes | Candidate bytes | Masked match | Function |
|---|---|---|---|---|
| `0x00460db0` | 186 | 176 | 16% | header check |
| `0x00461340` | 702 | 696 | 46% | seek |
| `0x00461640` | 663 | 670 | 71% | read |
| `0x004618e0` | 152 | 152 | 88% | write |
| `0x00461b90` | 282 | 282 | 90% | write the header |

Each miss comes down to one of:
- register assignment, or
- instruction scheduling (the 0xfa seed store and the name pointer in
  `0x00461b90`, the argument loads in `0x004618e0`).

The control flow of each one is reconstructed. None is a strict byte match.
Candidate extents above are measured from authentic VC6 SP3 COFF metadata with
`vc6_o2_mt`; masked percentages are diagnostic only. All five are registered as
`expect: partial` in `samples/io/targets.json`.

## Functional build review

The source build explicitly includes `samples/io/FileStream.cpp` as
`reviewed-nonexact`. Its five methods complement the thirteen exact methods in
`src/reconstructed/FileStream.cpp`; the candidate stays in `samples/`.
It supplies the missing `Read` and `Seek` bodies used throughout texture,
scene, terrain and audio loading, plus the header checks and writes needed by
`Open`. This is source linkage progress, not a successful game startup.

Review compared the complete decoded retail extents with the source, including
branch predicates, signed arithmetic, field widths, dependency calls, return
values and buffer/key mutations:

| Method | Confirmed behavior retained |
| --- | --- |
| `CheckHeader` | Seek to zero, disable decoding while checking signed header bytes in FAOE order, restore the encoded flag on mismatch/success, and close/null the file on either seek failure. |
| `Seek` | Follow archive links while combining the reset flag with each slice offset; honor the same-position fast path; reject nonzero END and negative CUR requests when decoding; restart or preserve the running key and decode forward in 1024-byte blocks; preserve early-return buffer state and file-close behavior. |
| `Read` | Follow the inner stream; consume and decode its buffer in place; preserve the unsigned remainder/division in the partial-buffer path; use 1024-byte plain reads; retain the retail short-read return values, including `count - read` and `count - 1` on their respective branches. |
| `Write` | Follow the inner stream; resynchronize encoded output through `ftell`/`fseek`; close/null on seek failure and still pass that resulting file pointer to the encoding helper, as retail does; preserve helper and plain-write returns. |
| `WriteHeader` | Write four signed header bytes with early failure exits, set encoded/header-size fields, split the filename, append its extension and derive the wrapping byte key from seed `0xfa`. |

The sample's `WriteXorEncoded` declaration now uses the canonical
`const unsigned char*` input and `unsigned char*` key types from
`EcoSystem.cpp`. Explicit casts at the stream boundary preserve its call
arguments. The subsequent shared inline ResetTextMode consolidation can
change Seek's emitted instructions; the five methods remain reviewed nonexact
candidates and are checked behaviorally rather than claimed byte-identical.

The native VC6 differential test runs **12,390 scenarios** against the
hash-pinned private executable. It compares returns, every byte of all three
stream records, output bytes, written payloads, FILE flags, file position/close
effects, and ordered dependency calls. Cases include direct and nested archive
views with distinct file identities and keys, buffered and unbuffered reads,
zero-size/count requests, exact 1024/2048-byte requests and partial blocks,
encoded/plain paths, all seek origins, same-position seeks, successive
dependency failures, header-byte mismatches and filename variations.

Both sides use the same deterministic file/encoding boundary mocks. Only the
five reference bodies are executed from private PE pages; decoded direct calls
to the twelve known dependencies are redirected to the fixture. ResetTextMode
uses its independently strict-verified shared accessor, including recursion;
the other dependencies use mocks. The source
test object substitutes only external CRT dependency names at compile time.
This fixture is validation-only and is never linked into the source game.
It does not exercise real disk errors or prove all possible inputs; malformed
stream graphs and negative-length decoding loops are outside the test domain.
The decoded source review and partial byte reports remain separate evidence.

No new stream hierarchy is asserted. This nonvirtual `0x134`-byte view has no
RTTI. Other named stream views remain separate reconstruction work.

## Constructor connection

The shared declaration in `FileStream.h` (included by `TextureMap.h` and
`EcoSystem.h`) takes
`UnknownResourceManager*`. This is a type inference supported by the constructor
storing its argument at `+0x12c`, `FindArchive` calling `FindItem` through that
field, and callers supplying `g_ResourceManager` or the resource manager's
`this` pointer. It connects EcoSystem's existing constructor calls to the
supplied body at `0x00460d10`; no overload or forwarding stub is introduced.

Callers no longer cast resource-manager pointers to `int`. The two TGA entry
points retain their separately provisional integer parameter and cast it at
the stream-construction boundary. The stream's layout and constructor stores
are unchanged; its exact comparison includes the complete emitted extent and
resolved relocations. The original class name remains unknown. The resource
manager consumers now share its [canonical declaration](RESOURCEMANAGER.md#existing-provider-connections).

The parameter-reader stream view is an alias of this shared declaration.
`ResetTextMode` is now a recursive inline accessor alongside GetTextMode and
SetTextMode. SelectSection preserves its strict 573-byte body and naturally
emits the accessor's complete, strict 30-byte copy in Parameterblocks.cpp;
the calibration check follows that emitted copy. This does not assign original
TU ownership. Parameterblocks now uses the shared ResourceManager declarations,
with the same archive and offset fields. FindItem and AddLooseFile resolve to
canonical providers; AddArchive is supplied by the reviewed nonexact
[archive loader candidate](RESOURCEMANAGER.md#functional-build-review).
The native stream differential test
still passes all 12,390 scenarios with the shared accessor; the fixture no
longer supplies its own duplicate reset body or records that internal accessor
as an external side effect. All stream-state bytes still participate in the
differential comparison.

```powershell
$env:VC6_ROOT = 'C:\path\to\vc6sp3'
$env:MCM2_EXE = 'C:\path\to\mcm2.exe'
python -m unittest discover -s tests -p test_filestream_behavior.py -v
```

## Reproduce

```bash
PYTHONPATH=. python tools/run_calibration.py --compiler vc6 --vc6-root "$VC6_ROOT"
```
