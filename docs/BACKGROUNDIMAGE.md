# BackgroundImage.cpp

`src/reconstructed/BackgroundImage.h` / `BackgroundImage.cpp`. Evidence: the
`__FILE__` literal `D:\aardvark\VC\krusty2\BackgroundImage.cpp`
(`0x00566978`, DebugMalloc/DebugRealloc/delete lines 212, 230, 270) and RTTI
`BackgroundImage : GameObject` (vtable `0x005506d8`, overriding slots 0, 8,
13, 15 and 18 and adding slot 27, the shared `return 1` body `0x004627f0`).
The file runs from `0x00403d50` to `0x00404de0`; Bike.cpp starts at
`0x00405190`. Names are provisional.

The object draws a PCTextureMap (+0x2c) behind the scene into its render
target (GameObject+0x18, a PCRenderTarget: +0x08 camera, +0x0c/+0x10 size,
+0x14/+0x18/+0x1c buffered-frame counters, +0x48 back buffer, +0x50 device).
It keeps an off-screen copy of the background (+0x3c, created in slot 8
with the back buffer's surface description) and an array of 0x40-byte
regions (+0x50): a frames-left count, one rectangle per buffered frame, a
pending frame, the frame count when set and an owner value. Each frame it
copies the regions back from the copy (BltFast) and clears their depth
(device Clear), or restores the whole copy when the camera does not cover
the screen. The camera's viewport fields are protected in Camera.h, so the
file reads them through `UnknownBackgroundCamera`.

Exact (13 calibration cases): the destructor and its deleting wrapper,
slots 8, 15 and 18, `0x004040b0` (set image), `0x00404200` (release a
region), `0x00404240` (record a region rectangle, clipped with `?:`),
`0x00404c80` (release the DC), `0x00404cb0`, `0x00404cd0`, `0x00404d30`
and `0x00404da0` (mark every frame dirty; an unsigned viewport compare).

Near misses (`samples/render/BackgroundImageNearMisses.cpp`, notes there):
the constructor (vptr store placement), slot 13, the region allocator
`0x004040f0`, the region loops `0x004042e0`/`0x004043c0` and the draw
routine `0x00404480`.

Not reconstructed: `0x00404700` and `0x004049d0`.
