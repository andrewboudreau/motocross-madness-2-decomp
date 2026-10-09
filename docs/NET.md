# Net.cpp: the DirectPlay network layer

`src/reconstructed/Net.cpp` reconstructs `D:\aardvark\VC\krusty2\Net.cpp`
(`0x004aab00`..`0x004ae2e7`). 83 functions match strictly under the default
VC6 profile and are registered as calibration cases; three near misses
are in `samples/net/NetNearMisses.cpp`. The RTTI names are `NetworkInterface`,
`InfoType`, `PlayerInfoType`, `ConnectionInfoType` and `SessionInfoType`. All
other type, member and function names are provisional.

## Names

Function and member names in `Net.h` are provisional (tier 3) unless noted:

- `ConnectUsingLobby` (`0x004ab960`) is the literal of its error texts
  ("Error: ConnectUsingLobby::...").
- `ReportDirectPlayError` (`0x004ad5a0`) formats "ERROR: DirectPlay (%s) in
  file %s at line %d".
- The wrappers are named after the single DirectPlay call each makes
  (`CreateSession`/`JoinSession` open with DPOPEN_CREATE/JOIN,
  `EnumConnections`/`EnumSessions`/`EnumPlayers` and their callbacks, and
  `CreatePlayer`, `DestroyPlayer`, `CreateGroup`).
- The keep-alive members (`StartKeepAlive`, `keepAliveTimeout`,
  `keepAliveInterval`, `keepAliveEvent`) are named from NetThread.cpp's loop.
  It drops a player not heard from for the timeout, sends message 0x4b every
  interval, and EventManager passes its "KeepAliveTimeout" setting.
- NetMessage's `type`/`from`/`to`/`flags`/`size`/`data` are the arguments
  `Set` (`0x004aacc0`) stores.

## Evidence

- `D:\aardvark\VC\krusty2\Net.cpp` (`0x0056e39c`) is the `__FILE__` of the
  allocations and error reports from `0x004aae50` to `0x004ae100`, for
  example lines 778/867/887 in `0x004ab960`, 2553/2564 in `0x004addc0` and
  2665 in `0x004adff0`.
- The extent starts with the four vector-constant initializer pairs at
  `0x004aab00`..`0x004aac3b`, which come before the first `__FILE__` user.
  It ends with the EnumPlayers callback at `0x004ae270`. NetProcs.cpp follows:
  its `__FILE__` is at `0x0056eb2c` and its literals begin at `0x0056eb04`
  (used from `0x004ae467`). The small switch helpers at
  `0x004ae2f0`..`0x004ae410` sit between the two files and are not claimed;
  they are reconstructed (all strict exact) in samples/net/SerialAddress.cpp.
- Vtables:
  - `NetworkInterface`: `0x005553e8`, one slot, `0x004ad050`.
  - `PlayerInfoType`: `0x005553f0`, slots `0x004adce0` and `0x004add00`.
  - `ConnectionInfoType`: `0x005553fc`.
  - `SessionInfoType`: `0x005589d4`, slot 1 is `0x004adc90`.

  The deleting destructor `0x004adce0` is slot 0 of all three derived
  vtables (identical-code folding). It is bound as PlayerInfoType's.
  - `InfoType` itself: `0x00551c20`, slots `0x0044d720` (its deleting
    destructor, exact from Net.cpp's inline `~InfoType`) and `0x0044d710`
    (the shared empty body, checked as RenderTarget slot 19). Both copies
    lie in dlgprocs.cpp's range, where the linker kept the COMDATs
    (inference from their position).
- `Game.cpp` line 979 allocates the 0x128-byte NetworkInterface into
  Game+0x08. `0x004add10` (ConnectionInfoType's constructor) is defined out
  of line here, because code at `0x0044b634`/`0x0044c0f5` passes its address
  to the vector constructor.
- DirectPlay calls go through decoded vtable slot offsets on hand-declared
  `UnknownDirectPlay4A`/`UnknownDirectPlayLobby3A` interfaces, because VC6's
  DPLAY.H predates IDirectPlay4.
  - The GUIDs bind to their SDK addresses (`0x005567b0`..`0x00556970`); the
    application GUID is at `0x00556dc0`.
  - CoCreateInstance binds to IAT `0x005503fc` and DirectPlayLobbyCreateA to
    `0x00534420`.
- The WinSock client `0x004ad3b0`..`0x004ad570` (`DebugSocket`,
  declared in TrackGame.h) has no `__FILE__`. It is placed here only because
  it lies between Net.cpp functions (`0x004ad280` and `0x004ad5a0`).
  - Its WSOCK32 calls bind to the ordinal-import thunks
    `0x005330ac`..`0x005330e2`: ordinals 111, 3, 19, 115, 23, 52, 9, 10, 4
    and 12.
  - TrackGame.cpp constructs it, calls `0x004ad3e0` and deletes it;
    `0x0052085c` calls `0x004ad570`.
- The enumeration counters are at `0x006886ac` (connections, at most 5),
  `0x006886b0` (sessions, at most 5) and `0x006886b4` (players, at most 7).

## Shared-header changes

- `Game.h`: the `UnknownNetObject` stub became `NetworkInterface` from
  Net.h. The bindings of Game, EventManager, TrackGame and GameNearMisses
  were renamed to match.
- `TrackGame.h`: `UnknownFunction4aa350` takes the DirectPlay interface
  pointers. The WinSock client gained its fields and `0x004ad530`/`0x004ad570`.
- `EventManager.cpp`: `0x004ac800` now returns `NetPlayer*`.

## Near misses (samples/net/NetNearMisses.cpp)

| VA | Size | Match | Difference |
|---|---:|---:|---|
| `0x004ab960` | 989 | ~15% | candidate is 1072 bytes; retail keeps zero in ebx and tail-merges the error texts |
| `0x004abf10` | 948 | 92.4% | MODEM case: `port` is in edx where retail uses ebx |
| `0x004aced0` | 370 | 95.5% | SIB operand order and how field_0x34 is reloaded after the slot-17 call |

## Source shapes that mattered

- `0x004aadc0` stores the sequence through a local `short*` into the data
  (`header[1] = sequence`); written as `*(short*)&data[2]` VC6 hoists the
  size store above the type and sequence stores.
- `0x004ac7c0` tests `players` before copying it into the walk pointer
  (`if (!players) return 0; NetPlayer* player = players;`) and compares
  `i != *index`; copying first swaps the counter and `*index` registers.

- Retail uses goto-failed shapes throughout: the success path is out of
  line in `0x004ac510`, `0x004ac720`, `0x004accd0`, `0x004acd20`,
  `0x004ac2d0`, `0x004ac480` and `0x004abf10`.
- For a truncated copy (`length > N ? N : length`), keep `strlen` in its
  own local. Folding it into the conditional calls `strlen` twice.
- VC6 overlays dead parameter slots with locals. In `0x004addc0` the
  address and its size live in the GUID and context argument slots, and
  in `0x004ad0f0` the size lives in the player slot.
- `0x004ad3e0`: retail's `mov ebp,2` shared by `socket()` and `sin_family`
  appears only when `sin_family = AF_INET` is written in both branches of
  the gethostbyname test.
- `0x004addc0`: the order of the two zero-initialized locals sets their
  frame slots.
- `0x004ae100`: the early `return 1` paths must share one tail. Nest the
  work under `if (sessions)` and `if (instance)` rather than returning early.
- `0x004ad5a0`: 65 `case`s with `sprintf` match exactly, including the
  three byte-indexed jump tables; the target size 1771 includes the tables.
- `0x004adff0`: write the retry as `result = Enum(); while (result ==
  DPERR_CONNECTING) result = Enum();`.
- `0x004ac830`: an if/else-if chain with one final `return 1` keeps the
  separate calls. A combined expression collapses into neg/sbb.

## Reproduce

```bash
python tools/run_calibration.py --compiler vc6 --profile vc6_o2_mt \
  --vc6-root "$VC6_ROOT" --exe work/game/mcm2.exe --jobs 8
```
