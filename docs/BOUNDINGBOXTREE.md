# BoundingBoxTreeBuild.cpp and GR_BitString

**BoundingBoxTreeBuild.cpp** (`0x0042ad30..0x0042e2a3`,
`src/reconstructed/BoundingBoxTreeBuild.*`; layouts and evidence in
`src/krusty2/bvh/README.md`). It moved from `samples/physics/bvh`; the old
sample copies were removed. Exact: 28 calibration cases. These are the six
split and bounds helpers, the three split choosers, the triangle, point
and box node and tree builders, BuildModelPointTree `0x0042d390`, AxisAngleMatrix, the three readers and
the eight `$E`. Its bindings were read back from the retail relocation
sites after a masked match. Each float constant was checked against its
stored value, and the kVec3 addresses against the `$E` bodies. The stream
stand-in is now the shared `FileStream`. BuildModelPointTree
needs a `TreeVec3* p = &points[count]` local in both vertex loops for
retail's operand order. Near miss, in the unit in retail order with a
`// Not exact` status comment: GatherModelTriangles `0x0042c260`
(1626/1627; [forms tried](#gathermodeltriangles-0x0042c260-forms-tried)).
It replaced the game build's bring-up copy
(`src/bringup/Group0_GatherModelTriangles.cpp`).

The code before `0x0042ad30` is a separate, unattested unit. It has its
own kVec3 `$E` set (`0x00429400`), its own bss, and the five empty `$E`
pairs at `0x00424690`. The `.CRT$XCU` order is bikerace.cpp, `0x00429400`,
`0x00424690`, then `0x0042d250`. That unit is now
`src/krusty2/bvh/BoundingBoxTreeQuery.cpp` (its name is ours); see
[PHYSICS_VALIDATION](PHYSICS_VALIDATION.md).

**GR_BitString** (`0x004238c0..0x00423f6d`, `src/reconstructed/GR_BitString.cpp`;
file name not recovered). This is a separate unit (strong inference):
- It has no `__FILE__` and no `$E`.
- Its `.data` block (`0x00567f90..0x00568227`) holds byte masks, the
  bit-reverse table and mask tables used only by this code, followed by the
  GR_PixelString and GR_BitString type descriptors.
- Its `.bss` is used only here.

BlockAllocator.cpp and bmpfile.cpp, which follow it, were already fully
matched.

Exact: 9 calibration cases. These are the constructor, the destructor and
its deleting wrapper, slots 0-3, and the whole-word shifts `0x004238c0`
(`ShiftTowardFront`) and `0x00423940` (`ShiftTowardBack`; provisional names). In `0x004238c0` the fill-only branch counts a separate
`remaining = words` down (counting `words` itself swaps esi and edi). GR_BitString.h drops the inline `~GR_PixelString`. With it,
VC6 gives `~GR_BitString` an EH frame that retail lacks. The two sub-word
helpers and the complete 125-byte reader at `0x00423ef0` sit in the same file
in retail order and are not exact; see [the shifts](BITSTRING_SHIFTS.md) and
[reader and source LZW validation](BITSTRING_READ.md). Its instruction shape
suggests original inline assembly; that source attribution is inference,
not confirmed original source.

## GatherModelTriangles `0x0042c260`: forms tried

Carried over from the header of the former `samples/physics/bvh/BoundingBoxTreeBuildNearMisses.cpp` when the body moved into BoundingBoxTreeBuild.cpp.

```text
PARTIAL 1626/1627: same size and instructions apart from one ModRM/SIB byte: the remap pass
reads keep[corners[k]] as [keep + offset] where VC6 gives [offset + keep].  The loop-variable
names of the merge passes (j outer, k inner, i for the copy) are the ones that put every local
in retail's stack slot; other choices only move slots.
Round 9: `remap = 0;` at the top of the function is what gives retail's reload order before
the 0x348 allocation (numCorners, then remap; without it VC6 reloads remap first, 1622).
Inferred from that order (the cleared value is never read), kept because only remap works.
Clearing any other single local there is inert or worse (keep 1614, k/i 1622, the rest
1618), and clearing keep as well, or numCorners before remap, loses it again (1622).  With it,
for the remaining byte: any second local cleared after remap, the remap pass on i/j/n/t/
first/end/inside/numTaken, a while loop, keep declared before remap, `!= 0`, `*(keep + c)`,
`corners[k][keep]`, `*&keep[...]` and an `int* slot = &keep[corners[k]]` local are all 1626;
`== 1` is 1624, `> 0` 1625, the negated test 1609, `n = remap[...]; if (!keep[...]) n =
remap[n];` 1481, a ternary 1472 and a descending loop 1378.
The older sessions below were measured without `remap = 0;`.
Session 14: for the remap read, *(keep + c), corners[k][keep], != 0, a corner local (t, n, i
or first), a ternary and the negated test leave [offset + keep] or reshape the pass (57+ lines).
For the reload order, numCorners set just before the 0x348 allocation (from numTris, n or
numTaken) or numTris * 3 as the size reshape the function (300+); declaration order does not
matter.  Both spots look like register-allocation choices with no source lever found.
Session 15: walking the corners with a pointer local (`int* corner = corners; ...
keep[*corner]`) in the remap pass keeps [offset + keep] and reshapes the pass (1370/1623).
Session 22: `numCorners = numTris * 3` moved after the corners, remap or positions allocation or
after `n = 0` is worse (1591, 1568, 1523, 401).
Remap-pass index casts ((unsigned), (unsigned long), (long), (int)) and keep/remap declared as
unsigned int*, long* or unsigned long* are inert (1622); (unsigned short) is 1467.  An inline
remap helper is 1622 as a for loop, 1470 with a corner local and 1370 as a pointer walk.
Round 15, with `remap = 0;` (all 1626 unless noted): keep/remap/corners pointer aliases taken
before the loop; each of the other 18 locals cleared before `remap = 0` or after `numTris = 0`
(keep, or numCorners before remap, 1622); `numTris = 0` first; a late-folded flag test at
four positions (inert, scratch only); inline TreeIsKept accessors in either parameter order,
int or bool; `0 !=`, `!!`, `!(== 0)`, `!= false` (`>= 1`/`== true` 1624, `& 1` 1623, `< 1`
1483); a corner local used in one of the three reads (with the test and another read 1478);
an `int* corner = &corners[k]` local in four use patterns; the else arm first 1609, a
two-step remap 1583, the remap read first 1485.
```
