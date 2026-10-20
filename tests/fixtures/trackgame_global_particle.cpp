#include "TrackGameGlobal.h"
#include "effects/ParticleManager.h"

int ReadParticleRendererFlag()
{
    return reinterpret_cast<ParticleGameContext*>(g_TrackGame)->hidden;
}

void WriteParticleRendererFlag(int value)
{
    reinterpret_cast<ParticleGameContext*>(g_TrackGame)->hidden = value;
}
