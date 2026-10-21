#include "TrackGame.h"
#include <stddef.h>
#include <string.h>

// Check the complete declaration's conversion independently of the narrow
// Game-only consumer. No game object or engine dependency is constructed.
int CheckGameBaseBoundary()
{
    TrackGame* track = reinterpret_cast<TrackGame*>(0x1000);
    if (static_cast<Game*>(track) != reinterpret_cast<Game*>(track))
        return 1;
    if (offsetof(Game, display) != 0x0c)
        return 2;
    union { double alignment; unsigned char bytes[sizeof(Game)]; } storage;
    memset(storage.bytes, 0, sizeof(storage.bytes));
    reinterpret_cast<Game*>(storage.bytes)->testKey = 1;
    if (storage.bytes[0x2d4] != 4)
        return 3;
    storage.bytes[0x2d4] = 0;
    for (unsigned int i = 0; i < sizeof(storage.bytes); ++i)
        if (storage.bytes[i])
            return 4;
    return 0;
}
