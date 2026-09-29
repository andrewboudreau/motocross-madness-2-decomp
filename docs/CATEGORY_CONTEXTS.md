# Category call-context atlas and agent packets

## Deliverable

This is an additive extension to `docs/CATEGORIES.md` and the existing
`make categories` anchor-level map on main. Its CLI and outputs remain intact.
The new pass adds bounded, one-hop call contexts, vptr-store clues, reverse
source lookup and per-category agent packets in a separate output directory.

A reproducible category-to-code-to-source index and 15 focused agent packets.
This extends the allocation evidence into useful subsystem navigation without
turning accounting labels into invented ownership or original source folders.
The canonical implementation is `mcm2tool/category_contexts.py` plus
`tools/build_category_contexts.py`; measured findings are in `CATEGORY_CONTEXTS_FIRST_PASS.md`.

```text
literal category argument -> decoded category-selection call
                            -> containing function candidate
                               -> literal source-path references
                               -> RTTI / vptr-store clues
                            -> bounded local call window
                               -> one-hop callee source/class clues
```

This pass independently reproduces **52/55 resolved selector calls**, across **15 categories**,
with links to **29 source/header filenames**. The older allocation recognizer
still reports 45/55: this pass additionally understands register loads and
positive ESP-relative stores that do not overwrite the pending argument.
It does not change the older measurement or claim seven new categories.

## Evidence levels, not an automatic reorganization

1. **Literal argument:** a decoded push-immediate supplies the category string.
   Backward tracing stops at stack changes, unmodelled writes, calls, gaps,
   branch entries and lookback limits. Possible alias stores are not waved away.
2. **Containing-candidate association:** a candidate CFG contains the selector
   and independently references a source pathname or class vtable. A function
   can touch multiple categories. Neither path nor class clues establish the
   exact original translation unit or method identity.
3. **One-hop context:** a direct callee is reachable after selection and before
   a local select/restore boundary. This is a navigation hint only. Unknown
   callees can change global category state, and traversal does not model
   exception edges or resolve indirect callees.

No label is propagated through a callee's callees. Known allocation support,
import boundaries and recognized compiler artifacts are not added to the
category work queue merely because application code calls them. Unidentified
shared helpers retain every observed context and an unassigned owner.

The tool leaves the existing source skeletons, function manifest, historical
match counts and provenance ownership labels unchanged.

## Concrete useful results

**Terrain:** candidate `0x005079f0` contains a Terrain selection at `0x00507a29`,
references `Terrain.cpp`, and writes a Terrain vtable. Two other direct selector
containers, `0x00506220` and `0x00507610`, occur in Terrain vtables. This is a
strong pilot for coherent reconstruction, not proof of original method names.

**EcoSystem:** candidate `0x0045aad0` has an EcoSystem selector, EcoSystem RTTI
usage and `EcoSystem.cpp` references. Yet `EcoSystem.cpp` also appears in a
Collision-selector candidate (`0x00457ed0`). File membership is not one-to-one
with accounting categories.

**TextureCache:** its selector window reaches `0x005113d0`, whose body references
`TextureMapManager.cpp`. This is a one-hop source-navigation link, not a proven
runtime scope invariant or complete module assignment.

**Audio:** one-hop callees reference `AuralScape.cpp` and `PCAudio.cpp`; **UI**
selector containers include GUIManager vtable entries, and one-hop callees
reference `krustyui.cpp`. These are useful entry points for agent investigation.

**Cross-cutting setup:** candidate `0x004de590` contains 21 selector calls and
references `QuarryStuntEvent.cpp`. It is not classified as 21 different source
modules. Candidate `0x00521050` references `trkgame.cpp` while selecting Audio,
Scene, Startup and UI. Shared setup/orchestration must remain distinguishable
from the subsystem implementation it invokes.

## Run and query

After extracting the owned installer:

```bash
make category-contexts
make category-contexts-test

python3 tools/build_category_contexts.py --category Terrain
python3 tools/build_category_contexts.py --source EcoSystem.cpp
python3 tools/build_category_contexts.py --address 0x005079f0
```

Direct analysis can use an arbitrary working directory:

```bash
python3 /path/to/repo/tools/build_category_contexts.py \
  --exe /private/game/mcm2.exe --out /private/analysis/category_contexts
```

Python 3.10+ and GNU objdump are required for live analysis. No game/SDK/VC6 is
needed for the synthetic category tests. Wrong executable hashes and changed
reviewed selector/restore bodies fail before disassembly. `--config` supplies
an explicitly reviewed configuration; the default is this repository's known
MCM2 build. Neither game nor installer code is executed.

Queries read an existing snapshot. They include its input hash, use candidate
entry addresses rather than arbitrary interior addresses, and return exit 1
when there is no result. Missing evidence is not a negative ownership claim.
Category names and exact source basenames/full paths are case-insensitive.

## Generated files

All outputs are local under ignored `analysis/category_contexts/`:

- `context_map.json`: arguments, evidence instructions, candidate associations,
  one-hop calls, boundary stops, source references, class clues and tool hashes.
- `source_index.json`: reverse index from all 111 observed source/header paths;
  paths without category links stay visible with empty evidence lists.
- `work_queue.json`: 24 deduplicated direct-selector candidate review leads.
- `packet_index.json` and `packets/*.json`: one focused context packet per label.
- `REPORT.md`: the measured navigation summary, including unresolved arguments.

Queue ordering prefers matching category/class/filename clues, then candidates
with source and class clues, source only, class only, and finally selector only.
This is a scheduling heuristic, not an authorship confidence score. The
triangulated first pilots are EcoSystem `0x0045aad0` and Terrain `0x005079f0`.

## Validation in this change

The existing anchor mapper and its tests were preserved, not replaced. The
counts below cover the new context tests plus allocation regression, not the
entire repository or the concurrent anchor implementation.

- **34 category tests** pass, including argument overwrite, stack alias, branch
  entry, CFG boundary, cyclic flow, shared context, no transitive fanout, query
  behavior, wrong-input rejection and deterministic packet generation.
- **33 existing allocation tests** pass, including the native C++ model's 14
  behavioral scenarios. Those scenarios execute our model, not the game.
- Two complete live category runs produce **20 byte-identical output files**.
- The previous 21 allocation-range hash checks pass. Rerunning its strict probe
  comparisons still gives **1/7 exact**, with six calibration mismatches.
- No historical VC6 execution, new CRT identification or full-project smoke
  revalidation is claimed. This change adds source-navigation evidence, not
  new compiled game-function matches.

## Next development gate

Review the two triangulated pilots as coherent class/source slices: validate
full function boundaries, category save/restore and called constructors, then
reconstruct readable C++ with explicit field uncertainty. Independently compare
lower runtime functions against privately supplied VC6 libraries when available.
Neither workstream requires forcing these category labels onto original files.

## Primary technical references

- Microsoft x86 thiscall: https://learn.microsoft.com/en-us/cpp/cpp/thiscall
- Predefined macros / __FILE__: https://learn.microsoft.com/en-us/cpp/preprocessor/predefined-macros

These support calling-convention and macro interpretation, not MCM2-specific
ownership. Game-specific claims above come from the hash-bound local analysis.
