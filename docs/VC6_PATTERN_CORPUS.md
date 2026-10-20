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
| x87 `fld st(0) ; fop m` (section 6) | 112 | 63 |
| x87 `fld m ; fop st(1)` (section 6) | 68 | 52 |

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
- NetworkInterface::ConnectUsingLobby `0x004ab960`: giving the size query its own failure test, so its error path no longer runs through the second query's test, gives retail's `cmp esi, ebx` against the entry zero.

Near misses that differ in this shape:
- Griddraw `0x00480ad0` (render).
- BikeRace `0x0041f1d0` (race): retail keeps 0 in edx from the first instruction.
- GUIManager `0x00486170` (ui): retail has no `xor ebx, ebx`.
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
of 558. Testing both failures once after the loops (the `Blt` result in
`result`, `break`, then `if (result) return 0;`) places the exit after the
loop without copying the head (5 differing instructions). It also keeps
`result` in a frame slot and tests it at the loop's normal exit, which
retail does not.

Retail's flow is: the first loop's `GetAttachedSurface` failure, the
`result` test after that loop (in eax) and the `Blt` failure all jump to one
`return 0` block. The second loop's `GetAttachedSurface` failures go to the
upload. The forms with that flow drop the `result` store but put the
`return 0` block at the end of the function with a plain `jg top` latch
(16). They were tried with `if (Blt(...)) return 0;` (with `while`,
`for (;;)` and a named result), with `if (result) return 0;` right after the
first loop (17, two epilogues), and with a single `failed:` label (16).

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

## 6. x87 operand form: `fld st(0) ; fop m` or `fld m ; fop st(1)`

A value on the x87 stack that is used more than once is combined with a
memory operand in one of two ways. Both take two instructions and leave the
value below the result:
- dup form: `fld st(0) ; fmul m` (the copy is duplicated, memory is the operand);
- load form: `fld m ; fmul st(1)` (memory is loaded, the stack value is the operand).

In the exact functions of the 2026-10-20 gates (calibration 2969, physics
1072, 4019 functions), with the operand kind of the memory word:

| form | functions | sites | commutative op: frame / indirect / global |
|---|---|---|---|
| dup `fld st(0) ; fop m` | 112 | 292 | 77 / 189 / 16 |
| load `fld m ; fop st(1)` | 68 | 165 | 112 / 2 / 2 |
| `fld st(N)`, N >= 1 | 103 | 304 | |
| `fxch` | 47 | 122 | |

"Frame" is an `esp`/`ebp` slot: a local, or a field of a local struct or of
a by-value parameter. The load form is almost always a frame word (112 of
116 commutative sites). The four others are a constant, a global and two
products in ManagedTextureGroup::DrawSelection.

**Rules (VC6 SP3 `vc6_o2_mt` probes; `work/s14/x87/p1.cpp` to `pd.cpp`):**
1. Written operand order never matters for a stack value against memory:
   `t * a->x` and `a->x * t`, and `v->x *= t`, `v->x = v->x * t` and
   `v->x = t * v->x`, compile the same.
2. A named float local against an indirect word (`p->x`, `this->m`) or a
   global takes the dup form for each use. The last use consumes the value
   (`fmul m` with no duplicate).
3. A named float local against a frame word takes the load form for each
   use. The local is then popped with `fstp st(0)`, or the last product
   keeps it when it feeds a further sum. This includes an inline helper's
   by-value float parameter (`Scale(d, a * b)`) and a call result
   (`float s = Ext(a)`).
4. A repeated expression with no named local (`d.x * (a * b)` three times)
   takes the dup form even against frame words.
5. An assignment expression as the operand (`v.x *= (s = e)`) takes the
   dup form for that use only. Later uses of `s` follow rule 3:
   `fld st(0) ; fmul x`, then `fld y ; fmul st(1)`, `fld z ; fmul st(1)`,
   `fstp st(0)`.
6. A frame word that the function stores to directly at an earlier point
   in the source becomes an ordinary memory operand: the dup form, against
   a named local. The store does not have to reach the use. A store in the
   other arm of an `if`, or on an early-return path, also counts.
   - Stores after the use do not count. Stores through a pointer
     (`V* p = &d; p->x = ...`, or an inline helper taking `&d`) do not count.
   - The store made by the use's own statement (`d.x *= s`) does not count.
   - In the probes this changes only the stored component: storing `d.y`
     in the other arm gives the dup form for `y` and keeps the load form
     for `x` and `z`.
7. A non-commutative op with the stack value on the right (`v.x -= s`)
   always takes `fld m ; fsub st(1)` and ends with `fstp st(0)`. With the
   stack value on the left (`s - v.x`), it takes the dup form `fsub m`.

**Two memory operands (`fld a ; fmul b` or `fld b ; fmul a`):**
- A frame word against an indirect word: the frame word is loaded first, in
  either written order.
- Two indirect words, or a global against an indirect word: the order is
  fixed whatever the written order. Even the declaration order of pointer
  locals does not move it. A larger field offset on the same base loads
  first. Otherwise the probes do not determine the order: two parameter
  pointers load `p` first, but two member pointers load `b` first.
- Two frame words that are never stored to (fields of one by-value struct,
  or two by-value parameters): written order decides. The right operand is
  loaded and the left one is the memory operand.
- Two frame words where one is stored to earlier (as in rule 6): the
  word that is never stored to is loaded first, in either written order.
- Inside longer sums of products the written order of each product had no
  effect in the cases tried (SoultreeBounds `0x004fd340`: 14 diffs either way).

The probes are direct observations of VC6 output; applying them to other
shapes is strong inference. The 77 frame dup sites in exact functions
include rule-6 cases such as ObjectPicker::PickAt `0x004b066f`, where
`direction` is stored component by component before it is scaled.
SafeNormalize `0x0040d1b0` (dup on a by-value parameter after a call) is not
explained by these rules.

**Near misses where a form is the remaining difference** (ranked hunks of
the 2026-10-20 ranking, `work/s14/x87/nearmiss.py`):

| form | lane | row | hunks of that form / all |
|---|---|---|---|
| ours dup, retail load | shell | Krusty3DObjects `0x0048b100` | 4 / 102 |
| | shell | CarProcedural `0x0042fd80` | 1 / 138 |
| ours load, retail dup | body | Vehicle slot 74 `0x005281b0` | 1 / 3 |
| two memory operands swapped, frame and frame | body | SoultreeTransform `0x004fd710` | 1 / 1 |
| | body | SoultreeBounds `0x004fd340` | 4 / 6 |
| | body | GraphicsTest `0x0047bd10` | 9 / 16 |
| | body | SoultreeMatrix `0x004fb8c0` | 2 / 37 |
| | rider | Tire `0x00515880` | 2 / 4 |
| | rider | BikeAI `0x0040e510` | 2 / 23 |
| | rider | Bike `0x0040a520` | 6 / 67 |
| two memory operands swapped, frame and indirect | body | SoulTreePhysics `0x00501600` | 1 / 13 |
| two memory operands swapped, indirect | render | MatrixUtil `0x004a1a50` | 1 / 1 |
| | render | Griddraw `0x0047e600` | 2 / 2 |
| | render | MorphBastard `0x004a33b0` | 2 / 61 |
| | body | Vehicle `0x005286a0` | 2 / 7 |
| | body | SoulTreePhysics `0x00501e90` | 1 / 9 |
| | body | Vehicle `0x005265d0` | 2 / 16 |
| | body | SoulTreePhysics `0x00500220` | 2 / 31 |
| | body | CollisionObject `0x00435830` | 3 / 58 |
| | rider | Motnctrl `0x004a7fd0` | 3 / 3 |
| | rider | Tire `0x00514550` | 1 / 18 |
| | rider | Bike `0x0040c000` | 1 / 40 |
| | rider | ContactImpulse `0x005004a0` | 2 / 71 |
| `fxch` only | body | BoundingBoxTreeQuery `0x00425900` | 6 / 161 |
| | rider | Bike `0x0040c000` | 1 / 40 |
| `fld st(N)` against a reload | body | Vehicle `0x00527a20` | 1 / 17 |

Rows whose remaining hunks are all x87 (operand form or scheduling):
- body: `0x004fd710`, `0x005281b0`, `0x0040b410`, `0x004fd340`,
  `0x00436720`, `0x00436e50`, `0x00446c30`;
- render: `0x00456890`, `0x004a1a50`, `0x0047e600`, `0x00443aa0` (none
  of them operand form that a rule above flips; see below);
- rider: `0x00504d30`, `0x00515880`, `0x004a7fd0`;
- shell: `0x00516ef0`.

Across all ranked rows, 295 hunks are x87 instructions moved past other code
(such as `fcom` placed after `fst ; fld st(0)` in Vehicle slot 74), 1427
are other x87 differences and 7418 are not x87.

**Two indirect words (probe, provisional).** In `r->x = a * b` with `a`
and `b` read through two different pointers, the written order is inert.
- A word that is also read in another statement, before or after,
  becomes the memory operand.
- When neither is read elsewhere, the larger displacement loads first.
- When both are, both orders occur in the probes; no rule was found.

The model does not explain every row. In Griddraw `0x0047e600` the
cell size is read in all six centre/extent statements but still loads first.
Deleting centre.x (or its minimum term) makes it the memory operand there,
as in retail. MatrixUtil `0x004a1a50` also departs from the model: retail
loads `p->y` (displacement 0) before the matrix in every row. Probes:
`work/s14/x87/ip/p1.cpp`, `p2.cpp` (untracked).

Rows checked with `tools/x87_slips.py` (s14). The rows below are not
operand form: no x87 rule applies, so they need a different lever.
- Camera `0x0042ee30`: `sub esp` against the copy placement (section 4).
- FollowCamera `0x00465c20`: `lea` differences.
- EcoSystem `0x00456890`: an `fmul` scheduled after integer moves.
- EcoSystem `0x00457ed0`: the cylinder height load is scheduled earlier.
- TextService `0x0050ade0`, `0x0050ba00`: integer register choice and
  store scheduling.
- D3DIMSoulTree `0x00443aa0`: sum term order, row 1 (see its sample).

D3DIMSoulTree `0x00440f30` has two swaps of stored frame words, which are
canonical. Its other 1389 differences are not x87.

Leads from the rules:
- **Vehicle slot 74.** The `u = t * s` after the `s` clamp has retail's
  rule-5 shape. The form below takes the row from 4 to 2 differing
  instructions; the two left are the `fcom` placement:

  ```
  u.x = t.x * (s = (s < 1.0f) ? s : 1.0f);
  u.y = t.y * s;
  u.z = t.z * s;
  ```

  `t * (s < 1 ? s : 1)` without the assignment, `VehScale(t, s)` and a
  component-wise form all stay at 4.
- **VehicleCamera slot 35 `0x0052c510` (now exact).** The rules located
  the source. Retail's `reach` product takes `direction.x` in the load
  form, so nothing stored it directly earlier (rule 6). Its short range
  arm takes x in the dup form and y and z in the load form, which is rule
  5's shape. Scaling through one `Vector3* scaled = &direction` in both arms,
  with `scaled->x *= (range *= 0.25f)` in the short range, is strict exact.
  Other spellings of the same two facts are also exact: a pointer-taking
  helper in the first arm, or `s = range * 0.25f` as the assignment.
- **SoultreeTransform `0x004fd710`.** The two memory operands are frame
  words. Rule 6 predicts the earlier finding that a store to `m._11` gives
  209/209: retail's `m._11` has a direct store, such as a transpose or
  copy that writes every element.

## Reproduction

The scan is an untracked script (`work/s14/corpus/scan.py` in the render
worktree). It disassembles the retail bytes of every row in the gate
outputs (`tools/run_calibration.py`, `tools/run_physics_samples.py
--json-out`) and the near-miss index with capstone, and matches the shapes
above instruction by instruction. To build a probe, compile a small `.cpp`
with `tools/compile.py --compiler vc6 --profile vc6_o2_mt` and disassemble
the object's function symbols. The x87 study scripts are
`work/s14/x87/scan.py` (form counts), `lines.py` (maps exact-function sites
to source lines through the `/Z7` COFF line numbers) and `nearmiss.py`
(classifies the ranked near-miss hunks); they are untracked work scripts.

`tools/x87_slips.py` does the per-row step: it aligns a ranked row's
candidate against retail, lists each dup/load or memory-order slip with its
source line, the operand kinds, the CodeView local behind each frame word
and the candidate's earlier direct stores to it, and names the rule above
that predicts retail's form (`python tools/x87_slips.py 0x52c510`, or
`--lane render` for a lane's rows).
