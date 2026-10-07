#include "SkyCube.h"

#include "Cube.h"

// SkyCube, 0x004fb1e0..0x004fb2ab (see SkyCube.h). The destructor is
// compiler-generated.

// The render target in field_0x18 and its camera, as slot 10 reads them
// (RenderTarget +0x08 is the current Camera; Camera +0x170 is its position,
// which Camera.h keeps protected).
struct UnknownSkyCubeCamera {
    unsigned char field_0x000[0x170];
    Vector3 field_0x170;
};

struct UnknownSkyCubeTarget {
    unsigned char field_0x00[8];
    UnknownSkyCubeCamera* field_0x08;
};

#define CAMERA() (((UnknownSkyCubeTarget*)field_0x18)->field_0x08)

// 0x004fb1e0
SkyCube::SkyCube(int flags) : DrawableCube(flags) {
}

// 0x004fb230
SkyCube* SkyCube::UnknownFunction4fb230(void* value, UnknownTextureStream* stream, UnknownCubeTextureContext* context,
                                        float height) {
    if (DrawableCube::Load(value, stream, context)) {
        centerHeight = height;
        return this;
    }
    return 0;
}

// 0x004fb260
int SkyCube::UnknownVirtualSlot10(float frameTime) {
    Vector3 center;
    center.x = CAMERA()->field_0x170.x;
    center.y = CAMERA()->field_0x170.y + centerHeight;
    center.z = CAMERA()->field_0x170.z;
    field_0x2c->UnknownFunction43d400(&center, 0, 0);
    return 1;
}
