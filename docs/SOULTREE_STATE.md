# Soultree instance counter storage

`src/reconstructed/SoultreeState.cpp` supplies the existing
`g_SoultreeInstanceCounter` import as one writable, zero-initialized `int`.
Retail address `0x00689f14` lies wholly in the zero-filled virtual tail of the
writable `.data` section. It has no file-backed initial bytes. The decoded
instructions establish a four-byte cell; the name and original translation-unit
ownership remain provisional.

An exhaustive scan for the encoded address finds four occurrences, all in the
complete **1,500-byte** initializer at `0x00500c50`:

| Instruction | Effect |
| --- | --- |
| `0x500de0` | Read the cell, then store its value at receiver `+0x204` |
| `0x500dec` | Clear receiver `+0x208` with the zeroed EBX register |
| `0x500df2` | Read the cell again and increment it |
| `0x500dfb` | Store the incremented value |
| `0x500e00` | Signed comparison against five selects whether to reset |
| `0x500e02` | Reset the cell with the zeroed EBX register |

With its initial zero and sequential calls, these instructions assign phases
zero through five. This is decoded behavior, not proof of arbitrary indirect
uses, threading guarantees or every instance's runtime lifecycle.

The retained regression in `tests/test_soultree_physics_methods.py` verifies
the retail section flags, zero-filled extent, all four address references and
the actual read/write sequence. It compiles the real consumer and preserves
its complete strict match, including all four DIR32 relocation destinations.
No initializer body, caller declaration, header or compiler option changes.

The new VC6 object contains `.drectve` (40 bytes) and `.bss` (four bytes), one
global definition, and no nondebug relocation, function, import or dynamic
initializer. Debug-record relocations are outside this storage claim. Its
binding uses the existing decorated name without an alias or address dispatch.

The isolated real link compiles **255/255** translation units and removes only
this missing global, **241 to 240** unresolved names, with none added. All 254
prior game objects retain their complete nondebug bytes and relocation
destinations under only the separately reviewed two palette getter names.
Independent recompilation also reproduces the new object's two sections and
zero nondebug relocations using its actual game command.

This supplies missing source-built storage. The regression does not execute
the complete physics initializer or establish startup, racing or shadow behavior.

```powershell
$env:PYTHONPATH='.;tests'
$env:VC6_ROOT=(Resolve-Path work/vc6sp3-disc).Path
$env:MCM2_EXE=(Resolve-Path work/game/mcm2.exe).Path
python -m unittest test_soultree_physics_methods -v
```
