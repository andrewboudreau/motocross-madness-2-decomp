// Near-miss BikeAI.cpp candidates (D:\aardvark\VC\krusty2\BikeAI.cpp), kept out of
// src/krusty2/vehicle/BikeAI.cpp until they match.  The canonical file is included
// first so the TU-local statics and helpers are the same.
//
// 0x0040d200 (4464 bytes, AI jump prediction along the path): strict exact.  The
// frame needed `normal` at function scope (inside the loop body VC6 packs it into
// dir's slot) and the done-block flat speed reusing `flatSpeed`.  Three source forms
// place the last instructions (docs/VC6_OPERAND_ORDER.md section 3):
//  * the segment length is BikeAISegmentLength, whose `(x*x)` parentheses put the
//    `seg.y = 0` store between the x*x and z*z products (BikeAILength's plain form
//    is needed at the takeoff-speed site; one helper for all sites loses the frame
//    slot of that site);
//  * BikeAIDotZ parenthesises y*y, which puts the ground-branch
//    `simPos.y = ground.y` store after the z*z product;
//  * before the loop the segment (single-point path) and the direction are taken
//    from `simPos` rather than from `pos` (the loop does the same), which gives
//    simPos.z an earlier leaf so `simPos += dt * simVel` loads the product's z first
//    (`fld t.z; fadd simPos.z`).
// Tried without effect on those spots: the parentheses on BikeAILength at the other
// sites, by-value/pointer helpers with the plain body, the declaration and
// constructor forms of simPos, ground and target, and 96 forms of `simPos += dt *
// simVel; simPos.y += gravity * halfDtSq; simVel.y += gravity * dt` (operator order,
// parentheses, written-out sums).
//
// 0x0040e510 (1936 bytes, AI path steering direction): every branch, call and
// constant is reconstructed; the candidate is 1935 bytes (retail's extent ends in one
// nop) and aligns 1:1 except the y product's operand order and frame slots.  The walk along the
// path uses its own index (`index = best`): with one variable for the nearest search
// and the walk, VC6 swaps esi/edi against retail (path in esi, index in edi).  The
// three perpendiculars are built component-wise (`perp.x = seg.z; perp.y = 0;
// perp.z = -seg.x; perp = SafeNormalize(perp)`) as in 0x0040d200, which copies seg.z with integer
// moves like retail.  The walk reuses `seg` for its direction (1844 -> 1846 over a
// separate `dir`), and the final step is written per component with the parameter
// assigned in the first product (`step.x = (t = ahead / len + bestT) * seg.x; step.y =
// t * seg.y; ...`): retail keeps the sum on the FPU stack for x (`fst t; fmul x`) and
// reloads t from memory for y and z (1422 -> 1846; `seg * (t = ...)` keeps t on the
// FPU stack for all three, a separate `t = ...;` gives `fstp t; fld x; fmul t`, and a
// `const float&` scale helper gives the same as the latter).  Left: the y product loads
// seg.y before t (retail t first; z already loads t first, seg.z having an older x87
// leaf from `perp.x = seg.z`; operand order, the y/z store order, the step declaration's
// position, a dead `seg.y = 0` after the segment differences and component-wise
// segment differences do not change it; nor do the walk lengths written out in the
// body, by-value or by-pointer length helpers, `seg` read through a reference or
// `(&seg)->y`, by-reference scale helpers for y or y and z, or brackets around single
// products of the step rows, `ahead / len`, the length-helper terms and the outDir sum,
// all 1884) and frame slots.  One perpendicular `perp` at the
// enclosing scope serves both intersection blocks and the final shift, which puts it at
// retail's +0x14 (block-local `perp`s in the two blocks: 1865; hoisting one of them: 1872;
// all three as one: 1884).  Left in the frame: retail pairs `seg` with `step` at +0x20
// and the segment difference temporary with `target` at +0x2c; here `seg` shares
// `target`'s slot and the temporary takes +0x20, with `step` alone at +0x38 (hoisting
// `step`, `target` or `dist`, block-local segments and perp/seg declaration order are
// inert or lose a byte).  The walk's two segment
// lengths use BikeAISegmentLength, as 0x0040d200 does: its `(x*x)` parentheses issue
// the named copy's z store after the z*z product like retail (BikeAILength issues it
// before the first faddp; `(y*y)` parentheses do the same as `(x*x)`, `(z*z)`, a flat
// sum, z first or y first change nothing; 1846 -> 1865).  Declaration order of the Vec3 locals does not
// change VC6's frame (all 24 permutations tried); declaring bestT before best fixed
// the immediate store of bestT = 0.  Also inert on the slots (session 14): a
// per-block `seg`, a `seg` reference alias and splitting the walk direction's
// declaration from its first assignment; block-scoped `t` locals for the two
// intersection tests lose 630 strict bytes.
//
// The inline length helper used here has the same body as the out-of-line copy at
// 0x00413190 (matched in the canonical file as a plain function).  Retail's
// 0x00413190 sits after 0x0040eca0, which is where VC6 emitted the COMDAT when that
// 17 KB function ran out of inline budget, so in the original source it is very
// likely one inline function used in both places.
//
// 0x00415640 (6111 bytes plus one byte of nop padding, obstacle query along a
// segment): 6063/6112 strict positions.  Control flow, calls, constants, every
// instruction's schedule and the 0x1ec frame size match.  Left: the order of the
// one-use operator temporaries above +0x100 in the frame (offset, the dn * k / dn * 10
// / dn * push temporaries of both owner branches, the segment-length temporaries, the
// vegetation position and the outPoint temporary): retail keeps the vegetation `p`
// alone and lets the outPoint temporary share the step / normalise slots; here `p`
// shares the early normalise temporaries.  The start-up zero vectors are a local
// cleared with a chained `zero.x = zero.y = zero.z = 0.0f` (stores z, y, x like
// retail; 6059 -> 6063 over `Vec3(0, 0, 0)`, which stores x first).
// Tried without effect on those: `k * d` / `10.0f * d` / `push * d` in each of the
// six update statements (64 combinations, identical code), parenthesised, integer,
// named-local, per-block local and negated zero vectors; `p` and the outPoint
// temporary at function scope instead of block scope, a named `v` in the segment-
// length blocks and an inline `*outPush += BikeAIVec3Call(...)` instead of the named
// step (32-variant sweep: function-scope `p` 6051, function-scope outPoint
// temporary worse, the others identical).
// 0..17 dead initialised int locals at the top change nothing; nor do the single-value
// `Vec3(0.0f)` zero (same z, y, x stores as the chained form) or block versus function
// scope for the `p` and the operator temporaries' named copies (session 14).
// Frame symbols (/FAs): retail orders the address-taken block locals `tmp`, `v`, `v`,
// `p` where VC6 gives `p`, `v`, `v`, `tmp`, and the six one-use `d * k`/`d * 10`/`d * push`
// results plus `offset` come out in another permutation.  One at a time (session 14,
// after RunSteps): `p` in the vegetation loop body, its hit block, the vegetation block,
// the quadtree block or before the object loop, the outPush step or one `zero` at
// function scope, and either `v` there, all leave 6063; both `v`s at function scope give
// 5919, both `zero`s 5924, `tmp` at function or quadtree-block scope 6005.
// Source forms that mattered: unsigned char type-id statics with an unsigned compare
// (`mov bl, 0xff`), the max/min helpers with by-value arguments (two spellings,
// see BikeAIMaxR), the squared distance and k dot product grouped z + ((x) + y)
// (BikeAISquareMagnitude, BikeAIDot3), SquareMagnitude for the unnormalised distance, hoisting s/u, d,
// distSq and normal out of the owner branches (shared slots), a named `step` for the
// vegetation push, and the call views below for the dot product, the constructor
// and operator*(Vec3, float) that retail calls even in this function's first lines.
//
// 0x0040eca0 (17585 bytes, look-ahead simulation of the two wheels): the whole
// function is reconstructed; frame 0x968 vs 0x93c (see the frame notes below).  The call sequence matches retail call for call except one
// site (0x00412e23: retail inlines operator+ with the out-of-line constructor, the
// candidate calls operator+).  This function exhausts VC6's per-function inline
// budget: the first part expands every Vec3 operator, from 0x00410d00 the operators
// are expanded but their constructor is the out-of-line 0x00404e60, and in the
// epilogue whole operators are called (0x00428090, 0x00421cb0, 0x00421d00,
// 0x005015b0, 0x00413160).  Natural Math3D operators reproduce that pattern; the
// budget is consumed per inline expansion, so `right` and the start-up cross product
// are written out component-wise (a CrossProduct call there moves the switch-over
// points too early).  The loop's `right` cross is built in the named copy `rearCross`
// and assigned whole (aligned 3737 -> 3760 of 3966 retail instructions); `right = Vec3(...)`
// (3723), a constructed named copy (3722) and `right = CrossProduct(fwd, normalR)`
// (3686) do not.  Replacing any DotProduct site with another inline dot helper drops
// to ~3671 (inline budget), and swapping its arguments is inert at every site.  The same
// named copy at dir2's double cross, the loop's flat right, both dir2 rebuilds and sideV's
// inner cross loses (3485..3728).  Written-out dots at sideDotF: 3737 (inline budget).  Remaining differences: the start-up `up` vector and the
// loop-head drag vectors keep their zero components in memory in retail (the
// candidate's are constant-folded), the order of a few temporaries and stores
// (0x00411dbc vF += (commuted operands), 0x00412883), and the frame layout.
//
// The game reads (race kind, tree collision, object root, difficulty, race mode) go
// through the complete TrackGame class, with no view cast: aligned 3731 of 3966, frame
// 0x968 vs 0x93c (the cast views gave 3402).  The read's member-path depth moves the
// inline switch-over although the global loads are identical: a flat member of a plain
// struct pointer gives 3760, a one-level nested member 3402 (frame 0x978), the complete
// two-level TrackGame path 3731 (the 0x00410d00 `aim = fwd +- right` site inlines one more
// constructor and the end site 0x00412e79 then calls operator+).
//
// Frame (tools/frame_layout.py after aba3ceb7; the earlier 0x95c reading is superseded):
// retail's 0x93c holds xf as a full Matrix4 at +0x568 (identity written in the prologue)
// and, just above it, the end site's single-use return buffers (CrossProduct at
// 0x00412b37, operator- at 0x00412eb2).  Declaring the four post-loop ground/normal
// vectors at function scope gives 0x938 with every temporary region at retail's offset
// except that 12-byte slot, but the loop's copy-initialisations become assignments and
// the inline switch-over moves (aligned 3631).  A bare block around the post-loop
// section (groundF..`back -= pen * normalR`) gives 0x92c, or 0x938 with groundF at
// function scope, with identical code; neither gains a strict byte because the ~100
// locals still sort in a different use-count order (front +0x54 vs +0x50, normalF +0x80
// vs +0x70, ...), so neither is kept.  A typedef-prefix scan (k = 0..63 extra
// declarations) is inert (3731 throughout).
//
// 0x00413160 (Vec3::operator-=, 33 bytes) is the out-of-line COMDAT that VC6 emits
// right after 0x0040eca0 for its epilogue; this file reproduces it strict exact.
#include "../../../src/reconstructed/TrackGame.h"
#include "../../../src/krusty2/vehicle/BikeAI.cpp"
#include "vehicle/KrustyBike.h"
#include "../../../src/reconstructed/GearRatios.h"
#include "../../../src/reconstructed/Wrecker.h"
#include "collision/CollisionObject.h"
#include "broadphase/Quadtree.h"
#include <math.h>
#include <stdlib.h>
#include <string.h>

// The AI racing line UnknownBikeAIPath is declared in vehicle/KrustyBikeTypes.h
// (KrustyBike+0x838 holds one).

// 0x0047b800 (cdecl, after gameui.cpp): 2D (x, z) intersection of the line a + s*da
// with the line b + u*db; writes s and u, returns 0 when they do not intersect
// (tier 2 from the callers; name tier 3).
int IntersectLines(float ax, float az, float dax, float daz, float bx, float bz,
                          float dbx, float dbz, float* s, float* u);

// Inline length helper: same body as the out-of-line 0x00413190 (inlined in 0x0040e510
// and 0x00413200).
inline float BikeAILength(const Vec3& v)
{
    float lenSq = (v.x * v.x + v.y * v.y) + v.z * v.z;
    if (lenSq == 0.0f)
        return 0.0f;
    if (lenSq == 1.0f)
        return 1.0f;
    return 1.0f / FastInvSqrt(lenSq);
}

// Length of a path segment, in 0x0040d200 (seg.y is 0 there, so VC6 folds the y*y
// term) and at 0x0040e510's walk.  Same result as BikeAILength; the redundant
// parentheses around x*x are a source-form difference (docs/VC6_OPERAND_ORDER.md
// section 3): with them VC6 emits the preceding store (`seg.y = 0` in 0x0040d200, the
// named copy's z in 0x0040e510) after the x*x product, as retail does at all four
// segment sites, while the takeoff-speed and nearest-point sites (BikeAILength) need the
// plain form.
inline float BikeAISegmentLength(const Vec3& v)
{
    float lenSq = ((v.x * v.x) + v.y * v.y) + v.z * v.z;
    if (lenSq == 0.0f)
        return 0.0f;
    if (lenSq == 1.0f)
        return 1.0f;
    return 1.0f / FastInvSqrt(lenSq);
}

// 0x0040e510, 1936 bytes, cdecl: steering direction along an AI path.  The fourth
// argument is the bike's speed (0x0040eca0 pushes it as a float); it is not read.  Finds the
// path point nearest to 'pos', refines the segment by intersecting the perpendicular
// through 'pos' with the neighbouring segments, walks 'lookAhead' (+ |offset| in race
// kinds 2 and 3) along the path, and returns in *outDir the flat unit direction from
// 'pos' to that point shifted sideways by 'side'.  Stores the reached segment/parameter
// in the path.  Callers: 0x0040f1ef and 0x0041256c.  Names tier 3.
int PathSteeringDirection(Vec3* outDir, Vec3 pos, UnknownBikeAIPath* path, float speed,
                          float side)
{
    if (!path || !outDir || path->count < 1)
        return 0;
    float bestT = 0.0f;
    int best = 0;
    if (path->count >= 2) {
        float bestDist = 1000000.0f;
        int last = path->count - 1;
        for (int i = 0; i < last; i++) {
            float dist = BikeAILength(path->points[i] - pos);
            if (dist < bestDist) {
                bestDist = dist;
                best = i;
            }
        }
        int nearest = best;
        float t;
        float offset;
        Vec3 seg;
        Vec3 perp;
        if (nearest > 0) {
            seg = path->points[nearest] - path->points[nearest - 1];
            perp.x = seg.z;
            perp.y = 0.0f;
            perp.z = -seg.x;
            perp = SafeNormalize(perp);
            if (!IntersectLines(path->points[nearest - 1].x, path->points[nearest - 1].z, seg.x, seg.z,
                                       pos.x, pos.z, perp.x, perp.z, &t, &offset))
                return 0;
            if (t >= 0.0f && t <= 1.0f && offset < bestDist) {
                bestDist = offset;
                best--;
                bestT = t;
            }
        }
        if (nearest < path->count - 1) {
            seg = path->points[nearest + 1] - path->points[nearest];
            perp.x = seg.z;
            perp.y = 0.0f;
            perp.z = -seg.x;
            perp = SafeNormalize(perp);
            if (!IntersectLines(path->points[nearest].x, path->points[nearest].z, seg.x, seg.z,
                                       pos.x, pos.z, perp.x, perp.z, &t, &offset))
                return 0;
            if (t >= 0.0f && t <= 1.0f && offset < bestDist) {
                best = nearest;
                bestT = t;
            }
        }
        int index = best;
        float ahead = path->lookAhead;
        int kind = g_TrackGame->mode.raceSettings.eventType;
        if (kind == 2 || kind == 3)
            ahead += (offset < 0.0f ? -offset : offset);
        seg = path->points[index + 1] - path->points[index];
        float len = (1.0f - bestT) * BikeAISegmentLength(seg);
        while (ahead > 0.0f) {
            if (!(len < ahead) || index >= path->count - 1)
                break;
            ahead -= len;
            index++;
            bestT = 0.0f;
            if (index < path->count - 1) {
                seg = path->points[index + 1] - path->points[index];
                len = BikeAISegmentLength(seg);
            }
        }
        perp.x = seg.z;
        perp.y = 0.0f;
        perp.z = -seg.x;
        perp = SafeNormalize(perp);
        Vec3 target;
        if (index >= path->count - 1) {
            t = 0.0f;
            target = path->points[index];
        } else if (len > ahead) {
            Vec3 step;
            step.x = (t = ahead / len + bestT) * seg.x;
            step.y = t * seg.y;
            step.z = t * seg.z;
            target = path->points[index] + step;
        }
        path->index = index;
        path->t = t;
        *outDir = (target - pos) + perp * side;
        outDir->y = 0.0f;
        if (outDir->x == 0.0f && outDir->z == 0.0f) {
            outDir->z = 1.0f;
            return 1;
        }
        *outDir = SafeNormalize(*outDir);
        return 1;
    }
    Vec3 toFirst = path->points[0] - pos;
    toFirst.y = 0.0f;
    if (outDir->x == 0.0f && outDir->z == 0.0f)
        outDir->z = 1.0f;
    else
        *outDir = SafeNormalize(toFirst);
    path->index = 0;
    path->t = 0.0f;
    return 1;
}

// Debug line endpoints at 0x00578dc0 drawn by BikeRace (BikeRace.h declares the same
// array as Vector3 g_PathTargetDebugLine[2]).
extern Vec3 g_PathTargetDebugLine[2];

// Dot product in the operand order retail 0x0040d200 evaluates: (y + x) + z.  The
// parentheses around y*y place the ground-branch `simPos.y = ground.y` store after
// the z*z product (retail); without them it is issued before the first faddp.
inline float BikeAIDotZ(const Vec3& a, const Vec3& b)
{
    return a.z * b.z + (a.x * b.x + (a.y * b.y));
}

// Call view of the out-of-line Vec3 constructor 0x00404e60 (see math/Math3D.h).
struct BikeAIVec3Call : Vec3 {
    BikeAIVec3Call(float x_, float y_, float z_);
};

// Inline normalize with the same body as Vec3Normalize (0x005087b0); in 0x0040d200 the
// dot product (0x0040ae30) and the constructor (0x00404e60) are out-of-line calls.
inline Vec3 BikeAINormalize(const Vec3& v)
{
    float lenSq = Vec3DotCall(&v, &v);
    if (lenSq == 1.0f)
        return v;
    float invLen = FastInvSqrt(lenSq);
    return BikeAIVec3Call(v.x * invLen, v.y * invLen, v.z * invLen);
}

// 0x0040d200, 4464 bytes, cdecl: predicts the bike's motion along the AI path.
// Starting at 'pos' with the flat speed of 'vel', it steps by 'dt' (gravity -32 when
// more than 0.5 above the ground under it, ground contact removes the velocity into
// the ground normal), advances the path parameter and steers towards the path point
// offset sideways by 'side'.  It records the first takeoff (time, position, velocity,
// flat direction) and stops at the landing (direction change below cos 45 deg, or
// the last path point).  For a jump longer than 20 units it computes the takeoff
// speed that would cover the distance and stores it in path->targetSpeed, raised to
// 40 when below 40, otherwise to takeoffSpeed - 20 when it is more than 20 below the
// takeoff speed.  The takeoff velocity is rescaled to that speed.
// Writes the reached segment/parameter to the path, the flat direction to the
// target in *outDir and the takeoff velocity in *outVel when the takeoff came
// within 0.5 s, and the takeoff time (10 when none) in *outTakeoffTime.  Always
// sets the debug line 0x00578dc0 (pos and target raised by 3).  Returns 0 for a
// missing path/terrain, an index outside the path or dt <= 0, else 1.
// Callers: 0x0040f2c9 and 0x00412654.  Arithmetic tier 1, names tier 3.
int PredictPathMotion(Vec3 pos, Vec3 vel, UnknownBikeAIPath* path, Vec3* outDir,
                          float maxTime, float dt, Terrain* terrain, float side,
                          float* outTakeoffTime, Vec3* outVel)
{
    float elapsed = 0.0f;
    int airborne = 0;
    if (!path || !terrain)
        return 0;
    int idx = path->index;
    if (idx >= path->count || idx < 0)
        return 0;
    if (!(dt > 0.0f))
        return 0;
    int done = 0;
    float takeoffTime = 10.0f;
    float landTimer = -1.0f;
    float t = path->t;
    Vec3 simPos = pos;
    float halfDtSq = dt * dt * 0.5f;
    float flatSpeed = FastSqrt(vel.x * vel.x + vel.z * vel.z);
    Vec3 seg;
    if (path->count == 1)
        seg = path->points[idx] - simPos;
    else if (idx < path->count - 1)
        seg = path->points[idx + 1] - path->points[idx];
    else
        seg = path->points[idx] - path->points[idx - 1];
    seg.y = 0.0f;
    float len = BikeAISegmentLength(seg);
    float invLen = len > 0.0f ? 1.0f / len : 0.0f;
    Vec3 perp;
    perp.x = seg.z;
    perp.y = 0.0f;
    perp.z = -seg.x;
    perp = SafeNormalize(perp);
    Vec3 target = path->points[idx] + seg * t + perp * side;
    Vec3 dir = target - simPos;
    dir.y = 0.0f;
    dir = SafeNormalize(dir);
    Vec3 simVel(dir.x * flatSpeed, vel.y, dir.z * flatSpeed);
    float align = 1.0f;
    Vec3 normal;
    Vec3 takeoffVel;
    float takeoffSpeed;
    Vec3 takeoffPos;
    Vec3 flatDir;
    while ((elapsed < maxTime || airborne) && !done) {
        float speedSq = (simVel.x * simVel.x + simVel.y * simVel.y) + simVel.z * simVel.z;
        float speed = speedSq == 1.0f ? 1.0f : FastSqrt(speedSq);
        flatSpeed = FastSqrt(simVel.x * simVel.x + simVel.z * simVel.z);
        Vec3 ground = simPos;
        terrain->QueryGround((TerrainVec3*)&ground, (TerrainVec3*)&normal, 0, 0);
        float gravity;
        if (ground.y + 0.5f >= simPos.y) {
            if (airborne && takeoffTime < 0.5f) {
                target = simPos;
                landTimer = 0.0f;
            }
            float intoGround = BikeAIDotZ(normal, simVel);
            simPos.y = ground.y;
            if (intoGround < 0.0f) {
                simVel -= normal * intoGround;
                if (!(intoGround < speed * -0.70755f))
                    simVel = SafeNormalize(simVel) * speed;
            }
            gravity = 0.0f;
            airborne = 0;
        } else {
            if (!airborne) {
                takeoffVel = simVel;
                takeoffSpeed = BikeAILength(simVel);
                takeoffPos = simPos;
                flatDir = simVel;
                flatDir.y = 0.0f;
                flatDir = BikeAINormalize(flatDir);
                takeoffTime = elapsed;
            } else if (align < 0.70755f) {
                done = 1;
                target = simPos;
            }
            gravity = -32.0f;
            airborne = 1;
        }
        simPos += dt * simVel;
        simPos.y += gravity * halfDtSq;
        simVel.y += gravity * dt;
        t += invLen * flatSpeed * dt;
        flatSpeed = FastSqrt(simVel.x * simVel.x + simVel.z * simVel.z);
        while (t > 1.0f && idx < path->count - 1) {
            t -= 1.0f;
            idx++;
            if (path->count == 1)
                seg = path->points[idx] - simPos;
            else if (idx < path->count - 1)
                seg = path->points[idx + 1] - path->points[idx];
            else
                seg = path->points[idx] - path->points[idx - 1];
            seg.y = 0.0f;
            len = BikeAISegmentLength(seg);
            invLen = len > 0.0f ? 1.0f / len : 0.0f;
            perp.x = seg.z;
            perp.y = 0.0f;
            perp.z = -seg.x;
            perp = SafeNormalize(perp);
        }
        Vec3 aim = path->points[idx] + seg * t + perp * side;
        if (airborne && idx == path->count - 1) {
            target = simPos;
            done = 1;
        }
        flatSpeed = FastSqrt(simVel.x * simVel.x + simVel.z * simVel.z);
        dir = aim - simPos;
        dir.y = 0.0f;
        dir = SafeNormalize(dir);
        simVel.x = dir.x * flatSpeed;
        simVel.z = dir.z * flatSpeed;
        if (airborne)
            align = BikeAIDotZ(seg * invLen, flatDir);
        else
            align = 1.0f;
        if (landTimer >= 0.0f)
            landTimer += dt;
        elapsed += dt;
    }
    if (done) {
        float dy = target.y - takeoffPos.y;
        Vec3 toLanding = target - takeoffPos;
        float dist = FastSqrt(toLanding.x * toLanding.x + toLanding.z * toLanding.z);
        flatSpeed = FastSqrt(takeoffVel.x * takeoffVel.x + takeoffVel.z * takeoffVel.z);
        if (dist > 20.0f) {
            float sinA, secA;   // left unset when the takeoff has no flat speed (as retail)
            if (flatSpeed > 0.0f) {
                float spd = BikeAILength(takeoffVel);
                sinA = takeoffVel.y / spd;
                secA = spd / flatSpeed;
            }
            path->targetSpeed = FastSqrt(dist * dist * 32.0f / ((sinA * dist + dy) * 2.0f)) * secA;
            float need = path->targetSpeed;
            Vec3 nv = SafeNormalize(takeoffVel) * need;
            if (path->targetSpeed < 40.0f)
                path->targetSpeed = 40.0f;
            else if (takeoffSpeed - path->targetSpeed > 20.0f)
                path->targetSpeed = takeoffSpeed - 20.0f;
            takeoffVel = SafeNormalize(nv) * path->targetSpeed;
        }
    }
    path->index = idx;
    path->t = t;
    if (outDir && takeoffTime < 0.5f) {
        *outDir = target - pos;
        outDir->y = 0.0f;
        *outDir = SafeNormalize(*outDir);
    }
    if (outTakeoffTime)
        *outTakeoffTime = takeoffTime;
    if (outVel && takeoffTime < 0.5f)
        *outVel = takeoffVel;
    g_PathTargetDebugLine[0] = pos;
    g_PathTargetDebugLine[0].y = pos.y + 3.0f;
    g_PathTargetDebugLine[1] = target;
    g_PathTargetDebugLine[1].y = target.y + 3.0f;
    return 1;
}

// ---------------------------------------------------------------------------
// 0x00415640 and its helpers.
// ---------------------------------------------------------------------------

// Type-id registry (TypeRegistry.cpp, stand-in as in CollisionObject.cpp).
class TypeRegistry {
public:
    char FindTypeId(const char* name);           // 0x00521ea0
};
extern TypeRegistry* g_TypeRegistry;             // 0x00575744

// gameobj.cpp's iterator (src/reconstructed/GameObjectIterator.h; local stand-in, 0x94 bytes).
class GameObjectIterator {
public:
    GameObjectIterator(GameObject* root, int mode, const char* filter);   // 0x00469950
    ~GameObjectIterator();                                                  // 0x00469a40
    GameObject* Next();                                                     // 0x00469a50
private:
    char field_0x00[0x94];
};

// Vegetation patch seen through the quadtree (CollisionVegetation in CollisionObject.cpp).
class BikeAIVegetation : public QuadTreeObject {
public:
    int GetObjectCount();                     // 0x00457080
    CollisionObject* GetObject(int index);    // 0x004570a0
};

// CollisionObject.cpp: 0x00439820 (segment) and 0x00439600 (capsule, near miss in
// samples/physics/collision) object tests.
int SegmentTouchesObject(const CollisionVec3* ends, CollisionObject* object, CollisionVec3* outPoint,
                         CollisionVec3* outNormal);
int Fn_00439600(CollisionVec3* ends, float radius, float radiusSq, CollisionObject* object);

// Length / normalize shapes of 0x00415640 and 0x0040eca0 (see the file header): the dot
// product 0x0040ae30 and, in the normalizes, the constructor 0x00404e60 are out-of-line.
inline float BikeAILengthCall(const Vec3& v)
{
    float lenSq = Vec3DotCall(&v, &v);
    if (lenSq == 0.0f)
        return 0.0f;
    if (lenSq == 1.0f)
        return 1.0f;
    return 1.0f / FastInvSqrt(lenSq);
}
inline Vec3 BikeAINormalizeSV(const Vec3& v)
{
    float lenSq = Vec3DotCall(&v, &v);
    if (lenSq == 1.0f)
        return v;
    float invLen = FastInvSqrt(lenSq);
    return BikeAIVec3Call(invLen * v.x, invLen * v.y, invLen * v.z);
}

// Squared distance and dot product of the owner branches of 0x00415640, grouped
// z + ((x) + y): retail sums (x + y) + z and issues the dot-call argument pushes after
// the z*z term.  Math3D's flat SquareMagnitude / DotProduct give (x + z) + y with the
// pushes before it (72 bytes).  Both helpers need the parenthesised x product.
inline float BikeAISquareMagnitude(const Vec3& v)
{
    return v.z * v.z + ((v.x * v.x) + v.y * v.y);
}

inline float BikeAIDot3(const Vec3& a, const Vec3& b)
{
    return a.z * b.z + ((a.x * b.x) + a.y * b.y);
}

// Max / min with by-value arguments.  Retail's four bounds of the query box need the two
// comparison spellings (0x00415c8f..0x00415cb5); the arguments are swapped in the call.
inline float BikeAIMax(float a, float b)
{
    return a > b ? a : b;
}
inline float BikeAIMaxR(float a, float b)
{
    return b > a ? b : a;
}
inline float BikeAIMinR(float a, float b)
{
    return b < a ? b : a;
}

inline Vec3 BikeAINormalizeVS(const Vec3& v)
{
    float lenSq = Vec3DotCall(&v, &v);
    if (lenSq == 1.0f)
        return v;
    float invLen = FastInvSqrt(lenSq);
    return BikeAIVec3Call(v.x * invLen, v.y * invLen, v.z * invLen);
}
// By-value view of operator*(const Vec3&, float) 0x005015b0.
Vec3 BikeAIVec3Scale(const Vec3& v, float s);
inline Vec3 BikeAINormalizeScaleCall(const Vec3& v)
{
    float lenSq = Vec3DotCall(&v, &v);
    if (lenSq == 1.0f)
        return v;
    float invLen = FastInvSqrt(lenSq);
    return BikeAIVec3Scale(v, invLen);
}

// Owners of the obstacle collision objects (CollisionObject::ownerObject) by ownerType:
// 0x64 / 0x65 a segment (start +0x0c, direction +0x64), 0x69 a direction at +0x40.
// Offsets tier 1 (0x00415db6, 0x00416732), roles tier 3.
struct UnknownBikeAISegmentOwner {
    char pad_0x00[0x0c];
    Vec3 start;          // +0x0c
    char pad_0x18[0x4c];
    Vec3 dir;            // +0x64
};
struct UnknownBikeAIPointOwner {
    char pad_0x00[0x40];
    Vec3 dir;            // +0x40
};

// Cached type ids (.data, initialised to 0xff), compared as unsigned bytes.
static unsigned char s_BikeAICollisionObjectTypeId = 0xff;   // 0x00566f70
static unsigned char s_BikeAIVegetationTypeId = 0xff;        // 0x00566f71

// Debug segments and hit flags drawn by BikeRace slot 14 (BikeRace.h declares them).
extern int g_ObstacleDebugUpperHit;
extern int g_ObstacleDebugHit;
extern int g_ObstacleDebugLowerHit;
extern int g_ObstacleDebugLeftHit;
extern int g_ObstacleDebugRightHit;
extern Vec3 g_ObstacleDebugUpperLine[2];
extern Vec3 g_ObstacleDebugLine[2];
extern Vec3 g_ObstacleDebugLowerLine[2];
extern Vec3 g_ObstacleDebugLeftLine[2];
extern Vec3 g_ObstacleDebugRightLine[2];

// 0x00415640, 6112 bytes, cdecl, EH frame (debug new of the iterator, line 0xb24):
// obstacle query along the segment seg[0]..seg[1] with the given radius.  Records the
// debug segments (raised by 3, lowered by 1.5, shifted sideways by +-3) for BikeRace's
// overlay.  With the collision quadtree it visits the CollisionObjects in the box around
// the segment (skipping ignoreList): segment owners (0x64/0x65) and direction owners
// (0x69) steer `acc` (starts at dir * push) away by the 2D crossing test 0x0047b800;
// other objects run the capsule test 0x00439600 and the five segment tests, keeping the
// nearest hit within 10 (or any hit unless nearOnly) and adding it to *outPush.
// Vegetation patches add a quarter of the offset to the patch object.  Without the
// quadtree it only runs the capsule test over all CollisionObjects.  Outputs: *outDist
// (nearest hit, 1000 when none), *outPush normalised (zero when flat), *outPoint = acc -
// dir * push.  Returns whether anything was hit.  Caller: 0x00410a43.  Names tier 3.
int QuerySegmentObstacles(Vec3* seg, float radius, int skipVegetation, int ignoreCount,
                          CollisionObject** ignoreList, float push, Vec3* outPush,
                          Vec3* outPoint, float* outDist, int nearOnly)
{
    float radiusSq = radius * radius;
    int hit = 0;
    if (s_BikeAICollisionObjectTypeId == 0xff)
        s_BikeAICollisionObjectTypeId = g_TypeRegistry->FindTypeId("CollisionObject");
    if (s_BikeAIVegetationTypeId == 0xff)
        s_BikeAIVegetationTypeId = g_TypeRegistry->FindTypeId("Vegetation");
    float minDist = 1000.0f;
    Vec3 delta = seg[1] - seg[0];
    Vec3 side;
    side.x = delta.z;
    side.y = 0.0f;
    side.z = -delta.x;
    side = BikeAINormalizeSV(side);
    Vec3 dir = BikeAINormalizeSV(delta);
    Vec3 offset = dir * push;
    Vec3 acc = offset;

    Vec3 up[2];
    up[0] = seg[0];
    up[0].y += 3.0f;
    up[1] = seg[1];
    up[1].y += 3.0f;
    Vec3 low[2];
    low[0] = seg[0];
    low[0].y -= 1.5f;
    low[1] = seg[1];
    low[1].y -= 1.5f;
    Vec3 left[2];
    left[0] = seg[0] - side * 3.0f;
    left[1] = seg[1] - side * 3.0f;
    Vec3 right[2];
    right[0] = seg[0] + side * 3.0f;
    right[1] = seg[1] + side * 3.0f;
    g_ObstacleDebugUpperLine[0] = up[0];
    g_ObstacleDebugUpperLine[1] = up[1];
    g_ObstacleDebugLine[0] = seg[0];
    g_ObstacleDebugLine[1] = seg[1];
    g_ObstacleDebugLowerLine[0] = low[0];
    g_ObstacleDebugLowerLine[1] = low[1];
    g_ObstacleDebugLeftLine[0] = left[0];
    g_ObstacleDebugLeftLine[1] = left[1];
    g_ObstacleDebugRightLine[0] = right[0];
    g_ObstacleDebugRightLine[1] = right[1];
    g_ObstacleDebugUpperHit = 0;
    g_ObstacleDebugHit = 0;
    g_ObstacleDebugLowerHit = 0;
    g_ObstacleDebugLeftHit = 0;
    g_ObstacleDebugRightHit = 0;
    if (outPush) {
        Vec3 zero;
        zero.x = zero.y = zero.z = 0.0f;
        *outPush = zero;
    }
    if (outPoint) {
        Vec3 zero;
        zero.x = zero.y = zero.z = 0.0f;
        *outPoint = zero;
    }

    Vec3 best;
    Vec3 pt;
    float s, u;
    Vec3 d;
    float distSq;
    Vec3 normal;
    if (g_collisionQuadTree) {
        float maxZ = BikeAIMax(seg[0].z, seg[1].z);
        float maxX = BikeAIMaxR(seg[1].x, seg[0].x);
        float minZ = BikeAIMinR(seg[1].z, seg[0].z);
        float minX = BikeAIMinR(seg[1].x, seg[0].x);
        g_collisionQuadTree->BeginQuery(minX - radius, minZ - radius, maxX + radius, maxZ + radius);
        for (QuadTreeObject* o = g_collisionQuadTree->NextObject(); o; o = g_collisionQuadTree->NextObject()) {
            if ((unsigned char)o->objectTypeId == s_BikeAICollisionObjectTypeId) {
                CollisionObject* obj = (CollisionObject*)o;
                int ignored = 0;
                for (int i = 0; i < ignoreCount; i++)
                    if (obj == ignoreList[i])
                        ignored = 1;
                if (ignored)
                    continue;
                obj->queryTouched = 1;
                if (obj->ownerType == 0x64 || obj->ownerType == 0x65) {
                    UnknownBikeAISegmentOwner* owner = (UnknownBikeAISegmentOwner*)obj->ownerObject;
                    d = owner->start - seg[0];
                    distSq = BikeAISquareMagnitude(d);
                    d = BikeAINormalizeVS(d);
                    IntersectLines(seg[0].x, seg[0].z, delta.x, delta.z, owner->start.x, owner->start.z,
                                          owner->dir.x, owner->dir.z, &s, &u);
                    if (s >= 0.0f && s <= 1.0f && u >= 0.0f && u <= 1.0f) {
                        hit = 1;
                        pt = owner->dir - acc;
                        float k = BikeAIDot3(pt, d);
                        if (k < 0.0f)
                            acc += d * k;
                    } else if (distSq < 100.0f && DotProduct(acc, d) > 0.0f) {
                        if (push > 10.0f)
                            acc -= d * 10.0f;
                        else
                            acc -= d * push;
                    }
                } else if (obj->ownerType == 0x69) {
                    UnknownBikeAIPointOwner* owner = (UnknownBikeAIPointOwner*)obj->ownerObject;
                    Vec3* center = (Vec3*)&obj->boundCenter;
                    d = *center - seg[0];
                    distSq = BikeAISquareMagnitude(d);
                    d = BikeAINormalizeVS(d);
                    IntersectLines(seg[0].x, seg[0].z, delta.x, delta.z, center->x, center->z,
                                          owner->dir.x, owner->dir.z, &s, &u);
                    if (s >= 0.0f && s <= 1.0f && u >= 0.0f && u <= 1.0f) {
                        hit = 1;
                        pt = owner->dir - acc;
                        float k = BikeAIDot3(pt, d);
                        if (k < 0.0f)
                            acc += d * k;
                    } else if (distSq < 100.0f && DotProduct(acc, d) > 0.0f) {
                        if (push > 10.0f)
                            acc -= d * 10.0f;
                        else
                            acc -= d * push;
                    }
                } else if (Fn_00439600((CollisionVec3*)seg, radius, radiusSq, obj)) {
                    if (SegmentTouchesObject((CollisionVec3*)up, obj, (CollisionVec3*)&pt, (CollisionVec3*)&normal)) {
                        float len = BikeAILength(pt - seg[0]);
                        if (!(len > 10.0f && nearOnly)) {
                            if (len < minDist) {
                                best = pt;
                                minDist = len;
                            }
                            g_ObstacleDebugUpperHit = 1;
                        }
                    }
                    if (SegmentTouchesObject((CollisionVec3*)seg, obj, (CollisionVec3*)&pt, (CollisionVec3*)&normal)
                        && normal.y < 0.70755f) {
                        float len = BikeAILength(pt - seg[0]);
                        if (!(len > 10.0f && nearOnly)) {
                            if (len < minDist) {
                                best = pt;
                                minDist = len;
                            }
                            g_ObstacleDebugHit = 1;
                        }
                    }
                    if (SegmentTouchesObject((CollisionVec3*)low, obj, (CollisionVec3*)&pt, (CollisionVec3*)&normal)
                        && normal.y < 0.70755f) {
                        float len = BikeAILength(pt - seg[0]);
                        if (!(len > 10.0f && nearOnly)) {
                            if (len < minDist) {
                                best = pt;
                                minDist = len;
                            }
                            g_ObstacleDebugLowerHit = 1;
                        }
                    }
                    if (SegmentTouchesObject((CollisionVec3*)left, obj, (CollisionVec3*)&pt, (CollisionVec3*)&normal)
                        && normal.y < 0.70755f) {
                        Vec3 v = pt - seg[0];
                        float len = BikeAILengthCall(v);
                        if (!(len > 10.0f && nearOnly)) {
                            if (len < minDist) {
                                best = pt;
                                minDist = len;
                            }
                            g_ObstacleDebugLeftHit = 1;
                        }
                    }
                    if (SegmentTouchesObject((CollisionVec3*)right, obj, (CollisionVec3*)&pt, (CollisionVec3*)&normal)
                        && normal.y < 0.70755f) {
                        Vec3 v = pt - seg[0];
                        float len = BikeAILengthCall(v);
                        if (!(len > 10.0f && nearOnly)) {
                            if (len < minDist) {
                                best = pt;
                                minDist = len;
                            }
                            g_ObstacleDebugRightHit = 1;
                        }
                    }
                    if (g_ObstacleDebugUpperHit || g_ObstacleDebugHit || g_ObstacleDebugLowerHit
                        || g_ObstacleDebugLeftHit || g_ObstacleDebugRightHit) {
                        hit = 1;
                        if (outPush && (minDist < 10.0f || !nearOnly)) {
                            pt = best - seg[0];
                            *outPush += pt;
                        }
                    }
                }
            } else if (!skipVegetation && (unsigned char)o->objectTypeId == s_BikeAIVegetationTypeId) {
                BikeAIVegetation* veg = (BikeAIVegetation*)o;
                for (int i = 0; i < veg->GetObjectCount(); i++) {
                    CollisionObject* obj = veg->GetObject(i);
                    if (Fn_00439600((CollisionVec3*)seg, radius, radiusSq, obj)) {
                        hit = 1;
                        if (outPush) {
                            if (minDist < 10.0f) {
                                pt = best - seg[0];
                            } else {
                                Vec3 p;
                                obj->GetShapePosition((CollisionVec3*)&p);
                                pt = BikeAIVec3Call(p.x - seg[0].x, p.y - seg[0].y, p.z - seg[0].z);
                            }
                            Vec3 step = BikeAIVec3Call(pt.x * 0.25f, pt.y * 0.25f, pt.z * 0.25f);
                            *outPush += step;
                        }
                    }
                }
            }
        }
        g_collisionQuadTree->EndQuery();
    } else {
        GameObjectIterator* it = new(__FILE__, 0xb24) GameObjectIterator(g_TrackGame->childRoot, 1, "CollisionObject");
        CollisionObject* obj;
        while ((obj = (CollisionObject*)it->Next()) != 0) {
            if (!obj->shape)
                continue;
            int ignored = 0;
            for (int i = 0; i < ignoreCount; i++)
                if (obj == ignoreList[i])
                    ignored = 1;
            if (ignored)
                continue;
            if (Fn_00439600((CollisionVec3*)seg, radius, radiusSq, obj))
                hit = 1;
        }
        delete it;
    }
    if (outDist)
        *outDist = minDist;
    if (outPush) {
        if (outPush->x == 0.0f && outPush->z == 0.0f) {
            *outPush = Vec3(0.0f, 0.0f, 0.0f);
        } else {
            *outPush = BikeAINormalizeScaleCall(*outPush);
        }
    }
    if (outPoint) {
        Vec3 tmp;
        *outPoint = *Vec3SubtractCall(&tmp, &acc, &offset);
    }
    return hit;
}


// ---------------------------------------------------------------------------
// 0x0040eca0.
// ---------------------------------------------------------------------------

// 0x0040eca0, 17585 bytes, cdecl, 22 arguments: simulates the bike as two wheel points
// (front = center + fwd * 3.5, back = center - fwd * 3.5) for maxTime seconds in steps of
// dt (at most 0.05).  Per step: terrain and probe-object ground under each wheel, spring
// (k = 90) and damper on the suspension travel, velocity reflection on contact, AI aim
// along the path (0x0040e510 / 0x0040d200) or the steering input, obstacle avoidance
// through 0x00415640, target speed from path->targetSpeed and the speed table, sharing
// of the longitudinal acceleration (0.65 / 0.35), integration, and the side axis leaning
// towards the lateral acceleration.  Writes back center, forward, side, both wheel
// velocities, their mean and its length, the suspension values, the contact flags, the
// airborne flag and the largest impact.  Returns 0 when the jump prediction fails, else
// 1.  Caller: KrustyBike 0x00413a0c.  Arithmetic tier 1 (decoded), names tier 3.
// Both 0x0040d200 calls pass 0.05f as the sixth argument (retail pushes 0x3d4ccccd at
// +0x5cd and +0x3951; an earlier 0.1f was a semantic slip).
int SimulateLookAhead(Vec3* steer, Vec3* frontVel, Vec3* rearVel, Vec3* outVel, float* outSpeed,
                          Vec3* center, Vec3* forward, Vec3* side, float maxTime, float dt,
                          Terrain* terrain, unsigned char* outAirborne, float* outImpact, CollisionObject* probe,
                          float* frontSusp, float* rearSusp, int* outFrontContact, int* outRearContact,
                          UnknownBikeAIPath* path, float* speedTable, float sideOffset, int nearOnly)
{
    int frontContact = 1;
    int rearContact = 1;
    float frontK = 90.0f;
    float rearK = 90.0f;
    int steerSign = 0;
    int steps = 0;
    if (dt > 0.05f)
        dt = 0.05f;
    float frontC;
    float rearC;
    frontC = rearC = ((dt - 0.005f) * 4.21053f + 0.6f) * 0.6f;
    float speedScale = path ? path->accelLimitScale : 1.5f;
    float halfDtSq = dt * dt * 0.5f;
    Matrix4 xf;
    memset(&xf, 0, sizeof(xf));
    xf._11 = 1.0f;
    xf._22 = 1.0f;
    xf._33 = 1.0f;
    xf._44 = 1.0f;
    float invDt = 1.0f / dt;
    Vec3 fwd = *forward;
    Vec3 sideV;
    Vec3 up;
    up = Vec3(0.0f, 0.0f, 0.0f);
    up.y = 1.0f;
    sideV = side ? *side : SafeNormalize(Vec3(up.y * fwd.z - up.z * fwd.y, up.z * fwd.x - up.x * fwd.z, up.x * fwd.y - up.y * fwd.x));
    Vec3 front = *center + fwd * 3.5f;
    Vec3 vF = *frontVel;
    Vec3 right;
    right.x = fwd.z;
    right.y = 0.0f;
    right.z = -fwd.x;
    right = SafeNormalize(right);
    Vec3 sideF = sideV;
    Vec3 sideR = sideV;
    Vec3 back = *center - fwd * 3.5f;
    Vec3 vR = *rearVel;
    float rearS;
    if (rearSusp) {
        rearS = *rearSusp;
    } else {
        rearS = 0.0f;
        rearC = 0.0f;
        rearK = 0.0f;
    }
    float frontS;
    if (frontSusp) {
        frontS = *frontSusp;
    } else {
        frontS = 0.0f;
        frontC = 0.0f;
        frontK = 0.0f;
    }
    Vec3 aim;
    Vec3 dir2;
    Vec3 takeoffVel;
    float takeoffTime;
    float sinA;
    float cosA;
    if (path) {
        float speed = BikeAILengthCall(vF);
        if (!PathSteeringDirection(&aim, front, path, speed, sideOffset)) {
            aim = vF;
            aim.y = 0.0f;
            aim = SafeNormalize(aim);
        }
        takeoffVel = vF;
        if (!PredictPathMotion(front, vF, path, &aim, 0.5f, 0.05f, terrain, sideOffset, &takeoffTime,
                                   &takeoffVel))
            return 0;
        if (DotProduct(aim, fwd) < 0.70755f) {
            if (DotProduct(aim, right) < 0.0f)
                aim = SafeNormalize(fwd - right);
            else
                aim = SafeNormalize(fwd + right);
        }
        dir2.x = aim.z;
        dir2.y = 0.0f;
        dir2.z = -aim.x;
    } else if (steer) {
        if (steer->z > 0.0f)
            steerSign = 1;
        else if (steer->z < 0.0f)
            steerSign = -1;
        else
            steerSign = 0;
        float angle = steer->x * -0.7854f;
        cosA = (float)cos(angle);
        sinA = (float)sin(angle);
        dir2.x = sinA * right.z + cosA * right.x;
        dir2.y = 0.0f;
        dir2.z = cosA * right.z - sinA * right.x;
        dir2 = SafeNormalize(dir2);
    } else {
        dir2 = right;
    }

    int airborne;
    float t = 0.0f;
    if (0.0f < maxTime) {
        Vec3 rearForce = Vec3(0.0f, 0.0f, 0.0f);
        Vec3 frontForce = Vec3(0.0f, 0.0f, 0.0f);
        do {
            sideF = sideV;
            sideR = sideV;
            Vec3 accR = rearForce;
            Vec3 rearDrag = Vec3(0.0f, 0.0f, 0.0f);
            accR.y = -32.0f;
            accR -= rearDrag;
            Vec3 accF = frontForce;
            Vec3 frontDrag = Vec3(0.0f, 0.0f, 0.0f);
            accF.y = -32.0f;
            accF -= frontDrag;
            Vec3 groundR = back;
            Vec3 groundF = front;
            float frontBrake = 0.0f;
            float impact = 0.0f;
            float rearBrake = 0.0f;
            Vec3 normalF;
            Vec3 normalR;
            if (terrain) {
                terrain->QueryGround((TerrainVec3*)&groundF, (TerrainVec3*)&normalF, 0, 0);
                terrain->QueryGround((TerrainVec3*)&groundR, (TerrainVec3*)&normalR, 0, 0);
            } else {
                normalF.x = 0.0f;
                normalF.y = 1.0f;
                normalF.z = 0.0f;
                normalR.x = 0.0f;
                normalR.y = 1.0f;
                normalR.z = 0.0f;
            }
            if (probe) {
                xf._41 = front.x;
                xf._42 = front.y;
                xf._43 = front.z;
                probe->SetTransform(&xf);
                probe->QueryCollisions();
                if (probe->hasContact) {
                    float* rec = (float*)probe->contactRecord;
                    float probeY = front.y + 4.0f - rec[0] * 5.0f;
                    probe->hitPoint.y = probeY;
                    if (probeY > groundF.y) {
                        groundF.y = probeY;
                        normalF = *(Vec3*)(rec + 2);
                    }
                }
                xf._41 = back.x;
                xf._42 = back.y;
                xf._43 = back.z;
                probe->SetTransform(&xf);
                probe->QueryCollisions();
                if (probe->hasContact) {
                    float* rec = (float*)probe->contactRecord;
                    float probeY = back.y + 4.0f - rec[0] * 5.0f;
                    probe->hitPoint.y = probeY;
                    if (probeY > groundR.y) {
                        groundR.y = probeY;
                        normalR = *(Vec3*)(rec + 2);
                    }
                }
            }
            Vec3 rearCross;
            rearCross.x = fwd.y * normalR.z - fwd.z * normalR.y;
            rearCross.y = fwd.z * normalR.x - fwd.x * normalR.z;
            rearCross.z = fwd.x * normalR.y - fwd.y * normalR.x;
            right = rearCross;
            if (right.x != 0.0f || right.y != 0.0f || right.z != 0.0f)
                right = SafeNormalize(right);
            dir2 = CrossProduct(CrossProduct(dir2, normalF), normalF);
            if (dir2.x != 0.0f || dir2.y != 0.0f || dir2.z != 0.0f)
                dir2 = SafeNormalize(dir2);
            float speedF = BikeAILengthCall(vF);
            float speedR = BikeAILengthCall(vR);

            // Front wheel.
            float kF = frontS * frontK;
            float sideDotF = DotProduct(sideF, normalF);
            Vec3 contactF = front + sideF * frontS;
            float pen = DotProduct(contactF - groundF, normalF);
            float dampF;
            float rollF;
            if (pen <= 0.5f) {
                float vn = DotProduct(vF, normalF);
                dampF = vn * sideDotF * frontC;
                if (vn < 0.0f) {
                    impact = vn;
                    if (frontS < 1.0f)
                        vF -= normalF * vn - sideF * dampF;
                    else
                        vF -= normalF * vn;
                    if (BikeAILengthCall(vF) < speedF)
                        vF = vF * 0.9f + SafeNormalize(vF) * speedF * 0.1f;
                }
                if (pen < 0.0f)
                    front -= normalF * pen;
                float an = DotProduct(accF, normalF);
                rollF = an * sideDotF;
                if (an < 0.0f) {
                    frontBrake = an * -8.0f;
                    accF -= normalF * an;
                }
                float sideSpring = sideDotF * kF;
                accF += sideF * (sideSpring + rollF);
                frontS -= sideSpring * halfDtSq + dampF * dt;
                if (frontS < 0.0f) {
                    frontS = 0.0f;
                } else if (frontS > 1.0f) {
                    frontS = 1.0f;
                }
                frontContact = 1;
            } else {
                frontS -= kF * halfDtSq * 3.0f;
                frontContact = 0;
                if (frontS < 0.0f)
                    frontS = 0.0f;
            }

            // Rear wheel.
            float kR = rearS * rearK;
            float sideDotR = DotProduct(normalR, sideR);
            Vec3 contactR = back + sideR * rearS;
            pen = DotProduct(contactR - groundR, normalR);
            float dampR;
            float rollR;
            if (pen <= 0.5f) {
                float vn = DotProduct(vR, normalR);
                dampR = vn * sideDotR * rearC;
                if (vn < 0.0f) {
                    impact += vn;
                    if (rearS < 1.0f)
                        vR -= normalR * vn - sideR * dampR;
                    else
                        vR -= normalR * vn;
                    if (BikeAILengthCall(vR) < speedR)
                        vR = vR * 0.9f + SafeNormalize(vR) * speedR * 0.1f;
                }
                if (pen < 0.0f)
                    back -= normalR * pen;
                float an = DotProduct(accR, normalR);
                rollR = an * sideDotR;
                if (an < 0.0f) {
                    rearBrake = an * -100.0f;
                    accR -= normalR * an;
                }
                float sideSpring = sideDotR * kR;
                accR += sideR * (sideSpring + rollR);
                rearS -= sideSpring * halfDtSq + dampR * dt;
                if (rearS < 0.0f) {
                    rearS = 0.0f;
                } else if (rearS > 1.0f) {
                    rearS = 1.0f;
                }
                rearContact = 1;
            } else {
                rearS -= kR * halfDtSq * 3.0f;
                rearContact = 0;
                if (rearS < 0.0f)
                    rearS = 0.0f;
            }

            airborne = !frontContact && !rearContact;
            if (!frontContact) {
                aim = vF;
                aim.y = 0.0f;
                aim = SafeNormalize(aim);
            }
            if (frontContact || rearContact) {
                Vec3 flatVel = vF;
                flatVel.y = 0.0f;
                SafeNormalize(flatVel);
                Vec3 seg[2];
                seg[0] = front;
                seg[0].y += 3.0f;
                seg[1] = front + (vR + vF) * 0.25f;
                seg[1].y += 3.0f;
                Vec3 push;
                Vec3 avoidPoint;
                float avoidDist;
                if (QuerySegmentObstacles(seg, 2.0f, g_TrackGame->mode.raceSettings.treeCollision == 0, probe->ignoreCount,
                                          (CollisionObject**)probe->ignoreList, speedF, &push, &avoidPoint,
                                          &avoidDist, nearOnly)
                    && !(push.x == 0.0f && push.z == 0.0f)) {
                    FastSqrt(vF.x * vF.x + vF.z * vF.z);
                    push.y = 0.0f;
                    push = SafeNormalize(push);
                    float pushSpeed = DotProduct(vF, push);
                    if (pushSpeed > 0.0f) {
                        aim = vF - push * pushSpeed;
                        aim.y = 0.0f;
                        aim = SafeNormalize(aim);
                    }
                }
                Vec3 flatFwd = fwd;
                flatFwd.y = 0.0f;
                flatFwd = SafeNormalize(flatFwd);
                if (DotProduct(flatFwd, aim) < 0.70755f) {
                    if (DotProduct(aim, right) > 0.0f)
                        aim = (fwd + right);
                    else
                        aim = (fwd - right);
                    aim.y = 0.0f;
                    aim = SafeNormalize(aim);
                }
                int kind = g_TrackGame->mode.raceSettings.eventType;
                if (kind == 5 || kind == 1) {
                    flatFwd = vF;
                    flatFwd.y = 0.0f;
                    flatFwd = SafeNormalize(flatFwd);
                    aim = SafeNormalize(flatFwd * 5.0f + aim);
                }
                float flatSpeed = FastSqrt(vF.x * vF.x + vF.z * vF.z);
                Vec3 dv = avoidPoint + aim * flatSpeed - vF;
                dv.y = 0.0f;
                float dvLen = BikeAILengthCall(dv);
                kind = g_TrackGame->mode.raceSettings.eventType;
                if (kind == 5 || kind == 1) {
                    float limit = speedF * 0.2f;
                    if (dvLen > limit)
                        dvLen = limit;
                } else {
                    float limit = speedF * 0.2f;
                    if (dvLen > limit)
                        dvLen = limit;
                }
                dv = SafeNormalize(dv) * dvLen;
                dv *= invDt;
                accF += dv;
            }

            float lateralAccel = DotProduct(vR, right) * invDt + DotProduct(accR, right);
            if (frontContact || rearContact) {
                accR -= right * lateralAccel;
                if (rearContact) {
                    if (path) {
                        float accel = (path->targetSpeed - speedR) * invDt;
                        if (speedTable) {
                            int i = (int)speedR;
                            if (i < 0)
                                i = 0;
                            else if (i > 199)
                                i = 199;
                            float limit = speedScale * speedTable[i];
                            if (accel > limit)
                                accel = limit;
                            else if (accel < -256.0f)
                                accel = -256.0f;
                        }
                        if (accel < 0.0f && -accel > speedR * invDt) {
                            vR = Vec3(0.0f, 0.0f, 0.0f);
                            accR = sideR * rollR;
                            if (frontContact) {
                                vF = Vec3(0.0f, 0.0f, 0.0f);
                                accF = sideF * rollF;
                            }
                        } else if (accel < 0.0f) {
                            if (speedR > 0.0f) {
                                accR += vR * accel * (1.0f / speedR);
                            }
                        } else {
                            accR += fwd * accel;
                        }
                    } else if (steerSign > 0) {
                        if (speedTable) {
                            float limit = speedTable[(int)speedR] * 2.0f;
                            if (rearBrake > limit)
                                rearBrake = limit;
                        }
                        accR += fwd * rearBrake;
                    } else if (steerSign < 0) {
                        if (speedF != 0.0f) {
                            if (speedF * invDt > frontBrake) {
                                accF -= vF * frontBrake * (1.0f / speedF);
                            } else {
                                accF = -vF * invDt;
                            }
                        }
                        if (speedR != 0.0f) {
                            if (speedR * invDt > 256.0f) {
                                accR -= vF * 256.0f * (1.0f / speedF);
                            } else {
                                accR = -vR * invDt;
                            }
                        }
                    }
                }
            }
            if (outImpact && (impact < 0.0f ? -impact : impact) > *outImpact)
                *outImpact = impact;

            // Share the longitudinal acceleration and speed between the wheels.
            float aF = DotProduct(accF, fwd);
            float aR = DotProduct(accR, fwd);
            float mix = aR * 0.65f + aF * 0.35f;
            Vec3 shared = fwd * mix;
            accF += shared - fwd * aF;
            accR += shared - fwd * aR;
            aF = DotProduct(vF, fwd);
            aR = DotProduct(vR, fwd);
            mix = aR * 0.65f + aF * 0.35f;
            shared = fwd * mix;
            vF = vF + (shared - fwd * aF);
            vR += shared - fwd * aR;

            // Integrate.
            front += accF * halfDtSq + vF * dt;
            back += accR * halfDtSq + vR * dt;
            if (steer && !frontContact && !rearContact) {
                front -= sideV * steer->y * dt;
                back += sideV * steer->y * dt;
            }
            vF += accF * dt;
            vR += accR * dt;
            fwd = (front - back);
            fwd = SafeNormalize(fwd);
            right.x = fwd.z;
            right.y = 0.0f;
            right.z = -fwd.x;
            right = SafeNormalize(right);
            if (path) {
                if (!PathSteeringDirection(&aim, front, path, speedF, sideOffset)) {
                    aim = fwd;
                    aim.y = 0.0f;
                    aim = SafeNormalize(aim);
                }
                takeoffVel = vF;
                if (!PredictPathMotion(front, vF, path, &aim, 0.5f, 0.05f, terrain, sideOffset, &takeoffTime,
                                           &takeoffVel))
                    return 0;
                if (DotProduct(aim, fwd) < 0.70755f) {
                    if (DotProduct(aim, right) < 0.0f)
                        aim = SafeNormalize((fwd - right));
                    else
                        aim = SafeNormalize((fwd + right));
                }
                dir2.x = aim.z;
                dir2.y = 0.0f;
                dir2.z = -aim.x;
            } else {
                dir2.x = sinA * right.z + cosA * right.x;
                dir2.y = 0.0f;
                dir2.z = cosA * right.z - sinA * right.x;
                dir2 = SafeNormalize(dir2);
            }
            if (outImpact)
                *outImpact *= 0.5f;

            // Turn the side axis towards the lateral acceleration.
            Vec3 acc = accF + accR;
            acc = right * DotProduct(acc, right);
            acc.y += 512.0;
            acc = SafeNormalize(acc);
            acc = acc * dt;
            sideV = (acc + sideV * 4.0f * maxTime) * (1.0f / (maxTime * 4.0f + dt));
            sideV = SafeNormalize(CrossProduct(fwd, CrossProduct(sideV, fwd)));

            t += dt;
            float remaining = maxTime - t;
            steps++;
            if (remaining < dt) {
                dt = remaining;
                halfDtSq = dt * dt * 0.5f;
            }
        } while (t < maxTime);
    }

    Vec3 groundF = front;
    Vec3 groundR;
    Vec3 normalF;
    Vec3 normalR;
    if (terrain) {
        terrain->QueryGround((TerrainVec3*)&groundF, (TerrainVec3*)&normalF, 0, 0);
        terrain->QueryGround((TerrainVec3*)&groundR, (TerrainVec3*)&normalR, 0, 0);
    } else {
        normalF.x = 0.0f;
        normalF.y = 1.0f;
        normalF.z = 0.0f;
        normalR.x = 0.0f;
        normalR.y = 1.0f;
        normalR.z = 0.0f;
    }
    if (probe) {
        xf._41 = front.x;
        xf._42 = front.y;
        xf._43 = front.z;
        probe->SetTransform(&xf);
        probe->QueryCollisions();
        if (probe->hasContact) {
            float* rec = (float*)probe->contactRecord;
            float probeY = front.y + 1.0f - rec[0] * 2.0f;
            probe->hitPoint.y = probeY;
            if (probeY > groundF.y) {
                groundF.y = probeY;
                normalF = *(Vec3*)(rec + 2);
            }
        }
        xf._41 = back.x;
        xf._42 = back.y;
        xf._43 = back.z;
        probe->SetTransform(&xf);
        probe->QueryCollisions();
        if (probe->hasContact) {
            float* rec = (float*)probe->contactRecord;
            float probeY = back.y + 1.0f - rec[0] * 2.0f;
            probe->hitPoint.y = probeY;
            if (probeY > groundR.y) {
                groundR.y = probeY;
                normalR = *(Vec3*)(rec + 2);
            }
        }
    }
    Vec3 contact = sideF * frontS + front;
    float pen = DotProduct(contact - groundF, normalF);
    if (pen < 0.0f)
        front -= pen * normalF;
    pen = DotProduct(back + frontS * sideR - groundR, normalR);
    if (pen < 0.0f)
        back -= pen * normalR;
    *center = (front + back) * 0.5f;
    *forward = fwd;
    if (side)
        *side = sideV;
    *outVel = (vF + vR) * 0.5f;
    if (outSpeed)
        *outSpeed = Vec3Magnitude(outVel);
    *frontVel = vF;
    *rearVel = vR;
    if (outAirborne)
        *outAirborne = (unsigned char)airborne;
    if (!frontContact && !rearContact) {
        frontVel->x = outVel->x;
        frontVel->z = outVel->z;
        rearVel->x = outVel->x;
        rearVel->z = outVel->z;
    }
    if (outFrontContact)
        *outFrontContact = frontContact != 0;
    if (outRearContact)
        *outRearContact = rearContact != 0;
    if (frontSusp)
        *frontSusp = frontS;
    if (rearSusp)
        *rearSusp = rearS;
    return 1;
}


// ---------------------------------------------------------------------------
// KrustyBike methods emitted in BikeAI.cpp: 0x00413200 and 0x00414370.
// ---------------------------------------------------------------------------
// Both are KrustyBike members (thiscall, ret 4, `this` in esi; KrustyBike.cpp's slot 49
// 0x00496c27 and slot 63 0x004924c0 call them) that the linker placed inside BikeAI.cpp's
// code: 0x00413200 reads and writes BikeAI's file-static filter 0x00577ac0, 0x00414370 holds
// BikeAI.cpp's own __FILE__ xrefs (0x00414847, 0x0041486f), and they bracket the unit's
// Math3D vector set 0x00414210.  The class layout is src/krusty2/vehicle/KrustyBike.h
// (aiPath at +0x838 and the AI fields +0x7e0..+0x834, +0x11b0, +0x11b4 are typed there).
//
// 0x00413200 (4112 bytes): every call, constant, branch and the instruction count (1120)
// match.  Differences: (1) stack slots; the 0x74 frame is reached with two of slot 11's three
// out-vectors at function scope, but VC6 places them above the steering vector where
// retail has them in the middle of the frame (659 of 4122 strict bytes); with all three
// in the hit block the frame is 0x5c, all three at function scope 0x80; (2) operand order
// in the angular-velocity block: retail loads the forward vector's components first in
// CrossProduct(forward, up) and mixes the order in CrossProduct(w, forward) (the leaf-age
// rule of docs/VC6_OPERAND_ORDER.md; pointer locals for forward/up/w fixed half of it);
// (3) the ground-branch lean clamp: retail stores the lean and reloads it for the -1 test,
// the candidate keeps it on the FPU stack (a reference clamp helper does not change it).
// Shapes that mattered: the gearbox timer as an inline taking dt (fld dt; fsubr), the
// filter step as `step = tc; if (dt < step) step = dt` (tc loaded first),
// the gearbox's one-byte drive-cut argument (retail pushes eax unmasked; the canonical
// unsigned-char declaration preserves the former bool call's complete emission),
// the x/y-grouped dot product, dot(normal, forward) in both AI branches, and
// `(controlInput.y + 1) + 1` through a named temporary (one expression folds to + 2.0).
// Not kept: `Vec3(0.0f)` zero temporaries (2033 strict but 27 differing instructions
// against 17).
//
// 0x00414370 (4802 bytes): block order, calls and constants match (1279 vs 1302
// instructions).  Retail's race-kind test lays out the default (free-roam) block first,
// then kinds 1/5, then 2/3, which the nested `kind != 3 && kind != 2` form reproduces (a
// switch builds a jump table).  Differences: frame 0x74 vs 0x78 (retail keeps the
// difficulty in a stack slot, which the uninitialised trick delay of the other kinds
// shares; the candidate gives it edi); in the kind 1/5 branch retail emits the gate
// distance test right after the track loop and jumps back to it from the single-point and
// no-track paths (the candidate places it after them); the random free-roam target scales
// through a stored temporary.  The trick search needs the `for` form (a do/while is
// rotated with the first division folded to * 0.2).  Declaring difficulty with the
// other locals, or kind as well, or splitting kind's declaration leaves edi and 394
// strict; reading difficulty after kind gives 237; a switch for the free-roam trick delay
// loses 2 aligned instructions and the edi piece stays.  roamTarget is a Vec3: retail copies it
// into the racing line with one struct copy and builds `roamTarget - position` as a vector
// temporary (component-wise floats: 1052 aligned, Vec3: 1076).  The random roam target is written x, z,
// then y (retail's zero store follows the second rand call; 1049 -> 1052 aligned of 1302).
// Retail keeps `invLen * roamOffset.y` in the inline normalize (roamOffset.y is the zero
// register's value) while VC6 folds it here; by-value, `v * invLen` and operand-order helper
// forms do not change that.
// The local declaration order is inert.  Writing the gate tail after the loop with gotos
// compiles to identical code: the block order comes from the flow graph, and retail's has
// an extra `xor ebp, ebp` re-zero edge after the loop (its tail stores avoidNearOnly from
// the zero register, the candidate an immediate). Both builds give the loop
// counter ebp (`xor ebp, ebp ; ... inc ebp`); retail then restores the zero web
// in ebp for the tail, the candidate ends the zero web at the loop.
// 0x00572988: look-ahead distance of the race-kind 2/3 path samples (100.0f).
extern float g_BikeAILookAhead_00572988;

// 0x00566f74: target speed along the racing line, 200 entries per AI class (KrustyBike+0x79c).
extern float g_BikeAISpeedTable_00566f74[][200];

// One step of the filter at 0x00577ac0 towards `target`; returns the new value.
inline float BikeAIFilterStep(UnknownBikeAIFilter* f, float dt, float target)
{
    float step = f->timeConstant;
    if (dt < step)
        step = dt;
    f->alpha = step / f->timeConstant;
    f->value = (target - f->value) * f->alpha + f->value;
    return f->value;
}

// The same step on a Vehicle smoother (Vehicle.cpp's VehSmooth), returning the new
// value. Retail reads the time constant before the step is clamped and stores the
// result straight from the smoother.
inline float BikeAISmoothValue(VehicleSmoother* s, float dt, float target)
{
    float timeConstant = s->timeConstant;
    float step = dt;
    if (!(step < timeConstant))
        step = timeConstant;
    s->blendFactor = step / s->timeConstant;
    s->smoothedValue = (target - s->smoothedValue) * s->blendFactor + s->smoothedValue;
    return s->smoothedValue;
}

// Dot product with the x/y pair grouped (retail sums y*y' + x*x' first, then z*z').
inline float BikeAIDot(const Vec3& a, const Vec3& b)
{
    return (a.x * b.x + a.y * b.y) + a.z * b.z;
}

// Counts the gearbox's shift timer down by dt, stopping at zero.
inline void BikeAITickGearTimer(VehicleSpeedState* ss, float dt)
{
    if (ss->gearTimer != 0.0f) {
        ss->gearTimer -= dt;
        if (ss->gearTimer < 0.0f)
            ss->gearTimer = 0.0f;
    }
}

// Clamps v to [-1, 1] in place.
inline void BikeAIClampUnit(float& v)
{
    if (v > 1.0f)
        v = 1.0f;
    else if (v < -1.0f)
        v = -1.0f;
}

#define BIKEAI_ABS(x) ((x) < 0.0f ? -(x) : (x))

// 0x00413200, 4112 bytes (ret 4): KrustyBike's AI physics step, run by slot 49 instead of
// the Vehicle step while the race context's +0x18a flag is set.  Steering comes from the
// controls (player) or from the landing prediction and the racing line (AI, +0x734); the
// bike is then advanced by the look-ahead model 0x0040eca0 as two wheel points, and the
// angular velocity, speed, suspension, wheel roll, crash test and orientation are derived
// from the result.  Names tier 3.
// Retail shapes reproduced (654 -> 2026 strict, candidate size 4114): the crash flag
// +0x464 as a `?:` (retail `mov eax, 1` / `xor eax, eax` and one store), airborneOut as an
// unsigned char (retail zero-extends it with `and ecx, 0xff` for slot 71), the travel
// argument written as `-field_0x7f8` twice (retail keeps the negated value with `fst` and
// scales with `fld st(0); fmul mem`), `if (!airborne)` first for +0x5a4 (retail's branch
// layout), the ground-branch lean clamp written on the assignment `(lean = pitch) > 1.0f`,
// `float height = position.y;` before BIKEAI_ABS (retail loads it once and fchs's it) and
// the value-returning smoother BikeAISmoothValue.  Each of the last three alone scores
// 549..695; the clamp and the height copy together move most stack slots onto retail's
// (1997), the smoother adds 24.  `up` is assigned before `fwd` and the two lever cross
// products name one operand as the member and one through the pointer: VC6 loads the
// newer leaf first (docs/VC6_OPERAND_ORDER.md 1.2), which puts the first two cross
// products in retail's operand order (2026; all-pointer or all-member forms 2023..2026
// with fewer aligned instructions).
// Remaining: slot order (retail steps, impact, steerDiff, airborneOut, lean, steer, start,
// b from esp; ours puts lean first and `a` at the top, where its slot 11 address needs a
// disp32, 6 bytes early; tools/frame_layout.py finds no use-count change for it), the
// ground-branch clamp still keeping lean on the FPU stack (`fld st(0)` ... `fstp st(0)`;
// retail reloads it), the operand order of the second lever's cross product x and y
// rows, and the smoother tail (retail spills the target and reloads it; ours keeps it
// with fxch).  The other lean clamp forms (`lean = pitch;` then an if-chain on lean or
// on pitch, BikeAIClampUnit(lean = pitch)) score 573..626 with the height copy.
// Also tried at 2026: `(lean = pitch)` in both tests, the second test without `else`
// or on pitch, an inner BikeAIClampUnit, a `||` test (2022..2026, or 597..655 with the
// size changed); the smoother with step read before the time constant, a named target
// local, `+=`, a copied target or blend * difference (2024..2026); `Vec3 a, b, c` at the
// top, all three in the hit block, or `dummy` at the top (227..462).
int KrustyBike::Fn_00413200(float dt)
{
    Vec3 a, c;
    float impact = 0.0f;
    justReset = 0;
    frameTime = dt;
    stepTime = dt;
    invStepTime = 1.0f / dt;
    int steps = 1;
    UnknownVirtualSlot30();
    UnknownVirtualSlot64(dt);
    pointsTouching = 0;
    int hit = UnknownVirtualSlot39(dt);
    justLanded = 0;
    BikeAITickGearTimer(engineState, stepTime);
    if (!hasFinished)
        throttleOn = crashState == 0 && UnknownVirtualSlot80() && race->isRacing;
    else
        throttleOn = 0;
    driveDisengaged = crashState == 0 && UnknownVirtualSlot81();
    reinterpret_cast<UnknownEngineGearbox*>(engineState)->UpdateThrottle(stepTime, throttleOn, driveDisengaged);
    reinterpret_cast<UnknownEngineGearbox*>(engineState)->AutoShift(throttleOn, linearSpeed);
    if (hit) {
        Vec3 b;
        int dummy = 0;
        UnknownVirtualSlot11(hit, &a, &b, &c, &dummy);
        UnknownVirtualSlot33(&a, &b, &bodyUp, &c, dummy, UnknownVirtualSlot32());
        steps = 0;
        stepRemainder = 0.0f;
    } else {
        PlaceWheels();
        if (spawnProtectTimer > 0.0f) {
            spawnProtectTimer -= stepTime;
            if (spawnProtectTimer <= 0.0f)
                UnknownVirtualSlot50(0, 0, 0);
        }
    }
    UnknownVirtualSlot26();

    Vec3 steer;
    float lean;
    if (!aiKind) {
        steer.x = controlInput.x;
        steer.y = controlInput.y;
        if (UnknownVirtualSlot80())
            steer.z = 1.0f;
        else if (UpdateWheelRampLevels())
            steer.z = -1.0f;
        else
            steer.z = 0.0f;
    } else {
        predictedLandingNormal.x = 0.0f;
        predictedLandingNormal.y = 1.0f;
        predictedLandingNormal.z = 0.0f;
        steer = Vec3(0.0f, 0.0f, 0.0f);
        if (airborne) {
            PredictLanding(position, velocity, &predictedLandingTime, 0, &predictedLandingNormal, (Terrain*)terrain);
            float pitch = BikeAIDot(predictedLandingNormal, bodyForward) - 0.087f;
            if (predictedLandingTime > 0.0f)
                steer.y = pitch * 3.5f / predictedLandingTime;
            else
                steer.y = pitch;
            if (pitch > 0.0f)
                lean = BikeAIFilterStep(&g_BikeAIFilter_00577ac0, dt, 1.0f);
            else
                lean = BikeAIFilterStep(&g_BikeAIFilter_00577ac0, dt, -1.0f);
            BikeAIClampUnit(lean);
        } else {
            float pitch = BikeAIDot(predictedLandingNormal, bodyForward);
            if (BIKEAI_ABS(pitch) < 0.3f)
                pitch = 0.0f;
            pitch *= 4.0f;
            if (BIKEAI_ABS(-pitch) > 0.1f) {
                if (pitch > 0.0f)
                    lean = 0.1f;
                else
                    lean = -0.1f;
            } else {
                if ((lean = pitch) > 1.0f)
                    lean = 1.0f;
                else if (lean < -1.0f)
                    lean = -1.0f;
            }
        }
        float speedGap = aiPath.targetSpeed - linearSpeed;
        if (!airborne && speedGap > 0.0f) {
            aiThrottleRequest = 1.0f;
            aiBrakeRequest = 0.0f;
            throttleOn = 1;
        } else {
            aiThrottleRequest = 0.0f;
            if (speedGap < -20.0f)
                aiBrakeRequest = 1.0f;
            else
                aiBrakeRequest = 0.0f;
            throttleOn = 0;
        }
    }
    float height = position.y;
    if (BIKEAI_ABS(height) > 10000.0f)
        position.y = 0.0f;

    Vec3 start = position;
    Vec3* up = &bodyUp;
    Vec3* fwd = &bodyForward;
    worldAngularVelocity = *fwd * angularVelocity.z + *up * angularVelocity.y
                           - CrossProduct(*fwd, *up) * angularVelocity.x;
    Vec3* w = &worldAngularVelocity;
    lookAheadFrontVelocity = CrossProduct(*w, bodyForward) * 3.5f + velocity;
    lookAheadRearVelocity = velocity - CrossProduct(worldAngularVelocity, *fwd) * 3.5f;
    unsigned char airborneOut = airborne;
    SimulateLookAhead(&steer, &lookAheadFrontVelocity, &lookAheadRearVelocity, &velocity, 0, &position,
                          &bodyForward, &bodyUp, dt, dt, race->raceTerrain, &airborneOut,
                          &impact, probeSegment, &lookAheadFrontTravel, &lookAheadRearTravel,
                          &frontWheel->inContact, &rearWheel->inContact, &aiPath,
                          g_BikeAISpeedTable_00566f74[engineClass], aiLineOffset, avoidNearOnly);
    worldAngularVelocity = -CrossProduct(lookAheadFrontVelocity - velocity, bodyForward) * 0.2857f;
    angularVelocity = modelNode->WorldToLocalDirection(worldAngularVelocity);
    linearSpeed = BikeAILength(velocity);
    turnRate = 0.0f;

    frontWheel->primaryAux->ClampAndStepAlong(-lookAheadFrontTravel, 0, frontWheel->primaryAux->q_0xc0 * -lookAheadFrontTravel);
    rearWheel->secondaryAux->q_0x98 = lookAheadRearTravel;
    rearWheel->secondaryAux->ClampAndStep(lookAheadRearTravel, 0);
    if (!airborne)
        anyWheelInContact = 1;
    else
        anyWheelInContact = 0;

    float steerTarget = worldAngularVelocity.y * -1.2732f;
    float steerDiff = steerTarget - controlInput.x;
    if (BIKEAI_ABS(steerDiff) > 0.125f) {
        if (steerDiff > 0.0f)
            controlInput.x += 0.125f;
        else
            controlInput.x -= 0.125f;
    } else {
        controlInput.x = steerTarget;
    }
    float leanDiff = lean - controlInput.y;
    if (BIKEAI_ABS(leanDiff) > 0.25f) {
        if (leanDiff > 0.0f)
            controlInput.y += 0.25f;
        else
            controlInput.y -= 0.25f;
    } else {
        controlInput.y = lean;
    }
    if (controlInput.x > 1.0f)
        controlInput.x = 1.0f;
    else if (controlInput.x < -1.0f)
        controlInput.x = -1.0f;
    if (controlInput.y > 1.0f)
        controlInput.y = 1.0f;
    else if (controlInput.y < -1.0f)
        controlInput.y = -1.0f;
    float leanAxis = controlInput.y + 1.0f;
    steerAxis->steerValue = (leanAxis + 1.0f) * 0.25f;
    UnknownVirtualSlot35(1, 1);
    if (aiKind && airborne)
        controlInput.x = 0.0f;

    forwardSpeed = BikeAIDot(velocity, bodyForward);
    rearWheel->tangentSpeed = forwardSpeed;
    frontWheel->tangentSpeed = forwardSpeed;
    rearWheel->slipSpeed = forwardSpeed;
    frontWheel->slipSpeed = forwardSpeed;
    rearWheel->spinSpeed = 0.0f;
    frontWheel->spinSpeed = 0.0f;
    if (forwardSpeed > 0.0f)
        movingForward = 1;
    else
        movingForward = 0;
    if (frontWheel->inContact)
        ((KbWheel*)frontWheel)->SetRollDistance(forwardSpeed / frontWheel->wheelRadius * dt);
    else
        ((KbWheel*)frontWheel)->SetRollDistance(forwardSpeed / frontWheel->wheelRadius * dt);
    if (rearWheel->inContact)
        ((KbWheel*)rearWheel)->SetRollDistance(forwardSpeed / rearWheel->wheelRadius * dt);
    else
        ((KbWheel*)rearWheel)->SetRollDistance(forwardSpeed / rearWheel->wheelRadius * dt);
    modelNode->SetPosition(position);
    UnknownVirtualSlot71(airborneOut);
    airborne = airborneOut;
    if (airborneOut)
        landingLatched = 0;
    if (crashState) {
        UnknownVirtualSlot87();
    } else {
        if (airborneOut)
            UnknownVirtualSlot91();
        UnknownVirtualSlot90(&steps, dt);
    }
    UnknownVirtualSlot28(steps);

    if (collisionObject->hasContact && lastCollisionType != 1000 &&
        lastCollisionType != 100 && lastCollisionType != 101)
        contactTime += dt;
    else
        contactTime = 0;
    if (BIKEAI_ABS(impact) > field_0x778 || BIKEAI_ABS(linearSpeed - prevSpeed) > 44.0f ||
        bodyUp.y <= 0.0f || contactTime > 0.5f) {
        crashState = 1;
        crashDirection = 1;
        crashReason = 4;
        UnknownVirtualSlot41();
        crashHeading = Vec3(savedForward.x, 0.0f, savedForward.z);
        motionCompleted = 0;
        motionIndex = 0;
        crashedUpsideDown = savedUp.y < 0.0f ? 1 : 0;
        crashYaw = savedYaw;
        wrecker->StartAnimatedWreck(crashDirection);
        UnknownVirtualSlot69();
    }
    savedForward = bodyForward;
    savedUp = bodyUp;
    centerNode->GetPositionIn(0, &centerOfMass);
    UnknownVirtualSlot34();
    OrientationAnglesFromVectors(bodyForward, bodyUp, &bodyYaw, &bodyPitch, &bodyRoll,
                                 &bodySinRoll, &bodyCosRoll, &bodyCosPitch, &bodySinPitch);
    UnknownVirtualSlot29(1);
    prevSpeed = linearSpeed;
    Vec3 moved = position - start;
    frontWheel->groundPoint += moved;
    rearWheel->groundPoint += moved;
    UnknownVirtualSlot21();
    scratchVector = modelNode->WorldToLocalDirection(velocity);
    smoothedForwardAccel = BikeAISmoothValue(forwardAccelSmoother, frameTime,
                                             (scratchVector.z - prevLocalForwardVelocity) / frameTime);
    prevLocalForwardVelocity = scratchVector.z;
    UnknownVirtualSlot102(dt);
    return 1;
}

// rand() scaled to [0, 1).  A float-returning inline keeps a later scale factor from being
// folded into the 1/32768 (KrustyBike.cpp's KbRandUnit has the same body).
inline float BikeAIRandUnit()
{
    float r = rand() * (1.0f / 32768.0f);
    return r;
}

// 0x0040d120's normalise without the zero test: v unchanged when |v|^2 is exactly 1.
inline Vec3 BikeAINormalizeInline(const Vec3& v)
{
    float lenSq = SquareMagnitude(v);
    if (lenSq == 1.0f)
        return v;
    float invLen = FastInvSqrt(lenSq);
    return Vec3(invLen * v.x, invLen * v.y, invLen * v.z);
}

// 0x00414370, 4802 bytes (ret 4): rebuilds the AI racing line aiPath for this frame and
// returns 1 (0 when a track query fails).  By race kind (game +0x2d74):
//  * 2 and 3 (track races): the shortest track path from the bike's progress position to
//    the race's target position, sampled every 0.1 of the speed (100 points) or ahead of
//    the bike; off the track for 5 s the bike is respawned through slots 11/33;
//  * 1 and 5: a short line along the gates of statusNode->nextGate (3 points around the gate,
//    or the gate itself), or the start-gate run-up while +0x7a4 is set;
//  * otherwise: one random free-roam target 444 units (scaled by the terrain) away.
// The racing line's target speed (40 near a gate, else 1000) and the AI class's speed scale
// (+0x7c4..+0x7cc by game difficulty +0x60c) follow; a local bike may then start a trick
// while airborne (0x0048d780 picks it, 0x0048d910 plays it).  Names tier 3.
// The point loops test the count at the top (retail `cmp ebp, count; jge exit` heads and a
// `t` test as the back edge), so they are written with a leading break rather than `for`.
int KrustyBike::Fn_00414370(float dt)
{
    KbTrackItem* list;
    KbTrackItem** tail;
    KbRaw3 from;
    KbRaw3 pathPos;
    KbGate* gate;
    Vec3 a, b, c;
    float distance;
    float right, left;
    float gateAhead, bikeAhead;
    float speed, t, divisor, lenSq, dx, dz;
    float trickDelay;
    int dummy;
    int count;
    int i;

    memset(&aiPath, 0, sizeof(aiPath));
    list = 0;
    dummy = 0;
    int difficulty = g_TrackGame->mode.difficulty;
    int kind = g_TrackGame->mode.raceSettings.eventType;
    if (kind != 3 && kind != 2) {
        if (kind != 1 && kind != 5) {
            aiPath.count = 1;
            Vec3 roamOffset;
            roamOffset = roamTarget - position;
            roamOffset.y = 0.0f;
            if (roamOffset.z * roamOffset.z + roamOffset.x * roamOffset.x < 2500.0f) {
                roamOffset.x = rand() * (1.0f / 32768.0f);
                roamOffset.z = rand() * (1.0f / 32768.0f);
                roamOffset.y = 0.0f;
                roamOffset = BikeAINormalizeInline(roamOffset) * 444.0f * terrain->worldScale;
                roamTarget.x = terrain->worldScale * 666.66f + roamOffset.x;
                roamTarget.y = 0.0f;
                roamTarget.z = terrain->worldScale * 666.66f + roamOffset.z;
            } else {
                aiPath.points[0] = roamTarget;
            }
            aiPath.targetSpeed = 1000.0f;
            aiPath.accelLimitScale = aiAccelScale3;
            aiPath.lookAhead = linearSpeed * 0.25f;
            aiStuntChance = race->aiStuntChance3;
            if (difficulty == 3)
                trickDelay = trickDelay3;
            else if (difficulty == 2)
                trickDelay = trickDelay2;
            else if (difficulty == 1)
                trickDelay = trickDelay1;
        } else {
            if (hasFinished) {
                startRunUpTime += dt;
                if (startRunUpTime < 2.0f) {
                    aiPath.count = 1;
                    aiPath.points[0].y = 0.0f;
                    aiPath.points[0].x = bodyForward.z * 40.0f + position.x;
                    aiPath.points[0].z = position.z - bodyForward.x * 40.0f;
                    aiPath.targetSpeed = 40.0f;
                    return 1;
                }
                aiPath.targetSpeed = 0.0f;
                return 1;
            }
            speed = linearSpeed;
            if (race->raceTrack) {
                if (!race->raceTrack->ClosestPointOnNode(position, race->raceTrack->startNode,
                                                                    &statusNode->placedPos, 0))
                    return 0;
                if (!statusNode->progressPos.a) {
                    statusNode->progressPos.a = statusNode->placedPos.a;
                    statusNode->progressPos.b = statusNode->placedPos.b;
                    statusNode->progressPos.c = statusNode->placedPos.c;
                }
                gate = statusNode->nextGate;
                if (!gate->trackPos.a)
                    race->raceTrack->ClosestPointOnNode(gate->position, race->raceTrack->startNode,
                                                                  &gate->trackPos, 0);
                gateAhead = race->raceTrack->DistanceAlong(statusNode->progressPos,
                                                                          statusNode->nextGate->trackPos);
                bikeAhead = race->raceTrack->DistanceAlong(statusNode->placedPos,
                                                                          statusNode->nextGate->trackPos);
                if (bikeAhead > 0.0f && gateAhead > 0.0f && bikeAhead < gateAhead) {
                    statusNode->progressPos.a = statusNode->placedPos.a;
                    statusNode->progressPos.b = statusNode->placedPos.b;
                    statusNode->progressPos.c = statusNode->placedPos.c;
                }
                list = (KbTrackItem*)DebugCalloc(1, 0x10, __FILE__, 0x99f);
                if (!list)
                    return 0;
                list->node = race->raceTrack->startNode;
                list->next = (KbTrackItem*)DebugCalloc(1, 0x10, __FILE__, 0x9a3);
                if (!list->next)
                    return 0;
                list->next->node = race->raceTrack->startNode;
                if (list && bikeAhead < gateAhead) {
                    aiPath.count = 100;
                    t = 0.0f;
                    i = 0;
                    while (1) {
                        if (i >= aiPath.count)
                            break;
                        if (!race->raceTrack->MoveAlongPath(statusNode->progressPos, &pathPos, &list,
                                                            t * linearSpeed, 0))
                            return 0;
                        if (!race->raceTrack->PointAt(pathPos, &aiPath.points[i]))
                            return 0;
                        aiPath.points[i].y = position.y + 2.0f;
                        i++;
                        t += 0.1f;
                        if (t > 10.0f)
                            break;
                    }
                } else {
                    aiPath.count = 1;
                    aiPath.points[0] = statusNode->nextGate->position;
                }
            } else if (statusNode->goalDistance < 300.0f) {
                aiPath.count = 3;
                gate = statusNode->nextGate;
                if (BikeAIDot(gate->position - position, gate->direction) < 0.0f) {
                    aiPath.points[0] = speed * gate->direction + gate->position;
                    aiPath.points[2] = statusNode->nextGate->position - speed * statusNode->nextGate->direction;
                } else {
                    aiPath.points[0] = gate->position - speed * gate->direction;
                    aiPath.points[2] = speed * statusNode->nextGate->direction + statusNode->nextGate->position;
                }
                aiPath.points[1] = statusNode->nextGate->position;
            } else {
                aiPath.count = 1;
                aiPath.points[0] = statusNode->nextGate->position;
            }
            gate = statusNode->nextGate;
            dx = position.x - gate->position.x;
            dz = position.z - gate->position.z;
            if (FastSqrt(dx * dx + dz * dz) < speed) {
                if (BikeAIDot(SafeNormalize(velocity), SafeNormalize(gate->position - position)) < 0.70755f)
                    aiPath.targetSpeed = 40.0f;
                else
                    aiPath.targetSpeed = 1000.0f;
                aiPath.lookAhead = 20.0f;
                avoidNearOnly = 1;
            } else {
                aiPath.targetSpeed = 1000.0f;
                aiPath.accelLimitScale = aiAccelScale3;
                avoidNearOnly = 0;
                aiPath.lookAhead = linearSpeed * 0.25f;
            }
            if (difficulty == 3)
                aiPath.accelLimitScale = aiAccelScale3;
            else if (difficulty == 2)
                aiPath.accelLimitScale = aiAccelScale2;
            else if (difficulty == 1)
                aiPath.accelLimitScale = aiAccelScale1;
            aiPath.lookAhead = linearSpeed * 0.25f;
        }
    } else {
        if (!statusNode->progressPos.a || !statusNode->progressPos.b)
            return 0;
        if (!race->finishProbePos.a || !race->finishProbePos.b)
            return 0;
        if (!race->raceTrack->ShortestPath(statusNode->progressPos, race->finishProbePos,
                                                             &list, &distance))
            return 0;
        tail = &list;
        while (*tail)
            tail = &(*tail)->next;
        from = race->finishProbePos;
        from.t += 0.01f;
        if (!race->raceTrack->ShortestPath(from, race->finishProbePos, tail, &distance))
            return 0;
        if (onTrack) {
            if (list) {
                aiPath.count = 100;
                t = 0.0f;
                i = 0;
                while (1) {
                    if (i >= aiPath.count)
                        break;
                    if (!race->raceTrack->MoveAlongPath(statusNode->progressPos, &pathPos, &list,
                                                        t * linearSpeed, 0))
                        return 0;
                    if (!race->raceTrack->PointAt(pathPos, &aiPath.points[i]))
                        return 0;
                    aiPath.points[i].y = position.y + 2.0f;
                    i++;
                    t += 0.1f;
                    if (t > 10.0f)
                        break;
                }
            } else {
                aiPath.count = 1;
                aiPath.points[0] = linearSpeed * bodyForward + position;
            }
            if (!g_TrackGame->mode.raceSettings.raceMode)
                aiOffTrackTime = 0.0f;
        } else {
            count = 1;
            if (!g_TrackGame->mode.raceSettings.raceMode) {
                aiOffTrackTime += dt;
                if (aiOffTrackTime >= 5.0f) {
                    UnknownVirtualSlot11(count, &a, &b, &c, &dummy);
                    UnknownVirtualSlot33(&a, &b, &bodyUp, &c, dummy, UnknownVirtualSlot32());
                    aiOffTrackTime = 0.0f;
                }
            }
            aiPath.count = count;
            if (list) {
                t = 1.0f;
                for (i = 0;;) {
                    if (i >= aiPath.count)
                        break;
                    if (!race->raceTrack->MoveAlongPath(statusNode->progressPos, &pathPos, &list,
                                                        g_BikeAILookAhead_00572988 * t, 0))
                        return 0;
                    if (!race->raceTrack->PointAt(pathPos, &aiPath.points[i]))
                        return 0;
                    aiPath.points[i].y = position.y + 2.0f;
                    lenSq = SquareMagnitude(aiPath.points[i] - position);
                    if (lenSq == 1.0f || FastSqrt(lenSq) < 100.0f)
                        aiPath.targetSpeed = 40.0f;
                    i++;
                    t += 0.1f;
                    if (t > 1.0f)
                        break;
                }
            } else {
                pathPos = statusNode->progressPos;
                if (!race->raceTrack->PointAt(statusNode->progressPos, &aiPath.points[0]))
                    return 0;
            }
        }
        aiPath.targetSpeed = 1000.0f;
        aiPath.accelLimitScale = aiAccelScale3;
        aiPath.lookAhead = linearSpeed * 0.25f;
        if (difficulty == 3)
            aiPath.accelLimitScale = aiAccelScale3;
        else if (difficulty == 2)
            aiPath.accelLimitScale = aiAccelScale2;
        else if (difficulty == 1)
            aiPath.accelLimitScale = aiAccelScale1;
        if (hasFinished) {
            onTrack = race->raceTrack->PlaceNear(position, statusNode->progressPos,
                                                                        -1.0f, -1.0f, 0, 0);
            if (!onTrack) {
                aiPath.targetSpeed = 0.0f;
                goto done;
            }
            aiPath.targetSpeed = 40.0f;
            Vec3 dir;
            if (!race->raceTrack->SegmentDirection(statusNode->progressPos.b, &dir))
                return 0;
            race->raceTrack->DistanceToStripEdge(position, statusNode->progressPos.b, &right, 10);
            race->raceTrack->DistanceToStripEdge(position, statusNode->progressPos.b, &left, 11);
            if (BikeAIDot(dir, bodyForward) < 0.0f)
                dir = -dir;
            if (left < right) {
                dir.x = dir.z;
                dir.z = -dir.x;
            } else {
                dir.x = -dir.z;
                dir.z = dir.x;
            }
            dir.y = 0.0f;
            dir = SafeNormalize(dir);
            aiPath.count = 1;
            aiPath.points[0] = aiPath.targetSpeed * dir + position;
        }
        if (onTrack)
            avoidNearOnly = 1;
        else
            avoidNearOnly = 0;
    }
    if (aiTricksEnabled && !(trickDelay > trickTimer)) {
        if (!motionActive && airborne) {
            PredictLanding(position, velocity, &predictedLandingTime, 0, 0, (Terrain*)terrain);
            if (BikeAIRandUnit() * 100.0f <= aiStuntChance) {
                for (divisor = 5.0f; divisor >= 1.0f; divisor -= 1.0f) {
                    if (Fn_0048D780(&field_0x15d4, predictedLandingTime / divisor - 0.5f, 0))
                        break;
                }
                if (divisor > 0.0f) {
                    Fn_0048D910(field_0x15d4);
                    trickTimer = 0;
                }
            }
        }
    } else {
        trickTimer += dt;
    }
done:
    if (list)
        race->raceTrack->FreeWorkList(&list, 0);
    return 1;
}
