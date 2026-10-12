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
// firstChild then each sibling's nextSibling; pendingRelease skips a child.

// 0x00468ca0: a fresh object is unlinked, gets an empty name string, and
// then appends its own class name to it.
GameObject::GameObject(int flags) {
    field_0x24 = 0xFF;
    prevSibling = 0;
    nextSibling = 0;
    firstChild = 0;
    renderContext = 0;
    parent = 0;
    isEnabled = flags;
    field_0x25_bit1 = flags;
    isPaused = 0;
    pendingRelease = 0;
    ownFlags = 0;
    childFlags = 0;
    classNames = static_cast<char*>(DebugMalloc(1, __FILE__, 31));
    classNames[0] = 0;
    AppendClassName(this);
}

// 0x00468d60 (scalar deleting wrapper 0x00468d40)
GameObject::~GameObject() {
    if (classNames)
        DebugFree(classNames, __FILE__, 40);
}

// 0x00469050
void GameObject::UnknownVirtualSlot6() {
    for (GameObject* child = firstChild; child; child = child->nextSibling) {
        if (!child->pendingRelease)
            child->UnknownVirtualSlot6();
    }
}

// 0x00469070
void GameObject::UnknownVirtualSlot7() {
    for (GameObject* child = firstChild; child; child = child->nextSibling) {
        if (child->isEnabled && !child->pendingRelease)
            child->UnknownVirtualSlot7();
    }
}

// 0x004690a0: clears bits 0 and 1, then slot 6.
void GameObject::UnknownVirtualSlot4() {
    isEnabled = 0;
    field_0x25_bit1 = 0;
    UnknownVirtualSlot6();
}

// 0x004690b0: sets bits 0 and 1, then slot 7.
void GameObject::UnknownVirtualSlot5() {
    isEnabled = 1;
    field_0x25_bit1 = 1;
    UnknownVirtualSlot7();
}

// 0x004692c0: visits every child, then sets bit 3 on itself.
void GameObject::UnknownVirtualSlot26() {
    for (GameObject* child = firstChild; child; child = child->nextSibling)
        child->UnknownVirtualSlot26();
    pendingRelease = 1;
}

// 0x004692f0
GameObject* GameObject::UnknownVirtualSlot8(void* value) {
    renderContext = value;
    return this;
}

// 0x00469300
int GameObject::UnknownVirtualSlot12() {
    if (childFlags & 0x1000) {
        for (GameObject* child = firstChild; child; child = child->nextSibling) {
            if (child->isEnabled && !child->pendingRelease)
                child->UnknownVirtualSlot12();
        }
    }
    return 1;
}

// 0x00469330
int GameObject::UnknownVirtualSlot13() {
    if (childFlags & 0x04) {
        for (GameObject* child = firstChild; child; child = child->nextSibling) {
            if (child->isEnabled && !child->pendingRelease)
                child->UnknownVirtualSlot13();
        }
    }
    return 1;
}

// 0x00469360: stops at the first child that returns 0.
int GameObject::UnknownVirtualSlot14() {
    if (childFlags & 0x08) {
        for (GameObject* child = firstChild; child; child = child->nextSibling) {
            if (child->isEnabled && !child->pendingRelease) {
                if (!child->UnknownVirtualSlot14())
                    return 0;
            }
        }
    }
    return 1;
}

// 0x004693a0
int GameObject::UnknownVirtualSlot15() {
    if (childFlags & 0x10) {
        for (GameObject* child = firstChild; child; child = child->nextSibling) {
            if (child->isEnabled && !child->pendingRelease)
                child->UnknownVirtualSlot15();
        }
    }
    return 1;
}

// 0x004693d0
int GameObject::UnknownVirtualSlot10(float frameTime) {
    if (childFlags & 0x02) {
        for (GameObject* child = firstChild; child; child = child->nextSibling) {
            if (child->isEnabled && !child->isPaused && !child->pendingRelease) {
                child->UnknownVirtualSlot10(frameTime);
                char* owner = static_cast<char*>(renderContext);
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
    if (childFlags & 0x01) {
        for (GameObject* child = firstChild; child; child = child->nextSibling) {
            if (child->isEnabled && !child->isPaused && !child->pendingRelease)
                child->UnknownVirtualSlot9(value);
        }
    }
    return 1;
}

// 0x00469480: stores the argument's low bit in bit 2, then propagates.
int GameObject::UnknownVirtualSlot16(int value) {
    isPaused = value;
    if (childFlags & 0x20) {
        for (GameObject* child = firstChild; child; child = child->nextSibling) {
            if (child->isEnabled && !child->pendingRelease)
                child->UnknownVirtualSlot16(value);
        }
    }
    return 1;
}

// 0x004694d0
int GameObject::UnknownVirtualSlot18() {
    if (childFlags & 0x80) {
        for (GameObject* child = firstChild; child; child = child->nextSibling) {
            if (!child->pendingRelease)
                child->UnknownVirtualSlot18();
        }
    }
    return 1;
}

// 0x00469500
int GameObject::UnknownVirtualSlot17() {
    if (childFlags & 0x40) {
        for (GameObject* child = firstChild; child; child = child->nextSibling) {
            if (!child->pendingRelease)
                child->UnknownVirtualSlot17();
        }
    }
    return 1;
}

// 0x00469530: returns 1 at the first child that returns nonzero.
int GameObject::UnknownVirtualSlot19(int value) {
    if (childFlags & 0x100) {
        for (GameObject* child = firstChild; child; child = child->nextSibling) {
            if (child->isEnabled && !child->pendingRelease) {
                if (child->UnknownVirtualSlot19(value))
                    return 1;
            }
        }
    }
    return 0;
}

// 0x00469580
int GameObject::UnknownVirtualSlot22(UnknownControlEvent* event, UnknownInputEntry* entry) {
    if (childFlags & 0x200) {
        for (GameObject* child = firstChild; child; child = child->nextSibling) {
            if (child->isEnabled && !child->pendingRelease) {
                if (child->UnknownVirtualSlot22(event, entry))
                    return 1;
            }
        }
    }
    return 0;
}

// 0x004695d0
int GameObject::UnknownVirtualSlot23(UnknownControlEvent* event, UnknownInputEntry* entry) {
    if (childFlags & 0x400) {
        for (GameObject* child = firstChild; child; child = child->nextSibling) {
            if (child->isEnabled && !child->pendingRelease) {
                if (child->UnknownVirtualSlot23(event, entry))
                    return 1;
            }
        }
    }
    return 0;
}

// 0x00469620: gated by flag 0x800 and child bit 1 (not bit 0).
int GameObject::UnknownVirtualSlot24(int type, void* data, int from, int to, int flags) {
    if (childFlags & 0x800) {
        for (GameObject* child = firstChild; child; child = child->nextSibling) {
            if (child->field_0x25_bit1 && !child->pendingRelease) {
                if (child->UnknownVirtualSlot24(type, data, from, to, flags))
                    return 1;
            }
        }
    }
    return 0;
}

// 0x00469680: recurses to the last sibling first, detaches each object from
// its predecessor, then releases it.
int GameObject::ReleaseSiblings() {
    if (g_UnknownGlobal65b548)
        return 0;
    if (nextSibling)
        nextSibling->ReleaseSiblings();
    if (prevSibling)
        prevSibling->nextSibling = 0;
    nextSibling = 0;
    return Release();
}

// 0x004696c0: refuses while the global is set; otherwise lets the first child
// run 0x00469680, unlinks this object from its siblings and parent, then
// releases the reference.
int GameObject::Release() {
    if (g_UnknownGlobal65b548)
        return 0;
    if (firstChild)
        firstChild->ReleaseSiblings();
    if (prevSibling)
        prevSibling->nextSibling = nextSibling;
    if (nextSibling)
        nextSibling->prevSibling = prevSibling;
    if (parent && parent->firstChild == this)
        parent->firstChild = nextSibling;
    return BaseObject::Release();
}

// 0x00469720: hands the value to every child, then stores it in field_0x18.
int GameObject::UnknownVirtualSlot25(void* value) {
    GameObject* child = firstChild;
    while (child) {
        GameObject* next = child->nextSibling;
        if (!child->pendingRelease)
            child->UnknownVirtualSlot25(value);
        child = next;
    }
    renderContext = value;
    return 1;
}

// 0x00469c00: unlike slots 19/22/23, not gated by childFlags.
int GameObject::UnknownVirtualSlot20(int value) {
    for (GameObject* child = firstChild; child; child = child->nextSibling) {
        if (child->isEnabled && !child->pendingRelease) {
            if (child->UnknownVirtualSlot20(value))
                return 1;
        }
    }
    return 0;
}

// 0x00469c40: like slot 20, not gated by childFlags.
int GameObject::UnknownVirtualSlot21(int value) {
    for (GameObject* child = firstChild; child; child = child->nextSibling) {
        if (child->isEnabled && !child->pendingRelease) {
            if (child->UnknownVirtualSlot21(value))
                return 1;
        }
    }
    return 0;
}

// 0x00469ce0: appends the RTTI name of `object`, without its "class " prefix,
// and a comma to classNames (+0x28). The constructor passes `this`, so each
// constructor in a hierarchy contributes its own class name.
void GameObject::AppendClassName(GameObject* object) {
    const char* name = typeid(*object).name();
    int length = strlen(name);
    if (strstr(name, "class ") == name) {
        length -= 6;
        name += 6;
    }
    classNames = static_cast<char*>(
        DebugRealloc(classNames, strlen(classNames) + length + 2, __FILE__, 1163));
    if (classNames) {
        strcat(classNames, name);
        strcat(classNames, ",");
    }
}

// 0x004da540: shared by many classes (identical-code folding).
int GameObject::UnknownVirtualSlot11(int) {
    return 1;
}

// 0x004690c0
void GameObject::AddFlags(unsigned int flags) {
    GameObject* object = this;
    unsigned int combined = object->childFlags |= flags;
    while (object->parent) {
        unsigned int own = object->ownFlags;
        object = object->parent;
        combined = object->childFlags |= own | combined;
    }
}

// 0x00469100
unsigned int GameObject::RecomputeFlags() {
    unsigned int flags = 0;
    for (GameObject* child = firstChild; child; child = child->nextSibling)
        flags |= child->RecomputeFlags();
    childFlags = flags;
    return ownFlags | flags;
}

// 0x00469130: appends `object` (and its later siblings) after this object's
// last sibling, under the same parent; returns `object`.
int GameObject::AppendSibling(GameObject* object, int value) {
    if (g_UnknownGlobal65b548)
        return 0;
    if (object) {
        GameObject* last = this;
        while (last->nextSibling)
            last = last->nextSibling;
        last->nextSibling = object;
        object->prevSibling = last;
        for (GameObject* sibling = object; sibling; sibling = sibling->nextSibling)
            sibling->parent = parent;
        object->ownFlags = value;
        object->AddFlags(object->childFlags);
    }
    return (int)object;
}

// 0x00469190: appends `child` (and its later siblings) to the children;
// returns `child`.
int GameObject::AppendChild(GameObject* child, int value) {
    if (g_UnknownGlobal65b548)
        return 0;
    if (child) {
        if (!firstChild) {
            firstChild = child;
            child->prevSibling = 0;
        } else {
            GameObject* last = firstChild;
            while (last->nextSibling)
                last = last->nextSibling;
            last->nextSibling = child;
            child->prevSibling = last;
        }
        for (GameObject* sibling = child; sibling; sibling = sibling->nextSibling)
            sibling->parent = this;
        child->ownFlags = value;
        child->AddFlags(child->childFlags);
    }
    return (int)child;
}

// 0x004691f0
int GameObject::Unlink() {
    if (g_UnknownGlobal65b548)
        return 0;
    if (prevSibling)
        prevSibling->nextSibling = nextSibling;
    if (nextSibling)
        nextSibling->prevSibling = prevSibling;
    if (parent && parent->firstChild == this)
        parent->firstChild = nextSibling;
    GameObject* root = this;
    while (root->parent)
        root = root->parent;
    root->RecomputeFlags();
    parent = 0;
    nextSibling = 0;
    prevSibling = 0;
    return 1;
}

// 0x00469260: unlinks this object and inserts it before `next`.
int GameObject::InsertBefore(GameObject* next, int value) {
    if (g_UnknownGlobal65b548)
        return 0;
    Unlink();
    prevSibling = next->prevSibling;
    nextSibling = next;
    if (prevSibling)
        prevSibling->nextSibling = this;
    nextSibling->prevSibling = this;
    if (next->parent && next->parent->firstChild == next)
        next->parent->firstChild = this;
    ownFlags = value;
    AddFlags(childFlags);
    return 1;
}

// 0x00469c80
void GameObject::ReleaseDetached() {
    GameObject* child = firstChild;
    while (child) {
        if (child->pendingRelease) {
            int previous = g_MemTagStack->Push("UI");
            child->Unlink();
            GameObject* removed = child;
            child = child->nextSibling;
            removed->Release();
            g_MemTagStack->Pop(previous);
        } else {
            child->ReleaseDetached();
            child = child->nextSibling;
        }
    }
}

// 0x00468dd0: calls slot 4 on every descendant (skipping bit 3) whose name
// list or RTTI class name starts with or contains "<name>,".
void GameObject::DisableByClassName(const char* name) {
    char className[128];
    char pattern[128];
    GameObject* child = firstChild;
    sprintf(pattern, "%s,", name);
    int length = strlen(pattern) - 1;
    for (; child; child = child->nextSibling) {
        if (child->pendingRelease)
            continue;
        if (strstr(child->classNames, pattern)) {
            child->UnknownVirtualSlot4();
        } else {
            strcpy(className, typeid(*child).name());
            if (strlen(className) > 6 && !strncmp(className + 6, pattern, length) &&
                strstr(className, "class ") == className)
                child->UnknownVirtualSlot4();
            if (!strncmp(className, pattern, length))
                child->UnknownVirtualSlot4();
        }
        child->DisableByClassName(name);
    }
}

// 0x00468f10: the same walk calling slot 5.
void GameObject::EnableByClassName(const char* name) {
    char className[128];
    char pattern[128];
    GameObject* child = firstChild;
    sprintf(pattern, "%s,", name);
    int length = strlen(pattern) - 1;
    for (; child; child = child->nextSibling) {
        if (child->pendingRelease)
            continue;
        if (strstr(child->classNames, pattern)) {
            child->UnknownVirtualSlot5();
        } else {
            strcpy(className, typeid(*child).name());
            if (strlen(className) > 6 && !strncmp(className + 6, pattern, length) &&
                strstr(className, "class ") == className)
                child->UnknownVirtualSlot5();
            if (!strncmp(className, pattern, length))
                child->UnknownVirtualSlot5();
        }
        child->EnableByClassName(name);
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
        object = firstChild;
        break;
    case 2:
        object = prevSibling;
        if (object) {
            while (object->prevSibling)
                object = object->prevSibling;
        } else {
            object = nextSibling;
        }
        break;
    case 3:
    case 4:
        object = parent;
        break;
    default:
        return 0;
    }
    while (object) {
        if (!name)
            return object;
        if (strstr(object->classNames, pattern))
            return object;
        strcpy(className, typeid(*object).name());
        if (strlen(className) > 6 && !strncmp(className + 6, pattern, length) &&
            strstr(className, "class ") == className)
            return object;
        if (!strncmp(className, pattern, length))
            return object;
        switch (mode) {
        case 0:
            object = object->nextSibling;
            break;
        case 1: {
            GameObject* found = object->FindByClassName(1, name);
            if (found)
                return found;
            object = object->nextSibling;
            break;
        }
        case 2:
            object = object->nextSibling;
            if (object == this)
                object = object->nextSibling;
            break;
        case 3:
            return 0;
        case 4:
            object = object->parent;
            break;
        }
    }
    return 0;
}

// 0x00469950
GameObjectIterator::GameObjectIterator(GameObject* root, int mode, const char* filter) {
    rootObject = root;
    walkMode = mode;
    if (filter) {
        sprintf(filterText, "%s,", filter);
        filterLength = strlen(filterText) - 1;
    } else {
        filterText[0] = 0;
        filterLength = 0;
    }
    switch (walkMode) {
    case 0:
    case 1:
        currentObject = rootObject->firstChild;
        break;
    case 2:
        if (rootObject->prevSibling) {
            currentObject = rootObject->prevSibling;
            while (currentObject->prevSibling)
                currentObject = currentObject->prevSibling;
        } else {
            currentObject = rootObject->nextSibling;
        }
        break;
    case 3:
    case 4:
        currentObject = rootObject->parent;
        break;
    }
    g_UnknownGlobal65b548++;
    exhausted = 0;
}

// 0x00469a20
void GameObjectIterator::Stop() {
    if (!exhausted)
        g_UnknownGlobal65b548--;
    currentObject = 0;
}

// 0x00469a40
GameObjectIterator::~GameObjectIterator() {
    Stop();
}

// 0x00469a50: the next match, or 0 (releasing the global) once the walk
// runs out.
GameObject* GameObjectIterator::Next() {
    char className[128];
    if (!currentObject)
        return 0;
    GameObject* found;
    do {
        if (filterText[0]) {
            if (strstr(currentObject->classNames, filterText)) {
                found = currentObject;
            } else {
                strcpy(className, typeid(*currentObject).name());
                if (strlen(className) > 6 && !strncmp(className + 6, filterText, filterLength) &&
                    strstr(className, "class ") == className)
                    found = currentObject;
                if (!strncmp(className, filterText, filterLength))
                    found = currentObject;
                else
                    found = 0;
            }
        } else {
            found = currentObject;
        }
        switch (walkMode) {
        case 0:
            currentObject = currentObject->nextSibling;
            break;
        case 1:
            if (currentObject->firstChild) {
                currentObject = currentObject->firstChild;
            } else if (currentObject->nextSibling) {
                currentObject = currentObject->nextSibling;
            } else if (currentObject == rootObject || currentObject->parent == rootObject) {
                currentObject = 0;
            } else {
                for (currentObject = currentObject->parent; currentObject; currentObject = currentObject->parent) {
                    if (currentObject == rootObject) {
                        currentObject = 0;
                        break;
                    }
                    if (currentObject->nextSibling) {
                        currentObject = currentObject->nextSibling;
                        break;
                    }
                }
            }
            break;
        case 2:
            currentObject = currentObject->nextSibling;
            if (currentObject == rootObject)
                currentObject = currentObject->nextSibling;
            break;
        case 3:
            currentObject = 0;
            break;
        case 4:
            currentObject = currentObject->parent;
            break;
        }
    } while (!found && currentObject);
    if (!currentObject) {
        g_UnknownGlobal65b548--;
        exhausted = 1;
    }
    return found;
}
