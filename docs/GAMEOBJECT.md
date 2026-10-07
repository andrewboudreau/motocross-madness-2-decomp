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
| +0x18 | Owner object (set by slots 8 and 25; Camera's owner) |
| +0x20 | Flag word gating slots 9-24 |
| +0x24 | Byte, initialised to 0xff |
| +0x25 | Bitfield: bits 0/1 from the constructor argument (set by slot 5, cleared by slot 4), bit 2 set by slot 16, bit 3 = skip/detached |
| +0x28 | Heap string of RTTI class names, one per constructor level, comma-terminated |

The constructor's single merged store of bits 0-3 (`and al,0f0h; xor al,dl;
or al,cl`) is VC6's code for four consecutive one-bit bitfield assignments; a
mask expression produces a different sequence, so +0x25 is declared as
bitfields. The other methods compile identically either way.

## Matches

Every GameObject function is reconstructed: 29 bodies, all strict exact under
the default profile (`vc6_o2_mt`) with every relocation bound; 21 of them do
not match under `/G6`.

| Function | Retail VA | Bytes | Behavior |
|---|---|---:|---|
| Constructor | `0x00468ca0` | 149 | `/GX` frame for the BaseObject subobject; clears links, sets +0x25 bits; `DebugMalloc(1, __FILE__, 31)` into +0x28; then `0x00469ce0(this)` |
| Scalar deleting destructor | `0x00468d40` | 30 | Destructor core, then `operator delete` `0x004a30c0` |
| Destructor core | `0x00468d60` | 97 | `/GX` frame; frees +0x28 with `operator delete(p, __FILE__, 40)` (`0x004a2e60`); `~BaseObject` |
| `0x00469680` | `0x00469680` | 58 | Recurses to the last next-sibling, unlinks each from its predecessor, virtual `Release` |
| `0x00469ce0` | `0x00469ce0` | 201 | Appends `typeid(*object).name()` minus `"class "` and a comma to +0x28 (`DebugRealloc`, line 1163) |

The `typeid`, `type_info::name` and `strstr` calls bind to LIBCMT
(`rtti.obj`, `typname.obj`, `strstr.obj` in the [CRT atlas](VC6_CRT_ATLAS.md)).
The `__FILE__` operand binds to `0x0056b6c8`. Pooled source-path literals
embed the build path, so the matcher keys them as `__FILE__:<basename>`,
falling back to plain `__FILE__`. `/GX` frame handlers bind as `<symbol>$ehhandler` (here
`0x0054ad48` and `0x0054ad68`, both `mov eax, funcinfo; jmp
___CxxFrameHandler` stubs).

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
| 5 | `0x004690b0` | 13 | Set +0x25 bits 0-1, tail-call slot 7 |
| 20 | `0x00469c00` | 60 | Ungated search, as slot 19 |
| 21 | `0x00469c40` | 60 | Ungated search, as slot 20 |
| 22 | `0x00469580` | 76 | +0x20 bit 9; two-argument search |
| 23 | `0x004695d0` | 76 | +0x20 bit 10; two-argument search |
| 24 | `0x00469620` | 94 | +0x20 bit 11; children with bit 1 (not bit 0), without bit 3; five-argument search |
| 25 | `0x00469720` | 68 | Slot 25 on every child without bit 3 (next read first), then +0x18 = argument |
| 26 | `0x004692c0` | 36 | Slot 26 on every child, then set own bit 3 |

Unless noted, walks call the slot on children with +0x25 bit 0 set and bit 3
clear, and return 1.

Tree helpers and the iterator (strict exact, not yet calibration cases):

| Function | Retail VA | Bytes | Behavior |
|---|---|---:|---|
| `0x004690c0` | `0x004690c0` | 50 | ORs flags into +0x20 and up the parents (each adds the child's +0x1c) |
| `0x00469100` | `0x00469100` | 42 | Recomputes +0x20 from the children (recursive); returns it with +0x1c |
| `0x00469130` | `0x00469130` | 86 | Appends an object chain after the last sibling |
| `0x00469190` | `0x00469190` | 96 | Appends an object chain to the children |
| `0x00469260` | `0x00469260` | 93 | Unlinks (`0x004691f0`) and inserts before `next` |
| `0x00468dd0` / `0x00468f10` | | 308 each | Slot 4 / slot 5 on descendants whose name list or RTTI name matches `"<name>,"` |
| `0x00469770` | `0x00469770` | 472 | Find by name; modes 0 children, 1 depth first, 2 siblings, 3 parent, 4 ancestors |
| `0x00469c80` | `0x00469c80` | 81 | Unlinks and releases descendants with +0x25 bit 3 (memory tag "UI") |
| `GameObjectIterator` ctor / `0x00469a20` / dtor / `Next` | `0x00469950` / `0x00469a20` / `0x00469a40` / `0x00469a50` | 204 / 22 / 5 / 424 | The same walk as an iterator (0x94 bytes, `GameObjectIterator.h`); holds `0x0065b548` while running |

`0x004691f0` (unlink) returns 1 in retail but stays declared `void`: other
units' bindings use the `void` mangled name.
