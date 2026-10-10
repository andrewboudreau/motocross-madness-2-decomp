// Near-miss BikeAI.cpp candidates (D:\aardvark\VC\krusty2\BikeAI.cpp), kept out of
// src/krusty2/vehicle/BikeAI.cpp until they match.  The canonical file is included
// first so the TU-local statics and helpers are the same.
//
// 0x0040d200 (4464 bytes, AI jump prediction along the path): 4430/4464 strict
// positions.  Control flow, calls, constants, FPU operand order and the 0x178-byte
// frame match; two stores are scheduled one slot apart from retail.  Retail emits
// the initial `seg.y = 0` after `fld/fmul seg.x` (the candidate after `fld/fmul
// seg.z`), and the ground-branch `p.y = ground.y` store after `fld/fmul normal.z`
// (the candidate right after the ground.y load).  The copy in the inner segment
// loop already matches.  Source order, indexed access, helper and pointer
// variants did not move either store.  The frame needed `normal` at function scope
// (inside the loop body VC6 packs it into dir's slot) and the done-block flat
// speed reusing `flatSpeed`.
//
// 0x0040e510 (1936 bytes, AI path steering direction): every branch, call and
// constant is reconstructed and the instruction sequence aligns 1:1 apart from
// two allocation choices.  Retail keeps the path pointer in esi and the best index
// in edi (the candidate swaps them), and retail's frame puts the perpendicular /
// normal at +0x24, the walk direction at +0x30 and the operator- temporaries and
// target at +0x3c (the candidate places the segment temporary at +0x24).
// Declaration order of the Vec3 locals does not change VC6's frame (all 24
// permutations tried); declaring bestT before best fixed the immediate store of
// bestT = 0.  The candidate is 1944 bytes.
//
// The inline length helper used here has the same body as the out-of-line copy at
// 0x00413190 (matched in the canonical file as a plain function).  Retail's
// 0x00413190 sits after 0x0040eca0, which is where VC6 emitted the COMDAT when that
// 17 KB function ran out of inline budget, so in the original source it is very
// likely one inline function used in both places.
//
// 0x00415640 (6112 bytes, obstacle query along a segment): 5987/6111 strict
// positions.  Control flow, calls, constants and the 0x1ec frame size match and the
// masked instruction stream differs in 17 places.  Left: the order of the one-use
// operator temporaries above +0x100 in the frame (offset, the dn * k / dn * 10 / dn *
// push temporaries of both owner branches, the segment-length temporaries and the
// vegetation position), and two scheduling spots where retail pushes the dot-call
// arguments after the z*z term of SquareMagnitude (0x00415e0c, 0x00416788).
// Source forms that mattered: unsigned char type-id statics with an unsigned compare
// (`mov bl, 0xff`), the max/min helpers with by-value arguments (two spellings,
// see BikeAIMaxR), SquareMagnitude for the unnormalised distance, hoisting s/u, d,
// distSq and normal out of the owner branches (shared slots), a named `step` for the
// vegetation push, and the call views below for the dot product, the constructor
// and operator*(Vec3, float) that retail calls even in this function's first lines.
//
// 0x0040eca0 (17585 bytes, look-ahead simulation of the two wheels): the whole
// function is reconstructed; 3929 vs 3966 instructions, 392 masked code differences,
// frame 0x8b8 vs 0x93c.  The call sequence matches retail call for call except one
// site (0x00412e23: retail inlines operator+ with the out-of-line constructor, the
// candidate calls operator+).  This function exhausts VC6's per-function inline
// budget: the first part expands every Vec3 operator, from 0x00410d00 the operators
// are expanded but their constructor is the out-of-line 0x00404e60, and in the
// epilogue whole operators are called (0x00428090, 0x00421cb0, 0x00421d00,
// 0x005015b0, 0x00413160).  Natural Math3D operators reproduce that pattern; the
// budget is consumed per inline expansion, so `right` and the start-up cross product
// are written out component-wise (a CrossProduct call there moves the switch-over
// points too early).  Remaining differences: the start-up `up` vector and the
// loop-head drag vectors keep their zero components in memory in retail (the
// candidate's are constant-folded), the order of a few temporaries and stores
// (0x00411dbc vF += (commuted operands), 0x00412883), and the frame layout.
//
// 0x00413160 (Vec3::operator-=, 33 bytes) is the out-of-line COMDAT that VC6 emits
// right after 0x0040eca0 for its epilogue; this file reproduces it strict exact.
#include "../../../src/krusty2/vehicle/BikeAI.cpp"
#include "vehicle/KrustyBike.h"
#include "collision/CollisionObject.h"
#include "broadphase/Quadtree.h"
#include <math.h>
#include <stdlib.h>
#include <string.h>

// The AI racing line UnknownBikeAIPath is declared in vehicle/KrustyBikeTypes.h
// (KrustyBike+0x838 holds one).

// View of the game object at 0x0056e26c (TrackGame in src/reconstructed): the object tree
// root, the race kind and the +0x2d80 flag are read here.
struct UnknownBikeAIGameView {
    char pad_0x0000[0x34];
    GameObject* root;   // +0x34 root of the object tree (GameObjectIterator walk in 0x00415640)
    char pad_0x0038[0x2d74 - 0x38];
    int raceKind;       // +0x2d74
    char pad_0x2d78[0x8];
    int field_0x2d80;   // +0x2d80 nonzero: vegetation is avoided too (0x00410a2a)
};
extern UnknownBikeAIGameView* g_BikeAIGame_0056e26c;

// 0x0047b800 (cdecl, after gameui.cpp): 2D (x, z) intersection of the line a + s*da
// with the line b + u*db; writes s and u, returns 0 when they do not intersect
// (tier 2 from the callers; name tier 3).
int UnknownFunction47b800(float ax, float az, float dax, float daz, float bx, float bz,
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

// 0x0040e510, 1936 bytes, cdecl: steering direction along an AI path.  The fourth
// argument is the bike's speed (0x0040eca0 pushes it as a float); it is not read.  Finds the
// path point nearest to 'pos', refines the segment by intersecting the perpendicular
// through 'pos' with the neighbouring segments, walks 'lookAhead' (+ |offset| in race
// kinds 2 and 3) along the path, and returns in *outDir the flat unit direction from
// 'pos' to that point shifted sideways by 'side'.  Stores the reached segment/parameter
// in the path.  Callers: 0x0040f1ef and 0x0041256c.  Names tier 3.
int UnknownFunction40e510(Vec3* outDir, Vec3 pos, UnknownBikeAIPath* path, float speed,
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
        if (nearest > 0) {
            seg = path->points[nearest] - path->points[nearest - 1];
            Vec3 n = SafeNormalize(Vec3(seg.z, 0.0f, -seg.x));
            if (!UnknownFunction47b800(path->points[nearest - 1].x, path->points[nearest - 1].z, seg.x, seg.z,
                                       pos.x, pos.z, n.x, n.z, &t, &offset))
                return 0;
            if (t >= 0.0f && t <= 1.0f && offset < bestDist) {
                bestDist = offset;
                best--;
                bestT = t;
            }
        }
        if (nearest < path->count - 1) {
            seg = path->points[nearest + 1] - path->points[nearest];
            Vec3 n = SafeNormalize(Vec3(seg.z, 0.0f, -seg.x));
            if (!UnknownFunction47b800(path->points[nearest].x, path->points[nearest].z, seg.x, seg.z,
                                       pos.x, pos.z, n.x, n.z, &t, &offset))
                return 0;
            if (t >= 0.0f && t <= 1.0f && offset < bestDist) {
                best = nearest;
                bestT = t;
            }
        }
        float ahead = path->lookAhead;
        int kind = g_BikeAIGame_0056e26c->raceKind;
        if (kind == 2 || kind == 3)
            ahead += (offset < 0.0f ? -offset : offset);
        Vec3 dir = path->points[best + 1] - path->points[best];
        float len = (1.0f - bestT) * BikeAILength(dir);
        while (ahead > 0.0f) {
            if (!(len < ahead) || best >= path->count - 1)
                break;
            ahead -= len;
            best++;
            bestT = 0.0f;
            if (best < path->count - 1) {
                dir = path->points[best + 1] - path->points[best];
                len = BikeAILength(dir);
            }
        }
        Vec3 n = SafeNormalize(Vec3(dir.z, 0.0f, -dir.x));
        Vec3 target;
        if (best >= path->count - 1) {
            t = 0.0f;
            target = path->points[best];
        } else if (len > ahead) {
            t = ahead / len + bestT;
            target = path->points[best] + dir * t;
        }
        path->index = best;
        path->t = t;
        *outDir = (target - pos) + n * side;
        outDir->y = 0.0f;
        if (outDir->x == 0.0f && outDir->z == 0.0f) {
            outDir->z = 1.0f;
            return 1;
        }
        *outDir = SafeNormalize(*outDir);
        return 1;
    }
    Vec3 d = path->points[0] - pos;
    d.y = 0.0f;
    if (outDir->x == 0.0f && outDir->z == 0.0f)
        outDir->z = 1.0f;
    else
        *outDir = SafeNormalize(d);
    path->index = 0;
    path->t = 0.0f;
    return 1;
}

// Debug line endpoints at 0x00578dc0 drawn by BikeRace (BikeRace.h declares the same
// array as Vector3 g_UnknownGlobal578dc0[2]).
extern Vec3 g_UnknownGlobal578dc0[2];

// Dot product in the operand order retail 0x0040d200 evaluates: (y + x) + z.
inline float BikeAIDotZ(const Vec3& a, const Vec3& b)
{
    return a.z * b.z + (a.x * b.x + a.y * b.y);
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
    float s = FastInvSqrt(lenSq);
    return BikeAIVec3Call(v.x * s, v.y * s, v.z * s);
}

// 0x0040d200, 4464 bytes, cdecl: predicts the bike's motion along the AI path.
// Starting at 'pos' with the flat speed of 'vel', it steps by 'dt' (gravity -32 when
// more than 0.5 above the ground under it, ground contact removes the velocity into
// the ground normal), advances the path parameter and steers towards the path point
// offset sideways by 'side'.  It records the first takeoff (time, position, velocity,
// flat direction) and stops at the landing (direction change below cos 45 deg, or
// the last path point).  For a jump longer than 20 units it computes the takeoff
// speed that would cover the distance and stores it in path->field_0x96c, raised to
// 40 when below 40, otherwise to takeoffSpeed - 20 when it is more than 20 below the
// takeoff speed.  The takeoff velocity is rescaled to that speed.
// Writes the reached segment/parameter to the path, the flat direction to the
// target in *outDir and the takeoff velocity in *outVel when the takeoff came
// within 0.5 s, and the takeoff time (10 when none) in *outTakeoffTime.  Always
// sets the debug line 0x00578dc0 (pos and target raised by 3).  Returns 0 for a
// missing path/terrain, an index outside the path or dt <= 0, else 1.
// Callers: 0x0040f2c9 and 0x00412654.  Arithmetic tier 1, names tier 3.
int UnknownFunction40d200(Vec3 pos, Vec3 vel, UnknownBikeAIPath* path, Vec3* outDir,
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
    Vec3 p = pos;
    float halfDtSq = dt * dt * 0.5f;
    float flatSpeed = FastSqrt(vel.x * vel.x + vel.z * vel.z);
    Vec3 seg;
    if (path->count == 1)
        seg = path->points[idx] - pos;
    else if (idx < path->count - 1)
        seg = path->points[idx + 1] - path->points[idx];
    else
        seg = path->points[idx] - path->points[idx - 1];
    seg.y = 0.0f;
    float len = BikeAILength(seg);
    float invLen = len > 0.0f ? 1.0f / len : 0.0f;
    Vec3 n;
    n.x = seg.z;
    n.y = 0.0f;
    n.z = -seg.x;
    n = SafeNormalize(n);
    Vec3 target = path->points[idx] + seg * t + n * side;
    Vec3 dir = target - pos;
    dir.y = 0.0f;
    dir = SafeNormalize(dir);
    Vec3 v(dir.x * flatSpeed, vel.y, dir.z * flatSpeed);
    float align = 1.0f;
    Vec3 normal;
    Vec3 takeoffVel;
    float takeoffSpeed;
    Vec3 takeoffPos;
    Vec3 flatDir;
    while ((elapsed < maxTime || airborne) && !done) {
        float speedSq = (v.x * v.x + v.y * v.y) + v.z * v.z;
        float speed = speedSq == 1.0f ? 1.0f : FastSqrt(speedSq);
        flatSpeed = FastSqrt(v.x * v.x + v.z * v.z);
        Vec3 ground = p;
        terrain->QueryGround((TerrainVec3*)&ground, (TerrainVec3*)&normal, 0, 0);
        float g;
        if (ground.y + 0.5f >= p.y) {
            if (airborne && takeoffTime < 0.5f) {
                target = p;
                landTimer = 0.0f;
            }
            float d = BikeAIDotZ(normal, v);
            p.y = ground.y;
            if (d < 0.0f) {
                v -= normal * d;
                if (!(d < speed * -0.70755f))
                    v = SafeNormalize(v) * speed;
            }
            g = 0.0f;
            airborne = 0;
        } else {
            if (!airborne) {
                takeoffVel = v;
                takeoffSpeed = BikeAILength(v);
                takeoffPos = p;
                flatDir = v;
                flatDir.y = 0.0f;
                flatDir = BikeAINormalize(flatDir);
                takeoffTime = elapsed;
            } else if (align < 0.70755f) {
                done = 1;
                target = p;
            }
            g = -32.0f;
            airborne = 1;
        }
        p += dt * v;
        p.y += g * halfDtSq;
        v.y += g * dt;
        t += invLen * flatSpeed * dt;
        flatSpeed = FastSqrt(v.x * v.x + v.z * v.z);
        while (t > 1.0f && idx < path->count - 1) {
            t -= 1.0f;
            idx++;
            if (path->count == 1)
                seg = path->points[idx] - p;
            else if (idx < path->count - 1)
                seg = path->points[idx + 1] - path->points[idx];
            else
                seg = path->points[idx] - path->points[idx - 1];
            seg.y = 0.0f;
            len = BikeAILength(seg);
            invLen = len > 0.0f ? 1.0f / len : 0.0f;
            n.x = seg.z;
            n.y = 0.0f;
            n.z = -seg.x;
            n = SafeNormalize(n);
        }
        Vec3 aim = path->points[idx] + seg * t + n * side;
        if (airborne && idx == path->count - 1) {
            target = p;
            done = 1;
        }
        flatSpeed = FastSqrt(v.x * v.x + v.z * v.z);
        dir = aim - p;
        dir.y = 0.0f;
        dir = SafeNormalize(dir);
        v.x = dir.x * flatSpeed;
        v.z = dir.z * flatSpeed;
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
        Vec3 d = target - takeoffPos;
        float dist = FastSqrt(d.x * d.x + d.z * d.z);
        flatSpeed = FastSqrt(takeoffVel.x * takeoffVel.x + takeoffVel.z * takeoffVel.z);
        if (dist > 20.0f) {
            float sinA, secA;   // left unset when the takeoff has no flat speed (as retail)
            if (flatSpeed > 0.0f) {
                float spd = BikeAILength(takeoffVel);
                sinA = takeoffVel.y / spd;
                secA = spd / flatSpeed;
            }
            path->field_0x96c = FastSqrt(dist * dist * 32.0f / ((sinA * dist + dy) * 2.0f)) * secA;
            float need = path->field_0x96c;
            Vec3 nv = SafeNormalize(takeoffVel) * need;
            if (path->field_0x96c < 40.0f)
                path->field_0x96c = 40.0f;
            else if (takeoffSpeed - path->field_0x96c > 20.0f)
                path->field_0x96c = takeoffSpeed - 20.0f;
            takeoffVel = SafeNormalize(nv) * path->field_0x96c;
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
    g_UnknownGlobal578dc0[0] = pos;
    g_UnknownGlobal578dc0[0].y = pos.y + 3.0f;
    g_UnknownGlobal578dc0[1] = target;
    g_UnknownGlobal578dc0[1].y = target.y + 3.0f;
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
    float s = FastInvSqrt(lenSq);
    return BikeAIVec3Call(s * v.x, s * v.y, s * v.z);
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
    float s = FastInvSqrt(lenSq);
    return BikeAIVec3Call(v.x * s, v.y * s, v.z * s);
}
// By-value view of operator*(const Vec3&, float) 0x005015b0.
Vec3 BikeAIVec3Scale(const Vec3& v, float s);
inline Vec3 BikeAINormalizeScaleCall(const Vec3& v)
{
    float lenSq = Vec3DotCall(&v, &v);
    if (lenSq == 1.0f)
        return v;
    float s = FastInvSqrt(lenSq);
    return BikeAIVec3Scale(v, s);
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
extern int g_UnknownGlobal577a00;
extern int g_UnknownGlobal578da8;
extern int g_UnknownGlobal578dac;
extern int g_UnknownGlobal578df8;
extern int g_UnknownGlobal578dd8;
extern Vec3 g_UnknownGlobal578de0[2];
extern Vec3 g_UnknownGlobal577a30[2];
extern Vec3 g_UnknownGlobal577a48[2];
extern Vec3 g_UnknownGlobal5779d8[2];
extern Vec3 g_UnknownGlobal577a18[2];

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
int UnknownFunction415640(Vec3* seg, float radius, int skipVegetation, int ignoreCount,
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
    g_UnknownGlobal578de0[0] = up[0];
    g_UnknownGlobal578de0[1] = up[1];
    g_UnknownGlobal577a30[0] = seg[0];
    g_UnknownGlobal577a30[1] = seg[1];
    g_UnknownGlobal577a48[0] = low[0];
    g_UnknownGlobal577a48[1] = low[1];
    g_UnknownGlobal5779d8[0] = left[0];
    g_UnknownGlobal5779d8[1] = left[1];
    g_UnknownGlobal577a18[0] = right[0];
    g_UnknownGlobal577a18[1] = right[1];
    g_UnknownGlobal577a00 = 0;
    g_UnknownGlobal578da8 = 0;
    g_UnknownGlobal578dac = 0;
    g_UnknownGlobal578df8 = 0;
    g_UnknownGlobal578dd8 = 0;
    if (outPush)
        *outPush = Vec3(0.0f, 0.0f, 0.0f);
    if (outPoint)
        *outPoint = Vec3(0.0f, 0.0f, 0.0f);

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
                obj->field_0x98 = 1;
                if (obj->ownerType == 0x64 || obj->ownerType == 0x65) {
                    UnknownBikeAISegmentOwner* owner = (UnknownBikeAISegmentOwner*)obj->ownerObject;
                    d = owner->start - seg[0];
                    distSq = SquareMagnitude(d);
                    d = BikeAINormalizeVS(d);
                    UnknownFunction47b800(seg[0].x, seg[0].z, delta.x, delta.z, owner->start.x, owner->start.z,
                                          owner->dir.x, owner->dir.z, &s, &u);
                    if (s >= 0.0f && s <= 1.0f && u >= 0.0f && u <= 1.0f) {
                        hit = 1;
                        pt = owner->dir - acc;
                        float k = DotProduct(pt, d);
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
                    Vec3* center = (Vec3*)&obj->field_0x34;
                    d = *center - seg[0];
                    distSq = SquareMagnitude(d);
                    d = BikeAINormalizeVS(d);
                    UnknownFunction47b800(seg[0].x, seg[0].z, delta.x, delta.z, center->x, center->z,
                                          owner->dir.x, owner->dir.z, &s, &u);
                    if (s >= 0.0f && s <= 1.0f && u >= 0.0f && u <= 1.0f) {
                        hit = 1;
                        pt = owner->dir - acc;
                        float k = DotProduct(pt, d);
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
                            g_UnknownGlobal577a00 = 1;
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
                            g_UnknownGlobal578da8 = 1;
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
                            g_UnknownGlobal578dac = 1;
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
                            g_UnknownGlobal578df8 = 1;
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
                            g_UnknownGlobal578dd8 = 1;
                        }
                    }
                    if (g_UnknownGlobal577a00 || g_UnknownGlobal578da8 || g_UnknownGlobal578dac
                        || g_UnknownGlobal578df8 || g_UnknownGlobal578dd8) {
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
        GameObjectIterator* it = new(__FILE__, 0xb24) GameObjectIterator(g_BikeAIGame_0056e26c->root, 1, "CollisionObject");
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
// through 0x00415640, target speed from path->field_0x96c and the speed table, sharing
// of the longitudinal acceleration (0.65 / 0.35), integration, and the side axis leaning
// towards the lateral acceleration.  Writes back center, forward, side, both wheel
// velocities, their mean and its length, the suspension values, the contact flags, the
// airborne flag and the largest impact.  Returns 0 when the jump prediction fails, else
// 1.  Caller: KrustyBike 0x00413a0c.  Arithmetic tier 1 (decoded), names tier 3.
int UnknownFunction40eca0(Vec3* steer, Vec3* frontVel, Vec3* rearVel, Vec3* outVel, float* outSpeed,
                          Vec3* center, Vec3* forward, Vec3* side, float maxTime, float dt,
                          Terrain* terrain, char* outAirborne, float* outImpact, CollisionObject* probe,
                          float* frontSusp, float* rearSusp, int* outFrontContact, int* outRearContact,
                          UnknownBikeAIPath* path, float* speedTable, float sideOffset, int nearOnly)
{
    if (dt > 0.05f)
        dt = 0.05f;
    int frontContact = 1;
    int rearContact = 1;
    float frontK = 90.0f;
    float rearK = 90.0f;
    int steerSign = 0;
    int steps = 0;
    float frontC;
    float rearC;
    frontC = rearC = ((dt - 0.005f) * 4.21053f + 0.6f) * 0.6f;
    float speedScale = path ? path->field_0x970 : 1.5f;
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
        if (!UnknownFunction40e510(&aim, front, path, speed, sideOffset)) {
            aim = vF;
            aim.y = 0.0f;
            aim = SafeNormalize(aim);
        }
        takeoffVel = vF;
        if (!UnknownFunction40d200(front, vF, path, &aim, 0.5f, 0.1f, terrain, sideOffset, &takeoffTime,
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
                    float h = front.y + 4.0f - rec[0] * 5.0f;
                    probe->hitPoint.y = h;
                    if (h > groundF.y) {
                        groundF.y = h;
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
                    float h = back.y + 4.0f - rec[0] * 5.0f;
                    probe->hitPoint.y = h;
                    if (h > groundR.y) {
                        groundR.y = h;
                        normalR = *(Vec3*)(rec + 2);
                    }
                }
            }
            right.x = fwd.y * normalR.z - fwd.z * normalR.y;
            right.y = fwd.z * normalR.x - fwd.x * normalR.z;
            right.z = fwd.x * normalR.y - fwd.y * normalR.x;
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
                float m = sideDotF * kF;
                accF += sideF * (m + rollF);
                frontS -= m * halfDtSq + dampF * dt;
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
                float m = sideDotR * kR;
                accR += sideR * (m + rollR);
                rearS -= m * halfDtSq + dampR * dt;
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
                if (UnknownFunction415640(seg, 2.0f, g_BikeAIGame_0056e26c->field_0x2d80 == 0, probe->ignoreCount,
                                          (CollisionObject**)probe->ignoreList, speedF, &push, &avoidPoint,
                                          &avoidDist, nearOnly)
                    && !(push.x == 0.0f && push.z == 0.0f)) {
                    FastSqrt(vF.x * vF.x + vF.z * vF.z);
                    push.y = 0.0f;
                    push = SafeNormalize(push);
                    float d = DotProduct(vF, push);
                    if (d > 0.0f) {
                        aim = vF - push * d;
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
                int kind = g_BikeAIGame_0056e26c->raceKind;
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
                kind = g_BikeAIGame_0056e26c->raceKind;
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

            float w = DotProduct(vR, right) * invDt + DotProduct(accR, right);
            if (frontContact || rearContact) {
                accR -= right * w;
                if (rearContact) {
                    if (path) {
                        float accel = (path->field_0x96c - speedR) * invDt;
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
                if (!UnknownFunction40e510(&aim, front, path, speedF, sideOffset)) {
                    aim = fwd;
                    aim.y = 0.0f;
                    aim = SafeNormalize(aim);
                }
                takeoffVel = vF;
                if (!UnknownFunction40d200(front, vF, path, &aim, 0.5f, 0.1f, terrain, sideOffset, &takeoffTime,
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
            float h = front.y + 1.0f - rec[0] * 2.0f;
            probe->hitPoint.y = h;
            if (h > groundF.y) {
                groundF.y = h;
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
            float h = back.y + 1.0f - rec[0] * 2.0f;
            probe->hitPoint.y = h;
            if (h > groundR.y) {
                groundR.y = h;
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
        *outAirborne = (char)airborne;
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
// VehicleSpeedState::Method_004D2F50's third parameter as bool (retail pushes eax unmasked),
// the x/y-grouped dot product, dot(normal, forward) in both AI branches, and
// `(controlInput.y + 1) + 1` through a named temporary (one expression folds to + 2.0).
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
// rotated with the first division folded to * 0.2).
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

// The same step on a Vehicle smoother (Vehicle.cpp's VehSmooth).
inline void BikeAISmooth(VehicleSmoother* s, float dt, float target)
{
    float step = dt;
    if (!(step < s->timeConstant))
        step = s->timeConstant;
    s->blendFactor = step / s->timeConstant;
    s->smoothedValue = (target - s->smoothedValue) * s->blendFactor + s->smoothedValue;
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
    if (!field_0x7a4)
        field_0x478 = crashState == 0 && UnknownVirtualSlot80() && field_0x740->field_0x18a;
    else
        field_0x478 = 0;
    field_0x479 = crashState == 0 && UnknownVirtualSlot81();
    engineState->Method_004D2F50(stepTime, field_0x478, field_0x479);
    engineState->Method_004D3030(field_0x478, linearSpeed);
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
    if (!field_0x734) {
        steer.x = controlInput.x;
        steer.y = controlInput.y;
        if (UnknownVirtualSlot80())
            steer.z = 1.0f;
        else if (UpdateWheelRampLevels())
            steer.z = -1.0f;
        else
            steer.z = 0.0f;
    } else {
        field_0x828.x = 0.0f;
        field_0x828.y = 1.0f;
        field_0x828.z = 0.0f;
        steer = Vec3(0.0f, 0.0f, 0.0f);
        if (airborne) {
            PredictLanding(position, velocity, &field_0x834, 0, &field_0x828, (Terrain*)terrain);
            float pitch = BikeAIDot(field_0x828, bodyForward) - 0.087f;
            if (field_0x834 > 0.0f)
                steer.y = pitch * 3.5f / field_0x834;
            else
                steer.y = pitch;
            if (pitch > 0.0f)
                lean = BikeAIFilterStep(&g_BikeAIFilter_00577ac0, dt, 1.0f);
            else
                lean = BikeAIFilterStep(&g_BikeAIFilter_00577ac0, dt, -1.0f);
            BikeAIClampUnit(lean);
        } else {
            float pitch = BikeAIDot(field_0x828, bodyForward);
            if (BIKEAI_ABS(pitch) < 0.3f)
                pitch = 0.0f;
            pitch *= 4.0f;
            if (BIKEAI_ABS(-pitch) > 0.1f) {
                if (pitch > 0.0f)
                    lean = 0.1f;
                else
                    lean = -0.1f;
            } else {
                lean = pitch;
                BikeAIClampUnit(lean);
            }
        }
        float speedGap = aiPath.field_0x96c - linearSpeed;
        if (!airborne && speedGap > 0.0f) {
            field_0x804 = 1.0f;
            field_0x808 = 0.0f;
            field_0x478 = 1;
        } else {
            field_0x804 = 0.0f;
            if (speedGap < -20.0f)
                field_0x808 = 1.0f;
            else
                field_0x808 = 0.0f;
            field_0x478 = 0;
        }
    }
    if (BIKEAI_ABS(position.y) > 10000.0f)
        position.y = 0.0f;

    Vec3 start = position;
    Vec3* fwd = &bodyForward;
    Vec3* up = &bodyUp;
    worldAngularVelocity = *fwd * angularVelocity.z + *up * angularVelocity.y
                           - CrossProduct(*fwd, *up) * angularVelocity.x;
    Vec3* w = &worldAngularVelocity;
    field_0x7e0 = CrossProduct(*w, *fwd) * 3.5f + velocity;
    field_0x7ec = velocity - CrossProduct(*w, *fwd) * 3.5f;
    char airborneOut = airborne;
    UnknownFunction40eca0(&steer, &field_0x7e0, &field_0x7ec, &velocity, 0, &position,
                          &bodyForward, &bodyUp, dt, dt, field_0x740->field_0x4c, &airborneOut,
                          &impact, field_0x15e0, &field_0x7f8, &field_0x7fc,
                          &frontWheel->inContact, &rearWheel->inContact, &aiPath,
                          g_BikeAISpeedTable_00566f74[field_0x79c], field_0x824, field_0x11b0);
    worldAngularVelocity = -CrossProduct(field_0x7e0 - velocity, bodyForward) * 0.2857f;
    angularVelocity = modelNode->WorldToLocalDirection(worldAngularVelocity);
    linearSpeed = BikeAILength(velocity);
    turnRate = 0.0f;

    float travel = -field_0x7f8;
    frontWheel->w_0x2b0->ClampAndStepAlong(travel, 0, frontWheel->w_0x2b0->q_0xc0 * travel);
    rearWheel->w_0x2ac->q_0x98 = field_0x7fc;
    rearWheel->w_0x2ac->ClampAndStep(field_0x7fc, 0);
    if (airborne)
        anyWheelInContact = 0;
    else
        anyWheelInContact = 1;

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
    if (field_0x734 && airborne)
        controlInput.x = 0.0f;

    field_0x434 = BikeAIDot(velocity, bodyForward);
    rearWheel->w_0x148 = field_0x434;
    frontWheel->w_0x148 = field_0x434;
    rearWheel->w_0x27c = field_0x434;
    frontWheel->w_0x27c = field_0x434;
    rearWheel->w_0x14c = 0.0f;
    frontWheel->w_0x14c = 0.0f;
    if (field_0x434 > 0.0f)
        movingForward = 1;
    else
        movingForward = 0;
    if (frontWheel->inContact)
        ((KbWheel*)frontWheel)->SetRollDistance(field_0x434 / frontWheel->w_0x274 * dt);
    else
        ((KbWheel*)frontWheel)->SetRollDistance(field_0x434 / frontWheel->w_0x274 * dt);
    if (rearWheel->inContact)
        ((KbWheel*)rearWheel)->SetRollDistance(field_0x434 / rearWheel->w_0x274 * dt);
    else
        ((KbWheel*)rearWheel)->SetRollDistance(field_0x434 / rearWheel->w_0x274 * dt);
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
        field_0x814 += dt;
    else
        field_0x814 = 0;
    if (BIKEAI_ABS(impact) > field_0x778 || BIKEAI_ABS(linearSpeed - prevSpeed) > 44.0f ||
        bodyUp.y <= 0.0f || field_0x814 > 0.5f) {
        crashState = 1;
        crashDirection = 1;
        crashReason = 4;
        UnknownVirtualSlot41();
        field_0x574 = Vec3(savedForward.x, 0.0f, savedForward.z);
        field_0x431 = 0;
        field_0x433 = 0;
        if (savedUp.y < 0.0f)
            field_0x464 = 1;
        else
            field_0x464 = 0;
        field_0x45c = savedYaw;
        field_0x604->Method_0x00532220(crashDirection);
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
    frontWheel->w_0x0d8 += moved;
    rearWheel->w_0x0d8 += moved;
    UnknownVirtualSlot21();
    scratchVector = modelNode->WorldToLocalDirection(velocity);
    BikeAISmooth(forwardAccelSmoother, frameTime,
                 (scratchVector.z - prevLocalForwardVelocity) / frameTime);
    smoothedForwardAccel = forwardAccelSmoother->smoothedValue;
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
    float s = FastInvSqrt(lenSq);
    return Vec3(s * v.x, s * v.y, s * v.z);
}

// 0x00414370, 4802 bytes (ret 4): rebuilds the AI racing line aiPath for this frame and
// returns 1 (0 when a track query fails).  By race kind (game +0x2d74):
//  * 2 and 3 (track races): the shortest track path from the bike's progress position to
//    the race's target position, sampled every 0.1 of the speed (100 points) or ahead of
//    the bike; off the track for 5 s the bike is respawned through slots 11/33;
//  * 1 and 5: a short line along the gates of KbGhost+0x34 (3 points around the gate,
//    or the gate itself), or the start-gate run-up while +0x7a4 is set;
//  * otherwise: one random free-roam target 444 units (scaled by the terrain) away.
// The racing line's target speed (40 near a gate, else 1000) and the AI class's speed scale
// (+0x7c4..+0x7cc by game difficulty +0x60c) follow; a local bike may then start a trick
// while airborne (0x0048d780 picks it, 0x0048d910 plays it).  Names tier 3.
int KrustyBike::Fn_00414370(float dt)
{
    KbTrackItem* list;
    KbTrackItem** tail;
    KbRaw3 from;
    KbRaw3 p;
    KbGate* gate;
    Vec3 a, b, c;
    float distance;
    float right, left;
    float gateAhead, bikeAhead;
    float speed, t, n, lenSq, dx, dz;
    float trickDelay;
    int dummy;
    int count;
    int i;

    memset(&aiPath, 0, sizeof(aiPath));
    list = 0;
    dummy = 0;
    int difficulty = g_kbGame->field_0x60c;
    int kind = g_kbGame->field_0x2d74;
    if (kind != 3 && kind != 2) {
        if (kind != 1 && kind != 5) {
            aiPath.count = 1;
            Vec3 d;
            d = Vec3(field_0x818 - position.x, field_0x81c - position.y, field_0x820 - position.z);
            d.y = 0.0f;
            if (d.x * d.x + d.z * d.z < 2500.0f) {
                d.x = rand() * (1.0f / 32768.0f);
                d.y = 0.0f;
                d.z = rand() * (1.0f / 32768.0f);
                d = BikeAINormalizeInline(d) * 444.0f * terrain->worldScale;
                field_0x818 = terrain->worldScale * 666.66f + d.x;
                field_0x81c = 0.0f;
                field_0x820 = terrain->worldScale * 666.66f + d.z;
            } else {
                aiPath.points[0].x = field_0x818;
                aiPath.points[0].y = field_0x81c;
                aiPath.points[0].z = field_0x820;
            }
            aiPath.field_0x96c = 1000.0f;
            aiPath.field_0x970 = field_0x7cc;
            aiPath.lookAhead = linearSpeed * 0.25f;
            field_0x15dc = field_0x740->field_0xa4;
            if (difficulty == 3)
                trickDelay = field_0x7d8;
            else if (difficulty == 2)
                trickDelay = field_0x7d4;
            else if (difficulty == 1)
                trickDelay = field_0x7d0;
        } else {
            if (field_0x7a4) {
                field_0x80c += dt;
                if (field_0x80c < 2.0f) {
                    aiPath.count = 1;
                    aiPath.points[0].y = 0.0f;
                    aiPath.points[0].x = bodyForward.z * 40.0f + position.x;
                    aiPath.points[0].z = position.z - bodyForward.x * 40.0f;
                    aiPath.field_0x96c = 40.0f;
                    return 1;
                }
                aiPath.field_0x96c = 0.0f;
                return 1;
            }
            speed = linearSpeed;
            if (field_0x740->field_0x48) {
                if (!field_0x740->field_0x48->UnknownFunction516ca0(position, field_0x740->field_0x48->field_0x0,
                                                                    &field_0x744->field_0x38, 0))
                    return 0;
                if (!field_0x744->field_0x44.a) {
                    field_0x744->field_0x44.a = field_0x744->field_0x38.a;
                    field_0x744->field_0x44.b = field_0x744->field_0x38.b;
                    field_0x744->field_0x44.c = field_0x744->field_0x38.c;
                }
                gate = field_0x744->field_0x34;
                if (!gate->trackPos.a)
                    field_0x740->field_0x48->UnknownFunction516ca0(gate->position, field_0x740->field_0x48->field_0x0,
                                                                  &gate->trackPos, 0);
                gateAhead = field_0x740->field_0x48->UnknownFunction517da0(field_0x744->field_0x44,
                                                                          field_0x744->field_0x34->trackPos);
                bikeAhead = field_0x740->field_0x48->UnknownFunction517da0(field_0x744->field_0x38,
                                                                          field_0x744->field_0x34->trackPos);
                if (bikeAhead > 0.0f && gateAhead > 0.0f && bikeAhead < gateAhead) {
                    field_0x744->field_0x44.a = field_0x744->field_0x38.a;
                    field_0x744->field_0x44.b = field_0x744->field_0x38.b;
                    field_0x744->field_0x44.c = field_0x744->field_0x38.c;
                }
                list = (KbTrackItem*)DebugCalloc(1, 0x10, __FILE__, 0x99f);
                if (!list)
                    return 0;
                list->field_0x04 = field_0x740->field_0x48->field_0x0;
                list->field_0x0c = (KbTrackItem*)DebugCalloc(1, 0x10, __FILE__, 0x9a3);
                if (!list->field_0x0c)
                    return 0;
                list->field_0x0c->field_0x04 = field_0x740->field_0x48->field_0x0;
                if (list && bikeAhead < gateAhead) {
                    aiPath.count = 100;
                    t = 0.0f;
                    for (i = 0; i < aiPath.count; i++) {
                        if (!field_0x740->field_0x48->UnknownFunction517ea0(field_0x744->field_0x44, &p, &list,
                                                                           t * linearSpeed, 0))
                            return 0;
                        if (!field_0x740->field_0x48->UnknownFunction518080(p, &aiPath.points[i]))
                            return 0;
                        aiPath.points[i].y = position.y + 2.0f;
                        t += 0.1f;
                        if (t > 10.0f)
                            break;
                    }
                } else {
                    aiPath.count = 1;
                    aiPath.points[0] = field_0x744->field_0x34->position;
                }
            } else if (field_0x744->field_0x08 < 300.0f) {
                aiPath.count = 3;
                gate = field_0x744->field_0x34;
                if (BikeAIDot(gate->position - position, gate->direction) < 0.0f) {
                    aiPath.points[0] = speed * gate->direction + gate->position;
                    aiPath.points[2] = field_0x744->field_0x34->position - speed * field_0x744->field_0x34->direction;
                } else {
                    aiPath.points[0] = gate->position - speed * gate->direction;
                    aiPath.points[2] = speed * field_0x744->field_0x34->direction + field_0x744->field_0x34->position;
                }
                aiPath.points[1] = field_0x744->field_0x34->position;
            } else {
                aiPath.count = 1;
                aiPath.points[0] = field_0x744->field_0x34->position;
            }
            gate = field_0x744->field_0x34;
            dx = position.x - gate->position.x;
            dz = position.z - gate->position.z;
            if (FastSqrt(dx * dx + dz * dz) < gateAhead) {
                if (BikeAIDot(SafeNormalize(velocity), SafeNormalize(gate->position - position)) < 0.70755f)
                    aiPath.field_0x96c = 40.0f;
                else
                    aiPath.field_0x96c = 1000.0f;
                aiPath.lookAhead = 20.0f;
                field_0x11b0 = 1;
            } else {
                aiPath.field_0x96c = 1000.0f;
                aiPath.field_0x970 = field_0x7cc;
                field_0x11b0 = 0;
                aiPath.lookAhead = linearSpeed * 0.25f;
            }
            if (difficulty == 3)
                aiPath.field_0x970 = field_0x7cc;
            else if (difficulty == 2)
                aiPath.field_0x970 = field_0x7c8;
            else if (difficulty == 1)
                aiPath.field_0x970 = field_0x7c4;
            aiPath.lookAhead = linearSpeed * 0.25f;
        }
    } else {
        if (!field_0x744->field_0x44.a || !field_0x744->field_0x44.b)
            return 0;
        if (!field_0x740->field_0x134.a || !field_0x740->field_0x134.b)
            return 0;
        if (!field_0x740->field_0x48->UnknownFunction5179f0(field_0x744->field_0x44, field_0x740->field_0x134,
                                                             &list, &distance))
            return 0;
        tail = &list;
        while (*tail)
            tail = &(*tail)->field_0x0c;
        from = field_0x740->field_0x134;
        from.t += 0.01f;
        if (!field_0x740->field_0x48->UnknownFunction5179f0(from, field_0x740->field_0x134, tail, &distance))
            return 0;
        if (field_0x78c) {
            if (list) {
                aiPath.count = 100;
                t = 0.0f;
                for (i = 0; i < aiPath.count; i++) {
                    if (!field_0x740->field_0x48->UnknownFunction517ea0(field_0x744->field_0x44, &p, &list,
                                                                       t * linearSpeed, 0))
                        return 0;
                    if (!field_0x740->field_0x48->UnknownFunction518080(p, &aiPath.points[i]))
                        return 0;
                    aiPath.points[i].y = position.y + 2.0f;
                    t += 0.1f;
                    if (t > 10.0f)
                        break;
                }
            } else {
                aiPath.count = 1;
                aiPath.points[0] = linearSpeed * bodyForward + position;
            }
            if (!g_kbGame->field_0x2d70)
                field_0x11b4 = 0.0f;
        } else {
            count = 1;
            if (!g_kbGame->field_0x2d70) {
                field_0x11b4 += dt;
                if (field_0x11b4 >= 5.0f) {
                    UnknownVirtualSlot11(count, &a, &b, &c, &dummy);
                    UnknownVirtualSlot33(&a, &b, &bodyUp, &c, dummy, UnknownVirtualSlot32());
                    field_0x11b4 = 0.0f;
                }
            }
            aiPath.count = count;
            if (list) {
                t = 1.0f;
                for (i = 0; i < aiPath.count; i++) {
                    if (!field_0x740->field_0x48->UnknownFunction517ea0(field_0x744->field_0x44, &p, &list,
                                                                       g_BikeAILookAhead_00572988 * t, 0))
                        return 0;
                    if (!field_0x740->field_0x48->UnknownFunction518080(p, &aiPath.points[i]))
                        return 0;
                    aiPath.points[i].y = position.y + 2.0f;
                    lenSq = SquareMagnitude(aiPath.points[i] - position);
                    if (lenSq == 1.0f || FastSqrt(lenSq) < 100.0f)
                        aiPath.field_0x96c = 40.0f;
                    t += 0.1f;
                    if (t > 1.0f)
                        break;
                }
            } else {
                p = field_0x744->field_0x44;
                if (!field_0x740->field_0x48->UnknownFunction518080(field_0x744->field_0x44, &aiPath.points[0]))
                    return 0;
            }
        }
        aiPath.field_0x96c = 1000.0f;
        aiPath.field_0x970 = field_0x7cc;
        aiPath.lookAhead = linearSpeed * 0.25f;
        if (difficulty == 3)
            aiPath.field_0x970 = field_0x7cc;
        else if (difficulty == 2)
            aiPath.field_0x970 = field_0x7c8;
        else if (difficulty == 1)
            aiPath.field_0x970 = field_0x7c4;
        if (field_0x7a4) {
            field_0x78c = field_0x740->field_0x48->UnknownFunction517340(position, field_0x744->field_0x44,
                                                                        -1.0f, -1.0f, 0, 0);
            if (!field_0x78c) {
                aiPath.field_0x96c = 0.0f;
                goto done;
            }
            aiPath.field_0x96c = 40.0f;
            Vec3 dir;
            if (!field_0x740->field_0x48->UnknownFunction518130(field_0x744->field_0x44.b, &dir))
                return 0;
            field_0x740->field_0x48->UnknownFunction518230(position, field_0x744->field_0x44.b, &right, 10);
            field_0x740->field_0x48->UnknownFunction518230(position, field_0x744->field_0x44.b, &left, 11);
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
            aiPath.points[0] = aiPath.field_0x96c * dir + position;
        }
        if (field_0x78c)
            field_0x11b0 = 1;
        else
            field_0x11b0 = 0;
    }
    if (field_0x15e4 && !(trickDelay > field_0x7dc)) {
        if (!field_0x430 && airborne) {
            PredictLanding(position, velocity, &field_0x834, 0, 0, (Terrain*)terrain);
            if (BikeAIRandUnit() * 100.0f <= field_0x15dc) {
                for (n = 5.0f; n >= 1.0f; n -= 1.0f) {
                    if (Fn_0048D780(&field_0x15d4, field_0x834 / n - 0.5f, 0))
                        break;
                }
                if (n > 0.0f) {
                    Fn_0048D910(field_0x15d4);
                    field_0x7dc = 0;
                }
            }
        }
    } else {
        field_0x7dc += dt;
    }
done:
    if (list)
        field_0x740->field_0x48->UnknownFunction517930(&list, 0);
    return 1;
}
