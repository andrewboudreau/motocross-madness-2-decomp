#include "PCCamera.h"
#include "visibility/VisibilityQuadTree.h"
#include <stddef.h>

typedef char ActualCameraSize[sizeof(PCCamera) == 0x220 ? 1 : -1];
typedef char SnapshotViewSize[sizeof(VisibilityCamera) == 0x220 ? 1 : -1];
typedef char ViewPosition[offsetof(VisibilityCamera, worldX) == 0x170 ? 1 : -1];
typedef char ViewContext[offsetof(VisibilityCamera, renderer) == 0x18 ? 1 : -1];

class CameraLayout : public PCCamera {
public:
    static int Check() {
        typedef char Context[offsetof(CameraLayout, renderContext) == 0x18 ? 1 : -1];
        typedef char Position[offsetof(CameraLayout, eyePosition) == 0x170 ? 1 : -1];
        typedef char File[offsetof(CameraLayout, file) == 0x1e4 ? 1 : -1];
        typedef char LastField[offsetof(CameraLayout, previousViewDirection) == 0x214 ? 1 : -1];
        return sizeof(Context) + sizeof(Position) + sizeof(File) + sizeof(LastField);
    }
};

// Compile-only boundary proof: no camera or provisional record is constructed.
Camera* FrozenCameraPrimary(PCCamera* camera) { return camera; }
int FrozenCameraLayout() { return CameraLayout::Check(); }
