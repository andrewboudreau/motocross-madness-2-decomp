#include "RenderTarget.h"

#include "Camera.h"
#include "TrackGame.h"

// 0x004e8c50
RenderTarget::RenderTarget() {
    targetDisplay = 0;
    currentCamera = 0;
    frameModulus = 0;
    frameIndex = 0;
    frameCount = 0;
    surfacePixelFormat = 0;
    clearTarget = 0;
    verticesDrawn = 0;
    pointsDrawn = 0;
    linesDrawn = 0;
    trianglesDrawn = 0;
}

// 0x004e8ca0
void RenderTarget::SetDisplay(UnknownDisplay* display) {
    targetDisplay = display;
}

// 0x004e8cb0
RenderTarget::~RenderTarget() {}

// 0x004e8cc0: unless the global's +0x0c object says otherwise, the frame index
// advances modulo frameModulus; the frame count always does.
void RenderTarget::AdvanceFrame() {
    if (!g_TrackGame->display->freezeFrameIndex) {
        if (++frameIndex == frameModulus)
            frameIndex = 0;
    }
    frameCount++;
}

// 0x004e8cf0: a camera whose cached size differs from this target's is
// refreshed through Camera 0x0042e550.
void RenderTarget::SetCamera(Camera* camera) {
    currentCamera = camera;
    if (camera && (targetWidth != camera->targetWidth || targetHeight != camera->targetHeight))
        camera->UpdateViewport();
}

// 0x004806f0 and 0x0044d710: empty bodies shared with other classes.
void RenderTarget::SetAlphaTest(int) {}

void RenderTarget::DisableAlphaTest() {}
