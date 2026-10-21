# Soultree hierarchy loading, cloning and reset connections

The rendering declaration of SoultreeObject now names five primary virtual
methods consistently with the existing soultree.cpp implementations. Its
D3DIMSoultreeObject overrides use the same names, preserving the override
relationships and slot order. These are declaration and call connections;
none of the provider algorithms changes. Semantic names remain inferred.

| Primary slot | Shared method | Base entry / complete bytes | Rendering override / complete bytes |
| ---: | --- | --- | --- |
| 2 | LoadHierarchy | `0x4fdc00` / 433 | `0x43f950` / 1,257 |
| 3 | LoadHierarchyFromParameters | `0x4fddc0` / 605 | `0x43fe40` / 543 |
| 6 | ResetLocalMatrices | `0x4fec70` / 83 | `0x4450c0` / 280 |
| 7 | CloneFrom | `0x4feb10` / 24 | `0x444560` / 1,234 |
| 8 | CopySubtree | `0x4feb30` / 252 | `0x444b60` / 263 |

All ten complete bodies match retail strictly with relocations resolved.
Each override has one direct call to the corresponding base implementation.
Retail primary tables `0x557c18` and `0x5513ec`, emitted table relocations,
constructor layouts and RTTI agree on these slots and the unchanged primary
receiver. GameObject is a separate base at `+0xc`; it is not the receiver for
these hierarchy operations. No adjustor thunk or forwarding wrapper is added.

## Stream boundary and retained behavior

LoadHierarchy now accepts the existing UnknownTextureStream declaration and
its eleven Read sites call the supplied provider at `0x461640`. The stream
pointer, destination buffers, item sizes and counts remain unchanged. The
private SoultreeFileStream stand-in is removed. This does not make Read exact:
its existing [reviewed nonexact implementation](FILESTREAM.md#functional-build-review)
stays in `samples/io/FileStream.cpp`. The separate native stream suite checks
12,390 scenarios, including nested streams, buffering, decoding and failures.

Binary loading retains its 256-entry local node array, virtual node factory,
field reads, parent-index allocation and hierarchy attachment. It still
ignores the Read return values; no new input validation or cleanup is implied.
Parameter loading retains its row order, parent-name matching and saved-parent
restoration. Cloning clears the destination's parent, copies node fields and
recurses through later siblings and children using the same virtual factory.
Reset clears world validity and installs local identity matrices recursively;
the rendering override still rebuilds the LOD vertex positions before invoking
the base reset. The nonadmitted race sample's virtual reset call uses the same
slot and receiver after the name change.

## Verification

Fresh same-path VC6 compilations of published `799f6e9d` and the changed source
preserve all **448 nondebug sections and 2,939 relocations** in the canonical
scene/model units and both D3DIM and BikeRace sample aggregates. Bytes, section
attributes, relocation addends and destinations are unchanged under the
explicit method/stream name substitutions. No instruction or filename
normalization is needed. Private evidence is in
`work/game-build/soultree26/{objects,renames}.json`.

`tests/test_soultree_hierarchy_connections.py` checks **116 complete strict
bodies**, all five direct base calls, eleven stream reads, both primary table
slot sets, RTTI and actual header layouts. Separately compiled rendering and
physics views produce identical dispatch code for all five slots and preserve
the `+0xc` GameObject conversion. Two complete nonexact rendering sample bodies
retain their independently captured resolved fingerprints.

The isolated real build `build-jkbvgguc` compiles **214/214** translation units
and removes exactly six unresolved names, reducing **393 to 387**, without
adding new unresolved names. This establishes source linkage connections.
Integrated scene loading, menus and racing still require a linked executable
and runtime tests.

```powershell
$env:PYTHONPATH='.'
$env:VC6_ROOT=(Resolve-Path work/vc6sp3-disc).Path
$env:MCM2_EXE=(Resolve-Path work/game/mcm2.exe).Path
python tests/test_soultree_hierarchy_connections.py
python tests/test_filestream_behavior.py
python tools/build_game.py -j6
```
