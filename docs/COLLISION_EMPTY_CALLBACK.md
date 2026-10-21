# Collision hit-by default callback

`CollisionEmptyCallback` now supplies the missing default collision callback
with ordinary empty C++. Authentic VC6 emits the complete one-byte `RET` at
retail `0x00464e90`. The following fifteen bytes are alignment NOPs; the
preceding function at `0x00464e80` instead returns with four-byte stack cleanup.
This is not a placeholder for missing behavior.

The identity evidence is the installation and invocation chain, not the shared
tiny address alone. The complete strict 59-byte
`ConstraintMethodCollisionModel::SetRenderContext` receives the secondary
GraphicsTest subobject at primary `+0xc`, obtains the primary pointer, and
installs `0x00464e90` at secondary `+0x80`, hence primary `+0x8c`.
The existing contact callback is separately installed at primary `+0x88`.

The complete strict 1,102-byte `CollisionObject::QueryCollisions` invokes the
hit-by callback in three paths, at `0x00438fe5`, `0x00439112` and `0x0043927d`.
Each loads the hit object's `+0x8c` pointer, checks it for null, pushes the
querying object followed by the hit object, calls indirectly, and adds eight
to ESP. Thus the existing two-pointer cdecl callback declaration is supported
by the actual call sites. The callback result is unused. Semantic names and
the original translation unit remain provisional; other methods sharing the
same retail address retain their separate identities and calling conventions.

The new object contains only the 40-byte compiler directives section and a
16-byte executable COMDAT (one return plus alignment). It introduces no
imports, nondebug relocations, storage, virtual table or CRT initializer.
No shared header, existing caller or compiler flag changes. The calibration
runner includes the full callback extent.

`tests/test_collision_empty_callback.py` checks the hash-pinned retail image,
all three complete strict bodies, the provider's complete section contents and
flags, the secondary-pointer installation, all three argument/cleanup sequences,
and the actual compiled member offsets and callback type. It does not claim
native execution of the broad-phase query or integrated collision gameplay.

```powershell
$env:PYTHONPATH = '.;tests'
python tests/test_collision_empty_callback.py -v
```

Set `VC6_ROOT` and `MCM2_EXE` to the verified private inputs. The isolated real
build `build-4h522ikm` compiles 260/260 translation units and reduces unresolved
symbols from 224 to 223, removing only this callback with no additions.
All 259 preceding production objects preserve 10,691 nondebug sections and
45,776 relocations under the separately verified two UIAnim call-name changes,
without filename exclusions. Linking still fails; startup, menus and racing
remain unverified.
