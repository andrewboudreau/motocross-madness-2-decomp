#pragma once

// An axis-aligned clip rectangle in the x/y plane with an outcode
// (Cohen-Sutherland) segment clipper and a polygon clipper that walks the
// rectangle's corners when the polygon wraps around it. No RTTI, vtable or
// __FILE__: the code sits at 0x004312b0..0x004318cf between the
// CarProcedural.cpp and CollisionCharacter.cpp brackets. The only users are
// D3DIMSoultreeShadow (0x00446f7d, 0x004472a5) and TerrainShadow
// (0x00509b0d, 0x00509e86), both through the pointer at 0x0056852c, which is
// initialised to the static instance at 0x00579730. The class, member and
// file names are provisional.

// A plain x/y/z point: retail constructs the corner array without a vector
// constructor iterator, so the element type has no constructor.
struct ClipPoint {
    float x;
    float y;
    float z;
};

struct ClipSegment {
    ClipPoint start;    // +0x00
    ClipPoint end;      // +0x0c
};

class ClipRectangle {
public:
    ClipRectangle();    // 0x004312f0
    ~ClipRectangle();   // empty, folded into 0x00464e90

    // 0x00431350 (ret 0x10)
    void Set(float left, float bottom, float right, float top);
    // 0x004313d0 (ret 0xc)
    int Outcode(ClipPoint point);
    // 0x00431430 (ret 0xc)
    int IntersectEdge(const ClipSegment* segment, int outcode, ClipPoint* out);
    // 0x004314e0 (ret 0x10)
    int ClipLine(const ClipSegment* segment, ClipSegment* out, int* enterEdge, int* exitEdge);
    // 0x00431680 (ret 0xc)
    int ClipPolygon(ClipPoint* points, ClipPoint* out, int count);

    float left;             // +0x00
    float bottom;           // +0x04
    float right;            // +0x08
    float top;              // +0x0c
    ClipPoint corners[8];   // +0x10: four corners, repeated at +0x40
};

extern ClipRectangle* g_clipRectangle;  // 0x0056852c = &0x00579730
