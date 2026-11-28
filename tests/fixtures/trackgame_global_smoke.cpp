#include "TrackGameGlobal.h"
#include <stdio.h>

// Validation-only storage. The game definition and singleton remain owned by
// MSZoneInterface.cpp. This fixture does not construct a TrackGame object.
TrackGame* g_TrackGame;

int ReadParticleRendererFlag();
void WriteParticleRendererFlag(int value);
int ReadEcoRendererFlag();
void WriteEcoRendererFlag(int value);
int CheckGameBaseBoundary();

// Backing storage up to Game::bladeRenderer (+0x2d0), the word both readers share.
struct RendererFlagStorage {
    char field_0x00[0x2d0];
    int bladeRenderer;
};

int main()
{
    if (CheckGameBaseBoundary())
        return 6;
    RendererFlagStorage first = {0};
    RendererFlagStorage second = {0};
    first.bladeRenderer = 17;
    second.bladeRenderer = 29;

    g_TrackGame = reinterpret_cast<TrackGame*>(&first);
    if (ReadParticleRendererFlag() != 17 || ReadEcoRendererFlag() != 17)
        return 1;
    WriteParticleRendererFlag(31);
    if (ReadEcoRendererFlag() != 31 || first.bladeRenderer != 31)
        return 2;

    g_TrackGame = reinterpret_cast<TrackGame*>(&second);
    if (ReadParticleRendererFlag() != 29 || ReadEcoRendererFlag() != 29)
        return 3;
    WriteEcoRendererFlag(43);
    if (ReadParticleRendererFlag() != 43 || second.bladeRenderer != 43 || first.bladeRenderer != 31)
        return 4;

    g_TrackGame = reinterpret_cast<TrackGame*>(&first);
    if (ReadParticleRendererFlag() != 31 || ReadEcoRendererFlag() != 31)
        return 5;
    puts("TrackGame views share writes and observe pointer reassignment");
    return 0;
}
