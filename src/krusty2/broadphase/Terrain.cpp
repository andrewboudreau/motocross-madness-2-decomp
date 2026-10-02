// Terrain.cpp -- reconstruction of D:\aardvark\VC\krusty2\Terrain.cpp.
#include "Terrain.h"

Terrain::Terrain(int a)
    : GameObject(a)
{
    field_0x40 = 1.0f;
    field_0x3c = 0;
    field_0x84 = 0;
    field_0x88 = 0;
    field_0x90 = 0;
    field_0x94 = 0;
    field_0x98 = 0;
    field_0x9c = 0;
    field_0xa0 = 0;
    field_0xa4 = 0;
    field_0xa8 = 0;
    field_0xac = 0;
    field_0xb0 = 0;
    field_0x34 = 0;
    field_0xb4 = 0;
    field_0x540 = 0;
    field_0x53c = 0;
    field_0x44 = 0;
    field_0xbec = 1000;
    field_0xbf4 = 0;
    field_0x30 = 0;
    field_0xbf8 = 0;
    field_0x8c = 0;
    field_0xbfc = 0;
    field_0xc00 = 0;
    field_0xc04 = 0;
    field_0xc08 = 0;
    field_0xc0c = 0;
    field_0x6c = 0;
    field_0x70 = 0;
    field_0x78 = 0;
    field_0x74 = 0;
    field_0x7c = 0;
    field_0x80 = 0;
    field_0xc10 = 0;
    field_0xc14 = 0;
    field_0xc30 = 1.0f;
    field_0xc34 = 0;
    field_0xc38 = 0;
    field_0xc3c = 0;
    TerrainMatrix tmp;
    field_0xc44 = *GetIdentityMatrix(&tmp);
    field_0xcb8 = 0;
    field_0xcbc = 0;
    field_0xcc0 = 0;
    field_0xcac = 0;
    field_0xcb0 = 0;
    field_0xcb4 = 0;
}

Terrain::~Terrain()
{
    int scope = g_MemTagStack->Push("Terrain");
    if (field_0xcb8) {
        for (int i = 0; i < field_0xcbc; i++)
            field_0xcb8[i]->BaseObjectVirtualSlot2();
        operator delete(field_0xcb8, __FILE__, 0x4b3);
    }
    if (field_0xc3c)
        field_0xc3c->BaseObjectVirtualSlot2();
    if (field_0x30)
        field_0x30->BaseObjectVirtualSlot2();
    if (field_0x540) {
        for (int i = 0; i < field_0x540; i++) {
            if (field_0x544[i])
                field_0x544[i]->BaseObjectVirtualSlot2();
        }
    }
    if (field_0x44) {
        field_0x44->Shutdown(1);
        if (field_0x44)
            delete field_0x44;
    }
    if (field_0x34) {
        field_0x34->Release();
        field_0x34 = 0;
    }
    if (field_0xc14) {
        for (int i = 0; i < field_0xc10; i++) {
            if (field_0xc14[i])
                delete field_0xc14[i];
        }
        operator delete(field_0xc14, __FILE__, 0x4d0);
    }
    if (field_0xc84)
        delete field_0xc84;
    if (field_0xc88)
        delete field_0xc88;
    g_MemTagStack->Pop(scope);
}

TerrainQualityEntry* g_pTerrainQualityTable;
int g_terrainQualityValue;

// Slot 19 (0x00507920) and slot 22 (0x004dc4c0) are the shared "return 0" stubs.
int Terrain::GameObjectVirtualSlot19(int)
{
    return 0;
}

int Terrain::GameObjectVirtualSlot22(int, int)
{
    return 0;
}

// 0x00507930: clamps at zero, marks field_0xbf4 (a dirty flag, tier 3) when the value changes.
void Terrain::SetField0xbec(int value)
{
    if (value < 0)
        value = 0;
    if (field_0xbec != value) {
        field_0xbec = value;
        field_0xbf4 = 1;
    }
}

void Terrain::SelectQuality(int index)
{
    field_0xbf0 = index;
    g_terrainQualityValue = g_pTerrainQualityTable[index].field_0x0c;
    SetField0xbec(g_pTerrainQualityTable[field_0xbf0].field_0x00);
    // if/else, not a ?: expression: VC6 merges the two stores but allocates the joined value
    // to edx only in this form (retail 0x5079a3).
    if (field_0xcb0)
        field_0xca4 = g_pTerrainQualityTable[9].field_0x04;
    else
        field_0xca4 = g_pTerrainQualityTable[field_0xbf0].field_0x04;
    field_0xca8 = g_pTerrainQualityTable[field_0xbf0].field_0x08;
}

TerrainSharedState g_terrainSharedState;
int g_terrainToggle314;
int g_terrainToggle718;

// Slot 23 (0x00508850): debug key handler.  The three TestInputEvent kinds 0x43, 2 and 3
// each flip one toggle and return 1 (tier 3 semantics).
int Terrain::GameObjectVirtualSlot23(int event, int)
{
    if (TestInputEvent(0x43, 0, event, 0x80)) {
        g_terrainToggle314 = 1 - g_terrainToggle314;
        if (g_terrainToggle314)
            g_terrainSharedState = *((TerrainHost*)field_0x18)->field_0x08;
        return 1;
    }
    if (TestInputEvent(2, 0, event, 0x80)) {
        if (g_terrainQualityValue && field_0xcac) {
            field_0xcb0 = 1 - field_0xcb0;
            if (field_0xcb0) {
                field_0xca4 = g_pTerrainQualityTable[9].field_0x04;
                return 1;
            }
            field_0xca4 = g_pTerrainQualityTable[field_0xbf0].field_0x04;
        }
        g_terrainQualityValue = 1 - g_terrainQualityValue;
        return 1;
    }
    if (TestInputEvent(3, 0, event, 0x80)) {
        g_terrainToggle718 = 1 - g_terrainToggle718;
        return 1;
    }
    return 0;
}

// 0x00507510 (Y) and 0x00507590 (Z): advance `origin` along `dir` to the plane coordinate
// `limit` when the ray is heading toward it from the outside, then return the result by value
// (tier 3 names: t = (limit - origin.c) / dir.c and the other two components step by t*dir).
// Both are `inline`: retail emits these copies right after CastSegment because some of its call
// sites were not inlined.  The early `return origin` matters: with a single return after the if, VC6 sinks the
// `push esi` used by the struct copy into the tail block; with two return statements it
// stays in the prologue as in retail.
inline TerrainVec3 TerrainClipRayToPlaneY(TerrainVec3& origin, const TerrainVec3& dir, float limit)
{
    if (!((dir.y > 0.0f && origin.y < limit) || (dir.y < 0.0f && origin.y > limit)))
        return origin;
    float t = (limit - origin.y) / dir.y;
    origin.x += t * dir.x;
    origin.y = limit;
    origin.z += t * dir.z;
    return origin;
}

inline TerrainVec3 TerrainClipRayToPlaneZ(TerrainVec3& origin, const TerrainVec3& dir, float limit)
{
    if (!((dir.z > 0.0f && origin.z < limit) || (dir.z < 0.0f && origin.z > limit)))
        return origin;
    float t = (limit - origin.z) / dir.z;
    origin.x += t * dir.x;
    origin.y += t * dir.y;
    origin.z = limit;
    return origin;
}

TerrainVec3 g_terrainRefDir;

// Truncates v to an int through *dst.  Retail does this with an inline `fistp` (the caller
// subtracts 0.5 first, so it floors), and the inlined helper materialises both arguments in
// stack temporaries (`lea eax,[ix]; mov [tmp],eax; fstp [tmp2]; fld [tmp2]; fistp [eax]`).
// NOTE: a plain (int) cast compiles to a call to _ftol under the gate profile. Spilling both
// arguments to stack temporaries is the shape of an inlined `__asm { fld f; mov edx,p;
// fistp [edx] }` FloatToInt helper (tier 3); /QIfist alone would emit a bare fistp. Inline
// asm is out of bounds here, so QueryGround stays partial (see targets.json).
static inline void FloatToInt(int* dst, float v)
{
    *dst = (int)v;
}

static inline TerrainVec3 TerrainNormalize(const TerrainVec3& v)
{
    float lenSq = TerrainDot(v, v);
    if (lenSq == 1.0f)
        return v;
    return v * FastInvSqrt(lenSq);
}

// 0x00507c10 (tier 3 names throughout).  x/z are scaled by field_0xc30 (grid units per world
// unit), floored to a cell (ix, iz), and the four corners are fetched.  The point's offsets
// (u, v) inside the cell give a squared distance to each corner (w[0..3]); the smallest picks
// the surface byte.  The cell is split into two triangles along a diagonal that alternates in a
// checkerboard ((ix ^ iz) & 1), the nearer-corner test picks the triangle, and the height is
// linear over it.  The normal is either the triangle's face normal (flatShaded) or the
// inverse-squared-distance weighted blend of the three corner normals.
void Terrain::QueryGround(TerrainVec3* pos, TerrainVec3* normal, int flatShaded,
                          unsigned char* surface)
{
    if (!field_0x44)
        return;

    float fx = pos->x * field_0xc30;
    float fz = pos->z * field_0xc30;
    if (fx < 0.0f)
        fx = 0.0f;
    if (fz < 0.0f)
        fz = 0.0f;

    int ix;
    int iz;
    FloatToInt(&ix, fx - 0.5f);
    FloatToInt(&iz, fz - 0.5f);
    if (ix < 0 || iz < 0)
        return;

    float x0 = (float)ix;
    float x1 = x0 + 1.0f;
    float z0 = (float)iz;
    float z1 = z0 + 1.0f;

    float heights[4];
    TerrainVec3 n[4];
    unsigned char kinds[4];
    TerrainVec3* cornerNormals = 0;
    if (normal)
        cornerNormals = n;
    field_0x44->GetCellCorners(ix, iz, heights, cornerNormals, kinds);

    float y0 = heights[0] * field_0x40;
    float y1 = heights[1] * field_0x40;
    float y2 = heights[2] * field_0x40;
    float y3 = heights[3] * field_0x40;
    TerrainVec3 v[4];
    v[0] = TerrainVec3(x0, y0, z0);
    v[1] = TerrainVec3(x1, y1, z0);
    v[2] = TerrainVec3(x0, y2, z1);
    v[3] = TerrainVec3(x1, y3, z1);

    float u = fx - x0;
    float t = fz - z0;
    float w[4];
    w[0] = u * u + t * t;
    w[1] = (1.0f - u) * (1.0f - u) + t * t;
    w[2] = u * u + (1.0f - t) * (1.0f - t);
    w[3] = (1.0f - u) * (1.0f - u) + (1.0f - t) * (1.0f - t);
    if (0.001f > w[0])
        w[0] = 0.001f;
    if (0.001f > w[1])
        w[1] = 0.001f;
    if (0.001f > w[2])
        w[2] = 0.001f;
    if (0.001f > w[3])
        w[3] = 0.001f;

    if (surface) {
        int nearest;
        float best = 3.4028235e+38f;
        for (int k = 0; k < 4; k++) {
            if (best > w[k]) {
                best = w[k];
                nearest = k;
            }
        }
        *surface = kinds[nearest];
    }

    if ((iz ^ ix) & 1) {
        if (w[0] < w[3]) {
            // triangle (0, 1, 2)
            pos->y = y0 + (y1 - y0) * (fx - ix) + (y2 - y0) * (fz - iz);
            if (!normal)
                return;
            if (flatShaded) {
                TerrainVec3 face;
                TerrainTriangleNormal(&v[0], &v[1], &v[2], &face, 0);
                if (TerrainDot(face, g_terrainRefDir) < 0.0f)
                    *normal = -face;
                else
                    *normal = face;
                return;
            }
            *normal = TerrainNormalize(n[0] / w[0] + n[1] / w[1] + n[2] / w[2]);
        } else {
            // triangle (1, 3, 2)
            pos->y = y2 + (y3 - y2) * (fx - ix) + (y1 - y3) * (1.0f - (fz - iz));
            if (!normal)
                return;
            if (flatShaded) {
                TerrainVec3 face;
                TerrainTriangleNormal(&v[1], &v[3], &v[2], &face, 0);
                if (TerrainDot(face, g_terrainRefDir) < 0.0f)
                    *normal = -face;
                else
                    *normal = face;
                return;
            }
            *normal = TerrainNormalize(n[1] / w[1] + n[3] / w[3] + n[2] / w[2]);
        }
    } else {
        if (w[1] < w[2]) {
            // triangle (0, 1, 3)
            pos->y = y0 + (y1 - y0) * (fx - ix) + (y3 - y1) * (fz - iz);
            if (!normal)
                return;
            if (flatShaded) {
                TerrainVec3 face;
                TerrainTriangleNormal(&v[0], &v[1], &v[3], &face, 0);
                if (TerrainDot(face, g_terrainRefDir) < 0.0f)
                    *normal = -face;
                else
                    *normal = face;
                return;
            }
            *normal = TerrainNormalize(n[0] / w[0] + n[1] / w[1] + n[3] / w[3]);
        } else {
            // triangle (0, 3, 2)
            pos->y = y2 + (y3 - y2) * (fx - ix) + (y0 - y2) * (1.0f - (fz - iz));
            if (!normal)
                return;
            if (flatShaded) {
                TerrainVec3 face;
                TerrainTriangleNormal(&v[0], &v[3], &v[2], &face, 0);
                if (TerrainDot(face, g_terrainRefDir) < 0.0f)
                    *normal = -face;
                else
                    *normal = face;
                return;
            }
            *normal = TerrainNormalize(n[0] / w[0] + n[3] / w[3] + n[2] / w[2]);
        }
    }
}

// X-axis sibling of TerrainClipRayToPlaneY/Z (0x00507510 / 0x00507590): retail has no
// out-of-line copy, every call site in CastSegment is inlined (tier 2: the same disjunction is
// visible inline at 0x00506f6d..0x00506ffb).  Same shape as Y/Z (tier 3).
inline TerrainVec3 TerrainClipRayToPlaneX(TerrainVec3& origin, const TerrainVec3& dir, float limit)
{
    if (!((dir.x > 0.0f && origin.x < limit) || (dir.x < 0.0f && origin.x > limit)))
        return origin;
    float t = (limit - origin.x) / dir.x;
    origin.x = limit;
    origin.y += t * dir.y;
    origin.z += t * dir.z;
    return origin;
}

// 0x00506e90 (tier 3 names).  Callers: SoultreePhysicsBaseObject slot 21 (0x00502632) and
// the TerrainShadow / projected-shadow code.  The world segment is divided by field_0x40 into
// grid space, each axis is clipped so both ends lie inside the box [0, W] x [yLo, yHi] x [0, W]
// (W = 16 << shift; the start point is advanced along the direction, the end point along the
// reversed direction), trivially rejected if both ends are on the outside of a face, then y and
// z are swapped (the grid stores (x, z, height)) and the grid's slot 1 does the actual cast.
//
// Inline budget (VC6 SP3 /O2, measured with probes): VC6 expands inline calls breadth-first --
// every direct call site in source order first (greedy: a callee that does not fit the remaining
// budget is skipped, later smaller ones may still fit), then the calls inside the inlined bodies
// (here the TerrainVec3 constructors), and the budget grows with the caller's own code size.
// Retail inlines ClipX x4, ClipZ for the start point, and every `-dir` but the last; the
// constructor is inlined only in the two scalings, `end - start` and the first `-dir`.  The
// first 0x3b7 bytes match; from the Z end-point clip on, this source still inlines one ClipZ
// too many (PARTIAL: the budget arithmetic, not the statement shapes, is the remaining gap).
int Terrain::CastSegment(const TerrainVec3* from, const TerrainVec3* to, TerrainVec3* out,
                         int a, int b, int c)
{
    TerrainVec3 start = *from / field_0x40;
    TerrainVec3 end = *to / field_0x40;
    TerrainVec3 dir = end - start;

    float size = (float)(16 << field_0x44->field_0x29);
    float yLo = field_0x44->field_0x18;
    float yHi = field_0x44->field_0x1c;

    // x slab
    if (dir.x > 0.0f) {
        TerrainClipRayToPlaneX(start, dir, 0.0f);
    } else if (dir.x < 0.0f) {
        TerrainClipRayToPlaneX(start, dir, size);
    }
    if (dir.x < 0.0f) {
        TerrainClipRayToPlaneX(end, -dir, 0.0f);
    } else if (dir.x > 0.0f) {
        TerrainClipRayToPlaneX(end, -dir, size);
    }

    // z slab
    if (dir.z > 0.0f) {
        TerrainClipRayToPlaneZ(start, dir, 0.0f);
    } else if (dir.z < 0.0f) {
        TerrainClipRayToPlaneZ(start, dir, size);
    }
    if (dir.z < 0.0f) {
        TerrainClipRayToPlaneZ(end, -dir, 0.0f);
    } else if (dir.z > 0.0f) {
        TerrainClipRayToPlaneZ(end, -dir, size);
    }

    // y slab
    if (dir.y > 0.0f) {
        TerrainClipRayToPlaneY(start, dir, yLo);
    } else if (dir.y < 0.0f) {
        TerrainClipRayToPlaneY(start, dir, yHi);
    }
    if (dir.y < 0.0f) {
        TerrainClipRayToPlaneY(end, -dir, yLo);
    } else if (dir.y > 0.0f) {
        TerrainClipRayToPlaneY(end, -dir, yHi);
    }

    if ((start.x <= 0.0f && end.x <= 0.0f) || (start.x >= size && end.x >= size) ||
        (start.z <= 0.0f && end.z <= 0.0f) || (start.z >= size && end.z >= size) ||
        (start.y <= yLo && end.y <= yLo) || (start.y >= yHi && end.y >= yHi))
        return 0;

    float t = start.y;
    start.y = start.z;
    start.z = t;
    t = end.y;
    end.y = end.z;
    end.z = t;
    if (!field_0x44->CastSegment(&start, &end, out, a, b, c))
        return 0;

    t = out->y;
    out->y = out->z;
    out->z = t;
    *out *= field_0x40;
    return 1;
}
