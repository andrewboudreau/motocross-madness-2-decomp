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
