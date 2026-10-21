# Bike focus-point vector connections

Bike slot 46 (`0x0040b600`, 1066 bytes) now uses the shared `Vec3` constructor
and addition/subtraction operators from `math/Vec3.h` and `math/Math3D.h`.
The complete function retains its retail match. VC6 naturally emits these
out-of-line COMDATs from the existing inline definitions:

| Operation | Retail VA | Complete bytes |
| --- | --- | ---: |
| Three-float constructor | `0x00404e60` | 25 |
| Addition | `0x00421cb0` | 69 |
| Subtraction | `0x00421d00` | 69 |

The constructor uses ECX for the three-float object and pops its three scalar
arguments. The free operators receive their two references after a hidden
12-byte result-buffer pointer, return that pointer, and use caller cleanup.
The direct calls, temporary copies and full caller bytes establish the ABI;
the provisional name `Vec3` does not claim the original source type.

Removing `BikeOolVec3` and `BikeVec3Add` connects two formerly missing link
names. The slot also stops calling the local subtraction alias. That alias
remains used by the unfinished steering method at `0x0040a520`, alongside its
constructor, scale, dot and compound-scale call views. Those are not supplied
by this slice. All 51 strict Bike cases and all seven partial comparison
results remain unchanged.

`tests/test_bike_math_connections.py` compiles the actual Bike and
CollisionObject translation units with VC6 SP3. It verifies the three complete
COMDAT bodies in both objects, their real call relocations in slot 46, absence
of the removed aliases, and all required Bike strict cases. No duplicate alias
implementation, wrapper, compiler attribute or linker substitution is added.
Original math-header and translation-unit ownership remain provisional.
