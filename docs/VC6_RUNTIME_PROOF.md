# VC6 SP3 / build-8447 runtime proof

Private bundle SHA-256:

    ce25eecdb4e0b55020847a32c9bd2b6449dbd82ceb3c83e7b4de8b27cc48b209

Retail mcm2.exe SHA-256:

    31fde4cc686a5ee89ef9095b90235325b195596867ecacefe511263e1509b874

## Result

The supplied private toolchain is demonstrated to be the correct VC6 SP3 /
build-8447 toolchain **family** used by MCM2, and its supplied multithread CRT
contains object code that is byte-identical to code linked into the retail
executable.

This is stronger than a version-string match. Object files are extracted
directly from the pinned \`VC98/LIB/LIBCMT.LIB\`, relocated to their observed
MCM2 addresses, and compared with the retail executable.

## Compiler/linker generation evidence

MCM2's decoded Rich header contains:

- product id 11, build **8447**, count **197** — the project's
  \`Utc12_CPP\` record;
- product id 4, build **8447**, count **2** — the project's
  \`Linker600\` record.

The supplied toolchain independently contains:

- \`C1XX.DLL 12.00.8472.0\`;
- \`C2.DLL 12.00.8447.0\`;
- \`C2.DLL\` internal build paths rooted at \`E:\\8447\\...\`;
- \`LINK.EXE\` banner \`6.00.8447\`.

## Fully relocation-resolved exact matches

All COFF relocations are applied to observed retail addresses, then every byte
is compared. No bytes are ignored.

| LIBCMT symbol | Retail VA | Size | Relocations | Result |
|---|---:|---:|---:|---|
| \`_malloc\` | \`0x0053789d\` | 18 | 2 | 18/18 exact |
| \`__nh_malloc\` | \`0x005378af\` | 44 | 2 | 44/44 exact |
| \`__heap_alloc\` | \`0x005378db\` | 78 | 6 | 78/78 exact |
| \`_free\` | \`0x00537929\` | 72 | 7 | 72/72 exact |
| \`__msize\` | \`0x005351f0\` | 69 | 6 | 69/69 exact |

**281/281 bytes exact after applying 23 linker relocations.**

## Larger supporting CRT matches

For these larger routines, every byte outside an explicit COFF relocation field
is compared directly. Relocation fields alone are excluded because the linker
necessarily rewrites those addresses.

| LIBCMT symbol | Retail VA | Body | Relocation bytes | Compared | Result |
|---|---:|---:|---:|---:|---|
| \`___sbh_find_block\` | \`0x0053b67f\` | 43 | 8 | 35 | 35/35 exact |
| \`___sbh_free_block\` | \`0x0053b6aa\` | 811 | 92 | 719 | 719/719 exact |
| \`___sbh_alloc_block\` | \`0x0053b9d5\` | 777 | 36 | 741 | 741/741 exact |
| \`__callnewh\` | \`0x00544e78\` | 27 | 4 | 23 | 23/23 exact |

That is another **1,518/1,518 directly compared bytes**, covering 1,658 bytes
of function bodies. Combined with the fully resolved set, **1,799 independently
compared bytes are exact** across nine CRT functions.

## Negative control: specifically the multithread CRT

The supplied single-thread \`LIBC.LIB\` does not have the retail shape:

- single-thread \`_free\`: 47 bytes; MCM2 / \`LIBCMT\` \`_free\`: 72 bytes;
- single-thread \`__msize\`: 41 bytes; MCM2 / \`LIBCMT\` \`__msize\`: 69 bytes.

The extra MCM2 instructions are the same lock/unlock paths present in the
supplied \`LIBCMT.LIB\`. This distinguishes the linked CRT variant instead of
accepting any VC6 runtime as a match.

## Reproduce

After installing the private bundle:

    python3 tools/with_private_env.py -- \
      python3 tools/verify_vc6_runtime.py

The machine-readable report is written to ignored
\`work/vc6-runtime-proof.json\`.

The archive parser itself is input-free:

    PYTHONPATH=. python3 -m unittest discover -s tests \
      -p 'test_coff_archive.py' -v

## Scope

This demonstrates the VC6 SP3/build-8447 **toolchain generation** and identical
shipped multithread CRT object code in MCM2. It does not recover every original
per-translation-unit compiler switch. Optimization, CPU, runtime-library,
exception/RTTI, and per-file settings still need authentic-compiler calibration
against reconstructed game functions. That is now a flags/source-shape problem,
not a remaining toolchain-family identification problem.
