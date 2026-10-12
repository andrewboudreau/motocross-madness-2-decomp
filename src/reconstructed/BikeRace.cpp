#include "BikeRace.h"

#include <stdio.h>
#include <string.h>

#include "ArcadeObject.h"
#include "DebugAlloc.h"

// A truncating copy into a `size`-byte buffer (as DlgProcs.cpp); retail
// evaluates `source` once for the length and again for the copy.
#define COPY_TEXT(dest, source, size)                              \
    {                                                              \
        int length = strlen(source);                               \
        int copied = length > (size) - 1 ? (size) - 1 : length;    \
        strncpy(dest, source, copied);                             \
        (dest)[copied] = 0;                                        \
    }

// The four vector constants of src/krusty2/math/Math3D.h: 0x00578e60,
// 0x00578e70, 0x00578e80 and 0x00578e50, initialised by
// 0x0041cdf0..0x0041cf2b.
static const Vector3 kVec3Zero = Vector3(0.0f, 0.0f, 0.0f);
static const Vector3 kVec3XAxis = Vector3(1.0f, 0.0f, 0.0f);
static const Vector3 kVec3YAxis = Vector3(0.0f, 1.0f, 0.0f);
static const Vector3 kVec3ZAxis = Vector3(0.0f, 0.0f, 1.0f);

// 0x00578e8c (.bss, after the vectors): the replay recorder's last tick
// time (0x00421d50).
static float s_UnknownStatic578e8c;

static inline Vector3 operator+(const Vector3& a, const Vector3& b) {
    return Vector3(a.x + b.x, a.y + b.y, a.z + b.z);
}

static inline Vector3 operator-(const Vector3& a, const Vector3& b) {
    return Vector3(a.x - b.x, a.y - b.y, a.z - b.z);
}

// 0x00417b00
void PickerHitCallback(CollisionObject* self, CollisionObject* other) {
    BikeRace* race = (BikeRace*)g_TrackGame->eventManager->FindRaceView();
    if (race != 0 && race->objectPicker != 0) {
        race->objectPicker->field_0x2c = (int)other;
    }
}

// 0x00417b30
void SortDescending(int* keys, int* values, int count) {
    int swapped = 1;
    for (int pass = 0; pass <= count && swapped; pass++) {
        swapped = 0;
        for (int i = 0; i < count - 1 - pass; i++) {
            if (keys[i] < keys[i + 1]) {
                int key = keys[i];
                keys[i] = keys[i + 1];
                keys[i + 1] = key;
                int value = values[i];
                values[i] = values[i + 1];
                swapped = 1;
                values[i + 1] = value;
            }
        }
    }
}

// 0x00417bc0
void BikeRace::RankByPoints(int* order) {
    int points[11];
    for (int i = 0; i < racerCount; i++) {
        points[i] = g_TrackGame->eventManager->results[i].points;
        order[i] = i + 1;
    }
    SortDescending(points, order, racerCount);
}

// 0x00417c30
BikeRace::BikeRace(int flags) : GraphicsTest(flags) {
    startProbe.nextGate = 0;
    finishProbe.nextGate = 0;
    ghostLap = 0;
    field_0x078 = 0;
    localRacer = 0;
    racerSlots = 0;
    aiRacers = 0;
    field_0x044 = 0;
    field_0x04c = 0;
    raceCamera = 0;
    field_0x054 = 0;
    raceScene = 0;
    field_0x05c = 0;
    youIndicator = 0;
    scoreBoard = 0;
    digitCounter = 0;
    raceTrack = 0;
    gateList = 0;
    targetRacer = 0;
    targetEntry = 0;
    targetCaster = 0;
    startCountdown = 0;
    isRacing = false;
    restartPending = false;
    trackLoaded = false;
    racerCount = 0;
    gridReady = false;
    startStampSent = false;
    if (g_TrackGame->playerCount > 1) {
        networkStartPending = true;
    } else {
        networkStartPending = false;
    }
    chatOpen = false;
    chatHideTimer = 0;
    field_0x198 = g_TrackGame->mode.chatInputLine;
    startGate = 0;
    thirtySecondMan = 0;
    flagGirl = 0;
    startGateEntry = -1;
    thirtySecondManEntry = -1;
    field_0x189 = 0;
    startGateTimer = 0;
    recordTickTime = 0;
    recordTickCount = 0;
    field_0x1b4 = 0;
    replayLength = 0;
    replayTime = 0;
    replayLengthFrames = 0;
    replayFrames = 0;
    bestGhostLength = FLT_MAX;
    replayVcr = 0;
    ghostVcr = 0;
    vcrFile = 0;
    g_TrackGame->replaySpeed = 1.0f;
    replayStepFrame = false;
    ghostTimeLimit = (float)g_TrackGame->GetRegistryInt("VCRGhostTimeLimit", 300);
    recordTimeLimit = (float)g_TrackGame->GetRegistryInt("VCRRecordTimeLimit", 3600);
    recorderTickElapsed = 0;
    replayPaused = false;
    replayStepFrame = false;
    replayMode = 4;
    appliedReplayMode = 4;
    field_0x1e4 = 0;
    replaySeekTarget = -1.0f;
    recorderTickDue = false;
    field_0x3fb = false;
    isRecording = 0;
    ghostFilePlayed[0] = 0;
    ghostFileRecorded[0] = 0;
    g_TrackGame->paused = 0;
    hasStartProbe = 0;
    hasFinishProbe = 0;
    chatOverlay = 0;
    forceHighLod = g_TrackGame->GetRegistryFlag("ForceHighLOD", 0);
    field_0x3fd = 1;
    lodDebugPage = -1;
    targetKind = 0xff;
    targetIndex = 0;
    objectPicker = 0;
    field_0x034 = 0;
}

// 0x0041cf30
BikeRace::~BikeRace() {
    if (racerSlots != 0) {
        delete racerSlots;
        racerSlots = 0;
    }
    if (aiRacers != 0) {
        if (g_TrackGame->InProCircuitCareer()) {
            DebugFree(aiRacers, __FILE__, 0x809);
        } else {
            delete aiRacers;
        }
        aiRacers = 0;
    }
    Track* track = raceTrack;
    if (track != 0) {
        if (track->startNode != 0) {
            track->FreeReachableNodes(&track->startNode);
        }
        delete track;
    }
    UnknownFunctionFreeNodes();
    if (field_0x0c4 != 0) {
        FreeStatusList(field_0x0c4);
        field_0x0c4 = 0;
    }
    if (vcrFile != 0) {
        delete vcrFile;
        vcrFile = 0;
        if (g_TrackGame->playingReplay) {
            UnknownFunctionCameraView()->cameraState = savedCameraState;
            raceCamera->UnknownVirtualSlot61();
        }
        g_TrackGame->playingReplay = 0;
        g_TrackGame->useReplayTape = 0;
    }
    while (gateList != 0) {
        UnknownBikeRaceNode* node = gateList;
        gateList = node->nextGate;
        DebugFree(node, __FILE__, 0x82e);
    }
}

// 0x0041d260
int BikeRace::UnknownVirtualSlot16(int value) {
    GameObject::UnknownVirtualSlot16(value);
    JoystickDevice* joystick = g_TrackGame->controlInterface->activeJoystick;
    if (joystick != 0 && g_TrackGame->mode.joystickOn) {
        joystick->UnknownVirtualSlot18(value);
    }
    return 1;
}

// 0x0041d0d0
int BikeRace::AppendNode(int index, UnknownBikeRaceNodeSource* source,
                         UnknownBikeRaceNode*** tail, float scale, int value) {
    UnknownBikeRaceNode* node =
        (UnknownBikeRaceNode*)DebugCalloc(1, sizeof(UnknownBikeRaceNode), __FILE__, 0x840);
    if (node == 0) {
        return 0;
    }
    UnknownBikeRaceNodeSource* record = &source[index];
    node->center.x = record->field_0x00.x;
    node->center.y = record->field_0x00.y;
    node->center.z = record->field_0x00.z;
    node->direction.x = record->field_0x0c.x;
    node->direction.y = record->field_0x0c.y;
    node->direction.z = record->field_0x0c.z;
    node->field_0x24 = scale;
    node->field_0x28 = value;
    node->halfExtents.y = 0.0f;
    node->halfExtents.x = scale * node->direction.z * 0.5f;
    node->halfExtents.z = scale * node->direction.x * -0.5f;
    **tail = node;
    *tail = &node->nextGate;
    return 1;
}

// 0x0041d170
int BikeRace::BuildNodeList(UnknownBikeRaceNodeOwner* owner) {
    int count = owner->field_0x0ac;
    UnknownBikeRaceNodeSource* sources = owner->field_0x0d8;
    UnknownBikeRaceNode** tail = &gateList;
    for (int i = 0; i < count; i++) {
        UnknownBikeRaceNodeEntry* entry = owner->field_0x420[i];
        if (!AppendNode(i, sources, &tail, entry->field_0x50, entry->field_0x54)) {
            return 0;
        }
    }
    return 1;
}

// 0x0041d1e0
int BikeRace::UpdateTrackSeparation() {
    if (raceTrack != 0 && raceTrack->DistanceAlong(finishProbe.field_0x2c, startProbe.field_0x2c) > 0.0f) {
        field_0x189 = 0;
    } else {
        field_0x189 = 1;
    }
    return field_0x189;
}

// 0x0041d2a0
void BikeRace::SetSceneTime(float time) {
    raceScene->RestartAllEntries(time);
}

// Inline: the racers in the race, own and AI (a single player race) or
// own and remote.
static inline int UnknownFunctionRacerCount() {
    if (g_TrackGame->playerCount == 1) {
        if (g_TrackGame->mode.raceSettings.raceMode == 4) {
            return g_TrackGame->playerCount;
        }
        return g_TrackGame->mode.raceSettings.aiRacerCount + 1;
    }
    return g_TrackGame->mode.raceSettings.multiplayerAiRacerCount + g_TrackGame->playerCount;
}

static inline float UnknownFunctionMax(float a, float b) {
    return a > b ? a : b;
}

// 0x0041d2b0
int BikeRace::UnknownVirtualSlot10(float frameTime) {
    if (!g_TrackGame->uiInteractionBlocked) {
        if (UnknownFunctionCameraView()->followingRacer &&
            UnknownFunctionCameraView()->field_0x3b0->outOfRace != 0) {
            NextCameraTarget(0, 1, 0);
        }
        if (!g_TrackGame->paused) {
            if (g_TrackGame->mode.joystickOn) {
                localRacer->UnknownRacerVirtualSlot94();
            }
            if (g_TrackGame->mode.raceSettings.raceMode == 4) {
                unsigned short lap = localRacer->lapCount;
                if (ghostLap != lap && (!field_0x189 || lap > 0)) {
                    RestartReplay();
                    ghostLap = localRacer->lapCount;
                    replayTime = 0;
                    replayFrames = 0;
                    replayLength = 0;
                    replayLengthFrames = 0;
                    recordTickTime = 0;
                    recordTickCount = 0;
                    field_0x1b4 = 0;
                    localRacer->field_0x13f8 = FLT_MAX / 2;
                } else {
                    replayTime += g_TrackGame->frameTime;
                    replayFrames++;
                    if (replayLength != ghostTimeLimit) {
                        replayLength += g_TrackGame->frameTime;
                        replayLengthFrames++;
                    }
                    if (replayLength > ghostTimeLimit) {
                        replayLength = ghostTimeLimit;
                        TextQueueOverlay* overlay = g_TrackGame->eventManager->FindTextQueue();
                        char text[0x100];
                        g_TrackGame->LoadResourceString(0x14de, text, 0x80);
                        UnknownMessage* message = new (__FILE__, 0x8bf) UnknownMessage(text, 3.25f);
                        if (message != 0) {
                            overlay->ReplaceHeadMessage(message);
                            delete message;
                        }
                    } else {
                        recorderTickElapsed += g_TrackGame->frameTime;
                        recorderTickDue = recorderTickElapsed >= g_TrackGame->shortRecordPacketIntervalSeconds;
                        if (recorderTickDue) {
                            recorderTickElapsed -= g_TrackGame->shortRecordPacketIntervalSeconds;
                            recordTickTime = replayLength;
                            recordTickCount++;
                            field_0x1b4 = 1;
                            if (replayVcr != 0) {
                                replayVcr->QueueRecord(-1, 0, &recordTickTime, 0xc);
                            }
                        }
                    }
                }
            }
            if (g_TrackGame->useReplayTape) {
                if (!replayPaused) {
                    if (replayMode == 8) {
                        replayTime += g_TrackGame->frameTime * g_TrackGame->replaySpeed;
                        replayFrames += (int)g_TrackGame->replaySpeed;
                    } else if (replayMode == 9) {
                        replayTime += g_TrackGame->frameTime * g_TrackGame->replaySpeed;
                        replayFrames += (int)g_TrackGame->replaySpeed;
                    } else if (replayMode != 11) {
                        replayTime += g_TrackGame->frameTime;
                        replayFrames++;
                        if (replayLength != recordTimeLimit) {
                            replayLength += g_TrackGame->frameTime;
                            replayLengthFrames++;
                        }
                    }
                    replayVcr->SetReplayLength(replayLength, 0);
                    if (replayMode == 4) {
                        if (appliedReplayMode != 4) {
                            appliedReplayMode = 4;
                            if (g_TrackGame->mode.soundOn) {
                                if (g_TrackGame->eventManager->FindRaceMode()->field_0x9c != 0) {
                                    g_TrackGame->eventManager->FindRaceMode()->field_0x9c->UnknownVirtualSlot5();
                                }
                                ((UnknownBikeRaceRoot*)g_TrackGame->childRoot)->EnableByClassName("SoundGroup");
                            }
                            if (g_TrackGame->mode.particlesOn) {
                                ((UnknownBikeRaceRoot*)g_TrackGame->childRoot)->EnableByClassName("ParticleManager");
                                ((UnknownBikeRaceRoot*)g_TrackGame->childRoot)->EnableByClassName("DirtParticleEmitter");
                                ((UnknownBikeRaceRoot*)g_TrackGame->childRoot)->EnableByClassName("DustParticleEmitter");
                                ((UnknownBikeRaceRoot*)g_TrackGame->childRoot)->EnableByClassName("DirtChunkParticleEmitter");
                                ((UnknownBikeRaceRoot*)g_TrackGame->childRoot)->EnableByClassName("SteamParticleEmitter");
                            }
                            if (g_TrackGame->mode.shadowsOn) {
                                ((UnknownBikeRaceRoot*)g_TrackGame->childRoot)->EnableByClassName("ProjectedShadow");
                                ((UnknownBikeRaceRoot*)g_TrackGame->childRoot)->EnableByClassName("TerrainShadow");
                                ((UnknownBikeRaceRoot*)g_TrackGame->childRoot)->EnableByClassName("D3DIMSoultreeShadow");
                            }
                        }
                    } else if (appliedReplayMode != replayMode) {
                        appliedReplayMode = replayMode;
                        if (g_TrackGame->eventManager->FindRaceMode()->field_0x9c != 0) {
                            g_TrackGame->eventManager->FindRaceMode()->field_0x9c->UnknownVirtualSlot4();
                        }
                        ((UnknownBikeRaceRoot*)g_TrackGame->childRoot)->DisableByClassName("SoundGroup");
                        ((UnknownBikeRaceRoot*)g_TrackGame->childRoot)->DisableByClassName("ParticleManager");
                        ((UnknownBikeRaceRoot*)g_TrackGame->childRoot)->DisableByClassName("DirtParticleEmitter");
                        ((UnknownBikeRaceRoot*)g_TrackGame->childRoot)->DisableByClassName("DustParticleEmitter");
                        ((UnknownBikeRaceRoot*)g_TrackGame->childRoot)->DisableByClassName("DirtChunkParticleEmitter");
                        ((UnknownBikeRaceRoot*)g_TrackGame->childRoot)->DisableByClassName("SteamParticleEmitter");
                        ((UnknownBikeRaceRoot*)g_TrackGame->childRoot)->DisableByClassName("ProjectedShadow");
                        ((UnknownBikeRaceRoot*)g_TrackGame->childRoot)->DisableByClassName("TerrainShadow");
                        ((UnknownBikeRaceRoot*)g_TrackGame->childRoot)->DisableByClassName("D3DIMSoultreeShadow");
                    }
                }
                if (!g_TrackGame->playingReplay && isRacing) {
                    if (replayLength > recordTimeLimit) {
                        replayLength = recordTimeLimit;
                        TextQueueOverlay* overlay = g_TrackGame->eventManager->FindTextQueue();
                        char text[0x100];
                        g_TrackGame->LoadResourceString(0x14dd, text, 0x80);
                        UnknownMessage* message = new (__FILE__, 0x912) UnknownMessage(text, 3.25f);
                        if (message != 0) {
                            overlay->ReplaceHeadMessage(message);
                            delete message;
                        }
                    } else {
                        recorderTickElapsed += g_TrackGame->frameTime;
                        recorderTickDue = recorderTickElapsed >= g_TrackGame->shortRecordPacketIntervalSeconds;
                        if (recorderTickDue) {
                            recorderTickElapsed -= g_TrackGame->shortRecordPacketIntervalSeconds;
                            recordTickTime = replayLength;
                            recordTickCount++;
                            TrackGameViewOwner* owner = g_TrackGame->tagEvent;
                            if (owner != 0 && owner->ball != 0 && owner->followedRacer == 0) {
                                field_0x1b4 = UnknownFunctionRacerCount() + 1;
                            } else {
                                field_0x1b4 = UnknownFunctionRacerCount();
                            }
                            int camera = g_TrackGame->mode.raceSettings.eventType;
                            if (camera == 0 || (camera == 4 && g_TrackGame->mode.raceSettings.stuntMode)) {
                                if (recordTickCount % 20 == 0) {
                                    field_0x3fb = true;
                                    field_0x1b4 *= 2;
                                } else {
                                    field_0x3fb = false;
                                }
                            }
                            if (replayVcr != 0) {
                                replayVcr->QueueRecord(-1, 0, &recordTickTime, 0xc);
                            }
                        }
                    }
                }
                if (replayVcr != 0 && replayVcr->sceneTimePending) {
                    SetSceneTime(replayVcr->pendingSceneTime);
                    replayVcr->sceneTimePending = 0;
                }
            }
            if (networkStartPending) {
                if (!startStampSent) {
                    UnknownBikeRaceNetStamp stamp;
                    stamp.timeStamp = ReadClock();
                    g_TrackGame->network->Send(
                        0x84, &stamp, 8, g_TrackGame->network->localPlayer, 0);
                    startStampSent = true;
                }
            }
            if (networkStartPending) {
                int ready = 1;
                int self = g_TrackGame->network->localPlayer;
                for (int i = 0; i < g_TrackGame->mode.racerCount; i++) {
                    int id = g_TrackGame->mode.racers[i].networkPlayerId;
                    if (id != self) {
                        NetPlayer* player = g_TrackGame->network->FindPlayer(id);
                        if (!g_TrackGame->mode.racers[i].hasLeft && player != 0) {
                            ready = 0;
                        } else {
                            racerSlots[i]->UnknownVirtualSlot5();
                            for (int j = 0; j < g_TrackGame->networkAiRacerCount + g_TrackGame->playerCount; j++) {
                                if (racerSlots[j]->recorderId == id) {
                                    racerSlots[j]->UnknownVirtualSlot5();
                                }
                            }
                        }
                        if (player == 0) {
                            racerSlots[i]->UnknownVirtualSlot4();
                            for (int j = 0; j < g_TrackGame->networkAiRacerCount + g_TrackGame->playerCount; j++) {
                                if (racerSlots[j]->recorderId == id) {
                                    racerSlots[j]->UnknownVirtualSlot4();
                                }
                            }
                        }
                    }
                }
                if (ready) {
                    ResetStartCountdown();
                    networkStartPending = false;
                    g_TrackGame->network->StopKeepAlive();
                    if (g_TrackGame->playerCount > 1) {
                        int count = g_TrackGame->mode.raceSettings.multiplayerAiRacerCount + g_TrackGame->playerCount;
                        for (int k = 0; k < count; k++) {
                            racerSlots[k]->UnknownRacerVirtualSlot50(1, racerSlots[k]->UnknownRacerVirtualSlot45(), 0);
                        }
                    }
                }
            } else {
                Vector3 position;
                if (isRacing) {
                    if (startGate != 0 && ((UnknownBikeRaceSceneCharacter*)startGate)->isEnabled &&
                        startGateTimer > 0.0f) {
                        startGateTimer -= g_TrackGame->frameTime;
                        if (startGateTimer <= 0.0f) {
                            startGateTimer = 0;
                            raceScene->RestartEntry(startGateEntry, 0.0f, 0, 0);
                            startGate->UnknownVirtualSlot4();
                            if (!UnknownFunctionCameraView()->followingRacer && targetEntry == startGateEntry) {
                                NextCameraTarget(1, 1, 0);
                            }
                            if (startGate->collisionObject != 0) {
                                startGate->collisionObject->Release();
                                startGate->collisionObject = 0;
                            }
                        }
                    }
                } else {
                    startCountdown -= g_TrackGame->frameTime;
                    if (startCountdown <= 0.0f || AnyRacerRemote()) {
                        startCountdown = 0;
                        isRacing = true;
                        raceScene->field_0x7c0 = 0;
                        if (startGate != 0) {
                            raceScene->UnknownFunction4eafd0(startGateEntry);
                            startGateTimer = 5.0f;
                        }
                        if (g_TrackGame->playerCount > 1) {
                            int self = g_TrackGame->network->localPlayer;
                            int count = g_TrackGame->mode.raceSettings.multiplayerAiRacerCount + g_TrackGame->playerCount;
                            for (int i = 0; i < count; i++) {
                                if (g_TrackGame->mode.racers[i].networkPlayerId == self) {
                                    racerSlots[i]->velocity = kVec3Zero;
                                    racerSlots[i]->field_0x0bc = 0;
                                }
                            }
                        }
                        replayTime = 0;
                        replayFrames = 0;
                        replayLength = 0;
                        replayLengthFrames = 0;
                        if (thirtySecondMan != 0) {
                            raceScene->RestartEntry(thirtySecondManEntry, 0.0f, 0, 0);
                            thirtySecondMan->UnknownVirtualSlot4();
                            if (!UnknownFunctionCameraView()->followingRacer && targetEntry == thirtySecondManEntry) {
                                NextCameraTarget(1, 1, 0);
                            }
                            if (thirtySecondMan->collisionObject != 0) {
                                thirtySecondMan->collisionObject->Release();
                                thirtySecondMan->collisionObject = 0;
                            }
                        }
                        switch (g_TrackGame->mode.raceSettings.eventType) {
                        case 2:
                            chatOverlay = (UnknownBikeRaceOverlay19c*)g_TrackGame->nationalEvent->field_0x64;
                            break;
                        case 3:
                            chatOverlay = (UnknownBikeRaceOverlay19c*)g_TrackGame->supercrossEvent->field_0x64;
                            break;
                        case 0:
                            chatOverlay = (UnknownBikeRaceOverlay19c*)g_TrackGame->quarryEvent->field_0x64;
                            break;
                        case 1:
                        case 5:
                            chatOverlay = (UnknownBikeRaceOverlay19c*)g_TrackGame->bajaEnduroEvent->field_0x64;
                            break;
                        case 4:
                            chatOverlay = (UnknownBikeRaceOverlay19c*)g_TrackGame->tagEvent->field_0x64;
                            break;
                        }
                    }
                }
                GameObject::UnknownVirtualSlot10(frameTime);
                int camera = g_TrackGame->mode.raceSettings.eventType;
                if (camera == 3 || camera == 2 || camera == 5 || camera == 1 || camera == 0) {
                    Track* track = raceTrack;
                    if ((track != 0 || gateList != 0 || camera == 0) && isRacing && !restartPending &&
                        localRacer != 0) {
                        int keepRacing = g_TrackGame->mode.raceSettings.raceMode == 0 ||
                                         g_TrackGame->mode.raceSettings.raceMode == 4;
                        if (!g_TrackGame->playingReplay) {
                            if (camera == 1 || camera == 5) {
                                UpdateGateRace(&field_0x0c4, gateList, g_TrackGame->frameTime,
                                                      g_TrackGame->mode.raceSettings.lapCount, keepRacing);
                            } else if (camera == 0) {
                                RankByScore(keepRacing);
                            } else {
                                UpdateLapRace(&field_0x0c4, track, g_TrackGame->frameTime, &startProbe,
                                                      &finishProbe, g_TrackGame->mode.raceSettings.lapCount,
                                                      keepRacing);
                            }
                        }
                    }
                }
                if (isRacing && restartPending) {
                    restartPending = false;
                    isRacing = false;
                }
                if (youIndicator != 0 && ((UnknownBikeRaceObjectFlags*)youIndicator)->isEnabled) {
                    position = localRacer->carriedFlagPosition;
                    position.y += 4.25f;
                    ((ArcadeObject*)youIndicator)->SetPosition(&position);
                }
                if (chatHideTimer > 0.0f && g_TrackGame->mode.chatInputLine != 1) {
                    chatHideTimer -= g_TrackGame->frameTime;
                    if (chatHideTimer <= 0.0f) {
                        chatHideTimer = 0;
                        if (chatOverlay != 0) {
                            chatOverlay->typing = 0;
                            chatOverlay->HideInputLine();
                        }
                        chatOpen = false;
                    }
                }
                if (flagGirl != 0 && localRacer->statusNode->field_0x44.node != 0) {
                    int laps = g_TrackGame->mode.raceSettings.lapCount;
                    int lap = localRacer->lapCount;
                    int last = lap == laps - 1;
                    int previous = lap == laps - 2;
                    if (previous || last) {
                        float toFinish = raceTrack->DistanceAlong(localRacer->statusNode->field_0x44,
                                                                            finishProbe.field_0x2c);
                        float fromFinish = raceTrack->DistanceAlong(finishProbe.field_0x2c,
                                                                              localRacer->statusNode->field_0x44);
                        if (field_0x078 && localRacer->finished && fromFinish > 100.0f) {
                            ReplaceMeshId(flagGirl, 1, 2);
                            flagGirl->SetMotionByName("Stand");
                            field_0x078 = 0;
                        } else if ((previous && toFinish < 300.0f) || (last && fromFinish < 100.0f)) {
                            if (field_0x078 != 1) {
                                ReplaceMeshId(flagGirl, 1, 2);
                                field_0x078 = 1;
                                flagGirl->SetMotionByName("FlagStart");
                            } else if (flagGirl->motionFinished) {
                                flagGirl->SetMotionByName("FlagLoop");
                            }
                        } else if (field_0x078 && field_0x078 < 3 &&
                                   ((previous && toFinish > 100.0f) || (last && fromFinish > 100.0f))) {
                            flagGirl->SetMotionByName("Stand");
                            field_0x078 = 0;
                        } else if ((last && toFinish < 300.0f) || (localRacer->finished && fromFinish < 100.0f)) {
                            if (field_0x078 < 3) {
                                ReplaceMeshId(flagGirl, 2, 1);
                                field_0x078 = 3;
                                flagGirl->SetMotionByName("FlagStart");
                            } else if (flagGirl->motionFinished) {
                                flagGirl->SetMotionByName("FlagLoop");
                                field_0x078 = 4;
                            }
                        }
                    }
                    flagGirl->UnknownVirtualSlot7(frameTime, 0, 0);
                }
            }
        } else if (replayVcr != 0) {
            replayVcr->UnknownVirtualSlot10(frameTime);
        }
    }
    if (g_TrackGame->testKey && g_TrackGame->debugOverlay != 0) {
        if (lodDebugPage < 0) {
            lodDebugPage = g_TrackGame->debugOverlay->NewPage();
        }
        if (g_TrackGame->debugOverlay->field_0x26c4 == lodDebugPage) {
            char name[0x20];
            UnknownBikeRaceModel* model;
            int pass = 0;
            if ((signed char)targetKind < 0) {
                strncpy(name,
                        ((UnknownBikeRaceUiModel*)g_TrackGame->ui->manufacturers)
                            [((UnknownBikeRaceUiChoice*)g_TrackGame->ui->bikes)
                                 [g_TrackGame->mode.racerChoice.bikeIndex].field_0x00].field_0x40,
                        0x14);
                name[0x14] = 0;
                raceScene->ResolveSltPath(name);
                model = localRacer->modelNode;
            } else if (targetKind == 0) {
                strncpy(name, raceScene->staticModels->casters[targetIndex].fileName, 0x14);
                name[0x14] = 0;
                model = (UnknownBikeRaceModel*)raceScene->staticModels->casters[targetIndex].model;
            } else {
                strncpy(name, raceScene->animationTable->entries[targetIndex].fileName, 0x14);
                name[0x14] = 0;
                UnknownSceneEntry* entry = &raceScene->animationTable->entries[targetIndex];
                if (entry->keyFramed) {
                    model = ((UnknownBikeRaceRider*)entry->character)->modelNode;
                } else {
                    model = (UnknownBikeRaceModel*)entry->proceduralObject->field_0x34;
                }
            }
            do {
                int faces = 0;
                UnknownBikeRaceMeshGroup* group = &model->levelGroups[model->currentDetailLevel];
                for (int i = 0; i < group->itemCount; i++) {
                    faces += group->field_0x04[i].faceCount;
                }
                g_TrackGame->debugOverlay->Title(lodDebugPage, "Soultree LOD (%s)", name);
                g_TrackGame->debugOverlay->Print(lodDebugPage, "Current LOD %d out of %d",
                                                                        model->currentDetailLevel + 1, model->detailLevelCount);
                g_TrackGame->debugOverlay->Print(lodDebugPage, "Number of Faces = %d", faces);
                if ((signed char)targetKind < 0) {
                    if (g_UnknownGlobal567a88 > 5) {
                        if ((pass == 1 && g_UnknownGlobal567a88 - 7 > model->detailLevelCount) || g_UnknownGlobal567a88 < 2) {
                            g_TrackGame->debugOverlay->SetRowHighlight(lodDebugPage, g_UnknownGlobal567a88 + 1, 0);
                            g_UnknownGlobal567a88 = 2;
                        }
                    } else if ((pass == 0 && g_UnknownGlobal567a88 > model->detailLevelCount) || g_UnknownGlobal567a88 < 2) {
                        g_TrackGame->debugOverlay->SetRowHighlight(lodDebugPage, g_UnknownGlobal567a88 + 1, 0);
                        g_UnknownGlobal567a88 = 9;
                    }
                } else if (g_UnknownGlobal567a88 > model->detailLevelCount || g_UnknownGlobal567a88 < 2) {
                    g_TrackGame->debugOverlay->SetRowHighlight(lodDebugPage, g_UnknownGlobal567a88 + 1, 0);
                    g_UnknownGlobal567a88 = 2;
                }
                if (model->detailLevelCount > 1) {
                    int up = g_TrackGame->controlInterface->UnknownVirtualSlot3(0xd0, 0, 0x80, 0);
                    int down = g_TrackGame->controlInterface->UnknownVirtualSlot3(0xc8, 0, 0x80, 0);
                    int row = g_UnknownGlobal567a88 - 1;
                    if (down || up) {
                        if ((signed char)targetKind < 0) {
                            if (g_UnknownGlobal567a88 > 5) {
                                if (pass == 1) {
                                    row = g_UnknownGlobal567a88 - 8;
                                } else {
                                    row = 99;
                                }
                            } else if (pass == 1) {
                                row = 99;
                            }
                        }
                        switch (row) {
                        case 1:
                            if (model->detailLevelCount > 1) {
                                if (down) {
                                    model->detailDistances[0] += 0.05f;
                                } else {
                                    model->detailDistances[0] -= 0.05f;
                                    model->detailDistances[0] = UnknownFunctionMax(model->detailDistances[0], 0.0f);
                                }
                            }
                            break;
                        case 2:
                            if (model->detailLevelCount > 2) {
                                if (down) {
                                    model->detailDistances[1] += 0.05f;
                                } else {
                                    model->detailDistances[1] -= 0.05f;
                                    model->detailDistances[1] = UnknownFunctionMax(model->detailDistances[1], 0.0f);
                                }
                            }
                            break;
                        case 3:
                            if (model->detailLevelCount > 3) {
                                if (down) {
                                    model->detailDistances[2] += 0.05f;
                                } else {
                                    model->detailDistances[2] -= 0.05f;
                                    model->detailDistances[2] = UnknownFunctionMax(model->detailDistances[2], 0.0f);
                                }
                            }
                            break;
                        case 4:
                            if (model->detailLevelCount > 4) {
                                if (down) {
                                    model->detailDistances[3] += 0.05f;
                                } else {
                                    model->detailDistances[3] -= 0.05f;
                                    model->detailDistances[3] = UnknownFunctionMax(model->detailDistances[3], 0.0f);
                                }
                            }
                            break;
                        }
                    }
                    g_TrackGame->debugOverlay->Print(
                        lodDebugPage, "AutoLOD#0 %.2f", model->detailLevelCount > 1 ? model->detailDistances[0] : 0.0f);
                    g_TrackGame->debugOverlay->Print(
                        lodDebugPage, "AutoLOD#1 %.2f", model->detailLevelCount > 2 ? model->detailDistances[1] : 0.0f);
                    g_TrackGame->debugOverlay->Print(
                        lodDebugPage, "AutoLOD#2 %.2f", model->detailLevelCount > 3 ? model->detailDistances[2] : 0.0f);
                    g_TrackGame->debugOverlay->Print(
                        lodDebugPage, "AutoLOD#3 %.2f", model->detailLevelCount > 4 ? model->detailDistances[3] : 0.0f);
                    g_TrackGame->debugOverlay->SetRowHighlight(lodDebugPage, g_UnknownGlobal567a88 + 1, 1);
                }
                if ((signed char)targetKind >= 0) {
                    break;
                }
                COPY_TEXT(name, "rider.slt", 0x20);
                model = localRacer->riderCharacter->modelNode;
            } while (++pass < 2);
        }
    }
    return 1;
}

// 0x0041ea10
int BikeRace::AnyRacerRemote() {
    if (g_TrackGame->playerCount > 1) {
        for (int i = 0; i < g_TrackGame->mode.racerCount; i++) {
            if (racerSlots[i]->UnknownFunction495c00()) {
                return 1;
            }
        }
    }
    return 0;
}

// 0x0041ea60
void BikeRace::ReplaceMeshId(UnknownBikeRaceCharacter* character, int from, int to) {
    for (int i = 0; i < character->modelNode->groupCount; i++) {
        for (int j = 0; j < character->modelNode->field_0x28c[i].itemCount; j++) {
            for (int k = 0; k < character->modelNode->field_0x28c[i].field_0x04[j].idCount; k++) {
                if (character->modelNode->field_0x28c[i].field_0x04[j].ids[k] == from) {
                    character->modelNode->field_0x28c[i].field_0x04[j].ids[k] = to;
                }
            }
        }
    }
}

// 0x0041f550
void BikeRace::ToggleReplayPause(int pauseOnly) {
    if (pauseOnly) {
        if (replayPaused) {
            return;
        }
        replayPaused = true;
    } else {
        replayPaused = !replayPaused;
    }
    g_TrackGame->paused = replayPaused;
}

// 0x0041f590
int BikeRace::UnknownVirtualSlot20(int value) {
    if (chatOpen && chatOverlay != 0 && chatOverlay->HandleChatKey(value, &value)) {
        if (value == 0) {
            chatHideTimer = 10.0f;
        }
        return 1;
    }
    return 0;
}

// 0x0041f5e0
int BikeRace::UnknownVirtualSlot23(UnknownControlEvent* event, UnknownInputEntry* entry) {
    int result;
    if (g_TrackGame->network != 0 && event->kind == 0) {
        g_TrackGame->controlInterface->keyboard->ModifierHolds(0xc);
    }
    if (chatOpen && chatOverlay != 0) {
        if (event->control == 1 && event->kind == 0) {
            chatHideTimer = 0.1f;
            chatOverlay->typing = 0;
            chatOpen = false;
            return 1;
        }
        if (chatOverlay->HandleChatInput(event, entry, &result)) {
            return 1;
        }
    }
    if (GameObject::UnknownVirtualSlot23(event, entry)) {
        return 1;
    }
    if (!chatOpen && g_TrackGame->uiInteractionBlocked) {
        return 0;
    }
    if (event->kind == 0) {
        switch (event->control) {
        case 0x35:
            if (g_TrackGame->playerCount > 1) {
                KeyboardDevice* keyboard = g_TrackGame->controlInterface->keyboard;
                if (!keyboard->UnknownVirtualSlot5(0x2a, 0x3f, 0) &&
                    !g_TrackGame->controlInterface->keyboard->UnknownVirtualSlot5(0x36, 0x3f, 0)) {
                    if (g_TrackGame->mode.chatInputLine == 2) {
                        break;
                    }
                    if (chatOverlay != 0) {
                        chatOverlay->ShowInputLine();
                        chatOverlay->typing = 1;
                    }
                    chatHideTimer = 10.0f;
                    chatOpen = true;
                    return 1;
                }
                g_TrackGame->mode.chatInputLine++;
                if (g_TrackGame->mode.chatInputLine == 3) {
                    g_TrackGame->mode.chatInputLine = 0;
                }
                if (g_TrackGame->mode.chatInputLine == 1 && chatOverlay != 0) {
                    chatOverlay->ShowInputLine();
                    chatOverlay->typing = 1;
                    chatOpen = true;
                }
                char title[0x80];
                char value[0x80];
                char text[0x100];
                g_TrackGame->LoadResourceString(0x1428, title, 0x80);
                if (g_TrackGame->mode.chatInputLine == 0) {
                    g_TrackGame->LoadResourceString(0x140b, value, 0x80);
                } else if (g_TrackGame->mode.chatInputLine == 1) {
                    g_TrackGame->LoadResourceString(0x140a, value, 0x80);
                } else if (g_TrackGame->mode.chatInputLine == 2) {
                    g_TrackGame->LoadResourceString(0x140c, value, 0x80);
                }
                sprintf(text, "%s : %s", title, value);
                UnknownMessage* message = new (__FILE__, 0xcdd) UnknownMessage(text, 3.25f);
                TextQueueOverlay* overlay;
                switch (g_TrackGame->mode.raceSettings.eventType) {
                case 2:
                    overlay = g_TrackGame->nationalEvent->textQueue;
                    if (overlay != 0 && message != 0) {
                        overlay->ReplaceHeadMessage(message);
                    }
                    break;
                case 0:
                    overlay = g_TrackGame->quarryEvent->textQueue;
                    if (overlay != 0 && message != 0) {
                        overlay->ReplaceHeadMessage(message);
                    }
                    break;
                case 1:
                case 5:
                    overlay = g_TrackGame->bajaEnduroEvent->textQueue;
                    if (overlay != 0 && message != 0) {
                        overlay->ReplaceHeadMessage(message);
                    }
                    break;
                case 4:
                    overlay = g_TrackGame->tagEvent->textQueue;
                    if (overlay != 0 && message != 0) {
                        overlay->ReplaceHeadMessage(message);
                    }
                    break;
                }
                if (message != 0) {
                    delete message;
                }
            }
            break;
        case 0x1b:
            if ((g_TrackGame->testKey) &&
                TestInputEvent(0x1b, 0, event, 3)) {
                NextCameraTarget(1, 0, 0);
                return 1;
            }
            if (!TestInputEvent(0x1b, 0, event, 0x80)) {
                NextCameraTarget(1, 0, 1);
            }
            return 1;
        case 0x1a:
            if ((g_TrackGame->testKey) &&
                TestInputEvent(0x1a, 0, event, 3)) {
                NextCameraTarget(0, 0, 0);
                return 1;
            }
            if (!TestInputEvent(0x1a, 0, event, 0x80)) {
                NextCameraTarget(0, 0, 1);
            }
            return 1;

        case 0xc9:
            if (TestInputEvent(0xc9, 0, event, 0x80000000)) {
                NextCameraTarget(1, 1, 0);
                return 1;
            }
            break;
        case 0xd1:
            if (TestInputEvent(0xd1, 0, event, 0x80000000)) {
                NextCameraTarget(0, 1, 0);
                return 1;
            }
            break;
        case 0x22:
            if (TestInputEvent(0x22, 0, event, 0xc)) {
                g_TrackGame->mode.gasGyro = 1 - g_TrackGame->mode.gasGyro;
                localRacer->gasGyro = g_TrackGame->mode.gasGyro;
                TextQueueOverlay* overlay = g_TrackGame->eventManager->FindTextQueue();
                if (overlay == 0) {
                    return 1;
                }
                char title[0x80];
                char value[0x80];
                char text[0x80];
                g_TrackGame->LoadResourceString(0x140d, title, 0x80);
                if (localRacer->gasGyro) {
                    g_TrackGame->LoadResourceString(0x1407, value, 0x80);
                } else {
                    g_TrackGame->LoadResourceString(0x1408, value, 0x80);
                }
                sprintf(text, "%s %s", title, value);
                UnknownMessage message(text, 1.5f);
                overlay->ReplaceHeadMessage(&message);
                return 1;
            }
            break;
        case 0x30:
            if (TestInputEvent(0x30, 0, event, 0xc)) {
                g_TrackGame->mode.brakeGyro = 1 - g_TrackGame->mode.brakeGyro;
                localRacer->brakeGyro = g_TrackGame->mode.brakeGyro;
                TextQueueOverlay* overlay = g_TrackGame->eventManager->FindTextQueue();
                if (overlay == 0) {
                    return 1;
                }
                char title[0x80];
                char value[0x80];
                char text[0x80];
                g_TrackGame->LoadResourceString(0x142d, title, 0x80);
                if (localRacer->brakeGyro) {
                    g_TrackGame->LoadResourceString(0x1407, value, 0x80);
                } else {
                    g_TrackGame->LoadResourceString(0x1408, value, 0x80);
                }
                sprintf(text, "%s %s", title, value);
                UnknownMessage message(text, 1.5f);
                overlay->ReplaceHeadMessage(&message);
                return 1;
            }
            break;
        case 0x1f:
            if (TestInputEvent(0x1f, 0, event, 0x80)) {
                field_0x034 = 1 - field_0x034;
            }
            break;
        }
    }
    if (g_TrackGame->testKey && g_TrackGame->debugOverlay != 0) {
        if (lodDebugPage < 0) {
            lodDebugPage = g_TrackGame->debugOverlay->NewPage();
        }
        int shown = g_TrackGame->debugOverlay->field_0x26c4;
        if (shown == lodDebugPage) {

            if (TestInputEvent(0x1c, 0, event, 0x80)) {
                if (g_TrackGame->ui->gui->GetUser(0)->userCursor == 0) {
                    g_TrackGame->ui->gui->CreateCursors("ui\\cursor.tga", 0);
                }
                if (!((UnknownBikeRaceObjectFlags*)g_TrackGame->ui->gui
                          ->GetUser(0)->userCursor)->isEnabled) {
                    if (objectPicker == 0) {
                        ObjectPicker* picker = new (__FILE__, 0xed2) ObjectPicker(1);
                        objectPicker = picker;
                        picker->Create(
                            g_TrackGame->renderTarget, 0, PickerHitCallback,
                            (GameCursor*)g_TrackGame->ui->gui->GetUser(0)->userCursor);
                        if (AppendChild(objectPicker, -1)) {
                            g_TrackGame->ui->ShowCursors();
                        }
                    } else {
                        g_TrackGame->ui->ShowCursors();
                    }
                } else {
                    g_TrackGame->debugOverlay->SetRowHighlight(
                        lodDebugPage, g_UnknownGlobal567a88 + 1, 0);
                    g_UnknownGlobal567a88++;
                    if ((signed char)targetKind < 0) {
                        if (g_UnknownGlobal567a88 > 12) {
                            g_UnknownGlobal567a88 = 2;
                        } else if (g_UnknownGlobal567a88 > 5 && g_UnknownGlobal567a88 < 9) {
                            g_UnknownGlobal567a88 = 9;
                        }
                    } else if (g_UnknownGlobal567a88 > 5) {
                        g_UnknownGlobal567a88 = 2;
                    }
                }
            }
            if (TestInputEvent(0, 1, event, 0x80000000)) {
                int picked = objectPicker->PickAtCursor();
                if (picked != 0) {
                    if (raceScene->staticModels != 0) {
                        for (int i = 0; i < raceScene->staticModels->casterCount; i++) {
                            UnknownBikeRacePickCaster* caster =
                                &((UnknownBikeRacePickCaster*)raceScene->staticModels->casters)[i];
                            if (caster->field_0x00 & 1) {
                                if (picked == caster->field_0x08->field_0x128) {
                                    UnknownFunctionCameraView()->followingRacer = 1;
                                    targetCaster = i;
                                    NextCameraTarget(1, 0, 0);
                                    return 1;
                                }
                            } else if (picked == caster->field_0x0c) {
                                UnknownFunctionCameraView()->followingRacer = 1;
                                targetCaster = i;
                                NextCameraTarget(1, 0, 0);
                                return 1;
                            }
                        }
                    }
                    if (raceScene->animationTable != 0) {
                        for (int i = 0; i < raceScene->animationTable->entryCount; i++) {
                            UnknownSceneEntry* entry = &raceScene->animationTable->entries[i];
                            if (entry->keyFramed) {
                                if (picked == ((UnknownBikeRacePickCharacter*)entry->character)->collisionObject) {
                                    UnknownFunctionCameraView()->followingRacer = 1;
                                    targetEntry = i;
                                    NextCameraTarget(1, 0, 1);
                                    return 1;
                                }
                            } else if (picked == ((UnknownBikeRacePickObject*)entry->proceduralObject)->field_0x3c) {
                                UnknownFunctionCameraView()->followingRacer = 1;
                                targetEntry = i;
                                NextCameraTarget(1, 0, 1);
                                return 1;
                            }
                        }
                    }
                }
            }
        }
    }
    return 0;
}

// 0x00420040
int BikeRace::UnknownVirtualSlot24(int type, void* data, int from, int to, int flags) {
    int count;
    if (GameObject::UnknownVirtualSlot24(type, data, from, to, flags)) {
        return 1;
    }
    if (g_TrackGame->uiInteractionBlocked) {
        return 1;
    }
    if (from == 0) {
        return 0;
    }
    if (type == 1) {
        UnknownBikeRaceNetState* state = (UnknownBikeRaceNetState*)data;
        int self = g_TrackGame->network->localPlayer;
        count = g_TrackGame->mode.racerCount;
        for (int i = 0; i < count; i++) {
            int id = g_TrackGame->mode.racers[i].networkPlayerId;
            if (id != self && id == from) {
                UnknownBikeRaceRacer* racer = racerSlots[i];
                if (racer != 0 && racer->netIndex == state->racerIndex) {
                    racerSlots[i]->field_0x11b8 = 1;
                    UnknownBikeRaceRacerPart* oldest = racerSlots[i]->receivedStates[2];
                    racerSlots[i]->receivedStates[2] = racerSlots[i]->receivedStates[1];
                    racerSlots[i]->receivedStates[1] = racerSlots[i]->receivedStates[0];
                    racerSlots[i]->receivedStates[0] = oldest;
                    racerSlots[i]->receivedStates[0]->messageFlags = flags;
                    racerSlots[i]->receivedStates[0]->state = *state;
                    racerSlots[i]->receivedStates[0]->fresh = 1;
                    racerSlots[i]->field_0x1358 = state->field_0x3c;
                    racerSlots[i]->field_0x1364 = state->field_0x08;
                    racerSlots[i]->field_0x1370 = state->field_0x30;
                    racerSlots[i]->field_0x137c = state->field_0x14;
                    racerSlots[i]->field_0x1380 = state->field_0x18;
                    racerSlots[i]->field_0x1384 = state->field_0x1c;
                    racerSlots[i]->field_0x1388 = replayMode == 11 ? flags : state->field_0x4c;
                    return 1;
                }
            }
        }
    } else if (type == 13) {
        UnknownBikeRaceNetMessage13* message = (UnknownBikeRaceNetMessage13*)data;
        int self = g_TrackGame->network->localPlayer;
        for (int i = 0; i < g_TrackGame->mode.racerCount; i++) {
            int id = g_TrackGame->mode.racers[i].networkPlayerId;
            if (id != self && id == from && racerSlots[i] != 0 &&
                racerSlots[i]->netIndex == message->racerIndex) {
                if (!racerSlots[i]->field_0x11b8) {
                    return 1;
                }
                UnknownBikeRaceRacerPart* oldest = racerSlots[i]->receivedStates[2];
                racerSlots[i]->receivedStates[2] = racerSlots[i]->receivedStates[1];
                racerSlots[i]->receivedStates[1] = racerSlots[i]->receivedStates[0];
                racerSlots[i]->receivedStates[0] = oldest;
                racerSlots[i]->receivedStates[0]->messageFlags = flags;
                racerSlots[i]->UnknownFunction4933e0(message, racerSlots[i]->receivedStates[0]);
                racerSlots[i]->receivedStates[0]->fresh = 0;
                return 1;
            }
        }
    } else if (type == 10) {
        UnknownBikeRaceNetScore* score = (UnknownBikeRaceNetScore*)data;
        count = g_TrackGame->mode.racerCount;
        int self = g_TrackGame->network->localPlayer;
        for (int i = 0; i < count; i++) {
            int id = g_TrackGame->mode.racers[i].networkPlayerId;
            if (id != self && id == from) {
                UnknownBikeRaceRacer* racer = racerSlots[i];
                if (racer != 0 && racer->netIndex == score->racerIndex) {
                    racerSlots[i]->score = score->field_0x04;
                    return 1;
                }
            }
        }
    } else if (type == 0x85) {
        char name[0x14];
        if (g_TrackGame->network->GetPlayerName(from, name)) {
            if (g_TrackGame->mode.chatInputLine != 2) {
                if (chatOverlay != 0) {
                    chatOverlay->ShowInputLine();
                }
                chatHideTimer = 10.0f;
                chatOpen = true;
            }
            if (chatOverlay != 0) {
                chatOverlay->AddMessage(name, &((UnknownBikeRaceNetStamp*)data)->timeStamp, 1);
            }
        }
        return 1;
    } else if (type == 0x84) {
        UnknownBikeRaceNetStamp* stamp = (UnknownBikeRaceNetStamp*)data;
        for (int i = 0; i < g_TrackGame->mode.racerCount; i++) {
            if (g_TrackGame->mode.racers[i].networkPlayerId == from) {
                float delay = (float)(unsigned int)flags - (float)stamp->timeStamp;
                if (delay < g_TrackGame->mode.racers[i].lowestMessageDelay) {
                    g_TrackGame->mode.racers[i].lowestMessageDelay = delay;
                }
                g_TrackGame->mode.racers[i].hasLeft = 1;
            }
        }
    }
    return 0;
}

// 0x004204e0
UnknownBikeRaceRacer* BikeRace::NextBike(int* iterator) {
    UnknownBikeRaceRacer* racer = 0;
    if (g_TrackGame->uiInteractionBlocked) {
        return 0;
    }
    int players = g_TrackGame->playerCount;
    if (players == 1 && *iterator == 0) {
        racer = localRacer;
    } else {
        int index = *iterator;
        if (players > 1) {
            racer = racerSlots[index];
        } else if (index >= players && aiRacers != 0) {
            racer = aiRacers[index - players];
        }
    }
    (*iterator)++;
    if (g_TrackGame->playerCount > 1) {
        if (*iterator > g_TrackGame->mode.raceSettings.multiplayerAiRacerCount + g_TrackGame->playerCount) {
            *iterator = 0;
            return 0;
        }
    } else {
        int count = g_TrackGame->mode.raceSettings.raceMode == 4 ? 1 : g_TrackGame->mode.raceSettings.aiRacerCount;
        if (*iterator > count + 1) {
            *iterator = 0;
            return 0;
        }
    }
    return racer;
}

// 0x00420590
void BikeRace::HandlePlayerLeft(int player) {
    if (g_TrackGame->uiInteractionBlocked) {
        return;
    }
    int self = g_TrackGame->network->localPlayer;
    for (int i = 0; i < g_TrackGame->mode.racerCount; i++) {
        int id = g_TrackGame->mode.racers[i].networkPlayerId;
        if (id != self && id == player && racerSlots[i] != 0) {
            racerSlots[i]->UnknownRacerVirtualSlot44();
            TrackGameViewOwner* owner = g_TrackGame->tagEvent;
            if (owner != 0 && owner->followedRacer == (UnknownEventRacer*)racerSlots[i]) {
                if (g_TrackGame->mode.raceSettings.tagBall) {
                    owner->followedRacer = 0;
                } else if (g_TrackGame->network->isHost) {
                    owner->PickFollowedRacer();
                }
            }
        }
    }
}

// 0x00420650
void BikeRace::RestartRace(int mode, char* path, char* description) {
    if (g_TrackGame->network != 0) {
        return;
    }
    if (g_TrackGame->uiInteractionBlocked) {
        return;
    }
    restartPending = true;
    gridReady = false;
    ResetStartCountdown();
    int iterator = 0;
    Vector3 position;
    Vector3 direction;
    if (startGate != 0 && g_TrackGame->mode.raceSettings.raceMode != 0 &&
        g_TrackGame->mode.raceSettings.raceMode != 4) {
        startGate->UnknownVirtualSlot5();
    }
    int positions[11];
    int order[11];
    if (g_TrackGame->mode.raceSettings.raceMode == 2) {
        RankByPoints(order);
        for (int i = 0; i < racerCount; i++) {
            for (int j = 0; j < racerCount; j++) {
                if (order[j] == i + 1) {
                    positions[i] = j + 1;
                    break;
                }
            }
        }
    } else {
        for (int i = 0; i < racerCount; i++) {
            positions[i] = i + 1;
        }
    }
    if (g_TrackGame->useReplayTape) {
        SaveReplay(path, description);
        replayTime = 0;
        replayFrames = 0;
        replayLength = 0;
        replayLengthFrames = 0;
        replayMode = 4;
        replayPaused = false;
        replayStepFrame = false;
        isRecording = mode;
        g_TrackGame->replaySpeed = 1.0f;
        g_TrackGame->paused = 0;
        field_0x044->field_0x3c->UnknownVirtualSlot16(0);
        if (field_0x044->field_0x40 != 0) {
            field_0x044->field_0x40->UnknownVirtualSlot16(0);
        }
        field_0x044->field_0x44->UnknownVirtualSlot16(0);
        replayVcr = new (__FILE__, 0x1068) KrustyVCR;
        if (replayVcr != 0) {
            if (mode == 0) {
                g_TrackGame->playingReplay = 1;
                if (!replayVcr->OpenRecording(RecorderCallback, 1, "VCRtape.dat",
                                                        vcrFile)) {
                    delete replayVcr;
                    replayVcr = 0;
                }
                g_TrackGame->ui->gui->ShowCursors(1);
                g_TrackGame->ui->gui->ShowDialog(
                    new (__FILE__, 0x1073) VCRDlg(1, "VCR.dtm"), 0, 1, 0, 0, 0, 0, 1);
            } else {
                g_TrackGame->playingReplay = 0;
                if (!replayVcr->OpenRecording(RecorderCallback, 0, "VCRtape.dat",
                                                        vcrFile)) {
                    delete replayVcr;
                    replayVcr = 0;
                }
            }
            if (replayVcr != 0) {
                AppendChild(replayVcr, -1);
            }
        }
    }
    if (flagGirl != 0) {
        ReplaceMeshId(flagGirl, 1, 2);
        flagGirl->SetMotionByName("Stand");
        field_0x078 = 0;
    }
    SetSceneTime(0.0001f);
    raceScene->field_0x7c0 = 1;
    for (UnknownBikeRaceRacer* racer = NextBike(&iterator); racer != 0;
         racer = NextBike(&iterator)) {
        racer->UnknownRacerVirtualSlot43();
        racer->restarted = true;
        WriteRelativePositions(&position, &direction, g_TrackGame->bajaEnduroEvent,
                              positions[iterator - 1]);
        racer->startPosition = position;
        racer->startDirection = direction;
        racer->UnknownFunction496e20(replayVcr);
        replayPaused = false;
    }
    if (g_TrackGame->tagEvent != 0 &&
        g_TrackGame->tagEvent->ball != 0) {
        g_TrackGame->tagEvent->ball->SetRecorder(replayVcr);
    }
    if (g_TrackGame->quarryEvent != 0) {
        g_TrackGame->quarryEvent->ResetClock();
    }
    if (g_TrackGame->tagEvent != 0) {
        g_TrackGame->tagEvent->ResetClock();
    }
    if (g_TrackGame->bajaEnduroEvent != 0) {
        g_TrackGame->bajaEnduroEvent->UnknownFunction404df0(0, 1, UnknownFunctionCameraView()->field_0x3b4);
    }
}

// 0x00420b00
void BikeRace::SaveReplay(char* path, char* description) {
    g_TrackGame->ui->replayAvailable = 0;
    if (replayVcr != 0) {
        char text[0x80];
        g_TrackGame->LoadResourceString(0x1437, text, sizeof(text));
        if (isRecording) {
            replayVcr->SaveRecording(replayLength, description ? description : text,
                                               path != 0);
        }
        if (path != 0) {
            CopyFileA("VCRtape.dat", path, 1);
        }
        replayVcr->Unlink();
        replayVcr->Release();
        replayVcr = 0;
        g_TrackGame->ui->replayAvailable = 1;
    }
}

// 0x00420bd0
void BikeRace::RestartReplay() {
    if (ghostVcr != 0) {
        ghostVcr->Unlink();
        ghostVcr->Release();
        ghostVcr = 0;
    }
    if (replayVcr == 0) {
        return;
    }
    if (localRacer->lapCount == 1 && field_0x189) {
        replayVcr->Unlink();
        replayVcr->Release();
        replayLength = 0;
    } else {
        char date[0x20];
        char time[0x20];
        char description[0x20];
        GetDateFormatA(0, 0, 0, "M/d/yyyy", date, sizeof(date));
        GetTimeFormatA(0, 0, 0, "h:mm tt", time, sizeof(time));
        sprintf(description, "%s %s", date, time);
        replayVcr->SaveRecording(localRacer->lastLapTime, description, 0);
        replayVcr->Unlink();
        replayVcr->Release();
        if (replayLength <= bestGhostLength) {
            bestGhostLength = replayLength;
            COPY_TEXT(ghostFilePlayed, ghostFileRecorded, 0x104);
            if (strcmp(ghostFileRecorded, "VCRghost.dat") == 0) {
                COPY_TEXT(ghostFileRecorded, "VCRgtemp.dat", 0x104);
            } else {
                COPY_TEXT(ghostFileRecorded, "VCRghost.dat", 0x104);
            }
        }
    }
    replayVcr = new (__FILE__, 0x10e7) KrustyVCR;
    if (replayVcr == 0) {
        return;
    }
    if (!replayVcr->OpenRecording(RecorderCallback, 0, ghostFileRecorded, vcrFile)) {
        return;
    }
    char model[0x104];
    char rider[0x104];
    char riderName[0x40];
    char bikeName[0x40];
    sprintf(model, "%s\\%s", "Res", "Ghost.mcf");
    sprintf(rider, "%s\\GhostRider.mcf", "Res");
    g_TrackGame->ui->ResolveBikeFile(g_TrackGame->mode.racerChoice.bikeName,
                                                     riderName, 0x3f);
    g_TrackGame->ui->ResolveRiderFile(g_TrackGame->mode.racerChoice.riderName,
                                                     bikeName, 0x3f);
    replayVcr->SetRacer(0, 0, 0, g_TrackGame->mode.profileName, model, rider,
                                       riderName, bikeName, aiRacers[0]->engineSize,
                                       (unsigned char)aiRacers[0]->engineKind,
                                       g_TrackGame->mode.plateNumber);
    AppendChild(replayVcr, -1);
    localRacer->UnknownFunction496e20(replayVcr);
    if (field_0x189 && localRacer->lapCount == 1 && ghostFilePlayed[0] == 0) {
        return;
    }
    ghostVcr = new (__FILE__, 0x1103) KrustyVCR;
    if (ghostVcr == 0) {
        return;
    }
    if (!ghostVcr->OpenRecording(RecorderCallback, 1, ghostFilePlayed, vcrFile)) {
        return;
    }
    localRacer->fastestLap = bestGhostLength = ghostVcr->RecordedLength();
    aiRacers[0]->UnknownFunction496e20(ghostVcr);
    aiRacers[0]->UnknownVirtualSlot5();
    aiRacers[0]->field_0x11b8 = 0;
    ClearAiRacerParts(0);
    AppendChild(ghostVcr, -1);
}

// 0x00421050
void BikeRace::ResetStartCountdown() {
    if (g_TrackGame->playingReplay) {
        startCountdown = 0.0f;
        return;
    }
    int mode = g_TrackGame->mode.raceSettings.raceMode;
    if (mode != 0 && mode != 4) {
        if (thirtySecondMan != 0) {
            startCountdown = 8.2f;
            thirtySecondMan->UnknownVirtualSlot5();
            raceScene->RestartEntry(thirtySecondManEntry, 0.0f, 0, 0);
            if (thirtySecondMan->collisionObject != 0) {
                thirtySecondMan->collisionObject->Release();
                thirtySecondMan->collisionObject = 0;
            }
            raceScene->UnknownFunction4eafd0(thirtySecondManEntry);
        } else {
            startCountdown = 5.2f;
        }
    }
}

// Inline: TrackGame+0x568's ghost bike, reloaded at each use.
static inline UnknownBikeRaceGhost* GhostBike() {
    return (UnknownBikeRaceGhost*)g_TrackGame->tagEvent->ball;
}

// 0x00421d50
int BikeRace::RecordFrame(int a, void* data, int flag, int b, int* keep, int time) {
    *keep = 0;
    field_0x1e4 = a;
    if (replayPaused && a == -1) {
        replayTime = ((UnknownBikeRaceVcrTick*)data)->time;
        replayStepFrame = false;
        return 5;
    }
    if (g_TrackGame->uiInteractionBlocked) {
        return 0;
    }
    if (a == -2) {
        ClearRacerParts();
        g_TrackGame->replaySpeed = 1.0f;
        if (g_TrackGame->ui->field_0x4a8) {
            g_TrackGame->ui->demoEnded = 1;
            return 1;
        }
        if (g_TrackGame->mode.raceSettings.raceMode == 4) {
            return 1;
        }
        replayTime = replayVcr->timePlayed;
        if (replaySeekTarget != -1.0f && replayTime > replaySeekTarget) {
            replayMode = 11;
            ToggleReplayPause(0);
            return 11;
        }
        if (replayMode == 12) {
            replayMode = 4;
            replayTime = 0;
            ToggleReplayPause(0);
            return 12;
        }
        ToggleReplayPause(1);
        if (replayMode == 11) {
            ToggleReplayPause(0);
            return 11;
        }
        return 1;
    } else if (a == -3) {
        ClearRacerParts();
        g_TrackGame->replaySpeed = 1.0f;
        if (replayMode == 14) {
            ToggleReplayPause(1);
            replayMode = 4;
            return 1;
        }
        if (replaySeekTarget != -1.0f && replaySeekTarget > replayTime) {
            replayMode = 9;
            g_TrackGame->replaySpeed = 15.0f;
            ToggleReplayPause(0);
            return 1;
        }
        if (replaySeekTarget != -1.0f) {
            g_TrackGame->replaySpeed = 1.0f;
            replaySeekTarget = -1.0f;
        }
        replayTime = 0;
        replayMode = 4;
        return 1;
    } else if (a == -1) {
        UnknownBikeRaceVcrTick* tick = (UnknownBikeRaceVcrTick*)data;
        if (replayStepFrame) {
            replayTime = tick->time;
            ToggleReplayPause(0);
            return 7;
        }
        float step = replayTime / (float)replayFrames;
        float tickStep = tick->time / (float)tick->tickCount;
        if (replaySeekTarget != -1.0f) {
            if ((replayMode == 9 && replaySeekTarget < replayTime) ||
                (replayMode == 11 && replaySeekTarget > replayTime)) {
                ClearRacerParts();
                replayMode = 4;
                g_TrackGame->replaySpeed = 1.0f;
                replaySeekTarget = -1.0f;
            } else if (replaySeekTarget > replayTime) {
                replayMode = 9;
                g_TrackGame->replaySpeed = 15.0f;
            } else {
                replayMode = 11;
                replayTime = tick->time;
                return 11;
            }
        }
        if (replayMode == 11) {
            replayTime = tick->time;
            return 11;
        }
        if (replayMode == 12) {
            replayTime = 0;
            replayMode = 14;
            return 12;
        }
        if (replayMode == 14) {
            if (replayTime > 0.5f) {
                ToggleReplayPause(0);
                ClearRacerParts();
                return 5;
            }
            return 1;
        }
        if (appliedReplayMode != replayMode) {
            replayTime = s_UnknownStatic578e8c = tick->time;
            ClearRacerParts();
        }
        if (tick->time > replayTime) {
            s_UnknownStatic578e8c = tick->time;
            return 3;
        }
        if (replayTime - step > s_UnknownStatic578e8c && tickStep + tick->time < replayTime) {
            s_UnknownStatic578e8c = tick->time;
            return 2;
        }
        s_UnknownStatic578e8c = tick->time;
        return 1;
    } else if (a == 6) {
        if (replayPaused) {
            ToggleReplayPause(0);
        }
        return replayMode == 11 ? 11 : 1;
    } else if (a == 1) {
        UnknownBikeRaceNetState* state = (UnknownBikeRaceNetState*)data;
        UnknownBikeRaceRacer* racer = localRacer;
        if (b == racer->recorderId && racer->netIndex == state->racerIndex) {
            if (g_TrackGame->mode.raceSettings.raceMode == 4) {
                racer = aiRacers[0];
            }
            if (racer == 0) {
                return 0;
            }
            racer->field_0x11b8 = 1;
            UnknownBikeRaceRacerPart* oldest;
            if (g_TrackGame->mode.raceSettings.raceMode == 4) {
                oldest = racer->receivedStates[2];
            } else {
                oldest = racer->receivedStates[3];
                racer->receivedStates[3] = racer->receivedStates[2];
            }
            racer->receivedStates[2] = racer->receivedStates[1];
            racer->receivedStates[1] = racer->receivedStates[0];
            racer->receivedStates[0] = oldest;
            oldest->messageFlags = time;
            racer->receivedStates[0]->state = *state;
            racer->receivedStates[0]->state.field_0x4c = (int)(s_UnknownStatic578e8c * 1000.0f);
            racer->receivedStates[0]->fresh = 1;
            racer->field_0x1358 = state->field_0x3c;
            racer->field_0x1364 = state->field_0x08;
            racer->field_0x1370 = state->field_0x30;
            racer->field_0x137c = state->field_0x14;
            racer->field_0x1380 = state->field_0x18;
            racer->field_0x1384 = state->field_0x1c;
            racer->field_0x1388 = replayMode == 11 ? time : state->field_0x4c;
            if (ghostVcr == 0) {
                racer->fastestLap = state->field_0x28;
                racer->field_0x770 = state->field_0x2c;
            }
            *keep = 1;
            if (replayMode == 11) {
                racer->receivedStates[1]->state = racer->receivedStates[0]->state;
                racer->receivedStates[2]->state = racer->receivedStates[1]->state;
                racer->receivedStates[3]->state = racer->receivedStates[2]->state;
                racer->receivedStates[1]->messageFlags = racer->receivedStates[0]->messageFlags;
                racer->receivedStates[2]->messageFlags = racer->receivedStates[1]->messageFlags;
                racer->receivedStates[3]->messageFlags = racer->receivedStates[2]->messageFlags;
                racer->receivedStates[1]->fresh = 1;
                racer->receivedStates[2]->fresh = 1;
                racer->receivedStates[3]->fresh = 1;
                return 11;
            }
            return 1;
        }
        for (int i = 0; i < g_TrackGame->mode.raceSettings.aiRacerCount; i++) {
            if (aiRacers[i] != 0 && b == aiRacers[i]->recorderId &&
                aiRacers[i]->netIndex == state->racerIndex) {
                aiRacers[i]->field_0x11b8 = 1;
                UnknownBikeRaceRacerPart* oldest;
                if (g_TrackGame->mode.raceSettings.raceMode == 4) {
                    oldest = aiRacers[i]->receivedStates[2];
                } else {
                    oldest = aiRacers[i]->receivedStates[3];
                    aiRacers[i]->receivedStates[3] = aiRacers[i]->receivedStates[2];
                }
                aiRacers[i]->receivedStates[2] = aiRacers[i]->receivedStates[1];
                aiRacers[i]->receivedStates[1] = aiRacers[i]->receivedStates[0];
                aiRacers[i]->receivedStates[0] = oldest;
                aiRacers[i]->receivedStates[0]->messageFlags = time;
                aiRacers[i]->receivedStates[0]->state = *state;
                aiRacers[i]->receivedStates[0]->state.field_0x4c = (int)(s_UnknownStatic578e8c * 1000.0f);
                aiRacers[i]->receivedStates[0]->fresh = 1;
                aiRacers[i]->field_0x1358 = state->field_0x3c;
                aiRacers[i]->field_0x1364 = state->field_0x08;
                aiRacers[i]->field_0x1370 = state->field_0x30;
                aiRacers[i]->field_0x137c = state->field_0x14;
                aiRacers[i]->field_0x1380 = state->field_0x18;
                aiRacers[i]->field_0x1384 = state->field_0x1c;
                aiRacers[i]->field_0x1388 = replayMode == 11 ? time : state->field_0x4c;
                if (ghostVcr == 0) {
                    aiRacers[i]->fastestLap = state->field_0x28;
                    aiRacers[i]->field_0x770 = state->field_0x2c;
                }
                *keep = 1;
                if (replayMode == 11) {
                    aiRacers[i]->receivedStates[1]->state = aiRacers[i]->receivedStates[0]->state;
                    aiRacers[i]->receivedStates[2]->state = aiRacers[i]->receivedStates[1]->state;
                    aiRacers[i]->receivedStates[3]->state = aiRacers[i]->receivedStates[2]->state;
                    aiRacers[i]->receivedStates[1]->messageFlags = aiRacers[i]->receivedStates[0]->messageFlags;
                    aiRacers[i]->receivedStates[2]->messageFlags = aiRacers[i]->receivedStates[1]->messageFlags;
                    aiRacers[i]->receivedStates[3]->messageFlags = aiRacers[i]->receivedStates[2]->messageFlags;
                    aiRacers[i]->receivedStates[1]->fresh = 1;
                    aiRacers[i]->receivedStates[2]->fresh = 1;
                    aiRacers[i]->receivedStates[3]->fresh = 1;
                    return 11;
                }
                return 1;
            }
        }
    } else if (a == 13) {
        UnknownBikeRaceNetMessage13* message = (UnknownBikeRaceNetMessage13*)data;
        if (flag) {
            return 1;
        }
        UnknownBikeRaceRacer* racer = localRacer;
        if (b == racer->recorderId && racer->netIndex == message->racerIndex) {
            if (g_TrackGame->mode.raceSettings.raceMode == 4) {
                racer = aiRacers[0];
            }
            if (racer == 0 || !racer->field_0x11b8) {
                return 0;
            }
            UnknownBikeRaceRacerPart* oldest;
            if (g_TrackGame->mode.raceSettings.raceMode == 4) {
                oldest = racer->receivedStates[2];
            } else {
                oldest = racer->receivedStates[3];
                racer->receivedStates[3] = racer->receivedStates[2];
            }
            racer->receivedStates[2] = racer->receivedStates[1];
            racer->receivedStates[1] = racer->receivedStates[0];
            racer->receivedStates[0] = oldest;
            oldest->messageFlags = time;
            racer->UnknownFunction4933e0(message, racer->receivedStates[0]);
            racer->receivedStates[0]->state.field_0x4c = (int)(s_UnknownStatic578e8c * 1000.0f);
            racer->receivedStates[0]->fresh = 0;
            return 1;
        }
        int i = 0;
        if (g_TrackGame->mode.raceSettings.raceMode == 4) {
            return 1;
        }
        for (; i < g_TrackGame->mode.raceSettings.aiRacerCount; i++) {
            if (aiRacers[i] != 0 && b == aiRacers[i]->recorderId &&
                aiRacers[i]->netIndex == message->racerIndex) {
                if (!aiRacers[i]->field_0x11b8) {
                    return 0;
                }
                UnknownBikeRaceRacerPart* oldest = aiRacers[i]->receivedStates[3];
                aiRacers[i]->receivedStates[3] = aiRacers[i]->receivedStates[2];
                aiRacers[i]->receivedStates[2] = aiRacers[i]->receivedStates[1];
                aiRacers[i]->receivedStates[1] = aiRacers[i]->receivedStates[0];
                aiRacers[i]->receivedStates[0] = oldest;
                aiRacers[i]->receivedStates[0]->messageFlags = time;
                aiRacers[i]->UnknownFunction4933e0(message, aiRacers[i]->receivedStates[0]);
                aiRacers[i]->receivedStates[0]->state.field_0x4c = (int)(s_UnknownStatic578e8c * 1000.0f);
                aiRacers[i]->receivedStates[0]->fresh = 0;
                return 1;
            }
        }
    } else if (a == 10) {
        UnknownBikeRaceNetScore* score = (UnknownBikeRaceNetScore*)data;
        UnknownBikeRaceRacer* racer = localRacer;
        if (b == racer->recorderId && racer->netIndex == score->racerIndex) {
            if (racer == 0) {
                return 0;
            }
            racer->score = score->field_0x04;
            *keep = 1;
            return 1;
        }
        for (int i = 0; i < g_TrackGame->mode.raceSettings.aiRacerCount; i++) {
            if (aiRacers[i] != 0 && b == aiRacers[i]->recorderId &&
                aiRacers[i]->netIndex == score->racerIndex) {
                aiRacers[i]->score = score->field_0x04;
                *keep = 1;
                return 1;
            }
        }
    } else if (a == 0x11) {
        UnknownBikeRaceGhostRecord* record = (UnknownBikeRaceGhostRecord*)data;
        if (g_TrackGame->tagEvent != 0 && g_TrackGame->tagEvent->ball != 0) {
            GhostBike()->hasState = 1;
            UnknownBikeRaceGhostPart* oldest = GhostBike()->receivedStates[3];
            GhostBike()->receivedStates[3] = GhostBike()->receivedStates[2];
            GhostBike()->receivedStates[2] = GhostBike()->receivedStates[1];
            GhostBike()->receivedStates[1] = GhostBike()->receivedStates[0];
            GhostBike()->receivedStates[0] = oldest;
            GhostBike()->receivedStates[0]->recordTime = time;
            GhostBike()->receivedStates[0]->record = *record;
            GhostBike()->receivedStates[0]->record.runningTime = (int)(s_UnknownStatic578e8c * 1000.0f);
            GhostBike()->receivedStates[0]->fresh = 1;
            GhostBike()->position = record->position;
            GhostBike()->field_0x5e0 = record->field_0x04;
            GhostBike()->field_0x5ec = record->field_0x1c;
            GhostBike()->field_0x5f8 = record->field_0x10;
            GhostBike()->field_0x5fc = record->field_0x14;
            GhostBike()->field_0x600 = record->field_0x18;
            GhostBike()->runningTime = replayMode == 11 ? time : record->runningTime;
            *keep = 1;
            if (replayMode == 11) {
                GhostBike()->receivedStates[1]->record = GhostBike()->receivedStates[0]->record;
                GhostBike()->receivedStates[2]->record = GhostBike()->receivedStates[1]->record;
                GhostBike()->receivedStates[3]->record = GhostBike()->receivedStates[2]->record;
                GhostBike()->receivedStates[1]->recordTime = GhostBike()->receivedStates[0]->recordTime;
                GhostBike()->receivedStates[2]->recordTime = GhostBike()->receivedStates[1]->recordTime;
                GhostBike()->receivedStates[3]->recordTime = GhostBike()->receivedStates[2]->recordTime;
                GhostBike()->receivedStates[1]->fresh = 1;
                GhostBike()->receivedStates[2]->fresh = 1;
                GhostBike()->receivedStates[3]->fresh = 1;
                return 11;
            }
            return 1;
        }
    } else if (a == 0x10) {
        if (flag) {
            return 1;
        }
        if (g_TrackGame->tagEvent != 0 && g_TrackGame->tagEvent->ball != 0 &&
            GhostBike()->hasState) {
            UnknownBikeRaceGhostPart* oldest = GhostBike()->receivedStates[3];
            GhostBike()->receivedStates[3] = GhostBike()->receivedStates[2];
            GhostBike()->receivedStates[2] = GhostBike()->receivedStates[1];
            GhostBike()->receivedStates[1] = GhostBike()->receivedStates[0];
            GhostBike()->receivedStates[0] = oldest;
            GhostBike()->receivedStates[0]->recordTime = time;
            GhostBike()->ApplyReplayDelta((UnknownBikeRaceGhostDelta*)data,
                                                          GhostBike()->receivedStates[0]);
            GhostBike()->receivedStates[0]->record.runningTime = (int)(s_UnknownStatic578e8c * 1000.0f);
            GhostBike()->receivedStates[0]->fresh = 0;
            return 1;
        }
    } else if (a == 0x87) {
        UnknownBikeRaceVcrFollow* follow = (UnknownBikeRaceVcrFollow*)data;
        if (g_TrackGame->tagEvent != 0) {
            g_TrackGame->tagEvent->followedRacer = 0;
            if (follow->field_0x08 != 0) {
                int iterator = 0;
                for (UnknownBikeRaceRacer* racer = NextBike(&iterator); racer != 0;
                     racer = NextBike(&iterator)) {
                    if (racer->recorderId == follow->field_0x08) {
                        g_TrackGame->tagEvent->followedRacer = (UnknownEventRacer*)racer;
                        break;
                    }
                }
            }
        }
    }
    return 0;
}

// 0x00423040
void BikeRace::ClearAiRacerParts(int index) {
    UnknownBikeRaceRacer* racer = aiRacers[index];
    if (racer != 0) {
        racer->receivedStates[0]->state.field_0x08 = kVec3Zero;
        racer->receivedStates[1]->state.field_0x08 = kVec3Zero;
        racer->receivedStates[2]->state.field_0x08 = kVec3Zero;
        racer->receivedStates[3]->state.field_0x08 = kVec3Zero;
    }
}

// 0x00422ec0
void BikeRace::ClearRacerParts() {
    UnknownBikeRaceRacer* racer = localRacer;
    racer->receivedStates[0]->state.field_0x08 = kVec3Zero;
    racer->receivedStates[1]->state.field_0x08 = kVec3Zero;
    racer->receivedStates[2]->state.field_0x08 = kVec3Zero;
    racer->receivedStates[3]->state.field_0x08 = kVec3Zero;
    for (int i = 0; i < UnknownFunctionRacerCount() - 1; i++) {
        racer = aiRacers[i];
        if (racer != 0) {
            racer->receivedStates[0]->state.field_0x08 = kVec3Zero;
            racer->receivedStates[1]->state.field_0x08 = kVec3Zero;
            racer->receivedStates[2]->state.field_0x08 = kVec3Zero;
            racer->receivedStates[3]->state.field_0x08 = kVec3Zero;
        }
    }
}

// 0x004230e0
int RecorderCallback(int a, void* data, int flag, int b, int* keep, int time,
                     int* milliseconds) {
    BikeRace* race = (BikeRace*)g_TrackGame->eventManager->FindRaceView();
    *milliseconds = (int)(race->replayTime * 1000.0f);
    return race->RecordFrame(a, data, flag, b, keep, time);
}

// 0x00423140
void BikeRace::DrawTrack(Track* track) {
    TrackListItem* list = 0;
    TrackListItem* item =
        (TrackListItem*)DebugCalloc(1, sizeof(TrackListItem), __FILE__, 0x139d);
    if (item == 0) {
        return;
    }
    item->node = track->startNode;
    item->field_0x0c = list;
    list = item;
    while (list != 0) {
        if (list->node->flags & 4) {
            item = list;
            list = list->field_0x0c;
            item->node->flags &= ~4;
            DebugFree(item, __FILE__, 0x13a8);
        } else {
            TrackSegment* segment = list->node->firstSegment;
            while (segment != 0 && segment->next != 0) {
                Vector3 a;
                Vector3 b;
                SetDrawColor(0, 0xff, 0);
                a = *(Vector3*)&segment->edgeAX;
                b = *(Vector3*)&segment->next->edgeAX;
                field_0x04c->QueryGround(&a, 0, 0, 0);
                field_0x04c->QueryGround(&b, 0, 0, 0);
                a.y += 5.0f;
                b.y += 5.0f;
                DrawLine(&a, &b);
                a = *(Vector3*)&segment->edgeBX;
                b = *(Vector3*)&segment->next->edgeBX;
                field_0x04c->QueryGround(&a, 0, 0, 0);
                field_0x04c->QueryGround(&b, 0, 0, 0);
                a.y += 5.0f;
                b.y += 5.0f;
                DrawLine(&a, &b);
                SetDrawColor(0, 0, 0xff);
                a = *(Vector3*)&segment->pointX;
                b = *(Vector3*)&segment->next->pointX;
                field_0x04c->QueryGround(&a, 0, 0, 0);
                field_0x04c->QueryGround(&b, 0, 0, 0);
                a.y += 5.0f;
                b.y += 5.0f;
                DrawLine(&a, &b);
                segment = segment->next;
            }
            item = list;
            item->node->flags |= 4;
            for (int i = 0; i < item->node->linkCount; i++) {
                TrackNode* link = item->node->links[i];
                if (!(link->flags & 4)) {
                    TrackListItem* next =
                        (TrackListItem*)DebugCalloc(1, sizeof(TrackListItem), __FILE__, 0x13c9);
                    if (next == 0) {
                        track->FreeWorkList(&list, 0);
                        return;
                    }
                    next->node = item->node->links[i];
                    next->field_0x0c = list;
                    list = next;
                }
            }
        }
    }
}

// 0x00423410
int BikeRace::UnknownVirtualSlot14() {
    Vector3 a;
    Vector3 b;
    if (field_0x3fd) {
        GameObject::UnknownVirtualSlot14();
    }
    if (field_0x034) {
        if (raceTrack != 0) {
            DrawTrack(raceTrack);
            if (hasStartProbe) {
                SetDrawColor(0xff, 0, 0xff);
                a = startProbe.center - startProbe.halfExtents;
                b = startProbe.center + startProbe.halfExtents;
                a.y += 2.5f;
                b.y += 2.5f;
                DrawLine(&a, &b);
            }
            if (hasFinishProbe) {
                SetDrawColor(0, 0xff, 0xff);
                a = finishProbe.center - finishProbe.halfExtents;
                b = finishProbe.center + finishProbe.halfExtents;
                a.y += 2.5f;
                b.y += 2.5f;
                DrawLine(&a, &b);
            }
        }
        if (field_0x034) {
            if (g_UnknownGlobal577a00) {
                SetDrawColor(0xff, 0, 0);
            } else {
                SetDrawColor(0, 0xff, 0);
            }
            DrawLine(&g_UnknownGlobal578de0[0], &g_UnknownGlobal578de0[1]);
            if (g_UnknownGlobal578da8) {
                SetDrawColor(0xff, 0, 0x80);
            } else {
                SetDrawColor(0, 0xff, 0x80);
            }
            DrawLine(&g_UnknownGlobal577a30[0], &g_UnknownGlobal577a30[1]);
            if (g_UnknownGlobal578dac) {
                SetDrawColor(0xff, 0, 0xff);
            } else {
                SetDrawColor(0, 0xff, 0xff);
            }
            DrawLine(&g_UnknownGlobal577a48[0], &g_UnknownGlobal577a48[1]);
            if (g_UnknownGlobal578df8) {
                SetDrawColor(0xff, 0, 0xff);
            } else {
                SetDrawColor(0, 0xff, 0xff);
            }
            DrawLine(&g_UnknownGlobal5779d8[0], &g_UnknownGlobal5779d8[1]);
            if (g_UnknownGlobal578dd8) {
                SetDrawColor(0xff, 0, 0xff);
            } else {
                SetDrawColor(0, 0xff, 0xff);
            }
            DrawLine(&g_UnknownGlobal577a18[0], &g_UnknownGlobal577a18[1]);
            SetDrawColor(0, 0, 0xff);
            DrawLine(&g_UnknownGlobal689c30[0], &g_UnknownGlobal689c30[1]);
            SetDrawColor(0xff, 0, 0xff);
            DrawLine(&g_UnknownGlobal689c48[0], &g_UnknownGlobal689c48[1]);
            SetDrawColor(0xff, 0, 0);
            DrawLine(&g_UnknownGlobal578dc0[0], &g_UnknownGlobal578dc0[1]);
        }
    }
    return 1;
}

// 0x00423790
void BikeRace::SetDetailLevel(int level) {
    g_UnknownGlobal689f18 = g_TrackGame->field_0x2d0 ? g_UnknownGlobal5744c8 : g_UnknownGlobal574428;
    localRacer->modelNode->ApplyLodSettings(level);
    localRacer->riderCharacter->modelNode->ApplyLodSettings(level);
    if (g_TrackGame->mode.raceSettings.raceMode != 4) {
        for (int i = 0; i < g_TrackGame->mode.raceSettings.aiRacerCount; i++) {
            aiRacers[i]->modelNode->ApplyLodSettings(level);
            aiRacers[i]->riderCharacter->modelNode->ApplyLodSettings(level);
        }
    }
    for (int j = 0; j < g_TrackGame->mode.raceSettings.networkPlayerCount; j++) {
        racerSlots[j]->modelNode->ApplyLodSettings(level);
        racerSlots[j]->riderCharacter->modelNode->ApplyLodSettings(level);
    }
}

// 0x00423880
int BikeRace::UnknownVirtualSlot22(UnknownControlEvent* event, UnknownInputEntry* entry) {
    if (TestInputEvent(6, 0, event, 0x80)) {
        field_0x3fd = 1 - field_0x3fd;
        return 1;
    }
    return 0;
}
