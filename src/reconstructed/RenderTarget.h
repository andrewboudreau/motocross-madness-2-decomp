#pragma once

// RTTI: RenderTarget (root). Its vtable holds the destructor, _purecall in
// slots 1-17, and shared empty bodies in slots 18 and 19; PCRenderTarget
// implements it. This is the object Camera keeps at +0x18. The code sits just
// before ResourceManager.cpp references; RenderTarget.cpp is not attested.

#include "CameraRect.h"
#include "Display.h"

class Camera;

class RenderTarget {
public:
    RenderTarget();                      // 0x004e8c50
    virtual ~RenderTarget();             // 0x004e8cb0 (deleting wrapper 0x004e8c80)

    // Slots 1-19 are named after PCRenderTarget's strict-exact overrides, each of
    // which forwards to the IDirect3DDevice3/IDirectDrawSurface4 method of the same
    // name (slots 18/19 program ALPHAREF/ALPHATESTENABLE/ALPHAFUNC).
    virtual int BeginScene() = 0;
    virtual int EndScene() = 0;
    virtual int Blit(void* a, void* b, void* c, int d) = 0;
    virtual void* LockSurface(void* rect, long* pitch, int flags) = 0; // locks the surface
    virtual int UnlockSurface(void* rect) = 0;
    virtual long GetTextureStageState(int stage, int type, int* value) = 0;
    virtual long SetTextureStageState(int stage, int type, int value) = 0;
    virtual void SetRenderState(int state, int value, int force) = 0;
    virtual long GetRenderState(int state, int* value) = 0;
    virtual void SetTextureBlend(int mode, int flag) = 0;
    virtual long ClearTexture(int stage) = 0;
    virtual int Clear(const CameraRect* rect, int flags) = 0;
    virtual int SupportsTextureFormat(int a) = 0;
    virtual int SetViewport(void* viewport) = 0;
    virtual int DrawIndexedPrimitive(int a, int b, int c, int d, int e, int f, int g) = 0;
    virtual int DrawPrimitive(int a, int b, int c, int d, int e) = 0;
    virtual int DrawIndexedPrimitiveVB(int a, int b, int c, int d, int e, int f, int g) = 0;
    virtual void SetAlphaTest(int value); // 0x004806f0 (shared empty body)
    virtual void DisableAlphaTest();      // 0x0044d710 (shared empty body)

    void SetDisplay(UnknownDisplay* display); // 0x004e8ca0: sets +0x04
    void AdvanceFrame();                        // 0x004e8cc0: advances the frame counters
    void SetCamera(Camera* camera);          // 0x004e8cf0: sets the current camera

    UnknownDisplay* targetDisplay;            // +0x04
    Camera* currentCamera;                    // +0x08 current camera
    int targetWidth;                          // +0x0c width
    int targetHeight;                         // +0x10 height
    int frameModulus;                         // +0x14 frame modulus (Camera slots 5/18 read it)
    int frameIndex;                           // +0x18 frame index, wraps at frameModulus
    int frameCount;                           // +0x1c frame count
    int zDepth;                               // +0x20 Z depth (PCRenderTarget: 16 or 32)
    int surfaceCaps;                          // +0x24 surface memory caps (0x4000 video, 0x800 system)
    int surfacePixelFormat;                   // +0x28 pixel format of the surface
    float zClearValue;                        // +0x2c Z clear value (1.0f)
    int clearColor;                           // +0x30 clear colour
    int clearTarget;                          // +0x34 nonzero: slot 12 also clears the target (D3DCLEAR_TARGET, 0x004c5549)
    int verticesDrawn;                        // +0x38 vertices drawn in formats 0x112/0x1e2 (reset by slot 12)
    int pointsDrawn;                          // +0x3c points drawn
    int linesDrawn;                           // +0x40 lines drawn
    int trianglesDrawn;                       // +0x44 triangles drawn
};
