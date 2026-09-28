// First-pass reconstruction from the retail MCM2 vtable/code at 0x005507c0.
// The names AddRef/Release/GetRefCount are semantic provisional names; the class
// name BaseObject itself is confirmed by MSVC RTTI in the executable.
class BaseObject {
public:
    virtual ~BaseObject();
    virtual int AddRef();
    virtual int Release();
    virtual int GetRefCount();

protected:
    int refCount; // confirmed at this+4 by all three methods
};

int BaseObject::AddRef() {
    return ++refCount;
}

int BaseObject::GetRefCount() {
    return refCount;
}
