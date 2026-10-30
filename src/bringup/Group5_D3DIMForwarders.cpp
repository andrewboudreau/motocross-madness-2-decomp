// Group5 bring-up definitions (best-guess, link frontier). Not strict matches.
//
// Two D3DIMSoultreeObject views name retail functions that
// src/reconstructed/D3DIMSoulTree.cpp already defines under its own names:
//   0x0043f4b0 D3DIMObjectVirtualSlot9 (SoultreePhysicsObject.h)
//              -> UnknownVirtualSlot9 (the loader; `ret 0x14`, `this` is the
//                 full object in both views, retail does not adjust ecx)
//   0x00443de0 SelectLod(int, int) (SoultreeQuadTreeRenderer.h)
//              -> SelectLod(Vector3*, Vector3*) (`ret 8`, null bounds pick
//                 the node's own)
// The class is declared only as far as these calls need; the provider calls
// are qualified, so they bind directly (no vtable is referenced).
class GameObject;
struct SoultreeLoadDesc;
struct Vector3;

class D3DIMSoultreeObject {
public:
    virtual GameObject* UnknownVirtualSlot9(void* owner, const char* name, int lights, int options,
                                            int useOptions);  // 0x0043f4b0
    virtual void D3DIMObjectVirtualSlot9(int a1, const char* a2, const SoultreeLoadDesc* a3, int a4, int a5);
    void SelectLod(Vector3* center, Vector3* extents);  // 0x00443de0
    void SelectLod(int center, int extents);
};

// best guess: SoultreePhysicsObject passes (renderContext, modelPath, desc,
// a4, 1); the loader reads the same five dwords as (owner, path, lights,
// options, useOptions), so they are handed over unchanged.
void D3DIMSoultreeObject::D3DIMObjectVirtualSlot9(int a1, const char* a2, const SoultreeLoadDesc* a3, int a4,
                                                  int a5)
{
    D3DIMSoultreeObject::UnknownVirtualSlot9((void*)a1, a2, (int)a3, a4, a5);
}

void D3DIMSoultreeObject::SelectLod(int center, int extents)
{
    SelectLod((Vector3*)center, (Vector3*)extents);
}
