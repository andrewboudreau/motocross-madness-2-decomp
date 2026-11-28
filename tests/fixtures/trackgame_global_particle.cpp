#include "TrackGameGlobal.h"
#include "TrackGame.h"
#include "effects/ParticleManager.h"

int ReadParticleRendererFlag()
{
    return g_TrackGame->bladeRenderer;
}

void WriteParticleRendererFlag(int value)
{
    g_TrackGame->bladeRenderer = value;
}
