// Group2_BikeA60C.cpp -- link bring-up: the handlebar part's constructor.
//
// Retail 0x004a23a0 (ret 0xc) is the shared MovingPart constructor body
// (src/reconstructed/MovingPart.cpp): Bike's loader builds its handlebars
// through BikeA60C's view of it. Finds `name` below `root`, copies that
// node's local matrix into +0x00, then stores the owner and clears +0x48.
#include "../krusty2/vehicle/BikeHandlebarView.h"

struct Matrix4;

// Only the two scene-node calls the constructor makes (defined in soultree.cpp).
class SoultreeObject {
public:
    SoultreeObject* FindByName(const char* name);   // 0x004fdae0
    void GetLocalMatrix(Matrix4* out);               // 0x004fca60
};

BikeA60C::BikeA60C(const char* name, SoultreeObject* root, VehicleSteerState* owner_)
{
    node = root->FindByName(name);
    node->GetLocalMatrix((Matrix4*)pad_0x00);   // the 0x40-byte local matrix at +0x00
    field_0x48 = 0;
    owner = owner_;
}
