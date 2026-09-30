#include "BaseObject.h"

// Assigned in the body: retail stores the vptr before refCount, the order
// VC6 uses for body assignments (an initializer list stores refCount first).
BaseObject::BaseObject() { refCount = 1; }
BaseObject::~BaseObject() {}

int BaseObject::AddRef() {
    return ++refCount;
}

// Single exit: retail returns the loaded count itself when it is already zero
// (no separate `return 0` path). Exact under VC6 SP3 /O2 without /G6; /G6
// schedules `push esi` after the zero test instead of before it.
int BaseObject::Release() {
    int remaining = refCount;
    if (remaining) {
        remaining = --refCount;
        if (!remaining)
            delete this;
    }
    return remaining;
}

int BaseObject::GetRefCount() {
    return refCount;
}
