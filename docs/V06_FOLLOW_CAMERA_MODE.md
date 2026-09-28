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

## Slot 69 — cached 12-byte value

At `0x00466A80`, the method:

1. invokes another virtual method through vtable offset `+0xE4`;
2. receives a three-dword / 12-byte result;
3. copies those 12 bytes to `this + 0x2A8`;
4. invokes another virtual through vtable offset `+0xAC`.

The 12-byte shape is consistent with a 3-component vector, but that type/name is
not yet promoted because the evidence only proves size and copy behavior.

## Slot 68 — bounded camera parameter update

At `0x00466D50`, the method performs floating-point camera math and updates
`this + 0x258`. Its code contains direct bounds corresponding to approximately
10.0 and 70.0, followed by another virtual dispatch.

This is strong evidence that `+0x258` is a bounded float-like camera parameter,
but it is not enough yet to call it distance/FOV/angle.

## Slot 71 — state dispatch

At `0x00466E50`, the method:

1. stores its argument to `this + 0x244`;
2. calls another virtual method through vtable offset `+0xE8`;
3. switches over the stored value for states 0–4;
4. dispatches to FollowCamera virtuals at offsets including `+0x108`,
   `+0x104`, `+0x100`, `+0xFC`, and `+0xF0`;
5. snapshots the exact preset triplet
   `+0x220/+0x22C/+0x234` into `+0x2C4/+0x2C8/+0x2CC`;
6. calls another virtual through `+0xF4`.

This strongly corroborates `+0x244` as a discrete camera-state field without
proving the original enum/type name.

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


## Concrete C++ calibration candidates for slots 68/69/71

The reconstructed source now lives in `samples/camera/FollowCameraStateProbe.cpp` and is wired into the historical compiler calibration runner.

### Slot 68 — distance-derived bounded parameter

Retail `0x00466D50` is 241 bytes. The recovered flow is:

```text
value = *(12-byte value*)(this + 0x2B4)
value.y += 3.0
dx = abs(*(float*)(this + 0x170) - value.x)
dz = abs(*(float*)(this + 0x178) - value.z)
distance = sqrt_like(dx*dx + dz*dz)
field_258 = (200 - distance) * (1/180) * 60 + 10
field_258 = clamp(field_258, 10, 70)
virtual_slot_29(value)
```

The default clang/MSVC-ABI candidate is **243 bytes**, only two bytes away in total size, but matches just **4.3062%** of comparable bytes because floating-point stack scheduling/comparison codegen differs heavily. This is a strong VC6 calibration target, not a clang match.

### Slot 69 — hidden return-buffer ABI confirmed

Retail `0x00466A80` is 65 bytes. The key ABI question is now resolved by the VehicleCamera slot-57 override at `0x0052CA10`:

- slot 57 returns a **12-byte value by value** using MSVC's hidden result pointer;
- it accepts one explicit 4-byte/float argument;
- slot 69 calls it with `0.0f`;
- retail copies the returned 12 bytes to `this + 0x2A8`;
- it then passes `this + 0x2A8` to virtual slot 43.

The natural C++ candidate is therefore:

```cpp
CameraValue3 value = UnknownVirtualSlot57(0.0f);
field_2A8 = value;
UnknownVirtualSlot43(&field_2A8);
```

Clang emits 86 bytes and matches **16.3934%** of comparable retail bytes. The mismatch is code-shape/toolchain evidence; the calling convention and data flow are strongly supported by the binaries.

### Slot 71 — state dispatcher source candidate

Retail `0x00466E50` is 171 bytes. The source candidate now explicitly preserves the recovered switch:

- write state to `+0x244`;
- call slot 58;
- states 0/1/2 dispatch to preset slots 66/65/64;
- state 3 restores `+0x258` from `+0x2F0` then calls slot 63;
- state 4 saves `+0x258` to `+0x2F0` then calls slot 60;
- snapshot `+0x220/+0x22C/+0x234` into `+0x2C4/+0x2C8/+0x2CC`;
- call slot 61.

Clang emits 151 bytes and matches **18.3673%**. It folds the case dispatch into a computed vtable offset, whereas retail VC6 emits explicit per-case virtual calls and a jump table.

These three functions increase the calibration corpus without changing the exact-match count. They are specifically intended to distinguish the authentic VC6 backend/profile from modern clang.
