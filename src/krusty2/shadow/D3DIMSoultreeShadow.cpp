// D3DIMSoultreeShadow.cpp -- reconstruction of D:\aardvark\VC\krusty2\D3DIMSoultreeShadow.cpp.
// owner: D3DIMSoultreeShadow.cpp. Slot 30 (0x446f40) pushes this file's __FILE__ 0x568cac, and the
// other methods are contiguous members of the same class (bracket 0x445feb..0x447a67).
#include "D3DIMSoultreeShadow.h"

// Per-TU vector constants (tier 2): 0x004477d0..0x004478d0 are the four VC6 dynamic initializers
// that open about 73 retail TUs, as in Quadtree.cpp: jmp thunks $E2/$E5/$E8/$E11 into bodies
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
    field_0x34 = 0;
    field_0x38 = 0;
    field_0x3c = 0;
    field_0x40 = 0;
    field_0x44 = 0;
    for (int i = 0; i < 3000; i++)
        g_d3dimShadowIndexTable[i] = (short)i;
}

// 0x004468b0 (ret 0xc).
D3DIMSoultreeShadow* D3DIMSoultreeShadow::Attach(int host, ShadowCaster* c, ProjectedShadow* s)
{
    if (s) {
        GameObject::GameObjectVirtualSlot8(host);
        if (this) {
            shadow = s;
            s->AddReceiver(this);
            caster = c;
            return this;
        }
    }
    return 0;
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
