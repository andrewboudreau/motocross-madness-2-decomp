// Group1_ShadowCaster.cpp -- bring-up body for ProjectedShadow.h's ShadowCaster view.
//
// Retail 0x00444fe0 is D3DIMSoultreeObject::GetVertexCount (src/reconstructed/
// D3DIMSoulTree.cpp): the casters ProjectedShadow holds are those soultree objects.
// ShadowCaster's flat view stops before the LOD members, so the same walk is written
// against local views of them: +0x274 LOD count, +0x28c table of 8-byte LODs
// {surface count, surfaces}, 0x38-byte surfaces with the vertex count at +8.

#include "shadow/ProjectedShadow.h"

struct Group1ShadowCasterSurface {
    char field_0x00[8];
    int vertexCount;            // +0x08
    char field_0x0c[0x38 - 0x0c];
};

struct Group1ShadowCasterLod {
    int surfaceCount;
    Group1ShadowCasterSurface* surfaces;
};

// 0x00444fe0: the largest total vertex count of any level of detail (0 without a table).
int ShadowCaster::GetVertexCount()
{
    int lodCount = *(int*)((char*)this + 0x274);
    Group1ShadowCasterLod* lods = *(Group1ShadowCasterLod**)((char*)this + 0x28c);
    int most = 0;
    if (lods) {
        for (int i = 0; i < lodCount; i++) {
            int total = 0;
            for (int j = 0; j < lods[i].surfaceCount; j++)
                total += lods[i].surfaces[j].vertexCount;
            if (total > most)
                most = total;
        }
    }
    return most;
}
