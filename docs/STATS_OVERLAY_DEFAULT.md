# StatsOverlay cue default method

`StatsOverlay::UnknownFunction464e80(int)` supplies the missing default method
called by `BaseQuarryEvent::Tick`. Its ordinary empty C++ emits retail's complete
three-byte `RET 4` at `0x00464e80`, followed by thirteen alignment NOPs. The
neighboring one-byte collision return at `0x00464e90` has a different calling
convention. Neither method is identified solely by sharing an empty address.

The complete strict 1,049-byte `BaseQuarryEvent::CreateOverlays` allocates
`0x9b8` bytes, passes the primary pointer to the StatsOverlay constructor, and
stores its result at event `+0x5c`. The complete strict 133-byte constructor
writes primary vtable `0x00558680` and returns the same pointer. Raw RTTI at
COL `0x0055f6e8` has object offset zero; StatsOverlay, Overlay, GameObject and
BaseObject have nonvirtual primary base offsets zero.

The strict 976-byte `StatsOverlay::LoadPanel` returns the original primary
receiver on success, or null on failure. CreateOverlays stores that result
back into event `+0x5c`. The complete strict 358-byte quarry tick checks the
cue index against minus one, pushes it as the sole argument, loads the same
primary receiver into ECX, and calls `0x00464e80` at `0x004de4a3`. It consumes
no return value and performs no caller stack cleanup. The method name, precise
semantic purpose and original translation unit remain provisional.

The provider object has only the 40-byte compiler directives and a 16-byte
executable COMDAT containing the full return and padding. It adds no imports,
nondebug relocations, storage, virtual table or CRT initializer. Existing
headers, callers and compiler flags remain unchanged.

`tests/test_stats_overlay_default.py` retains five complete strict comparisons,
the raw RTTI and allocation/return/caller chain, both complete provider sections
including flags and payloads, and compiled size/member-offset checks. The
calibration runner includes the full three-byte method. These checks establish
the narrow provider and call ABI; they do not execute a complete quarry event
or rendering lifetime.

```powershell
$env:PYTHONPATH = '.;tests'
python tests/test_stats_overlay_default.py -v
```

Set `VC6_ROOT` and `MCM2_EXE` to the verified private inputs. The isolated real
build `build-0s5uw7qd` compiles 261/261 translation units and removes only this
method, reducing unresolved symbols from 223 to 222. All 260 preceding game
objects retain 10,693 nondebug sections and 45,776 relocations, without filename
exclusions. Linking still fails; startup, menus and racing remain unverified.
