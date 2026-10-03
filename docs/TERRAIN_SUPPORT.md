# Verified Terrain support slice

`src/reconstructed/TerrainSupport.cpp` contains 25 strictly matching VC6 SP3
cases separated from the existing Terrain candidate. The filename is ours;
retail-source attribution is Terrain.cpp. No claim is made that all Terrain
methods or its full linked class are reconstructed.

The camera initializer at `0x00505490` passes 1 and object `0x0068a090` to
`0x004bed80`; the registration at `0x005054a0` passes its shutdown wrapper to
CRT atexit. That wrapper selects `0x004624d0`. These callees agree with the
separately reconstructed PCCamera constructor/destructor, including canonical
Camera/ShadowCamera binding evidence. TerrainSharedState remains an explicit
opaque boundary view because the physics and canonical camera headers have
not been consolidated into one linked class hierarchy.

The ten timer bodies each pass 5000 to `0x004cb670` on the distinct objects
listed in TerrainSupport.bindings.json. The callee is the independently
reconstructed UnknownPeakHold constructor; the source now uses that canonical
type rather than a duplicate timer class. Each thunk is bound to its own
initializer body, and no TU-local initializer identity is inferred from its
compiler-generated name alone.

AcquireOwnedObject at `0x00505600` first reuses the next entry of its object
pool. Its two allocation paths construct PCTextureMap (`0x004c5f00`) or
ManagedTexture (`0x00510500`); the managed path registers through
ManagedTextureGroup at `0x0050c6c0`. All three identities and signatures have
independent canonical reconstructions and RTTI evidence. Physics boundary
views remain explicitly named as such, and the constructor inputs are typed
TextureMapManager pointers. The age/purge callee at `0x004011b0` operates on
the age manager's records, sorting its pointer array through CRT qsort.

The owned-object function's registration prologue selects EH stub `0x0054e5b8`:
it loads FuncInfo `0x00563c28` (magic `0x19930520`) and jumps to `0x0053471a`.
Its allocation sites reference the checked full Terrain.cpp literal at
`0x00574720`. Because our slice has a different filename, the binding key
`__FILE__:terrainsupport.cpp` maps to that observed retail Terrain.cpp literal.
The allocator remains the canonical debug operator-new entry `0x004a3010`.

GetHeightRange at `0x00508970` has no external relocations. It conditionally
writes heightField+`0x18` and +`0x1c`, each multiplied by Terrain+`0x40`, and
returns with an eight-byte argument pop. Its full 41-byte body matches.

```bash
python tools/run_physics_samples.py --strict \
  --source src/reconstructed/TerrainSupport.cpp \
  --vc6-root "$VC6_ROOT" --exe "$MCM2_EXE"
```

The overall selected physics scope and remaining candidates are documented in
[PHYSICS_VALIDATION.md](PHYSICS_VALIDATION.md).
