// Group1_VisualCueViews.cpp -- bring-up forwarder around VisualCue.
//
// VisualCue::Tick and BonusObjectManager (src/reconstructed/Krusty3DObjects.cpp)
// call a provisional view method whose retail target is already reconstructed
// under its canonical class:
//   BonusSoultreeObjectView::GetPosition 0x004fc970 = SoultreeObject::GetPosition
// (GetPosition's returned pointer is the argument and is ignored by the view).

#include "MatrixUtil.h"   // Vector3

struct Vec3;

class SoultreeObject {
public:
    Vec3* GetPosition(Vec3* out);
};

struct BonusSoultreeObjectView {
    void GetPosition(Vector3* translation);
};

// 0x004fc970
void BonusSoultreeObjectView::GetPosition(Vector3* translation)
{
    reinterpret_cast<SoultreeObject*>(this)->GetPosition((Vec3*)translation);
}
