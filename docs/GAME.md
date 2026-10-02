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
- The named-setting slot (22) is `_purecall` in Game and implemented by
  PCGame (`0x004c1e80`).

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
| 2, 8, 10 | | Not reconstructed |

The constructor (`0x00467990`) is not reconstructed yet. It sets the +0x2d4
and +0x2d5 bit groups, copies an empty string to +0x40, sets 1.0 at
+0x2e0/+0x2e4 and 0.05 at +0x2ec, and decodes two static strings in place
(XOR 0x5b).
