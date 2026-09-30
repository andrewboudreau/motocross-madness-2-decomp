// Evidence-backed BaseObject special-member reconstruction.
// Retail constructor @ 0x405120 initializes vptr and refCount=1.
// Retail destructor @ 0x405150 resets the vptr to BaseObject's vtable.
// VC6 also emits the scalar deleting-destructor wrapper @ 0x405130.
class BaseObject {
public:
    BaseObject();
    virtual ~BaseObject();
    virtual int AddRef();
    virtual int Release();
    virtual int GetRefCount();
protected:
    int refCount;
};

// Assigned in the body: retail stores the vptr before refCount, the order
// VC6 uses for body assignments (an initializer list stores refCount first).
BaseObject::BaseObject() { refCount = 1; }
BaseObject::~BaseObject() {}
