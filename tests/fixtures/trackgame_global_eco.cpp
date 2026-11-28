#include "TrackGame.h"
#include "EcoSystem.h"

// EcoSystem.cpp reads the renderer flag through the complete class.
int ReadEcoRendererFlag()
{
    return g_TrackGame->bladeRenderer;
}

void WriteEcoRendererFlag(int value)
{
    g_TrackGame->bladeRenderer = value;
}
