# Recovered C++ class model evidence

This file summarizes structure recovered mechanically from MSVC RTTI, vtables, vtable writes, and tiny accessor bodies. It is evidence inventory, not an attempt to invent original source declarations.

## RTTI/vtable inventory

For the supplied retail `mcm2.exe`:

- 252 RTTI type descriptors
- 249 logical RTTI class hierarchy records
- 271 concrete vtables
- 22 secondary base-subobject vtables
- 17 classes with multiple direct bases and/or multiple vtables

An important v0.4 correction is that a logical C++ type may have more than one `CompleteObjectLocator`. Older bootstrap code collapsed those records and could accidentally expose a secondary vtable as though it were the class's only vtable. `analysis/rtti_classes.json` now preserves every COL in `vtable_records`, including its `object_offset`.

Examples:

```text
CollisionObject
  direct bases: QuadTreeObject, GraphicsTest
  vtable @ +0x000 : 0x005511b8
  vtable @ +0x00c : 0x00551148

SoultreeObject
  direct bases: QuadTreeObject, GameObject
  vtable @ +0x000 : 0x00557c18
  vtable @ +0x00c : 0x00557c40

Tire
  direct bases: CollisionObject, MovingPart, CollisionPoint
  vtable @ +0x000 : 0x00558578
  vtable @ +0x00c : 0x00558508
  vtable @ +0x0b8 : 0x005584fc

Vehicle
  vtable @ +0x000 : 0x00558a84
  vtable @ +0x21c : 0x00558a50
  vtable @ +0x5c0 : 0x005589e0

Bike
  vtable @ +0x000 : 0x00550900
  vtable @ +0x21c : 0x005508cc
  vtable @ +0x738 : 0x0055085c
```

The Vehicle/Bike layouts also contain virtual-base RTTI entries, so do not interpret every secondary vtable offset as a simple non-virtual direct-base offset.

## Primary vtable override map

`analysis/vtable_overrides.json` compares each class's primary vtable against its primary direct base and marks slots as:

- `inherited`
- `override`
- `introduced`
- `root`

This is safe for the primary vtable lane and useful for separating actual class behavior from inherited slots. Secondary-vtable override attribution is deliberately not flattened because virtual-base and adjustor behavior must be respected.

Example: `UIControl : GameObject` inherits 19 primary slots unchanged, overrides 8 existing slots, and introduces a large UI-specific tail of virtual methods.

## Hard member-offset evidence

`analysis/class_layout_hints.json` contains only direct field evidence from mechanically obvious functions. Current examples include:

```text
BaseObject
  +0x004 : 32-bit field used by GetRefCount / reference-count logic
  minimum evidenced size: 0x008

UIControl
  +0x02c/+0x034 : pair used by a difference getter
  +0x030/+0x038 : second pair used by a difference getter
  +0x07c         : 32-bit setter
  +0x0c0         : 32-bit getter
  +0x0d0         : 32-bit getter
  +0x0dc         : address-of member
  +0x0e4         : 32-bit getter
  +0x1b4         : 32-bit setter
  minimum evidenced size: 0x1b8

PhysicsBody
  +0x180 : 32-bit setter
  +0x234 : conditional 32-bit setter
  minimum evidenced size: 0x238

UIMultiState
  +0x060 : 32-bit setter destination
  +0x1bc : literal state store (`3`)
  +0x1f0 : current element index
  +0x1f4 : pointer/base for 32-byte element records

UIDropDownList/UIListBox family
  +0x060 : 32-bit setter destination
  +0x1c0 : literal state store (`1`)

Vehicle
  +0x454 <- +0x458 : 32-bit member copy
  +0x47c          : pointer dereferenced by a float getter
  +0x4ac          : float getter
  +0x4f0          : 32-bit getter
  +0x51c          : literal dword store (`0x3f000000`, consistent with 0.5f)
  minimum evidenced size: 0x520
```

Types such as `int`, pointer, handle, enum, and bitfield remain provisional unless code use proves them. A 32-bit load/store only proves width.

## Deleting destructors

`analysis/deleting_destructors.json` finds **145** canonical VC6 scalar deleting-destructor wrappers directly in vtable targets. All 145 dispatch deletion to the same retail routine at:

```text
0x004a30c0
```

The common wrapper is the classic VC6 shape:

```asm
push esi
mov  esi,ecx
call destructor_core
test byte ptr [esp+8],1
je   skip_delete
push esi
call 0x004a30c0
add  esp,4
skip_delete:
mov  eax,esi
pop  esi
ret  4
```

This gives us a mechanically recovered destructor-core address for 145 polymorphic class/vtable entries.

## `this` adjustor thunks

`analysis/vtable_thunks.json` currently identifies **28** short vtable adjustor thunks. They include:

- fixed `sub ecx, imm ; jmp target`
- virtual-base `sub ecx,[ecx-4] ; jmp target`
- combined virtual-base + fixed adjustment

Examples from Bike's secondary vtable at `this+0x738`:

```text
0x0040ca40  sub ecx,[ecx-4]          -> 0x0040ca50
0x0040ca90  sub ecx,0x308            -> 0x00446640
0x0040caa0  sub ecx,0x308            -> 0x00446620
0x0040cab0  sub ecx,[ecx-4]; -0x178  -> 0x0052a830
```

These are strong evidence for the real multiple/virtual-inheritance layout and should be retained rather than “simplified” in reconstructed declarations.

## Vtable writes / constructor-destructor evidence

`analysis/vtable_write_xrefs.json` contains **492** decoded `mov [memory], vtable` sites covering all 249 classes. These are strong constructor/destructor leads.

For example, BaseObject has:

```text
0x00405120  constructor start
0x00405122    write BaseObject vtable
0x00405128    refCount = 1

0x00405130  scalar deleting destructor
0x00405150  destructor core
0x00405150    write BaseObject vtable
```

The source candidate is therefore:

```cpp
BaseObject::BaseObject() { refCount = 1; }
BaseObject::~BaseObject() {}
```

The assignment is in the body because retail writes the vptr before
`refCount`; under VC6 SP3 an initializer list `: refCount(1)` emits the two
stores in the opposite order. The constructor and scalar deleting destructor
now match exactly under VC6 (see `docs/VC6_MATCHING.md`).

Modern clang does not reproduce VC6's special-member code shape, so those functions are compiler-calibration targets rather than clang smoke tests.
