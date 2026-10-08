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

Counts: a 0, b 199, c 168, total 367. The three class (a) functions of the
first scan (Cube `0x0043d230`, GUIManager::SetUp `0x004853b0`,
VisibilityClipper::SphereInFrustum `0x0052fbb0`) are exact: their slots were
homes in dead argument slots, which VC6 hands out in the order of the
arguments' first use in the source (docs/VC6_FRAME_LAYOUT.md, fact 9).

| TU | VA | score | class | what differs |
|---|---|---|---|---|
| audio/PCAudio | `0x004bb890` | 35/272 (masked) | c | 11 code diffs: `je @65` vs `jne @23 ; mov ecx, [esp + N] ; mov fs:[~`; `je @65` vs `jne @51 ; xor eax, eax ; mov ecx, [esp ~` |
| audio/PCAudio | `0x004bc320` | 188/285 (masked) | c | 16 code diffs: `mov edi, ecx` vs `mov ebp, ecx`; `mov ebp, [esp + N]` vs `mov edi, [esp + N]` |
| audio/PCAudio | `0x004bc4c0` | 192/264 (masked) | c | 23 code diffs: retail adds `mov ebx, [esp + N]`; retail lacks `mov ebp, [esp + N]` |
| audio/PCAudio | `0x004bc6b0` | 47/580 (masked) | c | 54 code diffs: retail lacks `mov eax, fs:[X] ; push -1 ; push A`; retail lacks `mov fs:[X], esp` |
| audio/PCAudio | `0x004bcb30` | 16/148 (masked) | c | 12 code diffs |
| audio/PCAudio | `0x004bcbe0` | 42/177 (masked) | c | 12 code diffs |
| audio/PCAudio | `0x004bcca0` | 106/142 (masked) | c | 11 code diffs |
| audio/PCAudio | `0x004bd0c0` | 110/314 (masked) | c | 16 code diffs |
| audio/PCAudio | `0x004bd4b0` | 65/124 (masked) | c | 11 code diffs: `mov eax, [esi + 0x1ec]` vs `mov ecx, [esi + 0x1ec] ; mov ax, 1 ; pu~`; retail lacks `mov cx, 1` |
| audio/PCAudio | `0x004bdc00` | 79/407 (masked) | b | 33 code diffs: `sub esp, 8` vs `sub esp, N`; retail lacks `mov ebx, [esp + N]`; frame 0x8 vs 0xc |
| audio/PCAudio | `0x004be910` | 108/158 | c | 6 code diffs: retail adds `mov [esp + N], edx`; retail adds `mov [esp + N], edx` |
| camera/FollowCamera | `0x00462ee0` | 418/564 | c | 12 code diffs: retail lacks `mov byte ptr [esi + 0x274], bl ; mov [e~`; retail adds `mov [edx + 8], eax` |
| camera/FollowCamera | `0x00465000` | 198/210 | c | 2 code diffs: retail lacks `mov byte ptr [esi + 0x277], 0`; retail adds `mov byte ptr [esi + 0x277], al` |
| camera/FollowCamera | `0x004650e0` | 262/943 | b | 3 code diffs: `je @266 ; jmp @261` vs `je A ; jmp A`; `je @79` vs `je A`; frame 0x180 vs 0x18c |
| camera/FollowCamera | `0x004654e0` | 358/576 | b | 6 code diffs: retail adds `fsubp st(1)`; retail lacks `fsubp st(1)`; slots differ |
| camera/FollowCamera | `0x00465720` | 931/1267 | c | 4 code diffs: `jne @301` vs `jne A`; retail adds `fld [esp + N]` |
| camera/FollowCamera | `0x00465c20` | 3659/3672 | b | 4 code diffs: retail adds `lea ecx, [esp + N]`; retail lacks `lea ecx, [esp + N]`; slots differ |
| camera/KrustyBikeCamera | `0x00497e20` | 337/376 | c | 10 code diffs: `mov ecx, [A]` vs `mov edx, [A]`; retail lacks `lea eax, [esp + N] ; mov edx, [ecx + 0x~` |
| camera/VehicleCamera | `0x0052cc80` | 553/573 | c | 4 code diffs: `mov byte ptr [esi + 0x277], 0` vs `xor al, al ; mov byte ptr [esi + 0x277]~` |
| control/ControlInterface | `0x004bf4f0` | 43/124 | c | 5 code diffs: `test ecx, ecx ; je @38 ; mov edx, [esp ~` vs `jmp @5` |
| control/ControlInterface | `0x004bf6a0` | 438/726 (masked) | c | 58 code diffs: `jne @158` vs `jne @165 ; lea eax, [esp + N]`; `jmp @169 ; lea ecx, [esp + N]` vs `push eax ; jmp @178` |
| ecosystem/EcoSystem | `0x00456050` | 163/1522 | b | 80 code diffs: retail adds `mov ebx, ecx ; mov cl, byte ptr [ebx + ~`; `mov ebp, ecx ; mov ebx, [esp + N] ; mov~` vs `mov ebp, [esp + N] ; lea eax, [ebx + 0x~`; frame 0x1b8 vs 0x1b4, slot order differs |
| ecosystem/EcoSystem | `0x00456890` | 356/369 | c | 2 code diffs: retail lacks `fmul [eax + 0x5a8]`; retail adds `fmul [eax + 0x5a8]` |
| ecosystem/EcoSystem | `0x004570a0` | 103/386 | b | retail keeps the definition-table entry in `edi` across the position conversions (`mov edi, [eax + ecx*4 + 0x58]`); register choice in the shape store blocks |
| ecosystem/EcoSystem | `0x00457480` | 371/2646 | c | 59 code diffs: `mov [esp + N], eax ; jmp @74` vs `mov esi, eax ; mov [esp + N], esi ; jmp~` |
| ecosystem/EcoSystem | `0x00457ed0` | 1105/1153 | b | 4 code diffs: retail adds `mov edx, [edi + 0x20]`; retail adds `fmul [A]`; slots differ |
| ecosystem/EcoSystem | `0x00458da0` | 457/461 | c | 4 code diffs: `mov cl, byte ptr [esi + eax + 0x14]` vs `mov cl, byte ptr [eax + esi + 0x14]`; `lea eax, [esi + edx]` vs `lea eax, [edx + esi]` |
| ecosystem/EcoSystem | `0x004598d0` | 445/455 | c | 1 code diff: `mov ecx, [0x68aba4]` sits between the last ComputeCode argument's `fsub` and `fstp` in retail |
| ecosystem/EcoSystem | `0x00459ce0` | 323/3300 | b | 112 code diffs: retail adds `xor edi, edi`; retail adds `mov [esp + N], edi`; slot order differs |
| ecosystem/EcoSystem | `0x0045b060` | 195/3919 | c | 5 code diffs: `sub esp, 0xe0` vs `sub esp, 0xf0`; `add esp, 0xe0` vs `add esp, 0xf0` |
| game/DeviceSetup | `0x00448560` | 148/1006 | b | 35 code diffs: retail adds `push edi`; `push edi ; xor ebp, ebp` vs `xor edi, edi`; frame 0x11c vs 0x118 |
| game/EventManager | `0x0045d480` | 331/3715 (masked) | b | 85 code diffs: `cmp [edi + 0x44], eax ; je @58 ; jmp @8~` vs `mov edx, [edi + 0x44] ; xor ecx, ecx ; ~`; frame 0x478 vs 0x460, slot order differs |
| game/EventManager | `0x0045e710` | 322/379 (masked) | c | 10 code diffs: `mov edx, [ecx + 0xc4c] ; mov [eax + 0x5~` vs `mov ecx, [ecx + 0xc4c] ; mov [eax + 0x5~`; `mov eax, [eax + 0x56c]` vs `mov edx, [eax + 0x56c]` |
| game/Game | `0x00467b70` | 406/813 | b | 11 code diffs: `jne @128 ; pop edi ; pop esi` vs `je @227`; slot order differs |
| game/PCGame | `0x004c0d10` | 198/1772 | c | 43 code diffs: retail adds `xor ebp, ebp`; retail lacks `xor ebx, ebx` |
| game/QuarryStuntEvent | `0x004e0560` | 22/331 | b | 30 code diffs: `sub esp, N` vs `push ecx`; retail lacks `xor ebx, ebx`; frame 0xc vs 0x0 |
| game/QuarryStuntEvent | `0x004e14a0` | 166/1760 | b | 9 code diffs: `mov edx, [A] ; test edx, edx` vs `mov ecx, [A] ; test ecx, ecx`; `mov eax, [ecx + 0x26c0] ; lea edx, [eax~` vs `mov eax, [eax + 0x38] ; mov ecx, [eax +~`; slots differ |
| game/QuarryStuntEventLoader | `0x004de590` | 210/8005 | b | 96 code diffs; slot order differs |
| game/UiInfo | `0x00522440` | 362/427 | c | 6 code diffs: `mov eax, 1` vs `mov ecx, 1 ; lea eax, [ebp + N]`; `mov [ebp + N], eax ; mov [ebp + N], eax~` vs `mov [ebp + N], ecx ; mov [ebp + N], ecx~` |
| game/UiInfo | `0x005238f0` | 189/379 | c | 10 code diffs: retail lacks `mov edx, eax`; `test edx, edx ; jne @118` vs `test eax, eax ; jne @97` |
| gameui/MediaControl | `0x004a2560` | 53/656 | b | 29 code diffs: retail adds `push ebp`; slots differ |
| inputdevice/PCJoystickDevice | `0x004c3100` | 102/1449 (masked) | b | 90 code diffs: retail lacks `push ebp`; retail lacks `xor ebp, ebp`; slots differ |
| inputdevice/PCJoystickDevice | `0x004c3790` | 345/417 (masked) | b | 7 code diffs: `mov eax, [esi + 0x260] ; mov ecx, [A] ;~` vs `mov ecx, [esi + 0x260] ; mov edx, [A] ;~`; `mov ecx, [ecx + 0x14]` vs `mov ecx, [edx + 0x14]`; slots differ |
| krusty2/broadphase/Quadtree | `0x004dcac0` | 20/562 (masked) | b | 37 code diffs: retail adds `sub esp, N`; `sub esp, 0x30` vs `mov [esp + N], ecx`; frame 0x0 vs 0x34, slot order differs |
| krusty2/broadphase/Quadtree | `0x004dcf20` | 35/309 (masked) | b | 20 code diffs: `push esi ; mov esi, [esp + N] ; cmp esi~` vs `push ebp ; mov ebp, ecx ; mov ecx, [esp~`; `dec eax ; mov ebx, esi ; mov [ecx + 0x8~` vs `mov ebx, [ebp + N] ; dec ebx ; mov eax,~`; slot order differs |
| krusty2/broadphase/Quadtree | `0x004dd600` | 316/318 (masked) | c | 2 code diffs |
| krusty2/broadphase/Terrain | `0x00506e90` | 850/1502 (masked) | b | 44 code diffs: `je @308` vs `je @282 ; fld [esp + N] ; push ecx`; retail adds `mov ecx, [esp + N]`; slots differ |
| krusty2/broadphase/Terrain | `0x00507c10` | 71/2731 (masked) | b | 128 code diffs: retail adds `push ebp ; mov ebp, esp`; retail lacks `push ebp`; frame 0x114 vs 0x108, slot order differs |
| krusty2/collision/CollisionObject | `0x00431df0` | 19/85 | c | 2 code diffs: `mov eax, [esi + 0x14] ; add eax, ebx` vs `mov edx, [esi + 0x14] ; mov eax, ebx ; ~` |
| krusty2/collision/CollisionObject | `0x00432260` | 441/458 | b | 2 code diffs: retail adds `mov [esp + N], eax`; retail lacks `mov [esp + N], eax`; slots differ |
| krusty2/collision/CollisionObject | `0x004334c0` | 1046/1066 (masked) | b | 11 code diffs: `call A` vs `call @0`; `fmul [edi + 4]` vs `fmul [edi + 0x24]`; slots differ |
| krusty2/collision/CollisionObject | `0x00434540` | 1054/1068 | b | 8 code diffs: retail adds `fld [eax + 0x38] ; fmul [eax + 4]`; retail lacks `fld [eax + 0x38] ; fmul [eax + 4]`; slots differ |
| krusty2/collision/CollisionObject | `0x00434bb0` | 163/305 | b | 10 code diffs: `fld [eax - 4]` vs `fld [eax - 8]`; `fld [eax - 0xc]` vs `fld [eax - 0x10]`; slots differ |
| krusty2/collision/CollisionObject | `0x00434cf0` | 147/448 | c | 16 code diffs: `fld [eax - 4]` vs `fld [eax - 8]`; `fld [eax - 0xc]` vs `fld [eax - 0x10]` |
| krusty2/collision/CollisionObject | `0x00436100` | 31/804 | c | 35 code diffs: `sub esp, 0x4c` vs `sub esp, 0xac`; `mov eax, [ecx + 0x54]` vs `mov ecx, [ecx + 0x54] ; lea edi, [esp +~` |
| krusty2/collision/CollisionObject | `0x00436720` | 66/790 (masked) | c | 52 code diffs: `sub esp, 0x18` vs `sub esp, 0xc`; `add esp, 0x18` vs `add esp, 0xc` |
| krusty2/collision/CollisionObject | `0x00436af0` | 33/812 (masked) | c | 16 code diffs: retail lacks `mov eax, [esp + N]`; `lea esi, [eax + 0x48] ; jle @48` vs `mov edi, [esp + N] ; jle @47` |
| krusty2/collision/CollisionObject | `0x00436e50` | 103/1087 (masked) | b | 56 code diffs: retail adds `mov ebx, [esp + N]`; retail lacks `mov ebp, [esp + N]`; frame 0x88 vs 0x120, slot order differs |
| krusty2/collision/CollisionObject | `0x004376f0` | 167/619 (masked) | b | 14 code diffs: retail adds `mov [esp + N], 0`; retail lacks `xor edi, edi ; mov [esp + N], edi`; slots differ |
| krusty2/collision/CollisionObject | `0x00437c20` | 177/623 (masked) | b | 20 code diffs: `add esp, 0x54` vs `add esp, 0x60`; retail adds `mov [esp + N], 0`; frame 0x54 vs 0x60 |
| krusty2/collision/CollisionObject | `0x00437ef0` | 66/704 (masked) | b | 26 code diffs: retail adds `mov [esp + N], 0`; retail adds `lea ecx, [ebx + 0x18]`; frame 0x54 vs 0x7c |
| krusty2/collision/CollisionObject | `0x00438280` | 171/619 (masked) | b | 24 code diffs: `add esp, 0x54` vs `add esp, 0x60`; retail adds `mov [esp + N], 0`; frame 0x54 vs 0x60 |
| krusty2/collision/CollisionObject | `0x00438550` | 55/677 (masked) | b | 28 code diffs: retail adds `mov [esp + N], 0`; `fmul st(3)` vs `fmul [esp + N]`; frame 0x54 vs 0x7c |
| krusty2/collision/CollisionObject | `0x00438c90` | 61/475 | c | 31 code diffs: `mov ebx, [ecx + 0x54]` vs `xor ebx, ebx`; `xor ebp, ebp` vs `mov ebp, [ecx + 0x54] ; sub edx, ebx` |
| krusty2/collision/CollisionObject | `0x004392c0` | 303/311 | c | 6 code diffs: `mov eax, [esp + N] ; mov ecx, [eax + 0x~` vs `mov ecx, [esp + N] ; mov eax, [ecx + 0x~`; `dec ecx` vs `dec eax` |
| krusty2/collision/CollisionObject | `0x00434eb0` | 382/1820 | b | UpdateModelBounds: same call/inline pattern; frame 0x12c vs 0x134, loop counter ebp vs esi, the rotated x component retail keeps on the x87 stack |
| krusty2/collision/CollisionObject | `0x00435830` | 157/1776 | b | SetTransform: the hull case's inline 4x4 product emits its terms as 1,3,2,4 (retail 3,2,1,4 with the local matrix loaded first); frame 0x30 both |
| krusty2/collision/CollisionObject | `0x00439820` | 122/1596 | b | SegmentTouchesObject: operator temporaries (frame 0xc0 vs 0xa8), register choices |
| krusty2/soultree/SoulTreePhysics | `0x00500220` | 270/619 (masked) | b | 24 code diffs: `je @201 ; fld [edi + 8]` vs `je @197`; `fmul [esi + 4]` vs `fld [esi + 4] ; fmul [edi + 8]`; slots differ |
| krusty2/soultree/SoulTreePhysics | `0x005013d0` | 400/457 (masked) | b | 8 code diffs: `fld [ecx + 8] ; fmul [eax + 4]` vs `fld [eax + 4] ; fmul [ecx + 8]`; `fld [ecx + 8] ; fmul [eax]` vs `fld [eax] ; fmul [ecx + 8]`; slots differ |
| krusty2/soultree/SoulTreePhysics | `0x00501600` | 651/925 | b | 18 code diffs: `jmp @239 ; fld [esp + N]` vs `jmp @242`; slots differ |
| krusty2/soultree/SoulTreePhysics | `0x00501e90` | 160/471 (masked) | b | 20 code diffs: `jne @56` vs `jne @55 ; mov [esp + N], 0 ; mov edx, [~`; slots differ |
| krusty2/soultree/SoulTreePhysics | `0x00502330` | 231/239 | b | 2 code diffs: retail adds `fld [esp + N] ; fmul st(1)`; retail lacks `fld st(0) ; fmul [esp + N]`; slots differ |
| krusty2/soultree/SoulTreePhysics | `0x00502f60` | 1366/1942 | c | 6 code diffs: retail adds `mov [esi + 0xbc], ebx`; retail lacks `mov [esi + 0xbc], ebx` |
| krusty2/vehicle/Bike | `0x00405190` | 139/2381 (masked) | b | 99 code diffs: retail adds `mov edx, [esp + N]`; retail adds `push edi ; xor ebx, ebx ; xor edi, edi`; frame 0x94 vs 0xac, slot order differs |
| krusty2/vehicle/Bike | `0x00405db0` | 657/664 (masked) | b | 2 code diffs: `fld [esi + 8] ; fmul [eax]` vs `fld [eax] ; fmul [esi + 8]`; `fld [esi + 4] ; fmul [eax]` vs `fld [eax] ; fmul [esi + 4]`; slots differ |
| krusty2/vehicle/Bike | `0x004060c0` | 121/1436 (masked) | b | 63 code diffs: `mov edx, [esi + 0x4a8] ; mov eax, [esi ~` vs `mov eax, [esi + 0x4a8] ; mov ecx, [esi ~`; retail lacks `xor ecx, ecx`; frame 0x204 vs 0x214 |
| krusty2/vehicle/Bike | `0x00406ae0` | 78/1576 (masked) | b | 118 code diffs: `push ecx` vs `push ebp ; mov ebp, esp ; sub esp, 8`; `jne @16 ; mov [esp + N], 0 ; jmp @20` vs `jne @18 ; mov [ebp - N], 0 ; jmp @22`; frame 0x0 vs 0x8 |
| krusty2/vehicle/Bike | `0x004079c0` | 2078/6718 | b | spill homes of the loop values, the matched wheel kind kept in esi by retail, store scheduling of the local string tables and Vec3 constants (see PHYSICS_VALIDATION.md) |
| krusty2/vehicle/Bike | `0x00407440` | 63/384 (masked) | c | 16 code diffs: `jne @25` vs `jne @37 ; fld [esp + N]`; `mov [esp + N], A ; jmp @26 ; mov [esp +~` vs `fstp st(0) ; fld [A] ; fst [esp + N]` |
| krusty2/vehicle/Bike | `0x00409b30` | 826/1260 (masked) | c | 18 code diffs: `jmp @217` vs `jmp @222 ; mov ecx, [esp + N]`; `mov edx, [esi]` vs `mov [esp + N], ecx ; jmp @222 ; mov edx~` |
| krusty2/vehicle/Bike | `0x0040a520` | 439/2272 | b | 96 code diffs: retail lacks `test al, al`; retail adds `test al, al`; slots differ |
| krusty2/vehicle/Bike | `0x0040aec0` | 527/1186 (masked) | b | 35 code diffs; slots differ |
| krusty2/vehicle/Bike | `0x0040b600` | 58/949 (masked) | b | 36 code diffs: retail adds `push edi`; frame 0x3c vs 0x48 |
| krusty2/vehicle/Bike | `0x0040ba30` | 55/973 (masked) | b | 64 code diffs: retail lacks `push edi`; frame 0x1c vs 0x18 |
| krusty2/vehicle/Bike | `0x0040c000` | 135/802 (masked) | b | 60 code diffs: retail adds `push ebp ; mov ebp, esp`; retail lacks `fld [A]`; slot order differs |
| krusty2/vehicle/Bike | `0x0040c8b0` | 329/334 (masked) | c | 1 code diffs: `fld st(0) ; fmul [esp + N]` vs `fld [esp + N] ; fmul st(1)` |
| krusty2/vehicle/Bike | `0x0040ce60` | 185/228 (masked) | b | 6 code diffs: retail adds `fld [esp + N] ; fmul st(1)`; retail lacks `fld st(0) ; fmul [esp + N]`; slots differ |
| krusty2/vehicle/Vehicle | `0x005257a0` | 579/637 (masked) | b | 5 code diffs: retail lacks `mov [esi + 0x4fc], ebx ; mov [esi + 0x5~`; retail adds `mov [edx + 8], ecx`; slots differ |
| krusty2/vehicle/Vehicle | `0x005265d0` | 51/471 (masked) | b | 35 code diffs: retail lacks `fld [eax + 8]`; retail adds `mov ebx, [esp + N] ; mov ecx, [esp + N]`; slots differ |
| krusty2/vehicle/Vehicle | `0x005268d0` | 248/1446 (masked) | b | 87 code diffs: retail adds `xor ebx, ebx`; retail lacks `xor ebp, ebp`; slot order differs |
| krusty2/vehicle/Vehicle | `0x00526fa0` | 96/1161 | b | 21 code diffs: retail lacks `xor ecx, ecx`; `mov [esp + N], ebp` vs `mov [esp + N], ebx`; slot order differs |
| krusty2/vehicle/Vehicle | `0x005276d0` | 40/481 (masked) | c | 26 code diffs: `sub esp, 0x18` vs `sub esp, 0x24`; `push edi ; lea edi, [ecx + 0x194] ; jne~` vs `jne @123` |
| krusty2/vehicle/Vehicle | `0x00527a20` | 349/870 | b | 25 code diffs: `jne @99` vs `jne @98 ; mov [esp + N], 0 ; mov edx, [~`; `lea edx, [esi + 0x120]` vs `mov [esi + 0x12c], 0`; slots differ |
| krusty2/vehicle/Vehicle | `0x00527e30` | 31/831 | b | 42 code diffs: `push ebp` vs `push ebx ; mov ebx, ecx ; xor ecx, ecx`; retail adds `mov eax, [ebx + 0x444]`; frame 0x28 vs 0x2c |
| krusty2/vehicle/Vehicle | `0x005281b0` | 40/479 (masked) | b | 37 code diffs: retail adds `mov esi, [esp + N]`; `mov edi, [esp + N] ; mov esi, ecx` vs `mov edi, ecx`; slots differ |
| krusty2/vehicle/Vehicle | `0x005286a0` | 236/722 (masked) | b | 53 code diffs: retail lacks `push ebp`; retail adds `push edi`; frame 0x30 vs 0x3c |
| krusty2/vehicle/Vehicle | `0x005293e0` | 88/100 (masked) | c | 2 code diffs: `mov ecx, [esi + 0x1e8] ; mov edx, [esi ~` vs `mov edx, [esi + 0x1e8]`; `push ecx` vs `push edx ; mov edx, [esi + 0x4dc]` |
| krusty2/vehicle/Vehicle | `0x00529dc0` | 96/513 (masked) | c | 22 code diffs: `jl @10 ; jmp @34 ; mov edx, [esp + N]` vs `jge @58 ; jmp @10` |
| krusty2/vehicle/Vehicle | `0x00529fe0` | 36/635 (masked) | b | 38 code diffs: retail adds `mov ebp, ecx`; retail lacks `mov esi, ecx`; frame 0x10 vs 0xc |
| krusty2/vehicle/Vehicle | `0x0052a290` | 31/570 (masked) | b | 35 code diffs: retail adds `xor eax, eax`; retail adds `mov ebp, ecx`; frame 0x10 vs 0xc |
| krusty2/vehicle/Vehicle | `0x0052a6a0` | 19/331 (masked) | b | 22 code diffs: `push ecx ; push ebx` vs `sub esp, 8`; retail lacks `fld [edi + 0xe0]`; frame 0x0 vs 0x8 |
| krusty2/vehicle/Vehicle | `0x0052a940` | 304/3018 (masked) | b | 120 code diffs: `test eax, eax ; mov [esp + N], eax ; jn~` vs `mov ebp, eax ; xor ebx, ebx ; cmp ebp, ~`; `push 0` vs `push ebx`; frame 0xdc vs 0x104, slot order differs |
| krusty2/vehicle/Vehicle | `0x0052b6d0` | 28/63 | c | 4 code diffs: retail lacks `fld [A]`; retail adds `mov edx, [A]` |
| net/MSZoneInterface | `0x004aa360` | 233/300 (masked) | c | 7 code diffs |
| net/MSZoneInterface | `0x004aa4e0` | 237/309 (masked) | c | 7 code diffs |
| net/Net | `0x004aadc0` | 74/84 | c | 2 code diffs: retail lacks `mov [esi + 0x8f4], edi`; retail adds `mov [esi + 0x8f4], edi` |
| net/Net | `0x004ab960` | 149/1072 | b | 51 code diffs: `test esi, esi ; jge @54` vs `cmp esi, ebx ; jge @51`; retail adds `lea edx, [esp + N]`; slots differ |
| net/Net | `0x004abf10` | 874/948 | b | 13 code diffs: retail adds `mov ecx, [A]`; `mov ecx, [A]` vs `mov ebx, [esp + N]`; slots differ |
| net/Net | `0x004ac7c0` | 46/53 | c | 3 code diffs: `xor ecx, ecx ; mov edx, [esi] ; test ed~` vs `xor edx, edx ; mov ecx, [esi] ; test ec~`; `inc ecx ; cmp ecx, edx` vs `inc edx ; cmp edx, ecx` |
| net/Net | `0x004aced0` | 355/370 | c | 15 code diffs: `mov eax, [edi + eax + 4]` vs `mov eax, [eax + edi + 4]`; `mov eax, [edi + edx + 4]` vs `mov eax, [edx + edi + 4]` |
| net/NetThread | `0x004af8e0` | 234/338 | c | 19 code diffs: retail adds `push ebx`; retail adds `mov eax, [A]` |
| net/NetThread | `0x004afa90` | 698/1251 | b | 14 code diffs: `jne @388` vs `jne A`; `jne @374` vs `jne A`; slots differ |
| net/ZoneReport | `0x0049ca60` | 432/4824 | b | 16 code diffs: `mov dl, byte ptr [A] ; mov byte ptr [es~` vs `mov dx, word ptr [A] ; mov word ptr [es~`; retail lacks `mov byte ptr [esp + N], bl ; mov bx, wo~`; frame 0x5ac vs 0x5a8 |
| physics/bikeai/BikeAI | `0x0040e510` | 1133/1944 | b | 53 code diffs: retail adds `mov esi, [esp + N] ; test esi, esi`; `mov edi, [esp + N] ; test edi, edi ; je~` vs `je @582`; slot order differs |
| physics/bvh/BoundingBoxTreeBuild | `0x0042c260` | 447/1627 | b | 1 code diffs: retail lacks `mov edx, [eax + 0x27c] ; mov eax, [eax ~`; slots differ |
| physics/bvh/BoundingBoxTreeBuild | `0x0042d390` | 1601/1608 | b | 3 code diffs: `mov ebx, [ebx + ecx]` vs `mov ebx, [ecx + ebx]`; `mov ebx, [ebx + ecx - 0x1c]` vs `mov ebx, [ecx + ebx - 0x1c]`; slots differ |
| physics/bvh/BoundingBoxTreeQuery | `0x004278d0` | 778/830 | b | 7 code diffs: retail adds `fsubp st(1) ; fld [esp + N] ; fmul [esp~`; retail adds `fld [esp + N] ; fmul [esp + N] ; lea eb~`; slots differ |
| physics/bvh/BoundingBoxTreeQuery | `0x00428db0` | 429/797 | b | 10 code diffs: retail lacks `mov [esp + N], eax`; retail adds `mov [esp + N], eax`; slot order differs |
| physics/bvh/BoundingBoxTreeQuery | `0x004290d0` | 564/813 | b | 6 code diffs: retail lacks `fstp [esp + N]`; retail lacks `fld [esp + N]`; slots differ |
| physics/bvh/BoundingBoxTreeQuery | `0x00429570` | 246/831 | b | 17 code diffs: `jne @82 ; xor eax, eax ; pop edi` vs `je @272`; slots differ |
| physics/collision/CollisionContactUpdate | `0x0043aa30` | 25/698 (masked) | b | 51 code diffs: retail lacks `sub esp, N`; retail adds `sub esp, 0x1c ; mov ecx, [edx]`; frame 0x10 vs 0x0 |
| physics/collision/CollisionContactUpdate | `0x0043ad80` | 26/315 (masked) | c | 19 code diffs: `sub esp, 0xc` vs `sub esp, 0x24`; retail adds `jne @7 ; xor eax, eax ; add esp, 0x24` |
| physics/collision/CollisionPoint | `0x0043a640` | 41/814 (masked) | b | 44 code diffs: retail adds `push ebx`; retail adds `fld [esi + 0x30] ; fmul [eax + 4] ; fld~`; slots differ |
| physics/collision/CollisionPoint | `0x0043aff0` | 36/81 (masked) | c | 2 code diffs: `je @28 ; mov edx, [eax + 0xa4] ; test e~` vs `je @29 ; mov ecx, [eax + 0xa4] ; test e~` |
| physics/collision/CollisionPoint | `0x0043b240` | 155/162 (masked) | c | 2 code diffs: retail adds `fstp [esp + N]`; retail lacks `fstp [esp + N]` |
| physics/collision/CollisionVectorHelpers | `0x0043b190` | 150/156 (masked) | b | 2 code diffs: retail adds `mov ecx, [esp + N]`; `mov ecx, [esp + N] ; fld [eax + 4] ; fm~` vs `fld [ecx + 4] ; fmul [eax + 4]`; slots differ |
| physics/collision/CollisionVectorHelpers | `0x0043ca20` | 61/115 | b | 5 code diffs: retail adds `mov edx, [esp + N]`; retail lacks `mov edx, [esp + N]`; slots differ |
| physics/constraint/ConstraintMethodCollisionModel | `0x0043ba70` | 100/756 (masked) | c | 11 code diffs: `lea ebx, [esi - 0xc]` vs `lea ecx, [esi - 0xc]`; `mov ecx, ebx ; je @20` vs `je @19` |
| physics/constraint/ConstraintMethodCollisionModel | `0x0043bdb0` | 91/1686 (masked) | b | 65 code diffs: retail adds `push ebx ; push ebp`; retail adds `fld [eax + 0x154] ; fsub [eax + 0x160] ~`; frame 0xd4 vs 0xb8, slot order differs |
| physics/contact/ContactImpulse | `0x005004a0` | 104/927 (masked) | b | 47 code diffs: retail adds `fmul [esi + 4]`; retail lacks `fxch st(1) ; fmul [esi + 4]`; slot order differs |
| physics/contact/ObjectPlacement | `0x004b0df0` | 451/3321 (masked) | b | 123 code diffs: retail adds `mov edi, [esp + N]`; retail lacks `mov edi, [esp + N]`; frame 0x1c4 vs 0x1ac, slot order differs |
| physics/effects/DirtChunkParticleEmitter | `0x004b8df0` | 17/235 (masked) | c | 15 code diffs: retail lacks `sub esp, 8`; `lea edi, [esi + 0x80]` vs `lea ecx, [esi + 0x50] ; mov edx, eax ; ~` |
| physics/effects/DirtChunkParticleEmitter | `0x004b8f40` | 942/958 (masked) | c | 3 code diffs: `fld st(0) ; fmul [esp + N]` vs `fld [esp + N] ; fmul st(1)`; `fld st(0)` vs `fld [esp + N]` |
| physics/effects/DirtSprayParticleEmitter | `0x004b9310` | 30/235 (masked) | c | 10 code diffs: retail adds `lea ecx, [esi + 0x50] ; mov edx, eax`; retail adds `mov [ecx], edx ; mov [esi], A ; mov [es~` |
| physics/effects/DirtSprayParticleEmitter | `0x004b9460` | 942/958 (masked) | c | 3 code diffs: `fld st(0) ; fmul [esp + N]` vs `fld [esp + N] ; fmul st(1)`; `fld st(0)` vs `fld [esp + N]` |
| physics/effects/DustParticleEmitter | `0x004b8a00` | 21/110 (masked) | c | 7 code diffs: retail adds `lea ecx, [esi + 0x50] ; mov edx, eax`; retail adds `mov [ecx], edx` |
| physics/effects/SparkParticleEmitter | `0x004b9830` | 27/227 (masked) | c | 11 code diffs: `lea edi, [esi + 0x84]` vs `lea ecx, [esi + 0x50] ; mov edx, eax ; ~`; retail lacks `mov [esi + 0x50], eax ; mov [esi + 0x54~` |
| physics/effects/SparkParticleEmitter | `0x004b99a0` | 1022/1039 (masked) | c | 3 code diffs: `fld st(0) ; fmul [esp + N]` vs `fld [esp + N] ; fmul st(1)`; `fld st(0)` vs `fld [esp + N]` |
| physics/effects/SteamParticleEmitter | `0x004b9f40` | 21/164 (masked) | c | 6 code diffs: `lea edi, [esi + 0x74]` vs `lea ecx, [esi + 0x54] ; mov edx, ebx ; ~`; `mov [esi + 0x54], ebx ; mov [esi + 0x58~` vs `mov [ecx + 8], edx` |
| physics/helpers/GraphicsTest | `0x0047bd10` | 423/917 | b | 16 code diffs: retail adds `fsubp st(1)`; retail lacks `fsubp st(1)`; slots differ |
| physics/helpers/GraphicsTest | `0x0047c270` | 622/626 | b | 1 code diffs: `fld [eax] ; fadd [esi + 8]` vs `fld [esi + 8] ; fadd [eax]`; slots differ |
| physics/helpers/OrientationAngles | `0x004b5a60` | 564/581 (masked) | c | 2 code diffs: retail adds `fld st(0) ; fmul st(1)`; retail lacks `fld st(1) ; fmul st(2)` |
| physics/helpers/SoultreeMatrix | `0x004fb8c0` | 249/1072 (masked) | b | 13 code diffs: `mov ecx, [esp + N] ; mov edx, [esp + N]~` vs `lea ecx, [esp + N] ; push ecx`; `fld [esp + N] ; mov [esp + N], ecx ; le~` vs `call A`; slot order differs |
| physics/helpers/SoultreeMatrix | `0x004fbd70` | 661/693 (masked) | b | 6 code diffs: retail lacks `test eax, eax`; retail adds `test eax, eax`; slots differ |
| physics/helpers/SoultreeMatrix | `0x004fc050` | 392/1104 (masked) | b | 35 code diffs: retail lacks `mov ebx, ecx`; retail adds `mov ebp, ecx`; slot order differs |
| physics/helpers/SoultreeMatrix | `0x004fca80` | 308/458 (masked) | b | 10 code diffs: `mov [ebx + 4], edx` vs `mov [ebx + 8], eax`; `mov [ebx + 8], eax ; add esp, 0xc` vs `mov eax, [esp + N] ; mov [ebx + 0x18], ~`; slots differ |
| physics/helpers/SoultreeRotate | `0x004fd1f0` | 285/322 (masked) | c | 6 code diffs: `fld st(2) ; fmul [esi + 0xb8] ; fld st(~` vs `fld st(0)`; retail lacks `faddp st(1)` |
| physics/helpers/SoultreeTransform | `0x004fd660` | 160/164 (masked) | c | 2 code diffs: retail adds `mov ecx, eax`; retail lacks `mov ecx, eax` |
| physics/helpers/SoultreeTransform | `0x004fd710` | 203/205 (masked) | c | FPU operand order: one `fld/fmul` pair loads the other operand first (2 bytes) |
| physics/helpers/SoultreeTransform | `0x004fd7f0` | 259/281 (masked) | b | 6 code diffs: retail adds `mov eax, [esp + N]`; retail lacks `mov eax, [esp + N]`; slots differ |
| physics/krustybike/KrustyBike | `0x0048e280` | 69/273 (masked) | c | 13 code diffs: retail adds `fld st(0) ; fxch st(2) ; fxch st(1)`; retail adds `fxch st(1) ; fstp st(0)` |
| physics/krustybike/KrustyBike | `0x004919a0` | 147/221 (masked) | c | 8 code diffs: retail adds `mov edx, [edx + 0x60c] ; fsub [ecx + 0x~`; retail lacks `fsub [ecx + 0xb8]` |
| physics/krustybike/KrustyBike | `0x00491d10` | 211/1176 (masked) | b | 53 code diffs: retail lacks `sub esp, N`; retail adds `sub esp, 0x20`; frame 0x24 vs 0x0 |
| physics/krustybike/KrustyBike | `0x00492ad0` | 190/2315 | c | 62 code diffs: `sub esp, 0x30` vs `sub esp, 0x34`; `je @628 ; jmp @48` vs `jne @50 ; jmp @630` |
| physics/krustybike/KrustyBike | `0x004965e0` | 243/407 (masked) | b | 10 code diffs: retail lacks `mov [esp + N], 0`; retail lacks `mov [esp + N], 0 ; mov edx, [esp + N]`; slots differ |
| physics/krustybike/KrustyBike | `0x00496d10` | 1/1 (masked) | c | 1 code diffs: `jmp A` vs `jmp @12 ; nop ; nop ` |
| physics/krustybike/KrustyBike | `0x00496d20` | 61/99 (masked) | c | 3 code diffs: retail adds `lea ecx, [eax + 0xc]`; `jne @27 ; mov edx, [eax + 0xc] ; lea ec~` vs `jne @28 ; mov eax, [ecx] ; call [eax + ~` |
| physics/krustybike/KrustyBike | `0x00496da0` | 61/99 (masked) | c | 3 code diffs: retail adds `lea ecx, [eax + 0xc]`; `jne @27 ; mov edx, [eax + 0xc] ; lea ec~` vs `jne @28 ; mov eax, [ecx] ; call [eax + ~` |
| physics/motion/D3DIMSoultreeMotnctrl | `0x00445680` | 244/616 (masked) | b | 34 code diffs: `mov esi, [esp + N] ; mov edi, eax ; tes~` vs `mov esi, eax ; test esi, esi ; jne @118`; retail adds `push edx`; frame 0x30c vs 0x308 |
| physics/motion/D3DIMSoultreeMotnctrl | `0x00445fc0` | 330/331 | b | 1 code diffs: `mov ebp, [ecx + eax + 0x74]` vs `mov ebp, [eax + ecx + 0x74]`; slots differ |
| physics/motion/D3DIMSoultreeMotnctrl | `0x00446210` | 232/235 | c | 3 code diffs: `mov eax, [eax + 0x3c]` vs `mov ecx, [eax + 0x3c]`; `test eax, eax` vs `test ecx, ecx` |
| physics/motion/Motnctrl | `0x004a5e40` | 375/780 (masked) | b | 24 code diffs: `push edi` vs `push esi`; `xor ebx, ebx ; cmp eax, ebx ; mov [esp ~` vs `xor ebp, ebp ; cmp eax, ebp ; mov [esp ~`; slots differ |
| physics/motion/Motnctrl | `0x004a6bb0` | 342/1177 (masked) | b | 17 code diffs: retail adds `fld st(0)`; slots differ |
| physics/motion/Motnctrl | `0x004a7dc0` | 456/503 (masked) | b | 4 code diffs: `fld [esi + 0x20]` vs `mov [esp + N], ecx`; retail lacks `mov [esp + N], ecx`; slots differ |
| physics/motion/Motnctrl | `0x004a7fd0` | 562/1009 (masked) | c | 37 code diffs: `fld [esi + 8] ; fmul [edi + 4] ; fld [e~` vs `fld [edi + 4] ; fmul [esi + 8] ; fld [e~`; retail adds `fsubp st(1)` |
| physics/motion/Motnctrl | `0x004a8470` | 212/1093 (masked) | c | 43 code diffs: retail adds `push ebp`; retail lacks `push ebp` |
| physics/motion/SteeringControl | `0x00504d30` | 125/206 (masked) | b | 6 code diffs: `fxch st(2) ; fmul st(2)` vs `fld st(0) ; fmul st(3)`; `fmul st(2)` vs `fmul st(1)`; slots differ |
| physics/rigidbody/PhysicsRigidBody | `0x004cc630` | 983/1030 (masked) | b | 5 code diffs: retail adds `lea edx, [esp + N]`; retail lacks `lea edx, [esp + N]`; slots differ |
| physics/shadow/ProjectedShadow | `0x004da570` | 231/275 (masked) | c | 4 code diffs: `mov [esi + 0x88], eax` vs `mov edx, A`; `mov ecx, A` vs `mov [esi + 0x88], eax` |
| physics/shadow/ProjectedShadow | `0x004da7b0` | 152/777 (masked) | b | 22 code diffs: `lea ecx, [esp + N] ; mov eax, [eax + 4]~` vs `lea edx, [esp + N] ; mov ecx, [eax + 4]~`; `push ecx` vs `push edx`; slots differ |
| physics/shadow/ProjectedShadow | `0x004dae50` | 21/345 (masked) | b | 27 code diffs: `mov eax, [esp + N] ; sub esp, 0xc ; cmp~` vs `sub esp, N ; push ebx ; mov ebx, [esp +~`; `mov esi, ecx ; jne @72 ; mov edi, [esp ~` vs `cmp ebx, 1 ; mov edi, ecx ; jne @74`; frame 0x0 vs 0xc |
| physics/shadow/ProjectedShadow | `0x004db0d0` | 88/1597 (masked) | b | 108 code diffs: retail adds `push ebp`; retail adds `mov ecx, [ebx + 0x2c]`; frame 0x9c vs 0xc0, slot order differs |
| physics/shadow/ProjectedShadow | `0x004db8c0` | 19/840 (masked) | b | 51 code diffs: retail adds `push ebp ; mov ebp, esp`; `mov [esp + N], ebx` vs `mov [ebp - N], ebx`; slot order differs |
| physics/shadow/ProjectedShadow | `0x004dbd30` | 81/647 (masked) | b | 20 code diffs: `mov ecx, [esi + 0x74] ; mov ebx, [esi +~` vs `mov edi, [esi + 0x74] ; mov ecx, [esi +~`; `mov edi, ebx` vs `sar edi, 1 ; mov [esp + N], edx ; mov [~`; frame 0x40 vs 0x3c, slot order differs |
| physics/shadow/ProjectedShadow | `0x004dc2b0` | 29/324 (masked) | b | 29 code diffs: retail adds `push ebp ; mov ebp, esp`; retail lacks `push ebp`; frame 0x24 vs 0x34, slot order differs |
| physics/shadow/TerrainShadow | `0x005097d0` | 105/442 (masked) | b | 16 code diffs: `jmp @29` vs `jmp @31 ; mov edx, [esp + N]`; `mov edx, [ecx + 0x54] ; fild [edx + 0x1~` vs `mov [esp + N], edx ; mov eax, [ecx + 0x~`; slots differ |
| physics/shadow/TerrainShadow | `0x00508bc0` | 353/3031 | b | slot 27 footprint: frame 0xdc vs 0xd0, slot order, the cross product's operand order and the corner loops' form; the inline budget needs the per-mode accumulate/finish text repeated (macros) |
| physics/shadow/TerrainShadow | `0x00509aa0` | 86/1548 (masked) | b | 90 code diffs: retail adds `push ebp ; mov ebp, esp`; retail lacks `push ebp`; frame 0x370c vs 0x3728, slot order differs |
| physics/soultree_base/SoultreePhysicsInlines | `0x0040b410` | 275/404 (masked) | b | 18 code diffs: `fld [edi + 0x1b0]` vs `fld [esi + 4]`; `fld [edi + 0x1b4]` vs `fld [esi + 8]`; slots differ |
| physics/suspension/Shock | `0x004f9f90` | 227/230 (masked) | b | 1 code diffs: `fld [A] ; fdiv st(1)` vs `fld st(0) ; fdivr [A]`; slots differ |
| physics/suspension/Shock | `0x004fa400` | 458/674 (masked) | b | 20 code diffs: `je @160 ; mov eax, [A]` vs `je @162`; `jmp @180` vs `mov eax, [A] ; mov [esp + N], ecx ; mov~`; slots differ |
| physics/suspension/Shock | `0x004fac60` | 764/812 (masked) | b | 17 code diffs: retail adds `fsubp st(1)`; retail lacks `fsubp st(1)`; slots differ |
| physics/tire/Tire | `0x00512e80` | 19/137 | c | 13 code diffs: retail adds `push esi` |
| physics/tire/Tire | `0x00513490` | 175/177 (masked) | c | 2 code diffs: `mov [ebp + N], ebx` vs `mov [ebp + N], edi`; `mov [ebp + N], edi` vs `mov [ebp + N], ebx` |
| physics/tire/Tire | `0x005135f0` | 248/1444 (masked) | b | 53 code diffs: retail adds `mov edx, [esp + N]`; retail lacks `fmul [esi + 0xe8] ; fld [esp + N] ; fmu~`; frame 0xc vs 0x18 |
| physics/tire/Tire | `0x00513c70` | 96/805 | b | 24 code diffs: retail adds `push ebx`; retail adds `xor ebx, ebx`; slots differ |
| physics/tire/Tire | `0x00514550` | 169/3838 (masked) | b | 214 code diffs: `lea edi, [esi + 0x200] ; push edi` vs `lea ebp, [esi + 0x200] ; push ebp`; retail lacks `lea ebp, [esi + 0x20c] ; lea ebx, [esi ~`; frame 0xb8 vs 0xc0 |
| physics/tire/Tire | `0x00515880` | 48/653 (masked) | b | 35 code diffs: retail adds `mov eax, [esp + N] ; push esi ; mov ecx~`; retail lacks `mov ecx, [esp + N]`; slots differ |
| physics/tire/Tire | `0x00515b50` | 132/271 (masked) | c | 9 code diffs: `fld [ecx + 0x180] ; fmul [ecx + 0x58] ;~` vs `fld [ecx + 0x54] ; fmul [ecx + 0x17c] ;~`; `fld [ecx + 0x178] ; fmul [ecx + 0x50]` vs `fld [ecx + 0x58] ; fmul [ecx + 0x180]` |
| physics/vehicle/VehicleInlines | `0x0040c4c0` | 103/114 (masked) | b | 3 code diffs: `fld [edi + 4] ; fmul [esi]` vs `fld [esi] ; fmul [edi + 4]`; retail adds `fsubp st(1) ; mov [ebx + 4], ecx`; slots differ |
| physics/vehicle/VehicleInlines | `0x0040c540` | 119/126 (masked) | b | 2 code diffs: retail adds `fsubp st(1) ; mov [ebx + 4], eax`; retail lacks `fsubp st(1) ; mov [ebx + 4], eax`; slots differ |
| physics/visibility/VisibilityClipper | `0x0052f190` | 382/418 | c | 8 code diffs: `fld [ecx + 0x1c] ; fmul [edx + 4]` vs `fld [edx + 4] ; fmul [ecx + 0x1c]`; `fld [ecx + 0x2c] ; fmul [edx + 8]` vs `fld [edx + 8] ; fmul [ecx + 0x2c]` |
| physics/visibility/VisibilityQuadTreeTraversal | `0x0052d610` | 2046/2814 (masked) | c | 137 code diffs: `mov eax, [esp + N] ; mov ecx, [esp + N]` vs `mov eax, [edi + 4]`; retail adds `mov ecx, [eax] ; mov eax, [esp + N] ; p~` |
| race/BikeRace | `0x00417ed0` | 191/6756 | c | 1 code diffs: retail lacks `add ecx, 0x1cc ; test ecx, ecx ; jne @50` |
| race/BikeRace | `0x00419970` | 667/13322 | b | 66 code diffs: `je @1965 ; jmp @1960` vs `je A ; jmp A`; `mov edx, [ebp + N] ; mov [eax + 0x2fc],~` vs `mov ecx, [ebp + N] ; mov [eax + 0x2fc],~`; frame 0xac4 vs 0xab0, slot order differs |
| race/BikeRace | `0x0041eb20` | 1135/1710 | c | 4 code diffs: `je @304` vs `je A`; `je @396` vs `je A` |
| race/BikeRace | `0x0041f1d0` | 230/881 | c | 44 code diffs: retail adds `xor edx, edx`; `mov eax, [edi + 0x3430] ; test eax, eax~` vs `cmp [edi + 0x3430], edx ; jne @271` |
| race/BikeRace | `0x0041f5e0` | 163/2636 | c | 59 code diffs: `sub esp, 0x594` vs `sub esp, 0x494`; retail adds `xor ebx, ebx ; mov ecx, [eax + 8]` |
| race/BikeRace | `0x004210f0` | 879/2940 | b | 25 code diffs: `ja @852` vs `ja A`; `jle @852` vs `jle A`; slots differ |
| race/CarProcedural | `0x0042fd80` | 807/3018 | b | slot 10: slot order (frame 0xe8 vs 0xdc) and the steering clamp's memory local |
| race/CarProcedural | `0x00430b10` | 299/843 | b | 45 code diffs: retail adds `mov esi, ecx`; `mov edi, ecx ; mov esi, [edi + 0x6c] ; ~` vs `mov edi, [esi + 0x6c] ; mov ebp, [esi +~`; slots differ |
| race/Krusty3DObjects | `0x0048b100` | 137/2092 (masked) | c | 91 code diffs: `sub esp, 0x54` vs `sub esp, 0x3c`; retail adds `push edi` |
| race/Krusty3DObjects | `0x0048ccc0` | 47/1308 | b | 3 code diffs: `je @349` vs `je A`; `jne @349` vs `jne A`; slots differ |
| race/RaceSound | `0x004e4d10` | 563/2038 | c | 10 code diffs: `je @599` vs `je A`; `jne @280` vs `jne A` |
| race/RaceStatus | `0x004e5d00` | 168/519 | c | 29 code diffs: retail adds `mov ebp, [esp + N]`; retail adds `test ebp, ebp` |
| race/RaceStatus | `0x004e6a50` | 490/1003 | b | 6 code diffs: `je @289` vs `je A`; `je @289` vs `je A`; slots differ |
| race/Recorder | `0x004e6f80` | 516/2180 | c | 52 code diffs |
| race/Recorder | `0x004e7d60` | 272/2448 | b | 99 code diffs: `jne @767 ; mov eax, [esi + 0x88] ; test~` vs `jne @761 ; mov ecx, [esi + 0x88] ; xor ~`; slot order differs |
| race/Wrecker | `0x005306e0` | 1817/4183 | b | slot 10: first 0x62c bytes exact; the inverse/product term order, an unread position copy and the second product's expansion (budget) |
| race/Wrecker | `0x00531740` | 768/1617 | b | frame 0xc4 matches; slot order and `0.0f - scaled.z` operand order |
| race/Wrecker | `0x00531da0` | 629/634 | b | 1 code diffs: `fld [esp + N] ; fmul st(1)` vs `fld st(0) ; fmul [esp + N]`; slots differ |
| render/BackgroundImage | `0x00403d50` | 24/69 (masked) | c | 2 code diffs: `mov [esi], A` vs `mov ecx, 1`; `mov eax, 1 ; mov [esi + 0x30], eax ; mo~` vs `mov [esi], A ; mov [esi + 0x30], ecx ; ~` |
| render/BackgroundImage | `0x00403dc0` | 53/222 (masked) | c | 15 code diffs |
| render/BackgroundImage | `0x004040f0` | 113/253 (masked) | c | 4 code diffs: `jne @53 ; pop edi ; pop esi` vs `je @18` |
| render/BackgroundImage | `0x004043c0` | 35/177 | c | 6 code diffs |
| render/BackgroundImage | `0x004049d0` | 39/603 (masked) | c | 33 code diffs: `xor edi, edi` vs `mov [esp + N], 0`; `mov [esp + N], edi ; mov [ecx], edi ; j~` vs `mov [ecx], 0 ; je @162` |
| render/CacheTexture | `0x0050f9b0` | 606/618 (masked) | b | 8 code diffs: `mov ebp, ecx` vs `mov ebx, ecx`; `mov ebx, [ebp + N]` vs `mov ebp, [ebx + 0x184]`; slots differ |
| render/CacheTexture | `0x005102d0` | 426/529 (masked) | b | 13 code diffs; slots differ |
| render/CubeDraw | `0x0043dc60` | 121/1064 | b | 53 code diffs: `xor ebp, ebp` vs `xor edx, edx`; `mov [esp + N], ebp ; mov edi, 0x100 ; o~` vs `mov [esp + N], edx ; mov ebx, 0x100 ; o~`; slots differ |
| render/CubeDraw | `0x0043e0b0` | 30/352 | c | 22 code diffs: retail adds `push ebx`; retail adds `mov ecx, [esp + N]` |
| render/CubeDraw | `0x0043e330` | 90/399 | b | 15 code diffs: `mov ecx, [edi + 0x18] ; mov eax, [ecx +~` vs `mov eax, [edi + 0x18] ; mov ecx, [eax +~`; `mov eax, [ecx + 8] ; mov edx, [eax + 0x~` vs `mov ecx, [eax + 8] ; mov edx, [ecx + 0x~`; slots differ |
| render/CubeDraw | `0x0043e4c0` | 203/886 | c | 38 code diffs: `sub esp, 0x50` vs `sub esp, 0x54`; `push 0 ; push 0` vs `xor esi, esi ; push esi ; push esi` |
| render/D3DIMSoulTree | `0x00440060` | 1955/1964 | b | 4 code diffs: `mov ebx, [esi + 4] ; mov edi, [esp + N]` vs `mov ebx, [esp + N] ; mov edi, [esi + 4]`; `mov word ptr [esi + ecx], dx` vs `mov word ptr [ecx + esi], dx`; slots differ |
| render/D3DIMSoulTree | `0x00440810` | 135/1349 | b | 50 code diffs: `jle @345 ; xor esi, esi` vs `jle @340`; `mov [esp + N], esi ; mov eax, [edi + 0x~` vs `mov edx, [edi + 0x1c] ; mov esi, ebx ; ~`; frame 0x84 vs 0x80, slot order differs |
| render/D3DIMSoulTree | `0x00440d40` | 241/486 | b | 8 code diffs: `mov ebp, [edi + 4]` vs `mov ecx, [edi + 4]`; `add ebp, eax` vs `mov ebp, eax ; add ebp, ecx`; slots differ |
| render/D3DIMSoulTree | `0x00440f30` | 241/763 (masked) | b | 39 code diffs: `push ebx ; mov ebx, ecx ; mov [esp + N]~` vs `push esi ; mov esi, ecx ; mov [esp + N]~`; `je @1860 ; mov eax, [ebx + 0x18c]` vs `je A ; mov eax, [esi + 0x18c]`; frame 0x940 vs 0x9e8, slot order differs |
| render/D3DIMSoulTree | `0x00442fe0` | 77/504 (masked) | b | 37 code diffs: retail adds `mov esi, ecx`; retail lacks `mov edi, ecx`; slot order differs |
| render/D3DIMSoulTree | `0x004431f0` | 100/122 (masked) | b | 8 code diffs: `xor ebx, ebx` vs `xor edi, edi`; `push ebx` vs `push edi`; frame 0x134 vs 0x130 |
| render/D3DIMSoulTree | `0x004435b0` | 135/404 | b | 22 code diffs: retail adds `xor ebp, ebp`; retail lacks `xor edi, edi`; slots differ |
| render/D3DIMSoulTree | `0x00443740` | 66/796 | b | 29 code diffs: retail adds `xor ebp, ebp`; retail lacks `push 0`; slots differ |
| render/D3DIMSoulTree | `0x00443aa0` | 799/832 | c | 11 code diffs: retail lacks `fmul [eax + 0x10] ; fld [esp + N]`; retail lacks `faddp st(1)` |
| render/D3DIMSoulTree | `0x00443de0` | 25/588 | b | 35 code diffs: `mov eax, [esp + N] ; sub esp, 0x28 ; te~` vs `sub esp, N`; retail adds `mov ebx, [esp + N] ; push ebp`; frame 0x0 vs 0x28, slot order differs |
| render/D3DIMSoulTree | `0x00444140` | 610/764 | b | 7 code diffs: retail lacks `add ebp, 0x164`; retail adds `lea edx, [ebp + N]`; slots differ |
| render/D3DIMSoulTree | `0x00444560` | 665/1233 | b | 49 code diffs: `push ebp ; mov ebp, [esp + N]` vs `push ebx ; mov ebx, [esp + N]`; `mov eax, [ebp + N]` vs `mov eax, [ebx + 0x24]`; frame 0x30 vs 0x2c |
| render/DebugOverlay | `0x00447a00` | 76/1027 | b | 56 code diffs: retail lacks `push ebp`; `xor ebp, ebp` vs `xor ebx, ebx`; frame 0x264 vs 0x26c, slot order differs |
| render/DebugOverlay | `0x00447de0` | 66/174 | c | 13 code diffs: retail adds `push ebx`; retail lacks `push edi` |
| render/DirectoryList | `0x0044a600` | 74/109 | c | 6 code diffs: `jne @31 ; mov al, byte ptr [esp + N]` vs `jne @32 ; mov eax, [esp + N]` |
| render/DirectoryList | `0x0044ac30` | 329/1151 | b | 72 code diffs: retail lacks `mov ebx, ecx`; retail adds `mov ebp, ecx`; frame 0x24 vs 0x20, slot order differs |
| render/FontTexture | `0x004673d0` | 607/645 | b | 5 code diffs: retail adds `push esi ; push edi`; `jne @196 ; push esi ; push edi` vs `jne @194`; slots differ |
| render/GR_BitString | `0x004238c0` | 111/121 | c | 8 code diffs: `mov esi, eax` vs `mov edi, eax`; `sar esi, 5` vs `sar edi, 5` |
| render/GR_BitString | `0x004239c0` | 343/420 | b | 14 code diffs: retail lacks `mov ebx, ecx`; retail adds `mov ebp, ecx`; slots differ |
| render/GR_BitString | `0x00423b70` | 93/442 | b | 16 code diffs: retail adds `add esi, 4`; `or ecx, eax` vs `or eax, ecx ; mov [edx - 4], eax`; slots differ |
| render/Grid1 | `0x0047d370` | 101/172 | c | 11 code diffs: retail adds `mov esi, ecx ; lea ecx, [eax + eax*4]`; `mov edi, ecx ; lea ecx, [eax + eax*4] ;~` vs `mov edi, [esi + 0x3c]` |
| render/Grid1 | `0x0047d470` | 309/786 | b | 31 code diffs: retail adds `mov edi, [esp + N]`; retail lacks `mov edi, [esp + N]`; frame 0x838 vs 0x834, slot order differs |
| render/Grid1 | `0x0047d780` | 216/1008 | b | 50 code diffs: `je @321 ; mov esi, [esp + N]` vs `je @315`; `mov eax, [esi]` vs `mov edi, [esp + N]`; slot order differs |
| render/GridNode | `0x00484d70` | 81/90 | c | 3 code diffs: `mov ecx, [eax*4 + A] ; add ecx, ebx ; m~` vs `mov eax, [eax*4 + A] ; add eax, ebx ; m~`; retail adds `xor ecx, ecx ; mov cx, word ptr [eax + ~` |
| render/GridNode | `0x00483910` | 1040/1067 | c | cell sampler: 4 code diffs in the shift setup only; retail reuses the level byte of the leaf test (`and eax, 0xff ; shl eax, 2`, children into ecx), VC6 re-reads it (`xor eax, eax ; mov al, [ecx + 0x28]`) |
| render/GridNode | `0x00483d40` | 96/4189 | b | slot 1: ebp frame and four bare `fistp [eax]` from an `__asm` rounding helper (`__ftol` still used elsewhere), so unreachable under the no-inline-asm rule; `/Oy-` changes nothing else |
| render/Griddraw | `0x0047de90` | 977/1083 (masked) | c | 30 code diffs: retail lacks `mov esi, [esp + N]`; `mov edi, [ebp*N + N]` vs `mov edi, [esp + N]` |
| render/Griddraw | `0x0047e430` | 140/144 (masked) | c | 1 code diffs: `mov edx, esi ; add edx, [ebp + N]` vs `mov edx, [ebp + N] ; add edx, esi` |
| render/Griddraw | `0x0047e600` | 994/1477 (masked) | b | 15 code diffs: `fld [edx + 0x40] ; fmul [ecx + 0x164]` vs `fld [ecx + 0x164] ; fmul [edx + 0x40]`; `fld [edx + 0x40] ; fmul [esi + 0x18]` vs `fld [esi + 0x18] ; fmul [edx + 0x40]`; slots differ |
| render/Griddraw | `0x0047f210` | 361/543 (masked) | b | 29 code diffs: retail adds `xor esi, esi`; retail lacks `xor edi, edi`; slot order differs |
| render/Griddraw | `0x00480ad0` | 363/406 (masked) | b | 15 code diffs: retail adds `push ebp`; `push ebp` vs `push esi`; slots differ |
| render/Griddraw | `0x004815e0` | 59/994 (masked) | b | 39 code diffs: `push edi ; mov edx, [esi + 0x34] ; mov ~` vs `mov ecx, [esi + 0x34] ; mov edx, [esi +~`; `je @22` vs `je @19 ; fstp [esp + N] ; fld st(0)`; slots differ |
| render/Griddraw | `0x00481b30` | 127/351 (masked) | b | 15 code diffs: `mov edx, [eax + 0x60] ; fld [eax + 0x64]` vs `fld [eax + 0x60] ; mov edx, [eax + 0x64]`; retail lacks `mov [esp + N], edx`; slots differ |
| render/Griddraw | `0x00481de0` | 133/1616 (masked) | b | 85 code diffs: retail adds `mov ebx, [esp + N]`; retail adds `mov ebp, [esp + N]`; slots differ |
| render/Griddraw | `0x00482b40` | 280/315 (masked) | c | 9 code diffs: `lea ebx, [eax + edx]` vs `lea esi, [eax + edx]`; `lea esi, [eax + ecx]` vs `mov [esp + N], esi ; lea ebx, [eax + ec~` |
| render/Griddraw | `0x00482dd0` | 142/288 (masked) | c | 20 code diffs |
| render/Griddraw | `0x00483200` | 45/727 (masked) | b | 53 code diffs: retail adds `mov ebp, [esp + N]`; retail lacks `mov ebp, [esp + N]`; slots differ |
| render/ManagedTextureGroup | `0x0050c960` | 17/227 (masked) | c | 5 code diffs: `sub esp, 0x80` vs `sub esp, 0x78`; `je @167` vs `je A` |
| render/ManagedTextureGroup | `0x0050ef70` | 16/231 (masked) | b | 19 code diffs: retail lacks `push ebx`; retail lacks `push esi`; slots differ |
| render/MatrixUtil | `0x004a11e0` | 274/281 | b | 3 code diffs: retail adds `fsubp st(1)`; retail lacks `fsubp st(1)`; slots differ |
| render/MatrixUtil | `0x004a1a50` | 171/175 | c | 1 code diffs: `fld [ecx + 4] ; fmul [eax + 0x28]` vs `fld [eax + 0x28] ; fmul [ecx + 4]` |
| render/MorphBastardModifier | `0x004a33b0` | 729/2035 | b | 11 code diffs: `jle @611` vs `jle A`; retail adds `mov [eax + 0x34], edx`; slots differ |
| render/MorphBastardModifier | `0x004a3c80` | 74/3663 | b | 4 code diffs: `push ebp` vs `mov ebx, [esp + N]`; `mov ecx, [esp + N]` vs `mov ecx, [ebx + 0x44]`; frame 0x1b4 vs 0x1fc |
| render/MorphBastardModifier | `0x004a4c60` | 107/1570 | c | 19 code diffs: retail lacks `xor edi, edi ; test eax, eax ; mov [esp~`; retail adds `cmp eax, esi` |
| render/Overlay | `0x004b6710` | 53/365 | b | 23 code diffs: retail adds `mov eax, [esp + N]`; `mov esi, [esp + N]` vs `mov ebx, [eax] ; mov edx, [eax + 4]`; slots differ |
| render/PCRenderTarget | `0x004c5510` | 38/299 | c | 17 code diffs: `jmp @23 ; cmp [ecx + 0x34], esi` vs `jmp @24 ; mov edi, [ecx + 0x34] ; xor e~`; retail lacks `and edx, 0xff` |
| render/PCTextureMap | `0x004c69c0` | 429/894 (masked) | c | 31 code diffs: `jne @390` vs `jne A`; `jne @120` vs `jne @116 ; lea edx, [esp + N]` |
| render/PCTextureMap | `0x004c71c0` | 286/538 (masked) | c | 27 code diffs: `mov ebp, 1` vs `mov ebx, 1`; `and eax, ebp` vs `and eax, ebx` |
| render/PCTextureMap | `0x004c7b40` | 247/767 | b | 35 code diffs: retail lacks `mov edi, [eax]`; slots differ |
| render/PCTextureMap | `0x004c8550` | 319/321 (masked) | b | 2 code diffs: `mov edx, [esi + 0xc]` vs `mov edx, [esi + 8]`; `imul edx, [esi + 8]` vs `imul edx, [esi + 0xc]`; slots differ |
| render/PCVideoCard | `0x004ca520` | 26/124 | c | 13 code diffs: `mov ebx, [esp + N]` vs `push ebp`; retail adds `mov ebp, [ecx + 0x74]` |
| render/PCVideoCard | `0x004ca5a0` | 50/490 | c | 15 code diffs: retail adds `push ebp ; mov ebp, [esp + N]`; retail lacks `mov [eax], ebx ; mov eax, [esp + N]` |
| render/PCVideoCard | `0x004cab00` | 165/1141 | b | 57 code diffs: `mov edi, ecx` vs `mov ebp, ecx`; `mov [esp + N], edi` vs `mov [esp + N], ebp`; slots differ |
| render/PCVideoCard | `0x004cb330` | 35/628 | b | 46 code diffs: `push ebx ; mov ebx, [esp + N]` vs `push ebp ; mov ebp, [esp + N]`; `mov eax, [ebx]` vs `mov eax, [ebp + N]`; slots differ |
| render/PCVideoCard | `0x004cb5b0` | 145/186 | c | 8 code diffs |
| render/PCVideoCard | `0x0052d180` | 96/115 | c | 3 code diffs: retail adds `xor esi, esi ; and cl, 0xfb`; `and cl, 0xfb` vs `mov byte ptr [edx + 0x70], cl` |
| render/Pixtrans | `0x004cdf10` | 46/636 | b | 33 code diffs: retail adds `mov ecx, [esp + N] ; mov [esp + N], eax~`; retail lacks `mov [esp + N], eax ; mov eax, [esp + N]~`; slot order differs |
| render/Pixtrans | `0x004ce420` | 55/456 | b | 23 code diffs: `sub esp, 8` vs `sub esp, N ; mov edx, [esp + N]`; `mov eax, [esp + N] ; test eax, eax` vs `test edx, edx`; frame 0x8 vs 0x10 |
| render/Pixtrans | `0x004d0700` | 16/208 | c | 19 code diffs: `mov ecx, eax` vs `mov edx, eax ; mov ecx, [esp + N] ; pus~`; `shr ecx, 5` vs `shr edx, 5` |
| render/Pixtrans | `0x004d07d0` | 72/146 | c | 4 code diffs: `mov byte ptr [eax - 2], bl` vs `mov byte ptr [eax - 3], bl`; `mov byte ptr [eax - 1], bl` vs `mov byte ptr [eax - 2], bl` |
| render/Pixtrans | `0x004d24d0` | 419/988 | c | 4 code diffs: retail lacks `jmp @207 ; mov eax, [esp + N] ; mov [es~`; retail adds `mov eax, [esp + N] ; mov [esp + N], eax~` |
| render/ResourceManager | `0x004e9030` | 100/798 | b | 5 code diffs: `jne @35` vs `mov [esp + N], ebx ; jne @36`; `je @250` vs `je A`; slots differ |
| render/SoultreeMaterial | `0x004ff180` | 650/652 | c | 2 code diffs: `mov eax, [ecx] ; call [eax + 0x20]` vs `mov edx, [ecx] ; call [edx + 0x20]`; `add byte ptr [eax], al ; mov al, byte p~` vs `mov esi, A ; dec edi ; add cl, dh` |
| render/TextService | `0x0050ade0` | 117/125 | c | 1 code diffs: `mov eax, [ebp + N] ; mov edx, edi ; mov~` vs `mov ecx, [ebp + N] ; mov eax, edi ; mov~` |
| render/TextService | `0x0050b080` | 477/890 | b | 34 code diffs: `mov edx, [esp + N]` vs `xor edx, edx ; not ecx`; `not ecx` vs `dec ecx`; slot order differs |
| render/TextService | `0x0050b400` | 354/758 | b | 33 code diffs: `mov edx, [esp + N]` vs `xor edx, edx ; not ecx`; `not ecx` vs `dec ecx`; slot order differs |
| render/TextService | `0x0050b700` | 354/758 | b | 33 code diffs: `mov edx, [esp + N]` vs `xor edx, edx ; not ecx`; `not ecx` vs `dec ecx`; slot order differs |
| render/TextService | `0x0050ba00` | 671/802 | b | 10 code diffs: `jbe @192` vs `jbe A`; `je @185` vs `je A`; slots differ |
| render/Tgafile | `0x00512990` | 354/982 (masked) | c | 49 code diffs |
| track/SceneManager | `0x004ecd60` | 2047/4536 | b | 11 code diffs: `je @1189` vs `je A`; `je @1189` vs `je A`; slots differ |
| track/SceneManager | `0x004edfe0` | 3468/5346 | b | 34 code diffs: `je @1459` vs `je A`; `je @1459` vs `je A`; slot order differs |
| track/Track | `0x00516ca0` | 307/520 (masked) | b | 20 code diffs; slots differ |
| track/Track | `0x00516ef0` | 1023/1042 | c | 4 code diffs: retail adds `fld [ecx] ; fsub [ecx + 0xc]`; retail lacks `fld [ecx] ; fsub [ecx + 0xc]` |
| track/Track | `0x00517340` | 57/655 (masked) | b | 44 code diffs: retail lacks `mov [esp + N], 0`; `add esp, 0x24` vs `add esp, 0x20`; frame 0x24 vs 0x20 |
| track/Track | `0x005179f0` | 843/845 (masked) | c | FPU operand order: `fld [esp+0x3c]; fadd [esp+0x14]` where retail loads 0x14 first (2 bytes) |
| track/Track | `0x00518130` | 174/234 (masked) | c | 8 code diffs |
| track/Track | `0x0051ffe0` | 186/511 (masked) | c | 6 code diffs: `test eax, eax ; je @238` vs `cmp eax, ebx ; je A`; `call A ; movsx ebx, byte ptr [esp + edi~` vs `jmp A` |
| track/TrackOverlay | `0x005194b0` | 246/844 (masked) | b | 10 code diffs: retail lacks `mov [esp + N], 0x10`; retail lacks `mov byte ptr [esp + N], bl`; slot order differs |
| track/TrackOverlay | `0x00519a20` | 535/894 (masked) | c | 31 code diffs: retail lacks `mov ebp, ecx`; retail adds `push edi ; mov edi, ecx ; xor ebx, ebx` |
| track/TrackOverlay | `0x0051bb60` | 174/194 (masked) | c | 4 code diffs: retail adds `fidiv [A]`; retail lacks `fidiv [A]` |
| track/TrackOverlay | `0x0051bed0` | 1057/1088 (masked) | b | 3 code diffs: retail lacks `add edx, 0x5e0 ; push edx ; push ecx`; retail adds `add edx, 0x5e0`; slots differ |
| track/TrackOverlay | `0x0051c4f0` | 321/518 (masked) | b | 13 code diffs: `mov ecx, [esp + N]` vs `mov eax, [esp + N]`; retail adds `fld [esp + N] ; faddp st(2)`; slots differ |
| track/TrackOverlay | `0x0051c720` | 249/962 (masked) | b | 10 code diffs: `mov [ebp + N], ecx ; fstp [ebx + 0x1dc]~` vs `mov [ebx + 0x1bc], ecx`; retail adds `fstp [esi] ; fild [esp + N]`; frame 0x74 vs 0x8c, slot order differs |
| track/TrackOverlay | `0x0051cf80` | 583/1779 (masked) | c | 35 code diffs: `jle @263 ; mov ecx, [ebx + 4] ; mov eax~` vs `jle @267 ; mov ecx, ebx ; mov ebx, [ebx]`; retail lacks `mov edx, [ebx] ; mov [esp + N], ecx` |
| track/TrackOverlay | `0x0051d730` | 214/510 (masked) | b | 26 code diffs: `xor ebp, ebp` vs `xor ebx, ebx`; `mov [esi + 0x198], ebp ; mov [esp + N],~` vs `mov [esi + 0x198], ebx ; mov [esp + N],~`; slots differ |
| track/TrackOverlay | `0x0051e3f0` | 886/894 (masked) | b | 6 code diffs: `mov edx, [ebx + ebp*4 + 0x16c]` vs `mov eax, [ebx + ebp*4 + 0x16c]`; `lea eax, [esp + N] ; fld [edx + 0x4a4]` vs `lea ecx, [esp + N] ; fld [eax + 0x4a4]`; slots differ |
| ui/DlgProcs | `0x0044b200` | 925/5232 | b | 144 code diffs: `ja @1362` vs `ja A`; `jmp @1362` vs `jmp A`; frame 0xc64 vs 0x6c4 |
| ui/DlgProcs | `0x0044f750` | 190/509 | b | 27 code diffs: `mov ecx, [ebp + N] ; cmp [ecx + 0x54], ~` vs `mov eax, [ebp + N] ; cmp [eax + 0x54], ~`; `mov eax, [ecx + 0x50] ; mov ebx, [esp +~` vs `mov ecx, [eax + 0x50] ; mov edx, [esp +~`; slots differ |
| ui/DlgProcs | `0x0044fa00` | 27/181 | c | 12 code diffs: retail adds `push ebx ; mov ebx, [esp + N]`; retail adds `mov esi, ecx ; cmp [ebx + 4], 1 ; jne @~` |
| ui/DlgProcs | `0x0044fae0` | 1181/1235 | b | 8 code diffs: retail lacks `mov edi, esp`; retail adds `mov edi, esp`; slots differ |
| ui/DlgProcs | `0x004507a0` | 589/1668 | c | 32 code diffs: `mov eax, [edx + 0x570] ; mov ecx, [eax ~` vs `mov ecx, [edx + 0x570] ; lea eax, [edx ~` |
| ui/DlgProcs | `0x004513a0` | 506/2004 | c | 19 code diffs: `jne @515` vs `jne A`; `ja @515` vs `ja A` |
| ui/DlgProcs | `0x00452930` | 276/1875 | c | 86 code diffs: retail adds `jmp @67 ; mov ebx, [esp + N] ; push 1` |
| ui/GUIManager | `0x00485a70` | 62/345 | c | 29 code diffs: retail lacks `mov edx, ebx`; retail lacks `mov ebp, [esp + N]` |
| ui/GUIManager | `0x00486170` | 280/896 | b | 52 code diffs: retail lacks `xor ebx, ebx`; `cmp ecx, ebx` vs `xor ebx, ebx`; frame 0xa4 vs 0xb0, slot order differs |
| ui/GUIManager | `0x00486b10` | 86/106 | c | 5 code diffs: retail lacks `mov edx, [esi + 0x48]`; `mov [esp + N], edx` vs `pop edi` |
| ui/GUIManager | `0x00486b80` | 227/555 | c | 13 code diffs: `add eax, -2` vs `sub eax, 2`; retail adds `mov edi, [esp + N]` |
| ui/GUIManager | `0x00487870` | 18/285 | c | 27 code diffs: retail adds `push edi ; mov edi, 1`; `mov [esp + N], 1 ; je @16` vs `mov [esp + N], edi ; je @19` |
| ui/GUIManager | `0x00488120` | 17/52 | c | 5 code diffs: retail adds `jmp @6 ; xor edx, edx` |
| ui/GameCursor | `0x0043ed40` | 871/946 | c | 8 code diffs: `je @259` vs `jne @312`; `xor eax, eax` vs `mov eax, 1` |
| ui/GameUi | `0x0046a920` | 684/2323 (masked) | b | 17 code diffs: `mov eax, 0x64` vs `mov ecx, 0x64 ; mov eax, 1 ; mov [esp +~`; `mov eax, 1` vs `mov [esp + N], eax`; frame 0x9bf4 vs 0x9c74, slot order differs |
| ui/GameUi | `0x0046e8c0` | 55/64 | c | 4 code diffs: retail lacks `mov edx, [ecx]`; `lea eax, [esp + N]` vs `mov eax, [ecx] ; lea edx, [esp + N]` |
| ui/GameUi | `0x0046ea80` | 160/164 | c | 2 code diffs: `cmp [eax + 0x78], edi` vs `cmp [eax + 0x78], esi`; `push esi` vs `push edi` |
| ui/GameUi | `0x0046ef00` | 352/555 | c | 13 code diffs: retail adds `mov [edi + 0xc], ebx`; retail lacks `mov [edi + 0xc], ebx` |
| ui/GameUi | `0x00470170` | 535/578 | c | 3 code diffs: retail lacks `jmp @136 ; mov [esi + 0x1dc], edi`; retail adds `mov [esi + 0x1dc], edi ; jmp @134` |
| ui/GameUi | `0x00470450` | 88/386 | c | 14 code diffs: `je @53 ; push edi ; mov edi, [eax + 0x1~` vs `je @43 ; cmp [eax + 0x1d4], esi ; jne @~` |
| ui/GameUi | `0x004705d0` | 91/101 | c | 5 code diffs: `mov edx, [esi + 0xcc]` vs `mov ecx, [esi + 0xcc] ; push ecx`; retail lacks `push edx` |
| ui/GameUi | `0x00470f10` | 118/332 | c | 7 code diffs: `mov edi, [esi + 0xb8]` vs `mov ebp, [esi + 0xb8]`; `fmul [edi + 0x9c4]` vs `fmul [ebp + N]` |
| ui/GameUi | `0x00472960` | 352/566 | c | 18 code diffs: retail lacks `push A`; retail adds `push A` |
| ui/GameUi | `0x00472e30` | 34/82 | c | 5 code diffs: retail lacks `mov eax, [esp + N]`; `mov [esi + 0xf0], eax` vs `mov edx, [esp + N]` |
| ui/GameUi | `0x00472fe0` | 77/200 | c | 18 code diffs |
| ui/GameUi | `0x004733a0` | 85/99 | c | 4 code diffs: `mov ecx, [esi + 0xcc] ; push ecx` vs `mov eax, [esi + 0xcc]`; retail adds `push eax` |
| ui/GameUi | `0x004734c0` | 256/354 | c | 8 code diffs: `je @82 ; jmp @81` vs `je @83 ; mov edi, 1 ; jmp @84` |
| ui/GameUi | `0x004738a0` | 206/975 | b | 53 code diffs: `mov edi, [esi + 0x1bc] ; mov ebx, eax` vs `mov ecx, [esi + 0x1bc] ; lea ebx, [esi ~`; retail lacks `lea ebp, [esi + 0x1bc]`; frame 0x38 vs 0x40 |
| ui/GameUi | `0x00474150` | 339/1048 | c | 28 code diffs |
| ui/GameUi | `0x00474880` | 254/355 | c | 34 code diffs: retail adds `push ebx`; retail adds `push esi ; mov esi, ecx` |
| ui/GameUi | `0x004749f0` | 49/274 | c | 9 code diffs: `je @77` vs `je A`; `jne @85` vs `jne A` |
| ui/GameUi | `0x00475c70` | 319/398 | c | 4 code diffs: `jne @81` vs `jne @82 ; mov ecx, [esi + 0xb8] ; cmp e~`; `cmp eax, ebx ; je @79 ; mov eax, [eax +~` vs `mov ecx, [eax + 0xdc] ; mov eax, [esi +~` |
| ui/GameUi | `0x00477110` | 356/646 | c | 12 code diffs: `mov ecx, [esp + N]` vs `mov eax, [esp + N]`; retail adds `mov [esp + N], eax ; mov ecx, [esi + 0x~` |
| ui/GameUi | `0x004773a0` | 169/232 | c | 5 code diffs: `mov ecx, [esp + N] ; mov [esp + N], ecx~` vs `mov eax, [esp + N]`; retail adds `mov ecx, [esi + 0xc] ; mov edx, [esi + ~` |
| ui/GameUi | `0x00477800` | 5/262 | c | 12 code diffs: `mov eax, [A]` vs `mov ecx, [A]`; retail adds `xor eax, eax` |
| ui/GameUi | `0x00477bc0` | 89/288 | c | 16 code diffs: retail lacks `xor edx, edx`; retail adds `xor ecx, ecx` |
| ui/GameUi | `0x00477e90` | 155/344 | c | 14 code diffs: `push ebp` vs `push ebx`; `mov ebp, [esp + N]` vs `mov ebx, [esp + N]` |
| ui/GameUi | `0x00478570` | 139/682 | b | 32 code diffs: `lea eax, [esi + 0x1bc] ; mov [esp + N],~` vs `mov eax, [esi + 0x1bc] ; lea ebx, [esi ~`; retail adds `mov [esp + N], ebx`; slot order differs |
| ui/GameUi | `0x00478e10` | 11/328 | b | 1 code diffs: retail lacks `push ebp ; push esi ; mov esi, ecx`; frame 0x90 vs 0x84 |
| ui/GameUi | `0x00479710` | 736/738 | c | 1 code diffs: `mov ebp, [edi + 0x204] ; sub eax, ebp` vs `mov ecx, [edi + 0x204] ; sub eax, ecx` |
| ui/GameUi | `0x0047a400` | 320/962 | c | 21 code diffs: retail adds `mov [esi + 0x20c], ecx`; `mov [esi + 0x20c], ecx` vs `add ecx, ebp ; mov [esi + 0x210], edx ;~` |
| ui/GameUi | `0x0047b490` | 116/220 | c | 14 code diffs: retail adds `mov esi, ecx`; `mov esi, ecx ; sar edx, 8 ; sar esi, 5` vs `sar esi, 8 ; sar edx, 5 ; and esi, 0xf8~` |
| ui/KrustyUI | `0x004988a0` | 872/1104 | b | 8 code diffs: `mov edx, [esi + 0x2c] ; mov eax, [esp +~` vs `mov eax, [esi + 0x2c] ; mov ecx, [esp +~`; `mov edx, [eax + 0xc0] ; mov [edx + 0x5c~` vs `mov eax, [eax + 0xc0] ; mov [eax + 0x5c~`; slots differ |
| ui/KrustyUI | `0x0049b0d0` | 880/916 | c | 5 code diffs: `jne @202 ; cmp ecx, 1 ; jne @204` vs `je @212`; `jne @205` vs `jne @202 ; jmp @218 ; cmp ecx, 1` |
| ui/OptionProcs | `0x004b5600` | 73/341 | b | 8 code diffs: retail adds `mov ebx, ecx`; retail lacks `mov ebp, ecx`; slots differ |
| ui/OptionProcs | `0x004b5760` | 259/708 | c | 29 code diffs |
| ui/ProCircuitProcs | `0x004d6fc0` | 555/557 | b | 1 code diffs: `mov ebp, [esp + N] ; add eax, ebp` vs `mov ecx, [esp + N] ; add eax, ecx`; slots differ |
| ui/ProCircuitProcs | `0x004d72c0` | 1125/1291 | b | 11 code diffs: retail lacks `mov edi, esp`; retail adds `mov edi, esp`; slots differ |
| ui/ProCircuitProcs | `0x004d8c40` | 427/886 | c | 15 code diffs: retail lacks `mov edx, [esi + 8]`; retail lacks `mov ebx, [esi + 0x14]` |
| ui/ProCircuitProcs | `0x004d8fc0` | 750/882 | c | 45 code diffs: retail adds `mov fs:[X], esp`; retail lacks `mov fs:[X], esp` |
| ui/ProCircuitProcs | `0x004d9860` | 225/1123 | b | 20 code diffs: retail adds `mov fs:[X], esp ; sub esp, N`; retail lacks `mov fs:[X], esp ; sub esp, 0x84`; frame 0x0 vs 0x84 |
| ui/ProCircuitProcs | `0x004d9cd0` | 48/755 | c | 37 code diffs: retail adds `mov eax, [A]`; retail adds `mov ebp, [eax + 0x3444]` |
| ui/ProCircuitProcs | `0x004d9fd0` | 312/312 (masked) | c | 1 code diffs: `add byte ptr [eax], al ; mov al, byte p~` vs `sub byte ptr [eax - A], ah ; dec ebp ; ~` |
| ui/SelectGamePicProcs | `0x004f17a0` | 617/2258 | b | 2 code diffs: `je @651` vs `je A`; retail lacks `mov [esp + N], 0 ; je @191 ; mov edx, [~`; slot order differs |
| ui/SelectGamePicProcs | `0x004f2340` | 933/2148 | b | 62 code diffs: `mov [esp + N], ecx` vs `push esi`; `push esi` vs `mov [esp + N], ecx`; slot order differs |
| ui/SelectGamePicProcs | `0x004f3720` | 92/380 | b | 10 code diffs: retail lacks `jmp @44 ; lea ecx, [esp + N] ; push 0x80`; slots differ |
| ui/SelectGamePicProcs | `0x004f3a70` | 2714/2729 | b | 2 code diffs: retail adds `mov [esp + N], edx ; fild qword ptr [es~`; retail lacks `fild qword ptr [esp + N] ; mov [esp + N~`; slots differ |
| ui/SelectGamePicProcs | `0x004f78a0` | 2415/2419 | b | 3 code diffs: `fld st(0) ; fmul st(1)` vs `fld st(2) ; fmul st(3)`; `fld st(3) ; fmul st(4)` vs `fld st(1) ; fmul st(2)`; slots differ |
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
