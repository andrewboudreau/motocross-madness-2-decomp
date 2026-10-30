// Group2_ShadowCameraLookAt.cpp -- link bring-up: ShadowCameraProxy::SetLookAt (0x0042e9b0).
//
// Decoded behaviour (ret 0x14, five argument dwords):
//   * eye != 0: copy *eye to +0x170 (eyePosition);
//   * a   != 0: a points at a vector, stored normalised at +0x17c (copied as is when its
//                squared length is exactly 1, else scaled by FastInvSqrt(lengthSq));
//   * b   != 0: the same for +0x188;
//   * c   != 0: *c (one dword) is copied to +0x194;
//   * fov != 0: SetLensScale(*fov) (0x0042e930, inlined below as a file-local helper so this
//                file exports nothing beyond SetLookAt).
// The header declares a/b/c as int (ProjectedShadow passes zeros); their pointer roles
// come from the dereferences above (tier 2). Semantic names are best guesses.
#include "../krusty2/shadow/ProjectedShadow.h"

namespace {

struct LensRenderContextView;

struct LookAtView {                  // the ShadowCameraProxy members touched here
    char pad_0x00[0x18];
    LensRenderContextView* renderContext; // +0x18 GameObject::renderContext
    char pad_0x1c[0x16c - 0x1c];
    float lensScale;                 // +0x16c 0x0042e930 stores a nonzero scale
    ShadowVec3 eyePosition;          // +0x170
    ShadowVec3 forward;              // +0x17c best guess: look direction (normalised)
    ShadowVec3 up;                   // +0x188 best guess: up direction (normalised)
    int field_0x194;                 // +0x194 *c
    char pad_0x198[0x1d0 - 0x198];
    int field_0x1d0;                 // +0x1d0 0x0042e930: renderContext->+0x14 plus one
};

// The render context's +0x14 counter read by 0x0042e930 (provisional view).
struct LensRenderContextView {
    char pad_0x00[0x14];
    int field_0x14;
};

void StoreNormalised(ShadowVec3* out, const ShadowVec3* v)
{
    float lengthSq = v->x * v->x + v->y * v->y + v->z * v->z;
    if (lengthSq == 1.0f) {
        *out = *v;
    } else {
        float inv = FastInvSqrt(lengthSq);
        ShadowVec3 r;
        r.x = inv * v->x;
        r.y = inv * v->y;
        r.z = inv * v->z;
        *out = r;
    }
}

// 0x0042e930 (ShadowCameraProxy::SetLensScale, thiscall ret 4).
void SetLensScaleOf(ShadowCameraProxy* camera, float scale)
{
    LookAtView* view = (LookAtView*)camera;
    if (scale != 0.0)
        view->lensScale = scale;
    view->field_0x1d0 = view->renderContext->field_0x14 + 1;
}

} // namespace

void ShadowCameraProxy::SetLookAt(const ShadowVec3* eye, int a, int b, int c, float* fov)
{
    LookAtView* view = (LookAtView*)this;
    if (eye)
        view->eyePosition = *eye;
    if (a)
        StoreNormalised(&view->forward, (const ShadowVec3*)a);
    if (b)
        StoreNormalised(&view->up, (const ShadowVec3*)b);
    if (c)
        view->field_0x194 = *(const int*)c;
    if (fov)
        SetLensScaleOf(this, *fov);
}
