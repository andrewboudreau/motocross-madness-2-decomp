// Group5 bring-up definition (best-guess, link frontier). Not a strict match.
//
// 0x00518080 is Track::PointAt (defined in Track.cpp). Krusty3DObjects.h
// views the racer's track (+0x48 of the race view) as UnknownRunwayTrack and
// its position as UnknownRunwayTrackPos (node, segment, t), the same 12-byte
// layout as Track.h's TrackPos; the output is three floats in both.
struct Vector3;
struct UnknownRunwayTrackPos {
    void* node;
    void* segment;
    float t;
};
struct UnknownRunwayTrack {
    int PointAt(UnknownRunwayTrackPos pos, Vector3* out);
};

// Track.h's provider, declared only as far as this call needs.
struct TrackNode;
struct TrackSegment;
struct TrackPos {
    TrackNode* node;
    TrackSegment* segment;
    float t;
};
struct TrackVec3;
class Track {
public:
    int PointAt(TrackPos pos, TrackVec3* out);  // 0x00518080
};

int UnknownRunwayTrack::PointAt(UnknownRunwayTrackPos pos, Vector3* out)
{
    TrackPos trackPos;
    trackPos.node = static_cast<TrackNode*>(pos.node);
    trackPos.segment = static_cast<TrackSegment*>(pos.segment);
    trackPos.t = pos.t;
    return reinterpret_cast<Track*>(this)->PointAt(trackPos, reinterpret_cast<TrackVec3*>(out));
}
