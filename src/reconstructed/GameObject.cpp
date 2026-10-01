#include "GameObject.h"

// Object at owner(field_0x18)+0x04 used by slot 10: byte +0x70 bit 2 gates a
// call to its virtual slot 4.
class UnknownGameObjectOwnerPart {
public:
    virtual void UnknownVirtualSlot0();
    virtual void UnknownVirtualSlot1();
    virtual void UnknownVirtualSlot2();
    virtual void UnknownVirtualSlot3();
    virtual void UnknownVirtualSlot4(int value);
};

// Methods from gameobj.cpp, in retail address order. Child walks visit
// field_0x10 then each sibling's field_0x0C; bit 3 of field_0x25 skips a child.

// 0x00469050
void GameObject::UnknownVirtualSlot6() {
    for (GameObject* child = field_0x10; child; child = child->field_0x0C) {
        if (!(child->field_0x25 & 0x08))
            child->UnknownVirtualSlot6();
    }
}

// 0x00469070
void GameObject::UnknownVirtualSlot7() {
    for (GameObject* child = field_0x10; child; child = child->field_0x0C) {
        if ((child->field_0x25 & 0x01) && !(child->field_0x25 & 0x08))
            child->UnknownVirtualSlot7();
    }
}

// 0x004690a0: clears bits 0 and 1, then slot 6.
void GameObject::UnknownVirtualSlot4() {
    field_0x25 &= 0xFC;
    UnknownVirtualSlot6();
}

// 0x004692c0: visits every child, then sets bit 3 on itself.
void GameObject::UnknownVirtualSlot26() {
    for (GameObject* child = field_0x10; child; child = child->field_0x0C)
        child->UnknownVirtualSlot26();
    field_0x25 |= 0x08;
}

// 0x004692f0
GameObject* GameObject::UnknownVirtualSlot8(void* value) {
    field_0x18 = value;
    return this;
}

// 0x00469300
int GameObject::UnknownVirtualSlot12() {
    if (field_0x20 & 0x1000) {
        for (GameObject* child = field_0x10; child; child = child->field_0x0C) {
            if ((child->field_0x25 & 0x01) && !(child->field_0x25 & 0x08))
                child->UnknownVirtualSlot12();
        }
    }
    return 1;
}

// 0x00469330
int GameObject::UnknownVirtualSlot13() {
    if (field_0x20 & 0x04) {
        for (GameObject* child = field_0x10; child; child = child->field_0x0C) {
            if ((child->field_0x25 & 0x01) && !(child->field_0x25 & 0x08))
                child->UnknownVirtualSlot13();
        }
    }
    return 1;
}

// 0x00469360: stops at the first child that returns 0.
int GameObject::UnknownVirtualSlot14() {
    if (field_0x20 & 0x08) {
        for (GameObject* child = field_0x10; child; child = child->field_0x0C) {
            if ((child->field_0x25 & 0x01) && !(child->field_0x25 & 0x08)) {
                if (!child->UnknownVirtualSlot14())
                    return 0;
            }
        }
    }
    return 1;
}

// 0x004693a0
int GameObject::UnknownVirtualSlot15() {
    if (field_0x20 & 0x10) {
        for (GameObject* child = field_0x10; child; child = child->field_0x0C) {
            if ((child->field_0x25 & 0x01) && !(child->field_0x25 & 0x08))
                child->UnknownVirtualSlot15();
        }
    }
    return 1;
}

// 0x004693d0
int GameObject::UnknownVirtualSlot10(int value) {
    if (field_0x20 & 0x02) {
        for (GameObject* child = field_0x10; child; child = child->field_0x0C) {
            if ((child->field_0x25 & 0x01) && !(child->field_0x25 & 0x0C)) {
                child->UnknownVirtualSlot10(value);
                char* owner = static_cast<char*>(field_0x18);
                if (owner) {
                    UnknownGameObjectOwnerPart* part = *reinterpret_cast<UnknownGameObjectOwnerPart**>(owner + 0x04);
                    if (reinterpret_cast<unsigned char*>(part)[0x70] & 0x04)
                        part->UnknownVirtualSlot4(0);
                }
            }
        }
    }
    return 1;
}

// 0x00469430
int GameObject::UnknownVirtualSlot9(int value) {
    if (field_0x20 & 0x01) {
        for (GameObject* child = field_0x10; child; child = child->field_0x0C) {
            if ((child->field_0x25 & 0x01) && !(child->field_0x25 & 0x0C))
                child->UnknownVirtualSlot9(value);
        }
    }
    return 1;
}

// 0x00469480: stores the argument's low bit in bit 2, then propagates.
int GameObject::UnknownVirtualSlot16(int value) {
    field_0x25 = (field_0x25 & ~0x04) | ((value & 1) << 2);
    if (field_0x20 & 0x20) {
        for (GameObject* child = field_0x10; child; child = child->field_0x0C) {
            if ((child->field_0x25 & 0x01) && !(child->field_0x25 & 0x08))
                child->UnknownVirtualSlot16(value);
        }
    }
    return 1;
}

// 0x004694d0
int GameObject::UnknownVirtualSlot18() {
    if (field_0x20 & 0x80) {
        for (GameObject* child = field_0x10; child; child = child->field_0x0C) {
            if (!(child->field_0x25 & 0x08))
                child->UnknownVirtualSlot18();
        }
    }
    return 1;
}

// 0x00469500
int GameObject::UnknownVirtualSlot17() {
    if (field_0x20 & 0x40) {
        for (GameObject* child = field_0x10; child; child = child->field_0x0C) {
            if (!(child->field_0x25 & 0x08))
                child->UnknownVirtualSlot17();
        }
    }
    return 1;
}

// 0x00469530: returns 1 at the first child that returns nonzero.
int GameObject::UnknownVirtualSlot19(int value) {
    if (field_0x20 & 0x100) {
        for (GameObject* child = field_0x10; child; child = child->field_0x0C) {
            if ((child->field_0x25 & 0x01) && !(child->field_0x25 & 0x08)) {
                if (child->UnknownVirtualSlot19(value))
                    return 1;
            }
        }
    }
    return 0;
}

// 0x00469580
int GameObject::UnknownVirtualSlot22(int a, int b) {
    if (field_0x20 & 0x200) {
        for (GameObject* child = field_0x10; child; child = child->field_0x0C) {
            if ((child->field_0x25 & 0x01) && !(child->field_0x25 & 0x08)) {
                if (child->UnknownVirtualSlot22(a, b))
                    return 1;
            }
        }
    }
    return 0;
}

// 0x004695d0
int GameObject::UnknownVirtualSlot23(int a, int b) {
    if (field_0x20 & 0x400) {
        for (GameObject* child = field_0x10; child; child = child->field_0x0C) {
            if ((child->field_0x25 & 0x01) && !(child->field_0x25 & 0x08)) {
                if (child->UnknownVirtualSlot23(a, b))
                    return 1;
            }
        }
    }
    return 0;
}

// 0x004696c0: refuses while the global is set; otherwise lets the first child
// run 0x00469680, unlinks this object from its siblings and parent, then
// releases the reference.
int GameObject::Release() {
    if (g_UnknownGlobal65b548)
        return 0;
    if (field_0x10)
        field_0x10->UnknownFunction469680();
    if (field_0x08)
        field_0x08->field_0x0C = field_0x0C;
    if (field_0x0C)
        field_0x0C->field_0x08 = field_0x08;
    if (field_0x14 && field_0x14->field_0x10 == this)
        field_0x14->field_0x10 = field_0x0C;
    return BaseObject::Release();
}

// 0x00469c00: unlike slots 19/22/23, not gated by field_0x20.
int GameObject::UnknownVirtualSlot20(int value) {
    for (GameObject* child = field_0x10; child; child = child->field_0x0C) {
        if ((child->field_0x25 & 0x01) && !(child->field_0x25 & 0x08)) {
            if (child->UnknownVirtualSlot20(value))
                return 1;
        }
    }
    return 0;
}

// 0x004da540: shared by many classes (identical-code folding).
int GameObject::UnknownVirtualSlot11(int) {
    return 1;
}
