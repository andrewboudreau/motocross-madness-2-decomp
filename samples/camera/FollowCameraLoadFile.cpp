// Complete readable camera-file loader; reviewed candidate, not byte-exact.
// See docs/FOLLOW_CAMERA_LOAD_FILE.md for the two observed behavior corrections.
#include "../../src/reconstructed/FollowCamera.h"
#include "../../src/reconstructed/DebugAlloc.h"
#include "../../src/reconstructed/TextureMap.h"
#include "../../src/reconstructed/UnknownResourceManager.h"
#include "../../src/krusty2/math/Vec3.h"
#include <stdio.h>
#include <string.h>
Vec3 CrossProductCall(const Vec3&,const Vec3&);
Vec3 Vec3Normalize(const Vec3&);
inline Vec3 FollowCameraCross(const Vec3& a, const Vec3& b) {
    return Vec3((a.y * b.z) - a.z * b.y, (a.z * b.x) - a.x * b.z, (a.x * b.y) - a.y * b.x);
}
void FollowCamera::LoadCameraFile(const char* path) {
    UnknownTextureStream* stream =
        new(__FILE__, 0x3c7) UnknownTextureStream(g_ResourceManager);
    if (!stream->Open(path, "rt", 0)) {
        delete stream;
        return;
    }
    char line[256];
    frameCount = 0;
    while (stream->ReadLine(line, 0xff)) {
        if (!_strnicmp(line, "CAMERA", 6))
            frameCount++;
    }
    frames = new(__FILE__, 0x3e1) Matrix4[frameCount];
    field_0x33c = new(__FILE__, 0x3e2) float[frameCount];
    stream->Seek(stream->sliceOffset, 0, 1);
    int k = 0;
    while (stream->ReadLine(line, 0xff)) {
        if (!_strnicmp(line, "CAMERA", 6)) {
            Vec3 direction;
            Vec3 upHint;
            Vec3 position;
            sscanf(line, "%*s%*s%*f%*f%*f%f%f%f%f%f%f%f%f%f%f", &direction.x, &direction.z,
                   &direction.y, &upHint.x, &upHint.z, &upHint.y, &position.x, &position.z,
                   &position.y, &field_0x33c[k]);
            Matrix4* frame = &frames[k];
            Vec3 forward = direction;
            Vec3 side = CrossProductCall(upHint, direction);
            Vec3 up = Vec3Normalize(FollowCameraCross(direction, side));
            forward = Vec3Normalize(forward);
            side = CrossProductCall(up, forward);
            // Reconstructed access-order model: retail interleaves these stores.
            // Preserve progressive access faults; this does not assert that the
            // original matrix type was volatile.
            volatile Matrix4* output = frame;
            output->m[0][0] = side.x;
            output->m[0][1] = side.y;
            output->m[1][0] = up.x;
            output->m[1][1] = up.y;
            output->m[0][2] = side.z;
            output->m[1][2] = up.z;
            output->m[2][0] = forward.x;
            output->m[0][3] = 0.0f;
            output->m[1][3] = 0.0f;
            output->m[2][1] = forward.y;
            output->m[2][2] = forward.z;
            output->m[2][3] = 0.0f;
            output->m[3][0] = position.x;
            output->m[3][1] = position.y;
            output->m[3][2] = position.z;
            output->m[3][3] = 1.0f;
            k++;
        }
    }
    delete stream;
}
