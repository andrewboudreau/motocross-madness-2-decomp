# Net.cpp: the DirectPlay network layer

`src/reconstructed/Net.cpp` reconstructs `D:\aardvark\VC\krusty2\Net.cpp`
(`0x004aab00`..`0x004ae2e7`). 86 functions match strictly under the default
VC6 profile and are registered as calibration cases; the near miss
`DispatchMessages` `0x004aced0` is in the file in retail order with a
`// Not exact` status comment. The RTTI names are `NetworkInterface`,
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
  they are reconstructed (all strict exact) in src/reconstructed/SerialAddress.cpp,
  where `BuildComPortAddress` (`0x004ae410`) fills the DPCOMPORTADDRESS.
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
  `DirectPlay4A`/`DirectPlayLobby3A` interfaces, because VC6's
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
  - trkgame.cpp constructs it, calls `0x004ad3e0` and deletes it;
    `0x0052085c` calls `0x004ad570`.
- The enumeration counters are at `0x006886ac` (connections, at most 5),
  `0x006886b0` (sessions, at most 5) and `0x006886b4` (players, at most 7).

## Shared-header changes

- `Game.h`: the `UnknownNetObject` stub became `NetworkInterface` from
  Net.h. The bindings of Game, EventManager and TrackGame were renamed to
  match.
- `TrackGame.h`: `SetDirectPlay` takes the DirectPlay interface
  pointers. The WinSock client gained its fields and `0x004ad530`/`0x004ad570`.
- `EventManager.cpp`: `0x004ac800` now returns `NetPlayer*`.

## Near miss

| VA | Size | Match | Difference |
|---|---:|---:|---|
| `0x004aced0` | 370 | 361/370 | the system-message block: retail loads the sender before forming the message pointer with `lea`; VC6 forms it first and swaps ebx and ebp (forms tried below) |

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
- `0x004abf10`: the TCP/IP port element points at a `WORD` local assigned
  `atoi(port)`. VC6 puts the WORD in the dead `port` slot and emits no
  store of atoi's result (retail has none either). Because `port` is never
  address-taken, the modem branch keeps it in ebx across `lstrlenA`.
  Pointing the element at `port` itself (the earlier form) cost that
  register (874/948). The modem branch stores the phone size before its data.
- `0x004ad3e0`: retail's `mov ebp,2` shared by `socket()` and `sin_family`
  appears only when `sin_family = AF_INET` is written in both branches of
  the gethostbyname test.
- `0x004addc0`: the order of the two zero-initialized locals sets their
  frame slots.
- `0x004ab960`: the size query fails on its own test (`result < 0 &&
  result != DPERR_BUFFERTOOSMALL`) instead of sharing the second query's.
  That gives retail's `cmp esi, ebx` tests, the cross-jumped error texts and
  the memory reload of `connection` in the failure block (149/1072 ->
  985/989). The four interface pointers are declared in retail's zero-store
  order (baseDirectPlay, directPlay4, baseLobby, connection) for the rest.
- `0x004ae100`: the early `return 1` paths must share one tail. Nest the
  work under `if (sessions)` and `if (instance)` rather than returning early.
- `0x004ad5a0`: 65 `case`s with `sprintf` match exactly, including the
  three byte-indexed jump tables; the target size 1771 includes the tables.
- `0x004adff0`: write the retry as `result = Enum(); while (result ==
  DPERR_CONNECTING) result = Enum();`.
- `0x004ac830`: an if/else-if chain with one final `return 1` keeps the
  separate calls. A combined expression collapses into neg/sbb.

## Network identifiers

`src/reconstructed/NetworkGuids.cpp` supplies the six 16-byte GUIDs read by
Net and MSZoneInterface. The score interface (`0x00556d80`) and class
(`0x00556da0`) are passed to `CoCreateInstance`; the player tag
(`0x00556db0`) and preset/rank tags (`0x00556dd0`, `0x00556de0`) identify the
lobby property queries. The application tag (`0x00556dc0`) is shared by
DirectPlay session setup, connection enumeration and Zone score reporting.

The values, sizes and readonly storage match retail. Their descriptive names
and original defining TU remain unproven. `tests/test_directx_guids.py`
compiles both identifier providers with VC6 SP3 and checks exported symbol
names, extents, storage flags and every byte against the hash-pinned private
EXE. Supplying these constants resolves linker dependencies; it does not
establish that the original external lobby service is operational.

`DirectXGuids.cpp` also supplies the C-linkage `IID_IDirectPlayLobby3A`
(`0x00556870`) and `DPAID_INetPort` (`0x00556960`) constants. ConnectUsingLobby
pushes the former at `0x004ab9ee` for QueryInterface. CreateAddress copies all
four DWORDs of the latter at `0x004ac044/49/4f/59` into its optional two-byte
TCP/IP port element. Their complete 989-byte and 948-byte callers remain strict
exact. These symbols are absent from the installed VC6 libraries, including
DXGUID.LIB; source definitions reproduce retail's 16-byte readonly storage.
This establishes the observed constants and their uses, not an original SDK
revision, library object or defining translation unit.

## Reproduce

```bash
python tools/run_calibration.py --compiler vc6 --profile vc6_o2_mt \
  --vc6-root "$VC6_ROOT" --exe work/game/mcm2.exe --jobs 8
```

## Forms tried

NetworkInterface::DispatchMessages (0x004aced0, 370 bytes): 361/370.
Addressing the slot through a byte offset (MessageAt below) fixes the
SIB base/index order everywhere (was 355/370 with messages[index]). The
rest is in the system-message block: retail loads [edx+edi+4] (from) and
then forms the pointer with `lea ebx,[edx+edi]`; VC6 here forms it first
(`mov ecx,[esi+0x34]; mov ebx,edi; add ebx,ecx`) and pushes system[2]
from edx instead of eax. Taking the pointer only inside the `if (!from)`
(through offset or index) gives retail's load+lea but swaps ebx and ebp
for the index and the pointer (197/370); local declaration order, name
buffer order, `last != index`, a split increment, removing the `system`
local or the `message` pointer, per-case `system` locals and a loop-scope
pointer declared outside the `if` do not change that swap. Using offset in
the pointer itself (`&MessageAt(..)` or a cast) outside the `if` gives a
368-byte form; a reference for the whole loop body is far worse. Sweeps of
the in-`if` pointer form (144 combinations: pointer and test forms, the
`system` local, the switch operand, increment forms and the `more` operand
order) keep the ebx/ebp swap (best 197) and leave the original at 361.
Also unchanged or worse: `value` reused as `last`, an inverted head test
(326), a `NetMessage* base` local (83), `++index >= 0x100`, a split
increment with `== 0x100` (360) and an `== 0xff` reset (318/372).
Session 15, worse: `last = nextMessage - 1` hoisted above the if/else (16),
`last` assigned before `index` in both arms (18), `offset` computed before
`more` (190).
Session 14: `messages + index`, `&(messages[index])` and `(int*)(..)`
spellings keep 361. `&MessageAt(messages, offset)` for the pointer with
`message->from` (or a reference) gives retail's ebx/ebp and the system
pushes but forms the pointer with `mov; add` (368 bytes, 3 code diffs);
testing `MessageAt(messages, offset).from` instead gives retail's
load+lea with the ebx/ebp swap (197), with or without the system local.
Testing `MessageAt(messages, offset).from` and taking the pointer inside
the `if` (or using no pointer) is identical to retail once registers are
masked (0 hunks, 139 aligned): only the ebx/ebp swap is left, so VC6 here
weights the index above the message web, where retail weights it below.
Pointer scope, `register`, const, a reference, per-use MessageAt and a
pointer shared with the 0x40 block do not change the weights.
Session 21: tools/decl_shift_scan.py on that 197 form (test through
MessageAt, `&MessageAt(messages, offset)` taken inside the `if`) is 197 at
every k = 0..63, so the ebx/ebp swap is not the symbol-arena tie-break;
retail keeps index in ebp and gives ebx to `last`'s loop-top reload, the
vtable and the message pointer. /ML keeps 361; /G6 gives 72.
Session 24, on that 197 form (all keep the swap unless noted): a `from`
local read through MessageAt before the pointer is taken, the pointer taken
before the test, a `NetMessage&` (232, 368 bytes), an assignment inside the
test (232), a `NetMessage* base = messages` local (83); the system block
with or without the `system` local or the pointer (no pointer: 60), an
`unsigned long*` view; `index++` then the test, a ternary or if/else
increment (144), `more` after `offset` (190), `last != index` (198) or
`index - last` (79); all six declaration orders of index/last/more before
or after the name buffers, `value` reused as `last`, a `next` local,
`offset` at function scope, `for (;;)` with a break, `while (more != 0)`,
a `bool more` (76) and `last = index - 1` (57).
Session 25 for 0x004aced0 (all 361, or the known 232/197 families): six
spellings of the `from` test times four pointer spellings with data/type
read through MessageAt (96 forms), 0..7 dead initialised locals and a
named player local for RemovePlayer.
2026-11-28: the system-message block as a static inline
`HandleSystemMessage(NetworkInterface*, NetMessage*, char*)` helper, with the
pointer, the sender or both passed in, and the name buffer inside or outside
it: 361, 232, 197 or 82; none moves the `from` load ahead of the `lea`.
