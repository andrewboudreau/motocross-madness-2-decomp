# AuralScape.cpp

`src/reconstructed/AuralScape.h` / `AuralScape.cpp`. Its classes, all
from RTTI: SoundGroup (vtable `0x00550500`), SoundInterface (`0x00550570`),
SoundEmitter (`0x0055057c`), SoultreeSoundEmitter (`0x005505f8`) and
AuralScape (`0x00550668`). There is also an 8-byte listener record with no
RTTI (`AuralScapeListener`). Member and method names are provisional.

Extent: `0x00401a30..0x00403d4b`. The end is confirmed: BackgroundImage.cpp
starts at `0x00403d50`. The start is a strong inference, about 90% sure.
`0x00401a30..0x00402060` has no `__FILE__` reference of its own, and five
pieces of evidence place it here:
- **`.CRT$XCU`:** the entries after Arrow.cpp's are the four kVec3
  initializers at `0x00403c10..0x00403d00` followed by `0x00402050`. That is
  an empty `$E` for a file-static `Vector3[2]` at `0x005776f8`. A TU's
  entries are contiguous, so `0x00402050` belongs to this unit.
- **`.data`:** the type descriptors of SoundGroup, SoundInterface,
  SoundEmitter, SoultreeSoundEmitter and AuralScape surround the only
  `ContainerList.h` literal and the AuralScape.cpp `__FILE__` (`0x005667e8`).
  No other unit's path falls in between.
- **Vtables:** they appear in `.rdata` in the same order as that code.
- **`.bss`:** the globals `0x005776d8..0x00577737` sit between Arrow.cpp's
  and BackgroundImage.cpp's.
- **Line numbers:** SoundEmitter's loader is line 130, which leaves room
  above it for SoundGroup and SoundInterface.

Exact: 71 calibration cases, the whole unit except two shared bodies
claimed elsewhere. The ContainerList destructor COMDAT `0x00402040` is
KeyboardDevice's case. The listener constructor is the folded body
`0x004676a0`.

PCAudio.h's SoundGroup now has its virtual overrides and its
`ContainerList<Sound*>` at +0x34. `0x00401b50` and `0x00401be0` return int,
so their PCAudio binding keys changed; the addresses are the same.

Source forms needed:
- The SoultreeSoundEmitter and AuralScape destructors are implicit.
- `0x00402810` writes `(rand() * (1.0f / 32768.0f)) * 100.0f`, which keeps
  the two constants separate.
- `0x00403150` uses two inline helpers. They reproduce VC6's inline
  budget: `ContainerList::Reserve` is inlined only in the last `Add`.
- `0x00403710` initialises its locals in a fixed order.
