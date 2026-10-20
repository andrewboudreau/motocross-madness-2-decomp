# Game import providers

These definitions let VC6 `LIB.EXE` supply the three imports absent from the
VC6 SDK libraries. They contain no game code or replacement implementations.
The game still needs its supplied `blade.dll` and the system `DINPUT.dll`.

```
LIB /nologo /machine:IX86 /def:config/game-imports/blade.def /out:work/game-build/blade.lib
LIB /nologo /machine:IX86 /def:config/game-imports/dinput.def /out:work/game-build/dinput-ex.lib
```

Link both generated libraries alongside VC6 `DINPUT.LIB`; the latter supplies
the `c_dfDI*` data formats. Name the generated DirectInput library `dinput-ex.lib`
so it cannot shadow the SDK library.

## Evidence and scope

Verified with VC6 SP3 `DUMPBIN.EXE` 6.00.8447:

| Caller symbol | Provider | Export ordinal | Retail import |
|---|---|---|---|
| `_DirectDrawCreateEx@16` | packaged demo `blade.dll` | 3 | named, hint 2 |
| `_DirectDrawEnumerateA@8` | packaged demo `blade.dll` | 4 | named, hint 3 |
| `_DirectInputCreateEx@20` | Windows x86 `DINPUT.dll` | 2 | named, hint 1 |

The blade ordinals come from the DLL in the same private game directory as
the hash-pinned retail executable. The DirectInput ordinal was checked in
`C:\Windows\SysWOW64\dinput.dll`; it is current-machine provider evidence,
not proof of every historical DirectInput DLL. Caller decoration is also
recorded in `PCVideoCard.bindings.json` and `PCControl.bindings.json`.

These libraries import by ordinal. Their PE import encoding therefore differs
from retail, and they are for the functional source build, not an exact-image
matching claim. Using a decorated name without the ordinal causes VC6 LIB to
request an export such as `DirectDrawCreateEx@16`, which the real DLL lacks.
No binary rewriting or placeholder function implementation is used.

Reproduce the provider checks with `DUMPBIN /exports` on each DLL and
`DUMPBIN /imports` on the private retail executable. A three-call C++ link
probe verifies the caller symbols resolve and the emitted PE imports contain
blade ordinals 3/4 and DirectInput ordinal 2.
