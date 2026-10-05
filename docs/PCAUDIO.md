# PCAudio.cpp: PCSoundInterface

`src/reconstructed/PCAudio.cpp` with `SoundInterface.h` and `PCAudio.h`.
Literal evidence: `D:\aardvark\VC\krusty2\PCAudio.cpp` (`0x0056fa28`, the
`__FILE__` of the allocations at lines 2367, 2574 and 2575), the option names
`AllowSoundHardware`, `AllowSoundEnumeration` and `AllowEAXExtension`, the
"Sound Card reports" format string (`0x0056fac8`), RTTI for
`PCSoundInterface : SoundInterface` (vtable `0x00555d9c`, whose only slot is
the deleting destructor `0x004be490`), `Sound` (`0x00555d88`) and
`SoundGroup` (`0x00550500`).

The DirectSound identities are strong inference, not RTTI: the listener is
queried from the primary buffer with the bytes of IID_IDirectSound3DListener
(`0x00556c90`), the primary buffer description is 0x24 bytes with flags 0x91
(primary, 3D, volume), the caps are 0x60 bytes, and the imports are
DSOUND.dll ordinals 1 and 2 (`0x005330a0`, `0x005330a6`). `0x00556d20` holds
the EAX 1.0 listener property set GUID. Method names on PCSoundInterface stay
`UnknownFunctionN`.

Layout (0x478 bytes): SoundInterface's two `ContainerList`s (+0x04, +0x18)
and started bit (+0x2c); the device count and chosen device (+0x30, +0x34);
four 0xf0-byte device records (+0x38: GUID, 128-byte description, caps); the
start-up primary volume (+0x3f8); the caps (+0x3fc); option bits (+0x45c);
DirectSound, listener and primary buffer (+0x460..+0x468); the 0x54-byte
thread helper (+0x46c); the EAX probe's SoundGroup and Sound (+0x470,
+0x474).

Exact (24 calibration cases): the helper's constructor and destructor
(`0x004bddf0`, `0x004bde40`), the device record's constructor and probe
(`0x004be280`, `0x004be2d0`), PCSoundInterface's constructor and destructor
(`0x004be370`, `0x004be4b0`), the start-up `0x004be5a0`, the enumeration
callback `0x004be7b0`, `0x004be800`–`0x004be8b0`, `0x004be9b0`, the EAX
probe `0x004be9e0`, the primary buffer `0x004beb10`, the listener setters
`0x004beb80`–`0x004becd0` and the EAX property setters `0x004bed00` and
`0x004bed40`.

Source shapes:

- `0x004bddf0` must be defined in this file: with its body visible VC6 emits
  no EH frame around `new UnknownPCAudioObject` in `0x004be5a0`, as retail.
- `0x004be2d0` and `0x004be5a0` share failure exits through `goto`; in
  `0x004be5a0` the result stays in edi and every DirectSound failure returns
  it through one exit. The start-up clears "AllowSoundEnumeration" before
  testing it, so the enumeration branch is dead (retail behaviour).
- `0x004beb10` declares the buffer pointer before the description, which is
  both `= {0}`-initialised and `memset`, as retail clears it twice.

Near miss (`samples/audio/PCAudioNearMisses.cpp`): `0x004be910`, which sets
the primary format (108 of 158 bytes; only the zeroing stores are scheduled
differently against the argument loads).

Not reconstructed: the rest of PCAudio.cpp (`Sound` and the helper's other
methods, `0x004bb810`–`0x004bde40` and `0x004bdef0`–`0x004be280`).
