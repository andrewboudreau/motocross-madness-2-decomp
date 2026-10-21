# Independently evidenced empty lifecycle bodies

Two missing providers now have their complete readable definitions in separate
source files: `GUIManagerHooks.cpp` defines
`GUIManager::UnknownFunction464e90()`, and `SceneArchiveFinderLifetime.cpp`
defines `UnknownTrackGameObject574::~UnknownTrackGameObject574()`.
Both bodies are empty because that is the complete observed retail behavior.
They are separate C++ definitions, not aliases, wrappers or fallback dispatch.
The GUI hook's original method name, the finder's original class name and their
defining translation-unit ownership remain unproven. GUIManager itself is
identified by RTTI; its hook retains the provisional address-based name.

## Complete body and independent roles

The pinned executable SHA-256 is
`31fde4cc686a5ee89ef9095b90235325b195596867ecacefe511263e1509b874`.
At `0x464e90` the entire reachable body is one `RET` (`c3`), with no calls,
branches, memory accesses, EH frame or callee stack-pop operands. Fifteen NOPs
follow before a distinct function starts at `0x464ea0`. No extent was shortened
to make a candidate fit. Other retail methods share this tiny body; that alone
establishes neither their identity nor their class relationship.

The two selected roles have independent direct-call and lifecycle evidence:

- `GUIManager::Tick`, `0x485870` / 83 bytes, clears `paletteChanged` at +0x1f0,
  sets ECX to the same GUIManager receiver and calls `0x464e90` at `0x4858a8`.
  `UIDialog::~UIDialog`, `0x46a070` / 649 bytes, also calls it at `0x46a0dc`
  using the GUI manager pointer from dialog+0x30, after the 8-bit pixel-format
  and `0x4864f0` checks. No stack argument or result is used. A zero-manager
  branch reaches this latter call too; the empty body must not dereference ECX.
  The palette context supports the descriptive role, not a recovered name.
- `TrackGame::Shutdown`, `0x521a70` / 103 bytes, loads the scene finder from
  +0x574 into EDI, tests it, moves it to ECX, and calls `0x464e90` at `0x521a80`.
  It then separately passes that same pointer to `operator delete` at `0x4a30c0`
  and clears +0x574. The existing constructor is `0x4e99d0`; the declared finder
  has no base classes or nontrivial value members. This sequence identifies the
  empty destructor independently of the GUI call. It does not release
  `sceneArchive`; no resource cleanup has been invented.

Both definitions remain out of line in their own source files. Making them
inline or exposing bodies in existing caller translation units could erase
calls and change already verified caller bytes. Including only the relevant
class declaration also avoids unrelated source or initializer dependencies.

## Verification

```powershell
$env:VC6_ROOT = (Resolve-Path work/vc6sp3-disc).Path
$env:MCM2_EXE = (Resolve-Path work/game/mcm2.exe).Path
python -m unittest discover -s tests -p test_empty_lifecycle_providers.py -v
```

Authentic VC6 SP3 emits each provider as a distinct public function with a
one-byte COFF function extent, exactly matching the complete retail body. Each
object has no external imports, CRT initialization or non-debug relocations.
The focused test decodes full caller bodies from their true entries before
checking the receiver setup, direct call sites and separate deallocation.
Actual caller objects must retain calls to the role-specific provider symbols.

Compile-only fixture functions independently check that a supplied pointer is
passed unchanged in ECX to each distinct provider. They execute no unconstructed
objects and do not infer lifecycle semantics from arbitrary receiver storage.
The test preserves **501 complete strict checks**, including both new providers,
all GUIManager, GameUi, TrackGame and SceneManager calibration entries, and the
existing scene-finder constructor. No identity of other functions sharing the
same `RET` address is claimed.

The fresh source build compiled **200/200** translation units and reduced
unresolved names **524 to 522**, exactly these two names with none added. It
still does not link; startup, menus and racing remain unverified.
