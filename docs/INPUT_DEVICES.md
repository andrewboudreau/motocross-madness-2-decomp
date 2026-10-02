# Input devices and ContainerList

RTTI: `InputDevice` → `PCInputDevice` → `JoystickDevice` / `KeyboardDevice` /
`MouseDevice` → `PCJoystickDevice` / `PCKeyboardDevice` / `PCMouseDevice`.
Canonical source is `src/reconstructed/InputDevice`, `PCInputDevice`,
`ControlInterface`,
`KeyboardDevice`, `MouseDevice`, `JoystickDevice`, `PCKeyboardDevice`,
`PCMouseDevice`, `PCJoystickDevice` (`.h`/`.cpp` and bindings)
and the template header `ContainerList.h`. Translation units are not
established, apart from ContainerList.h, which retail names through
`__FILE__`. Names are provisional. All functions listed are strict exact under
the default profile, except two near misses in
`samples/inputdevice/PCJoystickDeviceNearMisses.cpp`.

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
  entries stamped from cdecl `0x004bfa80`, six binding lists
  `ContainerList<UnknownControlBinding*>` at +0x1660 (each `Init(1, 1)`),
  and a modifier state at +0x16d8. Its other functions:
  - Slot 0 (`0x0048a030`) drops bindings by id, like the joystick's.
  - `0x0048a0c0` steps bindings while their keys are held, using float
    repeat timers in the binding and the global frame time at +0x2f0.
  - `0x0048a240` tests a modifier state. 0x3f always holds and 0x80000000
    means none; bit 7 needs bit 2 of the global's +0x2d4 (0x80 also holds
    while key 0x29 is down); bit 6 asks for an exact match.

  PCKeyboardDevice slot 2 (`0x004c4460`) creates the device and releases it
  on failure. Slot 6 (`0x004c45e0`) reads the keyboard:
  - NumLock (0x45) comes from the immediate state.
  - Other keys come from up to 16 buffered events, which are queued on the
    ControlInterface. A lost device is re-acquired, with errors reported at
    lines 1090, 1096 and 1113.
  - Then the key repeat runs and `0x004c4830` rebuilds the modifier mask:
    Shift bits 0 and 1, Ctrl 2 and 3, Alt 4 and 5, the grave key 7. PCKeyboardDevice slot 4 (`0x004c4520`) maps a control to a key and asks
  slot 5. Slot 5 (`0x004c4570`) tests a key with a modifier and copies its
  entry; it matches only as one `&&` condition sharing the failure return.
  Slot 5's return type is contradictory across callers. KeyboardDevice
  tests the full eax, so it is `int`. FollowCamera slot 56 returns it
  unconverted as `bool`, and KrustyBikeCamera overrides slot 56 as `bool`.
  ControlInterface therefore keeps a separate `bool` view of the keyboard
  (`UnknownInterface56e26c`).
- **MouseDevice** (`0x0048a2d0`): `PCInputDevice(1)`, two binding lists at
  +0x2b0 and four button entries at +0x260. Slot 0 (`0x0048a4c0`) drops
  bindings by id. PCMouseDevice implements the rest:

  | Slot | VA | Behaviour |
  |---|---|---|
  | 2 | `0x004c4970` | Creates the device and records the axis and button counts |
  | 3 | `0x004c4a50` | Maps subtypes 3–5 to modes 0–2 |
  | 4 | `0x004c4b10` | Control query: movement directions (beyond ±30), double clicks (presses under 175 apart), directions with button 0 down, else the mapping table |
  | 5 | `0x004c4a90` | Button state |
  | 6 | `0x004c4cf0` | Reads buffered button events (offsets 12–15), then the movement; x and y feed the bindings through MouseDevice `0x0048a550`, scaled by maximum / 375 |
- **JoystickDevice** (`0x00489800`): `PCInputDevice(2)` and the index at
  +0x260, 32 button entries at +0x264, six axis values (floats) at +0x4e4,
  six binding lists `ContainerList<UnknownControlBinding*>` at +0x4fc, and
  bit 0 of +0x574 from the global object's virtual slot 22 with
  `"JoyDirectionFlipped"`. Its other functions:
  - Slot 0 (`0x00489b70`) drops bindings by id.
  - `0x00489a20` attaches a binding to its axis list. Axis n reports controls
    -(2n + 2) and -(2n + 3).
  - `0x00489c00` feeds an axis value to that axis's bindings.
  - `0x00489c60` answers a control query. Controls -2 to -13 are axis
    directions: below 16384 or above 49152, for axes enabled by the six bits
    at +0x14. Other controls go through the mapping table to slot 2.

  Slot 2 (`0x00489980`) is the button query.

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

  Its destructor (`0x004c28d0`) calls slots 15 and 16. Every slot it
  defines (3–20) is strict exact.

  | Slot | VA | Behaviour |
  |---|---|---|
  | 3 | `0x004c3b20` | POV value (hundredths of a degree) in radians; -1 when centred |
  | 4 | `0x004c3ba0` | Slot 3's angle as a sine/cosine direction |
  | 5 | `0x004c3c10` | Property 9 (consistent with DIPROP_AUTOCENTER) |
  | 6 | `0x004c3c40` | Creates a constant-force effect (device method 18, DIEFFECT-shaped block) |
  | 7, 8 | `0x004c3d20`, `0x004c3db0` | Set an effect's magnitude and direction (effect method 6) |
  | 9 | `0x004c3e40` | Constant force with an envelope and an optional trigger button |
  | 10 | `0x004c3f70` | Square wave with an envelope and an optional trigger button |
  | 11–13 | | Stop, start and playing status of an effect (effect methods 8, 7, 9) |
  | 14 | `0x004c4190` | Enumerates effects through method 19 with the static callback `0x004c4260`; the callback copies 0x124-byte (DIEFFECTINFOA-shaped) entries, counting in `.bss` `0x00689958` |
  | 15 | | Releases the effects |
  | 16–18 | | Send force-feedback commands 1, 4 and 8 through method 22 |
  | 19 | `0x004c3af0` | Records joystick subtypes 4–7 from bits 8–15 of the device type |
  | 20 | `0x004c3960` | Polls (method 25), re-acquiring after input-lost/not-acquired errors, then runs the buffered (`0x004c3100`) or immediate (`0x004c3790`) reader |

  The effect GUIDs are named by the literals that `0x004beef0` reports for
  them: `GUID_ConstantForce` (`0x00556b90`) and `GUID_Square`
  (`0x00556bb0`). Slot 20 and `0x004c3a10` pass the literal `__FILE__`
  `PCInputDeviceType.cpp` (lines 540, 544 and 590), so at least they were
  compiled in that TU. In slot 20 the poll check must be an inline member
  (`CheckPollResult`); written in place, VC6 merges the null-device return
  with the error returns. `0x004c3a10` (switch buffered input) matches with
  `goto failed`. `0x004c3790`, the immediate reader, is a near miss. It
  reads a DIJOYSTATE-shaped state, feeds axes through `0x00489c00`, queues
  button changes on the ControlInterface, and keeps the POV values. 373 of
  452 bytes match; the rest is register rotation.

  Device setup and helpers:
  - `0x004c2930` opens a device from its description. It reads the
    capabilities (force feedback makes the device type 3), enumerates the
    axes through the static callback `0x004c2cb0` (GUID_XAxis–GUID_RzAxis),
    and reads each axis range into +0x58c, with the 5% dead-zone scale in
    +0x5bc.
  - `0x004c3090` applies that scale around 32768 and clamps to 0–65535.
  - `0x004c2d90` is the gamepad's axis-driven key repeat.
  - `0x004c2ef0` maps state offsets 0x30–0x4f to button indices.
  - The buffered reader `0x004c3100` is a near miss. Its flow matches, but
    VC6 here keeps 0 in a register for the whole function, while retail
    uses immediate zeros. It filters spikes over 10000, and applies the dead
    zone to X and Y only.
  - Slot 20 uses the "BufferedJoystick" setting through `0x004c3a10`.

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

## Source shapes

Several functions match only in a specific source shape:

- **Device creation** (PCMouseDevice and PCKeyboardDevice slot 2, and
  PCJoystickDevice `0x004c3a10`) needs `goto failed` with one failure block
  placed before the final `return 1`. Early returns, nested ifs and result
  variables all lay the blocks out differently, or fold the last test into
  `setge`.
- **Button queries** need a single `&&`/`||` condition so that every failure
  shares one `return 0`.

The device-creation calls use GUIDs that decode to GUID_SysMouse,
GUID_SysKeyboard and IID_IDirectInputDevice7A, and data formats with the
layout of c_dfDIMouse and c_dfDIKeyboard. The DirectInput object is at
ControlInterface +0xcc0.

## ControlInterface

RTTI ControlInterface is a root class with PCControlInterface derived from
it. Slot 0 is the deleting destructor (`0x0043ce40`) and slots 1–4 are
`_purecall`. The global object at `0x0056e26c` holds one at +0x14; the
evidence is the members the constructor (`0x0043ce00`) initialises and the
input code uses. Its layout:

| Offset | Member |
|---|---|
| +0x10 | Eight joysticks (ControlInterface `0x0043cf00` calls their slot 20) |
| +0x30 | Mouse |
| +0x34 | Keyboard (FollowCamera queries left Alt and left Shift, 0x38 and 0x2a, through its slot 5) |
| +0x38 | Event count |
| +0x3c | 160 queued 20-byte events, added by `0x0043cea0` |
| +0xcbc | Mapping table, set by `0x0043ce70` |

The 0x3c-byte, non-polymorphic `UnknownControlBinding` helpers sit just
before it (`0x0043cc40`–`0x0043cde0`). They cover the default and full
constructors, reset, and a value mapper with a dead zone and min/max clamp.
The clamp matches only as inline helper functions, and the mapper unbinds
through the device's slot 0. All 12 are strict exact. The TU is not
established.

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
  copies with `memcpy`. Its out-of-line copy is not matched yet, but its
  inlined copy matches inside JoystickDevice `0x00489a20`. That copy shows
  the new buffer is stored before the old one is deleted.

`Get`, `Add` and `Remove` (swap with last) are inlined into the joystick
binding functions.

`Init(capacity, growBy)` is inlined into its callers and allocates at line
59. Its statements must run data, grow step, capacity, then allocated bit;
other orders change how VC6 addresses the array elements in the device
constructors.

Arrays of lists are built with the `/GX` vector constructor and destructor
iterators (`0x00536234`, `0x00536140`). Because a device's object file now
holds two `__FILE__` literals (its own and ContainerList.h's), the matcher
keys path literals as `__FILE__:<basename>`, falling back to plain
`__FILE__`.
