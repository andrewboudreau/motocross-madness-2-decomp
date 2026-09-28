#include "BaseObject.h"

BaseObject::BaseObject() : refCount(1) {}
BaseObject::~BaseObject() {}

int BaseObject::AddRef() {
    return ++refCount;
}

int BaseObject::Release() {
    if (refCount != 0) {
        int remaining = refCount - 1;
        refCount = remaining;
        if (remaining == 0)
            delete this;
        return remaining;
    }
    return 0;
}

int BaseObject::GetRefCount() {
    return refCount;
}
