# Collision tick forwarding

`src/reconstructed/CollisionObjectTick.cpp` supplies the missing
`CollisionObject::Tick(float)` body. Authentic VC6 SP3 `/O2 /GR /GX /MT /Z7`
emits all 13 bytes at retail `0x00499ae0` exactly after resolving its one call
to `GameObject::Tick` (`0x004693d0`). No padding is substituted for function
bytes, and no original translation-unit ownership is claimed.

## Identity and receiver

CollisionObject's RTTI identifies the nonvirtual GameObject base at offset
12. Its secondary table `0x00551148` has slot 10 pointing to `0x00499ae0`.
The table's complete-object locator independently confirms the offset and
CollisionObject type. The primary QuadTreeObject base remains at offset zero.
The shared address used by other forwarding methods does not establish their
class identities.

The retail body copies the four-byte argument, calls `GameObject::Tick`, and
returns with `ret 4`. It performs no additional receiver adjustment: it is
entered on the secondary GameObject subobject. VC6 emits this convention from
the ordinary multiple-inheritance declaration. The natural qualified call in
the native fixture adjusts a complete CollisionObject pointer by 12 bytes.

ConstraintMethodCollisionModel's secondary table `0x005511f0` independently
identifies its override at `0x0043ba40`. That complete 37-byte body remains
strict: it conditionally invokes slot 11, then forwards the same secondary
receiver and float argument to CollisionObject::Tick. The existing complete
94-byte GameObject::Tick provider remains strict too. The collision object's
emitted table relocation uses the newly supplied method name at slot 10.

## Validation and limits

`tests/test_collision_tick.py` verifies both RTTI/base relations and table
identities directly from the hash-pinned retail image, decodes the complete
forwarder and caller, checks all three bodies strictly, and inspects the
compiled table relocation. The new provider imports only GameObject::Tick and
the ordinary CRT floating-point marker.

Its native fixture executes the actual provider in 4,096 cases with complete
guarded object storage. A scripted GameObject::Tick boundary checks the exact
secondary receiver and raw argument bits, mutates both base regions, returns
varying signed values or throws after mutation. Inputs include signed zero,
finite values, infinities, quiet NaN and signaling NaN bit patterns. The
forwarder preserves these bits because it performs no floating-point work.
These are forwarding and exception-propagation checks; they do not establish
integrated collision scheduling or a running race.

The isolated real link `work/game-build/build-07vzkgwm/report.json` compiles
233 translation units and reduces unresolved names from **310 to 309**.
Only `?Tick@CollisionObject@@UAEHM@Z` disappears; no new missing name appears.
The game still does not link.

```powershell
$env:PYTHONPATH='.'
$env:VC6_ROOT=(Resolve-Path work/vc6sp3-disc).Path
$env:MCM2_EXE=(Resolve-Path work/game/mcm2.exe).Path
python -m unittest discover -s tests -p test_collision_tick.py -v
python tools/build_game.py -j6
```
