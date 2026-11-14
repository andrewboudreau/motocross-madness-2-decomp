#include "EcoSystem.h"

int ReadEcoRendererFlag()
{
    return reinterpret_cast<EcoTrackGameView*>(g_TrackGame)->bladeRenderer;
}

void WriteEcoRendererFlag(int value)
{
    reinterpret_cast<EcoTrackGameView*>(g_TrackGame)->bladeRenderer = value;
}
