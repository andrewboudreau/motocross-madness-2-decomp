// Near-miss BikeAI.cpp candidates (D:\aardvark\VC\krusty2\BikeAI.cpp), kept out of
// src/krusty2/vehicle/BikeAI.cpp until they match.  The canonical file is included
// first so the TU-local statics and helpers are the same.
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
    int field_0x96c;    // +0x96c
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
