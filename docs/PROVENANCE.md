# Code and library ownership

Distinguish game reconstruction from external implementations without assigning
ownership from proximity or shared call targets. `tools/build_provenance.py`
reads imports/IAT slots, packaged DLL metadata, source strings, RTTI uses and
bounded decoded candidate ranges. It never loads the game or DLLs.

```bash
make provenance
python3 tools/build_provenance.py --exe /private/game/mcm2.exe
python3 tools/build_provenance.py --address 0x004a30c0
```

Python 3.10+ and GNU objdump are required. Generated `analysis/provenance/`
contains `provenance.json`, `imports.json`, `components.json`, `source_units.json`
and `REPORT.md`, bound to input/tool hashes. Use `--game-dir` for separately
stored DLLs. Unknown builds fail unless explicitly allowed for exploration.

## Established boundaries

- Packaged blade.dll identifies itself as Microsoft Blade Software Rasterizer
  and supplies DirectDrawEnumerateA/DirectDrawCreateEx.
- The d3drm.dll import is D3DRMVectorRotate; a helper import does not establish
  use of the Retained Mode renderer.
- DSETUP, EBUEula, lang, SETUPENU and uilang are packaged support DLLs outside
  the EXE's normal import table. Their runtime roles are not established here.
- Common deleting-destructor target `0x004a30c0` performs application-side
  [allocation accounting](ALLOCATION.md) and remains reconstruction work.
- Selected lower allocator routines are confirmed VC6 multithread CRT objects.
  The [CRT atlas](VC6_CRT_ATLAS.md) provides address-specific library evidence;
  the provenance generator does not automatically import it.

## Remaining uncertainty

Candidate entries and reachable ranges are heuristic, not a complete function
inventory. Decoded coverage is not decomp completion. Embedded paths may come
from headers/inlined code and do not establish original TU ranges.

Ownership and code role are separate: compiler-generated destructors can belong
to game classes; calling an API does not make the caller Microsoft code. IAT
slots are local pointer storage, not DLL addresses. Preserve shared/overlapping
ranges and unknown providers. Resolve COM interfaces, dynamic loads and switch
boundaries before claiming complete ownership or external call graphs.

Format reference: [Microsoft PE/COFF](https://learn.microsoft.com/en-us/windows/win32/debug/pe-format).
