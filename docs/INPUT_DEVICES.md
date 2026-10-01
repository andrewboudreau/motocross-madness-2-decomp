# Input devices and ContainerList

RTTI: `InputDevice` → `PCInputDevice` → `JoystickDevice` / `KeyboardDevice` /
`MouseDevice` → `PCJoystickDevice` / `PCKeyboardDevice` / `PCMouseDevice`.
Canonical source is `src/reconstructed/InputDevice`, `PCInputDevice`,
`KeyboardDevice`, `MouseDevice`, `JoystickDevice`, `PCKeyboardDevice`,
`PCMouseDevice`, `PCJoystickDevice` (`.h`/`.cpp` and bindings)
and the template header `ContainerList.h`. Translation units are not
established, apart from ContainerList.h, which retail names through
`__FILE__`. Names are provisional. All functions listed are strict exact under
the default profile.

## Classes

- **InputDevice:** slot 0 is `_purecall` and slot 1 its destructor, so a pure
  virtual precedes the destructor. Its constructor (`0x00489780`) takes an id,
  sets +0x10 to -1 and clears a six-bit bitfield at +0x14. The helper
  `0x004897e0` tests +0x10.
- **PCInputDevice** (`0x004c25f0`, destructor `0x004c2650`): clears a
  0x244-byte device description at +0x18. Its size and the offsets used (type
  at +0x24, product name at +0x12c) match DIDEVICEINSTANCEA. It owns a
  COM-style device at +0x25c that the destructor shuts down with method 8 then
  a guarded release, consistent with IDirectInputDevice Unacquire/Release.
  `0x004c26d0` calls method 7 or 8 (Acquire/Unacquire). `0x004c2710` sets a
  one-value property through method 6 with a DIPROPDWORD-shaped block.
- **KeyboardDevice** (`0x00489e30`): `PCInputDevice(0)`, 256 20-byte input
  entries stamped from cdecl `0x004bfa80`, and `ContainerList<int>[6]` at
  +0x1660, each `Init(1, 1)`.
- **MouseDevice** (`0x0048a2d0`): `PCInputDevice(1)`, `ContainerList<int>[2]`
  at +0x2b0 and four entries at +0x260.
- **JoystickDevice** (`0x00489800`): `PCInputDevice(2)` and the index at
  +0x260, 32 entries at +0x264, six counters at +0x4e4, `ContainerList<int>[6]`
  at +0x4fc, and bit 0 of +0x574 from the global object's virtual slot 22 with
  `"JoyDirectionFlipped"`.

- **PCKeyboardDevice** (`0x004c43c0`) has an empty constructor body.
  **PCMouseDevice** (`0x004c48c0`) clears four ints at +0x2d8 with `memset`.
  Both destructors (`0x004c4400`, `0x004c4910`) are compiler-generated.
- **PCJoystickDevice** (`0x004c2770`) repeats JoystickDevice's
  initialisation, including the `"JoyDirectionFlipped"` read. Its own fields
  are interleaved with it:
  - five effect objects at +0x578;
  - six-int arrays at +0x58c and +0x5a4;
  - four POV values at +0x5d8, set to -1 (only a loop matches);
  - the POV count at +0x5e8;
  - bits at +0x5d4.

  Its destructor (`0x004c28d0`) calls slots 15 and 16. The slots below are
  strict exact; slots 6–10 (effect creation) and 20 are not reconstructed yet.

  | Slot | VA | Behaviour |
  |---|---|---|
  | 3 | `0x004c3b20` | POV value (hundredths of a degree) in radians; -1 when centred |
  | 4 | `0x004c3ba0` | Slot 3's angle as a sine/cosine direction |
  | 5 | `0x004c3c10` | Property 9 (consistent with DIPROP_AUTOCENTER) |
  | 11–13 | | Stop, start and playing status of an effect (effect methods 8, 7, 9) |
  | 14 | `0x004c4190` | Enumerates effects through method 19 with the static callback `0x004c4260`; the callback copies 0x124-byte (DIEFFECTINFOA-shaped) entries, counting in `.bss` `0x00689958` |
  | 15 | | Releases the effects |
  | 16–18 | | Send force-feedback commands 1, 4 and 8 through method 22 |
  | 19 | `0x004c3af0` | Records joystick subtypes 4–7 from bits 8–15 of the device type |

  Effect-related slots act only for device type 3 (+0x0c). Slot 19 matches
  only with the Windows `HIBYTE` cast chain used by `GET_DIDEVICE_SUBTYPE`,
  which is what produces retail's byte load followed by a redundant
  `and eax, 0xff`. Both `sprintf` buffers (`0x00534aaf` is the CRT's
  `sprintf`) are formatted and never used again, so their output is
  presumably a compiled-out debug print.

Only JoystickDevice stores its vptr in its destructor (`0x00489920`). The
keyboard and mouse destructors (`0x00489f20`, `0x0048a3c0`) are
compiler-generated, so their classes declare none. Each has a scalar
deleting wrapper (`0x00489900`, `0x00489f00`, `0x0048a3a0`).

## ContainerList.h

A growable array template. Its layout:

| Offset | Member |
|---|---|
| +0x00 | Count |
| +0x04 | Data pointer |
| +0x08 | Grow step |
| +0x0c | Capacity |
| +0x10 | Allocated bit |

The out-of-line members are shared by every instantiation through
identical-code folding:

- **Default constructor:** `0x005109c0`.
- **Destructor:** `0x00402040`, which deletes the data.
- **`Reserve`:** `0x005109e0`, which allocates at ContainerList.h line 71 and
  copies with `memcpy`. It is written but not yet matched, because no
  reconstructed caller emits it.

`Init(capacity, growBy)` is inlined into its callers and allocates at line
59. Its statements must run data, grow step, capacity, then allocated bit;
other orders change how VC6 addresses the array elements in the device
constructors.

Arrays of lists are built with the `/GX` vector constructor and destructor
iterators (`0x00536234`, `0x00536140`). Because a device's object file now
holds two `__FILE__` literals (its own and ContainerList.h's), the matcher
keys path literals as `__FILE__:<basename>`, falling back to plain
`__FILE__`.
