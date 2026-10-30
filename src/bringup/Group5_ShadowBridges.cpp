// Group5 bring-up definitions (best-guess, link frontier). Not strict matches.
//
// ProjectedShadow.h names the shadow pass's providers with its own tier-3
// views (ShadowCaster, ShadowMatrix*, ShadowTransformPoints*, ShadowFillTriangle).
// Where the retail VA already has a definition under its owner's name, the
// view forwards to it:
//   0x00445030 ShadowCaster::PartCount  -> D3DIMSoultreeObject::GetSurfaceCount
//   0x004433f0 ShadowCaster::GetPart    -> D3DIMSoultreeObject::GetPart
//   0x004fe850 ShadowCaster::GetBounds  -> SoultreeObject::GetSubtreeBounds
//   0x004fdab0 ShadowCaster::ReleaseParts -> SoultreeObject::MarkSubtreeDirty
//   0x004a1410 ShadowMatrixIdentity     -> IdentityMatrix
//   0x004a1860 ShadowMatrixMultiply     -> MatrixMult
//   0x004a1b00 ShadowTransformPointsOrtho -> TransformPoints
// 0x004a1a50 (the perspective transform) and 0x00461e60 (the triangle
// rasteriser) have no definition yet and are written out here.
#include "shadow/ProjectedShadow.h"

// The providers, declared only as far as these calls need (the parameter
// types keep the providers' mangled names).
struct Vec3;
struct Matrix4 {
    float m[4][4];
};
struct UnknownSoultreeVertex;

class D3DIMSoultreeObject {
public:
    int GetSurfaceCount(int lod);  // 0x00445030
    void GetPart(int index, UnknownSoultreeVertex** vertices, int* vertexCount, unsigned short** indices,
                 int* indexCount, int c, int d);  // 0x004433f0
};

class SoultreeObject {
public:
    void GetSubtreeBounds(Vec3* center, Vec3* halfExtents);  // 0x004fe850
    void MarkSubtreeDirty();                                 // 0x004fdab0
};

Matrix4 IdentityMatrix();                    // 0x004a1410
Matrix4 MatrixMult(Matrix4 a, Matrix4 b);    // 0x004a1860
void TransformPoints(const void* destinationPoints, void* sourcePoints, const Matrix4* matrix, int count,
                     int destinationStride, int sourceStride);  // 0x004a1b00

int ShadowCaster::PartCount(int which)
{
    return reinterpret_cast<D3DIMSoultreeObject*>(this)->GetSurfaceCount(which);
}

void ShadowCaster::GetPart(int index, void** vertices, int* vertexCount, unsigned short** indices, int* indexCount,
                           int c, int d)
{
    reinterpret_cast<D3DIMSoultreeObject*>(this)->GetPart(
        index, reinterpret_cast<UnknownSoultreeVertex**>(vertices), vertexCount, indices, indexCount, c, d);
}

void ShadowCaster::GetBounds(ShadowVec3* center, ShadowVec3* halfExtents)
{
    reinterpret_cast<SoultreeObject*>(this)->GetSubtreeBounds(reinterpret_cast<Vec3*>(center),
                                                              reinterpret_cast<Vec3*>(halfExtents));
}

void ShadowCaster::ReleaseParts()
{
    reinterpret_cast<SoultreeObject*>(this)->MarkSubtreeDirty();
}

ShadowMatrix ShadowMatrixIdentity()
{
    Matrix4 identity = IdentityMatrix();
    return *reinterpret_cast<ShadowMatrix*>(&identity);
}

ShadowMatrix ShadowMatrixMultiply(ShadowMatrix a, ShadowMatrix b)
{
    Matrix4 product = MatrixMult(*reinterpret_cast<Matrix4*>(&a), *reinterpret_cast<Matrix4*>(&b));
    return *reinterpret_cast<ShadowMatrix*>(&product);
}

// TransformPoints writes its first pointer and reads its second (disassembly
// of 0x004a1b00), the same roles and stride order as the shadow view.
void ShadowTransformPointsOrtho(ShadowVec3* dst, const void* src, const ShadowMatrix* m, int count, int dstStride,
                                int srcStride)
{
    TransformPoints(dst, const_cast<void*>(src), reinterpret_cast<const Matrix4*>(m), count, dstStride, srcStride);
}

// 0x004a1a50: `count` points transformed by `m` with the perspective divide;
// a zero `srcStride` means `dstStride` (body as in
// samples/render/MatrixUtilNearMisses.cpp's ProjectPoints).
void ShadowTransformPointsProjective(ShadowVec3* dst, const void* src, const ShadowMatrix* m, int count,
                                    int dstStride, int srcStride)
{
    const ShadowVec3* srcPoint = (const ShadowVec3*)src;
    ShadowVec3* dstPoint = dst;
    if (!srcStride)
        srcStride = dstStride;
    while (count > 0) {
        float w = 1.0f / (srcPoint->x * m->m[0][3] + srcPoint->y * m->m[1][3] + srcPoint->z * m->m[2][3] +
                          m->m[3][3]);
        dstPoint->x = (srcPoint->x * m->m[0][0] + srcPoint->y * m->m[1][0] + srcPoint->z * m->m[2][0] +
                       m->m[3][0]) * w;
        dstPoint->y = (srcPoint->x * m->m[0][1] + srcPoint->y * m->m[1][1] + srcPoint->z * m->m[2][1] +
                       m->m[3][1]) * w;
        dstPoint->z = (srcPoint->x * m->m[0][2] + srcPoint->y * m->m[1][2] + srcPoint->z * m->m[2][2] +
                       m->m[3][2]) * w;
        srcPoint = (const ShadowVec3*)((const char*)srcPoint + srcStride);
        dstPoint = (ShadowVec3*)((char*)dstPoint + dstStride);
        count--;
    }
}

// 0x00461d60 (thiscall, ret 0xc): one edge of the triangle, stepped one row
// at a time with an integer error term (0x1c bytes).
struct Group5ShadowEdge {
    int x;              // +0x00
    int xStep;          // +0x04 floor(dx / dy)
    int errorAdjUp;     // +0x08 dx mod dy
    int errorAdjDown;   // +0x0c dy
    int errorTerm;      // +0x10
    int y;              // +0x14 first row
    int count;          // +0x18 rows

    void SetUp(const int* points, int start, int end);
    void Step();
};

// Floor division with a non-negative remainder (`denominator` > 0).
static void FloorDivMod(int numerator, int denominator, int* floor, int* mod)
{
    if (numerator >= 0) {
        *floor = numerator / denominator;
        *mod = numerator % denominator;
    } else {
        *floor = -(-numerator / denominator);
        *mod = -numerator % denominator;
        if (*mod) {
            (*floor)--;
            *mod = denominator - *mod;
        }
    }
}

void Group5ShadowEdge::SetUp(const int* points, int start, int end)
{
    y = points[start * 2 + 1];
    count = points[end * 2 + 1] - y;
    if (!count)
        return;
    int startX = points[start * 2];
    int startY = points[start * 2 + 1];
    int dx = points[end * 2] - startX;
    int dy = points[end * 2 + 1] - startY;
    FloorDivMod((y - startY) * dx + (startX + 1) * dy - 1, dy, &x, &errorTerm);
    FloorDivMod(dx, dy, &xStep, &errorAdjUp);
    errorAdjDown = dy;
}

void Group5ShadowEdge::Step()
{
    x += xStep;
    y++;
    count--;
    errorTerm += errorAdjUp;
    if (errorTerm >= errorAdjDown) {
        x++;
        errorTerm -= errorAdjDown;
    }
}

// Fills `count` 16-bit cells; retail aligns to 4 bytes and stores the doubled
// value as dwords (unrolled sixteen times through a jump table).
static void FillSpan(short* cell, int count, short value)
{
    if (count <= 0)
        return;
    if ((unsigned long)cell & 2) {
        *cell++ = value;
        count--;
    }
    unsigned long pair = ((unsigned long)(unsigned short)value << 16) | (unsigned short)value;
    unsigned long* cells = (unsigned long*)cell;
    for (int i = count >> 1; i > 0; i--)
        *cells++ = pair;
    if (count & 1)
        cell[count - 1] = value;
}

// 0x00461e60: rasterises the triangle `points` (three {x, y}) into the
// (1 << shift)-wide buffer, top-left fill convention; with `plotVertices` the
// three corners are written first. A single-row triangle fills [min x, max x].
// Retail returns 1 (eax), which the declared void signature drops.
void ShadowFillTriangle(int shift, short* buffer, short value, int* points, int plotVertices)
{
    int y0 = points[1];
    int y1 = points[3];
    int y2 = points[5];
    int top, middle, bottom;
    // The middle and bottom vertices' positions in winding order after `top`.
    int middleOrder, bottomOrder;
    if (y0 < y1) {
        if (y2 < y0) {
            top = 2;
            middle = 0;
            bottom = 1;
            middleOrder = 0;
            bottomOrder = 1;
        } else {
            top = 0;
            if (y1 < y2) {
                middle = 1;
                bottom = 2;
            } else {
                middle = 2;
                bottom = 1;
            }
            middleOrder = middle;
            bottomOrder = bottom;
        }
    } else if (y2 < y1) {
        top = 2;
        middle = 1;
        bottom = 0;
        middleOrder = 1;
        bottomOrder = 0;
    } else {
        top = 1;
        if (y0 < y2) {
            bottom = 2;
            middle = 0;
            middleOrder = 3;
            bottomOrder = 2;
        } else {
            middle = 2;
            bottom = 0;
            middleOrder = 2;
            bottomOrder = 3;
        }
    }

    int minX = points[0] < points[2] ? points[0] : points[2];
    int maxX = points[0] > points[2] ? points[0] : points[2];
    if (minX >= points[4])
        minX = points[4];
    if (maxX <= points[4])
        maxX = points[4];

    if (points[bottom * 2 + 1] == points[top * 2 + 1]) {
        // Every vertex on one row.
        FillSpan(buffer + (points[bottom * 2 + 1] << shift) + minX, maxX - minX + 1, value);
        return;
    }

    if (plotVertices) {
        buffer[(points[1] << shift) + points[0]] = value;
        buffer[(points[3] << shift) + points[2]] = value;
        buffer[(points[5] << shift) + points[4]] = value;
    }

    int rowWidth = 1 << shift;
    Group5ShadowEdge longEdge;
    Group5ShadowEdge upperEdge;
    Group5ShadowEdge lowerEdge;
    longEdge.SetUp(points, top, bottom);
    upperEdge.SetUp(points, top, middle);
    lowerEdge.SetUp(points, middle, bottom);

    // Which side the long edge is on follows the winding.
    int longOnRight;
    Group5ShadowEdge* left;
    Group5ShadowEdge* right;
    if (bottomOrder > middleOrder) {
        left = &longEdge;
        right = &upperEdge;
        longOnRight = 0;
    } else {
        left = &upperEdge;
        right = &longEdge;
        longOnRight = 1;
    }

    short* row = buffer + (left->y << shift);
    int rows;
    for (rows = upperEdge.count; rows; rows--) {
        FillSpan(row + left->x, right->x - left->x + 1, value);
        upperEdge.Step();
        longEdge.Step();
        row += rowWidth;
    }

    if (longOnRight) {
        left = &lowerEdge;
        right = &longEdge;
    } else {
        left = &longEdge;
        right = &lowerEdge;
    }
    for (rows = lowerEdge.count; rows; rows--) {
        FillSpan(row + left->x, right->x - left->x + 1, value);
        lowerEdge.Step();
        longEdge.Step();
        row += rowWidth;
    }
}
