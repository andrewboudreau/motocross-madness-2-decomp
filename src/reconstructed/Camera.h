#pragma once

#include "GameObject.h"

// RTTI: Camera : GameObject : BaseObject. Camera introduces primary slots 27-32
// and overrides (among others) slots 5, 13 and 18. Names are provisional.
//
// Observed fields:
//   +0x018  owner object (UnknownCameraOwner)
//   +0x02c, +0x0ac, +0x06c  64-byte blocks set by slots 30, 31, 32
//   +0x1a0..+0x1ac  x, y, width, height (ints) used by slot 13
//   +0x1cc  enables the slot 13 rectangle
//   +0x1d0  owner+0x14 plus one, refreshed by slots 5 and 18

struct CameraMatrix16 { float m[16]; };
struct CameraRect { int left; int top; int right; int bottom; };
// 12-byte float triple; passed by value to slot 29.
struct CameraFloat3 { float a; float b; float c; };

class Camera;

// Object referenced from Camera+0x18. Its +0x08 is compared with the camera,
// +0x14 is read by slots 5/18, +0x50 holds the render interface used by
// PCCamera, and virtual slot 12 receives a rectangle.
class UnknownCameraOwner {
public:
    virtual void UnknownVirtualSlot0();
    virtual void UnknownVirtualSlot1();
    virtual void UnknownVirtualSlot2();
    virtual void UnknownVirtualSlot3();
    virtual void UnknownVirtualSlot4();
    virtual void UnknownVirtualSlot5();
    virtual void UnknownVirtualSlot6();
    virtual void UnknownVirtualSlot7();
    virtual void UnknownVirtualSlot8();
    virtual void UnknownVirtualSlot9();
    virtual void UnknownVirtualSlot10();
    virtual void UnknownVirtualSlot11();
    virtual void UnknownVirtualSlot12(const CameraRect* rect, int flag);
    void UnknownFunction4e8cf0(Camera* camera); // retail 0x004e8cf0
};

class Camera : public GameObject {
public:
    virtual void UnknownVirtualSlot5();
    virtual int UnknownVirtualSlot13();
    virtual int UnknownVirtualSlot18();

    virtual void UnknownVirtualSlot27();
    virtual void UnknownVirtualSlot28();
    virtual void UnknownVirtualSlot29(CameraFloat3 value);
    virtual int UnknownVirtualSlot30(const CameraMatrix16* value);
    virtual int UnknownVirtualSlot31(const CameraMatrix16* value);
    virtual int UnknownVirtualSlot32(const CameraMatrix16* value);

    void UnknownFunction42e8e0(); // retail 0x0042e8e0, called by slot 13
};
