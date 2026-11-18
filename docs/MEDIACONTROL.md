# MediaControl and MovingPart

Canonical reconstruction: `src/reconstructed/MediaControl.*` and
`src/reconstructed/MovingPart.*`. The gap is `0x004a2350..0x004a2ac0`,
between the Math3D vector set (`.CRT$XCU` 179-182, `0x004a2210..0x004a2340`)
and the allocation accounting unit (`0x004a2ac0`). No `__FILE__` literal
reaches it, so every file name is ours (tier 3).

## MovingPart (2 exact)

- `0x004a2350` `GetMatrixRow` (66 bytes, cdecl, hidden result) returns
  `m[row][0..2]`. MorphBastardModifier and Bike call it.
- `0x004a23a0` is the `MovingPart` constructor (`ret 0xc`). It finds a named
  node below a root through `SoultreeObject::FindByName` (`0x004fdae0`),
  copies the node's local matrix to `+0` (`0x004fca60`), and stores the
  node at `+0x40`, its third argument at `+0x44` and zero at `+0x48`.
  MovingPart is the non-polymorphic base of Tire (mdisp `0x17c`) and of the
  Shock family (mdisp 4); see `src/krusty2/tire/Tire.h`.
- Open: whether these two belong to the Math3D unit before them, to
  MediaControl's unit, or to a unit of their own. Nothing calls across the
  boundaries.

Source form that mattered: `GetMatrixRow` copies the three components into
a local one by one. Returning `*(Vector3*)m->m[row]` copies straight to the
hidden result, and a `Vector3(x, y, z)` temporary goes through the x87.

## MediaControl (16 of 17 exact)

RTTI `.?AVMediaControl@@` (vtable `0x005551c0`), a GameObject that plays a
movie through DirectShow multimedia streams; see `MediaControl.h` for the
GUIDs and interface slots. The COM methods carry their mmstream.h/amstream.h
names (interface identity from the IIDs passed, method from the slot index).
Member names are ours: `Open` (`0x004a2560`), `IsDirectShowInstalled`
(`0x004a23d0`), `BlitFrame`
(`0x004a2a90`), and fields such as `multiMediaStream` (+0x50),
`videoStream` (+0x5c), `videoDrawStream` (+0x60), `videoSample` (+0x64),
`pausedTime` (+0x68) and `doneCallback`/`doneOwner` (+0x70/+0x74).

- `0x004a2900` `Restart` (50 bytes) matches with a `do { ... break; ... }
  while (0)` body. That form places the shared `return 0` before `return 1`
  (`jl` to it, then `jge` over it). The `&&`, nested-if, early-return,
  `goto`, ternary and result-flag forms all put `return 1` first.
- Near miss: `0x004a2560` (open, 646 bytes) in `MediaControl.cpp`, in
  retail order with a `// Not exact` status comment. Every call, argument and
  branch target matches. Retail allocates one more callee-saved register
  (a zero in `ebx`, `&videoDrawStream` in `ebp`), which shifts every frame
  offset by four.
  The [functional build review](MEDIA_CONTROL_OPEN.md#functional-build-review)
  records the complete-body native COM/output/alias/failure comparison and
  the remaining real-device and decoder limits.

## Open `0x004a2560`: forms tried

Carried over from the header of the former `samples/gameui/MediaControlNearMisses.cpp` when Open moved into MediaControl.cpp.

```text
0x004a2560 (open, 646 bytes; 9% masked): every call, argument, GUID and
branch target matches. Retail keeps a zero in ebx and &videoDrawStream in ebp
after the QueryInterface for the DirectDraw stream and pushes ebx for the
later zero arguments; VC6 here pushes immediates and uses one register
fewer, so the frame offsets differ by one dword. Neither local order nor
moving the `surface = 0` store before the memset changes it. Retail's ebx
holds &videoStream and then videoStream itself (the QueryInterface `this`)
before the zero; VC6 here reloads it into eax.

The zero register itself is reachable: `DirectDrawSurface* surface
= 0;` as a declaration initialiser (with the later `surface = 0` kept)
makes VC6 keep the zero in a register and reproduces retail's frame (637
bytes). But retail births the `xor ebx, ebx` inside the inlined memset
after that QueryInterface and keeps &videoDrawStream in ebp and videoStream in
ebx; VC6 hoists the xor to entry and swaps ebx/ebp, so the score stays.
Zeroing `surface` both before the memset and after frameHeight gives 11%
with two stores. Session 14: the initialiser in a block opened after the
videoDrawStream QueryInterface (later `surface = 0` kept) scores 73 of 646
strict against 53, but VC6 still births the zero at entry (in ebp,
&videoDrawStream in ebx) and adds a store, so it is not adopted. A bare
block (53) or the rest nested under `>= 0` instead of the goto (50; 64
with the initialiser) do not help.
Retail's second memset fills with eax, the zero the DirectDraw
QueryInterface's `jne failed` test leaves there.
```

## Reproduce

```bash
python3 tools/compile.py --compiler vc6 --vc6-root "$VC6_ROOT" src/reconstructed/MovingPart.cpp -o work/MovingPart.obj
python3 tools/match.py --exe "$MCM2_EXE" --target-va 0x004a23a0 --target-size 48 --obj work/MovingPart.obj --symbol '??0MovingPart' --bindings src/reconstructed/MovingPart.bindings.json
python3 tools/run_calibration.py --exe "$MCM2_EXE" --compiler vc6 --vc6-root "$VC6_ROOT"
```
