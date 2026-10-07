# PCAudio.cpp: PCSoundInterface and Sound

`src/reconstructed/PCAudio.cpp` with `SoundInterface.h` and `PCAudio.h`; the
file runs from `0x004bb630` to `0x004bed40`.
Literal evidence: `D:\aardvark\VC\krusty2\PCAudio.cpp` (`0x0056fa28`, the
`__FILE__` of the allocations at lines 2367, 2574 and 2575), the option names
`AllowSoundHardware`, `AllowSoundEnumeration` and `AllowEAXExtension`, the
"Sound Card reports" format string (`0x0056fac8`), RTTI for
`PCSoundInterface : SoundInterface` (vtable `0x00555d9c`, whose only slot is
the deleting destructor `0x004be490`; strict exact, as is Sound's `0x004bbb40`), `Sound` (`0x00555d88`) and
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

## Sound and its helpers

`Sound : BaseObject` (0x1f8 bytes) wraps one DirectSound buffer: +0x0c the
buffer and +0x10 its 3D interface (IID_IDirectSound3DBuffer bytes at
`0x00556ca0`), +0x14 its property set (IID_IKsPropertySet, `0x00556ce0`),
+0x18/+0x1c the buffer a streamed sound is being loaded into, +0x20/+0x24
duplicate buffers, +0x28 the sound a duplicate shares, a fade (+0x2c..+0x38),
the notifier (+0x3c), the stream (+0x40), a critical section (+0x48), the
name (+0x60), the time last played (+0x164), the 44-byte RIFF header
(+0x16a, so the PCM format sits at +0x17e), the buffer size, frequency,
volume and pan (+0x198..+0x1a4), the cached DS3DBUFFER (+0x1a8, 0x40 bytes)
and flag words (+0x1e8..+0x1f5). The 0x1c-byte notifier refills a streamed
buffer's halves from a thread woken through IDirectSoundNotify
(`0x00556cd0`). PCSoundInterface's +0x46c helper is a sound memory manager:
its thread creates and fills queued streamed sounds, evicting the least
recently played idle ones to stay within the budget.

Exact (41 more calibration cases): the WAV loader `0x004bbef0` (reads the
44-byte header, creates the buffer with a hardware-then-software retry for
looping sounds, registers the sound, fills it and makes duplicates) and the
streaming copy `0x004bd260`; also the notifier's thread, constructor,
start and destructor (`0x004bb630`–`0x004bb810`); Sound's constructor,
destructor, play, settings, control flags, property set, stop, playing test,
positions, restore, pause, fade, release, lock/unlock, buffer creation and
duplication and the 3D setters (`0x004bba10`–`0x004bdb60`); the qsort
comparison `0x004bdbd0`; and the manager's start, queue, record, eviction
and unload (`0x004bdef0`–`0x004be220`).

Source shapes:

- `SoundSystem()` is a macro for `(PCSoundInterface*)Game->field_0x04`; as
  an inline function VC6 allocates the two loads to other registers.
- `goto failed` gives the shared failure exit of play, stop and the 3D
  buffer query.
- `ContainerList::Init` returns whether it allocated: the manager's start
  returns the second list's result from the fresh pointer, not a reload.
- The notifier thread dispatches on the wait result with a `switch`.
- In the buffer creator the flags are or-ed before the algorithm GUID is
  copied.
- The streaming copy inlines UnknownTextureStream's end-of-stream test
  (out-of-line copy `0x00430ff0`, now an inline member in TextureMap.h),
  tests the locked sizes as `> 0` (unsigned, `jbe`) and jumps into the
  silence block from the first half.

Near misses (`samples/audio/PCAudioNearMisses.cpp`, notes there): the
frequency, volume and pan setters, the streamed buffer creator `0x004bd4b0`,
`0x004bc4c0`, `0x004bc320`, `0x004bd0c0`, the factory `0x004bb890` and the
manager's thread `0x004bdc00`, and the start-up `0x004bc6b0` (the setters'
loop shape; in the samples file it also gains an EH frame because the
notifier's constructor is not defined there).

Every function of PCAudio.cpp (`0x004bb630`–`0x004bed40`) is now either
exact or a documented near miss. `0x004be330` is not a function start (inside
`0x004be2d0`). For the setters, a `break` with a later index test, a
`while` loop, `continue` on an empty duplicate and a `goto` around the
failure return were also tried; VC6 still places the `return 0` after the
store.
