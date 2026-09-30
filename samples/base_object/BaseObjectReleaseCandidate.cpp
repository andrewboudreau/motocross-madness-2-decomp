// Next VC6 calibration target. Class name/layout are evidence-backed; method name is provisional.
class BaseObject {
public:
    virtual ~BaseObject();
    virtual int AddRef();
    virtual int Release();
    virtual int GetRefCount();
protected:
    int refCount;
};

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
