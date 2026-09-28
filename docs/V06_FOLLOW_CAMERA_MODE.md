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

Retail address:

```text
0x00467040
size: 101 bytes
```

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

Interpretation remains deliberately provisional:

- `+0x244`: current camera state/mode candidate
- `+0x248`: saved camera state/mode candidate
- `+0x24C`: saved copy of `+0x258`
- `+0x258`: float-like camera parameter used by the exact preset methods
- `+0x268`: low-byte input materialized into a 32-bit field
- `+0x26C`: state/flag reset when enabling the temporary state

The literal state value `5` is confirmed; its semantic enum name is not.

The ordinary C++ candidate is **not** a clang exact match:

```text
retail:    101 bytes
clang:      99 bytes
match:      18.1818%
```

The branch behavior/member offsets agree while register allocation and code
shape differ substantially. This is therefore a **VC6 calibration target**, not
a reason to contort the C++ around modern clang.

## Slot 69 — cached 12-byte value

At `0x00466A80`, the method:

1. invokes another virtual method through vtable offset `+0xE4`;
2. receives a three-dword / 12-byte result;
3. copies those 12 bytes to `this + 0x2A8`;
4. invokes another virtual through vtable offset `+0xAC`.

The 12-byte shape is consistent with a 3-component vector, but that type/name is
**not yet promoted** because the current evidence only proves size and copy
behavior.

## Slot 68 — bounded camera parameter update

At `0x00466D50`, the method performs floating-point camera math and updates
`this + 0x258`. Its code contains direct bounds corresponding to approximately
10.0 and 70.0, followed by another virtual dispatch.

This is strong evidence that `+0x258` is a bounded float-like camera
parameter, but it is not enough yet to call it distance/FOV/angle.

## Slot 71 — state dispatch

At `0x00466E50`, the method:

1. stores its argument to `this + 0x244`;
2. calls another virtual method;
3. switches over the stored value for states 0–4;
4. dispatches to other FollowCamera virtuals and uses fields including
   `+0x258` and `+0x2F0`.

This corroborates `+0x244` as a discrete camera-state field without proving
the original enum/type name.

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
+0x2F0   referenced by slot 71 state dispatch
```

Only offsets and direct machine behavior are evidence-backed. Semantic names
remain provisional until callers, strings, asset data, or additional code
corroborate them.
