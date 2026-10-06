# QuarryStuntEvent.cpp

`src/reconstructed/QuarryStuntEvent.cpp`, with the class in `QuarryEvent.h`.
Names are provisional.

Evidence:
- **`__FILE__`:** the literal at `0x00572154`; its first xref is at
  `0x004de2cd`.
- **RTTI:** the `BaseQuarryEvent` vtable at `0x0055766c` points into the
  range for slots 0, 10, 12, 14, 23, 24, 27, 29, 30 and 34.
- **MemTag strings:** "entering/exiting BaseQuarryEvent::Create" confirm
  the class name.
- **`.CRT$XCU`:** its four kVec3 `$E` are entries `0x005663e4..0x005663f0`.

Extent: `0x004de2a0..0x004e1fa7`. Quantize.cpp ends at `0x004de29f` and
RaceSound's constructor starts at `0x004e1fb0`.

Exact: 25 calibration cases:
- The constructor, the destructor and its deleting wrapper.
- `Create` `0x004de3b0`.
- Slots 10, 14, 23, 24, 27, 29, 30 and 34. Slots 14 and 34 are bodies that
  Wrecker's and Scene's vtables also use.
- The three memory-estimate helpers, the message toggle `0x004e0c30` and
  the clock reset `0x004e1f00`.
- The eight `$E`.

Source forms needed:
- The kVec3 statics are defined between slot 12 and slot 24, where their
  `$E` code sits. Defining them at the top changes the constructor.
- The render target is read through direct casts; an inline accessor
  changes the destructor's registers.

Near misses (`samples/game/QuarryStuntEventNearMisses.cpp`): the memory
estimate `0x004e0560` and slot 12 `0x004e14a0` (the "Atmosphere" debug
page). Both differ only in register choice. The 7.9 KB loader
`0x004de590` is not attempted.

QuarryEvent.h now types BaseQuarryEvent's fields (size still 0xa4).
TrackOverlay.h, SceneManager.h, RaceView.h and TrackGame.h gained members.
