// BikeAI.cpp -- reconstruction of D:\aardvark\VC\krusty2\BikeAI.cpp (string 0x005678d4).
//
// Extent (strong inference): 0x0040d070..0x00416e1f.
//  * Bike.cpp's last function is Bike vtable slot 96 at 0x0040d030 (ends 0x0040d063);
//    bikerace.cpp's first xref is 0x0041807d.  No other source path sorts between
//    Bike.cpp and BikeAI.cpp.
//  * BikeAI's own __FILE__ xrefs: 0x00414847 and 0x0041486f (debug new in 0x00414370)
//    and 0x00416c36 (in 0x00415640); EH funclet 0x00549916.
//  * The file-static filter at 0x00577ac0 initialised by 0x0040d080 is read and
//    written only by 0x00413200, a KrustyBike method called from KrustyBike.cpp
//    (0x00496c27).  So the $E group at 0x0040d070 opens this TU.
//  * One math/Math3D.h constant set (0x00414210..0x0041435b) sits in the range; the
//    next set (0x00417350) belongs to the TU that holds BikeCamera, so BikeCamera
//    (0x00416e20..) is outside this file.
// The free functions below are cdecl helpers used by the AI path code
// (0x0040d200, 0x0040e510, 0x0040eca0) and the path motion prediction 0x0040d200
// itself.  All names are provisional.  The other path functions (0x0040e510,
// 0x0040eca0, 0x00415640) and the KrustyBike members 0x00413200 and 0x00414370 that
// sit in this unit are near misses in samples/physics/bikeai/BikeAINearMisses.cpp
// (class layout: vehicle/KrustyBike.h).
#include "math/Math3D.h"
#include "broadphase/Terrain.h"
#include "vehicle/KrustyBikeRecords.h"
#include "../../reconstructed/MatrixUtil.h"

// 0x00577ac0, 20 bytes: a first-order smoothing filter used by 0x00413200:
// alpha = min(dt / timeConstant, 1); value += (target - value) * alpha.
// The constructor's store order (value, timeConstant, max, min, alpha) is tier 1;
// member roles are tier 3.
struct UnknownBikeAIFilter {
    float value;        // +0x00
    float timeConstant; // +0x04
    float alpha;        // +0x08
    float maxValue;     // +0x0c
    float minValue;     // +0x10

    UnknownBikeAIFilter(float tc)
    {
        value = 0.0f;
        timeConstant = tc;
        maxValue = 1.0f;
        minValue = -1.0f;
        alpha = 1.0f;
    }
};

UnknownBikeAIFilter g_BikeAIFilter_00577ac0(0.5f);

// Statics with an empty constructor: their $E initializers are a jmp to a bare ret
// (0x0040d0c0, 0x0040d0e0, 0x0040d100 here, 0x0040d1e0 after 0x0040d120 and
// 0x00414350 after the Math3D set).  Their addresses never appear in the code, so
// type and identity are unknown; Vec3 (empty user constructor) reproduces the shape.
static Vec3 s_BikeAIUnknownStatic_0;
static Vec3 s_BikeAIUnknownStatic_1;
static Vec3 s_BikeAIUnknownStatic_2;

// 0x0040d120, 182 bytes, cdecl, hidden result pointer: returns v unchanged when it is
// the zero vector, otherwise v * FastInvSqrt(|v|^2).
Vec3 SafeNormalize(Vec3 v)
{
    if (v.x == 0.0f && v.y == 0.0f && v.z == 0.0f)
        return v;
    float invLen = FastInvSqrt((v.x * v.x + v.y * v.y) + v.z * v.z);
    return Vec3(v.x * invLen, v.y * invLen, v.z * invLen);
}

static Vec3 s_BikeAIUnknownStatic_3;
static Vec3 s_BikeAIUnknownStatic_4;

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

// Matching notes for 0x0040d200 (PredictPathMotion below), which is strict exact.  Its
// frame needed `normal` at function scope (inside the loop body VC6 packs it into dir's
// slot) and the done-block flat speed reusing `flatSpeed`.  Three source forms place the last instructions
// (docs/VC6_OPERAND_ORDER.md section 3):
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

// Debug line endpoints at 0x00578dc0 drawn by BikeRace (BikeRace.h and
// BikeRaceDebugState.cpp define them as Vector3, MatrixUtil.h).  BikeAI writes them as
// Vec3: the same three floats, copied as one 12-byte struct.
extern Vector3 g_PathTargetDebugLine[2];

inline Vec3* PathTargetDebugLine()
{
    return reinterpret_cast<Vec3*>(g_PathTargetDebugLine);
}

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
int PredictPathMotion(Vec3 pos, Vec3 vel, BikeAIRacingLine* path, Vec3* outDir,
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
    PathTargetDebugLine()[0] = pos;
    PathTargetDebugLine()[0].y = pos.y + 3.0f;
    PathTargetDebugLine()[1] = target;
    PathTargetDebugLine()[1].y = target.y + 3.0f;
    return 1;
}

// 0x00413190, 106 bytes, cdecl: |v|, with 0 and 1 returned exactly.
float Vec3Magnitude(const Vec3* v)
{
    float lenSq = (v->x * v->x + v->y * v->y) + v->z * v->z;
    if (lenSq == 0.0f)
        return 0.0f;
    if (lenSq == 1.0f)
        return 1.0f;
    return 1.0f / FastInvSqrt(lenSq);
}

// 0x0040e370, 416 bytes, cdecl: ballistic landing prediction.  Steps a point from
// 'start' with velocity 'velocity' in 0.1 s steps (gravity 32: y += vy*dt - 0.16,
// vy -= 3.2) until it is no longer 6 units above the ground under it.  Writes the
// elapsed time, the displacement and the last ground normal; always returns 1.
// Tier 1 arithmetic; tier 3 names.  Callers: 0x004134f6 and 0x00415544.
int PredictLanding(Vec3 start, Vec3 velocity, float* outTime, Vec3* outDelta,
                          Vec3* outNormal, Terrain* terrain)
{
    Vec3 v = velocity;
    float t = 0.0f;
    Vec3 p = start;
    Vec3 ground = start;
    Vec3 normal;
    terrain->QueryGround((TerrainVec3*)&ground, (TerrainVec3*)&normal, 0, 0);
    while (p.y + 6.0f > ground.y) {
        p += v * 0.1f;
        p.y -= 0.16f;
        v.y -= 3.2f;
        ground = p;
        terrain->QueryGround((TerrainVec3*)&ground, (TerrainVec3*)&normal, 0, 0);
        t += 0.1f;
    }
    if (outTime)
        *outTime = t;
    if (outDelta)
        *outDelta = p - start;
    if (outNormal)
        *outNormal = normal;
    return 1;
}
