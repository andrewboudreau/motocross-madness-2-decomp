// Twenty RTTI-confirmed classes with independently decoded primary-vtable
// slots.  Names beyond the class and slot number remain provisional.  The
// bodies deliberately use only behavior visible in the retail instructions.

#define EMPTY_SLOT(class_name, slot_name) \
    class class_name { public: virtual void slot_name(); }; \
    void class_name::slot_name() {}

#define RETURN_ONE_SLOT(class_name, slot_name) \
    class class_name { public: virtual int slot_name(); }; \
    int class_name::slot_name() { return 1; }

#define RETURN_ZERO_SLOT(class_name, slot_name) \
    class class_name { public: virtual int slot_name(); }; \
    int class_name::slot_name() { return 0; }

#define EMPTY_INT_SLOT(class_name, slot_name) \
    class class_name { public: virtual void slot_name(int unknown); }; \
    void class_name::slot_name(int) {}

#define RETURN_ONE_INT_SLOT(class_name, slot_name) \
    class class_name { public: virtual int slot_name(int unknown); }; \
    int class_name::slot_name(int) { return 1; }

EMPTY_SLOT(ConstraintMethodCollisionModel, UnknownVirtualSlot2)
EMPTY_SLOT(InfoType, UnknownVirtualSlot1)
EMPTY_SLOT(RenderTarget, UnknownVirtualSlot19)
EMPTY_SLOT(SoultreeObject, UnknownVirtualSlot5)

RETURN_ZERO_SLOT(ShadowReceiver, UnknownVirtualSlot28)
RETURN_ZERO_SLOT(UIStatic, UnknownVirtualSlot30)

RETURN_ONE_SLOT(BaseQuarryEvent, UnknownVirtualSlot31)
RETURN_ONE_SLOT(Camera, UnknownVirtualSlot14)
RETURN_ONE_SLOT(DrawableCube, UnknownVirtualSlot13)
RETURN_ONE_SLOT(LightEmitter, UnknownVirtualSlot12)
RETURN_ONE_SLOT(UIDropDownList, UnknownVirtualSlot30)

EMPTY_INT_SLOT(DrawableGridNodeSharedTextures, UnknownVirtualSlot5)
EMPTY_INT_SLOT(MPOptionsDlg, UnknownVirtualSlot31)

RETURN_ONE_INT_SLOT(BackgroundImage, UnknownVirtualSlot27)
RETURN_ONE_INT_SLOT(Fog, UnknownVirtualSlot10)

class Character {
public:
    virtual void UnknownVirtualSlot4(int unknown0, int unknown1);
};

void Character::UnknownVirtualSlot4(int, int) {}

class DrawableGridNode {
public:
    virtual int UnknownVirtualSlot2(
        int, int, int, int, int, int, int, int, int, int);
};

int DrawableGridNode::UnknownVirtualSlot2(
    int, int, int, int, int, int, int, int, int, int) {
    return 0;
}

class SoultreePhysicsBaseObject {
public:
    virtual int UnknownVirtualSlot12(int unknown);
};

int SoultreePhysicsBaseObject::UnknownVirtualSlot12(int) {
    return 0;
}

class UIMultiState {
public:
    virtual int UnknownVirtualSlot34();
};

int UIMultiState::UnknownVirtualSlot34() {
    char* self = reinterpret_cast<char*>(this);
    int index = *reinterpret_cast<int*>(self + 0x1F0);
    char* entries = *reinterpret_cast<char**>(self + 0x1F4);
    return *reinterpret_cast<int*>(entries + index * 0x20 + 8);
}

class UIStaticText {
public:
    virtual void UnknownVirtualSlot29(int value);
};

void UIStaticText::UnknownVirtualSlot29(int value) {
    char* self = reinterpret_cast<char*>(this);
    *reinterpret_cast<unsigned int*>(self + 0x1C0) = 1;
    *reinterpret_cast<int*>(self + 0x60) = value;
}
