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

Counts: a 1, b 163, c 111, total 275. The three class (a) functions of the
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
| game/EventManager | `0x0045d480` | 2064/4246 | b | frame size 0x460 matches; scalar spill homes below +0x68 differ, the squared lengths load all three components before squaring (retail `fld; fld st(0); fmulp` per component), the Winner branch registers run one step behind retail |
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
| krusty2/broadphase/Quadtree | `0x004dcac0` | 20/562 (masked) | b | 37 code diffs: retail adds `sub esp, N`; `sub esp, 0x30` vs `mov [esp + N], ecx`; frame 0x0 vs 0x34, slot order differs |
| krusty2/broadphase/Quadtree | `0x004dcf20` | 35/309 (masked) | b | 20 code diffs: `push esi ; mov esi, [esp + N] ; cmp esi~` vs `push ebp ; mov ebp, ecx ; mov ecx, [esp~`; `dec eax ; mov ebx, esi ; mov [ecx + 0x8~` vs `mov ebx, [ebp + N] ; dec ebx ; mov eax,~`; slot order differs |
| krusty2/broadphase/Terrain | `0x00506e90` | 850/1502 (masked) | b | 44 code diffs: `je @308` vs `je @282 ; fld [esp + N] ; push ecx`; retail adds `mov ecx, [esp + N]`; slots differ |
| krusty2/broadphase/Terrain | `0x00507c10` | 71/2731 (masked) | b | 128 code diffs: retail adds `push ebp ; mov ebp, esp`; retail lacks `push ebp`; frame 0x114 vs 0x108, slot order differs |
| krusty2/broadphase/Terrain | `0x005059d0` | 1593/2116 | c | TRN loader: same frame (0x440, every local in its retail slot), calls and EH states; register choice (retail keeps -1 in edi/ebx, the candidate keeps 3 and 0 in ebx), colour-key scheduling and the palette loop's pre-offset pointer stride differ |
| krusty2/collision/CollisionObject | `0x00432260` | 441/458 | b | 2 code diffs: retail adds `mov [esp + N], eax`; retail lacks `mov [esp + N], eax`; slots differ |
| krusty2/collision/CollisionObject | `0x004334c0` | 1046/1066 (masked) | b | 11 code diffs: `call A` vs `call @0`; `fmul [edi + 4]` vs `fmul [edi + 0x24]`; slots differ |
| krusty2/collision/CollisionObject | `0x00434540` | 1054/1068 | b | 8 code diffs: retail adds `fld [eax + 0x38] ; fmul [eax + 4]`; retail lacks `fld [eax + 0x38] ; fmul [eax + 4]`; slots differ |
| krusty2/collision/CollisionObject | `0x00434bb0` | 163/305 | b | 10 code diffs: `fld [eax - 4]` vs `fld [eax - 8]`; `fld [eax - 0xc]` vs `fld [eax - 0x10]`; slots differ |
| krusty2/collision/CollisionObject | `0x00434cf0` | 147/448 | c | 16 code diffs: `fld [eax - 4]` vs `fld [eax - 8]`; `fld [eax - 0xc]` vs `fld [eax - 0x10]` |
| krusty2/collision/CollisionObject | `0x00436100` | 31/804 | c | 35 code diffs: `sub esp, 0x4c` vs `sub esp, 0xac`; `mov eax, [ecx + 0x54]` vs `mov ecx, [ecx + 0x54] ; lea edi, [esp +~` |
| krusty2/collision/CollisionObject | `0x00436720` | 66/790 (masked) | c | 52 code diffs: `sub esp, 0x18` vs `sub esp, 0xc`; `add esp, 0x18` vs `add esp, 0xc` |
| krusty2/collision/CollisionObject | `0x00436af0` | 33/812 (masked) | c | 16 code diffs: retail lacks `mov eax, [esp + N]`; `lea esi, [eax + 0x48] ; jle @48` vs `mov edi, [esp + N] ; jle @47` |
| krusty2/collision/CollisionObject | `0x00436e50` | 335/2099 | b | HullVsModel: frame 0x108 vs 0x120; retail inlines the first offset rotation and calls 0x42a4b0/0x42a450/0x42a510 for the rest (inline budget), a in ebx, b in ebp |
| krusty2/collision/CollisionObject | `0x004376f0` | 681/707 | c | HullVsModelSwept: mean as ModelVsHull. Left: one push schedule, the loop's this/offset loads and the mean copy registers |
| krusty2/collision/CollisionObject | `0x00437c20` | 703/716 | c | ModelVsHull: frame and slots match; mean written per component with `s` assigned inside the x product (x term now matches). Left: two argument-push schedules (before 0x424730 and before HullVsHull) and the mean copy registers (retail eax/ecx/edx) |
| krusty2/collision/CollisionObject | `0x00437ef0` | 847/906 | c | ModelVsHullSwept: frame 0x7c matches; mean as ModelVsHull. Left: scheduling of the two box copies, one push and the mean copy registers |
| krusty2/collision/CollisionObject | `0x00438280` | 699/710 | c | ModelVsModel: as ModelVsHull, with the same per-component mean. Left: one push schedule (`sub esp, 0xc` placement) and the mean copy registers |
| krusty2/collision/CollisionObject | `0x00438550` | 712/783 | c | ModelVsModelSwept keeps the last contact (no averaging); scheduling of the box copies, register choice in the loop |
| krusty2/collision/CollisionObject | `0x00438c90` | 61/475 | c | 31 code diffs: `mov ebx, [ecx + 0x54]` vs `xor ebx, ebx`; `xor ebp, ebp` vs `mov ebp, [ecx + 0x54] ; sub edx, ebx` |
| krusty2/collision/CollisionObject | `0x004392c0` | 303/311 | c | 6 code diffs: `mov eax, [esp + N] ; mov ecx, [eax + 0x~` vs `mov ecx, [esp + N] ; mov eax, [ecx + 0x~`; `dec ecx` vs `dec eax` |
| krusty2/collision/CollisionObject | `0x00434eb0` | 254/1885 | b | UpdateModelBounds: the Min argument now uses the out-of-line Vec3 ctor 0x404e60 as retail; 445/479 normalised insns align (was 438), 1902 vs 1885 bytes; byte score fell because stack slots shifted |
| krusty2/collision/CollisionObject | `0x00435830` | 157/1776 | b | SetTransform: the hull case's inline 4x4 product emits its terms as 1,3,2,4 (retail 3,2,1,4 with the local matrix loaded first); frame 0x30 both |
| krusty2/collision/CollisionObject | `0x00439820` | 149/1476 | b | SegmentTouchesObject: pre-test calls match; retail gives each operator result its own slot (frame 0xa8 vs 0x9c), segment pointer in ebx |
| krusty2/soultree/SoulTreePhysics | `0x00500220` | 270/619 (masked) | b | 24 code diffs: `je @201 ; fld [edi + 8]` vs `je @197`; `fmul [esi + 4]` vs `fld [esi + 4] ; fmul [edi + 8]`; slots differ |
| krusty2/soultree/SoulTreePhysics | `0x00501600` | 651/925 | b | 18 code diffs: `jmp @239 ; fld [esp + N]` vs `jmp @242`; slots differ |
| krusty2/soultree/SoulTreePhysics | `0x00501e90` | 160/471 (masked) | b | 20 code diffs: `jne @56` vs `jne @55 ; mov [esp + N], 0 ; mov edx, [~`; slots differ |
| krusty2/soultree/SoulTreePhysics | `0x00502f60` | 1366/1942 | c | 6 code diffs: retail adds `mov [esi + 0xbc], ebx`; retail lacks `mov [esi + 0xbc], ebx` |
| krusty2/vehicle/Bike | `0x00405190` | 139/2381 (masked) | b | 99 code diffs: retail adds `mov edx, [esp + N]`; retail adds `push edi ; xor ebx, ebx ; xor edi, edi`; frame 0x94 vs 0xac, slot order differs |
| krusty2/vehicle/Bike | `0x004060c0` | 121/1436 (masked) | b | 63 code diffs: `mov edx, [esi + 0x4a8] ; mov eax, [esi ~` vs `mov eax, [esi + 0x4a8] ; mov ecx, [esi ~`; retail lacks `xor ecx, ecx`; frame 0x204 vs 0x214 |
| krusty2/vehicle/Bike | `0x00406ae0` | 78/1576 (masked) | b | 118 code diffs: `push ecx` vs `push ebp ; mov ebp, esp ; sub esp, 8`; `jne @16 ; mov [esp + N], 0 ; jmp @20` vs `jne @18 ; mov [ebp - N], 0 ; jmp @22`; frame 0x0 vs 0x8 |
| krusty2/vehicle/Bike | `0x004079c0` | 2080/6718 | b | spill homes of the loop values, the matched wheel kind kept in esi by retail, store scheduling of the local string tables and Vec3 constants (see PHYSICS_VALIDATION.md); 40/32-degree constants now 40*0.01745329f / 32*0.01745329f |
| krusty2/vehicle/Bike | `0x00407440` | 188/485 | c | clamps are BikeMin(BikeMaxF(t,.25),.75) and BikeMaxF(BikeMin(a,32.2),-32.2), constants bound; left: retail's dead `cnt+=2` store, idx/next/step/cnt register roles swapped, 0.0310559 read from the 0x550790 copy (binding gives 0x550ad0) |
| krusty2/vehicle/Bike | `0x00409b30` | 826/1260 (masked) | c | 18 code diffs: `jmp @217` vs `jmp @222 ; mov ecx, [esp + N]`; `mov edx, [esi]` vs `mov [esp + N], ecx ; jmp @222 ; mov edx~` |
| krusty2/vehicle/Bike | `0x0040a520` | 439/2272 | b | 96 code diffs: retail lacks `test al, al`; retail adds `test al, al`; slots differ |
| krusty2/vehicle/Bike | `0x0040aec0` | 527/1186 (masked) | b | 35 code diffs; slots differ |
| krusty2/vehicle/Bike | `0x0040b600` | 1061/1066 | b | slot 46: frame offsets of the one-wheel path's constructor temporaries (sub/half constructor at retail 0x38/0x20, the 0x421cb0 mid at 0x40, the WorldToLocal temporary at 0x48/0x30); the register pair is fixed by `const Vec3&` rear/front locals |
| krusty2/vehicle/Bike | `0x0040ba30` | 55/973 (masked) | b | 64 code diffs: retail lacks `push edi`; frame 0x1c vs 0x18 |
| krusty2/vehicle/Bike | `0x0040c000` | 135/802 (masked) | b | 60 code diffs: retail adds `push ebp ; mov ebp, esp`; retail lacks `fld [A]`; slot order differs |
| krusty2/vehicle/Vehicle | `0x005257a0` | 585/637 (masked) | b | best of all 720 orders of the terrainScale..throttleAxis stores x 6 orders of the weightForce/leanCos/0x4b4 stores; retail still stores 0x4b0/0x4b4 before the leanAxis/throttleAxis pair |
| krusty2/vehicle/Vehicle | `0x005265d0` | 51/471 (masked) | b | 35 code diffs: retail lacks `fld [eax + 8]`; retail adds `mov ebx, [esp + N] ; mov ecx, [esp + N]`; slots differ |
| krusty2/vehicle/Vehicle | `0x005268d0` | 248/1446 (masked) | b | 87 code diffs: retail adds `xor ebx, ebx`; retail lacks `xor ebp, ebp`; slot order differs |
| krusty2/vehicle/Vehicle | `0x00526fa0` | 96/1161 | b | 21 code diffs: retail lacks `xor ecx, ecx`; `mov [esp + N], ebp` vs `mov [esp + N], ebx`; slot order differs |
| krusty2/vehicle/Vehicle | `0x005276d0` | 40/481 (masked) | c | 26 code diffs: `sub esp, 0x18` vs `sub esp, 0x24`; `push edi ; lea edi, [ecx + 0x194] ; jne~` vs `jne @123` |
| krusty2/vehicle/Vehicle | `0x00527a20` | 349/870 | b | 25 code diffs: `jne @99` vs `jne @98 ; mov [esp + N], 0 ; mov edx, [~`; `lea edx, [esi + 0x120]` vs `mov [esi + 0x12c], 0`; slots differ |
| krusty2/vehicle/Vehicle | `0x00527e30` | 31/831 | b | 42 code diffs: `push ebp` vs `push ebx ; mov ebx, ecx ; xor ecx, ecx`; retail adds `mov eax, [ebx + 0x444]`; frame 0x28 vs 0x2c |
| krusty2/vehicle/Vehicle | `0x005281b0` | 40/479 (masked) | b | 37 code diffs: retail adds `mov esi, [esp + N]`; `mov edi, [esp + N] ; mov esi, ecx` vs `mov edi, ecx`; slots differ |
| krusty2/vehicle/Vehicle | `0x005286a0` | 263/782 | b | slot 56: frame 0x3c matches; retail shares the ground-normal output with the projection and n with the side vector; register roles |
| krusty2/vehicle/Vehicle | `0x0052a6a0` | 19/331 (masked) | b | 22 code diffs: `push ecx ; push ebx` vs `sub esp, 8`; retail lacks `fld [edi + 0xe0]`; frame 0x0 vs 0x8 |
| krusty2/vehicle/Vehicle | `0x0052a940` | 304/3018 (masked) | b | 120 code diffs: `test eax, eax ; mov [esp + N], eax ; jn~` vs `mov ebp, eax ; xor ebx, ebx ; cmp ebp, ~`; `push 0` vs `push ebx`; frame 0xdc vs 0x104, slot order differs |
| net/Net | `0x004ab960` | 149/1072 | b | 51 code diffs: `test esi, esi ; jge @54` vs `cmp esi, ebx ; jge @51`; retail adds `lea edx, [esp + N]`; slots differ |
| net/Net | `0x004aced0` | 361/370 | c | ring slot addressed through a byte-offset helper (fixes every SIB order); retail loads `from` before `lea ebx, [edx + edi]` for the system-message pointer, VC6 forms the pointer first (`mov ebx, edi ; add ebx, ecx`) and pushes system[2] from edx instead of eax |
| net/NetThread | `0x004af8e0` | 234/338 | c | 19 code diffs: retail adds `push ebx`; retail adds `mov eax, [A]` |
| net/NetThread | `0x004afa90` | 698/1251 | b | 14 code diffs: `jne @388` vs `jne A`; `jne @374` vs `jne A`; slots differ |
| parser/Parser | `0x004b87f0` | 33/406 | b | prologue: retail pushes ebx/ebp/esi/edi before the null test, VC6 here pushes ebx/ebp after the early return (samples/parser/ParserNearMisses.cpp) |
| physics/bikeai/BikeAI | `0x0040e510` | 1133/1944 | b | 53 code diffs: retail adds `mov esi, [esp + N] ; test esi, esi`; `mov edi, [esp + N] ; test edi, edi ; je~` vs `je @582`; slot order differs |
| physics/bikeai/BikeAI | `0x00415640` | 6059/6112 | b | schedule and frame size match (squared distance and k dot grouped `z + ((x) + y)`); the one-use temporaries above +0x100 sit in other slots and the start-up zero vectors are stored x-first (retail z-first); the retail extent includes one nop of padding (candidate 6111 bytes) |
| physics/bikeai/BikeAI | `0x0040eca0` | 1288/17345 | b | both 0x40d200 calls now pass 0.05f as the sixth argument (retail 0x3d4ccccd); 3751/3966 normalised insns align; register/stack layout left |
| physics/bikeai/BikeAI | `0x00413200` | 659/4122 | b | KrustyBike member: frame 0x74 exact but slot 11's out-vectors at other offsets; operand order of the angular-velocity cross products; ground-branch lean clamp keeps the value on the FPU stack |
| physics/bikeai/BikeAI | `0x00414370` | 394/4678 | b | KrustyBike member: frame 0x74 vs 0x78 (retail spills the difficulty, candidate keeps it in edi); kind 1/5 gate-distance test placed after the single-point and no-track paths |
| physics/bvh/BoundingBoxTreeBuild | `0x0042c260` | 447/1627 | b | 1 code diffs: retail lacks `mov edx, [eax + 0x27c] ; mov eax, [eax ~`; slots differ |
| physics/bvh/BoundingBoxTreeQuery | `0x00425900` | 718/4237 | b | first draft: inline budget differs from the fifth normalisation on; radius sums accumulated in another x87 order |
| physics/bvh/BoundingBoxTreeQuery | `0x00426be0` | 1673/2572 | b | cross products match; min/max of the projections (retail keeps p2 on the x87 stack and stores p1 twice), one operand pair of a vertex transform and the frame (0x160 vs 0x164) |
| physics/bvh/BoundingBoxTreeQuery | `0x00427c10` | 349/1110 | b | natural operator form: retail's helper call sequence; retail keeps the constant 1 in ebp and copies segment[1] at entry, so registers and slots differ |
| physics/bvh/BoundingBoxTreeQuery | `0x004280e0` | 453/2228 | b | inline BoxOverlap copy: one constructor still expanded and the recursion tails cross-jumped differently |
| physics/bvh/BoundingBoxTreeQuery | `0x00428db0` | 773/789 | c | frame and slots match (loop output in its own local, `grown` after the last rotation, z, y, x transform sums); the first two products of the transform's x and z rows load the matrix element first |
| physics/bvh/BoundingBoxTreeQuery | `0x00429570` | 246/831 | b | 17 code diffs: `jne @82 ; xor eax, eax ; pop edi` vs `je @272`; slots differ |
| physics/bvh/BoundingBoxTreeQuery | `0x004298c0` | 532/1418 | b | matches through the distance-to-plane test; operator-result temporaries in other slots and a shared `return 1` afterwards |
| physics/bvh/BoundingBoxTreeQuery | `0x0042a640` | 41/1428 | b | retail homes floats in the dead d0/d1 argument slots (frame 0x60 vs 0x80); cross-product operand order of the x term |
| physics/collision/CollisionPoint | `0x0043aa30` | 377/791 (masked) | b | frame and blocks match (samples/physics/collision/CollisionPointNearMisses.cpp); sum * (1/merged) loads y component-first in retail, the squared normal length squares y, x, z (y consumed), one push fstp scheduled later |
| physics/collision/CollisionPoint | `0x0043ad80` | 47/599 (masked) | b | body matches; retail returns for a null first argument before pushing ebx/ebp/esi (VC6 pushes all four first) and homes the loop counter in the dead `points` slot, VC6 in a1's |
| physics/collision/CollisionVectorHelpers | `0x0043ca20` | 61/115 | b | 5 code diffs: retail adds `mov edx, [esp + N]`; retail lacks `mov edx, [esp + N]`; slots differ |
| physics/constraint/ConstraintMethodCollisionModel | `0x0043bdb0` | 1570/2619 | c | ApplyContactImpulse, frame 0xb8 and slot order match; termB (other-body branch) builds its cross/dot in a different x87 register order (fstp st(2), +2 bytes), plus scheduling of three struct copies and the `test edi` of `if (other)` |
| physics/contact/ContactImpulse | `0x005004a0` | 104/927 (masked) | b | 47 code diffs: retail adds `fmul [esi + 4]`; retail lacks `fxch st(1) ; fmul [esi + 4]`; slot order differs |
| physics/contact/ObjectPlacement | `0x004b0df0` | 451/3321 (masked) | b | 123 code diffs: retail adds `mov edi, [esp + N]`; retail lacks `mov edi, [esp + N]`; frame 0x1c4 vs 0x1ac, slot order differs |
| physics/effects/DirtChunkParticleEmitter | `0x004b8df0` | 17/235 (masked) | c | 15 code diffs: retail lacks `sub esp, 8`; `lea edi, [esi + 0x80]` vs `lea ecx, [esi + 0x50] ; mov edx, eax ; ~` |
| physics/effects/DirtChunkParticleEmitter | `0x004b8f40` | 947/958 (masked) | c | 1 code diff: gravStep first component `fld st(0) ; fmul [esp + N]` vs retail `fld [esp + N] ; fmul st(1)` (writing gravStep as `inv * (dt * manager->gravity)` fixed the velStep one; ~400 further variants do not move it) |
| physics/effects/DirtSprayParticleEmitter | `0x004b9310` | 30/235 (masked) | c | 10 code diffs: retail adds `lea ecx, [esi + 0x50] ; mov edx, eax`; retail adds `mov [ecx], edx ; mov [esi], A ; mov [es~` |
| physics/effects/DirtSprayParticleEmitter | `0x004b9460` | 947/958 (masked) | c | 1 code diff: gravStep first component `fld st(0) ; fmul [esp + N]` vs retail `fld [esp + N] ; fmul st(1)` (writing gravStep as `inv * (dt * manager->gravity)` fixed the velStep one; ~400 further variants do not move it) |
| physics/effects/DustParticleEmitter | `0x004b8a00` | 25/122 | c | emitRate fixed to 4000.0f (retail 0x457a0000) and GameObject(int) bound; retail's `lea ecx,[esi+0x50]; mov edx,eax` interleave for previousPosition not reproduced |
| physics/effects/SparkParticleEmitter | `0x004b9830` | 27/227 (masked) | c | 11 code diffs: `lea edi, [esi + 0x84]` vs `lea ecx, [esi + 0x50] ; mov edx, eax ; ~`; retail lacks `mov [esi + 0x50], eax ; mov [esi + 0x54~` |
| physics/effects/SparkParticleEmitter | `0x004b99a0` | 1027/1039 (masked) | c | 1 code diff: gravStep first component `fld st(0) ; fmul [esp + N]` vs retail `fld [esp + N] ; fmul st(1)` (writing gravStep as `inv * (dt * manager->gravity)` fixed the velStep one; ~400 further variants do not move it) |
| physics/effects/SteamParticleEmitter | `0x004b9f40` | 21/164 (masked) | c | 6 code diffs: `lea edi, [esi + 0x74]` vs `lea ecx, [esi + 0x54] ; mov edx, ebx ; ~`; `mov [esi + 0x54], ebx ; mov [esi + 0x58~` vs `mov [ecx + 8], edx` |
| physics/helpers/GraphicsTest | `0x0047bd10` | 818/917 | b | DrawCircle: frame 0xb84 vs 0xb78, the `[eax - 4]` store and the centre-sum operand order; a `Vec3* p = &points[i]` local gave +4 |
| physics/helpers/SoultreeBounds | `0x004fd340` | 618/630 | c | rows _11.._13 written k = 1,3,2,4 and _14 k = 3,1,2,4; the s13 product of _11.._14 loads its operands the other way (retail m3j first) and two _44 products differ (samples/physics/helpers/SoultreeBounds.cpp) |
| physics/helpers/SoultreeBounds | `0x004fe2e0` | 82/1325 | b | frame 0x3c vs retail 0x48 (by-value direction temporaries, centre in ebx/ebp/edx); *have clamps are unconditional selects; retail spends one more inline Vec3 site before h |
| physics/helpers/SoultreeBounds | `0x004fe8a0` | 541/610 | b | frame slot order (retail c, e, w, a) and the hi = w + ext tail |
| physics/helpers/SoultreeMatrix | `0x004fb8c0` | 249/1072 (masked) | b | 13 code diffs: `mov ecx, [esp + N] ; mov edx, [esp + N]~` vs `lea ecx, [esp + N] ; push ecx`; `fld [esp + N] ; mov [esp + N], ecx ; le~` vs `call A`; slot order differs |
| physics/helpers/SoultreeMatrix | `0x004fc050` | 392/1104 (masked) | b | 35 code diffs: retail lacks `mov ebx, ecx`; retail adds `mov ebp, ecx`; slot order differs |
| physics/helpers/SoultreeMatrix | `0x004fca80` | 308/458 (masked) | b | 10 code diffs: `mov [ebx + 4], edx` vs `mov [ebx + 8], eax`; `mov [ebx + 8], eax ; add esp, 0xc` vs `mov eax, [esp + N] ; mov [ebx + 0x18], ~`; slots differ |
| physics/helpers/SoultreeRotate | `0x004fd1f0` | 285/322 (masked) | c | 6 code diffs: `fld st(2) ; fmul [esi + 0xb8] ; fld st(~` vs `fld st(0)`; retail lacks `faddp st(1)` |
| physics/helpers/SoultreeTransform | `0x004fd710` | 203/205 (masked) | c | FPU operand order: one `fld/fmul` pair loads the other operand first (2 bytes) ; x-row, transpose-order, y/z-order and ~10k structural forms (constructor return, reference plus copy, pointer/by-value matrix, out pointer, Swap helper, inline transpose) do not move `v.x * m._11` |
| physics/helpers/SoultreeTransform | `0x004fd7f0` | 269/281 (masked) | b | InvertRigid + TransformPoint form; the y translation sums _32*_43 before _12*_41 and the second swap's integer moves come early; a parenthesised t.y fixes both but sinks r.z's last fadd/fstp (255) |
| physics/krustybike/KrustyBike | `0x0048eea0` | 608/2952 | c | slot 102, frame 0x50 exact; VC6 cross-jumps the head-turn arms differently (retail keeps the + side hold block and the - side retarget tail), `jne call ; jmp skip` vs retail `je skip ; jmp call` in the trick-chain switch, the `+0x153c = 0` else block before the shared advance, and the crash clamp keeps t on the x87 stack where retail spills it to the dt slot |
| physics/krustybike/KrustyBike | `0x0048fa60` | 380/440 (masked) | c | 4 code diffs: retail hoists `mov [esi + 0x11b8], bl ; mov [esi + 0x604], ebx` into the load delay of the +0x1540 kVec3Zero copy where the candidate hoists `+0x7c0/+0x7bc` (pick index = units after the copy - 32) |
| physics/krustybike/KrustyBike | `0x004919a0` | 147/221 (masked) | c | 8 code diffs: retail adds `mov edx, [edx + 0x60c] ; fsub [ecx + 0x~`; retail lacks `fsub [ecx + 0xb8]` |
| physics/krustybike/KrustyBike | `0x00491d10` | 523/1296 | b | prologue, frame 0x20 and lim in ebp match (if/else flag/lim, mode 2/3 copy path breaks); the 2/3 normalise join sits after the invsqrt arm (retail after the zero arm), its query call is not cross-jumped into the shared call, and retail homes the case 0/4 call in case 4 (candidate in case 0); candidate 1300 bytes |
| physics/krustybike/KrustyBike | `0x00492ad0` | 192/2315 | c | 10 code diffs (frame 0x34 exact): record-interval branch `je @end ; jmp @deltas` vs retail `jne @deltas ; jmp @end`, network branch conditional jump vs retail `push 0 ; jmp @end+1`, and the register choice it drags along |
| physics/krustybike/KrustyBike | `0x00493660` | 2005/8545 | c | 528 code diffs (frame 0x58 and slots exact): retail keeps 1 in ebp and 0 in edi from the second SetAxesPtr call on; retail stores `(len+50)*(len+50)` before the AllowWarping test; velocity y/z `fld s ; fmul` order; pose lerp `c - b` recompute; pose bytes in 0x492670/0x492ad0 scale by 255.0f (0x551420), and 100.0f is 0x5505ec |
| physics/krustybike/KrustyBike | `0x00496d10` | 1/1 (masked) | c | 1 code diffs: `jmp A` vs `jmp @12 ; nop ; nop ` |
| physics/motion/Motnctrl | `0x004a5e40` | 375/780 (masked) | b | 24 code diffs: `push edi` vs `push esi`; `xor ebx, ebx ; cmp eax, ebx ; mov [esp ~` vs `xor ebp, ebp ; cmp eax, ebp ; mov [esp ~`; slots differ |
| physics/motion/Motnctrl | `0x004a70c0` | 159/3327 (masked) | b | 733 code diffs: retail calls `CharacterPose::CharacterPose` 2 then 4 times where the candidate calls it 3 then 2; retail calls `operator*`/`operator+` in the first extrapolation loop; frame 0x2dc vs 0x284; slots differ |
| physics/motion/Motnctrl | `0x004a7dc0` | 456/503 (masked) | b | 4 code diffs: `fld [esi + 0x20]` vs `mov [esp + N], ecx`; retail lacks `mov [esp + N], ecx`; slots differ |
| physics/motion/Motnctrl | `0x004a7fd0` | 562/1009 (masked) | c | 37 code diffs: `fld [esi + 8] ; fmul [edi + 4] ; fld [e~` vs `fld [edi + 4] ; fmul [esi + 8] ; fld [e~`; retail adds `fsubp st(1)` |
| physics/motion/Motnctrl | `0x004a8470` | 212/1093 (masked) | c | 43 code diffs: retail adds `push ebp`; retail lacks `push ebp` |
| physics/motion/SteeringControl | `0x00504d30` | 125/206 (masked) | b | 6 code diffs: `fxch st(2) ; fmul st(2)` vs `fld st(0) ; fmul st(3)`; `fmul st(2)` vs `fmul st(1)`; slots differ |
| physics/shadow/D3DIMSoultreeShadow | `0x00446c30` | 730/771 | c | x87 term order of the inlined 4x4 product: one term pair in each of _11, _21, _31, _41 (retail's sum orders differ from slot 27's with the same helper) (samples/physics/shadow/D3DIMSoultreeShadowNearMisses.cpp); 748/771 with 1, 2, 9, 10, 18, 25, 26, 33 or 34 locals at the top; exact only with 5-6 locals inside the shared helper, which break the exact slot 27 |
| physics/shadow/D3DIMSoultreeShadow | `0x00446f40` | 180/1485 | b | retail's inline __asm fld/fistp rounding helper forces an ebp frame (0xe4 locals); the (int) casts call __ftol |
| physics/shadow/ProjectedShadow | `0x004da7b0` | 130/778 (masked) | b | tail and clip rectangle match (844/842 B, 18 aligned diffs); register choice in the sprintf/QueryFlag set-up and the saved render-target fields, and retail keeps a second release-and-return block for the surface-format failures |
| physics/shadow/ProjectedShadow | `0x004db0d0` | 111/1687 (masked) | b | magnitude/normalise/transform helpers as retail (1851/2020 B); frame and slot assignment differ (retail locals 0x14..0x3c higher), mode-1 fov block |
| physics/shadow/ProjectedShadow | `0x004db8c0` | 19/840 (masked) | b | 51 code diffs: retail adds `push ebp ; mov ebp, esp`; `mov [esp + N], ebx` vs `mov [ebp - N], ebx`; slot order differs; inline-asm `fistp` exclusion |
| physics/shadow/ProjectedShadow | `0x004dbd30` | 156/1244 (masked) | b | per-format row loops and the three Duff's-device fills as retail; register assignment of the clip fields and the format-9 constants (retail uses byte ops on field_0x94's low byte) |
| physics/shadow/ProjectedShadow | `0x004dc2b0` | 29/324 (masked) | b | 29 code diffs: retail adds `push ebp ; mov ebp, esp`; retail lacks `push ebp`; frame 0x24 vs 0x34, slot order differs; inline-asm `fistp` exclusion |
| physics/shadow/TerrainShadow | `0x005097d0` | 105/442 (masked) | b | 16 code diffs: `jmp @29` vs `jmp @31 ; mov edx, [esp + N]`; `mov edx, [ecx + 0x54] ; fild [edx + 0x1~` vs `mov [esp + N], edx ; mov eax, [ecx + 0x~`; slots differ |
| physics/shadow/TerrainShadow | `0x00508bc0` | 353/3031 | b | slot 27 footprint: frame 0xdc vs 0xd0, slot order, the cross product's operand order and the corner loops' form; the inline budget needs the per-mode accumulate/finish text repeated (macros) |
| physics/shadow/TerrainShadow | `0x00509aa0` | 86/1548 (masked) | b | 90 code diffs: retail adds `push ebp ; mov ebp, esp`; retail lacks `push ebp`; frame 0x370c vs 0x3728, slot order differs |
| physics/soultree_base/SoultreePhysicsInlines | `0x0040b410` | 275/404 (masked) | b | 18 code diffs: `fld [edi + 0x1b0]` vs `fld [esi + 4]`; `fld [edi + 0x1b4]` vs `fld [esi + 8]`; slots differ |
| physics/suspension/Shock | `0x004fa400` | 458/674 (masked) | b | 20 code diffs: `je @160 ; mov eax, [A]` vs `je @162`; `jmp @180` vs `mov eax, [A] ; mov [esp + N], ecx ; mov~`; slots differ |
| physics/tire/Tire | `0x00513490` | 175/177 (masked) | c | 2 code diffs: `mov [ebp + N], ebx` vs `mov [ebp + N], edi`; `mov [ebp + N], edi` vs `mov [ebp + N], ebx` in the pts[i].origin stores; store/declaration order, chained/comma assignment, member-wise copy, pointer/reference and typedef shifts give no gain |
| physics/tire/Tire | `0x005135f0` | 559/1666 | b | UpdateContactPatch: frame 0x18 matches; retail homes the projected axis in the dead `axis` argument slot, normal through ecx, tail-path copies |
| physics/tire/Tire | `0x00513c70` | 96/805 | b | 24 code diffs: retail adds `push ebx`; retail adds `xor ebx, ebx`; slots differ |
| physics/tire/Tire | `0x00513f90` | 470/479 | b | stack slots: retail keeps `boost` in the dead argument slot and the squared length/`minimum` in the local (reference counts 3 vs 4, VC6_FRAME_LAYOUT fact 9); rewrites of the boost/minimum conditions and the clamp give no gain |
| physics/tire/Tire | `0x00514170` | 276/589 | b | `fld st(0); fcomp` where retail uses `fcom`; x87 operand order of the `dir` scaling |
| physics/tire/Tire | `0x00514550` | 169/3838 (masked) | b | 214 code diffs: `lea edi, [esi + 0x200] ; push edi` vs `lea ebp, [esi + 0x200] ; push ebp`; retail lacks `lea ebp, [esi + 0x20c] ; lea ebx, [esi ~`; frame 0xb8 vs 0xc0 |
| physics/tire/Tire | `0x00515880` | 48/653 (masked) | b | 35 code diffs: retail adds `mov eax, [esp + N] ; push esi ; mov ecx~`; retail lacks `mov ecx, [esp + N]`; slots differ |
| physics/tire/Tire | `0x00515b50` | 132/271 (masked) | c | 9 code diffs: `fld [ecx + 0x180] ; fmul [ecx + 0x58] ;~` vs `fld [ecx + 0x54] ; fmul [ecx + 0x17c] ;~`; `fld [ecx + 0x178] ; fmul [ecx + 0x50]` vs `fld [ecx + 0x58] ; fmul [ecx + 0x180]` |
| physics/tire/Tire | `0x00515c90` | 287/292 | b | true extent 292; the x component of the normalisation scale loads the scalar first (`fld st(0); fmul c.x`), retail the component; cross product, registers and y/z scales match (samples/physics/tire/Tire.cpp) |
| physics/visibility/VisibilityQuadTreeTraversal | `0x0052d610` | 2046/2814 (masked) | c | 137 code diffs: `mov eax, [esp + N] ; mov ecx, [esp + N]` vs `mov eax, [edi + 4]`; retail adds `mov ecx, [eax] ; mov eax, [esp + N] ; p~` |
| race/BikeRace | `0x00417ed0` | 2804/6756 | c | retail stores `z*z` of each vector length to a stack temporary (`fstp [t]; faddp; fadd [t]`); the girl block's temporaries sit 4 to 12 bytes apart |
| race/BikeRace | `0x00419970` | 1386/13322 | b | frame 0xac4 vs 0xab0, slot order differs (register choice follows); retail keeps one more stream destructor tail |
| race/BikeRace | `0x0041eb20` | 1697/1710 | c | retail loads the scene-table index (+0x150) before the table pointer in the scene-object branch (edx/ecx swapped at the branch head) and again after the 0x4dab90 call; 162 parenthesisations of the index copy, table test and entry address give identical bytes |
| race/BikeRace | `0x0041f1d0` | 266/881 | c | retail keeps 0 in edx from the first instruction (`cmp [edi + 0x3430], edx`, `xor edx, edx` after calls) and has three 0x0041eb20 call copies; the objects scan's found path now calls and returns (second copy, size matches) |
| race/BikeRace | `0x004210f0` | 975/2940 | b | inline budget: retail calls the Vector3 constructor in the mode 2/3 tail and expands Scale in mode 4; registers follow |
| race/CarProcedural | `0x0042fd80` | 221/2844 | b | slot 10: frame 0xd0 vs 0xdc (retail shares the wheel loop's axisA with the orientation's `flat`, axisB with nothing), the steering clamp's memory local, component/scalar load order in the `back` normalisation and blend; masked, 781 instructions like retail's |
| race/CarProcedural | `0x00430b10` | 299/843 | b | 45 code diffs: retail adds `mov esi, ecx`; `mov edi, ecx ; mov esi, [edi + 0x6c] ; ~` vs `mov edi, [esi + 0x6c] ; mov ebp, [esi +~`; slots differ |
| race/Krusty3DObjects | `0x0048b100` | 166/2808 | b | the field-of-view conversion keeps retail's two multiplies (0.5 then 0.01745329) through a parenthesised macro; third camera column copied by integer moves in retail (two components kept on the x87 stack here); retail keeps 0 in edi and gives early returns their own epilogues |
| race/Krusty3DObjects | `0x0048ccc0` | 47/1308 | b | 3 code diffs: `je @349` vs `je A`; `jne @349` vs `jne A`; slots differ |
| race/RaceSound | `0x004e4d10` | 563/2038 | c | 10 code diffs: `je @599` vs `je A`; `jne @280` vs `jne A` |
| race/RaceStatus | `0x004e5d00` | 168/519 | c | 29 code diffs: retail adds `mov ebp, [esp + N]`; retail adds `test ebp, ebp` |
| race/Recorder | `0x004e6f80` | 652/2180 | c | case 1 `goto opened` reproduces retail's jmp into case 2's tail; left: case 2's `[0xc8]=1; Leave` suffix still cross-jumps into case 4 (retail keeps it), and `name`/`headerSize` slots swap (0x08/0x10); real extent 2148 |
| race/Recorder | `0x004e7d60` | 272/2448 | b | 99 code diffs: `jne @767 ; mov eax, [esi + 0x88] ; test~` vs `jne @761 ; mov ecx, [esi + 0x88] ; xor ~`; slot order differs |
| race/Wrecker | `0x00531740` | 1606/1624 | c | slots match; the argument push of two loop constructor calls comes one load earlier (`push ecx` before the particle-pointer load and before `along.y`), two registers differ |
| race/Wrecker | `0x005329e0` | 1200/1312 (inside Wrecker.cpp) | a | every instruction equal; slot order only (retail keeps `push` nearest esp, then the y/x extent copies, `p`) |
| render/BackgroundImage | `0x004049d0` | 108/606 (masked) | c | block layout as retail; VC6 caches 0 in edi (`xor edi, edi`, `mov [esp + N], edi`) where retail stores immediates, which moves the region loop's registers |
| render/CacheTexture | `0x005102d0` | 426/529 (masked) | b | 13 code diffs; slots differ |
| render/CubeDraw | `0x0043dc60` | 121/1064 | b | 53 code diffs: `xor ebp, ebp` vs `xor edx, edx`; `mov [esp + N], ebp ; mov edi, 0x100 ; o~` vs `mov [esp + N], edx ; mov ebx, 0x100 ; o~`; slots differ |
| render/CubeDraw | `0x0043e0b0` | 30/352 | c | 22 code diffs: retail adds `push ebx`; retail adds `mov ecx, [esp + N]` |
| render/CubeDraw | `0x0043e330` | 90/399 | b | 15 code diffs: `mov ecx, [edi + 0x18] ; mov eax, [ecx +~` vs `mov eax, [edi + 0x18] ; mov ecx, [eax +~`; `mov eax, [ecx + 8] ; mov edx, [eax + 0x~` vs `mov ecx, [eax + 8] ; mov edx, [ecx + 0x~`; slots differ |
| render/D3DIMSoulTree | `0x00440810` | 135/1349 | b | 50 code diffs: `jle @345 ; xor esi, esi` vs `jle @340`; `mov [esp + N], esi ; mov eax, [edi + 0x~` vs `mov edx, [edi + 0x1c] ; mov esi, ebx ; ~`; frame 0x84 vs 0x80, slot order differs |
| render/D3DIMSoulTree | `0x00440f30` | 241/763 (masked) | b | DrawCurrentLod: stale static-local and `$E` bindings fixed (ret stubs 0x442f60/0x442f40), all relocations bound; 1626/1893 normalised insns align; register allocation left |
| render/D3DIMSoulTree | `0x00443aa0` | 799/832 | c | 11 code diffs: retail lacks `fmul [eax + 0x10] ; fld [esp + N]`; retail lacks `faddp st(1)` |
| render/D3DIMSoulTree | `0x00444140` | 610/764 | b | retail sums low+high low-first in all components and forms +0x164 in edx after the +0x158 stores; VC6 sums y/z high-first and keeps `add ebp, 0x164`; zero-reset, declaration and Raise/Lower orders are worse |
| render/D3DIMSoulTree | `0x00444560` | 355/1231 | b | frame 0x2c matches; instruction stream equals retail up to a register rotation (source ebx/ebp, `from` edi/ebx, `to` ebp/edi), 2 bytes shorter from the [ebp] disp8 |
| render/DebugOverlay | `0x00447a00` | 79/989 | b | frame 0x26c matches; VC6 pushes ebp in the prologue and keeps `c` in ebp at the row head (retail: eax from its slot, ebp pushed only around the GDI section) |
| render/DebugOverlay | `0x00447de0` | 66/174 | c | 13 code diffs: retail adds `push ebx`; retail lacks `push edi` |
| render/DirectoryList | `0x0044a600` | 74/109 | c | 6 code diffs: `jne @31 ; mov al, byte ptr [esp + N]` vs `jne @32 ; mov eax, [esp + N]` |
| render/GR_BitString | `0x004239c0` | 343/420 | b | 14 code diffs: retail lacks `mov ebx, ecx`; retail adds `mov ebp, ecx`; slots differ |
| render/GR_BitString | `0x00423b70` | 93/442 | b | 16 code diffs: retail adds `add esi, 4`; `or ecx, eax` vs `or eax, ecx ; mov [edx - 4], eax`; slots differ |
| render/Grid1 | `0x0047caa0` | 104/1740 | b | slot 3 texture stage switch; frame 0x14 vs retail 0x10, 93 code diffs (samples/render/Grid1NearMisses.cpp) |
| render/Grid1 | `0x0047d470` | 313/786 | b | `rest` before `line`; line pointer still spilled (frame 0x838 vs 0x834), first/row in ebx/edx where retail has edx/eax |
| render/Grid1 | `0x0047d780` | 216/1008 | b | 50 code diffs: `je @321 ; mov esi, [esp + N]` vs `je @315`; `mov eax, [esi]` vs `mov edi, [esp + N]`; slot order differs |
| render/GridNode | `0x00484d70` | 81/90 | c | 3 code diffs: `mov ecx, [eax*4 + A] ; add ecx, ebx ; m~` vs `mov eax, [eax*4 + A] ; add eax, ebx ; m~`; retail adds `xor ecx, ecx ; mov cx, word ptr [eax + ~` ; ~110 loop, index-local, final-test and parenthesis spellings unchanged |
| render/GridNode | `0x00483d40` | 96/4189 | b | slot 1: ebp frame and four bare `fistp [eax]` from an `__asm` rounding helper (`__ftol` still used elsewhere), so unreachable under the no-inline-asm rule; `/Oy-` changes nothing else |
| render/Griddraw | `0x0047de90` | 977/1083 (masked) | c | 30 code diffs: retail lacks `mov esi, [esp + N]`; `mov edi, [ebp*N + N]` vs `mov edi, [esp + N]` |
| render/Griddraw | `0x0047e600` | 1605/1617 (masked) | c | frame matches; the extent.x/extent.y products load the cell size first (`fld [edx + 0x40] ; fmul [ecx + 0x164]`), retail the minimum ; the order follows which fields are read (+0x168 gives min-first, +0x164/+0x18 cell-first), not source order or parentheses (432 forms) |
| render/Griddraw | `0x0047f210` | 361/543 (masked) | b | 29 code diffs: retail adds `xor esi, esi`; retail lacks `xor edi, edi`; slot order differs |
| render/Griddraw | `0x00480ad0` | 395/438 | b | retail keeps the zero vertexStart in eax and compares ranges/block against it (`cmp esi,eax`, `indexStart = eax`), ranges in esi and data in edi; VC6 uses `test` and swaps esi/edi; ~20 init/type/declaration forms unchanged or worse |
| render/Griddraw | `0x00481300` | 121/216 (masked) | c | child index: `lea ecx, [edi + ebp] ; add ecx, table[quad]` vs `mov edx, table[quad] ; add edx, edi ; add edx, ebp` |
| render/Griddraw | `0x004815e0` | 64/1083 | b | retail keeps both axis deltas on the x87 stack across the near/far branch (`fcom`, spilled in the arms) and pushes edi after the coarse return; VC6 stores them, which shifts the frame (the 65535 clamps now branch as retail) |
| render/Griddraw | `0x00481de0` | 133/1616 (masked) | b | 85 code diffs: retail adds `mov ebx, [esp + N]`; retail adds `mov ebp, [esp + N]`; slots differ |
| render/Griddraw | `0x00483200` | 45/727 (masked) | b | 53 code diffs: retail adds `mov ebp, [esp + N]`; retail lacks `mov ebp, [esp + N]`; slots differ |
| render/ManagedTextureGroup | `0x0050c960` | 17/227 (masked) | c | 5 code diffs: `sub esp, 0x80` vs `sub esp, 0x78`; `je @167` vs `je A` |
| render/ManagedTextureGroup | `0x0050dad0` | 1887/5022 | b | repack with partial blits, 87 code diffs (samples/render/ManagedTextureGroupNearMisses.cpp) |
| render/ManagedTextureGroup | `0x0050ef70` | 16/231 (masked) | b | 19 code diffs: retail lacks `push ebx`; retail lacks `push esi`; slots differ |
| render/MatrixUtil | `0x004a1a50` | 171/175 | c | 1 code diffs: `fld [ecx + 4] ; fmul [eax + 0x28]` vs `fld [eax + 0x28] ; fmul [ecx + 4]` ; operator()/operator[] accessors give no gain either |
| render/MorphBastardModifier | `0x004a33b0` | 1938/2046 | b | spill homes of the loop scalars and the two inverse temporaries; operand order inside the channel inverse's products |
| render/MorphBastardModifier | `0x004a3c80` | 74/3663 | b | 4 code diffs: `push ebp` vs `mov ebx, [esp + N]`; `mov ecx, [esp + N]` vs `mov ecx, [ebx + 0x44]`; frame 0x1b4 vs 0x1fc |
| render/MorphBastardModifier | `0x004a4c60` | 107/1570 | c | 19 code diffs: retail lacks `xor edi, edi ; test eax, eax ; mov [esp~`; retail adds `cmp eax, esi` |
| render/Overlay | `0x004b6710` | 53/365 | b | 23 code diffs: retail adds `mov eax, [esp + N]`; `mov esi, [esp + N]` vs `mov ebx, [eax] ; mov edx, [eax + 4]`; slots differ |
| render/PCTextureMap | `0x004c69c0` | 429/894 (masked) | c | 31 code diffs: `jne @390` vs `jne A`; `jne @120` vs `jne @116 ; lea edx, [esp + N]` |
| render/PCTextureMap | `0x004c71c0` | 286/538 (masked) | c | 27 code diffs: `mov ebp, 1` vs `mov ebx, 1`; `and eax, ebp` vs `and eax, ebx` |
| render/PCTextureMap | `0x004c7b40` | 247/767 | b | 35 code diffs: retail lacks `mov edi, [eax]`; slots differ |
| render/PCVideoCard | `0x004ca520` | 26/124 | c | 13 code diffs: `mov ebx, [esp + N]` vs `push ebp`; retail adds `mov ebp, [ecx + 0x74]` |
| render/PCVideoCard | `0x004ca5a0` | 50/490 | c | 15 code diffs: retail adds `push ebp ; mov ebp, [esp + N]`; retail lacks `mov [eax], ebx ; mov eax, [esp + N]` |
| render/PCVideoCard | `0x004cab00` | 144/1113 | b | retail keeps the mode index in esi shared with the created target (two -3 stores), re-tests `render` after the found path and places the Slot8-success path after the epilogue |
| render/PCVideoCard | `0x004cb330` | 35/628 | b | 46 code diffs: `push ebx ; mov ebx, [esp + N]` vs `push ebp ; mov ebp, [esp + N]`; `mov eax, [ebx]` vs `mov eax, [ebp + N]`; slots differ |
| render/Pixtrans | `0x004cdf10` | 46/636 | b | 33 code diffs: retail adds `mov ecx, [esp + N] ; mov [esp + N], eax~`; retail lacks `mov [esp + N], eax ; mov eax, [esp + N]~`; slot order differs |
| render/Pixtrans | `0x004ce420` | 55/456 | b | 23 code diffs: `sub esp, 8` vs `sub esp, N ; mov edx, [esp + N]`; `mov eax, [esp + N] ; test eax, eax` vs `test edx, edx`; frame 0x8 vs 0x10 |
| render/Pixtrans | `0x004cee30` | 45/1133 | b | candidate 1093 B, 314 code diffs: `push ebp` in the prologue vs after the three palette checks; `sourceRow` homed in `source`'s slot shifts every slot; fast-loop channel sum order |
| render/Pixtrans | `0x004cf2a0` | 242/1735 | b | draft: format, pixel size and palette in ebx/esi/edi where retail uses edi/ebx/esi; frame 0x50 vs 0x54 |
| render/ResourceManager | `0x004e9030` | 100/798 | b | 5 code diffs: `jne @35` vs `mov [esp + N], ebx ; jne @36`; `je @250` vs `je A`; slots differ |
| render/TextService | `0x0050ade0` | 117/125 | c | 1 code diffs: `mov eax, [ebp + N] ; mov edx, edi ; mov~` vs `mov ecx, [ebp + N] ; mov eax, edi ; mov~` |
| render/TextService | `0x0050b080` | 477/890 | b | 34 code diffs: `mov edx, [esp + N]` vs `xor edx, edx ; not ecx`; `not ecx` vs `dec ecx`; slot order differs |
| render/TextService | `0x0050b400` | 354/758 | b | 33 code diffs: `mov edx, [esp + N]` vs `xor edx, edx ; not ecx`; `not ecx` vs `dec ecx`; slot order differs |
| render/TextService | `0x0050b700` | 354/758 | b | 33 code diffs: `mov edx, [esp + N]` vs `xor edx, edx ; not ecx`; `not ecx` vs `dec ecx`; slot order differs |
| render/TextService | `0x0050ba00` | 671/802 | b | 10 code diffs: `jbe @192` vs `jbe A`; `je @185` vs `je A`; slots differ |
| render/Tgafile | `0x00512990` | 354/982 (masked) | c | 49 code diffs |
| track/SceneManager | `0x004ecd60` | 2047/4536 | b | 11 code diffs: `je @1189` vs `je A`; `je @1189` vs `je A`; slots differ |
| track/SceneManager | `0x004edfe0` | 3468/5346 | b | 34 code diffs: `je @1459` vs `je A`; `je @1459` vs `je A`; slot order differs |
| track/Track | `0x00515ed0` | 117/2400 (masked) | b | draft: frame 0x64 vs retail 0x48 and retail keeps `this` in ebp, so every frame offset differs; dispatch, calls and arithmetic follow retail |
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
| ui/GameUi | `0x004738a0` | 206/975 | b | 53 code diffs: `mov edi, [esi + 0x1bc] ; mov ebx, eax` vs `mov ecx, [esi + 0x1bc] ; lea ebx, [esi ~`; retail lacks `lea ebp, [esi + 0x1bc]`; frame 0x38 vs 0x40 |
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
| ui/ProCircuitProcs | `0x004d9cd0` | 48/755 | c | 37 code diffs: retail adds `mov eax, [A]`; retail adds `mov ebp, [eax + 0x3444]` |
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
