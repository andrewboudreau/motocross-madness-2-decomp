// Bring-up glue (see Group3_QuarryLoadLinks.h): the BikeRace and overlay calls made by
// BaseQuarryEvent::LoadEventObjects (0x004de590).
#include "../reconstructed/BikeRaceCore.h"
#include "../reconstructed/TrackOverlay.h"
#include "../reconstructed/Krusty3DObjects.h"
#include "../reconstructed/DebugAlloc.h"
#include "Group3_QuarryLoadLinks.h"

// 0x00417c30 (0x400 bytes).
void* G3Q_NewBikeRace(const char* file, int line, int flags)
{
    return new(file, line) BikeRace(flags);
}

// 0x00417ed0: returns the race, or 0 after Release.
void* G3Q_BikeRaceCreate(void* race, void* target, void* particles, void* lights,
                         void* textures, void* terrain, void* camera, void* scene,
                         void* event, void* shadow)
{
    return ((BikeRace*)race)->Create(target, particles, (LightManager*)lights,
                                     (UnknownTextureFormatChoice*)textures,
                                     (UnknownBikeRaceProjector*)terrain, (KrustyBikeCamera*)camera,
                                     (Scene*)scene, (UnknownBikeRaceNodeOwner*)event,
                                     (UnknownBikeRaceShadow*)shadow);
}

void* G3Q_BikeRaceNextBike(void* race, int* iterator)
{
    return ((BikeRace*)race)->NextBike(iterator);
}

void G3Q_BikeRaceSetDetailLevel(void* race, int level)
{
    ((BikeRace*)race)->SetDetailLevel(level);
}

void G3Q_StatsOverlaySetSource(void* stats, void* race)
{
    ((StatsOverlay*)stats)->SetStatsSource((int)race);
}

void G3Q_ChatOverlayCollectRacers(void* chat, void* race)
{
    ((ChatOverlay*)chat)->CollectRacers((UnknownChatView*)race);
}

void G3Q_RadarOverlayTakeRacers(void* radar, void* race, void* chat)
{
    ((RadarOverlay*)radar)->TakeRacers((UnknownKrustyBikeView*)race, (int)chat);
}

void G3Q_VisualCueCollectRacers(void* cue, void* race)
{
    ((VisualCue*)cue)->CollectRacers((UnknownVisualCueView*)race);
}
