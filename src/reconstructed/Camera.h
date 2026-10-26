#pragma once

#include "ControlInterface.h"
#include "GameObject.h"
#include "MatrixUtil.h"
#include "RenderTarget.h"

// RTTI: Camera : GameObject : BaseObject. Camera introduces primary slots 27-32
// and overrides (among others) slots 5, 13 and 18. Names are provisional; the
// member offsets are confirmed by the strict-exact constructor (0x0042e340)
// and methods, and the class ends at +0x220 where FollowCamera's fields begin.


// .rdata floats just before Camera's vtable: the constructor's near and far
// planes, which ClampLimits also keeps as the closest near and farthest far
// plane. Storage is supplied by GraphicsState.cpp.
extern const float g_DefaultNearPlane; // 0x00550f6c 1.0f
extern const float g_DefaultFarPlane;  // 0x00550f70 100000.0f

struct _iobuf; // FILE

class Camera : public GameObject {
public:
    explicit Camera(int flags); // 0x0042e340
    virtual ~Camera();

    virtual void Disable();
    virtual void Enable();
    virtual GameObject* SetRenderContext(void* context);
    // 0x0042e690: per-frame update from the +0x170/+0x17c vectors (FollowCamera
    // slots 10 and 47 call it directly).
    virtual int Tick(float frameTime);
    virtual int RenderPre3D();
    virtual int UnknownVirtualSlot14();
    virtual int OnActivate();
    virtual int OnControlPress(UnknownControlEvent* event, UnknownInputEntry* entry);

    // Pure: Camera's vtable holds LIBCMT _purecall (0x00534cfe) in slot 27.
    virtual void TakeScreenshot() = 0;
    virtual void UpdateMatrices();
    virtual int LookAt(Vector3 value);
    virtual int SetWorldMatrix(const Matrix4* value);
    virtual int SetViewMatrix(const Matrix4* value);
    virtual int SetProjectionMatrix(const Matrix4* value);

    // 0x0042e8e0, called by slot 13: re-applies the default limits through
    // 0x0042e960 and slot 28 when +0x1bc/+0x1c0 differ from them.
    int ReapplyDefaultLimits();
    // 0x0042e960: +0x1bc = max(minimum, g_DefaultNearPlane),
    // +0x1c0 = min(maximum, g_DefaultFarPlane).
    void ClampLimits(float minimum, float maximum);
    int UpdateViewport(); // 0x0042e550, called by slot 8
    // 0x0042e930: stores a nonzero value in +0x16c and refreshes +0x1d0
    // (FollowCamera slot 47 passes 66.0f).
    void SetFieldOfView(float value);
    // 0x0042e9b0 (EventManager slot 10): sets each non-null part of the
    // camera frame; the two directions are stored normalized.
    void SetFrame(const Vector3* position, const Vector3* forward, const Vector3* up,
                               const float* roll, const float* fov);
    // 0x0042f0e0: sets (absolute) or offsets the viewport x/width, derives the
    // height from +0x1b8, centres it vertically in +0x1c8 and resubmits it.
    int SetViewportWidth(int x, int width, int absolute);
    // 0x0042f190: stores the viewport, clamped to +0x1c4/+0x1c8, and resubmits it.
    int SetViewport(int x, int y, int width, int height);
    // 0x0042f210: the viewport as left/top/right/bottom.
    void GetViewportRect(CameraRect* rect);

protected:
    // renderContext (GameObject +0x18) holds the camera's RenderTarget.
    RenderTarget* Owner() const { return static_cast<RenderTarget*>(renderContext); }

    friend class RenderTarget;
    friend class PCRenderTarget;    // PCRenderTarget.cpp slot 12 clears the viewport (+0x1a0)
    friend class PCGame;           // slot 19 reattaches the camera (+0x18, +0x1a0)
    friend class InstrumentOverlay; // TrackOverlay.cpp: slot 14 reads +0x1a0 and +0x1cc
    friend class NameOverlay;       // TrackOverlay.cpp: reads +0x170
    friend class KrustyUI;          // krustyui.cpp 0x00498cf0 clears +0x1d4/+0x1d8
    friend class Fog;               // Fog.cpp: projection matrix (+0x6c) and viewport (+0x1a0)
    friend class FogOff;            // Fog.cpp: projection matrix (+0x6c)

    // Provisional Direct3D semantics: PCCamera submits these with transform
    // kinds 1 (world), 3 (projection), and 2 (view), respectively.
    Matrix4 worldMatrix;       // +0x2c, set by slot 30
    Matrix4 projectionMatrix;  // +0x6c, set by slot 32
    Matrix4 viewMatrix;        // +0xac, set by slot 31
    // Rebuilt by slot 28: [1] (+0x12c) is the projection times the viewport
    // scale (0.5, -0.5, offset 0.5), [0] (+0xec) that times the view matrix.
    Matrix4 screenMatrix[2];                  // +0xec
    float fieldOfView;                        // +0x16c degrees, 77.0f (keys 0xc7/0xcf step it)
    Vector3 eyePosition;                      // +0x170 (0, 0, 0); copied to field_0x208
    Vector3 viewDirection;                    // +0x17c (0, 0, 1); copied to field_0x214
    Vector3 upDirection;                      // +0x188 (0, 1, 0)
    float roll;                               // +0x194 roll, passed to the view matrix by slot 28
    float focalLength;                        // +0x198 half viewport width / tan(fov / 2) (slot 28)
    float displayAspect;                      // +0x19c display-mode width/height (0x0042e550)
    int viewport[6];                          // +0x1a0 x, y, width, height, then min/max depth as floats (owner slot 14)
    float heightWidthRatio;                   // +0x1b8 height/width ratio (0x0042f0e0)
    float nearPlane;                          // +0x1bc g_DefaultNearPlane (1.0f); slot 28 passes it to 0x004a1460
    float farPlane;                           // +0x1c0 g_DefaultFarPlane (100000.0f); likewise
    int targetWidth;                          // +0x1c4 owner width (0x0042e550)
    int targetHeight;                         // +0x1c8 owner height (0x0042e550)
    int viewportClearFrames;                  // +0x1cc frames left in which slot 13 clears the viewport (owner slot 12)
    int projectionChangeFrames;               // +0x1d0 owner buffer count when the view changes, counted down by slot 10
    int fovKeysEnabled;                       // +0x1d4 field-of-view keys 0xc7/0xcf (slot 10)
    int widthKeysEnabled;                     // +0x1d8 viewport width keys 0xd2/0xd3 (slot 10)
    float maxFieldOfView;                     // +0x1dc 109.0f
    float minFieldOfView;                     // +0x1e0 10.0f
    _iobuf* file;                             // +0x1e4 FILE*, closed by the destructor
    int field_0x1e8;
    int field_0x1ec;
    int field_0x1f0;
    int field_0x1f4;
    int field_0x1f8;
    int field_0x1fc;
    float lastMoveDistance;                   // +0x200 distance moved last frame (slot 10)
    float lastTurnAngle;                      // +0x204 angle turned last frame (slot 10)
    Vector3 previousEyePosition;              // +0x208 eyePosition last frame (slot 10)
    Vector3 previousViewDirection;            // +0x214 viewDirection last frame (slot 10)
};
