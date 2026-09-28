#pragma once

// First promoted class reconstruction for MCM2.
//
// Confirmed by retail MSVC RTTI:
//   class name: BaseObject
//   primary vtable: 0x005507c0
//
// Confirmed layout:
//   +0x00 vptr
//   +0x04 32-bit reference-count-like field
class BaseObject {
public:
    BaseObject();
    virtual ~BaseObject();

    virtual int AddRef();
    virtual int Release();
    virtual int GetRefCount();

protected:
    int refCount; // offset/width confirmed; original member name unknown
};
