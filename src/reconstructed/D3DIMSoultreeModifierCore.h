#pragma once

// Shared complete declaration; D3DIMSoultreeModifier.h documents its provenance.
#include "Wrecker.h"
class D3DIMSoultreeObject;
struct UnknownSoultreeMesh;

class D3DIMSoultreeModifier : public GraphicsTest {
public:
    explicit D3DIMSoultreeModifier(int flags);        // 0x00445240 (ret 4)
    virtual ~D3DIMSoultreeModifier();                 // 0x00445290 (deleting wrapper 0x00445270)
    virtual GameObject* SetRenderContext(void* value);    // 0x004452e0
    // _purecall in this table. D3DIMSoultreeObject 0x00440ded calls it for
    // each modifier with itself, its mesh and the address of the mesh to
    // draw (MorphBastardModifier 0x004a4c60 gives the signature).
    virtual void UnknownVirtualSlot27(D3DIMSoultreeObject* object, UnknownSoultreeMesh* mesh,
                                      UnknownSoultreeMesh** out) = 0;

    // 0x004452f0 (ret 4): appends `object` (lines 26 and 33).
    void AddObject(D3DIMSoultreeObject* object);
    // 0x00445360 (ret 4): removes `object` (lines 41 and 53).
    void RemoveObject(D3DIMSoultreeObject* object);
    // 0x004453e0: detaches the modifier from every object (lines 64 and 80);
    // field_0x3c selects the object's first (0x00444de0) or second
    // (0x00444f10) modifier list.
    void DetachFromAll();

    D3DIMSoultreeObject** modifiedObjects;            // objects
    int objectCount;                                  // object count
    int field_0x3c;                                   // 1 by default
};
