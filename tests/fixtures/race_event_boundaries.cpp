#include <stddef.h>
#include <stdio.h>
#include <string.h>
#include "../../src/reconstructed/TrackGame.h"
#include "../../src/reconstructed/BaseQuarryEvent.h"
#define CHECK(name, value) typedef char name[(value) ? 1 : -1]
CHECK(base_size, sizeof(BaseQuarryEvent) == 0xa4);
#define FIELD(view, base) CHECK(offset_##view, offsetof(TrackGameViewOwner, view) == offsetof(BaseQuarryEvent, base))
FIELD(eventScene, eventScene); FIELD(raceView, raceView); FIELD(raceCamera, raceCamera);
FIELD(statsOverlay, statsOverlay); FIELD(radarOverlay, radarOverlay); FIELD(chatOverlay, chatOverlay);
FIELD(instrumentOverlay, instrumentOverlay); FIELD(textQueue, textQueue);
FIELD(elapsedMinutes, clockMinutes); FIELD(elapsedSeconds, clockSeconds); FIELD(auralScape, auralScape);
CHECK(queue_offset, offsetof(BaseQuarryEvent, textQueue) == 0x6c);
CHECK(minutes_offset, offsetof(BaseQuarryEvent, clockMinutes) == 0x70);
CHECK(seconds_offset, offsetof(BaseQuarryEvent, clockSeconds) == 0x74);
extern "C" int EventPrimaryBase() {
    return (int)static_cast<GameObject*>(reinterpret_cast<BaseQuarryEvent*>(0x1000)) - 0x1000;
}
int main() {
    union { double align; unsigned char bytes[sizeof(TrackGameViewOwner) + 32]; } actual, expected;
    for (int seed = 0; seed < 256; ++seed) {
        for (unsigned i = 0; i < sizeof(actual.bytes); ++i) actual.bytes[i] = (unsigned char)(seed + i * 37);
        memcpy(expected.bytes, actual.bytes, sizeof(actual.bytes));
        memset(expected.bytes + 16 + 0x70, 0, 8);
        TrackGameViewOwner* view = reinterpret_cast<TrackGameViewOwner*>(actual.bytes + 16);
        reinterpret_cast<BaseQuarryEvent*>(view)->ResetClock();
        if (memcmp(actual.bytes, expected.bytes, sizeof(actual.bytes))) return 1;
        reinterpret_cast<BaseQuarryEvent*>(view)->ResetClock();
        if (memcmp(actual.bytes, expected.bytes, sizeof(actual.bytes))) return 2;
    }
    printf("Race event clock: 256 full-storage cases passed\n");
    return 0;
}
