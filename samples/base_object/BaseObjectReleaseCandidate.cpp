// VC6 /O2 without /G6 matches retail at 0x00405170 (32 bytes).
// Class name/layout are evidence-backed; method/member names remain provisional.
class BaseObject {
public:
    virtual ~BaseObject();
    virtual int AddRef();
    virtual int Release();
    virtual int GetRefCount();
protected:
    int refCount;
};

int BaseObject::Release() {
    int remaining = refCount;
    if (remaining != 0) {
        remaining = --refCount;
        if (remaining == 0)
            delete this;
    }
    return remaining;
}
