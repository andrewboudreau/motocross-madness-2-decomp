// Group 3 bring-up: providers that BikeAI.cpp's KrustyBike steps (0x00413200, 0x00414370 in
// src/krusty2/vehicle/BikeAI.cpp) and the look-ahead simulation 0x0040eca0 call through tier-3
// views (KbRaceSub, KbWheel) that no src/ file defines.  KrustyBike::PickTrick is
// linked from samples/physics/bikeai/KrustyBikeTrickPick.cpp.
//
// Status: best-guess-unreviewed.  Each body was read from the retail disassembly at the
// noted address; byte matching is not attempted here.  The view methods forward to the
// canonical provider for the same retail entry when one is already built (Track.cpp,
// soultree.cpp); otherwise the retail body is written out against the view's offsets.
#include <float.h>

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
