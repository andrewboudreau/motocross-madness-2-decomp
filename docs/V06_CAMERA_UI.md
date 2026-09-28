# v0.6 camera / UI reconstruction slice

This pass promotes a coherent set of evidence-backed virtual methods rather than isolated return stubs.

## UIControl

RTTI confirms `UIControl : GameObject`. The primary vtable places the following method at slot 50:

```text
VA 0x00470640, 21 bytes
[this + 0x1C0] = 0
[this + 0x1BC] = 3
return
```

The ordinary C++ candidate in `samples/gameui/UIControlProbe.cpp` matches **21/21 retail bytes** under clang-cl's 32-bit MSVC ABI.

Together with the earlier accessors, direct UIControl layout evidence now includes `+0x2C/+0x30/+0x34/+0x38/+0x7C/+0xC0/+0xD0/+0xDC/+0xE4/+0x1B4/+0x1BC/+0x1C0`.

## FollowCamera ownership correction

The shared addresses at slots 63–67 initially appeared while looking at BikeCamera, but the primary override map resolves ownership:

- `FollowCamera : PCCamera` **introduces** slots 63–67.
- `VehicleCamera` inherits the same addresses.
- `BikeCamera` inherits them through VehicleCamera.
- `KrustyBikeCamera` also inherits them.

The strongest recovered translation-unit candidate is `D:\aardvark\VC\krusty2\FollowCam.cpp`: the filename/class normalized names overlap and a recovered `__FILE__` xref is within 0x100 of FollowCamera structural evidence.

### Exact presets

The ordinary C++ candidates for slots 64–67 match exactly:

| Slot | VA | Retail bytes | Result |
|---:|---:|---:|---|
| 64 | `0x00466CB0` | 31 | **31/31 exact** |
| 65 | `0x00466CD0` | 31 | **31/31 exact** |
| 66 | `0x00466CF0` | 41 | **41/41 exact** |
| 67 | `0x00466D20` | 41 | **41/41 exact** |

These provide direct field evidence at `+0x220`, `+0x22C`, `+0x234`, and `+0x258`.

Several immediate bit patterns decode cleanly as floats (for example 85.0, 15.0, pi, and 0.7), which suggests camera preset parameters, but semantic field names remain provisional until callers/usages corroborate them.

### VC6 calibration slot

Slot 63 at `0x00466C90` is semantically clear:

```cpp
field_22C = 0;
field_234 = 0;
field_220 = 0;
```

Retail VC6 emits `xor eax,eax` once and fans EAX into all three stores. Modern clang emits immediate-zero stores instead, so this remains a compiler-calibration target rather than a clang smoke match.

## Validation state after this slice

Local clean-room validation against the supplied retail executable:

- 57 mechanically classified easy targets
- 39/39 generated high-confidence probes exact
- 19/19 hand-written smoke targets exact
- 40 unique exact retail function addresses
- 12 compiler-calibration cases
- 14 classes / 39 direct field offsets
- 62 function-manifest records
- work queue: 40 validated / 22 next

No game or Microsoft compiler binaries are included.
