#include "BaseObject.h"

BaseObject::BaseObject() { refCount = 1; }
BaseObject::~BaseObject() {}

int BaseObject::AddRef() {
    return ++refCount;
}

int BaseObject::Release() {
    int remaining = refCount;
    if (remaining != 0) {
        remaining = --refCount;
        if (remaining == 0)
            delete this;
    }
    return remaining;
}

int BaseObject::GetRefCount() {
    return refCount;
}
