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
void UnknownFunction417b00(CollisionObject* self, CollisionObject* other) {
    BikeRace* race = (BikeRace*)g_TrackGame->eventManager->FindRaceView();
    if (race != 0 && race->objectPicker != 0) {
        race->objectPicker->field_0x2c = (int)other;
    }
}

// 0x00417b30
void UnknownFunction417b30(int* keys, int* values, int count) {
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
void BikeRace::UnknownFunction417bc0(int* order) {
    int points[11];
    for (int i = 0; i < racerCount; i++) {
        points[i] = g_TrackGame->eventManager->field_0x50[i].field_0x28;
        order[i] = i + 1;
    }
    UnknownFunction417b30(points, order, racerCount);
}

// 0x00417c30
BikeRace::BikeRace(int flags) : GraphicsTest(flags) {
    startProbe.field_0x38 = 0;
    finishProbe.field_0x38 = 0;
    field_0x1e8 = 0;
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
    field_0x060 = 0;
    field_0x0bc = 0;
    field_0x0c0 = 0;
    raceTrack = 0;
    field_0x0c8 = 0;
    field_0x14c = 0;
    field_0x150 = 0;
    field_0x154 = 0;
    field_0x15c = 0;
    isRacing = false;
    field_0x18b = false;
    trackLoaded = false;
    racerCount = 0;
    field_0x188 = false;
    field_0x18f = false;
    if (g_TrackGame->field_0x18 > 1) {
        field_0x18e = true;
    } else {
        field_0x18e = false;
    }
    field_0x190 = false;
    field_0x194 = 0;
    field_0x198 = g_TrackGame->mode.field_0x6b4;
    field_0x064 = 0;
    field_0x06c = 0;
    flagGirl = 0;
    field_0x068 = -1;
    field_0x070 = -1;
    field_0x189 = 0;
    field_0x160 = 0;
    field_0x1ac = 0;
    field_0x1b0 = 0;
    field_0x1b4 = 0;
    replayLength = 0;
    replayTime = 0;
    field_0x1d8 = 0;
    field_0x1d4 = 0;
    field_0x1c8 = FLT_MAX;
    replayVcr = 0;
    ghostVcr = 0;
    field_0x1a8 = 0;
    g_TrackGame->field_0x2e0 = 1.0f;
    field_0x3f9 = false;
    ghostTimeLimit = (float)g_TrackGame->GetRegistryInt("VCRGhostTimeLimit", 300);
    recordTimeLimit = (float)g_TrackGame->GetRegistryInt("VCRRecordTimeLimit", 3600);
    recorderTickElapsed = 0;
    field_0x3f8 = false;
    field_0x3f9 = false;
    replayMode = 4;
    field_0x1e0 = 4;
    field_0x1e4 = 0;
    replaySeekTarget = -1.0f;
    field_0x3fa = false;
    field_0x3fb = false;
    isRecording = 0;
    ghostFilePlayed[0] = 0;
    ghostFileRecorded[0] = 0;
    g_TrackGame->field_0x1c4 = 0;
    field_0x144 = 0;
    field_0x148 = 0;
    field_0x19c = 0;
    forceHighLod = g_TrackGame->GetRegistryFlag("ForceHighLOD", 0);
    field_0x3fd = 1;
    field_0x0a8 = -1;
    field_0x0b0 = 0xff;
    field_0x0ac = 0;
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
        if (g_TrackGame->UnknownFunction521cd0()) {
            DebugFree(aiRacers, __FILE__, 0x809);
        } else {
            delete aiRacers;
        }
        aiRacers = 0;
    }
    Track* track = raceTrack;
    if (track != 0) {
        if (track->field_0x00 != 0) {
            track->UnknownFunction516870(&track->field_0x00);
        }
        delete track;
    }
    UnknownFunctionFreeNodes();
    if (field_0x0c4 != 0) {
        FreeStatusList(field_0x0c4);
        field_0x0c4 = 0;
    }
    if (field_0x1a8 != 0) {
        delete field_0x1a8;
        field_0x1a8 = 0;
        if (g_TrackGame->field_0x3428) {
            UnknownFunctionCameraView()->field_0x244 = field_0x1ec;
            raceCamera->UnknownVirtualSlot61();
        }
        g_TrackGame->field_0x3428 = 0;
        g_TrackGame->field_0x342c = 0;
    }
    while (field_0x0c8 != 0) {
        UnknownBikeRaceNode* node = field_0x0c8;
        field_0x0c8 = node->field_0x38;
        DebugFree(node, __FILE__, 0x82e);
    }
}

// 0x0041d260
int BikeRace::UnknownVirtualSlot16(int value) {
    GameObject::UnknownVirtualSlot16(value);
    JoystickDevice* joystick = g_TrackGame->controlInterface->activeJoystick;
    if (joystick != 0 && g_TrackGame->mode.field_0xa88) {
        joystick->UnknownVirtualSlot18(value);
    }
    return 1;
}

// 0x0041d0d0
int BikeRace::UnknownFunction41d0d0(int index, UnknownBikeRaceNodeSource* source,
                                    UnknownBikeRaceNode*** tail, float scale, int value) {
    UnknownBikeRaceNode* node =
        (UnknownBikeRaceNode*)DebugCalloc(1, sizeof(UnknownBikeRaceNode), __FILE__, 0x840);
    if (node == 0) {
        return 0;
    }
    UnknownBikeRaceNodeSource* record = &source[index];
    node->field_0x00.x = record->field_0x00.x;
    node->field_0x00.y = record->field_0x00.y;
    node->field_0x00.z = record->field_0x00.z;
    node->field_0x0c.x = record->field_0x0c.x;
    node->field_0x0c.y = record->field_0x0c.y;
    node->field_0x0c.z = record->field_0x0c.z;
    node->field_0x24 = scale;
    node->field_0x28 = value;
    node->field_0x18.y = 0.0f;
    node->field_0x18.x = scale * node->field_0x0c.z * 0.5f;
    node->field_0x18.z = scale * node->field_0x0c.x * -0.5f;
    **tail = node;
    *tail = &node->field_0x38;
    return 1;
}

// 0x0041d170
int BikeRace::UnknownFunction41d170(UnknownBikeRaceNodeOwner* owner) {
    int count = owner->field_0x0ac;
    UnknownBikeRaceNodeSource* sources = owner->field_0x0d8;
    UnknownBikeRaceNode** tail = &field_0x0c8;
    for (int i = 0; i < count; i++) {
        UnknownBikeRaceNodeEntry* entry = owner->field_0x420[i];
        if (!UnknownFunction41d0d0(i, sources, &tail, entry->field_0x50,
                                   entry->field_0x54)) {
            return 0;
        }
    }
    return 1;
}

// 0x0041d1e0
int BikeRace::UnknownFunction41d1e0() {
    if (raceTrack != 0 && raceTrack->UnknownFunction517da0(finishProbe.field_0x2c, startProbe.field_0x2c) > 0.0f) {
        field_0x189 = 0;
    } else {
        field_0x189 = 1;
    }
    return field_0x189;
}

// 0x0041d2a0
void BikeRace::UnknownFunction41d2a0(float time) {
    raceScene->UnknownFunction4eb000(time);
}

// Inline: the racers in the race, own and AI (a single player race) or
// own and remote.
static inline int UnknownFunctionRacerCount() {
    if (g_TrackGame->field_0x18 == 1) {
        if (g_TrackGame->mode.field_0x27f8.field_0x00 == 4) {
            return g_TrackGame->field_0x18;
        }
        return g_TrackGame->mode.field_0x27f8.field_0x24 + 1;
    }
    return g_TrackGame->mode.field_0x27f8.field_0x28 + g_TrackGame->field_0x18;
}

static inline float UnknownFunctionMax(float a, float b) {
    return a > b ? a : b;
}

// 0x0041d2b0
int BikeRace::UnknownVirtualSlot10(float frameTime) {
    if (!g_TrackGame->uiInteractionBlocked) {
        if (UnknownFunctionCameraView()->field_0x390 &&
            UnknownFunctionCameraView()->field_0x3b0->field_0x4a0 != 0) {
            UnknownFunction41f1d0(0, 1, 0);
        }
        if (!g_TrackGame->field_0x1c4) {
            if (g_TrackGame->mode.field_0xa88) {
                localRacer->UnknownRacerVirtualSlot94();
            }
            if (g_TrackGame->mode.field_0x27f8.field_0x00 == 4) {
                unsigned short lap = localRacer->field_0x7a0;
                if (field_0x1e8 != lap && (!field_0x189 || lap > 0)) {
                    UnknownFunction420bd0();
                    field_0x1e8 = localRacer->field_0x7a0;
                    replayTime = 0;
                    field_0x1d4 = 0;
                    replayLength = 0;
                    field_0x1d8 = 0;
                    field_0x1ac = 0;
                    field_0x1b0 = 0;
                    field_0x1b4 = 0;
                    localRacer->field_0x13f8 = FLT_MAX / 2;
                } else {
                    replayTime += g_TrackGame->frameTime;
                    field_0x1d4++;
                    if (replayLength != ghostTimeLimit) {
                        replayLength += g_TrackGame->frameTime;
                        field_0x1d8++;
                    }
                    if (replayLength > ghostTimeLimit) {
                        replayLength = ghostTimeLimit;
                        TextQueueOverlay* overlay = g_TrackGame->eventManager->FindTextQueue();
                        char text[0x100];
                        g_TrackGame->LoadResourceString(0x14de, text, 0x80);
                        UnknownMessage* message = new (__FILE__, 0x8bf) UnknownMessage(text, 3.25f);
                        if (message != 0) {
                            overlay->UnknownFunction51b540(message);
                            delete message;
                        }
                    } else {
                        recorderTickElapsed += g_TrackGame->frameTime;
                        field_0x3fa = recorderTickElapsed >= g_TrackGame->shortRecordPacketIntervalSeconds;
                        if (field_0x3fa) {
                            recorderTickElapsed -= g_TrackGame->shortRecordPacketIntervalSeconds;
                            field_0x1ac = replayLength;
                            field_0x1b0++;
                            field_0x1b4 = 1;
                            if (replayVcr != 0) {
                                replayVcr->QueueRecord(-1, 0, &field_0x1ac, 0xc);
                            }
                        }
                    }
                }
            }
            if (g_TrackGame->field_0x342c) {
                if (!field_0x3f8) {
                    if (replayMode == 8) {
                        replayTime += g_TrackGame->frameTime * g_TrackGame->field_0x2e0;
                        field_0x1d4 += (int)g_TrackGame->field_0x2e0;
                    } else if (replayMode == 9) {
                        replayTime += g_TrackGame->frameTime * g_TrackGame->field_0x2e0;
                        field_0x1d4 += (int)g_TrackGame->field_0x2e0;
                    } else if (replayMode != 11) {
                        replayTime += g_TrackGame->frameTime;
                        field_0x1d4++;
                        if (replayLength != recordTimeLimit) {
                            replayLength += g_TrackGame->frameTime;
                            field_0x1d8++;
                        }
                    }
                    replayVcr->UnknownFunction49bff0(replayLength, 0);
                    if (replayMode == 4) {
                        if (field_0x1e0 != 4) {
                            field_0x1e0 = 4;
                            if (g_TrackGame->mode.field_0xa28) {
                                if (g_TrackGame->eventManager->FindRaceMode()->field_0x9c != 0) {
                                    g_TrackGame->eventManager->FindRaceMode()->field_0x9c->UnknownVirtualSlot5();
                                }
                                ((UnknownBikeRaceRoot*)g_TrackGame->field_0x34)->UnknownFunction468f10("SoundGroup");
                            }
                            if (g_TrackGame->mode.field_0xa54) {
                                ((UnknownBikeRaceRoot*)g_TrackGame->field_0x34)->UnknownFunction468f10("ParticleManager");
                                ((UnknownBikeRaceRoot*)g_TrackGame->field_0x34)->UnknownFunction468f10("DirtParticleEmitter");
                                ((UnknownBikeRaceRoot*)g_TrackGame->field_0x34)->UnknownFunction468f10("DustParticleEmitter");
                                ((UnknownBikeRaceRoot*)g_TrackGame->field_0x34)->UnknownFunction468f10("DirtChunkParticleEmitter");
                                ((UnknownBikeRaceRoot*)g_TrackGame->field_0x34)->UnknownFunction468f10("SteamParticleEmitter");
                            }
                            if (g_TrackGame->mode.field_0xa50) {
                                ((UnknownBikeRaceRoot*)g_TrackGame->field_0x34)->UnknownFunction468f10("ProjectedShadow");
                                ((UnknownBikeRaceRoot*)g_TrackGame->field_0x34)->UnknownFunction468f10("TerrainShadow");
                                ((UnknownBikeRaceRoot*)g_TrackGame->field_0x34)->UnknownFunction468f10("D3DIMSoultreeShadow");
                            }
                        }
                    } else if (field_0x1e0 != replayMode) {
                        field_0x1e0 = replayMode;
                        if (g_TrackGame->eventManager->FindRaceMode()->field_0x9c != 0) {
                            g_TrackGame->eventManager->FindRaceMode()->field_0x9c->UnknownVirtualSlot4();
                        }
                        ((UnknownBikeRaceRoot*)g_TrackGame->field_0x34)->UnknownFunction468dd0("SoundGroup");
                        ((UnknownBikeRaceRoot*)g_TrackGame->field_0x34)->UnknownFunction468dd0("ParticleManager");
                        ((UnknownBikeRaceRoot*)g_TrackGame->field_0x34)->UnknownFunction468dd0("DirtParticleEmitter");
                        ((UnknownBikeRaceRoot*)g_TrackGame->field_0x34)->UnknownFunction468dd0("DustParticleEmitter");
                        ((UnknownBikeRaceRoot*)g_TrackGame->field_0x34)->UnknownFunction468dd0("DirtChunkParticleEmitter");
                        ((UnknownBikeRaceRoot*)g_TrackGame->field_0x34)->UnknownFunction468dd0("SteamParticleEmitter");
                        ((UnknownBikeRaceRoot*)g_TrackGame->field_0x34)->UnknownFunction468dd0("ProjectedShadow");
                        ((UnknownBikeRaceRoot*)g_TrackGame->field_0x34)->UnknownFunction468dd0("TerrainShadow");
                        ((UnknownBikeRaceRoot*)g_TrackGame->field_0x34)->UnknownFunction468dd0("D3DIMSoultreeShadow");
                    }
                }
                if (!g_TrackGame->field_0x3428 && isRacing) {
                    if (replayLength > recordTimeLimit) {
                        replayLength = recordTimeLimit;
                        TextQueueOverlay* overlay = g_TrackGame->eventManager->FindTextQueue();
                        char text[0x100];
                        g_TrackGame->LoadResourceString(0x14dd, text, 0x80);
                        UnknownMessage* message = new (__FILE__, 0x912) UnknownMessage(text, 3.25f);
                        if (message != 0) {
                            overlay->UnknownFunction51b540(message);
                            delete message;
                        }
                    } else {
                        recorderTickElapsed += g_TrackGame->frameTime;
                        field_0x3fa = recorderTickElapsed >= g_TrackGame->shortRecordPacketIntervalSeconds;
                        if (field_0x3fa) {
                            recorderTickElapsed -= g_TrackGame->shortRecordPacketIntervalSeconds;
                            field_0x1ac = replayLength;
                            field_0x1b0++;
                            TrackGameViewOwner* owner = g_TrackGame->field_0x568;
                            if (owner != 0 && owner->field_0xdc != 0 && owner->field_0xa8 == 0) {
                                field_0x1b4 = UnknownFunctionRacerCount() + 1;
                            } else {
                                field_0x1b4 = UnknownFunctionRacerCount();
                            }
                            int camera = g_TrackGame->mode.field_0x27f8.field_0x04;
                            if (camera == 0 || (camera == 4 && g_TrackGame->mode.field_0x27f8.field_0x148)) {
                                if (field_0x1b0 % 20 == 0) {
                                    field_0x3fb = true;
                                    field_0x1b4 *= 2;
                                } else {
                                    field_0x3fb = false;
                                }
                            }
                            if (replayVcr != 0) {
                                replayVcr->QueueRecord(-1, 0, &field_0x1ac, 0xc);
                            }
                        }
                    }
                }
                if (replayVcr != 0 && replayVcr->field_0xcc) {
                    UnknownFunction41d2a0(replayVcr->field_0xd0);
                    replayVcr->field_0xcc = 0;
                }
            }
            if (field_0x18e) {
                if (!field_0x18f) {
                    UnknownBikeRaceNetStamp stamp;
                    stamp.field_0x04 = UnknownFunction4bfa80();
                    g_TrackGame->network->Send(
                        0x84, &stamp, 8, g_TrackGame->network->localPlayer, 0);
                    field_0x18f = true;
                }
            }
            if (field_0x18e) {
                int ready = 1;
                int self = g_TrackGame->network->localPlayer;
                for (int i = 0; i < g_TrackGame->mode.field_0x1be0; i++) {
                    int id = g_TrackGame->mode.field_0x1be4[i].field_0xd4;
                    if (id != self) {
                        NetPlayer* player = g_TrackGame->network->FindPlayer(id);
                        if (!g_TrackGame->mode.field_0x1be4[i].field_0xc8 && player != 0) {
                            ready = 0;
                        } else {
                            racerSlots[i]->UnknownVirtualSlot5();
                            for (int j = 0; j < g_TrackGame->field_0x3424 + g_TrackGame->field_0x18; j++) {
                                if (racerSlots[j]->field_0x11bc == id) {
                                    racerSlots[j]->UnknownVirtualSlot5();
                                }
                            }
                        }
                        if (player == 0) {
                            racerSlots[i]->UnknownVirtualSlot4();
                            for (int j = 0; j < g_TrackGame->field_0x3424 + g_TrackGame->field_0x18; j++) {
                                if (racerSlots[j]->field_0x11bc == id) {
                                    racerSlots[j]->UnknownVirtualSlot4();
                                }
                            }
                        }
                    }
                }
                if (ready) {
                    UnknownFunction421050();
                    field_0x18e = false;
                    g_TrackGame->network->StopKeepAlive();
                    if (g_TrackGame->field_0x18 > 1) {
                        int count = g_TrackGame->mode.field_0x27f8.field_0x28 + g_TrackGame->field_0x18;
                        for (int k = 0; k < count; k++) {
                            racerSlots[k]->UnknownRacerVirtualSlot50(1, racerSlots[k]->UnknownRacerVirtualSlot45(), 0);
                        }
                    }
                }
            } else {
                Vector3 position;
                if (isRacing) {
                    if (field_0x064 != 0 && ((UnknownBikeRaceSceneCharacter*)field_0x064)->field_0x25_bit0 &&
                        field_0x160 > 0.0f) {
                        field_0x160 -= g_TrackGame->frameTime;
                        if (field_0x160 <= 0.0f) {
                            field_0x160 = 0;
                            raceScene->UnknownFunction4eb040(field_0x068, 0.0f, 0, 0);
                            field_0x064->UnknownVirtualSlot4();
                            if (!UnknownFunctionCameraView()->field_0x390 && field_0x150 == field_0x068) {
                                UnknownFunction41f1d0(1, 1, 0);
                            }
                            if (field_0x064->field_0x210 != 0) {
                                field_0x064->field_0x210->Release();
                                field_0x064->field_0x210 = 0;
                            }
                        }
                    }
                } else {
                    field_0x15c -= g_TrackGame->frameTime;
                    if (field_0x15c <= 0.0f || UnknownFunction41ea10()) {
                        field_0x15c = 0;
                        isRacing = true;
                        raceScene->field_0x7c0 = 0;
                        if (field_0x064 != 0) {
                            raceScene->UnknownFunction4eafd0(field_0x068);
                            field_0x160 = 5.0f;
                        }
                        if (g_TrackGame->field_0x18 > 1) {
                            int self = g_TrackGame->network->localPlayer;
                            int count = g_TrackGame->mode.field_0x27f8.field_0x28 + g_TrackGame->field_0x18;
                            for (int i = 0; i < count; i++) {
                                if (g_TrackGame->mode.field_0x1be4[i].field_0xd4 == self) {
                                    racerSlots[i]->field_0x064 = kVec3Zero;
                                    racerSlots[i]->field_0x0bc = 0;
                                }
                            }
                        }
                        replayTime = 0;
                        field_0x1d4 = 0;
                        replayLength = 0;
                        field_0x1d8 = 0;
                        if (field_0x06c != 0) {
                            raceScene->UnknownFunction4eb040(field_0x070, 0.0f, 0, 0);
                            field_0x06c->UnknownVirtualSlot4();
                            if (!UnknownFunctionCameraView()->field_0x390 && field_0x150 == field_0x070) {
                                UnknownFunction41f1d0(1, 1, 0);
                            }
                            if (field_0x06c->field_0x210 != 0) {
                                field_0x06c->field_0x210->Release();
                                field_0x06c->field_0x210 = 0;
                            }
                        }
                        switch (g_TrackGame->mode.field_0x27f8.field_0x04) {
                        case 2:
                            field_0x19c = (UnknownBikeRaceOverlay19c*)g_TrackGame->field_0x564->field_0x64;
                            break;
                        case 3:
                            field_0x19c = (UnknownBikeRaceOverlay19c*)g_TrackGame->field_0x558->field_0x64;
                            break;
                        case 0:
                            field_0x19c = (UnknownBikeRaceOverlay19c*)g_TrackGame->field_0x55c->field_0x64;
                            break;
                        case 1:
                        case 5:
                            field_0x19c = (UnknownBikeRaceOverlay19c*)g_TrackGame->field_0x560->field_0x64;
                            break;
                        case 4:
                            field_0x19c = (UnknownBikeRaceOverlay19c*)g_TrackGame->field_0x568->field_0x64;
                            break;
                        }
                    }
                }
                GameObject::UnknownVirtualSlot10(frameTime);
                int camera = g_TrackGame->mode.field_0x27f8.field_0x04;
                if (camera == 3 || camera == 2 || camera == 5 || camera == 1 || camera == 0) {
                    Track* track = raceTrack;
                    if ((track != 0 || field_0x0c8 != 0 || camera == 0) && isRacing && !field_0x18b &&
                        localRacer != 0) {
                        int keepRacing = g_TrackGame->mode.field_0x27f8.field_0x00 == 0 ||
                                         g_TrackGame->mode.field_0x27f8.field_0x00 == 4;
                        if (!g_TrackGame->field_0x3428) {
                            if (camera == 1 || camera == 5) {
                                UpdateGateRace(&field_0x0c4, field_0x0c8, g_TrackGame->frameTime,
                                                      g_TrackGame->mode.field_0x27f8.field_0x20, keepRacing);
                            } else if (camera == 0) {
                                RankByScore(keepRacing);
                            } else {
                                UpdateLapRace(&field_0x0c4, track, g_TrackGame->frameTime, &startProbe,
                                                      &finishProbe, g_TrackGame->mode.field_0x27f8.field_0x20,
                                                      keepRacing);
                            }
                        }
                    }
                }
                if (isRacing && field_0x18b) {
                    field_0x18b = false;
                    isRacing = false;
                }
                if (field_0x060 != 0 && ((UnknownBikeRaceObjectFlags*)field_0x060)->field_0x25_bit0) {
                    position = localRacer->field_0x018;
                    position.y += 4.25f;
                    ((ArcadeObject*)field_0x060)->UnknownFunction4014f0(&position);
                }
                if (field_0x194 > 0.0f && g_TrackGame->mode.field_0x6b4 != 1) {
                    field_0x194 -= g_TrackGame->frameTime;
                    if (field_0x194 <= 0.0f) {
                        field_0x194 = 0;
                        if (field_0x19c != 0) {
                            field_0x19c->field_0x3dc = 0;
                            field_0x19c->UnknownFunction51dd40();
                        }
                        field_0x190 = false;
                    }
                }
                if (flagGirl != 0 && localRacer->field_0x744->field_0x44.node != 0) {
                    int laps = g_TrackGame->mode.field_0x27f8.field_0x20;
                    int lap = localRacer->field_0x7a0;
                    int last = lap == laps - 1;
                    int previous = lap == laps - 2;
                    if (previous || last) {
                        float toFinish = raceTrack->UnknownFunction517da0(localRacer->field_0x744->field_0x44,
                                                                            finishProbe.field_0x2c);
                        float fromFinish = raceTrack->UnknownFunction517da0(finishProbe.field_0x2c,
                                                                              localRacer->field_0x744->field_0x44);
                        if (field_0x078 && localRacer->field_0x7a4 && fromFinish > 100.0f) {
                            UnknownFunction41ea60(flagGirl, 1, 2);
                            flagGirl->UnknownFunction4a8b10("Stand");
                            field_0x078 = 0;
                        } else if ((previous && toFinish < 300.0f) || (last && fromFinish < 100.0f)) {
                            if (field_0x078 != 1) {
                                UnknownFunction41ea60(flagGirl, 1, 2);
                                field_0x078 = 1;
                                flagGirl->UnknownFunction4a8b10("FlagStart");
                            } else if (flagGirl->field_0x00c) {
                                flagGirl->UnknownFunction4a8b10("FlagLoop");
                            }
                        } else if (field_0x078 && field_0x078 < 3 &&
                                   ((previous && toFinish > 100.0f) || (last && fromFinish > 100.0f))) {
                            flagGirl->UnknownFunction4a8b10("Stand");
                            field_0x078 = 0;
                        } else if ((last && toFinish < 300.0f) || (localRacer->field_0x7a4 && fromFinish < 100.0f)) {
                            if (field_0x078 < 3) {
                                UnknownFunction41ea60(flagGirl, 2, 1);
                                field_0x078 = 3;
                                flagGirl->UnknownFunction4a8b10("FlagStart");
                            } else if (flagGirl->field_0x00c) {
                                flagGirl->UnknownFunction4a8b10("FlagLoop");
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
    if (g_TrackGame->field_0x2d4_bit2 && g_TrackGame->debugOverlay != 0) {
        if (field_0x0a8 < 0) {
            field_0x0a8 = g_TrackGame->debugOverlay->NewPage();
        }
        if (g_TrackGame->debugOverlay->field_0x26c4 == field_0x0a8) {
            char name[0x20];
            UnknownBikeRaceModel* model;
            int pass = 0;
            if ((signed char)field_0x0b0 < 0) {
                strncpy(name,
                        ((UnknownBikeRaceUiModel*)g_TrackGame->ui->field_0x48)
                            [((UnknownBikeRaceUiChoice*)g_TrackGame->ui->field_0x50)
                                 [g_TrackGame->mode.field_0x1974.field_0xc4].field_0x00].field_0x40,
                        0x14);
                name[0x14] = 0;
                raceScene->UnknownFunction4f0ec0(name);
                model = localRacer->field_0x3bc;
            } else if (field_0x0b0 == 0) {
                strncpy(name, raceScene->field_0xb8->field_0x04[field_0x0ac].field_0x10, 0x14);
                name[0x14] = 0;
                model = (UnknownBikeRaceModel*)raceScene->field_0xb8->field_0x04[field_0x0ac].field_0x04;
            } else {
                strncpy(name, raceScene->field_0xb4->field_0x04[field_0x0ac].field_0x2c, 0x14);
                name[0x14] = 0;
                UnknownSceneEntry* entry = &raceScene->field_0xb4->field_0x04[field_0x0ac];
                if (entry->field_0x00_bit3) {
                    model = ((UnknownBikeRaceRider*)entry->field_0x04)->field_0x1a0;
                } else {
                    model = (UnknownBikeRaceModel*)entry->field_0x08->field_0x34;
                }
            }
            do {
                int faces = 0;
                UnknownBikeRaceMeshGroup* group = &model->field_0x28c[model->field_0x27c];
                for (int i = 0; i < group->field_0x00; i++) {
                    faces += group->field_0x04[i].field_0x0c;
                }
                g_TrackGame->debugOverlay->UnknownFunction447fa0(field_0x0a8, "Soultree LOD (%s)", name);
                g_TrackGame->debugOverlay->UnknownFunction447f40(field_0x0a8, "Current LOD %d out of %d",
                                                                        model->field_0x27c + 1, model->field_0x274);
                g_TrackGame->debugOverlay->UnknownFunction447f40(field_0x0a8, "Number of Faces = %d", faces);
                if ((signed char)field_0x0b0 < 0) {
                    if (g_UnknownGlobal567a88 > 5) {
                        if ((pass == 1 && g_UnknownGlobal567a88 - 7 > model->field_0x274) || g_UnknownGlobal567a88 < 2) {
                            g_TrackGame->debugOverlay->UnknownFunction448000(field_0x0a8, g_UnknownGlobal567a88 + 1, 0);
                            g_UnknownGlobal567a88 = 2;
                        }
                    } else if ((pass == 0 && g_UnknownGlobal567a88 > model->field_0x274) || g_UnknownGlobal567a88 < 2) {
                        g_TrackGame->debugOverlay->UnknownFunction448000(field_0x0a8, g_UnknownGlobal567a88 + 1, 0);
                        g_UnknownGlobal567a88 = 9;
                    }
                } else if (g_UnknownGlobal567a88 > model->field_0x274 || g_UnknownGlobal567a88 < 2) {
                    g_TrackGame->debugOverlay->UnknownFunction448000(field_0x0a8, g_UnknownGlobal567a88 + 1, 0);
                    g_UnknownGlobal567a88 = 2;
                }
                if (model->field_0x274 > 1) {
                    int up = g_TrackGame->controlInterface->UnknownVirtualSlot3(0xd0, 0, 0x80, 0);
                    int down = g_TrackGame->controlInterface->UnknownVirtualSlot3(0xc8, 0, 0x80, 0);
                    int row = g_UnknownGlobal567a88 - 1;
                    if (down || up) {
                        if ((signed char)field_0x0b0 < 0) {
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
                            if (model->field_0x274 > 1) {
                                if (down) {
                                    model->field_0x280[0] += 0.05f;
                                } else {
                                    model->field_0x280[0] -= 0.05f;
                                    model->field_0x280[0] = UnknownFunctionMax(model->field_0x280[0], 0.0f);
                                }
                            }
                            break;
                        case 2:
                            if (model->field_0x274 > 2) {
                                if (down) {
                                    model->field_0x280[1] += 0.05f;
                                } else {
                                    model->field_0x280[1] -= 0.05f;
                                    model->field_0x280[1] = UnknownFunctionMax(model->field_0x280[1], 0.0f);
                                }
                            }
                            break;
                        case 3:
                            if (model->field_0x274 > 3) {
                                if (down) {
                                    model->field_0x280[2] += 0.05f;
                                } else {
                                    model->field_0x280[2] -= 0.05f;
                                    model->field_0x280[2] = UnknownFunctionMax(model->field_0x280[2], 0.0f);
                                }
                            }
                            break;
                        case 4:
                            if (model->field_0x274 > 4) {
                                if (down) {
                                    model->field_0x280[3] += 0.05f;
                                } else {
                                    model->field_0x280[3] -= 0.05f;
                                    model->field_0x280[3] = UnknownFunctionMax(model->field_0x280[3], 0.0f);
                                }
                            }
                            break;
                        }
                    }
                    g_TrackGame->debugOverlay->UnknownFunction447f40(
                        field_0x0a8, "AutoLOD#0 %.2f", model->field_0x274 > 1 ? model->field_0x280[0] : 0.0f);
                    g_TrackGame->debugOverlay->UnknownFunction447f40(
                        field_0x0a8, "AutoLOD#1 %.2f", model->field_0x274 > 2 ? model->field_0x280[1] : 0.0f);
                    g_TrackGame->debugOverlay->UnknownFunction447f40(
                        field_0x0a8, "AutoLOD#2 %.2f", model->field_0x274 > 3 ? model->field_0x280[2] : 0.0f);
                    g_TrackGame->debugOverlay->UnknownFunction447f40(
                        field_0x0a8, "AutoLOD#3 %.2f", model->field_0x274 > 4 ? model->field_0x280[3] : 0.0f);
                    g_TrackGame->debugOverlay->UnknownFunction448000(field_0x0a8, g_UnknownGlobal567a88 + 1, 1);
                }
                if ((signed char)field_0x0b0 >= 0) {
                    break;
                }
                COPY_TEXT(name, "rider.slt", 0x20);
                model = localRacer->field_0x5c4->field_0x1a0;
            } while (++pass < 2);
        }
    }
    return 1;
}

// 0x0041ea10
int BikeRace::UnknownFunction41ea10() {
    if (g_TrackGame->field_0x18 > 1) {
        for (int i = 0; i < g_TrackGame->mode.field_0x1be0; i++) {
            if (racerSlots[i]->UnknownFunction495c00()) {
                return 1;
            }
        }
    }
    return 0;
}

// 0x0041ea60
void BikeRace::UnknownFunction41ea60(UnknownBikeRaceCharacter* character, int from, int to) {
    for (int i = 0; i < character->field_0x1a0->field_0x274; i++) {
        for (int j = 0; j < character->field_0x1a0->field_0x28c[i].field_0x00; j++) {
            for (int k = 0; k < character->field_0x1a0->field_0x28c[i].field_0x04[j].field_0x20; k++) {
                if (character->field_0x1a0->field_0x28c[i].field_0x04[j].field_0x24[k] == from) {
                    character->field_0x1a0->field_0x28c[i].field_0x04[j].field_0x24[k] = to;
                }
            }
        }
    }
}

// 0x0041f550
void BikeRace::UnknownFunction41f550(int value) {
    if (value) {
        if (field_0x3f8) {
            return;
        }
        field_0x3f8 = true;
    } else {
        field_0x3f8 = !field_0x3f8;
    }
    g_TrackGame->field_0x1c4 = field_0x3f8;
}

// 0x0041f590
int BikeRace::UnknownVirtualSlot20(int value) {
    if (field_0x190 && field_0x19c != 0 && field_0x19c->UnknownFunction51da30(value, &value)) {
        if (value == 0) {
            field_0x194 = 10.0f;
        }
        return 1;
    }
    return 0;
}

// 0x0041f5e0
int BikeRace::UnknownVirtualSlot23(UnknownControlEvent* event, UnknownInputEntry* entry) {
    int result;
    if (g_TrackGame->network != 0 && event->kind == 0) {
        g_TrackGame->controlInterface->keyboard->UnknownFunction48a240(0xc);
    }
    if (field_0x190 && field_0x19c != 0) {
        if (event->control == 1 && event->kind == 0) {
            field_0x194 = 0.1f;
            field_0x19c->field_0x3dc = 0;
            field_0x190 = false;
            return 1;
        }
        if (field_0x19c->UnknownFunction51dce0(event, entry, &result)) {
            return 1;
        }
    }
    if (GameObject::UnknownVirtualSlot23(event, entry)) {
        return 1;
    }
    if (!field_0x190 && g_TrackGame->uiInteractionBlocked) {
        return 0;
    }
    if (event->kind == 0) {
        switch (event->control) {
        case 0x35:
            if (g_TrackGame->field_0x18 > 1) {
                KeyboardDevice* keyboard = g_TrackGame->controlInterface->keyboard;
                if (!keyboard->UnknownVirtualSlot5(0x2a, 0x3f, 0) &&
                    !g_TrackGame->controlInterface->keyboard->UnknownVirtualSlot5(0x36, 0x3f, 0)) {
                    if (g_TrackGame->mode.field_0x6b4 == 2) {
                        break;
                    }
                    if (field_0x19c != 0) {
                        field_0x19c->UnknownFunction51dd10();
                        field_0x19c->field_0x3dc = 1;
                    }
                    field_0x194 = 10.0f;
                    field_0x190 = true;
                    return 1;
                }
                g_TrackGame->mode.field_0x6b4++;
                if (g_TrackGame->mode.field_0x6b4 == 3) {
                    g_TrackGame->mode.field_0x6b4 = 0;
                }
                if (g_TrackGame->mode.field_0x6b4 == 1 && field_0x19c != 0) {
                    field_0x19c->UnknownFunction51dd10();
                    field_0x19c->field_0x3dc = 1;
                    field_0x190 = true;
                }
                char title[0x80];
                char value[0x80];
                char text[0x100];
                g_TrackGame->LoadResourceString(0x1428, title, 0x80);
                if (g_TrackGame->mode.field_0x6b4 == 0) {
                    g_TrackGame->LoadResourceString(0x140b, value, 0x80);
                } else if (g_TrackGame->mode.field_0x6b4 == 1) {
                    g_TrackGame->LoadResourceString(0x140a, value, 0x80);
                } else if (g_TrackGame->mode.field_0x6b4 == 2) {
                    g_TrackGame->LoadResourceString(0x140c, value, 0x80);
                }
                sprintf(text, "%s : %s", title, value);
                UnknownMessage* message = new (__FILE__, 0xcdd) UnknownMessage(text, 3.25f);
                TextQueueOverlay* overlay;
                switch (g_TrackGame->mode.field_0x27f8.field_0x04) {
                case 2:
                    overlay = g_TrackGame->field_0x564->field_0x6c;
                    if (overlay != 0 && message != 0) {
                        overlay->UnknownFunction51b540(message);
                    }
                    break;
                case 0:
                    overlay = g_TrackGame->field_0x55c->field_0x6c;
                    if (overlay != 0 && message != 0) {
                        overlay->UnknownFunction51b540(message);
                    }
                    break;
                case 1:
                case 5:
                    overlay = g_TrackGame->field_0x560->field_0x6c;
                    if (overlay != 0 && message != 0) {
                        overlay->UnknownFunction51b540(message);
                    }
                    break;
                case 4:
                    overlay = g_TrackGame->field_0x568->field_0x6c;
                    if (overlay != 0 && message != 0) {
                        overlay->UnknownFunction51b540(message);
                    }
                    break;
                }
                if (message != 0) {
                    delete message;
                }
            }
            break;
        case 0x1b:
            if ((g_TrackGame->field_0x2d4_bit2) &&
                UnknownFunction43caa0(0x1b, 0, event, 3)) {
                UnknownFunction41f1d0(1, 0, 0);
                return 1;
            }
            if (!UnknownFunction43caa0(0x1b, 0, event, 0x80)) {
                UnknownFunction41f1d0(1, 0, 1);
            }
            return 1;
        case 0x1a:
            if ((g_TrackGame->field_0x2d4_bit2) &&
                UnknownFunction43caa0(0x1a, 0, event, 3)) {
                UnknownFunction41f1d0(0, 0, 0);
                return 1;
            }
            if (!UnknownFunction43caa0(0x1a, 0, event, 0x80)) {
                UnknownFunction41f1d0(0, 0, 1);
            }
            return 1;

        case 0xc9:
            if (UnknownFunction43caa0(0xc9, 0, event, 0x80000000)) {
                UnknownFunction41f1d0(1, 1, 0);
                return 1;
            }
            break;
        case 0xd1:
            if (UnknownFunction43caa0(0xd1, 0, event, 0x80000000)) {
                UnknownFunction41f1d0(0, 1, 0);
                return 1;
            }
            break;
        case 0x22:
            if (UnknownFunction43caa0(0x22, 0, event, 0xc)) {
                g_TrackGame->mode.field_0xa8c = 1 - g_TrackGame->mode.field_0xa8c;
                localRacer->field_0x5bc = g_TrackGame->mode.field_0xa8c;
                TextQueueOverlay* overlay = g_TrackGame->eventManager->FindTextQueue();
                if (overlay == 0) {
                    return 1;
                }
                char title[0x80];
                char value[0x80];
                char text[0x80];
                g_TrackGame->LoadResourceString(0x140d, title, 0x80);
                if (localRacer->field_0x5bc) {
                    g_TrackGame->LoadResourceString(0x1407, value, 0x80);
                } else {
                    g_TrackGame->LoadResourceString(0x1408, value, 0x80);
                }
                sprintf(text, "%s %s", title, value);
                UnknownMessage message(text, 1.5f);
                overlay->UnknownFunction51b540(&message);
                return 1;
            }
            break;
        case 0x30:
            if (UnknownFunction43caa0(0x30, 0, event, 0xc)) {
                g_TrackGame->mode.field_0xa90 = 1 - g_TrackGame->mode.field_0xa90;
                localRacer->field_0x5c0 = g_TrackGame->mode.field_0xa90;
                TextQueueOverlay* overlay = g_TrackGame->eventManager->FindTextQueue();
                if (overlay == 0) {
                    return 1;
                }
                char title[0x80];
                char value[0x80];
                char text[0x80];
                g_TrackGame->LoadResourceString(0x142d, title, 0x80);
                if (localRacer->field_0x5c0) {
                    g_TrackGame->LoadResourceString(0x1407, value, 0x80);
                } else {
                    g_TrackGame->LoadResourceString(0x1408, value, 0x80);
                }
                sprintf(text, "%s %s", title, value);
                UnknownMessage message(text, 1.5f);
                overlay->UnknownFunction51b540(&message);
                return 1;
            }
            break;
        case 0x1f:
            if (UnknownFunction43caa0(0x1f, 0, event, 0x80)) {
                field_0x034 = 1 - field_0x034;
            }
            break;
        }
    }
    if (g_TrackGame->field_0x2d4_bit2 && g_TrackGame->debugOverlay != 0) {
        if (field_0x0a8 < 0) {
            field_0x0a8 = g_TrackGame->debugOverlay->NewPage();
        }
        int shown = g_TrackGame->debugOverlay->field_0x26c4;
        if (shown == field_0x0a8) {

            if (UnknownFunction43caa0(0x1c, 0, event, 0x80)) {
                if (g_TrackGame->ui->field_0x2c->GetUser(0)->field_0x30 == 0) {
                    g_TrackGame->ui->field_0x2c->UnknownFunction486590("ui\\cursor.tga", 0);
                }
                if (!((UnknownBikeRaceObjectFlags*)g_TrackGame->ui->field_0x2c
                          ->GetUser(0)->field_0x30)->field_0x25_bit0) {
                    if (objectPicker == 0) {
                        ObjectPicker* picker = new (__FILE__, 0xed2) ObjectPicker(1);
                        objectPicker = picker;
                        picker->UnknownFunction4b0210(
                            g_TrackGame->renderTarget, 0, UnknownFunction417b00,
                            (GameCursor*)g_TrackGame->ui->field_0x2c->GetUser(0)->field_0x30);
                        if (AppendChild(objectPicker, -1)) {
                            g_TrackGame->ui->UnknownFunction499b00();
                        }
                    } else {
                        g_TrackGame->ui->UnknownFunction499b00();
                    }
                } else {
                    g_TrackGame->debugOverlay->UnknownFunction448000(
                        field_0x0a8, g_UnknownGlobal567a88 + 1, 0);
                    g_UnknownGlobal567a88++;
                    if ((signed char)field_0x0b0 < 0) {
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
            if (UnknownFunction43caa0(0, 1, event, 0x80000000)) {
                int picked = objectPicker->UnknownFunction4b04c0();
                if (picked != 0) {
                    if (raceScene->field_0xb8 != 0) {
                        for (int i = 0; i < raceScene->field_0xb8->field_0x00; i++) {
                            UnknownBikeRacePickCaster* caster =
                                &((UnknownBikeRacePickCaster*)raceScene->field_0xb8->field_0x04)[i];
                            if (caster->field_0x00 & 1) {
                                if (picked == caster->field_0x08->field_0x128) {
                                    UnknownFunctionCameraView()->field_0x390 = 1;
                                    field_0x154 = i;
                                    UnknownFunction41f1d0(1, 0, 0);
                                    return 1;
                                }
                            } else if (picked == caster->field_0x0c) {
                                UnknownFunctionCameraView()->field_0x390 = 1;
                                field_0x154 = i;
                                UnknownFunction41f1d0(1, 0, 0);
                                return 1;
                            }
                        }
                    }
                    if (raceScene->field_0xb4 != 0) {
                        for (int i = 0; i < raceScene->field_0xb4->field_0x00; i++) {
                            UnknownSceneEntry* entry = &raceScene->field_0xb4->field_0x04[i];
                            if (entry->field_0x00_bit3) {
                                if (picked == ((UnknownBikeRacePickCharacter*)entry->field_0x04)->field_0x210) {
                                    UnknownFunctionCameraView()->field_0x390 = 1;
                                    field_0x150 = i;
                                    UnknownFunction41f1d0(1, 0, 1);
                                    return 1;
                                }
                            } else if (picked == ((UnknownBikeRacePickObject*)entry->field_0x08)->field_0x3c) {
                                UnknownFunctionCameraView()->field_0x390 = 1;
                                field_0x150 = i;
                                UnknownFunction41f1d0(1, 0, 1);
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
        count = g_TrackGame->mode.field_0x1be0;
        for (int i = 0; i < count; i++) {
            int id = g_TrackGame->mode.field_0x1be4[i].field_0xd4;
            if (id != self && id == from) {
                UnknownBikeRaceRacer* racer = racerSlots[i];
                if (racer != 0 && racer->field_0x734 == state->field_0x54) {
                    racerSlots[i]->field_0x11b8 = 1;
                    UnknownBikeRaceRacerPart* oldest = racerSlots[i]->field_0x11c8[2];
                    racerSlots[i]->field_0x11c8[2] = racerSlots[i]->field_0x11c8[1];
                    racerSlots[i]->field_0x11c8[1] = racerSlots[i]->field_0x11c8[0];
                    racerSlots[i]->field_0x11c8[0] = oldest;
                    racerSlots[i]->field_0x11c8[0]->field_0x58 = flags;
                    racerSlots[i]->field_0x11c8[0]->field_0x00 = *state;
                    racerSlots[i]->field_0x11c8[0]->field_0x5c = 1;
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
        for (int i = 0; i < g_TrackGame->mode.field_0x1be0; i++) {
            int id = g_TrackGame->mode.field_0x1be4[i].field_0xd4;
            if (id != self && id == from && racerSlots[i] != 0 &&
                racerSlots[i]->field_0x734 == message->field_0x16) {
                if (!racerSlots[i]->field_0x11b8) {
                    return 1;
                }
                UnknownBikeRaceRacerPart* oldest = racerSlots[i]->field_0x11c8[2];
                racerSlots[i]->field_0x11c8[2] = racerSlots[i]->field_0x11c8[1];
                racerSlots[i]->field_0x11c8[1] = racerSlots[i]->field_0x11c8[0];
                racerSlots[i]->field_0x11c8[0] = oldest;
                racerSlots[i]->field_0x11c8[0]->field_0x58 = flags;
                racerSlots[i]->UnknownFunction4933e0(message, racerSlots[i]->field_0x11c8[0]);
                racerSlots[i]->field_0x11c8[0]->field_0x5c = 0;
                return 1;
            }
        }
    } else if (type == 10) {
        UnknownBikeRaceNetScore* score = (UnknownBikeRaceNetScore*)data;
        count = g_TrackGame->mode.field_0x1be0;
        int self = g_TrackGame->network->localPlayer;
        for (int i = 0; i < count; i++) {
            int id = g_TrackGame->mode.field_0x1be4[i].field_0xd4;
            if (id != self && id == from) {
                UnknownBikeRaceRacer* racer = racerSlots[i];
                if (racer != 0 && racer->field_0x734 == score->field_0x01) {
                    racerSlots[i]->field_0x768 = score->field_0x04;
                    return 1;
                }
            }
        }
    } else if (type == 0x85) {
        char name[0x14];
        if (g_TrackGame->network->GetPlayerName(from, name)) {
            if (g_TrackGame->mode.field_0x6b4 != 2) {
                if (field_0x19c != 0) {
                    field_0x19c->UnknownFunction51dd10();
                }
                field_0x194 = 10.0f;
                field_0x190 = true;
            }
            if (field_0x19c != 0) {
                field_0x19c->UnknownFunction51dd70(name, &((UnknownBikeRaceNetStamp*)data)->field_0x04, 1);
            }
        }
        return 1;
    } else if (type == 0x84) {
        UnknownBikeRaceNetStamp* stamp = (UnknownBikeRaceNetStamp*)data;
        for (int i = 0; i < g_TrackGame->mode.field_0x1be0; i++) {
            if (g_TrackGame->mode.field_0x1be4[i].field_0xd4 == from) {
                float delay = (float)(unsigned int)flags - (float)stamp->field_0x04;
                if (delay < g_TrackGame->mode.field_0x1be4[i].field_0xd0) {
                    g_TrackGame->mode.field_0x1be4[i].field_0xd0 = delay;
                }
                g_TrackGame->mode.field_0x1be4[i].field_0xc8 = 1;
            }
        }
    }
    return 0;
}

// 0x004204e0
UnknownBikeRaceRacer* BikeRace::UnknownFunction4204e0(int* iterator) {
    UnknownBikeRaceRacer* racer = 0;
    if (g_TrackGame->uiInteractionBlocked) {
        return 0;
    }
    int players = g_TrackGame->field_0x18;
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
    if (g_TrackGame->field_0x18 > 1) {
        if (*iterator > g_TrackGame->mode.field_0x27f8.field_0x28 + g_TrackGame->field_0x18) {
            *iterator = 0;
            return 0;
        }
    } else {
        int count = g_TrackGame->mode.field_0x27f8.field_0x00 == 4 ? 1 : g_TrackGame->mode.field_0x27f8.field_0x24;
        if (*iterator > count + 1) {
            *iterator = 0;
            return 0;
        }
    }
    return racer;
}

// 0x00420590
void BikeRace::UnknownFunction420590(int player) {
    if (g_TrackGame->uiInteractionBlocked) {
        return;
    }
    int self = g_TrackGame->network->localPlayer;
    for (int i = 0; i < g_TrackGame->mode.field_0x1be0; i++) {
        int id = g_TrackGame->mode.field_0x1be4[i].field_0xd4;
        if (id != self && id == player && racerSlots[i] != 0) {
            racerSlots[i]->UnknownRacerVirtualSlot44();
            TrackGameViewOwner* owner = g_TrackGame->field_0x568;
            if (owner != 0 && owner->field_0xa8 == (UnknownEventRacer*)racerSlots[i]) {
                if (g_TrackGame->mode.field_0x27f8.field_0x144) {
                    owner->field_0xa8 = 0;
                } else if (g_TrackGame->network->isHost) {
                    owner->UnknownFunction4a9d20();
                }
            }
        }
    }
}

// 0x00420650
void BikeRace::UnknownFunction420650(int mode, char* path, char* description) {
    if (g_TrackGame->network != 0) {
        return;
    }
    if (g_TrackGame->uiInteractionBlocked) {
        return;
    }
    field_0x18b = true;
    field_0x188 = false;
    UnknownFunction421050();
    int iterator = 0;
    Vector3 position;
    Vector3 direction;
    if (field_0x064 != 0 && g_TrackGame->mode.field_0x27f8.field_0x00 != 0 &&
        g_TrackGame->mode.field_0x27f8.field_0x00 != 4) {
        field_0x064->UnknownVirtualSlot5();
    }
    int positions[11];
    int order[11];
    if (g_TrackGame->mode.field_0x27f8.field_0x00 == 2) {
        UnknownFunction417bc0(order);
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
    if (g_TrackGame->field_0x342c) {
        UnknownFunction420b00(path, description);
        replayTime = 0;
        field_0x1d4 = 0;
        replayLength = 0;
        field_0x1d8 = 0;
        replayMode = 4;
        field_0x3f8 = false;
        field_0x3f9 = false;
        isRecording = mode;
        g_TrackGame->field_0x2e0 = 1.0f;
        g_TrackGame->field_0x1c4 = 0;
        field_0x044->field_0x3c->UnknownVirtualSlot16(0);
        if (field_0x044->field_0x40 != 0) {
            field_0x044->field_0x40->UnknownVirtualSlot16(0);
        }
        field_0x044->field_0x44->UnknownVirtualSlot16(0);
        replayVcr = new (__FILE__, 0x1068) KrustyVCR;
        if (replayVcr != 0) {
            if (mode == 0) {
                g_TrackGame->field_0x3428 = 1;
                if (!replayVcr->UnknownFunction49bf10(UnknownFunction4230e0, 1, "VCRtape.dat",
                                                        field_0x1a8)) {
                    delete replayVcr;
                    replayVcr = 0;
                }
                g_TrackGame->ui->field_0x2c->ShowCursors(1);
                g_TrackGame->ui->field_0x2c->ShowDialog(
                    new (__FILE__, 0x1073) VCRDlg(1, "VCR.dtm"), 0, 1, 0, 0, 0, 0, 1);
            } else {
                g_TrackGame->field_0x3428 = 0;
                if (!replayVcr->UnknownFunction49bf10(UnknownFunction4230e0, 0, "VCRtape.dat",
                                                        field_0x1a8)) {
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
        UnknownFunction41ea60(flagGirl, 1, 2);
        flagGirl->UnknownFunction4a8b10("Stand");
        field_0x078 = 0;
    }
    UnknownFunction41d2a0(0.0001f);
    raceScene->field_0x7c0 = 1;
    for (UnknownBikeRaceRacer* racer = UnknownFunction4204e0(&iterator); racer != 0;
         racer = UnknownFunction4204e0(&iterator)) {
        racer->UnknownRacerVirtualSlot43();
        racer->field_0x109 = true;
        UnknownFunction4210f0(&position, &direction, g_TrackGame->field_0x560,
                              positions[iterator - 1]);
        racer->field_0x10c = position;
        racer->field_0x118 = direction;
        racer->UnknownFunction496e20(replayVcr);
        field_0x3f8 = false;
    }
    if (g_TrackGame->field_0x568 != 0 &&
        g_TrackGame->field_0x568->field_0xdc != 0) {
        g_TrackGame->field_0x568->field_0xdc->UnknownFunction4a9d10(replayVcr);
    }
    if (g_TrackGame->field_0x55c != 0) {
        g_TrackGame->field_0x55c->UnknownFunction4e1f00();
    }
    if (g_TrackGame->field_0x568 != 0) {
        g_TrackGame->field_0x568->UnknownFunction4e1f00();
    }
    if (g_TrackGame->field_0x560 != 0) {
        g_TrackGame->field_0x560->UnknownFunction404df0(0, 1, UnknownFunctionCameraView()->field_0x3b4);
    }
}

// 0x00420b00
void BikeRace::UnknownFunction420b00(char* path, char* description) {
    g_TrackGame->ui->field_0x40 = 0;
    if (replayVcr != 0) {
        char text[0x80];
        g_TrackGame->LoadResourceString(0x1437, text, sizeof(text));
        if (isRecording) {
            replayVcr->UnknownFunction49c010(replayLength, description ? description : text,
                                               path != 0);
        }
        if (path != 0) {
            CopyFileA("VCRtape.dat", path, 1);
        }
        replayVcr->UnknownFunction4691f0();
        replayVcr->Release();
        replayVcr = 0;
        g_TrackGame->ui->field_0x40 = 1;
    }
}

// 0x00420bd0
void BikeRace::UnknownFunction420bd0() {
    if (ghostVcr != 0) {
        ghostVcr->UnknownFunction4691f0();
        ghostVcr->Release();
        ghostVcr = 0;
    }
    if (replayVcr == 0) {
        return;
    }
    if (localRacer->field_0x7a0 == 1 && field_0x189) {
        replayVcr->UnknownFunction4691f0();
        replayVcr->Release();
        replayLength = 0;
    } else {
        char date[0x20];
        char time[0x20];
        char description[0x20];
        GetDateFormatA(0, 0, 0, "M/d/yyyy", date, sizeof(date));
        GetTimeFormatA(0, 0, 0, "h:mm tt", time, sizeof(time));
        sprintf(description, "%s %s", date, time);
        replayVcr->UnknownFunction49c010(localRacer->field_0x74c, description, 0);
        replayVcr->UnknownFunction4691f0();
        replayVcr->Release();
        if (replayLength <= field_0x1c8) {
            field_0x1c8 = replayLength;
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
    if (!replayVcr->UnknownFunction49bf10(UnknownFunction4230e0, 0, ghostFileRecorded, field_0x1a8)) {
        return;
    }
    char model[0x104];
    char rider[0x104];
    char riderName[0x40];
    char bikeName[0x40];
    sprintf(model, "%s\\%s", "Res", "Ghost.mcf");
    sprintf(rider, "%s\\GhostRider.mcf", "Res");
    g_TrackGame->ui->UnknownFunction49b560(g_TrackGame->mode.field_0x1974.field_0x40,
                                                     riderName, 0x3f);
    g_TrackGame->ui->UnknownFunction49b7f0(g_TrackGame->mode.field_0x1974.field_0x80,
                                                     bikeName, 0x3f);
    replayVcr->UnknownFunction49c070(0, 0, 0, g_TrackGame->mode.field_0x00, model, rider,
                                       riderName, bikeName, aiRacers[0]->field_0x738,
                                       (unsigned char)aiRacers[0]->field_0x737,
                                       g_TrackGame->mode.field_0x1bcc);
    AppendChild(replayVcr, -1);
    localRacer->UnknownFunction496e20(replayVcr);
    if (field_0x189 && localRacer->field_0x7a0 == 1 && ghostFilePlayed[0] == 0) {
        return;
    }
    ghostVcr = new (__FILE__, 0x1103) KrustyVCR;
    if (ghostVcr == 0) {
        return;
    }
    if (!ghostVcr->UnknownFunction49bf10(UnknownFunction4230e0, 1, ghostFilePlayed, field_0x1a8)) {
        return;
    }
    localRacer->field_0x750 = field_0x1c8 = ghostVcr->UnknownFunction49c000();
    aiRacers[0]->UnknownFunction496e20(ghostVcr);
    aiRacers[0]->UnknownVirtualSlot5();
    aiRacers[0]->field_0x11b8 = 0;
    UnknownFunction423040(0);
    AppendChild(ghostVcr, -1);
}

// 0x00421050
void BikeRace::UnknownFunction421050() {
    if (g_TrackGame->field_0x3428) {
        field_0x15c = 0.0f;
        return;
    }
    int mode = g_TrackGame->mode.field_0x27f8.field_0x00;
    if (mode != 0 && mode != 4) {
        if (field_0x06c != 0) {
            field_0x15c = 8.2f;
            field_0x06c->UnknownVirtualSlot5();
            raceScene->UnknownFunction4eb040(field_0x070, 0.0f, 0, 0);
            if (field_0x06c->field_0x210 != 0) {
                field_0x06c->field_0x210->Release();
                field_0x06c->field_0x210 = 0;
            }
            raceScene->UnknownFunction4eafd0(field_0x070);
        } else {
            field_0x15c = 5.2f;
        }
    }
}

// Inline: TrackGame+0x568's ghost bike, reloaded at each use.
static inline UnknownBikeRaceGhost* UnknownFunctionGhost() {
    return (UnknownBikeRaceGhost*)g_TrackGame->field_0x568->field_0xdc;
}

// 0x00421d50
int BikeRace::UnknownFunction421d50(int a, void* data, int flag, int b, int* keep, int time) {
    *keep = 0;
    field_0x1e4 = a;
    if (field_0x3f8 && a == -1) {
        replayTime = ((UnknownBikeRaceVcrTick*)data)->field_0x00;
        field_0x3f9 = false;
        return 5;
    }
    if (g_TrackGame->uiInteractionBlocked) {
        return 0;
    }
    if (a == -2) {
        UnknownFunction422ec0();
        g_TrackGame->field_0x2e0 = 1.0f;
        if (g_TrackGame->ui->field_0x4a8) {
            g_TrackGame->ui->field_0x44 = 1;
            return 1;
        }
        if (g_TrackGame->mode.field_0x27f8.field_0x00 == 4) {
            return 1;
        }
        replayTime = replayVcr->field_0x10c;
        if (replaySeekTarget != -1.0f && replayTime > replaySeekTarget) {
            replayMode = 11;
            UnknownFunction41f550(0);
            return 11;
        }
        if (replayMode == 12) {
            replayMode = 4;
            replayTime = 0;
            UnknownFunction41f550(0);
            return 12;
        }
        UnknownFunction41f550(1);
        if (replayMode == 11) {
            UnknownFunction41f550(0);
            return 11;
        }
        return 1;
    } else if (a == -3) {
        UnknownFunction422ec0();
        g_TrackGame->field_0x2e0 = 1.0f;
        if (replayMode == 14) {
            UnknownFunction41f550(1);
            replayMode = 4;
            return 1;
        }
        if (replaySeekTarget != -1.0f && replaySeekTarget > replayTime) {
            replayMode = 9;
            g_TrackGame->field_0x2e0 = 15.0f;
            UnknownFunction41f550(0);
            return 1;
        }
        if (replaySeekTarget != -1.0f) {
            g_TrackGame->field_0x2e0 = 1.0f;
            replaySeekTarget = -1.0f;
        }
        replayTime = 0;
        replayMode = 4;
        return 1;
    } else if (a == -1) {
        UnknownBikeRaceVcrTick* tick = (UnknownBikeRaceVcrTick*)data;
        if (field_0x3f9) {
            replayTime = tick->field_0x00;
            UnknownFunction41f550(0);
            return 7;
        }
        float step = replayTime / (float)field_0x1d4;
        float tickStep = tick->field_0x00 / (float)tick->field_0x04;
        if (replaySeekTarget != -1.0f) {
            if ((replayMode == 9 && replaySeekTarget < replayTime) ||
                (replayMode == 11 && replaySeekTarget > replayTime)) {
                UnknownFunction422ec0();
                replayMode = 4;
                g_TrackGame->field_0x2e0 = 1.0f;
                replaySeekTarget = -1.0f;
            } else if (replaySeekTarget > replayTime) {
                replayMode = 9;
                g_TrackGame->field_0x2e0 = 15.0f;
            } else {
                replayMode = 11;
                replayTime = tick->field_0x00;
                return 11;
            }
        }
        if (replayMode == 11) {
            replayTime = tick->field_0x00;
            return 11;
        }
        if (replayMode == 12) {
            replayTime = 0;
            replayMode = 14;
            return 12;
        }
        if (replayMode == 14) {
            if (replayTime > 0.5f) {
                UnknownFunction41f550(0);
                UnknownFunction422ec0();
                return 5;
            }
            return 1;
        }
        if (field_0x1e0 != replayMode) {
            replayTime = s_UnknownStatic578e8c = tick->field_0x00;
            UnknownFunction422ec0();
        }
        if (tick->field_0x00 > replayTime) {
            s_UnknownStatic578e8c = tick->field_0x00;
            return 3;
        }
        if (replayTime - step > s_UnknownStatic578e8c && tickStep + tick->field_0x00 < replayTime) {
            s_UnknownStatic578e8c = tick->field_0x00;
            return 2;
        }
        s_UnknownStatic578e8c = tick->field_0x00;
        return 1;
    } else if (a == 6) {
        if (field_0x3f8) {
            UnknownFunction41f550(0);
        }
        return replayMode == 11 ? 11 : 1;
    } else if (a == 1) {
        UnknownBikeRaceNetState* state = (UnknownBikeRaceNetState*)data;
        UnknownBikeRaceRacer* racer = localRacer;
        if (b == racer->field_0x11bc && racer->field_0x734 == state->field_0x54) {
            if (g_TrackGame->mode.field_0x27f8.field_0x00 == 4) {
                racer = aiRacers[0];
            }
            if (racer == 0) {
                return 0;
            }
            racer->field_0x11b8 = 1;
            UnknownBikeRaceRacerPart* oldest;
            if (g_TrackGame->mode.field_0x27f8.field_0x00 == 4) {
                oldest = racer->field_0x11c8[2];
            } else {
                oldest = racer->field_0x11c8[3];
                racer->field_0x11c8[3] = racer->field_0x11c8[2];
            }
            racer->field_0x11c8[2] = racer->field_0x11c8[1];
            racer->field_0x11c8[1] = racer->field_0x11c8[0];
            racer->field_0x11c8[0] = oldest;
            oldest->field_0x58 = time;
            racer->field_0x11c8[0]->field_0x00 = *state;
            racer->field_0x11c8[0]->field_0x00.field_0x4c = (int)(s_UnknownStatic578e8c * 1000.0f);
            racer->field_0x11c8[0]->field_0x5c = 1;
            racer->field_0x1358 = state->field_0x3c;
            racer->field_0x1364 = state->field_0x08;
            racer->field_0x1370 = state->field_0x30;
            racer->field_0x137c = state->field_0x14;
            racer->field_0x1380 = state->field_0x18;
            racer->field_0x1384 = state->field_0x1c;
            racer->field_0x1388 = replayMode == 11 ? time : state->field_0x4c;
            if (ghostVcr == 0) {
                racer->field_0x750 = state->field_0x28;
                racer->field_0x770 = state->field_0x2c;
            }
            *keep = 1;
            if (replayMode == 11) {
                racer->field_0x11c8[1]->field_0x00 = racer->field_0x11c8[0]->field_0x00;
                racer->field_0x11c8[2]->field_0x00 = racer->field_0x11c8[1]->field_0x00;
                racer->field_0x11c8[3]->field_0x00 = racer->field_0x11c8[2]->field_0x00;
                racer->field_0x11c8[1]->field_0x58 = racer->field_0x11c8[0]->field_0x58;
                racer->field_0x11c8[2]->field_0x58 = racer->field_0x11c8[1]->field_0x58;
                racer->field_0x11c8[3]->field_0x58 = racer->field_0x11c8[2]->field_0x58;
                racer->field_0x11c8[1]->field_0x5c = 1;
                racer->field_0x11c8[2]->field_0x5c = 1;
                racer->field_0x11c8[3]->field_0x5c = 1;
                return 11;
            }
            return 1;
        }
        for (int i = 0; i < g_TrackGame->mode.field_0x27f8.field_0x24; i++) {
            if (aiRacers[i] != 0 && b == aiRacers[i]->field_0x11bc &&
                aiRacers[i]->field_0x734 == state->field_0x54) {
                aiRacers[i]->field_0x11b8 = 1;
                UnknownBikeRaceRacerPart* oldest;
                if (g_TrackGame->mode.field_0x27f8.field_0x00 == 4) {
                    oldest = aiRacers[i]->field_0x11c8[2];
                } else {
                    oldest = aiRacers[i]->field_0x11c8[3];
                    aiRacers[i]->field_0x11c8[3] = aiRacers[i]->field_0x11c8[2];
                }
                aiRacers[i]->field_0x11c8[2] = aiRacers[i]->field_0x11c8[1];
                aiRacers[i]->field_0x11c8[1] = aiRacers[i]->field_0x11c8[0];
                aiRacers[i]->field_0x11c8[0] = oldest;
                aiRacers[i]->field_0x11c8[0]->field_0x58 = time;
                aiRacers[i]->field_0x11c8[0]->field_0x00 = *state;
                aiRacers[i]->field_0x11c8[0]->field_0x00.field_0x4c = (int)(s_UnknownStatic578e8c * 1000.0f);
                aiRacers[i]->field_0x11c8[0]->field_0x5c = 1;
                aiRacers[i]->field_0x1358 = state->field_0x3c;
                aiRacers[i]->field_0x1364 = state->field_0x08;
                aiRacers[i]->field_0x1370 = state->field_0x30;
                aiRacers[i]->field_0x137c = state->field_0x14;
                aiRacers[i]->field_0x1380 = state->field_0x18;
                aiRacers[i]->field_0x1384 = state->field_0x1c;
                aiRacers[i]->field_0x1388 = replayMode == 11 ? time : state->field_0x4c;
                if (ghostVcr == 0) {
                    aiRacers[i]->field_0x750 = state->field_0x28;
                    aiRacers[i]->field_0x770 = state->field_0x2c;
                }
                *keep = 1;
                if (replayMode == 11) {
                    aiRacers[i]->field_0x11c8[1]->field_0x00 = aiRacers[i]->field_0x11c8[0]->field_0x00;
                    aiRacers[i]->field_0x11c8[2]->field_0x00 = aiRacers[i]->field_0x11c8[1]->field_0x00;
                    aiRacers[i]->field_0x11c8[3]->field_0x00 = aiRacers[i]->field_0x11c8[2]->field_0x00;
                    aiRacers[i]->field_0x11c8[1]->field_0x58 = aiRacers[i]->field_0x11c8[0]->field_0x58;
                    aiRacers[i]->field_0x11c8[2]->field_0x58 = aiRacers[i]->field_0x11c8[1]->field_0x58;
                    aiRacers[i]->field_0x11c8[3]->field_0x58 = aiRacers[i]->field_0x11c8[2]->field_0x58;
                    aiRacers[i]->field_0x11c8[1]->field_0x5c = 1;
                    aiRacers[i]->field_0x11c8[2]->field_0x5c = 1;
                    aiRacers[i]->field_0x11c8[3]->field_0x5c = 1;
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
        if (b == racer->field_0x11bc && racer->field_0x734 == message->field_0x16) {
            if (g_TrackGame->mode.field_0x27f8.field_0x00 == 4) {
                racer = aiRacers[0];
            }
            if (racer == 0 || !racer->field_0x11b8) {
                return 0;
            }
            UnknownBikeRaceRacerPart* oldest;
            if (g_TrackGame->mode.field_0x27f8.field_0x00 == 4) {
                oldest = racer->field_0x11c8[2];
            } else {
                oldest = racer->field_0x11c8[3];
                racer->field_0x11c8[3] = racer->field_0x11c8[2];
            }
            racer->field_0x11c8[2] = racer->field_0x11c8[1];
            racer->field_0x11c8[1] = racer->field_0x11c8[0];
            racer->field_0x11c8[0] = oldest;
            oldest->field_0x58 = time;
            racer->UnknownFunction4933e0(message, racer->field_0x11c8[0]);
            racer->field_0x11c8[0]->field_0x00.field_0x4c = (int)(s_UnknownStatic578e8c * 1000.0f);
            racer->field_0x11c8[0]->field_0x5c = 0;
            return 1;
        }
        int i = 0;
        if (g_TrackGame->mode.field_0x27f8.field_0x00 == 4) {
            return 1;
        }
        for (; i < g_TrackGame->mode.field_0x27f8.field_0x24; i++) {
            if (aiRacers[i] != 0 && b == aiRacers[i]->field_0x11bc &&
                aiRacers[i]->field_0x734 == message->field_0x16) {
                if (!aiRacers[i]->field_0x11b8) {
                    return 0;
                }
                UnknownBikeRaceRacerPart* oldest = aiRacers[i]->field_0x11c8[3];
                aiRacers[i]->field_0x11c8[3] = aiRacers[i]->field_0x11c8[2];
                aiRacers[i]->field_0x11c8[2] = aiRacers[i]->field_0x11c8[1];
                aiRacers[i]->field_0x11c8[1] = aiRacers[i]->field_0x11c8[0];
                aiRacers[i]->field_0x11c8[0] = oldest;
                aiRacers[i]->field_0x11c8[0]->field_0x58 = time;
                aiRacers[i]->UnknownFunction4933e0(message, aiRacers[i]->field_0x11c8[0]);
                aiRacers[i]->field_0x11c8[0]->field_0x00.field_0x4c = (int)(s_UnknownStatic578e8c * 1000.0f);
                aiRacers[i]->field_0x11c8[0]->field_0x5c = 0;
                return 1;
            }
        }
    } else if (a == 10) {
        UnknownBikeRaceNetScore* score = (UnknownBikeRaceNetScore*)data;
        UnknownBikeRaceRacer* racer = localRacer;
        if (b == racer->field_0x11bc && racer->field_0x734 == score->field_0x01) {
            if (racer == 0) {
                return 0;
            }
            racer->field_0x768 = score->field_0x04;
            *keep = 1;
            return 1;
        }
        for (int i = 0; i < g_TrackGame->mode.field_0x27f8.field_0x24; i++) {
            if (aiRacers[i] != 0 && b == aiRacers[i]->field_0x11bc &&
                aiRacers[i]->field_0x734 == score->field_0x01) {
                aiRacers[i]->field_0x768 = score->field_0x04;
                *keep = 1;
                return 1;
            }
        }
    } else if (a == 0x11) {
        UnknownBikeRaceGhostRecord* record = (UnknownBikeRaceGhostRecord*)data;
        if (g_TrackGame->field_0x568 != 0 && g_TrackGame->field_0x568->field_0xdc != 0) {
            UnknownFunctionGhost()->field_0x4f8 = 1;
            UnknownBikeRaceGhostPart* oldest = UnknownFunctionGhost()->field_0x4fc[3];
            UnknownFunctionGhost()->field_0x4fc[3] = UnknownFunctionGhost()->field_0x4fc[2];
            UnknownFunctionGhost()->field_0x4fc[2] = UnknownFunctionGhost()->field_0x4fc[1];
            UnknownFunctionGhost()->field_0x4fc[1] = UnknownFunctionGhost()->field_0x4fc[0];
            UnknownFunctionGhost()->field_0x4fc[0] = oldest;
            UnknownFunctionGhost()->field_0x4fc[0]->field_0x3c = time;
            UnknownFunctionGhost()->field_0x4fc[0]->field_0x00 = *record;
            UnknownFunctionGhost()->field_0x4fc[0]->field_0x00.field_0x34 = (int)(s_UnknownStatic578e8c * 1000.0f);
            UnknownFunctionGhost()->field_0x4fc[0]->field_0x40 = 1;
            UnknownFunctionGhost()->field_0x5d4 = record->field_0x28;
            UnknownFunctionGhost()->field_0x5e0 = record->field_0x04;
            UnknownFunctionGhost()->field_0x5ec = record->field_0x1c;
            UnknownFunctionGhost()->field_0x5f8 = record->field_0x10;
            UnknownFunctionGhost()->field_0x5fc = record->field_0x14;
            UnknownFunctionGhost()->field_0x600 = record->field_0x18;
            UnknownFunctionGhost()->field_0x604 = replayMode == 11 ? time : record->field_0x34;
            *keep = 1;
            if (replayMode == 11) {
                UnknownFunctionGhost()->field_0x4fc[1]->field_0x00 = UnknownFunctionGhost()->field_0x4fc[0]->field_0x00;
                UnknownFunctionGhost()->field_0x4fc[2]->field_0x00 = UnknownFunctionGhost()->field_0x4fc[1]->field_0x00;
                UnknownFunctionGhost()->field_0x4fc[3]->field_0x00 = UnknownFunctionGhost()->field_0x4fc[2]->field_0x00;
                UnknownFunctionGhost()->field_0x4fc[1]->field_0x3c = UnknownFunctionGhost()->field_0x4fc[0]->field_0x3c;
                UnknownFunctionGhost()->field_0x4fc[2]->field_0x3c = UnknownFunctionGhost()->field_0x4fc[1]->field_0x3c;
                UnknownFunctionGhost()->field_0x4fc[3]->field_0x3c = UnknownFunctionGhost()->field_0x4fc[2]->field_0x3c;
                UnknownFunctionGhost()->field_0x4fc[1]->field_0x40 = 1;
                UnknownFunctionGhost()->field_0x4fc[2]->field_0x40 = 1;
                UnknownFunctionGhost()->field_0x4fc[3]->field_0x40 = 1;
                return 11;
            }
            return 1;
        }
    } else if (a == 0x10) {
        if (flag) {
            return 1;
        }
        if (g_TrackGame->field_0x568 != 0 && g_TrackGame->field_0x568->field_0xdc != 0 &&
            UnknownFunctionGhost()->field_0x4f8) {
            UnknownBikeRaceGhostPart* oldest = UnknownFunctionGhost()->field_0x4fc[3];
            UnknownFunctionGhost()->field_0x4fc[3] = UnknownFunctionGhost()->field_0x4fc[2];
            UnknownFunctionGhost()->field_0x4fc[2] = UnknownFunctionGhost()->field_0x4fc[1];
            UnknownFunctionGhost()->field_0x4fc[1] = UnknownFunctionGhost()->field_0x4fc[0];
            UnknownFunctionGhost()->field_0x4fc[0] = oldest;
            UnknownFunctionGhost()->field_0x4fc[0]->field_0x3c = time;
            UnknownFunctionGhost()->UnknownFunction4a9aa0((UnknownBikeRaceGhostDelta*)data,
                                                          UnknownFunctionGhost()->field_0x4fc[0]);
            UnknownFunctionGhost()->field_0x4fc[0]->field_0x00.field_0x34 = (int)(s_UnknownStatic578e8c * 1000.0f);
            UnknownFunctionGhost()->field_0x4fc[0]->field_0x40 = 0;
            return 1;
        }
    } else if (a == 0x87) {
        UnknownBikeRaceVcrFollow* follow = (UnknownBikeRaceVcrFollow*)data;
        if (g_TrackGame->field_0x568 != 0) {
            g_TrackGame->field_0x568->field_0xa8 = 0;
            if (follow->field_0x08 != 0) {
                int iterator = 0;
                for (UnknownBikeRaceRacer* racer = UnknownFunction4204e0(&iterator); racer != 0;
                     racer = UnknownFunction4204e0(&iterator)) {
                    if (racer->field_0x11bc == follow->field_0x08) {
                        g_TrackGame->field_0x568->field_0xa8 = (UnknownEventRacer*)racer;
                        break;
                    }
                }
            }
        }
    }
    return 0;
}

// 0x00423040
void BikeRace::UnknownFunction423040(int index) {
    UnknownBikeRaceRacer* racer = aiRacers[index];
    if (racer != 0) {
        racer->field_0x11c8[0]->field_0x00.field_0x08 = kVec3Zero;
        racer->field_0x11c8[1]->field_0x00.field_0x08 = kVec3Zero;
        racer->field_0x11c8[2]->field_0x00.field_0x08 = kVec3Zero;
        racer->field_0x11c8[3]->field_0x00.field_0x08 = kVec3Zero;
    }
}

// 0x00422ec0
void BikeRace::UnknownFunction422ec0() {
    UnknownBikeRaceRacer* racer = localRacer;
    racer->field_0x11c8[0]->field_0x00.field_0x08 = kVec3Zero;
    racer->field_0x11c8[1]->field_0x00.field_0x08 = kVec3Zero;
    racer->field_0x11c8[2]->field_0x00.field_0x08 = kVec3Zero;
    racer->field_0x11c8[3]->field_0x00.field_0x08 = kVec3Zero;
    for (int i = 0; i < UnknownFunctionRacerCount() - 1; i++) {
        racer = aiRacers[i];
        if (racer != 0) {
            racer->field_0x11c8[0]->field_0x00.field_0x08 = kVec3Zero;
            racer->field_0x11c8[1]->field_0x00.field_0x08 = kVec3Zero;
            racer->field_0x11c8[2]->field_0x00.field_0x08 = kVec3Zero;
            racer->field_0x11c8[3]->field_0x00.field_0x08 = kVec3Zero;
        }
    }
}

// 0x004230e0
int UnknownFunction4230e0(int a, void* data, int flag, int b, int* keep, int time,
                          int* milliseconds) {
    BikeRace* race = (BikeRace*)g_TrackGame->eventManager->FindRaceView();
    *milliseconds = (int)(race->replayTime * 1000.0f);
    return race->UnknownFunction421d50(a, data, flag, b, keep, time);
}

// 0x00423140
void BikeRace::UnknownFunction423140(Track* track) {
    TrackListItem* list = 0;
    TrackListItem* item =
        (TrackListItem*)DebugCalloc(1, sizeof(TrackListItem), __FILE__, 0x139d);
    if (item == 0) {
        return;
    }
    item->field_0x04 = track->field_0x00;
    item->field_0x0c = list;
    list = item;
    while (list != 0) {
        if (list->field_0x04->field_0x00 & 4) {
            item = list;
            list = list->field_0x0c;
            item->field_0x04->field_0x00 &= ~4;
            DebugFree(item, __FILE__, 0x13a8);
        } else {
            TrackSegment* segment = list->field_0x04->field_0x08;
            while (segment != 0 && segment->field_0x2c != 0) {
                Vector3 a;
                Vector3 b;
                SetDrawColor(0, 0xff, 0);
                a = *(Vector3*)&segment->field_0x0c;
                b = *(Vector3*)&segment->field_0x2c->field_0x0c;
                field_0x04c->UnknownFunction507c10(&a, 0, 0, 0);
                field_0x04c->UnknownFunction507c10(&b, 0, 0, 0);
                a.y += 5.0f;
                b.y += 5.0f;
                DrawLine(&a, &b);
                a = *(Vector3*)&segment->field_0x18;
                b = *(Vector3*)&segment->field_0x2c->field_0x18;
                field_0x04c->UnknownFunction507c10(&a, 0, 0, 0);
                field_0x04c->UnknownFunction507c10(&b, 0, 0, 0);
                a.y += 5.0f;
                b.y += 5.0f;
                DrawLine(&a, &b);
                SetDrawColor(0, 0, 0xff);
                a = *(Vector3*)&segment->field_0x00;
                b = *(Vector3*)&segment->field_0x2c->field_0x00;
                field_0x04c->UnknownFunction507c10(&a, 0, 0, 0);
                field_0x04c->UnknownFunction507c10(&b, 0, 0, 0);
                a.y += 5.0f;
                b.y += 5.0f;
                DrawLine(&a, &b);
                segment = segment->field_0x2c;
            }
            item = list;
            item->field_0x04->field_0x00 |= 4;
            for (int i = 0; i < item->field_0x04->field_0x10; i++) {
                TrackNode* link = item->field_0x04->field_0x14[i];
                if (!(link->field_0x00 & 4)) {
                    TrackListItem* next =
                        (TrackListItem*)DebugCalloc(1, sizeof(TrackListItem), __FILE__, 0x13c9);
                    if (next == 0) {
                        track->UnknownFunction517930(&list, 0);
                        return;
                    }
                    next->field_0x04 = item->field_0x04->field_0x14[i];
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
            UnknownFunction423140(raceTrack);
            if (field_0x144) {
                SetDrawColor(0xff, 0, 0xff);
                a = startProbe.field_0x00 - startProbe.field_0x18;
                b = startProbe.field_0x00 + startProbe.field_0x18;
                a.y += 2.5f;
                b.y += 2.5f;
                DrawLine(&a, &b);
            }
            if (field_0x148) {
                SetDrawColor(0, 0xff, 0xff);
                a = finishProbe.field_0x00 - finishProbe.field_0x18;
                b = finishProbe.field_0x00 + finishProbe.field_0x18;
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
void BikeRace::UnknownFunction423790(int level) {
    g_UnknownGlobal689f18 = g_TrackGame->field_0x2d0 ? g_UnknownGlobal5744c8 : g_UnknownGlobal574428;
    localRacer->field_0x3bc->UnknownFunction4451e0(level);
    localRacer->field_0x5c4->field_0x1a0->UnknownFunction4451e0(level);
    if (g_TrackGame->mode.field_0x27f8.field_0x00 != 4) {
        for (int i = 0; i < g_TrackGame->mode.field_0x27f8.field_0x24; i++) {
            aiRacers[i]->field_0x3bc->UnknownFunction4451e0(level);
            aiRacers[i]->field_0x5c4->field_0x1a0->UnknownFunction4451e0(level);
        }
    }
    for (int j = 0; j < g_TrackGame->mode.field_0x27f8.field_0x35; j++) {
        racerSlots[j]->field_0x3bc->UnknownFunction4451e0(level);
        racerSlots[j]->field_0x5c4->field_0x1a0->UnknownFunction4451e0(level);
    }
}

// 0x00423880
int BikeRace::UnknownVirtualSlot22(UnknownControlEvent* event, UnknownInputEntry* entry) {
    if (UnknownFunction43caa0(6, 0, event, 0x80)) {
        field_0x3fd = 1 - field_0x3fd;
        return 1;
    }
    return 0;
}
