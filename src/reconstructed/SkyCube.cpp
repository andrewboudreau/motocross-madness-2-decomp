#include "SkyCube.h"

#include "Camera.h"
#include "Cube.h"
#include "RenderTarget.h"

// SkyCube, 0x004fb1e0..0x004fb2ab (see SkyCube.h). The destructor is
// compiler-generated.

// Slot 10 centres the cube on the current camera (RenderTarget +0x08).
#define CAMERA() (((RenderTarget*)renderContext)->currentCamera)

// 0x004fb1e0
SkyCube::SkyCube(int flags) : DrawableCube(flags) {
}

// 0x004fb230
SkyCube* SkyCube::Create(void* value, FileStream* stream, TextureContext* context,
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
