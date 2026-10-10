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
#include "collision/CollisionObject.h"
#include "broadphase/Quadtree.h"
#include <math.h>
#include <string.h>

// AI racing line: up to 200 points.  Offsets tier 1 (0x0040e510), names tier 3.
struct UnknownBikeAIPath {
    int count;          // +0x000
    int index;          // +0x004 current segment
    float t;            // +0x008 parameter along the current segment
    Vec3 points[200];   // +0x00c
    float field_0x96c;  // +0x96c takeoff speed written by 0x0040d200 (fstp, clamped to >= 40.0f)
    float field_0x970;  // +0x970 read by 0x0040eca0 (1.5f when there is no path)
    float lookAhead;    // +0x974
};

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

// Inline length helper: same body as the out-of-line 0x00413190 (inlined in 0x0040e510).
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
