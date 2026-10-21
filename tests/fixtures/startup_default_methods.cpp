#include <stddef.h>
#include <stdio.h>
#include <string.h>
#include <new>
#include "../../src/reconstructed/TrackGame.h"
#include "../../src/reconstructed/BackgroundImage.h"

#define CHECK(name, condition) typedef char name[(condition) ? 1 : -1]
CHECK(first_pointer, offsetof(UnknownTrackGameObject3410Base, directPlay) == 0);
CHECK(second_pointer, offsetof(UnknownTrackGameObject3410Base, lobby) == 4);
CHECK(zone_pointer, offsetof(TrackGame, networkGameObject) == 0x3410);
CHECK(zone_allocation, sizeof(UnknownTrackGameObject3410) == 0x32c);

int main() {
    union { double align; unsigned char bytes[sizeof(UnknownTrackGameObject3410) + 32]; } zone, expected;
    union { double align; unsigned char bytes[sizeof(BackgroundImage) + 32]; } image, imageExpected;
    for (int seed = 0; seed < 256; ++seed) {
        unsigned i;
        for (i = 0; i < sizeof(zone.bytes); ++i) zone.bytes[i] = (unsigned char)(seed + 37 * i);
        memcpy(expected.bytes, zone.bytes, sizeof(zone.bytes));
        memset(expected.bytes + 16, 0, 8);
        UnknownTrackGameObject3410* created = new(zone.bytes + 16) UnknownTrackGameObject3410;
        if ((void*)created != zone.bytes + 16 || memcmp(zone.bytes, expected.bytes, sizeof(zone.bytes))) return 1;
        for (i = 0; i < sizeof(image.bytes); ++i) image.bytes[i] = (unsigned char)(seed + 53 * i);
        memcpy(imageExpected.bytes, image.bytes, sizeof(image.bytes));
        BackgroundImage* background = reinterpret_cast<BackgroundImage*>(image.bytes + 16);
        // Qualified call tests this exact default without invoking an invented vtable.
        if (background->BackgroundImage::UnknownVirtualSlot27(0) != 1 ||
            background->BackgroundImage::UnknownVirtualSlot27(reinterpret_cast<UnknownSurfaceInterface*>(1)) != 1 ||
            background->BackgroundImage::UnknownVirtualSlot27(reinterpret_cast<UnknownSurfaceInterface*>(image.bytes)) != 1)
            return 2;
        if (memcmp(image.bytes, imageExpected.bytes, sizeof(image.bytes))) return 3;
    }
    printf("Startup defaults: 256 complete constructor storage and 768 unchanged hook cases passed\n");
    return 0;
}
