# recorder.cpp

`src/reconstructed/Recorder.h` / `Recorder.cpp`. Evidence: the `__FILE__`
literal `D:\aardvark\VC\krusty2\recorder.cpp` (`0x00572b04`; lines 442,
477, 535), RTTI `VCRInterface : GameObject` (vtable `0x0055778c`,
overriding slots 0 and 10) and `KrustyVCR : VCRInterface` (`0x00550e9c`),
and the Win32 imports the code calls. The TU starts at `0x004e6e40` (its
per-TU vector initializers) after RaceStatus.cpp; the shared
`Rectangle2D`/`RenderTarget` code from `0x004e8ad0` follows. Names other
than the RTTI classes are provisional.

VCRInterface records and plays back replays on a worker thread
(`0x004e6f80`, started with `_beginthreadex`): `0x004e7a90` creates the
UnknownVcr ring (VCR.h), eleven auto-reset events and a critical section,
then starts mode 0, 1 or 2; the game queues and takes records
(`0x004e8720`, `0x004e8810`), seeks, rewinds and flushes by signalling the
events, and `0x004e7a00` waits for the worker's acknowledgement.

Exact: 20 calibration cases (the eight vector initializers, the
constructor and destructors and every method above). Near misses
(`samples/race/RecorderNearMisses.cpp`, notes there): the worker thread
`0x004e6f80` (an 11-way `WaitForMultipleObjects` switch over the VCR
file), whose case bodies and jump table line up; VC6 merges the shared
tails of cases 1, 2 and 4 differently. Also slot 10 `0x004e7d60`, which
KrustyVCR shares; its control flow and callback switch order are
reconstructed, but register and stack-slot assignment differ.
