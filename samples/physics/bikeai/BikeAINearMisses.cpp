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
#include "../../../src/krusty2/vehicle/BikeAI.cpp"

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

// View of the game object at 0x0056e26c (TrackGame in src/reconstructed): only the race
// kind at +0x2d74 is read here.
struct UnknownBikeAIGameView {
    char pad_0x0000[0x2d74];
    int raceKind;       // +0x2d74
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

// 0x0040e510, 1936 bytes, cdecl: steering direction along an AI path.  Finds the
// path point nearest to 'pos', refines the segment by intersecting the perpendicular
// through 'pos' with the neighbouring segments, walks 'lookAhead' (+ |offset| in race
// kinds 2 and 3) along the path, and returns in *outDir the flat unit direction from
// 'pos' to that point shifted sideways by 'side'.  Stores the reached segment/parameter
// in the path.  Callers: 0x0040f1ef and 0x0041256c.  Names tier 3.
int UnknownFunction40e510(Vec3* outDir, Vec3 pos, UnknownBikeAIPath* path, int unused,
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
