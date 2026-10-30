// Bring-up glue (see Group3_QuarryLoadLinks.h): the KrustyBikeCamera calls made by
// BaseQuarryEvent::LoadEventObjects (0x004de590).
#include "../reconstructed/KrustyBikeCamera.h"
#include "../reconstructed/FollowCamera.h"
#include "../reconstructed/DebugAlloc.h"
#include "Group3_QuarryLoadLinks.h"

// 0x00497cb0 (0x3bc bytes).
void* G3Q_NewBikeCamera(const char* file, int line, int flags)
{
    return new(file, line) KrustyBikeCamera(flags);
}

// 0x00497d90.
void* G3Q_BikeCameraSetupFlyby(void* camera, void* target, float a, float b, float c,
                               float d, float e, int capacity, int count, const int* presets)
{
    return ((KrustyBikeCamera*)camera)->SetupFlyby(target, a, b, c, d, e, capacity, count, presets);
}

// 0x00463600: FollowCamera::SetSubject on the camera (the subject is the terrain).
void G3Q_BikeCameraSetSubject(void* camera, void* subject)
{
    ((FollowCamera*)(KrustyBikeCamera*)camera)->SetSubject((UnknownFollowCameraSubject*)subject);
}
