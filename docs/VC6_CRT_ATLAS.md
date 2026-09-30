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

Use admitted rows to avoid reconstructing CRT code as game source. Gaps inside
the span remain unknown. Application accounting wrappers remain in scope.

```bash
python3 tools/with_private_env.py -- python3 tools/verify_vc6_runtime.py
make vc6-crt-atlas
```

Outputs: `work/vc6-runtime-proof.json` and `work/vc6-crt-atlas/atlas.json` plus
reports, with input/library hashes. Static comparison requires neither Wine nor
execution of game code.
