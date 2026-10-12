# ResourceManager.cpp

`src/reconstructed/ResourceManager.h` / `ResourceManager.cpp`. Evidence:
the `__FILE__` literal `D:\aardvark\VC\krusty2\ResourceManager.cpp`
(`0x00572b64`; lines 24, 42, 135/139, 387–420, 464–478), RTTI
`ResourceItem : BaseObject` (vtable `0x00557858`, 0x20 bytes) and the
"RS2" archive tag (`0x00572b90`). The file runs from `0x004e8d30` (the
initializers of the global manager at `0x00689c78`, which `0x00572b44`
points to) to `0x004e9960`, an out-of-line copy of an inline
`UnknownTextureStream` method; SceneManager.cpp starts at `0x004e9980`.
Names other than ResourceItem are provisional.

The manager keeps named items (ResourceItem: a name, the archive stream
holding the file and its offset, or the object loaded from it), opens
"RS2" archives and adds their items, finds items by name or object, and
releases archives. `ResourceManager.h` is the full class;
`UnknownResourceManager.h` stays the reduced view other files include,
because declaring the destructor and fields there flips a register choice
in TrackGame `0x00520ab0` (see [SCENEMANAGER](SCENEMANAGER.md)). The two
headers must not be included together. Two methods have different
declared return types in the two headers (`0x004e9360`, `0x004e96b0`);
both bind to the same retail address.

Exact: 19 calibration cases. Near miss
(`samples/render/ResourceManagerNearMisses.cpp`): the archive opener
`0x004e9030`, where retail also keeps the result in the `new` temporary's
stack slot.

## Field names

From the allocations and loops in this file (tier 3 names; each declaration
keeps its `// +0xNN`): the manager's `itemCapacity`, `archiveCapacity`,
`itemCount`, `archiveCount`, `archives` and `items` (+0x00..+0x14, grown
by `DebugRealloc` in 0x004e9430, 0x004e96b0 and the 0x004e9030 near
miss), and ResourceItem's
`name`, `nameSize` and `object` (+0x08, +0x0c, +0x10). ResourceItem's
archive and offset (+0x14, +0x18) keep their placeholders because
FileStream.cpp (UI lane) reads them.

## Function names

`0x004e8f40` is `MergeWithExisting` (it moves an item onto an existing item
of the same name; tier 3). `0x004e9010` is `SetItemObject` in the lane's files
(PCAudio.cpp and Palette8.cpp keep their own address-named declarations);
`0x004e9030` keeps its address name because SceneManager.cpp and the
FileStream sample reach it through headers other lanes share.
`0x004e93f0` is `FindItemByObject` (returns the item that records an object)
and `0x004e9900` is `ReleaseItem` (both tier 3). `0x004e9360`, `0x004e9430`,
`0x004e96b0` and `0x004e9830` keep their address names: SceneManager.cpp also
sees the address-named declarations in UnknownResourceManager.h and
Parameterblocks.h, so the lane-only rename does not compile there.
