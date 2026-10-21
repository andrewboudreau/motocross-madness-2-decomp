# Texture loader to LZW decoder connection

The two compressed-image paths in `PCTextureMap::LoadSurface` now call the
existing declaration in `Lzw.h`. The former local `void*/void*` declaration
produced a different C++ symbol from the actual `unsigned char*/const void*`
provider. The output pointer conversion and added input constness preserve
the original x86 arguments; no wrapper, replacement decoder or storage is added.

The complete 2,129-byte loader at `0x004c6080` remains strict under authentic
VC6 SP3 `vc6_o2_mt`. Its mip-level call at `0x004c63d1` and single-image call
at `0x004c6717` each pass decoded byte count, compressed input, and output on
the stack, in that order. The decoded count is bytes per pixel multiplied by
the corresponding dimensions. Both use cdecl and ignore EAX. Their 16-byte
stack cleanup also removes the preceding BytesPerPixel argument.

The complete 522-byte provider at `0x004a03d0` remains strict. Its first read
discards the 32-bit stream prefix, and later reads use the current code width.
The declaration connection alone does not supply the separately missing
`GR_BitString::ReadBits` method.

Before/after compilation at the same source paths preserves all **164 nondebug
sections and 512 relocations** across the canonical loader and its near-miss
aggregate. The only permitted destination substitution is:

```text
?ExpandLzw@@YAXPAX0H@Z -> ?ExpandLzw@@YAXPAEPBXH@Z
```

Every payload byte, section flag, relocation addend and internal destination
is retained. The sample's seven sections are unchanged without substitutions.
The shared header includes the actual `Lzw.h` declaration, preserving its
visibility to other header consumers. The complete transitive menu consumer,
`ProCircuitProcs.cpp`, also retains all 334 sections and 2,083 relocations.
The retained regression checks all three complete-object fingerprints, both actual
call relocations, the primary slot-5 destination, all 27 strict canonical
texture cases and all 11 strict LZW cases. Portable fingerprints normalize only
the three exact source filenames; the private same-path proof excludes nothing.

The isolated real build `build-otes7syr` compiles all 250 translation units and
removes only this obsolete decoder symbol: **263 to 262 unresolved names**.
It still fails to link. This connection does not establish integrated texture
loading, DirectDraw surfaces or game startup. The separately reviewed
[bit reader](BITSTRING_READ.md) supplies native reader and source-only LZW
decoding evidence within its stated input and allocation limits.

```powershell
$env:VC6_ROOT = (Resolve-Path work/vc6sp3-disc).Path
$env:MCM2_EXE = (Resolve-Path work/game/mcm2.exe).Path
python -m unittest discover -s tests -p test_texture_lzw_connection.py -v
```
