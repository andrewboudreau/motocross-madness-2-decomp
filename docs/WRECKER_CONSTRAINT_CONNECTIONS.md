# Wrecker constraint-model connections

Wrecker's ragdoll setup and probe append now call the existing
`ConstraintMethodCollisionModel` constructor, `SetBody`, and `AddProbePoint`.
The complete providers at `0x43b8d0`, `0x43b9e0`, and `0x43c7f0` are respectively
121, 85, and 139 bytes and match retail under authentic VC6 SP3. This removes
three provisional external spellings without adding an implementation, wrapper,
linker alias, or changing a class declaration. Original source ownership is not
inferred from this connection.

The setup caller at `0x5301c0` is a complete strict 995-byte body. It allocates
`0x10c` bytes, calls the constructor with argument 1, and stores the primary
pointer at Wrecker `+0x38`. Retail RTTI and constructor stores distinguish the
primary CollisionObject/QuadTreeObject subobject from the GameObject subobject
at `+0xc`. The latter adjustment remains at virtual dispatch and `AppendChild`;
the constructor and the two nonvirtual methods receive the primary pointer.
The allocated canonical object is viewed through the existing Wrecker field
declaration for the remaining accesses. This does not unify every provisional
type used by Wrecker.

`SetBody` receives the separate `0x2a8`-byte PhysicsRigidBody allocation stored
at Wrecker `+0x40`. Its constructor writes the primary `0x556ec0` vtable. Slot
28 stores the rider's model node at body `+0x234`, which the complete `SetBody`
provider reads. The `ConBody` cast is limited to that existing boundary and
does not claim complete declaration equivalence.

The full 210-byte `Wrecker::AddProbe` caller passes a 12-byte vector by value
followed by a 32-bit node pointer. Its older `int bone` parameter carries the
unaltered result of `SoultreeObject::FindByName`: all fifteen calls in the full
1,516-byte Bike slot 97 preserve the return pointer before forwarding it.
The provider stores the pointer at probe record `+0xc`, the three coordinate
words at `+0/+4/+8`, and clears the hit word at `+0x10`. Records have a 32-byte
stride. The retained declarations and explicit casts preserve these bits;
`bone` is not treated as an integer index.

## Preservation

The canonical Wrecker object retains all 67 nondebug sections and 237
relocations; its aggregate retains all 72 sections and 277 relocations. Every
payload byte, section flag, relocation type/addend, and resolved local
destination remains unchanged under exactly the three method-name mappings.
Same-path comparisons exclude no regions or filenames. Portable retained pins
normalize only the explicitly enumerated canonical and sample filenames.

The retained test checks 87 complete strict bodies, including all 29 Wrecker
calibration bodies in both objects, all 19 constraint providers, the Bike and
PhysicsRigidBody producer bodies, the node setter and finder, and the actual
native helper extractions. The two unfinished Wrecker sample functions retain
their complete resolved fingerprints and scores of 1,200/1,312 and 1,606/1,624;
neither is promoted or rebaselined. Existing partial pins need no updates.

## Native boundary

The fixture executes the unchanged source `Wrecker::AddProbe`, the actual
`ConstraintMethodCollisionModel::AddProbePoint`, and real `MemTag.cpp`
allocation/accounting and copy code. Both extracted append functions retain
all instruction bytes and relocation targets from their complete source
objects, apart from the explicitly identified extraction filename. Retail
equivalence is established by their full strict comparisons, not by executing
a mapped retail image in this fixture.

There are 2,560 ordinary cases: five initial counts (0, 1, 7, 15, 31), 128
deterministic raw coordinate/node patterns, two argument-storage choices,
and two allocator-accounting routes. The nonempty aliased cases read the
by-value point from an old probe that the real allocator subsequently frees.
Independent oracles check all old array bytes, the complete new probe record,
new zero position/hit entries, the full prepared receiver/model state and
guards, allocation sizes/free ownership, and accounting changes. The new
allocation's otherwise uninitialized bytes are filled with a known sentinel
at the instrumented CRT boundary, allowing the untouched normal field to be
checked rather than ignored.

Eight additional cases independently check the first access fault's kind,
address, and read/write direction, completed allocations, exact partial
stores, and accounting. These cover short receiver/model storage and each of
the three allocation failures. Failed allocations use initial count zero;
the fixture does not pretend that realloc fallback into an undersized old
block is safe. The complete allocator object is unchanged under only the
`malloc`, `free`, and `_msize` instrumented CRT names. Successful operations
use the real VC6 CRT; explicit NULL injection and sentinel filling are fixture
contracts, not claims about all heap failures or arbitrary allocator behavior.

Native receivers and accounting records are prepared raw storage. The
constructor and `SetBody` have strict, raw producer, layout, and call-ABI
evidence, but their complete lifetime/shape-loading graph is not executed
here. The finite integer-copy checks do not establish a running ragdoll,
solver behavior, arbitrary invalid-pointer semantics, or portable C++ object
lifetime and aliasing guarantees.

Reproduce with `VC6_ROOT` and `MCM2_EXE` set to the verified private inputs:

```powershell
$env:PYTHONPATH='.;tests'
python -m unittest test_wrecker_constraint_connections -v
```

## Actual game integration

Build `work/game-build/build-taq3kk_h` compiles 273/273 translation units and
removes exactly these three unresolved symbols: **183 -> 180**, none added.
Every old production object preserves its complete bytes and relocation
destinations under those three names: 10,866 nondebug sections and 46,268
relocations, with no filename exclusions. The shared header resolves its
collision include relative to its own directory.
Both Wrecker units also compile without project include paths in the retained
test, preserving the standalone calibration route. This include-path repair
preserves all production objects without symbol mappings or exclusions. Build
configuration and compiler flags remain unchanged. The retained test passes at
the actual source path. Startup, integrated menus and racing remain unverified
because the game still does not link.
