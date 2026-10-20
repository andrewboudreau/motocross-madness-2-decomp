// D3DIMSoultreeShadow.cpp -- reconstruction of D:\aardvark\VC\krusty2\D3DIMSoultreeShadow.cpp.
// owner: D3DIMSoultreeShadow.cpp. Slot 30 (0x446f40) pushes this file's __FILE__ 0x568cac, and the
// other methods are contiguous members of the same class (bracket 0x445feb..0x447a67).
#include <string.h>

#include "D3DIMSoultreeShadow.h"

// Per-TU vector constants (tier 2): 0x004477d0..0x004478d0 are the four VC6 dynamic initializers
// that many retail TUs have, as in ProjectedShadow.cpp: jmp thunks $E2/$E5/$E8/$E11 into bodies
// $E1/$E4/$E7/$E10 that build (0,0,0), (1,0,0), (0,1,0), (0,0,1) in a temporary and copy it into
// private globals (0x0057efa8, 0x0057efb8, 0x0057efc8, 0x0057ef98).  Copy-initialisation gives
// the temporary-then-copy shape.
struct ShadowConstVec3 {
    float x, y, z;
    ShadowConstVec3(float x_, float y_, float z_) { x = x_; y = y_; z = z_; }
};
static const ShadowConstVec3 kVec3Zero = ShadowConstVec3(0.0f, 0.0f, 0.0f);
static const ShadowConstVec3 kVec3XAxis = ShadowConstVec3(1.0f, 0.0f, 0.0f);
static const ShadowConstVec3 kVec3YAxis = ShadowConstVec3(0.0f, 1.0f, 0.0f);
static const ShadowConstVec3 kVec3ZAxis = ShadowConstVec3(0.0f, 0.0f, 1.0f);

// 0x005995b8..0x0059ad28: 3000 shorts filled with their index by the ctor (tier 2: the loop
// `mov [eax],cx; add eax,2; inc ecx; cmp eax,0x59ad28`).  PROVISIONAL name.
short g_d3dimShadowIndexTable[3000];

D3DIMSoultreeShadow::D3DIMSoultreeShadow(int flags)
    : ShadowReceiver(flags)
{
    caster = 0;
    shadow = 0;
    inShadowView = 0;
    inCameraView = 0;
    field_0x3c = 0;
    field_0x40 = 0;
    vertexCount = 0;
    for (int i = 0; i < 3000; i++)
        g_d3dimShadowIndexTable[i] = (short)i;
}

// 0x004468b0 (ret 0xc).
D3DIMSoultreeShadow* D3DIMSoultreeShadow::Attach(int host, ShadowCaster* c, ProjectedShadow* s)
{
    if (s) {
        GameObject::SetRenderContext((void*)host);
        if (this) {
            shadow = s;
            s->AddReceiver(this);
            caster = c;
            return this;
        }
    }
    return 0;
}

// Row-major 4x4 matrix with the D3DMATRIX member names and the index view (Math3D.h's
// Matrix4 shape; this TU's ShadowMatrix only has m[4][4]).  Local view, tier 3.
struct D3DIMShadowMatrix4 {
    D3DIMShadowMatrix4() {}
    union {
        struct {
            float _11, _12, _13, _14;
            float _21, _22, _23, _24;
            float _31, _32, _33, _34;
            float _41, _42, _43, _44;
        };
        float m[4][4];
    };
};

// The caster as slot 27 uses it (a D3DIMSoultreeObject; BoundingBoxTreeBuild.h
// TreeModelSource).  Local view, tier 3.
class D3DIMShadowCasterView {
public:
    void GetMatrixIn(void* frame, D3DIMShadowMatrix4* out);           // 0x004fca80
    void GetBounds(ShadowVec3* center, ShadowVec3* halfExtents);      // 0x004fe850
    char pad_0x00[0x14c];
    int field_0x14c;                // +0x14c nonzero skips the visibility test
};
class D3DIMShadowCameraView {
public:
    char pad_0x00[0xec];
    D3DIMShadowMatrix4 matrix_0xec; // +0xec
};
// The object at 0x00575a98 (src/krusty2/visibility/VisibilityQuadTree.h VisibilityClipper).
class D3DIMShadowClipper {
public:
    int TestBox(D3DIMShadowCameraView* camera, const float* matrix, const float* center,
                const float* extent, int* screenRect, int* cornersInside, int unused);  // 0x0052f570
};
extern D3DIMShadowClipper* g_visibilityClipper;   // 0x00575a98

static inline void D3DIMShadowMatrixProduct(D3DIMShadowMatrix4* out, const D3DIMShadowMatrix4& a,
                                            const D3DIMShadowMatrix4& b)
{
    // Reading b through a second name in _24 only makes VC6 order the row-2 sums as retail
    // does in slot 27 (docs/VC6_OPERAND_ORDER.md section 1.2: the alias gives b's row-2
    // column leaves a different age).
    const D3DIMShadowMatrix4& right = b;
    out->_11 = a._11 * b._11 + a._12 * b._21 + a._13 * b._31 + a._14 * b._41;
    out->_12 = a._11 * b._12 + a._12 * b._22 + a._13 * b._32 + a._14 * b._42;
    out->_13 = a._11 * b._13 + a._12 * b._23 + a._13 * b._33 + a._14 * b._43;
    out->_14 = a._11 * b._14 + a._12 * b._24 + a._13 * b._34 + a._14 * b._44;
    out->_21 = a._21 * b._11 + a._22 * b._21 + a._23 * b._31 + a._24 * b._41;
    out->_22 = a._21 * b._12 + a._22 * b._22 + a._23 * b._32 + a._24 * b._42;
    out->_23 = a._21 * b._13 + a._22 * b._23 + a._23 * b._33 + a._24 * b._43;
    out->_24 = a._21 * right._14 + a._22 * right._24 + a._23 * right._34 + a._24 * right._44;
    out->_31 = a._31 * b._11 + a._32 * b._21 + a._33 * b._31 + a._34 * b._41;
    out->_32 = a._31 * b._12 + a._32 * b._22 + a._33 * b._32 + a._34 * b._42;
    out->_33 = a._31 * b._13 + a._32 * b._23 + a._33 * b._33 + a._34 * b._43;
    out->_34 = a._31 * b._14 + a._32 * b._24 + a._33 * b._34 + a._34 * b._44;
    out->_41 = a._41 * b._11 + a._42 * b._21 + a._43 * b._31 + a._44 * b._41;
    out->_42 = a._41 * b._12 + a._42 * b._22 + a._43 * b._32 + a._44 * b._42;
    out->_43 = a._41 * b._13 + a._42 * b._23 + a._43 * b._33 + a._44 * b._43;
    out->_44 = a._41 * b._14 + a._42 * b._24 + a._43 * b._34 + a._44 * b._44;
}

// 0x004468f0 (slot 27): clears inCameraView and inShadowView, then sets inShadowView when the caster's
// bounds pass the shadow camera's box test; returns inShadowView.  Slot 29
// (0x00446c30, the same test with the render target's camera) is a near miss in
// samples/physics/shadow/D3DIMSoultreeShadowNearMisses.cpp.
int D3DIMSoultreeShadow::UnknownVirtualSlot27()
{
    D3DIMShadowCasterView* c = (D3DIMShadowCasterView*)caster;
    inCameraView = 0;
    inShadowView = 0;
    if (!c->field_0x14c) {
        D3DIMShadowMatrix4 local;
        ShadowVec3 center, extent;
        D3DIMShadowMatrix4 m;
        c->GetMatrixIn(0, &local);
        ((D3DIMShadowCasterView*)caster)->GetBounds(&center, &extent);
        const D3DIMShadowMatrix4& view = ((D3DIMShadowCameraView*)shadow->camera)->matrix_0xec;
        D3DIMShadowMatrixProduct(&m, local, view);
        if (g_visibilityClipper->TestBox((D3DIMShadowCameraView*)shadow->camera, &m._11, &center.x,
                                         &extent.x, 0, 0, 0))
            inShadowView = 1;
    }
    return inShadowView;
}

// 0x00446bf0: texel size of the shadow texture (0.5 / size when the +0x98 flag is set, else
// 1.0 / size), kept in a file-scope float; returns 0.
float g_d3dimShadowTexel;  // 0x0057efa4
int D3DIMSoultreeShadow::UnknownVirtualSlot28()
{
    ProjectedShadow* s = shadow;
    float k;
    if (s->field_0x98)
        k = 0.5f;
    else
        k = 1.0f;
    g_d3dimShadowTexel = k / s->texture->size;
    return 0;
}

// 0x00581eb8..0x005995b8: 3000 32-byte lit vertices (FVF 0x1e2, the D3DLVERTEX layout) that
// slot 14 draws with g_d3dimShadowIndexTable (tier 2: the slot 14 draw call and the extent up
// to the index table).  PROVISIONAL name.  The empty constructor (d3dtypes.h gives
// D3DLVERTEX one under D3D_OVERLOADS) makes the array dynamically initialized: .CRT$XCU
// entry 94, after this file's vector set, is the thunk 0x00446820 to the bare `ret` body
// 0x00446830 (strong inference: the entry follows the set with no gap and the array is the
// only class-typed static left in the file).
struct D3DIMShadowVertex {
    float x, y, z;
    int reserved;
    unsigned int diffuse;
    unsigned int specular;
    float tu, tv;
    D3DIMShadowVertex() {}
};
D3DIMShadowVertex g_d3dimShadowVertices[3000];

// The render target at GameObject::renderContext as slot 14 uses it (RTTI PCRenderTarget; the
// src/reconstructed RenderTarget.h slot names).  Local view, tier 3.
class D3DIMShadowCamera {
public:
    virtual void Slot0(); virtual void Slot1(); virtual void Slot2(); virtual void Slot3();
    virtual void Slot4(); virtual void Slot5(); virtual void Slot6(); virtual void Slot7();
    virtual void Slot8(); virtual void Slot9(); virtual void Slot10(); virtual void Slot11();
    virtual void Slot12(); virtual void Slot13(); virtual void Slot14(); virtual void Slot15();
    virtual void Slot16(); virtual void Slot17(); virtual void Slot18(); virtual void Slot19();
    virtual void Slot20(); virtual void Slot21(); virtual void Slot22(); virtual void Slot23();
    virtual void Slot24(); virtual void Slot25(); virtual void Slot26(); virtual void Slot27();
    virtual void Slot28(); virtual void Slot29();
    virtual void SetWorldMatrix(const ShadowMatrix* m);           // slot 30 (0x78)
};
// The shadow texture's slot 19 (0x4c): binds it to texture stage 0 (PCTextureMap slot 19 in
// src/reconstructed/PCTextureMap.h).  Local view, tier 3.
class D3DIMShadowTextureView {
public:
    virtual void Slot0(); virtual void Slot1(); virtual void Slot2(); virtual void Slot3();
    virtual void Slot4(); virtual void Slot5(); virtual void Slot6(); virtual void Slot7();
    virtual void Slot8(); virtual void Slot9(); virtual void Slot10(); virtual void Slot11();
    virtual void Slot12(); virtual void Slot13(); virtual void Slot14(); virtual void Slot15();
    virtual void Slot16(); virtual void Slot17(); virtual void Slot18();
    virtual void Bind();                                          // slot 19
};

class D3DIMShadowRenderTarget {
public:
    virtual void Slot0(); virtual void Slot1(); virtual void Slot2(); virtual void Slot3();
    virtual void Slot4(); virtual void Slot5();
    virtual long GetTextureStageState(int stage, int type, int* value);  // slot 6
    virtual long SetTextureStageState(int stage, int type, int value);   // slot 7
    virtual void SetRenderState(int state, int value, int force);        // slot 8
    virtual long GetRenderState(int state, int* value);                  // slot 9
    virtual void Slot10(); virtual void Slot11(); virtual void Slot12(); virtual void Slot13();
    virtual void Slot14();
    // slot 15: indexed draw (type, FVF, vertices, vertex count, indices, index count, flags).
    virtual int DrawIndexed(int type, int fvf, void* vertices, int vertexCount, short* indices,
                            int indexCount, int flags);
    virtual void Slot16(); virtual void Slot17();
    virtual void Slot18(int value);
    void* display;                               // +0x04
    D3DIMShadowCamera* camera;                   // +0x08
    char pad_0x0c[0x1c8 - 0x0c];
    unsigned char field_0x1c8;                   // bit 2 tested by slot 14
};

// Slot 14 (0x00447540): draws the receiver's shadow triangles (vertexCount vertices and indices)
// with the shadow texture, an identity world matrix and modulating texture stages; the address
// mode and render state 4 are restored afterwards.  Format 0x613 textures also switch render
// state 0x21 on around the draw.
int D3DIMSoultreeShadow::UnknownVirtualSlot14()
{
    if (inCameraView && inShadowView && vertexCount) {
        ShadowMatrix world;
        memset(&world, 0, sizeof(world));
        world.m[3][3] = 1.0f;
        world.m[2][2] = 1.0f;
        world.m[1][1] = 1.0f;
        world.m[0][0] = 1.0f;
        ((D3DIMShadowRenderTarget*)renderContext)->camera->SetWorldMatrix(&world);
        ((D3DIMShadowTextureView*)shadow->texture)->Bind();
        int address;
        ((D3DIMShadowRenderTarget*)renderContext)->GetTextureStageState(0, 0xc, &address);
        if (address != 3)
            ((D3DIMShadowRenderTarget*)renderContext)->SetTextureStageState(0, 0xc, 3);
        if (shadow->surfaceFormat == 0x613) {
            ((D3DIMShadowRenderTarget*)renderContext)->SetRenderState(0x21, 1, 0);
            ((D3DIMShadowRenderTarget*)renderContext)->SetRenderState(0x1b, 1, 0);
            if (((D3DIMShadowRenderTarget*)renderContext)->field_0x1c8 & 4) {
                ((D3DIMShadowRenderTarget*)renderContext)->SetTextureStageState(0, 1, 4);
                ((D3DIMShadowRenderTarget*)renderContext)->SetTextureStageState(0, 2, 2);
                ((D3DIMShadowRenderTarget*)renderContext)->SetTextureStageState(0, 3, 0);
                ((D3DIMShadowRenderTarget*)renderContext)->SetTextureStageState(0, 4, 4);
                ((D3DIMShadowRenderTarget*)renderContext)->SetTextureStageState(0, 5, 2);
                ((D3DIMShadowRenderTarget*)renderContext)->SetTextureStageState(0, 6, 0);
            }
        } else {
            ((D3DIMShadowRenderTarget*)renderContext)->SetRenderState(0x1b, 1, 0);
            ((D3DIMShadowRenderTarget*)renderContext)->SetTextureStageState(0, 1, 4);
            ((D3DIMShadowRenderTarget*)renderContext)->SetTextureStageState(0, 2, 2);
            ((D3DIMShadowRenderTarget*)renderContext)->SetTextureStageState(0, 3, 0);
            ((D3DIMShadowRenderTarget*)renderContext)->SetTextureStageState(0, 4, 4);
            ((D3DIMShadowRenderTarget*)renderContext)->SetTextureStageState(0, 5, 2);
            ((D3DIMShadowRenderTarget*)renderContext)->SetTextureStageState(0, 6, 0);
        }
        int blend;
        ((D3DIMShadowRenderTarget*)renderContext)->GetRenderState(4, &blend);
        ((D3DIMShadowRenderTarget*)renderContext)->SetRenderState(4, 1, 0);
        ((D3DIMShadowRenderTarget*)renderContext)->SetRenderState(0xe, 0, 0);
        ((D3DIMShadowRenderTarget*)renderContext)->Slot18(0);
        ((D3DIMShadowRenderTarget*)renderContext)->DrawIndexed(4, 0x1e2, g_d3dimShadowVertices, vertexCount,
                                                            g_d3dimShadowIndexTable, vertexCount, 0);
        ((D3DIMShadowRenderTarget*)renderContext)->SetRenderState(0xe, 1, 0);
        ((D3DIMShadowRenderTarget*)renderContext)->SetRenderState(0x1b, 0, 0);
        if (address != 3)
            ((D3DIMShadowRenderTarget*)renderContext)->SetTextureStageState(0, 0xc, address);
        ((D3DIMShadowRenderTarget*)renderContext)->SetRenderState(4, blend, 0);
        if (shadow->surfaceFormat == 0x613)
            ((D3DIMShadowRenderTarget*)renderContext)->SetRenderState(0x21, 0, 0);
    }
    return 1;
}
