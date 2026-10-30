// Bring-up glue for Group3_QuarryLoad.cpp (BaseQuarryEvent::LoadEventObjects, 0x004de590).
//
// The loader body comes from samples/game/QuarryStuntEventLoaderNearMiss.cpp, which reads
// the objects it creates through local UnknownQuarry* views. The canonical classes live in
// headers that cannot share one translation unit (EcoSystem.h, QuarryEvent.h and
// krusty2/collision each declare their own CollisionObject/VisualCue/EcoSystem views), so
// each canonical call is made from a small Group3_QuarryLink*.cpp file through these
// opaque helpers. Every helper is a direct forward of one retail call made by the loader;
// the retail callee address is noted with each.
#ifndef MCM2_BRINGUP_GROUP3_QUARRYLOADLINKS_H
#define MCM2_BRINGUP_GROUP3_QUARRYLOADLINKS_H

// Group3_QuarryLinkCamera.cpp
void* G3Q_NewBikeCamera(const char* file, int line, int flags);                 // 0x00497cb0
void* G3Q_BikeCameraSetupFlyby(void* camera, void* target, float a, float b, float c,
                               float d, float e, int capacity, int count,
                               const int* presets);                              // 0x00497d90
void G3Q_BikeCameraSetSubject(void* camera, void* subject);                      // 0x00463600

// Group3_QuarryLinkKrusty.cpp
void* G3Q_NewParticleManager(const char* file, int line, int flags);            // 0x004ba390
void* G3Q_NewVisibilityQuadTree(const char* file, int line, int flags);         // 0x0052d340
void* G3Q_VisibilitySetup(void* tree, void* target, float x0, float z0, float x1, float z1,
                          float minCell);                                        // 0x0052d460 (GameObject part)
void* G3Q_NewProjectedShadow(const char* file, int line, int flags);            // 0x004da570
void* G3Q_ProjectedShadowInit(void* shadow, void* target, void* textureManager); // 0x004da7b0
void G3Q_ProjectedShadowAddCaster(void* shadow, void* caster);                   // 0x004dab00
void G3Q_ProjectedShadowSetStrength(void* shadow, int strength);                 // 0x004dace0
void G3Q_ProjectedShadowSetGrey(void* shadow, int level);                        // 0x004dae30
void G3Q_ProjectedShadowSetLight(void* shadow, const void* position, const void* direction,
                                 int mode);                                      // 0x004dae50
void* G3Q_NewTerrainShadow(const char* file, int line, int flags);              // 0x00508ae0
void* G3Q_TerrainShadowAttach(void* terrainShadow, void* target, void* terrain,
                              void* shadow);                                     // 0x00508b80
void* G3Q_NewTerrain(const char* file, int line, int flags);                    // 0x00505830
void* G3Q_TerrainLoad(void* terrain, void* target, void* stream, int unused, void* textures,
                      int quality, int field0xc40, const char* detailTexture,
                      int halfFormat);                                           // 0x005059d0
void* G3Q_NewQuadTreeRenderer(const char* file, int line, int flags);           // 0x00504490
void G3Q_QuadTreeRendererInsertNodes(void* renderer);                            // 0x005046e0
void G3Q_CollisionSnapPlacement(void* collision);                                // 0x00435fe0
void G3Q_CollisionSetUseBroadphase(void* collision, int enable);                 // 0x00432120
void G3Q_CollisionSetIgnoreListMode(void* collision, int mode);                  // 0x00439400
void G3Q_CollisionAddIgnoredOwner(void* collision, void* owner);                 // 0x00439410
void G3Q_CollisionSetOwner(void* collision, void* owner, int tag);               // 0x004394d0

// Group3_QuarryLinkEco.cpp
void* G3Q_NewEcoSystem(const char* file, int line, int flags);                  // 0x00457250
void* G3Q_EcoSystemCreate(void* eco, void* target, void* textureManager, void* terrain,
                          void* lights, void* environment, void* stream, int format,
                          int collisions, int level);                            // 0x004594d0
void* G3Q_NewSkyCube(const char* file, int line, int flags);                    // 0x004fb1e0
void* G3Q_SkyCubeCreate(void* sky, void* target, void* stream, void* textures,
                        float size);                                             // 0x004fb230

// Group3_QuarryLinkRace.cpp
void* G3Q_NewBikeRace(const char* file, int line, int flags);                   // 0x00417c30
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
