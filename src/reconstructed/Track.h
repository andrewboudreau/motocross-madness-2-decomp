#pragma once

// Track.cpp (literal __FILE__ "D:\aardvark\VC\krusty2\Track.cpp", 0x0057508c).
// No RTTI names a class here: the functions are __thiscall members of a
// non-polymorphic object whose name, Track, is inferred from the file name
// only. The loader 0x00515ed0 stores the start node at +0x00 and the lap
// length at +0x04. The functions that never read `this` are still called
// with ecx set, so they are members too. Structure sizes are the DebugCalloc
// sizes at the allocation sites; names are provisional (docs/TRACK.md).

struct TrackSegment;

// A track node (calloc(1, 0x18), Track.cpp line 108): a run of segments
// plus links to the nodes that follow it.
struct TrackNode {
    unsigned char field_0x00;       // bit 0: read from the file (chunk 6); bit 2: visited
    unsigned char field_0x01[3];
    float field_0x04;               // length used by the path search (0x005179f0)
    TrackSegment* field_0x08;       // first segment
    TrackSegment* field_0x0c;       // last segment
    int field_0x10;                 // link count
    TrackNode** field_0x14;         // links (indices while loading)
};

// A segment point (calloc(1, 0x30), Track.cpp line 141), kept in a doubly
// linked list per node.
struct TrackSegment {
    float field_0x00;               // x
    float field_0x04;               // y
    float field_0x08;               // z
    float field_0x0c;
    float field_0x10;
    float field_0x14;
    float field_0x18;
    float field_0x1c;
    float field_0x20;
    float field_0x24;               // horizontal distance to the next point
    TrackSegment* field_0x28;       // previous
    TrackSegment* field_0x2c;       // next
};

// A position on the track, passed by value (12 bytes).
struct TrackPos {
    TrackNode* node;
    TrackSegment* segment;
    float t;                        // 0..1 along the segment
};

struct TrackVec3 {
    float x;
    float y;
    float z;
};

// Work-list entry of the graph walks (calloc(1, 0x10)); only +0x04 and
// +0x0c are used.
struct TrackListItem {
    unsigned char field_0x00;       // bit 1: walked in reverse (0x00517ea0)
    unsigned char field_0x01[3];
    TrackNode* field_0x04;
    TrackListItem* field_0x08;
    TrackListItem* field_0x0c;
};

// 0x00515dc0/0x00515df0: a strtok-style splitter. Callers are in gameui.cpp,
// Parameterblocks.cpp and others; it is placed in Track.cpp by position only
// (it sits directly before 0x00515e70).
class UnknownTokenizer {
public:
    UnknownTokenizer(char* text);           // 0x00515dc0
    char* UnknownFunction515df0(const char* delimiters);

    char* field_0x00;               // current token
    char* field_0x04;               // rest of the text
};

class UnknownStream;

class Track {
public:
    int UnknownFunction515e70(UnknownStream* stream, TrackNode** nodes, int count);
    int UnknownFunction516800(TrackNode* node);
    int UnknownFunction516870(TrackNode** start);
    int UnknownFunction516ca0(TrackVec3 p, TrackNode* node, TrackPos* out, float* outDistance);
    int UnknownFunction516ef0(TrackVec3 p, TrackSegment* segment, TrackSegment* next);
    int UnknownFunction517310(TrackSegment* segment, TrackNode* node);
    int UnknownFunction517930(TrackListItem** list, int all);
    int UnknownFunction5179a0(TrackPos a, TrackPos b);
    int UnknownFunction5179f0(TrackPos a, TrackPos b, TrackListItem** path, float* distance);
    float UnknownFunction517da0(TrackPos a, TrackPos b);
    int UnknownFunction517ea0(TrackPos pos, TrackPos* out, TrackListItem** path, float distance, unsigned char flags);
    int UnknownFunction518080(TrackPos pos, TrackVec3* out);
    int UnknownFunction518130(TrackSegment* segment, TrackVec3* out);

    TrackNode* field_0x00;          // start node
    float field_0x04;               // lap length
};

// 0x00518640: formats whole seconds as "mm:ss".
void UnknownFunction518640(char* text, float seconds);
// 0x00518690: formats a lap time as "m:ss.ss", or "--:--.--" when unset.
void UnknownFunction518690(char* text, float seconds);
