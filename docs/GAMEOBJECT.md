# GameObject reconstruction

RTTI: `GameObject : BaseObject`. GameObject introduces primary slots 4–26 and
overrides slot 2 (`Release`). Its constructor (`0x00468ca0`) cites
`D:\aardvark\VC\krusty2\gameobj.cpp` through `__FILE__`. Canonical source:
`src/reconstructed/GameObject.h` and `.cpp`; call/data bindings in
`src/reconstructed/GameObject.bindings.json`. Names are provisional.

GameObjects form a tree. Most introduced slots walk the children and forward
the same virtual call, gated by bits of the flag word at +0x20 and by each
child's bit flags at +0x25.

| Offset | Observed role |
|---|---|
| +0x08 | Previous sibling |
| +0x0c | Next sibling |
| +0x10 | First child |
| +0x14 | Parent |
| +0x18 | Owner object (set by slot 8; Camera's owner) |
| +0x20 | Flag word gating slots 9-23 |
| +0x25 | Bit flags: bits 0/1 cleared by slot 4, bit 2 set by slot 16, bit 3 = skip/detached |

## Matches

All 20 bodies are strict exact under the default profile (`vc6_o2_mt`) with
every relocation bound; 14 of them do not match under `/G6`.

| Slot | Retail VA | Bytes | Behavior |
|---:|---|---:|---|
| 2 (`Release`) | `0x004696c0` | 81 | Returns 0 while global `0x0065b548` is set; first child runs `0x00469680`; unlink from siblings/parent; `BaseObject::Release` |
| 4 | `0x004690a0` | 13 | Clear +0x25 bits 0-1, tail-call slot 6 |
| 6 | `0x00469050` | 30 | Slot 6 on each child without bit 3 |
| 7 | `0x00469070` | 35 | Slot 7 on each child with bit 0, without bit 3 |
| 8 | `0x004692f0` | 12 | +0x18 = argument; returns `this` |
| 9 | `0x00469430` | 65 | +0x20 bit 0; children with bit 0, without bits 2/3 |
| 10 | `0x004693d0` | 94 | +0x20 bit 1; after each child, owner+0x04 object's slot 4 when its +0x70 bit 2 is set |
| 11 | `0x004da540` | 8 | Returns 1 (address shared by many classes) |
| 12 | `0x00469300` | 48 | +0x20 bit 12 |
| 13 | `0x00469330` | 47 | +0x20 bit 2 |
| 14 | `0x00469360` | 55 | +0x20 bit 3; returns 0 at the first failing child |
| 15 | `0x004693a0` | 47 | +0x20 bit 4 |
| 16 | `0x00469480` | 74 | Store the argument's low bit in +0x25 bit 2; +0x20 bit 5 walk |
| 17 | `0x00469500` | 42 | +0x20 bit 6; children without bit 3 |
| 18 | `0x004694d0` | 42 | +0x20 bit 7; children without bit 3 |
| 19 | `0x00469530` | 68 | +0x20 bit 8; returns 1 at the first child returning nonzero |
| 20 | `0x00469c00` | 60 | Ungated search, as slot 19 |
| 22 | `0x00469580` | 76 | +0x20 bit 9; two-argument search |
| 23 | `0x004695d0` | 76 | +0x20 bit 10; two-argument search |
| 26 | `0x004692c0` | 36 | Slot 26 on every child, then set own bit 3 |

Unless noted, walks call the slot on children with +0x25 bit 0 set and bit 3
clear, and return 1. Remaining GameObject work: the constructor
(`0x00468ca0`, exception-handling frame), destructor core (`0x00468d60`) and
scalar deleting wrapper (`0x00468d40`), and slots 5, 21, 24 and 25.
