# LightEmitter.cpp, Quantize.cpp, MSZoneInterface.cpp

Names are provisional.

- `LightEmitter.cpp` (`src/reconstructed/LightEmitter.cpp`; RTTI
  LightEmitter vtable `0x005550cc`, LightManager `0x00555144`; xrefs
  `0x0049e276..0x0049e4fc`): the per-file vector initializers, the emitter
  constructor/destructors, colour/range/position/direction setters, the
  initialiser `0x0049e230` ("sphere.slt", jump table in the extent), the
  manager constructor/destructors, add, change counter, find-by-type
  `0x004a0190` and the cdecl vector negation `0x0049f180`. 26 cases.
  The lighting routine `0x0049e4a0` and its three workers (`0x0049e6b0`,
  `0x0049f1c0`, `0x0049f8d0`) round with the `fld; fistp [mem]` pattern of
  inline assembly and are not attempted. The shared bodies at slot 12
  (`0x00467ae0`) and slot 8 (`0x004452e0`) compile exact but are not
  counted, since the address alone does not prove ownership.
- `Quantize.cpp` (RTTI `ColorMapper : BaseObject`, vtable `0x00557658`;
  xrefs `0x004dde47..0x004de12e`): the table loader (raw or LZW), the
  555/565 table builder, constructor, destructors and three accessors.
  8 cases.
- `MSZoneInterface.cpp` (xrefs `0x004aa36c..0x004aa64d`, code
  `0x004aa010..0x004aa7e9`): the MSN Gaming Zone queries and score report
  of TrackGame+0x3410, the per-file vectors and the construction/atexit of
  the global TrackGame at `0x006851a0`. The definition of
  `g_UnknownGlobal56e26c` is placed here because `0x0056e26c` follows this
  file's strings in .data (strong inference). `UnknownFunction520820` is
  now declared variadic (retail calls `_vsnprintf`). 14 cases. Near misses
  in `samples/net/MSZoneInterfaceNearMisses.cpp`: the lobby queries
  `0x004aa360` and `0x004aa4e0` (failure-block placement). The soultree
  physics inlines inside this TU (`0x004aa150..0x004aa340`) stay in
  `samples/physics`.
