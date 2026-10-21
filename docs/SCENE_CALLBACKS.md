# Scene reader callbacks

`src/reconstructed/SceneCallbacks.cpp` supplies six previously missing symbols
used by the existing Scene readers. Every definition is an ordinary empty C++
body compiled with authentic VC6 SP3 and the normal `vc6_o2_mt` profile. Each
emitted function matches its complete three-byte retail body, including stack
cleanup. The provider has no imports, runtime data, initializers, or code/data
relocations. Existing headers, reader bodies, and binding names are unchanged.

| Existing declaration | Retail entry | Complete behavior | Direct calls |
| --- | --- | --- | ---: |
| `UnknownFunction4de580(const char*, const char*, const char*)` | `0x004de580` | `ret 12` | 25 |
| `UnknownFunction4de580(const char*, const char*, float)` | `0x004de580` | `ret 12` | 12 |
| `UnknownFunction4de580(const char*, const char*, int)` | `0x004de580` | `ret 12` | 9 |
| `UnknownFunction4de580(const char*, const char*, const Vector3*)` | `0x004de580` | `ret 12` | 1 |
| `UnknownFolded4de580(const char*, const char*, const char*)` | `0x004de580` | `ret 12` | 2 |
| `UnknownFunction464e80(const char*)` | `0x00464e80` | `ret 4` | 25 |

Each return is followed by thirteen NOPs before the next entry. There are no
receiver or argument reads, logging calls, output writes, or returned-value
assignments. The reconstructed definitions therefore add no such effects.
They remain separate functions: the shared retail return address does not prove
that the original methods were identical. Names, overload grouping, the
`UnknownFolded` distinction, and original translation-unit ownership remain
provisional. The new file groups these confirmed call contracts; it does not
claim an original filename.

## Receiver and call evidence

Scene's primary vtable is `0x0055786c`, with COL `0x0055ecc8`, object offset zero,
and literal RTTI name `.?AVScene@@`. Parsed Scene, GameObject, and BaseObject
relations all have PMD `(0, -1, 0)`. The complete 338-byte Scene constructor at
`0x004ea660` preserves its incoming ECX in ESI, calls GameObject on that same
pointer, writes the Scene vtable to `[esi]` at `0x004ea6f7`, and returns ESI.
The native fixture also checks Scene's `0x8cc` size and zero-offset GameObject
conversion.

The regression checks all 74 actual COFF call relocations against decoded retail
destinations in ten complete readers: ReadVector, ReadColor, ReadLight, ReadFog,
ReadEnvironment, OpenResourceFiles, ReadSounds, Load, CountObjects, and
CountTextures. Each saves the incoming primary Scene pointer and restores it to
ECX at these direct call boundaries. All 51 registered SceneManager bodies,
including its constructor and these ten readers, remain full strict matches.

Six representative instruction sequences independently check the argument
shapes. The callers push section/key/value or one formatted-message pointer;
the float occupies one four-byte stack slot, the integer example is a masked
flag, and the vector example passes the pointer loaded from the scene-light
record. No float-to-double promotion or secondary-object adjustment is added.
For example, ReadLight calls the string form at `0x004eb623`, vector form at
`0x004eb6c7`, integer form at `0x004eb70d`, float form at `0x004eb961`, and
message form at `0x004ebb76`; ReadEnvironment calls the separately declared
folded form at `0x004ec3c3`.

## Native validation

`tests/test_scene_callbacks.py` compiles the actual standalone provider and
links it unchanged with `tests/fixtures/scene_callbacks.cpp`. It verifies six
complete strict providers, all 51 existing SceneManager cases, the 74 call
boundaries, and **36,864 native calls**: 256 storage patterns, eight argument
patterns, three receiver arrangements, and six methods. Every call checks the
complete prepared Scene-sized storage and adjacent guards. Receiver arrangements
are prepared raw storage, a no-access page, and null; arguments include null,
prepared storage, and unreadable pointers. Unexpected memory access is a test
failure. The eight float arguments are finite bit patterns; this is not a
floating-point exception-context test.

These are deliberately bounded tests of the emitted no-access bodies, not
constructed C++ Scene lifetimes, an executed scene loader, or a working game.
The full retail matches establish each return instruction's behavior; native
execution checks the actual source-defined calling boundaries. Other unresolved
scene/game dependencies remain outside this slice.

The isolated real build `build-l12kgew3` compiles 240 translation units and
removes exactly these six unresolved names, **290 to 284**, with none added.
The subsequent combined build and complete-object comparison are recorded in
[GAME_BUILD.md](GAME_BUILD.md). The source-built game still does not link.

```powershell
$env:PYTHONPATH = '.;tests'
$env:VC6_ROOT = (Resolve-Path work/vc6sp3-disc).Path
$env:MCM2_EXE = (Resolve-Path work/game/mcm2.exe).Path
python -m unittest test_scene_callbacks -v
```
