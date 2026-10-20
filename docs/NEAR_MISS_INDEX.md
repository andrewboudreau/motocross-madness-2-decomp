# Near-miss index
Every near miss kept in the repository (`samples/**/*NearMisses*.cpp`, the
`"expect": "partial"` entries of every `targets.json`, and unregistered
functions of `src/` files whose VA is bound), compiled with the calibrated
VC6 profile and compared with retail through `tools/frame_layout.py`
(docs/VC6_FRAME_LAYOUT.md). Classes:

- **a**: the instruction stream is retail's once frame displacements are
  masked; only the assignment or sharing of stack slots differs.
- **b**: slot differences plus other differences (register choice,
  scheduling, operand order, block layout, missing or extra code).
- **c**: no slot differences; the code itself differs.

Score is strict (relocations resolved) matching bytes over compared bytes;
"masked" means the sample has no complete bindings, so relocation bytes are
ignored. "what differs" is the first difference the scan found, not a full
diagnosis; the sample headers hold the forms already tried. Regenerate with
the scan described at the end.

Counts: a 2, b 130, c 101, total 233. The three class (a) functions of the
first scan (Cube `0x0043d230`, GUIManager::SetUp `0x004853b0`,
VisibilityClipper::SphereInFrustum `0x0052fbb0`) are exact: their slots were
homes in dead argument slots, which VC6 hands out in the order of the
arguments' first use in the source (docs/VC6_FRAME_LAYOUT.md, fact 9).
Game `0x00467b70` (class a, 761/775) is exact too: retail's compare reads
`type` (`type == REG_DWORD`) where the candidate read `size`, so the
"use-count change" `tools/frame_layout.py` proposed was a different variable
in the same instruction, not an extra reference. Check what retail's
instruction addresses before adding uses.
FileStream open `0x00460f50` (class c, 284/957) is exact too: each branch
declares its own message buffer. With one function-wide buffer VC6
cross-jumps error blocks of one branch into identical tails of the other;
retail keeps them apart. When retail repeats a tail that VC6 merges across
branches, try locals declared in the branches.
TrackOverlay `0x005194b0` (class b, 246/844 masked) is exact too: retail
computes the LOGFONT face-name address in both branches of the face test,
and that second use raises the LOGFONT's uses per byte above the two
rectangles', so it sits below them as in retail. A slot order that the
scan calls "differs" can come from a reference VC6 duplicates into branches.
MoveBox `0x00428db0` (class c, 773/789) is exact too: passing the point to the
inline transform through a local pointer (`const Vec3* point = center`) makes its
components newer leaves than the matrix elements, so the z row loads them first as
retail does; grouping the x row's first two products does the same for that row.

| TU | VA | score | class | what differs |
|---|---|---|---|---|
| audio/PCAudio | `0x004bc6b0` | 238/629 (inside PCAudio.cpp) | c | VC6 cross-jumps the two `Play()` failure tails (streamed `return 0`, the `& 5` branch's `goto failed`) where retail keeps them apart; the sample copy also gains an EH frame |
| audio/PCAudio | `0x004bdc00` | 91/483 | b | retail extent 483; `while (1)` gives retail's top-tested queue loop (155 insns); left: retail ebp=audio, ebx=&cs then loaded, edi=&sound cs, ready/bytes spilled (frame 0xc vs 8) |
| audio/PCAudio | `0x004be910` | 108/158 | c | 6 code diffs: retail adds `mov [esp + N], edx`; retail adds `mov [esp + N], edx` |
| camera/Camera | `0x0042ee30` | 487/492 | c | `sub esp, 0x40` emitted after the `rep movsd` that copies the projection argument (retail before it) in the first MatrixMult; argument copies, viewport temporaries and declaration forms do not move it |
| camera/FollowCamera | `0x00462ee0` | 511/564 | c | constructor: the post-call copies match; VC6 fills the +0x29c vector copy's load-delay slot with the +0x274/+0x2dc zero stores where retail uses +0x2e4/+0x276 (28 positions of the pair and 29 of the copy tried) |
| camera/FollowCamera | `0x00463a30` | 903/4042 | b | slot 45 (keyboard/joystick controls), 138 code diffs; aligned instruction ratio 0.937 (samples/camera/FollowCameraNearMisses.cpp) |
| camera/FollowCamera | `0x004650e0` | 509/937 | b | retail keeps the two cross products in memory temporaries (frame 0x18c vs 0x180); branch targets of the two `je` tests |
| camera/FollowCamera | `0x00465c20` | 3659/3672 | b | 4 code diffs: retail adds `lea ecx, [esp + N]`; retail lacks `lea ecx, [esp + N]`; slots differ |
| camera/KrustyBikeCamera | `0x00497e20` | 337/376 | c | 10 code diffs: `mov ecx, [A]` vs `mov edx, [A]`; retail lacks `lea eax, [esp + N] ; mov edx, [ecx + 0x~` |
| camera/VehicleCamera | `0x0052c510` | 817/1261 | c | slot 35: frame and slots match; `(x*x) + y*y + z*z` gives retail's x*x-first squared length; retail duplicates the x scaling into both range branches (`fld x; fmul st(1)` / `fld st(0); fmul x`) where VC6 shares one block ; writing the scaling out in the body (slot 41's lever) gives 798..824 |
| control/ControlInterface | `0x004bf4f0` | 76/104 (inside ControlInterface.cpp) | c | tails merged as in retail; retail places the mouse pointer load after the shared block's `ret` and jumps back, VC6 places it after the keyboard load |
| control/ControlInterface | `0x004bf6a0` | 438/726 (masked) | c | 58 code diffs: `jne @158` vs `jne @165 ; lea eax, [esp + N]`; `jmp @169 ; lea ecx, [esp + N]` vs `push eax ; jmp @178` |
| ecosystem/EcoSystem | `0x00456890` | 356/369 | c | 2 code diffs: retail lacks `fmul [eax + 0x5a8]`; retail adds `fmul [eax + 0x5a8]`; redundant parentheses on the differences, products and detail-band casts (225 combinations) do not move it |
| ecosystem/EcoSystem | `0x004570a0` | 103/386 | b | retail keeps the definition-table entry in `edi` across the position conversions (`mov edi, [eax + ecx*4 + 0x58]`); register choice in the shape store blocks |
| ecosystem/EcoSystem | `0x00457480` | 561/2629 (in the unit) | c | 3 code diffs: `mov edi, 1 ; mov [esi + 0x30], edi` vs `mov dword ptr [esi + 0x30], 1`; `mov [esp + N], edi` vs `mov dword ptr [esp + N], 1` (the constant 1 of `method = 1` / `i = 1` kept in `edi`) |
| ecosystem/EcoSystem | `0x00457ed0` | 1105/1153 | b | retail reads the cylinder height right after the fild, before the `j` spill store; paren sweeps (216), statement orders and split cos/sin locals do not move it |
| ecosystem/EcoSystem | `0x00458da0` | 457/461 | c | four SIB operands `[esi + eax]` for `[eax + esi]`; the `name[length] = 0` byte store flips the later array-base/induction order (without it the loop has retail's order; each further variable-index byte store toggles it); no code-neutral form found (~60 forms incl. paren sweeps, zero-store spellings, loop shapes, 0..40 typedef shift) |
| ecosystem/EcoSystem | `0x00459ce0` | 323/3300 | b | 112 code diffs: retail adds `xor edi, edi`; retail adds `mov [esp + N], edi`; slot order differs |
| ecosystem/EcoSystem | `0x0045b060` | 461/3967 (in the unit) | c | frame 0xe0 vs 0xf0 and the slot sharing; retail leans the side normals through memory temporaries and pops `edi` / `ebp` / `ebx` before the overlay rows |
| game/DeviceSetup | `0x00448560` | 148/1006 | b | 35 code diffs: retail adds `push edi`; `push edi ; xor ebp, ebp` vs `xor edi, edi`; frame 0x11c vs 0x118 |
| game/EventManager | `0x0045d480` | 4139/4247 | b | frame size 0x460 and extent match; the squared lengths square each component as retail with `xx`/`yy` locals, but retail spills the z square to [esp+0x1c] (so the +0x1c/+0x20 homes swap); the Winner branch registers run one step behind retail |
| game/EventManager | `0x0045e710` | 454/533 | c | 10 code diffs: `mov edx, [ecx + 0xc4c] ; mov [eax + 0x5~` vs `mov ecx, [ecx + 0xc4c] ; mov [eax + 0x5~`; `mov eax, [eax + 0x56c]` vs `mov edx, [eax + 0x56c]` |
| game/PCGame | `0x004c0d10` | 1776/1790 | c | retail stores the surface description's size right after the memset, VC6 after pushing the 0x005119c0 arguments ; helpers, a derived-struct constructor, pointer/format locals and sizeof spellings do not move it (retail stores it after the pushes in the matched 0x004c5950) |
| game/QuarryStuntEvent | `0x004e14a0` | 166/1760 | b | 9 code diffs: `mov edx, [A] ; test edx, edx` vs `mov ecx, [A] ; test ecx, ecx`; `mov eax, [ecx + 0x26c0] ; lea edx, [eax~` vs `mov eax, [eax + 0x38] ; mov ecx, [eax +~`; slots differ |
| game/QuarryStuntEventLoader | `0x004de590` | 397/7912 | b | camera argument fixed to 1.048f (retail 0x3f8624dd); 2074/2208 normalised insns align (was 2063); scalar stack slots and decision-tree registers left |
| game/UiInfo | `0x00522440` | 383/427 | c | 2 diffs: retail schedules the +0x2828 store after `mov edx, 2` and places the second 0x1ec-byte rep movsd after the +0x196c store; ~21,000 store-order/copy-form/zeroing variants swept |
| game/UiInfo | `0x005238f0` | 190/357 | c | block layout and the single `return 0` tail are retail's; retail keeps fopen's result in eax until the found block (`mov edx, eax` there), VC6 here gives `file` edx after each fopen |
| gameui/MediaControl | `0x004a2560` | 53/656 | b | 29 code diffs: retail adds `push ebp`; slots differ |
| inputdevice/PCJoystickDevice | `0x004c3100` | 102/1449 (masked) | b | 90 code diffs: retail lacks `push ebp`; retail lacks `xor ebp, ebp`; slots differ |
| inputdevice/PCJoystickDevice | `0x004c3790` | 345/417 (masked) | b | 7 code diffs: `mov eax, [esi + 0x260] ; mov ecx, [A] ;~` vs `mov ecx, [esi + 0x260] ; mov edx, [A] ;~`; `mov ecx, [ecx + 0x14]` vs `mov ecx, [edx + 0x14]`; slots differ |
| io/FileStream | `0x00460db0` | 27/186 | c | 10 code diffs: retail keeps the first fseek-failure block inline (samples/io/FileStream.cpp; the extent is 186 bytes, docs/FILESTREAM.md) |
| io/FileStream | `0x00461340` | 286/702 | c | +0x0c read once into `position` and cleared when not below `offset` (201 with the ternary); left: origin/offset in ebp/ebx swapped, the zero and next pointer in other registers, the SEEK_CUR test repeated where retail threads it |
| io/FileStream | `0x00461640` | 389/671 | c | a separate `out` pointer gives retail's homed buffer copy; left: file pointer in edx (retail eax/ebx), a third 45c7b0 call retail cross-jumps, and the two `(done + read) / size` returns merged |
| io/FileStream | `0x004618e0` | 136/152 | c | 7 code diffs: retail loads buffer/size/count into registers before the pushes |
| io/FileStream | `0x00461b90` | 257/282 | c | 3 code diffs: retail stores the 0xfa seed inside the inlined strcat and sets up the name pointer right after strlen |
| krusty2/broadphase/Quadtree | `0x004dcac0` | 620/627 | a | x and z spill slots swapped (0x2c/0x30) |
| krusty2/broadphase/Quadtree | `0x004dcf20` | 182/331 | b | i and cell cached in edi/edx where retail reads their slots; z in esi; slot order differs |
| krusty2/broadphase/Terrain | `0x00506e90` | 920/1654 | b | 44 code diffs: `je @308` vs `je @282 ; fld [esp + N] ; push ecx`; retail adds `mov ecx, [esp + N]`; slots differ |
| krusty2/broadphase/Terrain | `0x00507c10` | 71/2731 (masked) | b | 128 code diffs: retail adds `push ebp ; mov ebp, esp`; retail lacks `push ebp`; frame 0x114 vs 0x108, slot order differs |
| krusty2/broadphase/Terrain | `0x005059d0` | 1593/2116 | c | TRN loader: same frame (0x440, every local in its retail slot), calls and EH states; register choice (retail keeps -1 in edi/ebx, the candidate keeps 3 and 0 in ebx), colour-key scheduling and the palette loop's pre-offset pointer stride differ |
| krusty2/collision/CollisionObject | `0x00432260` | 441/458 | b | 2 code diffs: retail adds `mov [esp + N], eax`; retail lacks `mov [esp + N], eax`; slots differ |
| krusty2/collision/CollisionObject | `0x00436100` | 727/804 | c | two diagonal zero stores scheduled after the first products; pop edi/esi placed before the third extent row |
| krusty2/collision/CollisionObject | `0x00436720` | 936/962 | c | same size, frame and registers; the scratch loop's rows are grouped (x + z first) to match; the second forward rotation's x row still sums x, z, y where retail sums z, y, x (no term order or grouping of that row, nor an xf/direct/local-pointer matrix source, fixes it without breaking the loop) |
| krusty2/collision/CollisionObject | `0x00436af0` | 809/852 | c | same size, frame and registers; the zero-vector and contactCount stores are scheduled into the neighbouring rotation/call setup in another order |
| krusty2/collision/CollisionObject | `0x00436e50` | 2109/2205 | b | HullVsModel: same instructions as retail; the x87 tail of the inlined product (rel._43/_44) is scheduled after the first SweptBoxOverlap pushes |
| krusty2/collision/CollisionObject | `0x004376f0` | 695/707 | c | HullVsModelSwept: mean as ModelVsHull; the enabled test nests outside the element-pointer local and call, which fixes the loop's loads and the mean copy registers. Left: the schedule of the first by-value box copy (`mov edx, esp` placement); locals for the box pointer or copies of its vectors do not move it |
| krusty2/collision/CollisionObject | `0x00437c20` | 712/716 | c | ModelVsHull: frame and slots match; mean written per component with `s` assigned inside the x product; the enabled test nests outside the element-pointer local and call (fixes the second push schedule and the mean copy registers). Left: one push schedule before 0x424730 (`push ecx` before `add eax, 4`); a box-pointer local or a half-extents pointer local does not move it |
| krusty2/collision/CollisionObject | `0x00437ef0` | 857/906 | c | ModelVsHullSwept: frame 0x7c matches; mean and nested enabled test as ModelVsHull. Left: scheduling of the two box copies and one push; assignment after declaration, component-wise copies and the reversed declaration order do not move it |
| krusty2/collision/CollisionObject | `0x00438280` | 705/710 | c | ModelVsModel: as ModelVsHull, with the same per-component mean; an element-pointer local before the call fixes the mean copy registers. Left: one by-value copy schedule (`sub esp, 0xc` placement) |
| krusty2/collision/CollisionObject | `0x00438550` | 734/783 | c | ModelVsModelSwept keeps the last contact (no averaging); an element-pointer local before the call fixes the loop registers. Left: scheduling of the two box copies (as 0x00437ef0); assignment after declaration, component-wise copies and the reversed declaration order do not move it |
| krusty2/collision/CollisionObject | `0x00434eb0` | 390/1885 | b | UpdateModelBounds: natural `center - extent` Min argument; this source exhausts the inline budget one site before retail (operator- called, frame 0x128 vs 0x134). Two dead `if`s (calibration) reproduce all but the loop entry test, see VC6_INLINE_BUDGET |
| krusty2/collision/CollisionObject | `0x00435830` | 163/1688 | b | SetTransform: per-element term order of the hull case's inline 4x4 product (retail 3,2,1,4 / 1,3,4,2 / ...; arena ordering, see VC6_OPERAND_ORDER.md 3.2); frame 0x30 both |
| krusty2/collision/CollisionObject | `0x00439820` | 1309/1493 | b | SegmentTouchesObject: same instructions as retail; some temporaries in other frame slots, one `add esp` scheduled late |
| krusty2/soultree/SoulTreePhysics | `0x00500220` | 286/639 | b | 24 code diffs: `je @201 ; fld [edi + 8]` vs `je @197`; `fmul [esi + 4]` vs `fld [esi + 4] ; fmul [edi + 8]`; slots differ |
| krusty2/soultree/SoulTreePhysics | `0x00501600` | 651/925 | b | 18 code diffs: `jmp @239 ; fld [esp + N]` vs `jmp @242`; slots differ |
| krusty2/soultree/SoulTreePhysics | `0x00501e90` | 169/483 | b | 487 B vs 483: the cross product passed straight to WorldToLocalDirection as `SoultreeCrossCtor` gives retail's copy/push schedule; left: retail stores normalForce member-wise to `[esi + 0x68]` in both branches (a by-reference member-wise helper does that but constant-folds the zero temporary), the x component's products load leverArm first and `*a2 +=` loads `*a2` first |
| krusty2/soultree/SoulTreePhysics | `0x00502f60` | 1804/1930 | c | 0 code diffs: stack-slot packing of the integration temporary and merge outputs; 9 bytes longer |
| krusty2/vehicle/Bike | `0x00405190` | 211/2874 | b | ratio 0.87: frame 0xa0 vs 0xac (0x3ea factor shares the l10 slot, f23c copied twice); other/ctx in ebp/edi swapped |
| krusty2/vehicle/Bike | `0x004060c0` | 1677/1678 | c | second contact loop loads `[eax + ecx]` where retail has `[ecx + eax]` (Vehicle slot 7's identical loop gets retail's order). Removing both `BikeWheelDistance` calls of the `mode > 1` branch flips it; one call, helper reshapes, named lenSq/sum/reference locals, dead int locals (0-11), declaration order, `i[arr]`, pointer arithmetic and a `pts` alias do not |
| krusty2/vehicle/Bike | `0x00406ae0` | 104/2396 | b | retail has three bare `fistp` (inline `__asm`, docs/REBUILD_GAPS.md), hence its ebp frame; 118 code diffs: `push ecx` vs `push ebp ; mov ebp, esp ; sub esp, 8`; `jne @16 ; mov [esp + N], 0 ; jmp @20` vs `jne @18 ; mov [ebp - N], 0 ; jmp @22`; frame 0x0 vs 0x8 |
| krusty2/vehicle/Bike | `0x004079c0` | 2080/6718 | b | spill homes of the loop values, the matched wheel kind kept in esi by retail, store scheduling of the local string tables and Vec3 constants (see PHYSICS_VALIDATION.md); 40/32-degree constants now 40*0.01745329f / 32*0.01745329f |
| krusty2/vehicle/Bike | `0x00407440` | 188/485 | c | clamps are BikeMin(BikeMaxF(t,.25),.75) and BikeMaxF(BikeMin(a,32.2),-32.2), constants bound; left: retail's dead `cnt+=2` store (to the shared [esp+0x10] slot, tail-merged: the `idx > 2, idx != 10` arm also jumps into `add ecx, 2`), idx/next/step/cnt register roles swapped (declaration order of the four locals and `cnt += 2` placements in both arms are insensitive: VC6 drops the dead update), 0.0310559 read from the 0x550790 copy (binding gives 0x550ad0) |
| krusty2/vehicle/Bike | `0x0040a520` | 439/2272 | b | 96 code diffs: retail lacks `test al, al`; retail adds `test al, al`; slots differ |
| krusty2/vehicle/Bike | `0x0040c000` | 162/1015 | b | retail has a bare `fistp` (inline `__asm`, docs/REBUILD_GAPS.md), hence its ebp frame; 60 code diffs: retail adds `push ebp ; mov ebp, esp`; retail lacks `fld [A]`; slot order differs |
| krusty2/vehicle/Vehicle | `0x005257a0` | 693/745 | b | best of all 720 orders of the terrainScale..throttleAxis stores x 6 orders of the weightForce/leanCos/0x4b4 stores; retail still stores 0x4b0/0x4b4 before the leanAxis/throttleAxis pair |
| krusty2/vehicle/Vehicle | `0x005265d0` | 381/555 | b | same size, 0.82 ratio: two products per cross load a before b; the crashState ?: argument is hoisted above the pushes |
| krusty2/vehicle/Vehicle | `0x005268d0` | 223/1578 | b | ratio 0.59: retail adds `xor ebx, ebx`; hasBody/otherVel registers swapped (ebp/ebx); slot order differs |
| krusty2/vehicle/Vehicle | `0x00526fa0` | 1159/1161 | c | single-wheel branch: `appliedShare = Vec3(0.0f, 0.0f, 0.0f)` gives retail's instructions and registers, but the temporary's dead zero stores go x, y, z where retail stores z, y, x. Component stores into a `share` local (every order, chained, partial aliases) store z first but swap the edx/edi copies |
| krusty2/vehicle/Vehicle | `0x005276d0` | 496/523 | b | 3 code diffs: `mov [edx + 4], esi` scheduled before `fstp [esp + 0x14]` (midpoint branch); return-pointer `lea eax, [esp + 0x20] ; push eax` earlier in the lead-wheel branch |
| krusty2/vehicle/Vehicle | `0x00527a20` | 349/870 | b | 25 code diffs: `jne @99` vs `jne @98 ; mov [esp + N], 0 ; mov edx, [~`; `lea edx, [esi + 0x120]` vs `mov [esi + 0x12c], 0`; slots differ |
| krusty2/vehicle/Vehicle | `0x00527e30` | 770/831 | c | frame and slots match; the prologue's kVec3Zero copies use edx and load `out` into esi first (retail: esi as the copy temp, then esi = `out`, edi = its copy) |
| krusty2/vehicle/Vehicle | `0x005281b0` | 502/539 | b | 4 code diffs: retail stores field_0x4e8 before the abs `fcom`; `fld st(0) ; fmul [t.x]` vs `fld [t.x] ; fmul st(1)`; scratchVector .y store scheduled later; len/sign dead-parameter slots swapped |
| krusty2/vehicle/Vehicle | `0x005286a0` | 291/784 | b | slot 56: frame 0x3c matches; retail shares the ground-normal output with the projection and n with the side vector; register roles |
| krusty2/vehicle/Vehicle | `0x0052a940` | 302/3254 | b | frame 0x104 matches; int `settled` matches retail; retail keeps a function-level zero in ebx and `hit` in ebp (`mov ebp, eax ; xor ebx, ebx`, both reloaded at the back edge), ours only has loop-local zero registers; up/zeroB/lift/pivot/o1..o3 slot order differs |
| net/Net | `0x004ab960` | 149/1072 | b | 51 code diffs: `test esi, esi ; jge @54` vs `cmp esi, ebx ; jge @51`; retail adds `lea edx, [esp + N]`; slots differ |
| net/Net | `0x004aced0` | 361/370 | c | ring slot addressed through a byte-offset helper (fixes every SIB order); retail loads `from` before `lea ebx, [edx + edi]` for the system-message pointer, VC6 forms the pointer first (`mov ebx, edi ; add ebx, ecx`) and pushes system[2] from edx instead of eax |
| net/NetThread | `0x004af8e0` | 234/338 | c | 19 code diffs: retail adds `push ebx`; retail adds `mov eax, [A]` |
| net/NetThread | `0x004afa90` | 740/1251 | b | two blocks: the 0x80 acknowledgement swaps edx and ax/dx, the 0x4b keep-alive caches data in esi across EnterCriticalSection in retail |
| physics/bikeai/BikeAI | `0x0040e510` | 1133/1944 | b | 53 code diffs: retail adds `mov esi, [esp + N] ; test esi, esi`; `mov edi, [esp + N] ; test edi, edi ; je~` vs `je @582`; slot order differs |
| physics/bikeai/BikeAI | `0x00415640` | 6063/6112 | b | schedule and frame size match (squared distance and k dot grouped `z + ((x) + y)`, start-up zero vectors cleared with a chained assignment so they store z-first like retail); the one-use temporaries above +0x100 sit in other slots; the retail extent includes one nop of padding (candidate 6111 bytes) |
| physics/bikeai/BikeAI | `0x0040eca0` | 1493/17585 | b | prologue matches (frame/flag initialisations before the dt clamp); frame 0x95c vs retail 0x93c: named locals map ~1:1 (frame_layout), the gap is +0xc in the single-use lea return buffers (78 slots/0x3a0 vs 77/0x394) and +0x14 below `xf`; the ~60 two-use operator temporaries are the same set in a different quicksort-tie order. Retail keeps `up` (not address-taken) and the loop-head drag (`rf.x - 0.0`, `-32 - drag.y`) in memory; every drag spelling tried (7 drag x 3 force forms, address-taken/tracked padding, `v*0.0f`) is folded by VC6. Retail selects `side` by pointer and copies it to sideV and sideF at the merge (declaring sideF there: strict 1520 but aligned 3737 -> 3703, not kept) |
| physics/bikeai/BikeAI | `0x00413200` | 2026/4112 | b | KrustyBike member: frame 0x74 and most slots exact; first three locals at other offsets (`a` needs disp32; retail: c 0x5c, b 0x50, a 0x44 shared with `start`, steps 0, the WorldToLocalDirection/negated-cross temp alone at 0x68 -- moving a/b into the hit block gives retail's a/b/c offsets but merges the temps and shrinks the frame to 0x68; no a/b/c placement x start form beats 2026); operand order of the second lever cross product; ground-branch lean clamp keeps the value on the FPU stack; smoother target kept in a register |
| physics/bikeai/BikeAI | `0x00414370` | 424/4802 | b | KrustyBike member (candidate 4677 B): the gate-distance tail compares against `speed` (retail reads the speed slot there, 422 -> 424); the point loops are top-tested like retail (leading `if (i >= aiPath.count) break;`, 394 -> 422, 5 fewer differing instructions); frame 0x74 vs 0x78 (retail spills the difficulty, candidate keeps it in edi); retail lays the kinds 1/5 gate-distance tail out right after the first loop (its exit falls into it, the single-point/no-track paths jump back; duplicating the tail per branch is not cross-jumped, 5364 B); kind 1/5 gate-distance test placed after the single-point and no-track paths |
| physics/bvh/BoundingBoxTreeBuild | `0x0042c260` | 1622/1627 | c | retail reloads numCorners before remap; the remap pass reads `keep[corners[k]]` as `[offset + keep]` where retail has `[keep + offset]` |
| physics/bvh/BoundingBoxTreeQuery | `0x00425900` | 699/4200 | b | corner distances as float locals squared through QuerySquares (normalised diff 1552 -> 377); left: retail calls DotProduct in normalisations 5-7 and the two squared-corner constructors (more inline budget spent), and sums the projected radii before the half-segment projection in each axis test |
| physics/bvh/BoundingBoxTreeQuery | `0x00426be0` | 1673/2572 | b | cross products match; min/max of the projections (retail keeps p2 on the x87 stack and stores p1 twice), one operand pair of a vertex transform and the frame (0x160 vs 0x164) |
| physics/bvh/BoundingBoxTreeQuery | `0x00427c10` | 349/1110 | b | natural operator form: retail's helper call sequence; retail keeps the constant 1 in ebp and copies segment[1] at entry, so registers and slots differ |
| physics/bvh/BoundingBoxTreeQuery | `0x004280e0` | 453/2228 | b | inline BoxOverlap copy: one constructor still expanded and the recursion tails cross-jumped differently |
| physics/bvh/BoundingBoxTreeQuery | `0x0042a640` | 41/1428 | b | retail homes floats in the dead d0/d1 argument slots (frame 0x60 vs 0x80); cross-product operand order of the x term |
| physics/collision/CollisionPoint | `0x0043aa30` | 820/845 | c | frame, blocks and the y, x, z accumulated squared normal length match (samples/physics/collision/CollisionPointNearMisses.cpp); sum * (1/merged) loads y component-first in retail, one push fstp scheduled later |
| physics/collision/CollisionPoint | `0x0043ad80` | 597/615 | b | if/else with one exit gives retail's delayed pushes; one vector-copy store and the `points`/`frame` loads are swapped |
| physics/constraint/ConstraintMethodCollisionModel | `0x0043bdb0` | 1570/2619 | c | ApplyContactImpulse, frame 0xb8 and slot order match; termB (other-body branch) builds its cross/dot in a different x87 register order (fstp st(2), +2 bytes), plus scheduling of three struct copies and the `test edi` of `if (other)` |
| physics/contact/ContactImpulse | `0x005004a0` | 94/1062 | b | 0.94 instruction ratio, 1049 B: tail calls 0x404e60/0x5015b0/0x428060 as retail; frame 0x54 vs 0x60, torque cross loads r before n (leaf age), closingXY x before y |
| physics/contact/ObjectPlacement | `0x004b0df0` | 3535/3971 | b | 2 differing instructions (two lea esi one slot apart); frame: up/forward/firstPass in other slots (`tools/frame_layout.py` finds no use-count fix) |
| physics/helpers/GraphicsTest | `0x0047bd10` | 818/917 | b | DrawCircle: frame 0xb84 vs 0xb78, the `[eax - 4]` store and the centre-sum operand order; a `Vec3* p = &points[i]` local gave +4 |
| physics/helpers/SoultreeBounds | `0x004fd340` | 618/630 | c | rows _11.._13 written k = 1,3,2,4 and _14 k = 3,1,2,4; the s13 product of _11.._14 loads its operands the other way (retail m3j first) and two _44 products differ (samples/physics/helpers/SoultreeBounds.cpp) |
| physics/helpers/SoultreeBounds | `0x004fe8a0` | 541/610 | b | frame slot order (retail c, e, w, a) and the hi = w + ext tail |
| physics/helpers/SoultreeMatrix | `0x004fb8c0` | 262/1096 | b | 13 code diffs: `mov ecx, [esp + N] ; mov edx, [esp + N]~` vs `lea ecx, [esp + N] ; push ecx`; `fld [esp + N] ; mov [esp + N], ecx ; le~` vs `call A`; slot order differs |
| physics/helpers/SoultreeMatrix | `0x004fc050` | 1121/1171 | b | same instructions and length; three 12-byte slots ordered temporaries, pos, r where retail has r (shared with the delta copy), temporaries, pos |
| physics/helpers/SoultreeRotate | `0x004fd1f0` | 302/334 | c | x87 order matches; retail pushes `&p` for the second SetPosition after the z row's products, VC6 here before the y row's store |
| physics/helpers/SoultreeTransform | `0x004fd710` | 203/205 (masked) | c | FPU operand order: one `fld/fmul` pair loads the other operand first (2 bytes) ; x-row, transpose-order, y/z-order and ~10k structural forms (constructor return, reference plus copy, pointer/by-value matrix, out pointer, Swap helper, inline transpose) do not move `v.x * m._11` |
| physics/krustybike/KrustyBike | `0x0048eea0` | 608/2952 | c | slot 102, frame 0x50 exact; VC6 cross-jumps the head-turn arms differently (retail keeps the + side hold block and the - side retarget tail), `jne call ; jmp skip` vs retail `je skip ; jmp call` in the trick-chain switch, the `+0x153c = 0` else block before the shared advance, and the crash clamp keeps t on the x87 stack where retail spills it to the dt slot |
| physics/krustybike/KrustyBike | `0x0048fa60` | 472/532 | c | 4 code diffs: retail hoists `mov [esi + 0x11b8], bl ; mov [esi + 0x604], ebx` into the load delay of the +0x1540 kVec3Zero copy where the candidate hoists `+0x7c0/+0x7bc` (pick index = units after the copy - 32) |
| physics/krustybike/KrustyBike | `0x004919a0` | 151/230 | c | 8 code diffs: retail adds `mov edx, [edx + 0x60c] ; fsub [ecx + 0x~`; retail lacks `fsub [ecx + 0xb8]` |
| physics/krustybike/KrustyBike | `0x00491d10` | 523/1296 | b | prologue, frame 0x20 and lim in ebp match (if/else flag/lim, mode 2/3 copy path breaks); the 2/3 normalise join sits after the invsqrt arm (retail after the zero arm), its query call is not cross-jumped into the shared call, and retail homes the case 0/4 call in case 4 (candidate in case 0); candidate 1300 bytes |
| physics/krustybike/KrustyBike | `0x00492ad0` | 192/2315 | c | 10 code diffs (frame 0x34 exact): record-interval branch `je @end ; jmp @deltas` vs retail `jne @deltas ; jmp @end`, network branch conditional jump vs retail `push 0 ; jmp @end+1`, and the register choice it drags along |
| physics/krustybike/KrustyBike | `0x00493660` | 2005/8545 | c | 528 code diffs (frame 0x58 and slots exact): retail keeps 1 in ebp and 0 in edi from the second SetAxesPtr call on; retail stores `(len+50)*(len+50)` before the AllowWarping test; velocity y/z `fld s ; fmul` order; pose lerp `c - b` recompute; pose bytes in 0x492670/0x492ad0 scale by 255.0f (0x551420), and 100.0f is 0x5505ec |
| physics/motion/Motnctrl | `0x004a5e40` | 505/1090 | b | register permutation: retail file in ebx, poses and the zero constant in ebp, path in esi; the candidate swaps ebx/ebp and esi/edi |
| physics/motion/Motnctrl | `0x004a70c0` | 152/3330 | b | 733 code diffs: retail calls `CharacterPose::CharacterPose` 2 then 4 times where the candidate calls it 3 then 2; retail calls `operator*`/`operator+` in the first extrapolation loop; frame 0x2dc vs 0x284; slots differ (the 0x58 gap is two 0x2c CharacterPose; only spare unused pose locals reach 243, rejected as padding) |
| physics/motion/Motnctrl | `0x004a7dc0` | 476/523 | b | 4 code diffs: `fld [esi + 0x20]` vs `mov [esp + N], ecx`; retail lacks `mov [esp + N], ecx`; slots differ |
| physics/motion/Motnctrl | `0x004a7fd0` | 1097/1121 | c | axisZ cross product loads b's component first in each product (retail a's) |
| physics/motion/SteeringControl | `0x00504d30` | 145/234 | b | 6 code diffs: `fxch st(2) ; fmul st(2)` vs `fld st(0) ; fmul st(3)`; `fmul st(2)` vs `fmul st(1)`; slots differ |
| physics/shadow/D3DIMSoultreeShadow | `0x00446c30` | 730/771 | c | x87 term order of the inlined 4x4 product: one term pair in each of _11, _21, _31, _41 (retail's sum orders differ from slot 27's with the same helper) (samples/physics/shadow/D3DIMSoultreeShadowNearMisses.cpp); 748/771 with 1, 2, 9, 10, 18, 25, 26, 33 or 34 locals at the top; exact only with 5-6 locals inside the shared helper, which break the exact slot 27 |
| physics/shadow/D3DIMSoultreeShadow | `0x00446f40` | 180/1485 | b | retail's inline __asm fld/fistp rounding helper forces an ebp frame (0xe4 locals); the (int) casts call __ftol |
| physics/shadow/ProjectedShadow | `0x004da7b0` | 130/778 (masked) | b | tail and clip rectangle match (844/842 B, 18 aligned diffs); register choice in the sprintf/QueryFlag set-up and the saved render-target fields, and retail keeps a second release-and-return block for the surface-format failures |
| physics/shadow/ProjectedShadow | `0x004db0d0` | 123/2020 | b | opcode ratio 0.9874 (1924/2020 B, 4 hunks): int return, out-pointer transforms, by-value temporaries and shared mode tail as retail; retail's three-slot copy of the GetBounds extents is elided by VC6, shifting every frame slot |
| physics/shadow/ProjectedShadow | `0x004db8c0` | 21/1136 | b | 51 code diffs: retail adds `push ebp ; mov ebp, esp`; `mov [esp + N], ebx` vs `mov [ebp - N], ebx`; slot order differs; inline-asm `fistp` exclusion |
| physics/shadow/ProjectedShadow | `0x004dbd30` | 164/1244 (masked) | b | per-format row loops and the three Duff's-device fills as retail; register assignment of the clip fields and the format-9 constants (retail uses byte ops on field_0x94's low byte) |
| physics/shadow/ProjectedShadow | `0x004dc2b0` | 29/352 | b | 29 code diffs: retail adds `push ebp ; mov ebp, esp`; retail lacks `push ebp`; frame 0x24 vs 0x34, slot order differs; inline-asm `fistp` exclusion |
| physics/shadow/TerrainShadow | `0x005097d0` | 121/486 | b | 6 code diffs: retail copies extent onto its own slot through edx on the `extentX <= extentZ` path (`mov edx, [esp + 0xc] ; mov [esp + 0xc], edx`) and reloads `shadow->texture` in eax/edx where the candidate keeps edx; frame and both matrix branches match |
| physics/shadow/TerrainShadow | `0x00508bc0` | 353/3031 | b | slot 27 footprint: frame 0xdc vs 0xd0, slot order, the cross product's operand order and the corner loops' form; the inline budget needs the per-mode accumulate/finish text repeated (macros) |
| physics/shadow/TerrainShadow | `0x00509aa0` | 86/1548 (masked) | b | 90 code diffs: retail adds `push ebp ; mov ebp, esp`; retail lacks `push ebp`; frame 0x370c vs 0x3728, slot order differs |
| physics/soultree_base/SoultreePhysicsInlines | `0x0040b410` | 287/432 | b | 18 code diffs: `fld [edi + 0x1b0]` vs `fld [esi + 4]`; `fld [edi + 0x1b4]` vs `fld [esi + 8]`; slots differ |
| physics/tire/Tire | `0x00513490` | 199/201 | c | 2 code diffs: `mov [ebp + N], ebx` vs `mov [ebp + N], edi`; `mov [ebp + N], edi` vs `mov [ebp + N], ebx` in the pts[i].origin stores; store/declaration order, chained/comma assignment, member-wise copy, pointer/reference and typedef shifts give no gain |
| physics/tire/Tire | `0x005135f0` | 1584/1664 | b | UpdateContactPatch: lever `lea edi`/store scheduled later; second TireDot push earlier |
| physics/tire/Tire | `0x00513c70` | 769/797 | c | per-attachment failure clears give retail's zero register and length ([VC6_ZERO_REGISTER](VC6_ZERO_REGISTER.md)); left: rotating path's first world-position add operand order, z sum staged in the x slot |
| physics/tire/Tire | `0x00514550` | 2504/4258 | b | 20 differing instructions (abs masked), frame 0xc0 matches (function-scope `rolled`); retail keeps four call/ctor temporaries in one high slot (disp32 leas, most of the 10-byte size gap); left: owner branch loads the side pointer late, first lateral cross loads three operands in the other order, retail `fcom` where VC6 emits `fld st(0); fcomp` for the kept score, a few copy-store orders, owner-branch temporaries in other low slots, one `add esp` merge |
| physics/tire/Tire | `0x00515880` | 677/703 | c | retail tests keepZ one product later and loads z (and y.y) before y.x in the y.x products; operand/dot/decl order insensitive |
| physics/visibility/VisibilityQuadTreeTraversal | `0x0052d610` | 2218/3027 | c | 137 code diffs: `mov eax, [esp + N] ; mov ecx, [esp + N]` vs `mov eax, [edi + 4]`; retail adds `mov ecx, [eax] ; mov eax, [esp + N] ; p~` |
| race/BikeRace | `0x00417ed0` | 2828/6768 | c | retail stores `z*z` of each vector length to a stack temporary (`fstp [t]; faddp; fadd [t]`); the girl block's temporaries sit 4 to 12 bytes apart |
| race/BikeRace | `0x00419970` | 1386/13322 | b | frame 0xac4 vs 0xab0, slot order differs (register choice follows); retail keeps one more stream destructor tail |
| race/BikeRace | `0x0041eb20` | 1697/1710 | c | retail loads the scene-table index (+0x150) before the table pointer in the scene-object branch (edx/ecx swapped at the branch head) and again after the 0x4dab90 call; 162 parenthesisations of the index copy, table test and entry address give identical bytes |
| race/BikeRace | `0x0041f1d0` | 266/881 | c | retail keeps 0 in edx from the first instruction (`cmp [edi + 0x3430], edx`, `xor edx, edx` after calls) and has three 0x0041eb20 call copies; the objects scan's found path now calls and returns (second copy, size matches) |
| race/BikeRace | `0x004210f0` | 975/2940 | b | inline budget: retail calls the Vector3 constructor in the mode 2/3 tail and expands Scale in mode 4; registers follow |
| race/CarProcedural | `0x0042fd80` | 221/2844 | b | slot 10: frame 0xd0 vs 0xdc (retail shares the wheel loop's axisA with the orientation's `flat`, axisB with nothing), the steering clamp's memory local, component/scalar load order in the `back` normalisation and blend; masked, 781 instructions like retail's |
| race/CarProcedural | `0x00430b10` | 299/843 | b | 45 code diffs: retail adds `mov esi, ecx`; `mov edi, ecx ; mov esi, [edi + 0x6c] ; ~` vs `mov edi, [esi + 0x6c] ; mov ebp, [esi +~`; slots differ |
| race/Krusty3DObjects | `0x0048b100` | 254/2820 | b | the field-of-view conversion keeps retail's two multiplies (0.5 then 0.01745329) through a parenthesised macro; third camera column copied by integer moves in retail (two components kept on the x87 stack here); explicit hide returns cache 0 in ebx (retail edi, `finished` tested through cl) and still share one return where retail gives the two early ones their own epilogues |
| race/Krusty3DObjects | `0x0048ccc0` | 1299/1308 | c | retail loads the camera (`mov edx, [esi + 0x38]`) before storing the length to field_0x1ec |
| race/RaceSound | `0x004e4d10` | 806/2038 | c | two blocks: retail reloads the remainder for the first half's `field_0x18 = rest` (cross-jumped); the second half's looping path keeps half in ebp and the remainder in edx |
| race/RaceStatus | `0x004e5d00` | 168/519 | c | 29 code diffs: retail adds `mov ebp, [esp + N]`; retail adds `test ebp, ebp` |
| race/Recorder | `0x004e6f80` | 652/2180 | c | case 1 `goto opened` reproduces retail's jmp into case 2's tail; left: case 2's `[0xc8]=1; Leave` suffix still cross-jumps into case 4 (retail keeps it), and `name`/`headerSize` slots swap (0x08/0x10); real extent 2148 |
| race/Recorder | `0x004e7d60` | 272/2448 | b | 99 code diffs: `jne @767 ; mov eax, [esi + 0x88] ; test~` vs `jne @761 ; mov ecx, [esi + 0x88] ; xor ~`; slot order differs |
| race/Wrecker | `0x00531740` | 1606/1624 | c | slots match; the argument push of two loop constructor calls comes one load earlier (`push ecx` before the particle-pointer load and before `along.y`), two registers differ |
| race/Wrecker | `0x005329e0` | 1200/1312 (inside Wrecker.cpp) | a | every instruction equal; slot order only (retail keeps `push` nearest esp, then the y/x extent copies, `p`) |
| render/BackgroundImage | `0x004049d0` | 108/606 (masked) | c | block layout as retail; VC6 caches 0 in edi (`xor edi, edi`, `mov [esp + N], edi`) where retail stores immediates, which moves the region loop's registers |
| render/CacheTexture | `0x005102d0` | 426/529 (masked) | b | 13 code diffs; slots differ |
| render/CubeDraw | `0x0043dc60` | 121/1064 | b | 53 code diffs: `xor ebp, ebp` vs `xor edx, edx`; `mov [esp + N], ebp ; mov edi, 0x100 ; o~` vs `mov [esp + N], edx ; mov ebx, 0x100 ; o~`; slots differ |
| render/CubeDraw | `0x0043e0b0` | 30/352 | c | 22 code diffs: retail adds `push ebx`; retail adds `mov ecx, [esp + N]` |
| render/D3DIMSoulTree | `0x00440810` | 135/1349 | b | 50 code diffs: `jle @345 ; xor esi, esi` vs `jle @340`; `mov [esp + N], esi ; mov eax, [edi + 0x~` vs `mov edx, [edi + 0x1c] ; mov esi, ebx ; ~`; frame 0x84 vs 0x80, slot order differs |
| render/D3DIMSoulTree | `0x00440f30` | 657/7296 | b | DrawCurrentLod: static-local and `$E` bindings fixed, all relocations bound; mode 8/2 normalisations call the out-of-line Vector3 constructor 0x00404e60 as retail (525 differing insns, was 548); frame (0x940 vs 0x9e8) and register allocation left |
| render/D3DIMSoulTree | `0x00443aa0` | 799/832 | c | 11 code diffs: retail lacks `fmul [eax + 0x10] ; fld [esp + N]`; retail lacks `faddp st(1)` |
| render/DebugOverlay | `0x00447a00` | 460/999 | b | body matches; VC6 copies the epilogue into the failure and success returns (retail: one shared epilogue) |
| render/DirectoryList | `0x0044a600` | 74/109 | c | 6 code diffs: `jne @31 ; mov al, byte ptr [esp + N]` vs `jne @32 ; mov eax, [esp + N]` |
| render/GR_BitString | `0x004239c0` | 343/420 | b | 14 code diffs: retail lacks `mov ebx, ecx`; retail adds `mov ebp, ecx`; slots differ |
| render/GR_BitString | `0x00423b70` | 93/442 | b | 16 code diffs: retail adds `add esi, 4`; `or ecx, eax` vs `or eax, ecx ; mov [edx - 4], eax`; slots differ |
| render/Grid1 | `0x0047caa0` | 213/1728 | b | slot 3 texture stage switch; with the terrain/extra casts 63 instructions differ, mostly after the stage ladder, whose fourth compare VC6 folds (20 bytes short) |
| render/Grid1 | `0x0047d470` | 313/786 | b | `rest` before `line`; line pointer still spilled (frame 0x838 vs 0x834), first/row in ebx/edx where retail has edx/eax |
| render/Grid1 | `0x0047d780` | 227/1010 | b | 50 code diffs: `je @321 ; mov esi, [esp + N]` vs `je @315`; `mov eax, [esi]` vs `mov edi, [esp + N]`; slot order differs |
| render/GridNode | `0x00484d70` | 81/90 | c | 3 code diffs: `mov ecx, [eax*4 + A] ; add ecx, ebx ; m~` vs `mov eax, [eax*4 + A] ; add eax, ebx ; m~`; retail adds `xor ecx, ecx ; mov cx, word ptr [eax + ~` ; ~110 loop, index-local, final-test and parenthesis spellings unchanged |
| render/GridNode | `0x00483d40` | 96/4189 | b | slot 1: ebp frame and four bare `fistp [eax]` from an `__asm` rounding helper (`__ftol` still used elsewhere), so unreachable under the no-inline-asm rule; `/Oy-` changes nothing else |
| render/Griddraw | `0x0047de90` | 977/1083 (masked) | c | 30 code diffs: retail lacks `mov esi, [esp + N]`; `mov edi, [ebp*N + N]` vs `mov edi, [esp + N]` |
| render/Griddraw | `0x0047e600` | 1605/1617 (masked) | c | frame matches; the extent.x/extent.y products load the cell size first (`fld [edx + 0x40] ; fmul [ecx + 0x164]`), retail the minimum ; the order follows which fields are read (+0x168 gives min-first, +0x164/+0x18 cell-first), not source order or parentheses (432 forms) |
| render/Griddraw | `0x0047f210` | 361/543 (masked) | b | 29 code diffs: retail adds `xor esi, esi`; retail lacks `xor edi, edi`; slot order differs |
| render/Griddraw | `0x00480ad0` | 395/438 | b | retail keeps the zero vertexStart in eax and compares ranges/block against it (`cmp esi,eax`, `indexStart = eax`), ranges in esi and data in edi; VC6 uses `test` and swaps esi/edi; ~20 init/type/declaration forms unchanged or worse |
| render/Griddraw | `0x00481300` | 121/216 (masked) | c | child index: `lea ecx, [edi + ebp] ; add ecx, table[quad]` vs `mov edx, table[quad] ; add edx, edi ; add edx, ebp` |
| render/Griddraw | `0x004815e0` | 64/1083 | b | retail keeps both axis deltas on the x87 stack across the near/far branch (`fcom`, spilled in the arms) and pushes edi after the coarse return; VC6 stores them, which shifts the frame (the 65535 clamps now branch as retail) |
| render/Griddraw | `0x00481de0` | 133/1616 (masked) | b | 85 code diffs: retail adds `mov ebx, [esp + N]`; retail adds `mov ebp, [esp + N]`; slots differ |
| render/ManagedTextureGroup | `0x0050c960` | 17/227 (masked) | c | 5 code diffs: `sub esp, 0x80` vs `sub esp, 0x78`; `je @167` vs `je A` |
| render/ManagedTextureGroup | `0x0050dad0` | 1887/5022 | b | repack with partial blits, 87 code diffs (samples/render/ManagedTextureGroupNearMisses.cpp) |
| render/MatrixUtil | `0x004a1a50` | 171/175 | c | 1 code diffs: `fld [ecx + 4] ; fmul [eax + 0x28]` vs `fld [eax + 0x28] ; fmul [ecx + 4]` ; operator()/operator[] accessors give no gain either |
| render/MorphBastardModifier | `0x004a33b0` | 1938/2046 | b | spill homes of the loop scalars and the two inverse temporaries; operand order inside the channel inverse's products |
| render/MorphBastardModifier | `0x004a3c80` | 74/3663 | b | 4 code diffs: `push ebp` vs `mov ebx, [esp + N]`; `mov ecx, [esp + N]` vs `mov ecx, [ebx + 0x44]`; frame 0x1b4 vs 0x1fc |
| render/MorphBastardModifier | `0x004a4c60` | 185/1570 | c | object/channel loop registers: retail keeps the object offset in esi, reloads the index from +0x28 and keeps the channel in ebp |
| render/Overlay | `0x004b6710` | 63/361 | b | retail keeps `whole` and the byte count on the stack (6 slots), the destination in ebp and the row count in eax; VC6 keeps `whole` in edi, the byte count in ebp and the destination and row count on the stack |
| render/PCTextureMap | `0x004c69c0` | 429/894 (masked) | c | 31 code diffs: `jne @390` vs `jne A`; `jne @120` vs `jne @116 ; lea edx, [esp + N]` |
| render/PCTextureMap | `0x004c71c0` | 286/538 (masked) | c | 27 code diffs: `mov ebp, 1` vs `mov ebx, 1`; `and eax, ebp` vs `and eax, ebx` |
| render/PCTextureMap | `0x004c7b40` | 257/767 | b | rect read in retail order; the row pointers swap esi/ecx in the loop nest |
| render/PCVideoCard | `0x004ca520` | 26/124 | c | 13 code diffs: `mov ebx, [esp + N]` vs `push ebp`; retail adds `mov ebp, [ecx + 0x74]` |
| render/PCVideoCard | `0x004ca5a0` | 66/490 | c | retail keeps `target` in ebp outside the mode loop and 480 in ebp inside it; VC6 here never pushes ebp |
| render/PCVideoCard | `0x004cab00` | 144/1113 | b | retail keeps the mode index in esi shared with the created target (two -3 stores), re-tests `render` after the found path and places the Slot8-success path after the epilogue |
| render/PCVideoCard | `0x004cb330` | 35/628 | b | 46 code diffs: `push ebx ; mov ebx, [esp + N]` vs `push ebp ; mov ebp, [esp + N]`; `mov eax, [ebx]` vs `mov eax, [ebp + N]`; slots differ |
| render/Pixtrans | `0x004cdf10` | 46/636 | b | 33 code diffs: retail adds `mov ecx, [esp + N] ; mov [esp + N], eax~`; retail lacks `mov [esp + N], eax ; mov eax, [esp + N]~`; slot order differs |
| render/Pixtrans | `0x004ce420` | 280/458 | b | frame 0x10 and the x loop match; VC6 loads `source` into ebp at entry (ebp pushed before the height guard) and forms the next source row in ebp where retail uses eax and copies it at the row head; first pixel's zero extension in ecx (retail edi) |
| render/Pixtrans | `0x004cee30` | 1121/1125 masked | b | after the keyed x loop retail reloads the source stride before the width |
| render/Pixtrans | `0x004cf2a0` | 242/1735 | b | draft: format, pixel size and palette in ebx/esi/edi where retail uses edi/ebx/esi; frame 0x50 vs 0x54 |
| render/ResourceManager | `0x004e9030` | 100/798 | b | 5 code diffs: `jne @35` vs `mov [esp + N], ebx ; jne @36`; `je @250` vs `je A`; slots differ |
| render/TextService | `0x0050ade0` | 117/125 | c | 1 code diffs: `mov eax, [ebp + N] ; mov edx, edi ; mov~` vs `mov ecx, [ebp + N] ; mov eax, edi ; mov~` |
| render/TextService | `0x0050b080` | 530/890 | b | 30 differing instructions: w and v trade esi and a stack slot; row offsets in other registers |
| render/TextService | `0x0050b400` | 398/758 | b | 31 differing instructions: w and v trade esi and a stack slot; row offsets in other registers |
| render/TextService | `0x0050b700` | 398/758 | b | 31 differing instructions: w and v trade esi and a stack slot; row offsets in other registers |
| render/TextService | `0x0050ba00` | 770/802 | b | scheduling of the v3.sx and v5.sx stores |
| render/Tgafile | `0x00512990` | 354/982 (masked) | c | 49 code diffs |
| track/SceneManager | `0x004ecd60` | 4534/4536 | c | the name terminator adds the count before the element offset (`add edx, esi`; retail `add edx, ebx`); ReadAnimations' identical terminator matches. Only deleting the whole later `if (auralScape)` block flips it; declaration shifts 0-63, 0-33 dead locals, count/length forms, aliases and pointer arithmetic do not |
| track/SceneManager | `0x004edfe0` | 4372/5346 | b | scalar frame slots, the NumberOfMotions count kept in a register, one shared `Cannot find %s` sprintf tail where retail keeps two, random-set loop-end load order |
| track/Track | `0x00516ef0` | 1032/1042 | c | edge 6 of the crossing test computes `b.z - a.z` before `p.x - a.x` in retail; edge 2 matches with the helper's first factor written `((b->x - a->x))` ; retail orders edges 2, 5, 6 one way and 1, 3, 4 the other; ~5,500 parenthesis/cast/struct/local forms give 1032 at best |
| track/Track | `0x00517340` | 57/655 (masked) | b | 44 code diffs: retail lacks `mov [esp + N], 0`; `add esp, 0x24` vs `add esp, 0x20`; frame 0x24 vs 0x20 |
| track/Track | `0x0051ffe0` | 186/511 (masked) | c | 6 code diffs: `test eax, eax ; je @238` vs `cmp eax, ebx ; je A`; `call A ; movsx ebx, byte ptr [esp + edi~` vs `jmp A` |
| track/TrackOverlay | `0x00519a20` | 535/894 (masked) | c | 31 code diffs: retail lacks `mov ebp, ecx`; retail adds `push edi ; mov edi, ecx ; xor ebx, ebx` |
| track/TrackOverlay | `0x0051bed0` | 1057/1088 (masked) | b | 3 code diffs: retail lacks `add edx, 0x5e0 ; push edx ; push ecx`; retail adds `add edx, 0x5e0`; slots differ |
| track/TrackOverlay | `0x0051c4f0` | 334/520 (masked) | b | 13 code diffs: `mov ecx, [esp + N]` vs `mov eax, [esp + N]`; retail adds `fld [esp + N] ; faddp st(2)`; slots differ |
| track/TrackOverlay | `0x0051c720` | 249/962 (masked) | b | 10 code diffs: `mov [ebp + N], ecx ; fstp [ebx + 0x1dc]~` vs `mov [ebx + 0x1bc], ecx`; retail adds `fstp [esi] ; fild [esp + N]`; frame 0x74 vs 0x8c, slot order differs |
| track/TrackOverlay | `0x0051cf80` | 1768/1785 (masked) | c | right panel block: retail `mov eax, [ebp + 0x134] ; mov ecx, eax` forms the width in ecx; VC6 swaps eax and ecx |
| track/TrackOverlay | `0x0051d730` | 214/510 (masked) | b | 26 code diffs: `xor ebp, ebp` vs `xor ebx, ebx`; `mov [esi + 0x198], ebp ; mov [esp + N],~` vs `mov [esi + 0x198], ebx ; mov [esp + N],~`; slots differ |
| track/TrackOverlay | `0x0051e3f0` | 886/894 (masked) | b | 6 code diffs: `mov edx, [ebx + ebp*4 + 0x16c]` vs `mov eax, [ebx + ebp*4 + 0x16c]`; `lea eax, [esp + N] ; fld [edx + 0x4a4]` vs `lea ecx, [esp + N] ; fld [eax + 0x4a4]`; slots differ ; the "%.3f" branch register choice (racer eax, buffer ecx, game edx); shadowing-size, racer-local and branch forms reach 853-886 |
| ui/DlgProcs | `0x0044b200` | 925/5232 | b | 144 code diffs: `ja @1362` vs `ja A`; `jmp @1362` vs `jmp A`; frame 0xc64 vs 0x6c4 |
| ui/DlgProcs | `0x0044fae0` | 1181/1235 | b | 8 code diffs: retail lacks `mov edi, esp`; retail adds `mov edi, esp`; slots differ |
| ui/DlgProcs | `0x004507a0` | 589/1668 | c | 32 code diffs: retail computes `lea eax,[TrackGame+0x570]` in each loop test and reloads eventManager through it at the body top; VC6 here reuses the loaded pointer; a reference-returning accessor, `++i`, a reversed test, `(&em)[0]` and `field_0x50 + i` all compile the same |
| ui/DlgProcs | `0x004513a0` | 595/2004 | c | first loop: retail reloads EventManager for the +0x04 sprintf arg, VC6 here reuses it; `continue`, `EventManager* const&` alias, entry reference/pointer, TrackGame local, `(int)` cast and `(field_0x50 + i)->` all give 595 |
| ui/GUIManager | `0x00485a70` | 62/345 | c | 29 code diffs: retail lacks `mov edx, ebx`; retail lacks `mov ebp, [esp + N]` |
| ui/GUIManager | `0x00486170` | 280/896 | b | 52 code diffs: retail lacks `xor ebx, ebx`; `cmp ecx, ebx` vs `xor ebx, ebx`; frame 0xa4 vs 0xb0, slot order differs |
| ui/GUIManager | `0x00486b10` | 89/106 | c | two per-component ternaries give retail's `pop edi` placement; retail loads bottom into eax after storing x and passes the text in eax and &position in edx (VC6 here hoists the bottom load into edx beside the cdq and swaps the call registers) |
| ui/GUIManager | `0x00488120` | 46/52 | c | retail tests the owner for zero before comparing it with `this`; VC6 threads the ternary's xor arm into a leading zero test, so the sample compares with `this` first (samples/ui/GUIManagerNearMisses.cpp) |
| ui/GameUi | `0x0046a920` | 684/2323 (masked) | b | 17 code diffs: `mov eax, 0x64` vs `mov ecx, 0x64 ; mov eax, 1 ; mov [esp +~`; `mov eax, 1` vs `mov [esp + N], eax`; frame 0x9bf4 vs 0x9c74, slot order differs |
| ui/GameUi | `0x0046ef00` | 352/555 | c | 13 code diffs: retail adds `mov [edi + 0xc], ebx`; retail lacks `mov [edi + 0xc], ebx` |
| ui/GameUi | `0x004705d0` | 91/101 | c | 5 code diffs: `mov edx, [esi + 0xcc]` vs `mov ecx, [esi + 0xcc] ; push ecx`; retail lacks `push edx` |
| ui/GameUi | `0x004733a0` | 85/99 | c | 4 code diffs: `mov ecx, [esi + 0xcc] ; push ecx` vs `mov eax, [esi + 0xcc]`; retail adds `push eax` |
| ui/GameUi | `0x004734c0` | 256/354 | c | 8 code diffs: `je @82 ; jmp @81` vs `je @83 ; mov edi, 1 ; jmp @84` |
| ui/GameUi | `0x004738a0` | 440/975 | b | retail keeps the width and the redraw count in memory |
| ui/GameUi | `0x00474150` | 1018/1044 | c | Backspace: retail gives the cursor eax, VC6 here edx; one local (1004 length / 351 cursor), no locals (339), call as condition and swapped decrements (1002) are worse |
| ui/GameUi | `0x00477e90` | 322/342 | c | register choice after the row-height call: `mov edx, [esp + N] ; mov ecx, [ebx]` vs `mov ecx, [esp + N] ; mov edx, [ebx + 4]` (rect.top/point register choice) ; without the height local (as in the exact slot 10 at 0x476020) only ebx/ebp are swapped (155/344) |
| ui/GameUi | `0x00478570` | 139/682 | b | 32 code diffs: `lea eax, [esi + 0x1bc] ; mov [esp + N],~` vs `mov eax, [esi + 0x1bc] ; lea ebx, [esi ~`; retail adds `mov [esp + N], ebx`; slot order differs |
| ui/GameUi | `0x00478e10` | 231/303 | c | frame and head are retail's (the area is tested in place, no RECT copy); the opaque-result tail differs as in UIButton slot 28 `0x004734c0` (retail returns through edi, 8-bit hit sets edi directly) |
| ui/GameUi | `0x0047a400` | 702/962 | c | the area bottom written from the list gives retail's store order; VC6 forms it with `lea edx, [edi + edx - 2]` (retail adds the scroll bottom's `sub eax, 2` value) and swaps ebx/edi, shifting later scratch registers |
| ui/GameUi | `0x0047b490` | 120/220 | c | the key pixel kept in the red term's variable stays in a register and the row counter spills into the key's slot as retail; VC6 ORs green and blue first where retail ORs red and green first, saves edi in the prologue (retail inside the row loop) and steps the row with one lea |
| ui/KrustyUI | `0x004988a0` | 872/1104 | b | 8 code diffs: `mov edx, [esi + 0x2c] ; mov eax, [esp +~` vs `mov eax, [esi + 0x2c] ; mov ecx, [esp +~`; `mov edx, [eax + 0xc0] ; mov [edx + 0x5c~` vs `mov eax, [eax + 0xc0] ; mov [eax + 0x5c~`; slots differ |
| ui/ProCircuitProcs | `0x004d72c0` | 1125/1291 | b | 11 code diffs: retail lacks `mov edi, esp`; retail adds `mov edi, esp`; slots differ |
| ui/ProCircuitProcs | `0x004d8c40` | 427/886 | c | 15 code diffs: retail lacks `mov edx, [esi + 8]`; retail lacks `mov ebx, [esi + 0x14]` |
| ui/ProCircuitProcs | `0x004d8fc0` | 836/882 | c | 12 instruction diffs: retail loads the message kind into ecx before the circuit (VC6 here: circuit first, kind in eax) and stores the fee member first with other registers |
| ui/ProCircuitProcs | `0x004d9860` | 1068/1122 | c | 13 instruction diffs: the prologue and init-store differences of `0x004d8fc0` |
| ui/SelectGamePicProcs | `0x004f2340` | 933/2148 | b | 62 code diffs: `mov [esp + N], ecx` vs `push esi`; `push esi` vs `mov [esp + N], ecx`; slot order differs |
| ui/SelectGamePicProcs | `0x004f3720` | 92/380 | b | 10 code diffs: retail lacks `jmp @44 ; lea ecx, [esp + N] ; push 0x80`; slots differ |
| ui/SelectGamePicProcs | `0x004f8220` | 110/840 | b | 57 code diffs: `xor ebp, ebp` vs `xor edi, edi`; `mov [esp + N], ebp ; mov [esp + N], ebp` vs `mov [esp + N], edi ; mov [esp + N], edi`; slot order differs |
| ui/SelectGamePicProcs | `0x004f8820` | 1214/1268 | b | 8 code diffs: retail lacks `mov edi, esp`; retail adds `mov edi, esp`; slots differ |

## Declaration-shift scan of class (c)

The class (c) near misses above whose files compile standalone with the
default include paths (81 functions in 78 files) were also compiled with
0..63 unrelated typedefs prepended to the file (`tools/decl_shift_scan.py`,
one full period of VC6's symbol-arena tie-break, docs/VC6_OPERAND_ORDER.md).
None changes score at any count, and of the 1356 other bound functions in
those files only BikeRace `0x00419970` (itself a partial) moves. The header
sensitivity of
TrackGame slot 1 is therefore not what keeps these functions from matching;
their differences (x87 load order, `[base+index]` order, register choice)
follow the deterministic leaf-age rule of that document and are source-form
differences.

## Measuring inside the canonical unit

A sample compiled with only the headers can differ from the same body
compiled inside its unit (`#include "../../src/reconstructed/X.cpp"` at the
top of the sample); the cause is not isolated, the differences show in tail
merging, block placement and epilogue copies. Game `0x00467b70` (406 -> 761, now exact),
Wrecker `0x005329e0` (763 -> 1200, with the unit's own vector operators
instead of copies) and ControlInterface slot 3 `0x004bf4f0` (43 -> 76) are
measured that way. Every other sample that did not include its unit was
also compiled inside it (2026-10-12 scan, duplicate statics and helpers
removed): no other function's bytes changed except PCAudio `0x004bc6b0` and
EcoSystem `0x00457480` (their rows are already measured in the unit), CarProcedural slot 10
`0x0042fd80` (221 -> 211, worse) and Net `0x004ab960` (149 either way, the
candidate's extent becomes retail's 1072); those samples keep their header
includes.

## Reproduction

For one function: compile its file with `tools/compile.py --compiler vc6`
(physics files need `--include src/krusty2`) and run
`python3 tools/frame_layout.py OBJ SYMBOL --va 0xVA --exe "$MCM2_EXE"`.
The classification masks frame displacements, relocation immediates and
branch targets (as instruction indices) in both streams; a function is
**a** when the masked streams are identical and the candidate-to-retail
slot map is a permutation (retail may merge two candidate slots into one),
**b** when the streams differ and the frame size, slot order or mapped
slots differ too, **c** otherwise. Operand swaps between two frame slots
inside one instruction pair (`fld [a]; fadd [b]` against `fld [b]; fadd
[a]`) are operand order, not slots, and stay in **c**.
