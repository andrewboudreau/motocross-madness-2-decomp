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
