# Game

RTTI: `Game` is a root class; `PCGame : Game` and `TrackGame : PCGame` derive
from it. Game's functions pass the literal `__FILE__`
`D:\aardvark\VC\krusty2\Game.cpp`, so the translation unit is named. Canonical
source is `src/reconstructed/Game.h` / `Game.cpp`. Names are provisional.

## The global object is a Game

The object behind the global pointer at `0x0056e26c` (read throughout the
camera and input code) is a Game. This is strong inference, from these
observations:

- Game's slots 13 and 14 (`0x00468900`, `0x00468930`) take the
  (event, entry) pair that ControlInterface `0x0043cf00` passes to the
  global's slots 13 and 14. Slot 14 matches control 0x20 through
  `0x0043caa0`.
- Game's constructor initialises the members the other code reads:
  - +0x0c, with a virtual slot 3 and +0x6c.
  - +0x14, which Game uses as a ControlInterface (slot 3 queries).
  - The +0x2d4 bits.
  - The float at +0x2f0.
- The setting slots (20-29) are `_purecall` in Game and implemented by
  PCGame (see [PCGAME.md](PCGAME.md)). +0x0c is the display and +0x10 a
  PCRenderTarget.

`UnknownObject56e26c` therefore derives from `Game`. It declares only the
members past Game's layout that other code reads, such as the instance and
window handles at +0x318/+0x31c and the camera state at +0x558 onward. Its
most-derived class is not established; TrackGame and PCGame are the
candidates.

## Vtable (`0x0055299c`)

| Slot | VA | Status |
|---|---|---|
| 0 | `0x00467ac0` / `0x00468a10` | Exact (wrapper, destructor: closes the file at `0x0065b4ac`; `0x00534c3d` is the CRT's `fclose`) |
| 1, 5, 6 | `0x00467ae0` | Exact (`return 1`, one body through identical-code folding) |
| 3, 4 | `0x00468c90` | Exact (`return 1`) |
| 9 | `0x004685c0` | Exact |
| 11, 12, 13, 17 | | Exact (forwards to the interface at +0x2f4) |
| 14 | `0x00468930` | Exact. Input presses: control 0x20 drives the +0x38 object; Ctrl+F (key 0x21, modifier 0xc) toggles +0x1c4 and E (0x12) clears it |
| 15 | `0x00468a30` | Exact. Shutdown: releases the interface, deletes the network object (+0x08), the owners (+0x04, +0x10) and the ControlInterface |
| 16 | `0x00468ae0` | Exact. Creates the network object (`new` at Game.cpp line 979) |
| 18 | `0x00468bd0` | Exact. Builds `<+0x1cc>\<name>` |
| 30 | `0x00468c60` | Exact ("No Strings Available") |
| 33 | `0x00467e80` | Exact |
| 7, 19–29, 31, 32, 34 | | `_purecall` |
| 2 | `0x00467af0` | Exact. Creates the PCControlInterface (`new` at line 187) and calls its slot 1 |
| 8 | `0x00467eb0` | Exact. Renders a frame, timing each phase, and fills the debug overlay's profile and memory pages |
| 10 | `0x004685d0` | Not reconstructed: it reads the time stamp counter with `rdtsc`, which VC6 can only emit from inline assembly |

The members at +0x2f4 and +0x34 are GameObjects (the root objects). The
non-virtual initialiser `0x00467b70` constructs them with GameObject's
constructor and links +0x34 under +0x2f4 (`0x00469190`). It then creates:
- the PCSoundInterface (+0x04, `0x004be370`, 0x478 bytes);
- the random seed: `srand` with the time stamp, then 1-256 `rand` calls.

It also reads the `TestKey` DWORD under
HKEY_LOCAL_MACHINE\SOFTWARE\Rainbow Studios into bit 2 of +0x2d4. When
slot 32 allows, it then creates:
- the TextureMapManager (+0x3c, `0x00510bd0`), with the +0x1c..+0x30 block;
- with bit 2 only, the DebugOverlay (+0x38, `0x00447920`/`0x00447de0`,
  under the "DebugOverlay" memory tag);
- the "AllowFreezeCamera" setting, stored into bit 0 of +0x2d4.

It is a near miss (761/775, `samples/game/GameNearMisses.cpp`). Only the
frame slots of the registry locals and the `new` temporary differ. Game's forwarding calls also line up with GameObject's slots
16, 19, 22–25 and `Release`. Input events therefore travel down the
GameObject tree: GameObject slots 22 and 23 take ControlInterface's
(event, entry) pair, and Camera slot 23 matches control 0xb7 against it.

The constructor (`0x00467990`) is exact. It:
- calls the table setup `0x00460ad0`;
- sets the +0x2d4 bits (masked byte stores) and the +0x2d5 bits (merged
  bitfield stores);
- copies an empty string to +0x40;
- sets 1.0 at +0x2e0/+0x2e4 and 0.05 at +0x2ec;
- decodes two static strings in place (XOR 0x5b): "SOFTWARE\Rainbow
  Studios\" and "TestKey".

`0x00468880` (called by PCCamera slot 27) stamps +0x2d8/+0x2dc with the
time from `0x004bfa80`.

## Slot 8 and the profile pages

Slot 8 (`0x00467eb0`) times the root object's slots 12-15 (prepare geometry,
pre-3D, 3D, post-3D) and the gap around the display object's slot 4 (wait for
flip). It runs the 3D phase through the +0x10 object's slots 1, 12 and 2 only
with bit 3 of +0x2d5. With bit 2 of +0x2d4 and a DebugOverlay at +0x38, it
feeds ten peak-hold values and prints two overlay pages:
- the profile page: mode, timings with peaks, and transform/primitive counts;
- the memory page: the MemTagStack categories, `GlobalMemoryStatus` and
  video memory.

Each page number is a function-local static (`0x0056b32c`, `0x0056b330`,
initially -1) allocated from DebugOverlay+0x26c0.

Matching depends on these source shapes:
- A running `last` time stamp, rather than separate start/end variables.
- One `failed:` block shared by both 3D-phase failures, reached by `goto`.
- An inline `DebugOverlay::NewPage()` for the page allocation.

The overlay row printers (`0x00447f40`, `0x00447fa0`) are cdecl varargs
members.

The peak-hold class (`src/reconstructed/PeakHold.cpp`, `0x004cb670`..
`0x004cb6d8`) is exact. Its TU is not established; the nearest source literal
is PCVideoCard.cpp's. Game.cpp's ten statics of it are constructed by the
dynamic initializers at `0x00467850`..`0x0046798f`, which are also exact.
