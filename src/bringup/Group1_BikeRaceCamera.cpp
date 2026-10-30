// Group1_BikeRaceCamera.cpp -- bring-up bodies for three BikeRace methods that
// reconstructed code calls but src/ does not define yet.  The bodies are taken
// unchanged from samples/race/BikeRaceNearMisses.cpp (near misses; every branch
// decoded against retail, differences are register/scheduling only, see that file's
// header):
//   0x0041eb20 BikeRace::AimCamera            (1708 bytes, 1699/1710 positions)
//   0x0041f1d0 BikeRace::NextCameraTarget     (881 bytes; branches and calls as retail)
//   0x004210f0 BikeRace::WriteRelativePositions (2981 bytes; all cases decoded)
// AimCamera is included because NextCameraTarget calls it and nothing else defines it.
// The sample compiles them by including BikeRace.cpp; here the same headers and the
// TU-local helpers they use are repeated instead.

#include "../krusty2/broadphase/Terrain.h"
#include "GUIManagerCore.h"
#include "../krusty2/motion/D3DIMSoultreeCharacter.h"
#include "BikeRace.h"
#include "TrackOverlay.h"

#include <stdio.h>
#include <string.h>

#include "ArcadeObject.h"
#include "D3DIMSoulTree.h"
#include "DebugAlloc.h"
#include "EventManager.h"
#include "BaseQuarryEvent.h"
#include "../krusty2/vehicle/KrustyBikeCore.h"

#include <math.h>
#include <stdlib.h>

#include "CarProcedural.h"
#include "Krusty3DObjects.h"
#include "MemTag.h"
#include "OptionProcs.h"
#include "RaceSound.h"
#include "RaceStatus.h"
#include "SelectGamePicProcs.h"
#include "TextureMap.h"

// BikeRace.cpp's TU-local vector helpers.
static inline Vector3 operator+(const Vector3& a, const Vector3& b) {
    return Vector3(a.x + b.x, a.y + b.y, a.z + b.z);
}

static inline Vector3 operator-(const Vector3& a, const Vector3& b) {
    return Vector3(a.x - b.x, a.y - b.y, a.z - b.z);
}

// 0x0041eb20. Left: the caster arm loads targetCaster then raceScene as
// retail does, the entry arm loads targetEntry and raceScene in the other
// order. Splitting the two arms' targetKind/targetIndex stores into their
// own `if (caster) ... else ...` ahead of the bodies flips both arms (the
// entry arm then matches, the caster arm does not); splitting one arm,
// `!caster` tests, ternary stores or one store per split leave it or worse.
int BikeRace::AimCamera(int caster) {
    if (UnknownFunctionCameraView()->followingRacer) {
        targetKind = 0xff;
        targetIndex = 0;
        if (g_TrackGame->playerCount > 1) {
            UnknownFunctionCameraView()->followedBike->modelNode->ClearFlag14c();
            UnknownFunctionCameraView()->followedBike->riderCharacter->modelNode->ClearFlag14c();
            UnknownFunctionCameraView()->UnknownFunctionFollow(racerSlots[targetRacer]);
            if (raceShadow != 0) {
                raceShadow->ClearCasters();
                raceShadow->AddCaster(racerSlots[targetRacer]->modelNode);
                raceShadow->AddCaster(racerSlots[targetRacer]->riderCharacter->modelNode);
            }
            if (g_TrackGame->bajaEnduroEvent != 0) {
                UnknownBikeRaceRacer* racer = racerSlots[targetRacer];
                g_TrackGame->bajaEnduroEvent->SelectLapGates(racer->lapGate,
                                                                          racer->nextLapGate, racer);
            }
        } else if (targetRacer == 0) {
            UnknownFunctionCameraView()->followedBike->modelNode->ClearFlag14c();
            UnknownFunctionCameraView()->followedBike->riderCharacter->modelNode->ClearFlag14c();
            UnknownFunctionCameraView()->UnknownFunctionFollow(localRacer);
            if (raceShadow != 0) {
                raceShadow->ClearCasters();
                raceShadow->AddCaster(localRacer->modelNode);
                raceShadow->AddCaster(localRacer->riderCharacter->modelNode);
            }
            if (g_TrackGame->bajaEnduroEvent != 0) {
                UnknownBikeRaceRacer* racer = localRacer;
                g_TrackGame->bajaEnduroEvent->SelectLapGates(racer->lapGate,
                                                                          racer->nextLapGate, racer);
            }
        } else {
            UnknownFunctionCameraView()->followedBike->modelNode->ClearFlag14c();
            UnknownFunctionCameraView()->followedBike->riderCharacter->modelNode->ClearFlag14c();
            int index = targetRacer - 1;
            UnknownFunctionCameraView()->UnknownFunctionFollow(aiRacers[index]);
            if (raceShadow != 0) {
                raceShadow->ClearCasters();
                raceShadow->AddCaster(aiRacers[index]->modelNode);
                raceShadow->AddCaster(aiRacers[index]->riderCharacter->modelNode);
            }
            if (g_TrackGame->bajaEnduroEvent != 0) {
                UnknownBikeRaceRacer* racer = aiRacers[index];
                g_TrackGame->bajaEnduroEvent->SelectLapGates(racer->lapGate,
                                                                          racer->nextLapGate, racer);
            }
        }
    } else {
    if (caster) {
        targetKind = 0;
        targetIndex = targetCaster;
        if (raceScene->staticModels != 0 && raceScene->staticModels->casterCount > 0) {
            if (raceShadow != 0) {
                raceShadow->ClearCasters();
            }
            UnknownFunctionCameraView()->UnknownFunctionTargetCaster(
                raceScene->staticModels->casters[targetCaster].model);
        } else {
            UnknownFunctionCameraView()->followingRacer = 1;
        }
    } else {
        targetKind = 1;
        targetIndex = targetEntry;
        if (raceScene->animationTable != 0 && raceScene->animationTable->entryCount != 0) {
            if (raceShadow != 0) {
                raceShadow->ClearCasters();
            }
            UnknownSceneEntry* entry = &raceScene->animationTable->entries[targetEntry];
            if (entry->keyFramed) {
                UnknownBikeRaceSceneCharacter* character =
                    (UnknownBikeRaceSceneCharacter*)entry->character;
                if (character->isEnabled) {
                    UnknownFunctionCameraView()->UnknownFunctionTargetCharacter(character);
                } else {
                    return 0;
                }
            } else {
                GameObject* object = (GameObject*)entry->proceduralObject;
                if (((UnknownBikeRaceObjectFlags*)object)->isEnabled) {
                    UnknownFunctionCameraView()->UnknownFunctionTargetObject(object);
                } else {
                    return 0;
                }
            }
        } else {
            UnknownFunctionCameraView()->followingRacer = 1;
        }
    }
    if (UnknownFunctionCameraView()->followingRacer) {
        targetKind = 0xff;
        targetIndex = 0;
    }
    }
    return 1;
}

// Inline: racer `index` in camera order (own racer first, then the AI or
// remote racers).
inline UnknownBikeRaceRacer* BikeRace::UnknownFunctionRacerAt(int index) {
    int players = g_TrackGame->playerCount;
    if (players == 1 && index == 0) {
        return localRacer;
    }
    if (players > 1 || index < players) {
        return racerSlots[index];
    }
    return aiRacers[index - players];
}

// 0x0041f1d0
void BikeRace::NextCameraTarget(int forward, int racers, int objects) {
    int caster;
    if (g_TrackGame->uiInteractionBlocked) {
        return;
    }
    UnknownBikeRaceCameraView* camera = UnknownFunctionCameraView();
    if ((camera->overrideActive || camera->cameraState == 7) &&
        (camera->cameraState != 7 || racers || objects)) {
        return;
    }
    if (camera->followingRacer != racers) {
        camera->followingRacer = racers;
        if (AimCamera(objects == 0)) {
            return;
        }
    }
    if (racers) {
        caster = 0;
        int others;
        if (g_TrackGame->playerCount > 1) {
            others = g_TrackGame->networkAiRacerCount;
        } else if (g_TrackGame->mode.raceSettings.raceMode == 4) {
            others = 1;
        } else {
            others = g_TrackGame->mode.raceSettings.aiRacerCount;
        }
        if (g_TrackGame->playerCount == 1 && others == 0) {
            return;
        }
        int old = targetRacer;
        do {
            if (forward) {
                targetRacer++;
            } else {
                targetRacer--;
            }
            if (targetRacer < 0) {
                targetRacer = g_TrackGame->playerCount + others - 1;
            }
            if (targetRacer >= g_TrackGame->playerCount + others) {
                targetRacer = 0;
            }
            if (old == targetRacer) {
                UnknownFunctionCameraView()->followingRacer = 1;
                return;
            }
        } while (UnknownFunctionRacerAt(targetRacer)->outOfRace != 0);
    } else if (objects) {
        caster = 0;
        Scene* scene = raceScene;
        if (scene->animationTable == 0 || scene->animationTable->entryCount <= 0) {
            UnknownFunctionCameraView()->followingRacer = 1;
            return;
        }
        int hidden = 1;
        int old = targetEntry;
        do {
            if (forward) {
                targetEntry++;
            } else {
                targetEntry--;
            }
            if (targetEntry < 0) {
                targetEntry = scene->animationTable->entryCount - 1;
            }
            if (targetEntry >= scene->animationTable->entryCount) {
                targetEntry = 0;
            }
            UnknownSceneEntry* entry = &scene->animationTable->entries[targetEntry];
            if (entry->keyFramed) {
                if (((UnknownBikeRaceSceneCharacter*)entry->character)->isEnabled) {
                    hidden = entry->excludeFromCameraIteration;
                } else {
                    hidden = 1;
                }
            } else if (((UnknownBikeRaceObjectFlags*)entry->proceduralObject)->isEnabled) {
                hidden = entry->excludeFromCameraIteration;
            } else {
                hidden = 1;
            }
            if (old == targetEntry) {
                UnknownBikeRaceCameraView* view = UnknownFunctionCameraView();
                if (view->targetCharacter != 0 || view->targetCar != 0) {
                    return;
                }
                entry = &scene->animationTable->entries[targetEntry];
                if (entry->keyFramed) {
                    if (!((UnknownBikeRaceSceneCharacter*)entry->character)->isEnabled) {
                        view->followingRacer = 1;
                        return;
                    }
                } else if (!((UnknownBikeRaceObjectFlags*)entry->proceduralObject)->isEnabled) {
                    view->followingRacer = 1;
                    return;
                }
                AimCamera(caster);
                return;
            }
        } while (hidden);
    } else {
        caster = 1;
        UnknownSceneBuffer* casters = raceScene->staticModels;
        if (casters == 0 || casters->casterCount <= 0) {
            UnknownFunctionCameraView()->followingRacer = 1;
            return;
        }
        int old = targetCaster;
        targetCaster = forward ? old + 1 : old - 1;
        if (targetCaster < 0) {
            targetCaster = raceScene->staticModels->casterCount - 1;
        }
        if (targetCaster >= raceScene->staticModels->casterCount) {
            targetCaster = 0;
        }
        if (old == targetCaster && UnknownFunctionCameraView()->shadowCaster != 0) {
            return;
        }
    }
    AimCamera(caster);
}


float FastInvSqrt(float x); // 0x00460c00 (FastMath)

static inline float DotProduct(const Vector3& a, const Vector3& b) {
    return a.x * b.x + a.y * b.y + a.z * b.z;
}

static inline Vector3& operator-=(Vector3& v, const Vector3& o) {
    v.x -= o.x;
    v.y -= o.y;
    v.z -= o.z;
    return v;
}

static inline float SquareMagnitude(const Vector3& v) {
    return DotProduct(v, v);
}

static inline Vector3 Scale(const Vector3& v, float s) {
    return Vector3(s * v.x, s * v.y, s * v.z);
}

float Vec3Dot(const Vector3* a, const Vector3* b); // 0x0040ae30: a.b

static inline Vector3 Normalize(const Vector3& v) {
    float length = SquareMagnitude(v);
    if (length == 1.0f) {
        return v;
    }
    float scale = FastInvSqrt(length);
    return Scale(v, scale);
}

// 0x004210f0
void BikeRace::WriteRelativePositions(Vector3* a, Vector3* b, void* reference, int count) {
    if (gridReady && count != 0) {
        *a = Scale(gridSlotStep, (float)count) + gridFirstSlot;
        *b = gridDirection;
        return;
    }
    switch (g_TrackGame->mode.raceSettings.eventType) {
    case 2:
    case 3: {
        if (hasStartProbe) {
            a->x = startProbe.center.x;
            a->z = startProbe.center.z;
            a->y = 0.0f;
            b->x = startProbe.direction.x;
            b->z = startProbe.direction.z;
            b->y = 0.0f;
        } else {
            *a = raceScene->defaultStartPosition;
            *b = Normalize(raceScene->defaultStartDirection);
        }
        if (count <= 0) {
            return;
        }
        float side;
        if (raceTrack->startNode != 0 && raceTrack->startNode->firstSegment != 0 &&
            raceTrack->startNode->firstSegment->next != 0 &&
            raceTrack->startNode->firstSegment->next->next != 0) {
            TrackVec3 p0;
            TrackVec3 p1;
            raceTrack->SegmentDirection(raceTrack->startNode->firstSegment, &p0);
            raceTrack->SegmentDirection(raceTrack->startNode->firstSegment->next, &p1);
            if (p1.x * p0.z - p0.x * p1.z <= 0.0f) {
                side = 1.0f;
            } else {
                side = -1.0f;
            }
        } else {
            side = -1.0f;
        }
        gridDirection = *b;
        *a -= Scale(*b, 4.66f);
        gridSlotStep = Vector3(b->z, 0.0f, -b->x);
        gridFirstSlot = *a - Scale(Scale(Scale(gridSlotStep, 6.75f), raceTerrain->gridCellSize), side);
        gridSlotStep = Scale(gridSlotStep, raceTerrain->gridCellSize * 15.5f / (racerCount + 1) * side);
        *a = Scale(gridSlotStep, (float)count) + gridFirstSlot;
        break;
    }
    case 0:
        *a = raceScene->defaultStartPosition;
        *b = Normalize(raceScene->defaultStartDirection);
        if (count <= 0) {
            return;
        }
        gridDirection = *b;
        *a -= Scale(*b, 4.66f);
        gridSlotStep = Vector3(b->z, 0.0f, -b->x);
        gridFirstSlot = *a - Scale(gridSlotStep, 20.0f);
        gridSlotStep = Scale(Vector3(b->z, 0.0f, -b->x), 20.0f);
        *a = Scale(gridSlotStep, (float)count) + gridFirstSlot;
        break;
    case 1:
    case 5: {
        UnknownBikeRaceNodeOwner* owner = (UnknownBikeRaceNodeOwner*)reference;
        Vector3 direction = owner->gateSources[owner->gateCount - 1].direction;
        direction.y = 0.0f;
        *b = Normalize(direction);
        *a = owner->gateSources[owner->gateCount - 1].center;
        *a += Scale(*b, 8.0f);
        a->y = 0.0f;
        if (count <= 0) {
            return;
        }
        float width = owner->gateEntries[owner->gateCount - 1]->width * 1.05f;
        float spacing = width / (racerCount + 1);
        if (3.4f > spacing) {
            spacing = 3.4f;
        }
        gridDirection = *b;
        gridSlotStep = Vector3(b->z, 0.0f, -b->x);
        gridFirstSlot = *a - Scale(gridSlotStep, width * 0.48f);
        gridSlotStep = Scale(Vector3(b->z, 0.0f, -b->x), spacing);
        *a = Scale(gridSlotStep, (float)count) + gridFirstSlot;
        break;
    }
    case 4:
        *a = raceScene->defaultStartPosition;
        *b = Normalize(raceScene->defaultStartDirection);
        if (count <= 0) {
            return;
        }
        gridDirection = *b;
        *a -= Scale(*b, 4.66f);
        gridSlotStep = Vector3(b->z, 0.0f, -b->x);
        gridFirstSlot = *a - Scale(gridSlotStep, 20.0f);
        gridSlotStep = Scale(Vector3(b->z, 0.0f, -b->x), 20.0f);
        *a = gridFirstSlot + Scale(gridSlotStep, (float)count);
        break;
    default:
        return;
    }
    gridReady = true;
}
