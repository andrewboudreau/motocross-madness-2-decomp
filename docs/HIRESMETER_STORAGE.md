# HiResMeter pointer storage

`src/reconstructed/HiResMeterStorage.cpp` supplies the initialized writable
pointer `g_HiResMeter` at retail `0x0056c470`. Its initial value is
`0x0067b468`, the existing `g_HiResMeterInstance` defined in `GUIManager.cpp`.
The provider creates no second meter and needs no dynamic initializer.
Original translation-unit ownership of this pointer is not established.

Game retains its smaller `GameHiResMeterView` declaration. The explicit cast
at the storage boundary is supported by retail HiResMeter RTTI: primary
vtable `0x00554170`, object offset zero, with a nonvirtual `GameObject` base
at offset zero. Both declarations expose `counterUsable` at +0x2c. This is
a verified object view, not a claim that the two C++ classes are identical.

`tests/test_hires_meter_storage.py` compiles the actual provider, the existing
instance's production translation unit, and a native layout fixture with
VC6 SP3. It verifies the retail image hash and RTTI, the complete meter size
(0xe50), both +0x2c field offsets and zero-offset native base conversions.
The provider must define only the four-byte writable pointer, emit no code,
and contain exactly one `IMAGE_REL_I386_DIR32` data relocation to the existing
instance with zero addend. Resolving that relocation must reproduce all four
retail pointer bytes. The target instance must be defined by its actual
production object.

This data connection does not establish clock calibration, timer behavior,
or successful game startup. Those require independent implementation and
validation of the consuming methods.
