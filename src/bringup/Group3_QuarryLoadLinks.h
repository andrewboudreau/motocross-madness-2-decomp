// Bring-up glue for Group3_QuarryLoad.cpp (BaseQuarryEvent::LoadEventObjects, 0x004de590).
//
// The loader body comes from samples/game/QuarryStuntEventLoaderNearMiss.cpp, which reads
// the objects it creates through local UnknownQuarry* views. The canonical classes live in
// headers that cannot share one translation unit (EcoSystem.h, QuarryEvent.h and
// krusty2/collision each declare their own CollisionObject/VisualCue/EcoSystem views), so
// each canonical call is made from a small Group3_QuarryLink*.cpp file through these
// opaque helpers. Every helper is a direct forward of one retail call made by the loader;
// the retail callee address is noted with each. The loader allocates its objects inline
// through the canonical constructors, so no allocation helpers remain.
#ifndef MCM2_BRINGUP_GROUP3_QUARRYLOADLINKS_H
#define MCM2_BRINGUP_GROUP3_QUARRYLOADLINKS_H

// Group3_QuarryLinkCamera.cpp
void* G3Q_BikeCameraSetupFlyby(void* camera, void* target, float a, float b, float c,
                               float d, float e, int capacity, int count,
                               const int* presets);                              // 0x00497d90
void G3Q_BikeCameraSetSubject(void* camera, void* subject);                      // 0x00463600

// Group3_QuarryLinkKrusty.cpp
void G3Q_QuadTreeRendererInsertNodes(void* renderer);                            // 0x005046e0

// Group3_QuarryLinkRace.cpp
void* G3Q_BikeRaceCreate(void* race, void* target, void* particles, void* lights,
                         void* textures, void* terrain, void* camera, void* scene,
                         void* event, void* shadow);                             // 0x00417ed0
void* G3Q_BikeRaceNextBike(void* race, int* iterator);                           // 0x004204e0
void G3Q_BikeRaceSetDetailLevel(void* race, int level);                          // 0x00423790
void G3Q_StatsOverlaySetSource(void* stats, void* race);                         // 0x00519880
void G3Q_ChatOverlayCollectRacers(void* chat, void* race);                       // 0x0051d730
void G3Q_RadarOverlayTakeRacers(void* radar, void* race, void* chat);            // 0x0051baf0
void G3Q_VisualCueCollectRacers(void* cue, void* race);                          // 0x0048af20

#endif
