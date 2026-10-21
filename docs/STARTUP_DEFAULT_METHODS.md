# Startup and background default methods

Two missing definitions now reproduce their complete retail bodies under
authentic VC6 SP3. They have separate reconstructed source files; that grouping
does not claim original translation-unit ownership. Their shared retail
addresses do not identify them with unrelated classes using the same code.

| Definition | Retail body | Independent identity evidence |
|---|---|---|
| `UnknownTrackGameObject3410Base` constructor | `0x004676a0`, 16 bytes | `TrackGame::CreateGameObjects` allocation and constructor call, followed by storage at `TrackGame+0x3410` |
| `BackgroundImage::UnknownVirtualSlot27` | `0x004627f0`, 8 bytes | Primary vtable `0x005506d8`, slot 27; `RenderPre3D` passes the off-screen or render-target surface |

The constructor moves `this` to its return register, clears DWORDs at offsets
zero and four, and returns. These are the DirectPlay and lobby pointers later
assigned by `SetDirectPlay`. It leaves every other field untouched. The
provisional class name remains unchanged; the folded address also implements
unrelated constructors and supplies no additional type or ownership evidence.

The background hook returns one and pops its four-byte argument. It accesses
neither the object nor the surface. This is the actual default method recorded
in the vtable, rather than invented surface-copy behavior. RTTI independently
places `GameObject` at offset zero. `RenderPre3D` calls slot `0x6c` with the
background receiver and either its `+0x3c` surface or the render target's
`+0x48` surface. Derived overrides remain ordinary virtual dispatch.

`tests/test_startup_default_methods.py` checks the complete emitted function
extents against retail with no ignored bytes, decodes the allocation/storage
and rendering call paths, and checks the primary vtable entry. Its native VC6
fixture links both actual production objects. Across 256 initial storage
patterns, placement construction changes only the first eight bytes of the
complete object, preserving surrounding guards and all other fields. Another
768 qualified hook calls return one and leave complete object storage intact,
including calls with null and invalid surface pointers. The fixture also
checks the declared field offsets at compilation.

The isolated source build `work/game-build/build-rbhd2472/report.json` compiles
219 translation units and removes exactly these two unresolved names:
**372 → 370**. Existing production objects are unchanged. Neither this link
reduction nor the bounded tests demonstrate successful game startup or menus.

With native `VC6_ROOT` and private `MCM2_EXE` configured:

```powershell
$env:PYTHONPATH = '.'
python -m unittest discover -s tests -p test_startup_default_methods.py -v
python tools/build_game.py -j6
```
