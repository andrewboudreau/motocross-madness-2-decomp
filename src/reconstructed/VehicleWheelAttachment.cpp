// Complete attachment update for the existing primary wheel call view.
// The original Tire sample retains its separately declared view of this body;
// this is one retail implementation, not two recovered functions. TU ownership is unproven.
#include "TireAttachmentViews.h"
#include "../krusty2/vehicle/Vehicle.h"
#include "../krusty2/math/Vec3.h"
#include "Matrix4.h"
#include "../krusty2/core/SoultreeObject.h"
#include <float.h>

// 0x00515660, complete 195 bytes. Attachment pointers retain their provisional local views.
void VehicleWheel::UpdateAttachment()
{
    if (TireAttachA* slider = (TireAttachA*)primaryAux) {
        float offset = -slider->position;
        ((SoultreeObject*)slider->node)->SetPositionVec3(Vec3(offset * slider->direction.x,
                                           offset * slider->direction.y,
                                           offset * slider->direction.z));
    } else if (TireAttachB* hinge = (TireAttachB*)secondaryAux) {
        float angle = hinge->position / hinge->scale;
        if (_finite(angle)) {
            ((SoultreeObject*)hinge->node)->SetLocalMatrix((const Matrix4*)&hinge->restMatrix);
            ((SoultreeObject*)hinge->node)->Rotate(*(const Vec3*)&hinge->axis, angle);
        }
    }
}
