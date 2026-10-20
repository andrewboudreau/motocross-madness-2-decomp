#include "TrackGameGlobal.h"
#include "effects/ParticleManager.h"
#include <stdio.h>

// Validation-only storage. The game definition and singleton remain owned by
// MSZoneInterface.cpp. This fixture does not construct a TrackGame object.
TrackGame* g_TrackGame;

int ReadParticleRendererFlag();
void WriteParticleRendererFlag(int value);
int ReadEcoRendererFlag();
void WriteEcoRendererFlag(int value);

int main()
{
    ParticleGameContext first = {0};
    ParticleGameContext second = {0};
    first.hidden = 17;
    second.hidden = 29;

    g_TrackGame = reinterpret_cast<TrackGame*>(&first);
    if (ReadParticleRendererFlag() != 17 || ReadEcoRendererFlag() != 17)
        return 1;
    WriteParticleRendererFlag(31);
    if (ReadEcoRendererFlag() != 31 || first.hidden != 31)
        return 2;

    g_TrackGame = reinterpret_cast<TrackGame*>(&second);
    if (ReadParticleRendererFlag() != 29 || ReadEcoRendererFlag() != 29)
        return 3;
    WriteEcoRendererFlag(43);
    if (ReadParticleRendererFlag() != 43 || second.hidden != 43 || first.hidden != 31)
        return 4;

    g_TrackGame = reinterpret_cast<TrackGame*>(&first);
    if (ReadParticleRendererFlag() != 31 || ReadEcoRendererFlag() != 31)
        return 5;
    puts("TrackGame views share writes and observe pointer reassignment");
    return 0;
}
