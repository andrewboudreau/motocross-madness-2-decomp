# VC6 pattern corpus

Retail code shapes that some near misses do not reproduce, looked up in the
functions that are already exact. A scan of retail bytes found which exact
functions contain each shape. Their matched source shows the construct that
produces it, and small probe files compiled with the calibrated profile
(`vc6_o2_mt`) check the rule. "Exact" means strict exact in the calibration
(2968) and physics (1072) gates of 2026-10-20. Near misses come
from those gates and from [NEAR_MISS_INDEX](NEAR_MISS_INDEX.md).

Counts are of functions whose retail bytes contain the shape at least once.

| shape | exact | near misses |
|---|---|---|
| `xor r, r` then `cmp x, r` | 384 | 82 |
| `jcc exit` directly followed by `jmp top` | 24 | 1 |
| `sub esp, N` directly before `rep movs` | 2 | 5 |
| `rep movs` directly before `sub esp, N` | 0 | 2 |
| by-value push, source word loaded before `sub esp, 0xc` | 37 | 22 |
| by-value push, `sub esp, 0xc` before the first load | 51 | 26 |
| a load hoisted above the first branch, unused before it | 31 | 5 |

## 1. A compare against a zero register

The thresholds and register choice for zero constants are in
[VC6_ZERO_REGISTER](VC6_ZERO_REGISTER.md). This section adds the compare
case.

**Rule (confirmed by a probe):** VC6 compares a value with a register that
holds zero (`xor eax, eax ; cmp edx, eax`) when the zero is a constant that
is never redefined. Two cases produce it: a zero store later in the function
on every path, and a local initialised to 0 that is never assigned again
(the "register variable initialised to the constant" case of
VC6_ZERO_REGISTER). When the zero is a variable that is redefined on some
path, VC6 tests with `test r, r` instead and does not reuse the register.

| probe | code |
|---|---|
| `if (!exhausted) g_n--; current = 0;` | `xor eax, eax ; cmp edx, eax ; ... mov [ecx+4], eax` |
| `int v = 0; if (!exhausted) g_n--; current = v;` | the same |
| `int x = 0; ... if (r) { if (b == 0) y = 0; else { x = r[b]; ... } }` | `xor edx, edx ; test eax, eax` |
| zeros stored only inside one branch | `test`, with the `xor` inside the branch |
| `if (!exhausted) { ...; current = 0; } else current = 1;` | `cmp dword ptr [ecx], 0` |

Exact examples:
- GameObjectIterator::Stop `0x00469a20` (`currentObject = 0` after the test).
- UIAnim::Rewind `0x00472f50` (zero stores in both arms).
- KrustyUI::ClearEntries `0x0049bb80` (`entries = 0` after a guarded free; the zero is kept in edi across the call).
- D3DIMSoultreeObject::SetLowestLod `0x00444d40` and Terrain `0x00507930` (a clamp to 0, then the clamped value is compared and stored).

Near misses that differ in this shape:
- Griddraw `0x00480ad0` (render).
- BikeRace `0x0041f1d0` (race): retail keeps 0 in edx from the first instruction.
- GUIManager `0x00486170` (ui): retail has no `xor ebx, ebx`.
- Net `0x004ab960` (net): retail tests where the candidate compares with ebx.
- KrustyBike `0x00493660` (physics): retail keeps 0 in edi.
- FileStream `0x00461340` (io).

The rule says which way to move each of these: make the zero a constant
that is never redefined, or make it a variable that is redefined.

In Griddraw `0x00480ad0`, retail's eax behaves as such a constant. It is
compared with `ranges` and `block` and stored to `indexStart`. The forms
tried so far, and why each misses, are in VC6_ZERO_REGISTER's Griddraw
entry: no source found yet keeps the zero at the top without a
redefinition.

## 2. A loop bottom of `jcc exit ; jmp top`

**Rule (from exact examples and probes):** the loop latch ends in an
inverted test and a `jmp` back when VC6 places a block that leaves the loop
early directly after the latch. That block is a `return`, or a `goto`
target. Otherwise the latch is a plain `jcc top`, with the early-exit block
moved to the end of the function.

The exact examples are all a guarded do-while, or a `for`, whose early exit
comes in the middle or at the end of the body:
- Sound::SetPan, SetVolume and SetFrequency `0x004bcca0`, `0x004bcbe0`, `0x004bcb30`: `if (n > 0) do { if (... < 0) goto failed; i++; } while (i < n);`.
- BackgroundImage::ClearRegionDepth `0x004043c0`: `return 0` as the last test of a guarded do-while.
- UnknownDisplay::SetGDISurfaceVisible `0x004cb5b0`: three `return 0` tests after the body's first statement.
- UnknownVcrFile Read and Write `0x00525070`, `0x005251f0`, and CacheTexture::ReserveRegions `0x0050f9b0`: a `for` whose `if` block holds the rest of the function.

**Probe (confirmed):** in a guarded do-while whose *first* statement is a
conditional exit (`if (d->Blt(...)) return 0;`, `if (n == k) return 0;`), VC6
places the exit block after the loop. It also copies the loop head to the
back edge: `jle exit ; <head copy> ; je body`. This holds for one-argument
and five-argument calls, for `goto failed`, and for an if/else body. The
same loop written as `while` or `for` gives `jg top`, with the exit block at
the end.

CacheTexture::CopyMipLevels `0x005102d0` (render) has the exit first, and
retail does not copy its head. Its guarded do-while forms place the exit as
retail does, but they copy the head (294 of 586). The `while` form keeps 462
of 558. The construct that places the exit after the loop without copying
the head is not known yet.

## 3. A load hoisted above the first branch

**Rule (exact example):** VC6 hoists a load above a test when both arms use
the value. In UnknownDisplay::RetryFlip `0x004ca4a0`, `primarySurface` is
loaded between `test eax, eax` and `je` because both the `if` and the
`else` call through it.

PCVideoCard::CreateModeSurfaces `0x004ca520` (render; the only near miss
with three or more such loads) hoists five values above the `fullScreen`
test. Retail's windowed arm calls with `buffers - 1` only. A wrong-code
probe that uses bitDepth and the budget in the windowed call reproduces
retail's prologue. Forms that do not reproduce it:
- dead uses and empty inline functions (removed before the decision);
- a by-value copy of the mode struct (2 of 160).

## 4. `sub esp` for a by-value argument before an earlier copy

Exact examples:
- PhysicsBody::SetInertia `0x004cbe40`: the `sub esp, 0x40` for MatrixInverse's by-value matrix is placed above the `rep movsd` of the preceding `inertia = I`.
- TreeBoxOverlap `0x00428c20`: the `sub esp, 0x40` for MatrixMultiply moves above a copy into a static matrix and the inline InvertRigid.

In both, the copy it passes writes a member or a global. Camera::UpdateMatrices
`0x0042ee30` (camera) needs the same move over a copy whose destination is
a stack argument slot (`mov edi, esp`). No exact function in the corpus has
that case, so the corpus does not decide it.

Other near misses with the retail shape:
- TerrainShadow `0x005097d0`;
- CollisionObject `0x00437ef0`;
- SoultreeMatrix `0x004fc050`;
- ProjectedShadow `0x004db0d0`.

Motnctrl `0x004a7dc0` and SoultreeMatrix `0x004fc050` also contain the
reverse order in retail.

## 5. By-value pushes: a load before `sub esp, 0xc`

The by-value copy of a 12-byte vector argument is `sub esp, 0xc ; mov r, esp`
followed by three word copies. The first source load comes either before the
`sub` or after it. The table classifies the 138 sites in exact functions by
the instruction just before the first of the two (the load or the `sub`):

| previous instruction | load first | `sub` first |
|---|---|---|
| `push` (a scalar argument to the right) | 38 | 9 |
| `lea` (the source address formed just before) | 0 | 18 |
| `add esp` (the previous call's cleanup) | 0 | 10 |
| `mov` | 20 | 25 |
| a branch or call | 6 | 2 |
| other (x87, `cmp`, `xor`) | 3 | 7 |

This is a strong inference, not a probe result. The load moves above the
`sub` when a scalar argument push comes just before it. The `sub` comes
first when the source address is an `lea` of a member, or when the copy
follows a call's stack cleanup. Load-first examples:
- AuralScape::SetListener `0x004030a0`;
- ArcadeObject::Tick `0x00401800` (`push edx ; mov edx, [eax] ; sub esp, 0xc`);
- Tire::UpdateAttachment `0x00515660`.

`sub`-first examples:
- BikeRace::UpdateTrackSeparation `0x0041d1e0` (`lea eax, [esi + 0xf8] ; sub esp, 0xc`);
- LightEmitter::SetPosition `0x0049e150`.

## Reproduction

The scan is an untracked script (`work/s14/corpus/scan.py` in the render
worktree). It disassembles the retail bytes of every row in the gate
outputs (`tools/run_calibration.py`, `tools/run_physics_samples.py
--json-out`) and the near-miss index with capstone, and matches the shapes
above instruction by instruction. To build a probe, compile a small `.cpp`
with `tools/compile.py --compiler vc6 --profile vc6_o2_mt` and disassemble
the object's function symbols.
