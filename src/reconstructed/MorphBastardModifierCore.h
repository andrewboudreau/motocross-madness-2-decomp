#pragma once

// Shared complete declaration; MorphBastardModifier.h documents its provenance.
#include "D3DIMSoultreeModifierCore.h"
struct MorphBastardChannel;
struct MorphBastardObject;
struct Matrix4;
struct Vector3;
class SoultreeObject;
typedef SoultreeObject MorphBastardNode;

class MorphBastardModifier : public D3DIMSoultreeModifier {
public:
    explicit MorphBastardModifier(int flags);        // 0x004a3150 (ret 4)
    virtual ~MorphBastardModifier();                 // 0x004a31a0 (deleting wrapper 0x004a3180)
    // 0x004a4c60 (ret 0xc)
    virtual void UnknownVirtualSlot27(D3DIMSoultreeObject* object, UnknownSoultreeMesh* mesh,
                                      UnknownSoultreeMesh** out);

    // 0x004a33b0 (ret 0xc): slot 8 of the base, then loads `path` with the
    // nodes of `model`.
    MorphBastardModifier* LoadParameterFile(void* value, const char* path, MorphBastardNode* model);
    // 0x004a3c80 (ret 0x44)
    void ControllerAngles(MorphBastardChannel* channel, Matrix4 inverse);
    // 0x004a4bb0 (ret 4): updates every channel of `object`.
    void UpdateChannels(MorphBastardObject* object);
    // 0x00464e80 (ret 4): the shared empty stub, called with the object index.
    void UnknownFunction464e80(int index);
    // 0x004a5290 (ret 4): makes meshCopy a copy of `mesh`.
    void CopyMesh(UnknownSoultreeMesh* mesh);

    UnknownSoultreeMesh* meshCopy;  // +0x40  CopyMesh (0x004a5290) allocates it as a copy of the mesh slot 27 receives
    int objectCount;                  // morph object count
    MorphBastardObject* morphObjects; // line 0x57
    Vector3* deltaSums;               // summed deltas per vertex (lines 0x21d..0x21f)
    Vector3* deltaMinimums;           // smallest delta per vertex
    Vector3* deltaMaximums;           // largest delta per vertex
};
