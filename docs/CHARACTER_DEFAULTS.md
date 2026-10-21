# Character primary virtual defaults

`src/reconstructed/CharacterDefaults.cpp` supplies five individually identified
default methods as ordinary empty C++ definitions. They reproduce complete retail
bodies under authentic VC6 SP3, with no imports, data, initializers, assembly or
retail dispatch. The original translation unit and unused argument types remain
unproven. Shared return instructions alone do not establish method identity.

| Primary slot | Existing source method | Retail VA | Complete bytes |
| --- | --- | --- | --- |
| 0 | `CharacterVirtualSlot0()` | 0x00464e90 | 1, `ret` |
| 2 | `CharacterVirtualSlot2()` | 0x00464e90 | 1, `ret` |
| 3 | `CharacterVirtualSlot3(int)` | 0x00464e80 | 3, `ret 4` |
| 9 | `CharacterVirtualSlot9(int)` | 0x00464e80 | 3, `ret 4` |
| 10 | `CharacterVirtualSlot10()` | 0x00464e90 | 1, `ret` |

The one-word cleanup is decoded evidence; `int` remains the existing provisional
argument type. No argument or receiver memory is accessed. Existing slots 4, 5
and 6 already have separate source definitions and are unchanged.

## Individual identity and call boundaries

The raw retail primary table at 0x00555318 has COL 0x0055d7d8, literal RTTI name
`.?AVCharacter@@`, and object offset zero. Its five entries identify these
defaults separately. The actual Motnctrl object emits those same five decorated
method references in `??_7Character@@6B0@@`. Character's GameObject virtual-base
table is distinct, at 0x005552a8 with object offset 416. Raw base descriptors
record Character's PMD as `(0,-1,0)` and GameObject's as `(0,4,4)`.

The complete strict 335-byte Character constructor at 0x004a6780 writes the
primary table through `[ebx]` at 0x004a67af. Its `ret 8` includes the existing
hidden most-derived flag; that constructor interface is not the callbacks'
interface. The complete strict 145-byte slot 1 at 0x004a6930 calls primary slots
0 and 2 during motion setup, through `[edx]` and `[eax+8]` respectively.

D3DIMSoultreeCharacter's primary table at 0x00551530 has its own literal RTTI
and zero-offset COL. It overrides all five entries with 0x00445a70, 0x00445fc0,
0x00446110, 0x00445ec0 and 0x00446520. Its GameObject virtual-base table is at
0x005514c0, object offset 532. The retained test checks these raw identities and
the distinct emitted derived primary references. No inheritance, vtable or class
declaration changes are needed for the new definitions.

## Validation and source linking

`tests/test_character_defaults.py` passes **72 strict checks**: the five new
complete bodies and every registered strict body in Motnctrl and
D3DIMSoultreeMotnctrl. COFF auxiliary extents independently report 1 or 3 bytes;
the bodies have no relocations. The test also checks the retail padding after
each full return body, four raw tables, the two actual primary tables and the
constructor/motion-setup instructions above.

The native fixture links the actual standalone provider, compiling only its
harness with `/Od`. It performs **4,902 calls**:

- 256 prepared complete Character storage patterns, each calling the three
  no-argument methods and both one-argument methods with eight edge/raw integer
  values: 4,864 calls.
- The same 19-call set with null and inaccessible receiver addresses: 38 calls.

Every call must complete without an exception, leave the entire Character-sized
buffer and surrounding guards unchanged, and preserve floating-point status and
control. Prepared runs vary rounding and precision modes. Calls are qualified
direct calls to the actual source methods; they deliberately bypass vptr and
virtual-base-table reads. The runtime test proves their no-read/default behavior,
while separate raw and compiled table checks establish dispatch identity. It
does not construct a Character, execute its full lifecycle or test racing.

The isolated real build `build-iqbfly5u` compiles 239 translation units and removes
exactly these five unresolved names, **295 to 290**. Subsequent composition/link
and whole-object preservation are recorded in [GAME_BUILD.md](GAME_BUILD.md).
This is source-link progress, not a completed executable.

```powershell
$env:PYTHONPATH='.'
$env:VC6_ROOT=(Resolve-Path work/vc6sp3-disc).Path
$env:MCM2_EXE=(Resolve-Path work/game/mcm2.exe).Path
python -m unittest discover -s tests -p test_character_defaults.py -v
```
