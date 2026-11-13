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
    if (RaceCameraView()->followingRacer) {
        targetKind = 0xff;
        targetIndex = 0;
        if (g_TrackGame->playerCount > 1) {
            RaceCameraView()->followedBike->modelNode->ClearFlag14c();
            RaceCameraView()->followedBike->riderCharacter->modelNode->ClearFlag14c();
            RaceCameraView()->UnknownFunctionFollow(racerSlots[targetRacer]);
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
            RaceCameraView()->followedBike->modelNode->ClearFlag14c();
            RaceCameraView()->followedBike->riderCharacter->modelNode->ClearFlag14c();
            RaceCameraView()->UnknownFunctionFollow(localRacer);
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
            RaceCameraView()->followedBike->modelNode->ClearFlag14c();
            RaceCameraView()->followedBike->riderCharacter->modelNode->ClearFlag14c();
            int index = targetRacer - 1;
            RaceCameraView()->UnknownFunctionFollow(aiRacers[index]);
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
            RaceCameraView()->UnknownFunctionTargetCaster(
                raceScene->staticModels->casters[targetCaster].model);
        } else {
            RaceCameraView()->followingRacer = 1;
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
                    RaceCameraView()->UnknownFunctionTargetCharacter(character);
                } else {
                    return 0;
                }
            } else {
                GameObject* object = (GameObject*)entry->proceduralObject;
                if (((UnknownBikeRaceObjectFlags*)object)->isEnabled) {
                    RaceCameraView()->UnknownFunctionTargetObject(object);
                } else {
                    return 0;
                }
            }
        } else {
            RaceCameraView()->followingRacer = 1;
        }
    }
    if (RaceCameraView()->followingRacer) {
        targetKind = 0xff;
        targetIndex = 0;
    }
    }
    return 1;
}

// Inline: racer `index` in camera order (own racer first, then the AI or
// remote racers). The player count is read at each test: retail re-tests
// `playerCount > 1` after the `index == 0` miss (`cmp ecx, 1` twice at
// 0x0041f2f6/0x0041f304), which a `players` local lets VC6 thread away.
inline UnknownBikeRaceRacer* BikeRace::UnknownFunctionRacerAt(int index) {
    if (g_TrackGame->playerCount == 1 && index == 0) {
        return localRacer;
    }
    if (g_TrackGame->playerCount > 1 || index < g_TrackGame->playerCount) {
        return racerSlots[index];
    }
    return aiRacers[index - g_TrackGame->playerCount];
}

// 0x0041f1d0. Open gap: retail keeps 0 in edx through the body and has three
// AimCamera call copies. Ruled out (sweep, strict/aligned): `break` to one
// shared AimCamera after the objects wrap (879 bytes, strict 209 against 297),
// `caster = 0` once at the top instead of per branch (906-915 bytes),
// `uiInteractionBlocked != 0`, `!objects` and `objects` reused as the caster
// variable (all inert).
void BikeRace::NextCameraTarget(int forward, int racers, int objects) {
    int caster;
    if (g_TrackGame->uiInteractionBlocked) {
        return;
    }
    UnknownBikeRaceCameraView* camera = RaceCameraView();
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
                RaceCameraView()->followingRacer = 1;
                return;
            }
        } while (UnknownFunctionRacerAt(targetRacer)->outOfRace != 0);
    } else if (objects) {
        caster = 0;
        Scene* scene = raceScene;
        if (scene->animationTable == 0 || scene->animationTable->entryCount <= 0) {
            RaceCameraView()->followingRacer = 1;
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
                UnknownBikeRaceCameraView* view = RaceCameraView();
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
            RaceCameraView()->followingRacer = 1;
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
        if (old == targetCaster && RaceCameraView()->shadowCaster != 0) {
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

float Vec3Dot(const Vector3* a, const Vector3* b); // 0x0040ae30: (a.y*b.y + a.x*b.x) + a.z*b.z
// 0x005015b0 (cdecl, hidden result pointer): v * s; linked to Vec3ScaleCall.
Vector3 Vec3ScaleCall(const Vector3& v, float s);

// Retail calls the out-of-line dot 0x0040ae30 and scale 0x005015b0 here: the length is
// summed (y + x) + z, and the inverse square root is rounded to float as the scale's
// argument. The TU-local DotProduct sums (z + y) + x, and an inline Scale multiplies by
// the unrounded FastInvSqrt result (emu_diff case bikerace_write_relative_positions).
static inline Vector3 Normalize(const Vector3& v) {
    float length = Vec3Dot(&v, &v);
    if (length == 1.0f) {
        return v;
    }
    return Vec3ScaleCall(v, FastInvSqrt(length));
}

// Vector3::Vector3(float, float, float), 0x00404e60: the out-of-line copy retail
// calls at most grid sites below (its float arguments round each component);
// linked to Vec3Call's constructor (the same body).
struct Vector3Call : Vector3 {
    Vector3Call(float x_, float y_, float z_);
};

// The helpers at the sites where retail constructs through 0x00404e60. The
// inline forms let VC6 keep a product in a register where retail rounds it.
static inline Vector3 ScaleCall(const Vector3& v, float s) {
    return Vector3Call(s * v.x, s * v.y, s * v.z);
}

static inline Vector3 AddCall(const Vector3& a, const Vector3& b) {
    return Vector3Call(a.x + b.x, a.y + b.y, a.z + b.z);
}

static inline Vector3 SubCall(const Vector3& a, const Vector3& b) {
    return Vector3Call(a.x - b.x, a.y - b.y, a.z - b.z);
}

// 0x00421cb0 / 0x00421d00 (cdecl, hidden result pointer): the out-of-line Vector3
// operator+ and operator- event 4 calls; linked to Vec3AddCall / Vec3SubtractCall.
Vector3 BikeRaceAddCall(const Vector3& a, const Vector3& b);
Vector3 BikeRaceSubtractCall(const Vector3& a, const Vector3& b);

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
        {
            // Scaled in place through a reference: retail multiplies each
            // component at [eax], [eax + 4], [eax + 8] (0x00421453).
            float step = raceTerrain->gridCellSize * 15.5f / (racerCount + 1) * side;
            Vector3& slotStep = gridSlotStep;
            slotStep.x *= step;
            slotStep.y *= step;
            slotStep.z *= step;
        }
        *a = AddCall(ScaleCall(gridSlotStep, (float)count), gridFirstSlot);
        break;
    }
    case 0:
        *a = raceScene->defaultStartPosition;
        *b = Normalize(raceScene->defaultStartDirection);
        if (count <= 0) {
            return;
        }
        gridDirection = *b;
        *a -= ScaleCall(*b, 4.66f);
        gridSlotStep = Vector3(b->z, 0.0f, -b->x);
        gridFirstSlot = SubCall(*a, ScaleCall(gridSlotStep, 20.0f));
        gridSlotStep = ScaleCall(Vector3(b->z, 0.0f, -b->x), 20.0f);
        *a = AddCall(ScaleCall(gridSlotStep, (float)count), gridFirstSlot);
        break;
    case 1:
    case 5: {
        RaceGateEvent* owner = (RaceGateEvent*)reference;
        Vector3 direction = owner->gateSources[owner->gateCount - 1].direction;
        direction.y = 0.0f;
        *b = Normalize(direction);
        *a = owner->gateSources[owner->gateCount - 1].center;
        *a += ScaleCall(*b, 8.0f);
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
        gridFirstSlot = SubCall(*a, ScaleCall(gridSlotStep, width * 0.48f));
        gridSlotStep = ScaleCall(Vector3(b->z, 0.0f, -b->x), spacing);
        *a = AddCall(ScaleCall(gridSlotStep, (float)count), gridFirstSlot);
        break;
    }
    case 4:
        *a = raceScene->defaultStartPosition;
        *b = Normalize(raceScene->defaultStartDirection);
        if (count <= 0) {
            return;
        }
        gridDirection = *b;
        *a -= ScaleCall(*b, 4.66f);
        gridSlotStep = Vector3(b->z, 0.0f, -b->x);
        gridFirstSlot = BikeRaceSubtractCall(*a, ScaleCall(gridSlotStep, 20.0f));
        gridSlotStep = Vec3ScaleCall(Vector3Call(b->z, 0.0f, -b->x), 20.0f);
        *a = BikeRaceAddCall(gridFirstSlot, Vec3ScaleCall(gridSlotStep, (float)count));
        break;
    default:
        return;
    }
    gridReady = true;
}
