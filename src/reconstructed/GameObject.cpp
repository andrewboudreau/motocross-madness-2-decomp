#include "GameObject.h"
#include "GameObjectIterator.h"
#include "DebugAlloc.h"
#include "MemTag.h"

#include <stdio.h>
#include <string.h>
#include <typeinfo.h>

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
// field_0x10 then each sibling's field_0x0C; field_0x25_bit3 skips a child.

// 0x00468ca0: a fresh object is unlinked, gets an empty name string, and
// then appends its own class name to it.
GameObject::GameObject(int flags) {
    field_0x24 = 0xFF;
    field_0x08 = 0;
    field_0x0C = 0;
    field_0x10 = 0;
    field_0x18 = 0;
    field_0x14 = 0;
    field_0x25_bit0 = flags;
    field_0x25_bit1 = flags;
    field_0x25_bit2 = 0;
    field_0x25_bit3 = 0;
    field_0x1C = 0;
    field_0x20 = 0;
    field_0x28 = static_cast<char*>(DebugMalloc(1, __FILE__, 31));
    field_0x28[0] = 0;
    AppendClassName(this);
}

// 0x00468d60 (scalar deleting wrapper 0x00468d40)
GameObject::~GameObject() {
    if (field_0x28)
        DebugFree(field_0x28, __FILE__, 40);
}

// 0x00469050
void GameObject::UnknownVirtualSlot6() {
    for (GameObject* child = field_0x10; child; child = child->field_0x0C) {
        if (!child->field_0x25_bit3)
            child->UnknownVirtualSlot6();
    }
}

// 0x00469070
void GameObject::UnknownVirtualSlot7() {
    for (GameObject* child = field_0x10; child; child = child->field_0x0C) {
        if (child->field_0x25_bit0 && !child->field_0x25_bit3)
            child->UnknownVirtualSlot7();
    }
}

// 0x004690a0: clears bits 0 and 1, then slot 6.
void GameObject::UnknownVirtualSlot4() {
    field_0x25_bit0 = 0;
    field_0x25_bit1 = 0;
    UnknownVirtualSlot6();
}

// 0x004690b0: sets bits 0 and 1, then slot 7.
void GameObject::UnknownVirtualSlot5() {
    field_0x25_bit0 = 1;
    field_0x25_bit1 = 1;
    UnknownVirtualSlot7();
}

// 0x004692c0: visits every child, then sets bit 3 on itself.
void GameObject::UnknownVirtualSlot26() {
    for (GameObject* child = field_0x10; child; child = child->field_0x0C)
        child->UnknownVirtualSlot26();
    field_0x25_bit3 = 1;
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
            if (child->field_0x25_bit0 && !child->field_0x25_bit3)
                child->UnknownVirtualSlot12();
        }
    }
    return 1;
}

// 0x00469330
int GameObject::UnknownVirtualSlot13() {
    if (field_0x20 & 0x04) {
        for (GameObject* child = field_0x10; child; child = child->field_0x0C) {
            if (child->field_0x25_bit0 && !child->field_0x25_bit3)
                child->UnknownVirtualSlot13();
        }
    }
    return 1;
}

// 0x00469360: stops at the first child that returns 0.
int GameObject::UnknownVirtualSlot14() {
    if (field_0x20 & 0x08) {
        for (GameObject* child = field_0x10; child; child = child->field_0x0C) {
            if (child->field_0x25_bit0 && !child->field_0x25_bit3) {
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
            if (child->field_0x25_bit0 && !child->field_0x25_bit3)
                child->UnknownVirtualSlot15();
        }
    }
    return 1;
}

// 0x004693d0
int GameObject::UnknownVirtualSlot10(float frameTime) {
    if (field_0x20 & 0x02) {
        for (GameObject* child = field_0x10; child; child = child->field_0x0C) {
            if (child->field_0x25_bit0 && !child->field_0x25_bit2 && !child->field_0x25_bit3) {
                child->UnknownVirtualSlot10(frameTime);
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
            if (child->field_0x25_bit0 && !child->field_0x25_bit2 && !child->field_0x25_bit3)
                child->UnknownVirtualSlot9(value);
        }
    }
    return 1;
}

// 0x00469480: stores the argument's low bit in bit 2, then propagates.
int GameObject::UnknownVirtualSlot16(int value) {
    field_0x25_bit2 = value;
    if (field_0x20 & 0x20) {
        for (GameObject* child = field_0x10; child; child = child->field_0x0C) {
            if (child->field_0x25_bit0 && !child->field_0x25_bit3)
                child->UnknownVirtualSlot16(value);
        }
    }
    return 1;
}

// 0x004694d0
int GameObject::UnknownVirtualSlot18() {
    if (field_0x20 & 0x80) {
        for (GameObject* child = field_0x10; child; child = child->field_0x0C) {
            if (!child->field_0x25_bit3)
                child->UnknownVirtualSlot18();
        }
    }
    return 1;
}

// 0x00469500
int GameObject::UnknownVirtualSlot17() {
    if (field_0x20 & 0x40) {
        for (GameObject* child = field_0x10; child; child = child->field_0x0C) {
            if (!child->field_0x25_bit3)
                child->UnknownVirtualSlot17();
        }
    }
    return 1;
}

// 0x00469530: returns 1 at the first child that returns nonzero.
int GameObject::UnknownVirtualSlot19(int value) {
    if (field_0x20 & 0x100) {
        for (GameObject* child = field_0x10; child; child = child->field_0x0C) {
            if (child->field_0x25_bit0 && !child->field_0x25_bit3) {
                if (child->UnknownVirtualSlot19(value))
                    return 1;
            }
        }
    }
    return 0;
}

// 0x00469580
int GameObject::UnknownVirtualSlot22(UnknownControlEvent* event, UnknownInputEntry* entry) {
    if (field_0x20 & 0x200) {
        for (GameObject* child = field_0x10; child; child = child->field_0x0C) {
            if (child->field_0x25_bit0 && !child->field_0x25_bit3) {
                if (child->UnknownVirtualSlot22(event, entry))
                    return 1;
            }
        }
    }
    return 0;
}

// 0x004695d0
int GameObject::UnknownVirtualSlot23(UnknownControlEvent* event, UnknownInputEntry* entry) {
    if (field_0x20 & 0x400) {
        for (GameObject* child = field_0x10; child; child = child->field_0x0C) {
            if (child->field_0x25_bit0 && !child->field_0x25_bit3) {
                if (child->UnknownVirtualSlot23(event, entry))
                    return 1;
            }
        }
    }
    return 0;
}

// 0x00469620: gated by flag 0x800 and child bit 1 (not bit 0).
int GameObject::UnknownVirtualSlot24(int type, void* data, int from, int to, int flags) {
    if (field_0x20 & 0x800) {
        for (GameObject* child = field_0x10; child; child = child->field_0x0C) {
            if (child->field_0x25_bit1 && !child->field_0x25_bit3) {
                if (child->UnknownVirtualSlot24(type, data, from, to, flags))
                    return 1;
            }
        }
    }
    return 0;
}

// 0x00469680: recurses to the last sibling first, detaches each object from
// its predecessor, then releases it.
int GameObject::UnknownFunction469680() {
    if (g_UnknownGlobal65b548)
        return 0;
    if (field_0x0C)
        field_0x0C->UnknownFunction469680();
    if (field_0x08)
        field_0x08->field_0x0C = 0;
    field_0x0C = 0;
    return Release();
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

// 0x00469720: hands the value to every child, then stores it in field_0x18.
int GameObject::UnknownVirtualSlot25(void* value) {
    GameObject* child = field_0x10;
    while (child) {
        GameObject* next = child->field_0x0C;
        if (!child->field_0x25_bit3)
            child->UnknownVirtualSlot25(value);
        child = next;
    }
    field_0x18 = value;
    return 1;
}

// 0x00469c00: unlike slots 19/22/23, not gated by field_0x20.
int GameObject::UnknownVirtualSlot20(int value) {
    for (GameObject* child = field_0x10; child; child = child->field_0x0C) {
        if (child->field_0x25_bit0 && !child->field_0x25_bit3) {
            if (child->UnknownVirtualSlot20(value))
                return 1;
        }
    }
    return 0;
}

// 0x00469c40: like slot 20, not gated by field_0x20.
int GameObject::UnknownVirtualSlot21(int value) {
    for (GameObject* child = field_0x10; child; child = child->field_0x0C) {
        if (child->field_0x25_bit0 && !child->field_0x25_bit3) {
            if (child->UnknownVirtualSlot21(value))
                return 1;
        }
    }
    return 0;
}

// 0x00469ce0: appends the RTTI name of `object`, without its "class " prefix,
// and a comma to field_0x28. The constructor passes `this`, so each
// constructor in a hierarchy contributes its own class name.
void GameObject::AppendClassName(GameObject* object) {
    const char* name = typeid(*object).name();
    int length = strlen(name);
    if (strstr(name, "class ") == name) {
        length -= 6;
        name += 6;
    }
    field_0x28 = static_cast<char*>(
        DebugRealloc(field_0x28, strlen(field_0x28) + length + 2, __FILE__, 1163));
    if (field_0x28) {
        strcat(field_0x28, name);
        strcat(field_0x28, ",");
    }
}

// 0x004da540: shared by many classes (identical-code folding).
int GameObject::UnknownVirtualSlot11(int) {
    return 1;
}

// 0x004690c0
void GameObject::UnknownFunction4690c0(unsigned int flags) {
    GameObject* object = this;
    unsigned int combined = object->field_0x20 |= flags;
    while (object->field_0x14) {
        unsigned int own = object->field_0x1C;
        object = object->field_0x14;
        combined = object->field_0x20 |= own | combined;
    }
}

// 0x00469100
unsigned int GameObject::UnknownFunction469100() {
    unsigned int flags = 0;
    for (GameObject* child = field_0x10; child; child = child->field_0x0C)
        flags |= child->UnknownFunction469100();
    field_0x20 = flags;
    return field_0x1C | flags;
}

// 0x00469130: appends `object` (and its later siblings) after this object's
// last sibling, under the same parent; returns `object`.
int GameObject::UnknownFunction469130(GameObject* object, int value) {
    if (g_UnknownGlobal65b548)
        return 0;
    if (object) {
        GameObject* last = this;
        while (last->field_0x0C)
            last = last->field_0x0C;
        last->field_0x0C = object;
        object->field_0x08 = last;
        for (GameObject* sibling = object; sibling; sibling = sibling->field_0x0C)
            sibling->field_0x14 = field_0x14;
        object->field_0x1C = value;
        object->UnknownFunction4690c0(object->field_0x20);
    }
    return (int)object;
}

// 0x00469190: appends `child` (and its later siblings) to the children;
// returns `child`.
int GameObject::AppendChild(GameObject* child, int value) {
    if (g_UnknownGlobal65b548)
        return 0;
    if (child) {
        if (!field_0x10) {
            field_0x10 = child;
            child->field_0x08 = 0;
        } else {
            GameObject* last = field_0x10;
            while (last->field_0x0C)
                last = last->field_0x0C;
            last->field_0x0C = child;
            child->field_0x08 = last;
        }
        for (GameObject* sibling = child; sibling; sibling = sibling->field_0x0C)
            sibling->field_0x14 = this;
        child->field_0x1C = value;
        child->UnknownFunction4690c0(child->field_0x20);
    }
    return (int)child;
}

// 0x004691f0
int GameObject::UnknownFunction4691f0() {
    if (g_UnknownGlobal65b548)
        return 0;
    if (field_0x08)
        field_0x08->field_0x0C = field_0x0C;
    if (field_0x0C)
        field_0x0C->field_0x08 = field_0x08;
    if (field_0x14 && field_0x14->field_0x10 == this)
        field_0x14->field_0x10 = field_0x0C;
    GameObject* root = this;
    while (root->field_0x14)
        root = root->field_0x14;
    root->UnknownFunction469100();
    field_0x14 = 0;
    field_0x0C = 0;
    field_0x08 = 0;
    return 1;
}

// 0x00469260: unlinks this object and inserts it before `next`.
int GameObject::UnknownFunction469260(GameObject* next, int value) {
    if (g_UnknownGlobal65b548)
        return 0;
    UnknownFunction4691f0();
    field_0x08 = next->field_0x08;
    field_0x0C = next;
    if (field_0x08)
        field_0x08->field_0x0C = this;
    field_0x0C->field_0x08 = this;
    if (next->field_0x14 && next->field_0x14->field_0x10 == next)
        next->field_0x14->field_0x10 = this;
    field_0x1C = value;
    UnknownFunction4690c0(field_0x20);
    return 1;
}

// 0x00469c80
void GameObject::UnknownFunction469c80() {
    GameObject* child = field_0x10;
    while (child) {
        if (child->field_0x25_bit3) {
            int previous = g_MemTagStack->Push("UI");
            child->UnknownFunction4691f0();
            GameObject* removed = child;
            child = child->field_0x0C;
            removed->Release();
            g_MemTagStack->Pop(previous);
        } else {
            child->UnknownFunction469c80();
            child = child->field_0x0C;
        }
    }
}

// 0x00468dd0: calls slot 4 on every descendant (skipping bit 3) whose name
// list or RTTI class name starts with or contains "<name>,".
void GameObject::UnknownFunction468dd0(const char* name) {
    char className[128];
    char pattern[128];
    GameObject* child = field_0x10;
    sprintf(pattern, "%s,", name);
    int length = strlen(pattern) - 1;
    for (; child; child = child->field_0x0C) {
        if (child->field_0x25_bit3)
            continue;
        if (strstr(child->field_0x28, pattern)) {
            child->UnknownVirtualSlot4();
        } else {
            strcpy(className, typeid(*child).name());
            if (strlen(className) > 6 && !strncmp(className + 6, pattern, length) &&
                strstr(className, "class ") == className)
                child->UnknownVirtualSlot4();
            if (!strncmp(className, pattern, length))
                child->UnknownVirtualSlot4();
        }
        child->UnknownFunction468dd0(name);
    }
}

// 0x00468f10: the same walk calling slot 5.
void GameObject::UnknownFunction468f10(const char* name) {
    char className[128];
    char pattern[128];
    GameObject* child = field_0x10;
    sprintf(pattern, "%s,", name);
    int length = strlen(pattern) - 1;
    for (; child; child = child->field_0x0C) {
        if (child->field_0x25_bit3)
            continue;
        if (strstr(child->field_0x28, pattern)) {
            child->UnknownVirtualSlot5();
        } else {
            strcpy(className, typeid(*child).name());
            if (strlen(className) > 6 && !strncmp(className + 6, pattern, length) &&
                strstr(className, "class ") == className)
                child->UnknownVirtualSlot5();
            if (!strncmp(className, pattern, length))
                child->UnknownVirtualSlot5();
        }
        child->UnknownFunction468f10(name);
    }
}

// 0x00469770: finds an object whose name list or RTTI class name matches
// "<name>," (any object when `name` is 0). Modes: 0 the children, 1 all
// descendants (depth first), 2 the other siblings, 3 the parent, 4 the
// ancestors.
GameObject* GameObject::FindByClassName(int mode, const char* name) {
    char className[128];
    char pattern[128];
    int length = 0;
    if (name) {
        sprintf(pattern, "%s,", name);
        length = strlen(pattern) - 1;
    }
    GameObject* object;
    switch (mode) {
    case 0:
    case 1:
        object = field_0x10;
        break;
    case 2:
        object = field_0x08;
        if (object) {
            while (object->field_0x08)
                object = object->field_0x08;
        } else {
            object = field_0x0C;
        }
        break;
    case 3:
    case 4:
        object = field_0x14;
        break;
    default:
        return 0;
    }
    while (object) {
        if (!name)
            return object;
        if (strstr(object->field_0x28, pattern))
            return object;
        strcpy(className, typeid(*object).name());
        if (strlen(className) > 6 && !strncmp(className + 6, pattern, length) &&
            strstr(className, "class ") == className)
            return object;
        if (!strncmp(className, pattern, length))
            return object;
        switch (mode) {
        case 0:
            object = object->field_0x0C;
            break;
        case 1: {
            GameObject* found = object->FindByClassName(1, name);
            if (found)
                return found;
            object = object->field_0x0C;
            break;
        }
        case 2:
            object = object->field_0x0C;
            if (object == this)
                object = object->field_0x0C;
            break;
        case 3:
            return 0;
        case 4:
            object = object->field_0x14;
            break;
        }
    }
    return 0;
}

// 0x00469950
GameObjectIterator::GameObjectIterator(GameObject* root, int mode, const char* filter) {
    field_0x00 = root;
    field_0x08 = mode;
    if (filter) {
        sprintf(field_0x0c, "%s,", filter);
        field_0x8c = strlen(field_0x0c) - 1;
    } else {
        field_0x0c[0] = 0;
        field_0x8c = 0;
    }
    switch (field_0x08) {
    case 0:
    case 1:
        field_0x04 = field_0x00->field_0x10;
        break;
    case 2:
        if (field_0x00->field_0x08) {
            field_0x04 = field_0x00->field_0x08;
            while (field_0x04->field_0x08)
                field_0x04 = field_0x04->field_0x08;
        } else {
            field_0x04 = field_0x00->field_0x0C;
        }
        break;
    case 3:
    case 4:
        field_0x04 = field_0x00->field_0x14;
        break;
    }
    g_UnknownGlobal65b548++;
    field_0x90 = 0;
}

// 0x00469a20
void GameObjectIterator::UnknownFunction469a20() {
    if (!field_0x90)
        g_UnknownGlobal65b548--;
    field_0x04 = 0;
}

// 0x00469a40
GameObjectIterator::~GameObjectIterator() {
    UnknownFunction469a20();
}

// 0x00469a50: the next match, or 0 (releasing the global) once the walk
// runs out.
GameObject* GameObjectIterator::Next() {
    char className[128];
    if (!field_0x04)
        return 0;
    GameObject* found;
    do {
        if (field_0x0c[0]) {
            if (strstr(field_0x04->field_0x28, field_0x0c)) {
                found = field_0x04;
            } else {
                strcpy(className, typeid(*field_0x04).name());
                if (strlen(className) > 6 && !strncmp(className + 6, field_0x0c, field_0x8c) &&
                    strstr(className, "class ") == className)
                    found = field_0x04;
                if (!strncmp(className, field_0x0c, field_0x8c))
                    found = field_0x04;
                else
                    found = 0;
            }
        } else {
            found = field_0x04;
        }
        switch (field_0x08) {
        case 0:
            field_0x04 = field_0x04->field_0x0C;
            break;
        case 1:
            if (field_0x04->field_0x10) {
                field_0x04 = field_0x04->field_0x10;
            } else if (field_0x04->field_0x0C) {
                field_0x04 = field_0x04->field_0x0C;
            } else if (field_0x04 == field_0x00 || field_0x04->field_0x14 == field_0x00) {
                field_0x04 = 0;
            } else {
                for (field_0x04 = field_0x04->field_0x14; field_0x04; field_0x04 = field_0x04->field_0x14) {
                    if (field_0x04 == field_0x00) {
                        field_0x04 = 0;
                        break;
                    }
                    if (field_0x04->field_0x0C) {
                        field_0x04 = field_0x04->field_0x0C;
                        break;
                    }
                }
            }
            break;
        case 2:
            field_0x04 = field_0x04->field_0x0C;
            if (field_0x04 == field_0x00)
                field_0x04 = field_0x04->field_0x0C;
            break;
        case 3:
            field_0x04 = 0;
            break;
        case 4:
            field_0x04 = field_0x04->field_0x14;
            break;
        }
    } while (!found && field_0x04);
    if (!field_0x04) {
        g_UnknownGlobal65b548--;
        field_0x90 = 1;
    }
    return found;
}
