// Group5 bring-up definition (best-guess, link frontier). Not a strict match.
//
// SoultreeQuadTreeRenderer.h names 0x00443de0 SelectLod(int, int); the retail
// function is D3DIMSoulTree.cpp's SelectLod(Vector3*, Vector3*) (`ret 8`, null
// bounds pick the node's own). The class is declared only as far as this
// forwarder needs; the provider call is qualified, so it binds directly (no
// vtable is referenced).
struct Vector3;

class D3DIMSoultreeObject {
public:
    void SelectLod(Vector3* center, Vector3* extents);  // 0x00443de0
    void SelectLod(int center, int extents);
};

void D3DIMSoultreeObject::SelectLod(int center, int extents)
{
    SelectLod((Vector3*)center, (Vector3*)extents);
}
