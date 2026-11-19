# Grid1 (DrawableGridNodeSharedTextures)

Unit: [`src/reconstructed/Grid1.cpp`](../src/reconstructed/Grid1.cpp)
(`D:\aardvark\VC\krusty2\Grid1.cpp`, `__FILE__` `0x0056c0a0`). The extent,
RTTI and the ten exact functions are in [CARPROCEDURAL.md](CARPROCEDURAL.md).
Slot 3 `SelectTexture` (`0x0047caa0`) and the two run-length texture fills
`BuildNodeTexture` (`0x0047d470`) and `BuildBlockTexture` (`0x0047d780`) sit
in the unit in retail order with `// Not exact` status comments. All three
keep retail's control flow and calls and differ in register allocation.
Scores are matching bytes under `vc6_o2_mt` with relocations resolved (213 of
1728, 313 of 786, 228 of 1010). Emu cases: `grid_select_texture`,
`grid_build_node_texture`, `grid_build_block_texture`.

## Near misses: forms tried

Slot 3 (0x0047caa0, 1728 bytes; ~104 positions of 1740, aligned instruction
ratio 0.58): control flow, the jump table, the case fall-through chain, the
per-block loop (nested ifs, else paths out of line), all calls and the x87
`scale` kept on the FPU stack to the final divide match. Differences: retail
keeps a zero constant in ebp that is coalesced with `block` (re-zeroed by `xor
ebp,ebp` at the loop exit and the in-loop retry), `stage` in ebx and the CSE'd
extra pointer in edx; VC6 here spills `block` (one extra frame slot, 0x14) and
caches the extra pointer in ebp. Declaring `block` without an initializer and
the loop as `for (; block < 16; block++)` produces retail's zero/ebp
coalescing (ratio 0.71) but reads an uninitialised variable, so it is not
used. The stage ladder's fourth compare (retail keeps `fcomp g*4` with no
branch after a speculative `stage = 3`) is folded away here. Reading the
terrain through the `(Grid1Terrain*)terrain` cast everywhere, and the node's
extra through `(Grid1NodeExtra*)extra` in the two texture build blocks,
instead of the Terrain()/Extra() accessors, gives retail's shrink-wrapped
pushes and most of its register choice (257 -> 63 differing instructions, 104
-> 213 strict positions; found by a greedy per-call accessor/cast search).
Casting the other Extra() uses makes it worse. The folded fourth compare comes
back (stage store hoisted above the fcomp, as retail) only with a second store
in one arm: `else { stage = 3; keep = 0; }` gives 287 strict but leaves that
store; equal arms through locals, casts, a static const, dead locals or dead
parameter stores fold. Retail's arm is empty after `stage = 3` (the `test ah,
1` falls into the join), so the branch went late while the x87 compare stayed.
Also kept with a leftover store or branch: `stage = 3; if (g*4 >= detail)
stage = 3;` (264), `if (g*4 < detail) detail = 10000.0f;` (267); folded
entirely: an empty `if`, `?:` with equal arms, a clamp of a stage 4, `block =
0`, an inline helper returning the stage (with or without the final else), a
reversed fourth compare and a local holding 3 (s15 reference diff: the missing
g*4.0f loads and `test ah, 1` are this compare).

0x0047d470 (783 bytes; 313 of 786): identical control flow and calls; retail
keeps the row (y & 15) in eax, the head length in edx and the run line pointer
in ebx (frame 0x834), VC6 here uses edx/ebx and spills the line pointer (frame
0x838). Writing the tile-span test as a subtraction (`last - first == 0`) is
required for retail's `sub ecx,edx`; the equality form becomes xor/test.
Computing `rest` before `line` gives retail's tail-loop shape (313 of 786; 149
-> 113 differing instructions, 124 -> 90 with stack offsets masked); `line` is
still spilled and `first`/`row` sit in ebx/edx where retail has edx/eax.
Inline row terms, unsigned x/y, a `tile += row * tileStride` reuse, `16 * bpp`
in the tail loop, a ternary `first`, `rest -= first` in the head, a `column`
local, a `last`/`span` local and `line` declared at the block start were tried
without closing the gap.

0x0047d780 (977 bytes; 227 of 1010): the per-block variant (4 x 4 pixel cells,
width / 4 tiles, four rows per run, a dead `done` accumulator that retail
keeps). Retail computes the head source pointer before the `x & mask` test and
reuses row * tileStride for the line pointer; texture in edi and tileSize in
esi (swapped here); its inner row copies keep the destination in
ebp/[esp+0x14]. Retail has 339 instructions to 325 here. The head source
pointer (with or without a `column` local) computed before the test is worse
(144-154); `rest` before `line` gains only two bytes (retail computes `line`
first here). A declaration-order sweep found `dst` before `src` in the head
copy and `done` before `tileStride`: strict 216 -> 227, differing instructions
176 -> 158. k-scan (s22): compiled behind 0..63 prepended typedefs,
0x0047caa0, 0x0047d470, 0x0047d780 have the same code at every count, so their
differences are not a declaration-count effect (docs/VC6_OPERAND_ORDER.md
section 2).
