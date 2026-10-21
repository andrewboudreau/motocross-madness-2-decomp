# Input and named race-child connections

CollisionObject, Terrain, VisibilityQuadTree and SoultreeQuadTreeRenderer now
use ControlInterface's existing `TestInputEvent` declaration. Their nine
calls pass the same event pointer and four cdecl arguments. This removes the
integer-event and void-pointer declarations of `CheckKey`, and the separate
integer-event declaration of `TestInputEvent`.

| Complete caller | Retail VA | Bytes | Predicate calls |
| --- | --- | --- | --- |
| CollisionObject::OnControlPress | `0x00434970` | 153 | 3 |
| Terrain::OnControlPress | `0x00508850` | 276 | 3 |
| VisibilityQuadTree::OnControlPress | `0x0052eec0` | 135 | 2 |
| SoultreeQuadTreeRenderer::OnControlPress | `0x005048d0` | 101 | 1 |

The predicate at `0x0043caa0` has a complete strict 63-byte body; its keyboard
modifier provider at `0x0048a240` has a complete strict 139-byte body. Event
control and device-kind comparisons short-circuit before the global game is
read. Matching events accept a missing keyboard, or ask the keyboard whether
the requested modifier holds. The event pointer is passed directly, with no
integer conversion or new wrapper. Names remain reconstructed semantics.

`tests/test_input_provider_connections.py` checks all six complete strict
bodies and the nine decoded retail call destinations. Its native fixture
executes 6,720 event/modifier scenarios, including absent keyboards, debug-key
gating, exact modifier masks, no-modifier masks and mismatched events. Both
complete ordinary C++ provider bodies are extracted for the fixture and
independently compared strictly alongside the production objects. Retail's
two functions and their direct call remain unpatched. Full game, control and
keyboard storage is checked for writes; independent ordinary expectations
also test short-circuiting with a null global game. Actual DirectInput device
creation, event dispatch and the complete debug handlers are not exercised.

The isolated real build `build-86pxtd8a` compiles 209/209 units and removes
exactly three names, **461 → 458**, without adding unresolved names. All 209
objects preserve 10,427 complete nondebug sections and 45,122 relocations
under those substitutions. Private evidence is
`work/game-build/input-round22-all-objects.json`.

## Named descendant switches

BikeRace's tick now calls `GameObject::DisableByClassName` and
`GameObject::EnableByClassName` through the actual `Game::childRoot` pointer.
The obsolete `UnknownBikeRaceRoot` view is removed. A friend declaration
allows BikeRace to use the existing protected methods without changing their
access or mangled names. The original access policy is not established.

Both providers have complete strict 308-byte bodies at `0x00468dd0` and
`0x00468f10`. The complete 5,980-byte tick, including its trailing table data,
also stays strict. Each provider has nine decoded direct calls in that tick;
each loads the primary root pointer at Game offset `0x34` without adjustment.
The providers traverse descendants, skip pending-release children, and match
the stored class-name list or RTTI name before invoking slots 4 or 5.

`tests/test_race_tree_connections.py` checks the full caller/providers and all
18 call routes. The native GameObject link fixture executes the actual
providers with nested children, nonmatching siblings, missing names,
pending-release children and stored class-name aliases. It does not execute
the complete BikeRace tick or actual sound/particle/shadow subclasses.

After the [Bike contact connections](BIKE_CONTACT_CONNECTIONS.md), real build
`build-5u6vd8ni` removes the two old root-method names, **456 → 454**, with no
new unresolved names. All 209 objects again preserve the same complete
sections and relocations under only those two substitutions. Evidence is in
`work/game-build/race-tree-round22-all-objects.json`.

Shared declarations change Terrain Load's compiler-local EH label to
`$L6028`, still section 29 offset 182. The named fingerprint is refreshed only
after the complete object audit proves unchanged bytes and destination.
The nonexact Load and QueryGround bodies remain partial reconstructions.
