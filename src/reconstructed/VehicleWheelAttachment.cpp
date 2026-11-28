// Complete attachment update for the existing primary wheel call view.
// The original Tire sample retains its separately declared view of this body;
// this is one retail implementation, not two recovered functions. TU ownership is unproven.
#include "Suspension.h"
#include "../krusty2/vehicle/Vehicle.h"
#include "../krusty2/math/Vec3.h"
#include "Matrix4.h"
#include "../krusty2/core/SoultreeObject.h"
#include <float.h>

// 0x00515660, complete 195 bytes. The attachments are the wheel's InlineShock (primaryAux)
// and RotatingShock (secondaryAux), Tire.h's inlineShock and rotatingShock.
void VehicleWheel::UpdateAttachment()
{
    if (InlineShock* slider = (InlineShock*)primaryAux) {
        float offset = -slider->length;
        slider->node->SetPositionVec3(Vec3(offset * slider->axis.x,
                                           offset * slider->axis.y,
                                           offset * slider->axis.z));
    } else if (RotatingShock* hinge = (RotatingShock*)secondaryAux) {
        float angle = hinge->length / hinge->armLength;
        if (_finite(angle)) {
            hinge->node->SetLocalMatrix(&hinge->matrix);
            hinge->node->Rotate(*(const Vec3*)&hinge->rotationAxis, angle);
        }
    }
}
