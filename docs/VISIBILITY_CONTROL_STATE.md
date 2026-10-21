# Visibility control storage

`src/reconstructed/VisibilityControlState.cpp` supplies three missing mutable
four-byte cells used by the existing visibility implementation. Their retail
addresses lie entirely in the writable `.data` section's zero-filled virtual
tail; none has file-backed initial bytes.

| Cell | Retail address | Decoded use |
| --- | --- | --- |
| `g_visibilityQueryTicks` | `0x0068ab88` | Stores the clock difference around the query and supplies profiling reads |
| `g_visibilityDebugDraw` | `0x0068ab9c` | Updated to `1 - value` for input code `0x44`; gates debug drawing |
| `g_visibilityFreeze` | `0x0068aba0` | Updated to `1 - value` for input code `0x57`; selects the saved camera |

The names and original translation-unit ownership remain provisional. These
are separate globals, not a recovered contiguous structure. In particular,
the query cell precedes the existing timing object at `0x0068ab90`; the two
toggle cells follow it. This definition does not replace the frozen camera,
timing object, collision-tree pointer or overlapping vertex/entry views.

The exhaustive encoded-address scan finds nine references:

| Cell | Operand addresses |
| --- | --- |
| Query ticks | `0x52ef99`, `0x52efc2`, `0x52f02d` |
| Debug draw | `0x52e3b1`, `0x52eee0`, `0x52eeed` |
| Freeze | `0x52ef11`, `0x52ef1e`, `0x52ef5c` |

Every reference belongs to one of three complete, independently strict source
consumers: debug drawing at `0x52e3b0` (148 bytes), OnControlPress at `0x52eec0`
(135 bytes), and PrepareGeometry at `0x52ef50` (252 bytes). Their declarations,
instructions and destination bindings are unchanged. The regression retains
all nine DIR32 destinations and the actual read/write sequences. It also
checks the camera copy's 0x88 DWORD count and the per-frame query's explicit
`-0x874` receiver adjustment; it does not collapse the secondary GameObject
base into the primary QuadTree pointer.

The provider's authentic VC6 object contains only `.drectve` (40 bytes) and
`.bss` (12 bytes), with three distinct four-byte public definitions, no code,
imports, dynamic initializer or nondebug relocations. No original adjacency
is inferred from the new object's ordinary local storage order.

`tests/test_visibility_control_state.py` checks PE section flags and zero-fill
extent, complete explicit address coverage, all three full strict consumers,
and the new COFF storage. This is storage and instruction evidence; it does
not execute keyboard events, camera freezing, scene queries or rendering.
The explicit address scan does not exclude possible indirect uses.

The isolated real build `work/game-build/build-q0ppplkb` compiles **257/257**
translation units and reduces unresolved names **239 to 236**, removing only
these three cells and adding none. All 256 prior objects preserve their
10,675 nondebug sections and 45,728 relocations with no name mappings or filename
exclusions. Independent recompilation reproduces the new object's complete
contents using its actual game command. The game still does not link.

```powershell
$env:VC6_ROOT=(Resolve-Path work/vc6sp3-disc).Path
$env:MCM2_EXE=(Resolve-Path work/game/mcm2.exe).Path
$env:PYTHONPATH='.;tests'
python -m unittest test_visibility_control_state -v
```
