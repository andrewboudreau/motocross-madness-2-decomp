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

The archive opener `0x004e9030` is a nonexact candidate in
`samples/render/ResourceManagerNearMisses.cpp`. It is a standalone source
containing only `AddArchive`; it does not include the canonical `.cpp` or
redefine the global manager. Independent review accepted it for explicit
reviewed-nonexact admission through `config/game_build.json`; its source
remains in `samples/` and is not a strict-match provider.

## Functional build review

The complete retail extent is 804 bytes. Authentic VC6 SP3 emits 798 bytes
with `vc6_o2_mt` and `vc6_o2_ml`, 795 with their `g6` profiles, and 670 with
`vc6_o1_mt` / `vc6_o1_ml`. None is a strict match. The O2 body preserves the
decoded branches, calls and arguments; retail additionally keeps the result
in the `new` temporary's stack slot and returns through that slot, with a
corresponding final store/pop scheduling difference. The reference comparison
finds no missing or extra call targets, address references or immediates.
This supports a complete candidate, not a claim of byte identity.

Retail FuncInfo `0x005632f8` has two unwind states, each leading to `-1`,
and no try blocks. The cleanup funclets at `0x0054daa0` and `0x0054dab7`
release the line-212 stream allocation and line-261 item allocation when
their constructors throw. The candidate has the same unwind structure.
No forced stack homing, extra cleanup, assembly or extent trimming is used.

`test_resource_archive_behavior.py` compiles the standalone source and a
native fixture with VC6, verifies that the sample defines only the one
method and no manager storage, and checks its authoritative 798-byte COFF
extent and exception metadata. It hashes the private retail image, decodes
the full target and its exception handlers, and redirects only their
verified direct dependency calls inside private validation memory. The
retail exception handler uses the same linked VC6 runtime as the source.
Production never loads or executes this reference memory.

The fixture compares 17,292 finite scenarios, including:

- Empty and populated managers, spare and exhausted capacities, and archive entry
  counts `-1`, `0`, `1` and `3`.
- Open failure, invalid and lowercase archive headers, first/second header
  read failures and later entry-read failures.
- Flags `0`, `1` and `-7`, with both scripted `IsValidFile` results, checking
  the short circuit and normalized argument passed to `Open`.
- Failed table/name reallocations, duplicate entries, virtual release and
  destruction, and exceptions from each allocation/constructor position.

Each pair compares the result or exception, every manager byte, complete
arena contents (including retired allocations), allocation ownership, and
ordered calls with receiver identities, sizes, line numbers and payload
hashes. The diagnostic `__FILE__` basenames are checked and normalized:
retail names `ResourceManager.cpp`, while the candidate names its actual
sample source. Their diagnostic strings are not byte-identical.

These are scripted dependency tests. Constructors, allocation, file I/O,
merge behavior and release are controlled; this does not validate real
archive parsing end to end or arbitrary allocation failures. In particular,
a failed name realloc produces a null destination, which the scripted read
returns as failure and the merge transcript accepts. Real stream behavior
on that invalid destination is outside this test. Actual stream and resource
providers must still be tested together before claiming functional startup.

Two unusual retail behaviors are deliberately preserved: archive insertion
occurs only inside the capacity-growth branch, and entry read return values
are ignored. Failed table reallocations can still leave a nonnull return
stream and an increased item count. The fixture observes these outcomes;
it does not repair them.

Reproduce on native Windows with the private inputs installed:

```powershell
$env:VC6_ROOT = (Resolve-Path work/vc6sp3-disc).Path
$env:MCM2_EXE = (Resolve-Path work/game/mcm2.exe).Path
python -m unittest discover -s tests -p test_resource_archive_behavior.py -v
python tools/reference_diff.py --vc6-root work/vc6sp3-disc --exe work/game/mcm2.exe --va 0x004e9030 --json work/resource-archive-references.json
```

## Field names

From the allocations and loops in this file (tier 3 names; each declaration
keeps its `// +0xNN`): the manager's `itemCapacity`, `archiveCapacity`,
`itemCount`, `archiveCount`, `archives` and `items` (+0x00..+0x14, grown
by `DebugRealloc` in 0x004e9430, 0x004e96b0 and the 0x004e9030 near
miss), and ResourceItem's
`name`, `nameSize` and `object` (+0x08, +0x0c, +0x10). ResourceItem's
archive, offset and size (+0x14, +0x18, +0x1c) are also consumed by
FileStream.cpp. These field names describe the decoded uses.

## Function names

`0x004e8f40` is `MergeWithExisting` (it moves an item onto an existing
item of the same name), `0x004e9010` is `SetItemObject`, and `0x004e9030`
is `AddArchive`. The full header also names `FindItem` (`0x004e9360`),
`FindItemByObject` (`0x004e93f0`), `AddLooseFile` (`0x004e9430`),
`AddObject` (`0x004e96b0`), `RemoveArchiveItems` (`0x004e9830`) and
`ReleaseItem` (`0x004e9900`). These semantic names remain provisional.
Reduced declarations still differ in several return/argument types;
method naming does not establish complete C++ type identity across those
views.
