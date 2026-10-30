// Group5 bring-up definitions (best-guess, link frontier). Not strict matches.
//
// UnknownSoultreeNodeView is the drawing function's (Group5_D3DIMSoulTreeDraw.cpp,
// from samples/render/D3DIMSoulTreeNearMisses.cpp) view of soultree.cpp's node
// calls. The retail functions are already defined as SoultreeObject members,
// with Vec3 instead of Vector3 (same 12-byte x, y, z layout), so the view
// forwards to them:
//   0x004fca80 GetMatrixIn, 0x004fc9a0 GetPositionIn,
//   0x004fd710 WorldToLocalDirection, 0x004fd7f0 WorldToLocalPoint,
//   0x004fd5c0 LocalToWorldDirection.
#include "math/Vec3.h"

struct Matrix4;
struct Vector3 {
    float x, y, z;
};

// The providers, declared only as far as these calls need.
class SoultreeObject {
public:
    void GetMatrixIn(SoultreeObject* frame, Matrix4* out);   // 0x004fca80
    void GetPositionIn(SoultreeObject* frame, Vec3* out);    // 0x004fc9a0
    Vec3 WorldToLocalDirection(const Vec3& v);               // 0x004fd710
    Vec3 WorldToLocalPoint(const Vec3& p);                   // 0x004fd7f0
    Vec3 LocalToWorldDirection(const Vec3& v);               // 0x004fd5c0
};

class UnknownSoultreeNodeView {
public:
    void GetMatrixIn(SoultreeObject* frame, Matrix4* out);
    void GetPositionIn(SoultreeObject* frame, Vector3* out);
    Vector3 WorldToLocalDirection(const Vector3* v);
    Vector3 WorldToLocalPoint(const Vector3* v);
    Vector3 LocalToWorldDirection(const Vector3* v);
};

static inline SoultreeObject* Node(UnknownSoultreeNodeView* view)
{
    return reinterpret_cast<SoultreeObject*>(view);
}

static inline Vector3 ToVector3(const Vec3& v)
{
    Vector3 result;
    result.x = v.x;
    result.y = v.y;
    result.z = v.z;
    return result;
}

void UnknownSoultreeNodeView::GetMatrixIn(SoultreeObject* frame, Matrix4* out)
{
    Node(this)->GetMatrixIn(frame, out);
}

void UnknownSoultreeNodeView::GetPositionIn(SoultreeObject* frame, Vector3* out)
{
    Node(this)->GetPositionIn(frame, reinterpret_cast<Vec3*>(out));
}

Vector3 UnknownSoultreeNodeView::WorldToLocalDirection(const Vector3* v)
{
    return ToVector3(Node(this)->WorldToLocalDirection(*reinterpret_cast<const Vec3*>(v)));
}

Vector3 UnknownSoultreeNodeView::WorldToLocalPoint(const Vector3* v)
{
    return ToVector3(Node(this)->WorldToLocalPoint(*reinterpret_cast<const Vec3*>(v)));
}

Vector3 UnknownSoultreeNodeView::LocalToWorldDirection(const Vector3* v)
{
    return ToVector3(Node(this)->LocalToWorldDirection(*reinterpret_cast<const Vec3*>(v)));
}
