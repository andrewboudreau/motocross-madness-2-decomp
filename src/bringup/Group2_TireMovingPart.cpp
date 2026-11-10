// Group2_TireMovingPart.cpp -- link shim: Tire.h declares the MovingPart constructor
// 0x004a23a0 as (void*, int, int); src/reconstructed/MovingPart.cpp defines it as
// (const char*, SoultreeObject*, int). Group2_VehicleWheel.cpp's Tire-view constructor
// forwards here, where the canonical declaration is visible. Bring-up glue only.
#include <new.h>

#include "MovingPart.h"

void TireConstructMovingPart(void* part, void* name, int root, int ownerRef)
{
    ::new (part) MovingPart((const char*)name, (SoultreeObject*)root, ownerRef);
}
