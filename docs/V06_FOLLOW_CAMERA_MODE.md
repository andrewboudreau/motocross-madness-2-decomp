# FollowCamera mode/state control — v0.6

This note extends the exact preset reconstruction in `V06_CAMERA_UI.md` into the
next FollowCamera vtable neighborhood.

## Ownership

RTTI/vtable override evidence says `FollowCamera : PCCamera` introduces slots
63–72. `VehicleCamera`, `BikeCamera`, and `KrustyBikeCamera` inherit many of
those addresses unchanged. Therefore the methods below belong to the
**FollowCamera layer**, not BikeCamera merely because they also appear in the
BikeCamera vtable.

The strongest recovered source-unit candidate remains:

```text
D:\aardvark\VC\krusty2\FollowCam.cpp
```

This is supported by normalized filename/class overlap and nearby recovered
`__FILE__` xrefs.

## Slot 70 — state/mode toggle

Retail address `0x00467040`, size 101 bytes.

The direct machine behavior is equivalent to:

```cpp
field_268 = arg & 0xff;

if ((unsigned char)arg != 0) {
    current = field_244;

    if (current != 5) {
        field_248 = current;
        field_24C = field_258;
        field_244 = 5;
    }

    field_26C = 0;
} else {
    field_244 = field_248;
    field_258 = field_24C;
}
```

Interpretation remains provisional:

- `+0x244`: current camera state/mode candidate
- `+0x248`: saved camera state/mode candidate
- `+0x24C`: saved copy of `+0x258`
- `+0x258`: float-like camera parameter used by the exact preset methods
- `+0x268`: low-byte input materialized into a 32-bit field
- `+0x26C`: state/flag reset when enabling the temporary state

The literal state value `5` is confirmed; its semantic enum name is not.

The ordinary C++ candidate is not a clang exact match:

```text
retail:    101 bytes
clang:      99 bytes
match:      18.1818%
```

The branch behavior/member offsets agree while register allocation and code
shape differ substantially. This is therefore a VC6 calibration target.

## Slot 72 — cyclic state-list advance

Retail address `0x00466FB0`, size 62 bytes.

The machine behavior is equivalent to:

```cpp
field_30C++;

if (field_30C >= field_310)
    field_30C = 0;

value = field_314[field_30C];
field_244 = value;
virtual_slot_71(value);
```

This adds direct layout evidence for:

- `+0x30C`: current index
- `+0x310`: item count / exclusive upper bound
- `+0x314`: beginning of an inline dword state/value table

The selected table value becomes `field_244`, tying this method directly to
the state dispatcher in slot 71.

Modern clang provides an unusually useful compiler-profile experiment here:

| clang CPU target | candidate size | comparable match |
|---|---:|---:|
| i686 / Pentium Pro+ | 47 | 17.02% |
| Pentium / Pentium-MMX | 46 | 19.57% |

P6+ clang replaces the wrap branch with `cmovl`. Pentium-target clang removes
the CMOV but still folds/reorders the retail load/store sequence. This does
**not** prove the original `/G5` vs `/G6` setting by itself; it makes slot 72
a particularly useful VC6 profile-calibration target.

## Slot 69 — cached 12-byte aggregate / hidden return buffer

At `0x00466A80`, the call sequence is stronger than a generic 12-byte copy:

1. reserve a 12-byte local;
2. push explicit argument `0`;
3. push the address of that 12-byte local;
4. call virtual slot 57 through vtable offset `+0xE4`;
5. use returned EAX as the address of that same aggregate;
6. copy the three dwords to `this + 0x2A8`;
7. call virtual slot 43 through `+0xAC`, passing the cached aggregate by reference.

That is a strong match for the MSVC x86 hidden-return-buffer ABI for a
12-byte struct returned by value. A deliberately non-semantic provisional
declaration is:

```cpp
struct CameraValue12 {
    unsigned int a, b, c;
};

virtual CameraValue12 UnknownVirtualSlot57(int mode);
virtual void UnknownVirtualSlot43(const CameraValue12& value);
```

and slot 69 becomes:

```cpp
CameraValue12 value = UnknownVirtualSlot57(0);
field_2A8 = value;
UnknownVirtualSlot43(field_2A8);
```

The 12-byte type is intentionally **not** called `Vector3` yet. The size,
copy behavior, hidden-return-buffer convention, and cache offset are strong
evidence; semantic type identity still needs use-site corroboration.

With the modern clang calibration profile using `/GS-`, the candidate is
64 bytes versus retail's 65 bytes. Register allocation and three-dword copy
scheduling differ, so this is a historical-compiler calibration target rather
than a clang smoke match.

## Slot 68 — bounded camera parameter update

At `0x00466D50`, the method performs floating-point camera math and updates
`this + 0x258`. Its code contains direct bounds corresponding to approximately
10.0 and 70.0, followed by another virtual dispatch.

This is strong evidence that `+0x258` is a bounded float-like camera parameter,
but it is not enough yet to call it distance/FOV/angle.

## Slot 71 — state dispatch

Retail address `0x00466E50`, size 171 bytes.

The machine behavior reconstructs cleanly as ordinary C++:

```cpp
field_244 = value;
virtual_slot_58();

switch (field_244) {
    case 0:
        virtual_slot_66();
        break;
    case 1:
        virtual_slot_65();
        break;
    case 2:
        virtual_slot_64();
        break;
    case 3:
        field_258 = field_2F0;
        virtual_slot_63();
        break;
    case 4:
        field_2F0 = field_258;
        virtual_slot_60();
        break;
}

field_2C4 = field_220;
field_2C8 = field_22C;
field_2CC = field_234;
virtual_slot_61();
```

This ties several previously independent observations together:

- `+0x244` is a discrete current-state value;
- exact preset methods at slots 64–66 are direct state-dispatch destinations;
- state 3 restores `+0x258` from `+0x2F0` before slot 63;
- state 4 saves `+0x258` to `+0x2F0` before slot 60;
- every dispatch snapshots the three exact preset fields into
  `+0x2C4/+0x2C8/+0x2CC`.

The readable candidate compiles to 151 bytes under the current clang
MSVC-x86 profile versus 171 retail bytes, with about **18.37%** comparable
byte agreement. The semantic structure is much stronger than that raw number:
the difference is dominated by switch lowering, register allocation, and copy
scheduling. Slot 71 is therefore a high-value VC6 calibration target.

## Current direct FollowCamera field map

```text
+0x220   exact preset parameter
+0x22C   exact preset parameter
+0x234   exact preset parameter
+0x244   discrete current state/mode candidate
+0x248   saved state/mode candidate
+0x24C   saved copy of +0x258
+0x258   bounded float-like camera parameter
+0x268   low-byte toggle/input materialized as dword
+0x26C   state/flag reset on enable
+0x2A8   12-byte cached value
+0x2C4   snapshot of +0x220
+0x2C8   snapshot of +0x22C
+0x2CC   snapshot of +0x234
+0x2F0   state-specific saved copy of +0x258
+0x30C   cyclic state/value-table index
+0x310   cyclic state/value-table count
+0x314   inline dword state/value table begins
```

Only offsets and direct machine behavior are evidence-backed. Semantic names
remain provisional until callers, strings, asset data, or additional code
corroborate them.
