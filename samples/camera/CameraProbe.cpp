// RTTI confirms PCCamera : Camera : GameObject : BaseObject. Camera introduces
// slots 27-32 and overrides 5/13/18 (among others); PCCamera overrides 13 and
// 30-32 (among others). BaseObject's slots are folded into GameObject here.
// Names are provisional.
//
// Camera slots 30/31/32 copy a 64-byte block (16 dwords) into +0x2c/+0xac/+0x6c
// and return 1. PCCamera's overrides call the Camera version, then pass the same
// block to method 11 of the COM-style interface held at (+0x18)->+0x50 with
// argument 1, 2 or 3 respectively. That shape is consistent with
// IDirect3DDevice7::SetTransform(world/view/projection), but the interface
// identity is inference, so neutral names are kept.

struct CameraMatrix16 { float m[16]; };
struct CameraRect { int left; int top; int right; int bottom; };

// Object referenced from Camera+0x18. Its +0x08 is compared with the camera and
// +0x50 holds the render interface; virtual slot 12 receives a rectangle.
class UnknownCameraOwner {
public:
    virtual void v00(); virtual void v01(); virtual void v02(); virtual void v03();
    virtual void v04(); virtual void v05(); virtual void v06(); virtual void v07();
    virtual void v08(); virtual void v09(); virtual void v10(); virtual void v11();
    virtual void UnknownVirtualSlot12(const CameraRect* rect, int flag);
    void UnknownFunction4e8cf0(class Camera* camera);
};

class GameObject {
public:
    virtual void v00(); virtual void v01(); virtual void v02(); virtual void v03();
    virtual void v04(); virtual void UnknownVirtualSlot5(); virtual void v06(); virtual void v07();
    virtual void v08(); virtual void v09(); virtual void v10(); virtual void v11();
    virtual void v12(); virtual int UnknownVirtualSlot13(); virtual void v14(); virtual void v15();
    virtual void v16(); virtual void v17(); virtual int UnknownVirtualSlot18(); virtual void v19();
    virtual void v20(); virtual void v21(); virtual void v22(); virtual void v23();
    virtual void v24(); virtual void v25(); virtual void v26();
};

// COM-style interface: `this` is passed on the stack (__stdcall virtuals).
struct UnknownRenderInterface {
    virtual long __stdcall m00(); virtual long __stdcall m01(); virtual long __stdcall m02();
    virtual long __stdcall m03(); virtual long __stdcall m04(); virtual long __stdcall m05();
    virtual long __stdcall m06(); virtual long __stdcall m07(); virtual long __stdcall m08();
    virtual long __stdcall m09(); virtual long __stdcall m10();
    virtual long __stdcall UnknownMethod11(int kind, const CameraMatrix16* value);
};

class Camera : public GameObject {
public:
    virtual void UnknownVirtualSlot5();
    virtual int UnknownVirtualSlot13();
    virtual int UnknownVirtualSlot18();
    virtual void v27(); virtual void v28(); virtual void v29();
    void UnknownFunction42e8e0();
    virtual int UnknownVirtualSlot30(const CameraMatrix16* value);
    virtual int UnknownVirtualSlot31(const CameraMatrix16* value);
    virtual int UnknownVirtualSlot32(const CameraMatrix16* value);
};

class PCCamera : public Camera {
public:
    virtual int UnknownVirtualSlot13();
    virtual int UnknownVirtualSlot30(const CameraMatrix16* value);
    virtual int UnknownVirtualSlot31(const CameraMatrix16* value);
    virtual int UnknownVirtualSlot32(const CameraMatrix16* value);
};

int Camera::UnknownVirtualSlot30(const CameraMatrix16* value) {
    *reinterpret_cast<CameraMatrix16*>(reinterpret_cast<char*>(this) + 0x2C) = *value;
    return 1;
}

int Camera::UnknownVirtualSlot31(const CameraMatrix16* value) {
    *reinterpret_cast<CameraMatrix16*>(reinterpret_cast<char*>(this) + 0xAC) = *value;
    return 1;
}

int Camera::UnknownVirtualSlot32(const CameraMatrix16* value) {
    *reinterpret_cast<CameraMatrix16*>(reinterpret_cast<char*>(this) + 0x6C) = *value;
    return 1;
}

// Both call the GameObject version, then store owner(+0x18)->+0x14 plus one at
// +0x1d0. Slot 5 first hands the camera to the owner's non-virtual 0x004e8cf0.
void Camera::UnknownVirtualSlot5() {
    GameObject::UnknownVirtualSlot5();
    char* p = reinterpret_cast<char*>(this);
    (*reinterpret_cast<UnknownCameraOwner**>(p + 0x18))->UnknownFunction4e8cf0(this);
    *reinterpret_cast<int*>(p + 0x1D0) = *reinterpret_cast<int*>(*reinterpret_cast<char**>(p + 0x18) + 0x14) + 1;
}

int Camera::UnknownVirtualSlot18() {
    GameObject::UnknownVirtualSlot18();
    char* p = reinterpret_cast<char*>(this);
    *reinterpret_cast<int*>(p + 0x1D0) = *reinterpret_cast<int*>(*reinterpret_cast<char**>(p + 0x18) + 0x14) + 1;
    return 1;
}

// If +0x1cc is set, passes the rectangle at +0x1a0 (x, y, width, height as
// ints) to the owner's slot 12, then calls the non-virtual 0x0042e8e0.
int Camera::UnknownVirtualSlot13() {
    char* p = reinterpret_cast<char*>(this);
    if (*reinterpret_cast<int*>(p + 0x1CC)) {
        CameraRect rect;
        int x = *reinterpret_cast<int*>(p + 0x1A0);
        int y = *reinterpret_cast<int*>(p + 0x1A4);
        rect.left = x;
        rect.top = y;
        rect.right = x + *reinterpret_cast<int*>(p + 0x1A8);
        rect.bottom = y + *reinterpret_cast<int*>(p + 0x1AC);
        (*reinterpret_cast<UnknownCameraOwner**>(p + 0x18))->UnknownVirtualSlot12(&rect, 0);
    }
    UnknownFunction42e8e0();
    return 1;
}

inline UnknownRenderInterface* CameraRenderInterface(PCCamera* camera) {
    char* owner = *reinterpret_cast<char**>(reinterpret_cast<char*>(camera) + 0x18);
    return *reinterpret_cast<UnknownRenderInterface**>(owner + 0x50);
}

int PCCamera::UnknownVirtualSlot30(const CameraMatrix16* value) {
    Camera::UnknownVirtualSlot30(value);
    UnknownRenderInterface* render = CameraRenderInterface(this);
    if (render && render->UnknownMethod11(1, value))
        return 0;
    return 1;
}

int PCCamera::UnknownVirtualSlot31(const CameraMatrix16* value) {
    Camera::UnknownVirtualSlot31(value);
    UnknownRenderInterface* render = CameraRenderInterface(this);
    if (render && render->UnknownMethod11(2, value))
        return 0;
    return 1;
}

int PCCamera::UnknownVirtualSlot32(const CameraMatrix16* value) {
    Camera::UnknownVirtualSlot32(value);
    UnknownRenderInterface* render = CameraRenderInterface(this);
    if (render && render->UnknownMethod11(3, value))
        return 0;
    return 1;
}

// After the Camera version succeeds, re-sends the +0xac and +0x6c blocks (kinds
// 2 and 3) when the owner's +0x08 is this camera. No null check here.
int PCCamera::UnknownVirtualSlot13() {
    if (!Camera::UnknownVirtualSlot13())
        return 0;
    char* owner = *reinterpret_cast<char**>(reinterpret_cast<char*>(this) + 0x18);
    if (*reinterpret_cast<PCCamera**>(owner + 0x08) == this) {
        CameraRenderInterface(this)->UnknownMethod11(2, reinterpret_cast<CameraMatrix16*>(reinterpret_cast<char*>(this) + 0xAC));
        CameraRenderInterface(this)->UnknownMethod11(3, reinterpret_cast<CameraMatrix16*>(reinterpret_cast<char*>(this) + 0x6C));
    }
    return 1;
}
