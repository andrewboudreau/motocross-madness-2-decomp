#include "ShadowCamera.h"

// 0x004da520: PCCamera(flags), then the ShadowCamera vptr.
ShadowCamera::ShadowCamera(int flags) : PCCamera(flags) {}

// 0x004da550: no child search.
int ShadowCamera::UnknownVirtualSlot19(int) {
    return 0;
}

// 0x004da560
int ShadowCamera::UnknownVirtualSlot22(UnknownControlEvent*, UnknownInputEntry*) {
    return 0;
}
