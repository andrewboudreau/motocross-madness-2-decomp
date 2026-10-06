# racesnd.cpp

`src/reconstructed/RaceSound.h` / `RaceSound.cpp`. Evidence: the `__FILE__`
literal `D:\aardvark\VC\krusty2\racesnd.cpp` (`0x005726b8`; DebugMalloc line
77, deletes at lines 128–156 and 177, `new` at 1630/1678, DebugMalloc 1652)
and RTTI `RaceSound : GameObject` (vtable `0x00557710`, 27 slots, overriding
slots 0, 10 and 23). bikerace.cpp allocates 0x12b0 bytes for it
(`0x004195e4`) and passes 1 to the constructor. The file's code runs from
`0x004e1fb0` (constructor) to the qsort callback `0x004e5880`; the code before
it belongs to BaseQuarryEvent (vtable `0x0055766c`) and RaceStatus.cpp
follows. Names are provisional.

The object plays the race audio. Eleven 0x54-byte channels (+0x98), one per
racer, each own a 44100-byte buffer; every frame they are sorted by ground
distance from the camera (`0x004e4460`, clamped at 500) and the nearest four
get the four engine voices (+0x43c, in-use flags +0x44c). Engine samples
come in eight sets of three engines by ten samples (data +0x840.., byte
counts +0xc00.., loaded counts +0x1128..), streamed into the channel's
Sound in two halves (`0x004e4a30` starts, `0x004e4d10` refills). Other
sounds: idle effects (+0x11b8, +0x11d0), position/lap cues (+0x11ec..
+0x1204), the crash sound (`0x004e57d0`) and keyboard toggles in slot 23
(Ctrl+S sound, Ctrl+C music volume, Ctrl+V the EAX 1.0 environment). The
camera's protected Camera/BikeCamera fields are read through
`UnknownRaceSoundCamera`.

Exact (24 calibration cases): constructor, destructor and deleting
wrapper, the channel reset `0x004e3430`, slots 10 and 23, `0x004e42e0`,
`0x004e43a0`, `0x004e4460`, `0x004e4a30`, the Audio.res loaders
`0x004e5500`/`0x004e5660`, the play helper `0x004e5780`, `0x004e57d0`,
`0x004e5860`, the qsort callback `0x004e5880` and the eight vector
initializers `0x004e48f0..0x004e4a2b`. Changing PCSoundInterface's
listener setters (`0x004bec00/50/90`) to take `Vector3` by value keeps
PCAudio.cpp exact and matches these callers.

Near misses (`samples/race/RaceSoundNearMisses.cpp`, notes there): the
per-racer update `0x004e39b0` and the half-buffer refill `0x004e4d10`.

Not reconstructed: the 4153-byte loader `0x004e23f0` (creates the sound
groups and loads every sample).
