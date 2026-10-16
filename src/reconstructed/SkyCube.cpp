#include "SkyCube.h"

#include "Cube.h"

// SkyCube, 0x004fb1e0..0x004fb2ab (see SkyCube.h). The destructor is
// compiler-generated.

// The render target in renderContext and its camera, as slot 10 reads them
// (RenderTarget +0x08 is the current Camera; Camera +0x170 is its position,
// which Camera.h keeps protected).
struct UnknownSkyCubeCamera {
    unsigned char field_0x000[0x170];
    Vector3 eyePosition;                       // +0x170
};

struct UnknownSkyCubeTarget {
    unsigned char field_0x00[8];
    UnknownSkyCubeCamera* currentCamera;       // +0x08
};

#define CAMERA() (((UnknownSkyCubeTarget*)renderContext)->currentCamera)

// 0x004fb1e0
SkyCube::SkyCube(int flags) : DrawableCube(flags) {
}

// 0x004fb230
SkyCube* SkyCube::Create(void* value, UnknownTextureStream* stream, UnknownCubeTextureContext* context,
                                        float height) {
    if (DrawableCube::Load(value, stream, context)) {
        centerHeight = height;
        return this;
    }
    return 0;
}

// 0x004fb260
int SkyCube::Tick(float frameTime) {
    Vector3 center;
    center.x = CAMERA()->eyePosition.x;
    center.y = CAMERA()->eyePosition.y + centerHeight;
    center.z = CAMERA()->eyePosition.z;
    cubeObject->SetVectors(&center, 0, 0);
    return 1;
}
