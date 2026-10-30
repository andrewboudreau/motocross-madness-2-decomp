// Group 3 bring-up: providers that BikeAI.cpp's KrustyBike steps (0x00413200, 0x00414370 in
// Group3_KrustyBikeAI.cpp) and the look-ahead simulation 0x0040eca0 call through tier-3
// views (KbRaceSub, KbWheel, BikeQ, KrustyBike::Fn_0048D780) that no src/ file defines.
//
// Status: best-guess-unreviewed.  Each body was read from the retail disassembly at the
// noted address; byte matching is not attempted here.  The view methods forward to the
// canonical provider for the same retail entry when one is already built (Track.cpp,
// soultree.cpp); otherwise the retail body is written out against the view's offsets.
#include <float.h>
#include <stdlib.h>

#include "math/Math3D.h"
#include "core/SoultreeObject.h"
#include "vehicle/KrustyBike.h"
#include "Track.h"

// ---------------------------------------------------------------------------------------
// KbRaceSub: the race's Track (BikeRace+0x48).  Every method is the Track entry of the same
// retail address; KbRaw3 {node, segment, t} is TrackPos and Vec3 is TrackVec3 / Vector3
// (three floats), so the forwarders only reinterpret the arguments.
// ---------------------------------------------------------------------------------------
static inline Track* G3Track(KbRaceSub* t) { return reinterpret_cast<Track*>(t); }
static inline TrackPos G3Pos(const KbRaw3& p) { return *reinterpret_cast<const TrackPos*>(&p); }
static inline TrackVec3 G3TVec(const Vec3& v) { return *reinterpret_cast<const TrackVec3*>(&v); }

// 0x00516ca0.. (Track::ClosestPointOnNode)
int KbRaceSub::ClosestPointOnNode(Vec3 p, int* node, KbRaw3* out, float* outDistance)
{
    return G3Track(this)->ClosestPointOnNode(G3TVec(p), reinterpret_cast<TrackNode*>(node),
                                             reinterpret_cast<TrackPos*>(out), outDistance);
}

int KbRaceSub::PlaceNear(Vec3 p, KbRaw3 from, float distance, float range, int path, KbRaw3* out)
{
    return G3Track(this)->PlaceNear(*reinterpret_cast<Vector3*>(&p), G3Pos(from), distance, range,
                                    path, reinterpret_cast<TrackPos*>(out));
}

int KbRaceSub::FreeWorkList(KbTrackItem** list, int all)
{
    return G3Track(this)->FreeWorkList(reinterpret_cast<TrackListItem**>(list), all);
}

int KbRaceSub::ShortestPath(KbRaw3 a, KbRaw3 b, KbTrackItem** path, float* distance)
{
    return G3Track(this)->ShortestPath(G3Pos(a), G3Pos(b), reinterpret_cast<TrackListItem**>(path),
                                       distance);
}

float KbRaceSub::DistanceAlong(KbRaw3 a, KbRaw3 b)
{
    return G3Track(this)->DistanceAlong(G3Pos(a), G3Pos(b));
}

int KbRaceSub::MoveAlongPath(KbRaw3 pos, KbRaw3* out, KbTrackItem** path, float distance,
                             unsigned char flags)
{
    return G3Track(this)->MoveAlongPath(G3Pos(pos), reinterpret_cast<TrackPos*>(out),
                                        reinterpret_cast<TrackListItem**>(path), distance, flags);
}

int KbRaceSub::PointAt(KbRaw3 pos, Vec3* out)
{
    return G3Track(this)->PointAt(G3Pos(pos), reinterpret_cast<TrackVec3*>(out));
}

int KbRaceSub::SegmentDirection(int segment, Vec3* out)
{
    return G3Track(this)->SegmentDirection(reinterpret_cast<TrackSegment*>(segment),
                                           reinterpret_cast<TrackVec3*>(out));
}

int KbRaceSub::DistanceToStripEdge(Vec3 p, int segment, float* out, int mode)
{
    return G3Track(this)->DistanceToStripEdge(G3TVec(p), reinterpret_cast<TrackSegment*>(segment),
                                              out, mode);
}

// ---------------------------------------------------------------------------------------
// KbWheel::SetRollDistance, 0x00513560 (Tire::SetRollDistance, samples/physics/tire/Tire.cpp):
// rollAngle (+0x278) = distance * invWheelRadius (+0x1c8); a finite angle rolls the wheel's
// scene node (+0x1bc) about its local X axis.
// ---------------------------------------------------------------------------------------
struct G3TireView {
    char pad_0x000[0x1bc];
    SoultreeObject* sceneNode;   // +0x1bc
    char pad_0x1c0[0x1c8 - 0x1c0];
    float invWheelRadius;        // +0x1c8
    char pad_0x1cc[0x278 - 0x1cc];
    float rollAngle;             // +0x278
};

void KbWheel::SetRollDistance(float d)
{
    G3TireView* tire = reinterpret_cast<G3TireView*>(this);
    tire->rollAngle = d * tire->invWheelRadius;
    if (_finite(tire->rollAngle))
        tire->sceneNode->RotateAbout(1.0f, 0.0f, 0.0f, tire->rollAngle);
}

// ---------------------------------------------------------------------------------------
// BikeQ: the wheel's shock (InlineShock at BikeWheel+0x2b0, RotatingShock at +0x2ac).  The
// layout is the Shock of src/reconstructed/Suspension.h (MovingPart at +0x04 with its local
// matrix at +0x04 and scene node at +0x44).  Bodies from 0x004fa090 / 0x004fa8b0
// (samples/physics/suspension/Shock.cpp, checked against the retail disassembly).
// ---------------------------------------------------------------------------------------
struct G3ShockView {
    void* vtable;                // +0x00
    char localMatrix[0x40];      // +0x04 MovingPart local matrix (Matrix4)
    SoultreeObject* sceneNode;   // +0x44
    char pad_0x48[0x08];
    int atLimit;                 // +0x50
    int extending;               // +0x54
    Vec3 spring;                 // +0x58
    Vec3 damper;                 // +0x64
    float ratio;                 // +0x70
    Vec3 displacement;           // +0x74
    Vec3 stepDisplacement;       // +0x80
    float prevLength;            // +0x8c
    float clearedLength;         // +0x90
    float impulseScale;          // +0x94
    float length;                // +0x98
    char pad_0x9c[0xb0 - 0x9c];
    Vec3 velocity;               // +0xb0
    char pad_0xbc[0xc0 - 0xbc];
    // InlineShock: axis +0xc0, minLength +0xcc.  RotatingShock: maxAngle +0xc0, chord +0xc4,
    // armLength +0xc8, angle +0xd0, rotationAxis +0xd4.
    float f_0xc0;
    float f_0xc4;
    float f_0xc8;
    float f_0xcc;
    float angle;                 // +0xd0 (RotatingShock)
    Vec3 rotationAxis;           // +0xd4 (RotatingShock)
};

static inline Vec3 G3Scale(const Vec3& v, float s) { return Vec3(v.x * s, v.y * s, v.z * s); }

// Common tail of both ClampAndStep bodies: the per-step velocity from the length change,
// clamped to maxDelta, and the step displacement.
static void G3ShockStepTail(G3ShockView* s, float maxDelta, float dt)
{
    float delta = s->prevLength - s->length;
    if (s->length != 0.0f) {
        if (delta >= 0.0f) {
            float clamped = (delta < maxDelta) ? delta : maxDelta;
            s->velocity = G3Scale(s->displacement, clamped / (dt * s->length));
        } else {
            float clamped = (delta > -maxDelta) ? delta : -maxDelta;
            s->velocity = G3Scale(s->displacement, clamped / (dt * s->length));
        }
        s->stepDisplacement = G3Scale(s->displacement, -(delta / s->length));
    } else {
        // kShockZero (0x00689e58) is the zero vector.
        s->velocity = Vec3(0.0f, 0.0f, 0.0f);
        s->stepDisplacement = Vec3(0.0f, 0.0f, 0.0f);
    }
    s->extending = (s->length > s->prevLength);
    s->prevLength = s->length;
}

// 0x004fa090 (InlineShock::ClampAndStep, ret 0xc).
void BikeQ::ClampAndStepAlong(float maxDelta, float dt, Vec3& pos)
{
    G3ShockView* s = reinterpret_cast<G3ShockView*>(this);
    const Vec3& axis = *reinterpret_cast<const Vec3*>(&s->f_0xc0);
    float minLength = s->f_0xcc;
    if (s->length >= minLength) {
        float negMin = -minLength;
        s->atLimit = 1;
        s->ratio = 1.0f;
        pos = G3Scale(axis, negMin);
        s->displacement = G3Scale(s->displacement, minLength / s->length);
        s->length = minLength;
    } else {
        s->atLimit = 0;
        s->ratio = s->length / minLength;
    }
    s->sceneNode->SetPosition(pos);
    G3ShockStepTail(s, maxDelta, dt);
}

// 0x004fa8b0 (RotatingShock::ClampAndStep, ret 8).
void BikeQ::ClampAndStep(float maxDelta, float dt)
{
    G3ShockView* s = reinterpret_cast<G3ShockView*>(this);
    float chord = s->f_0xc4;
    if (s->length >= chord) {
        float limitScale = chord / s->length;
        s->atLimit = 1;
        s->ratio = 1.0f;
        s->angle = s->f_0xc0;            // maxAngle
        s->displacement = G3Scale(s->displacement, limitScale);
        s->length = chord;
    } else {
        s->angle = s->length / s->f_0xc8;  // armLength
        s->atLimit = 0;
        s->ratio = s->length / chord;
    }
    if (_finite(s->angle)) {
        s->sceneNode->SetLocalMatrix(reinterpret_cast<const Matrix4*>(s->localMatrix));
        s->sceneNode->Rotate(s->rotationAxis, s->angle);
    }
    G3ShockStepTail(s, maxDelta, dt);
}

// ---------------------------------------------------------------------------------------
// KrustyBike::Fn_0048D780 (0x0048d780, ret 0xc): picks a trick that fits in `t` seconds.
// The trick table 0x0056cca0 holds 12 rows of 0x50 bytes in decreasing clip length:
// {minimum time, all count, first-group count, second-group count, 16 trick ids}.  The first
// row whose time is below t is used; `group` 0 draws from all its tricks, 1 from the first
// group, 2 from the second group (the first when the second is empty).  An AI bike also
// sets its trick hold time from the slack left after the clip (0x0056cc60 clip lengths);
// it fails when that slack is under 0.1 s.  Returns 1 with *out the trick id, else 0.
// ---------------------------------------------------------------------------------------
struct G3TrickRow {
    float minTime;
    int allCount;
    int firstCount;
    int secondCount;
    int tricks[16];
};

static const G3TrickRow s_g3AiTrickRows[12] = {   // 0x0056cca0
    {2.4000001f, 16, 8, 8, {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15}},
    {2.3333001f, 15, 7, 8, {0, 1, 2, 3, 4, 5, 6, 8, 9, 10, 11, 12, 13, 14, 15, 0}},
    {2.2665999f, 12, 5, 7, {0, 1, 2, 4, 5, 8, 9, 10, 12, 13, 14, 15, 0, 0, 0, 0}},
    {2.1333001f, 11, 4, 7, {0, 1, 4, 5, 8, 9, 10, 12, 13, 14, 15, 0, 0, 0, 0, 0}},
    {1.9375f, 8, 4, 4, {0, 1, 4, 5, 8, 9, 10, 14, 15, 0, 0, 0, 0, 0, 0, 0}},
    {1.8f, 9, 4, 5, {0, 1, 4, 5, 8, 9, 10, 15, 0, 0, 0, 0, 0, 0, 0, 0}},
    {1.7647001f, 7, 4, 3, {0, 1, 4, 5, 8, 10, 15, 0, 0, 0, 0, 0, 0, 0, 0, 0}},
    {1.7222000f, 6, 4, 2, {0, 1, 4, 5, 8, 15, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}},
    {1.6667000f, 4, 4, 0, {0, 1, 4, 5, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}},
    {1.6111000f, 3, 3, 0, {0, 1, 5, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}},
    {1.5f, 2, 2, 0, {1, 5, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}},
    {1.3f, 1, 1, 0, {1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}},
};

static const float s_g3AiTrickLength[16] = {      // 0x0056cc60 clip lengths (s)
    1.6111f, 1.3f, 2.2666f, 2.3333f, 1.6667f, 1.5f, 2.3333f, 2.4f,
    1.7222f, 1.8f, 1.7647f, 2.3333f, 2.1333f, 2.1333f, 1.9375f, 1.7222f,
};

int KrustyBike::Fn_0048D780(int* out, float t, int group)
{
    int row;
    for (row = 0; row < 12; row++) {
        if (t > s_g3AiTrickRows[row].minTime)
            break;
    }
    if (row >= 12)
        return 0;
    const G3TrickRow& r = s_g3AiTrickRows[row];
    int first = 0;
    int count;
    switch (group) {
    case 0:
        count = r.allCount;
        break;
    case 1:
        count = r.firstCount;
        break;
    case 2:
        if (r.secondCount) {
            first = r.firstCount;
            count = r.secondCount;
        } else {
            count = r.firstCount;
        }
        break;
    default:
        return 0;
    }
    // best guess: retail converts with `fistp` (round to nearest) after subtracting 0.5, i.e.
    // floor() of rand/32768 * count; truncation gives the same index except at exact ties.
    int index = (int)(rand() * (1.0f / 32768.0f) * (float)count);
    int trick = r.tricks[index + first];
    *out = trick;
    if (aiKind) {
        float slack = t - s_g3AiTrickLength[trick];
        if (slack < 0.1f)
            return 0;
        aiTrickHoldLeft = rand() * (1.0f / 32768.0f) * slack * 0.8f;
    }
    return 1;
}
