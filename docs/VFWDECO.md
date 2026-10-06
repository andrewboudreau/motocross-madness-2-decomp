# vfwdeco.cpp

`src/reconstructed/VfwDeco.h` / `VfwDeco.cpp`. The whole unit is a single
function: `UnknownVideoDecoder`'s destructor `0x0052d050`. The object is
the one at PCTextureMap+0x7c, and PCTextureMap.h now includes VfwDeco.h
instead of its own stub.

Evidence: the three xrefs of the `__FILE__` literal at `0x005759cc` all lie
inside `0x0052d050` (lines 178-180). Its MSVFW32 import stubs ICClose and
ICSendMessage are called only from there. VehicleCamera's slot 75 ends just
before it, and VideoCard.cpp starts at `0x0052d0d0`. No other decoder code
survives in the binary.

The destructor closes the HIC at +0x18 and then sends
`ICM_DECOMPRESSEX_END`; retail really does close first. It then frees the
buffers at +0x00, +0x10 and +0x14 with the debug delete. Exact: 1
calibration case.
