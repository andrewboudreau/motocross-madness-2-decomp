# PCVideoCard.cpp and VideoCard.cpp

`src/reconstructed/VideoCard.h/.cpp` and `PCVideoCard.h/.cpp`. Evidence:
the `__FILE__` literals of both files (PCVideoCard.cpp lines 280, 307,
381 and 2053–2110; VideoCard.cpp line 63, xref `0x0052d22a`), RTTI
`PCVideoCard : VideoCard` (vtables `0x00556df4` and `0x00558d74`,
constructor `0x004c9760`, 0xb78 bytes) and the DirectDraw 7 API the code
calls (IIDs confirmed by their GUID bytes). PCVideoCard.cpp spans
`0x004c94a0..0x004cb667`, VideoCard.cpp `0x0052d0d0..0x0052d2bc`. Names
other than the RTTI classes are provisional.

The display object (Game+0x0c) keeps the stand-in name `UnknownDisplay`
in `Display.h`, now derived from `VideoCard`, because GUIManager,
PCGame, Game and EventManager bind its mangled names. It enumerates
DirectDraw devices and display modes (`DirectDrawEnumerateExA`,
`EnumDisplayModes`, a sorted 36-byte mode table), creates the device,
sets cooperative level and display mode (full screen or windowed with a
clipper), builds the flip chain and render surfaces with a gamma ramp,
presents frames, and reads the back buffer back for partial texture
blits. `0x004ca790` and `0x004ca900` return int (PCGame and its near
miss gained the new bindings keys).

Exact: 28 calibration cases (22 PCVideoCard, 6 VideoCard). Near misses
(`samples/render/PCVideoCardNearMisses.cpp`, notes there): `0x004ca520`,
`0x004ca5a0`, the PartialTexBlt driver `0x004cab00`, `0x004cb330`,
`0x004cb5b0` (declared void for GUIManager's bindings; retail returns
int) and the VideoCard constructor `0x0052d180`.
