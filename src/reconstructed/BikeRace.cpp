#include "../krusty2/broadphase/Terrain.h"
#include "GUIManagerCore.h"
#include "../krusty2/motion/CollisionCharacter.h"
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
#include "../krusty2/shadow/ProjectedShadow.h"

#include <math.h>
#include <stdlib.h>

#include "CarProcedural.h"
#include "DemoDlg.h"
#include "FollowCamera.h"
#include "Krusty3DObjects.h"
#include "MemTag.h"
#include "OptionProcs.h"
#include "RaceSound.h"
#include "RaceStatus.h"
#include "SelectGamePicProcs.h"
#include "TextureMap.h"

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
        race->objectPicker->pickedObject = (int)other;
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
    flagGirlState = 0;
    localRacer = 0;
    racerSlots = 0;
    aiRacers = 0;
    raceSound = 0;
    raceTerrain = 0;
    raceCamera = 0;
    particleManager = 0;
    raceScene = 0;
    raceShadow = 0;
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
    chatInputLineSetting = g_TrackGame->mode.chatInputLine;
    startGate = 0;
    thirtySecondMan = 0;
    flagGirl = 0;
    startGateEntry = -1;
    thirtySecondManEntry = -1;
    startBeforeFinish = 0;
    startGateTimer = 0;
    recordTickTime = 0;
    recordTickCount = 0;
    recordTickRecordCount = 0;
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
    lastRecordKind = 0;
    replaySeekTarget = -1.0f;
    recorderTickDue = false;
    scoreRecordDue = false;
    isRecording = 0;
    ghostFilePlayed[0] = 0;
    ghostFileRecorded[0] = 0;
    g_TrackGame->paused = 0;
    hasStartProbe = 0;
    hasFinishProbe = 0;
    chatOverlay = 0;
    forceHighLod = g_TrackGame->GetRegistryFlag("ForceHighLOD", 0);
    drawChildren = 1;
    lodDebugPage = -1;
    targetKind = 0xff;
    targetIndex = 0;
    objectPicker = 0;
    debugDraw = 0;
}

// Inline: the racers in the race, own and AI (a single player race) or
// own and remote.
static inline int CountRaceRacers() {
    if (g_TrackGame->playerCount == 1) {
        if (g_TrackGame->mode.raceSettings.raceMode == 4) {
            return g_TrackGame->playerCount;
        }
        return g_TrackGame->mode.raceSettings.aiRacerCount + 1;
    }
    return g_TrackGame->mode.raceSettings.multiplayerAiRacerCount + g_TrackGame->playerCount;
}

// The length of `v` (1 without a square root for unit vectors); the setup
// restores the lengths D3DRMVectorRotate's unit results lose. Retail adds z*z
// last, rounded to a float temporary (0x004188d8); this sum has its order, so
// the two agree at 24-bit precision.
static inline float VectorLength(const Vector3& v) {
    float zz = v.z * v.z;
    float lengthSquared = v.x * v.x + v.y * v.y + zz;
    if (lengthSquared == 1.0f)
        return 1.0f;
    return (float)sqrt(lengthSquared);
}

static inline Vector3 ScaledVector(const Vector3& v, float scale) {
    return Vector3(v.x * scale, v.y * scale, v.z * scale);
}

// 0x00417ed0. Not exact (6756 bytes against 6745, 2800 strict, linked 0.998):
// all 166 calls in retail order, EH states 0..0x10 and the 0x4e8 frame with
// its buffers at retail's offsets. Retail spills z*z of both vector lengths
// to a stack temporary and orders the girl block's temporaries differently,
// so most esp offsets below +0x44 differ (docs/BIKERACE.md).
BikeRace* BikeRace::Create(void* owner, void* particles, LightManager* lights,
                                          TextureContext* textures,
                                          Terrain* terrain, KrustyBikeCamera* camera,
                                          Scene* scene, RaceGateEvent* event,
                                          ProjectedShadow* shadow) {
    char name[0x104];
    char path[0x104];
    char modelName[0x104];
    char message[0x144];
    Vector3 position;
    Vector3 direction;
    Vector3 normal;
    Vector3 point;

    GameObject::SetRenderContext(owner);
    receivesNetworkMessages = 1;
    statusList = 0;
    aiSettings[0] = 50.0f;
    aiSettings[1] = 10.0f;
    aiSettings[2] = 40.0f;
    aiSettings[3] = 40.0f;
    aiSettings[4] = 60.0f;
    aiSettings[5] = 0.0f;
    aiSettings[6] = 100.0f;
    aiSettings[7] = 10.0f;
    aiSettings[8] = 10.0f;
    aiSettings[9] = 50.0f;
    aiSettings[10] = 100.0f;
    forceAiStep = 0;
    particleManager = (int)particles;
    raceScene = scene;
    raceShadow = shadow;
    if (!g_TrackGame->workingDirectory) {
        Release();
        return 0;
    }
    raceTerrain = terrain;
    raceCamera = camera;
    replayPaused = false;
    replayStepFrame = false;
    replayTime = 0;
    replayLength = 0;
    replayFrames = 0;
    replayLengthFrames = 0;
    g_TrackGame->ui->StepProgressBar();

    int mode = g_TrackGame->mode.raceSettings.eventType;
    switch (mode) {
    case 2:
    case 3: {
        COPY_TEXT(path, raceScene->environment->terrainFile, 0x104);
        strcpy(strrchr(path, '.'), ".tdf");
        FileStream* stream =
            new(__FILE__, 0x193) FileStream(g_ResourceManager);
        if (!g_TrackGame->sceneObject->OpenArchiveFile(stream, path, "rb", (int)modelName)) {
            sprintf(message, "No Track file found in env file.  Abort.\n");
            delete stream;
            Release();
            return 0;
        }
        if (stream) {
            raceTrack = new(__FILE__, 0x1a5) Track;
            trackLoaded = raceTrack->LoadTrack(stream, &startProbe, &finishProbe, &hasStartProbe,
                                                 &hasFinishProbe);
            delete stream;
        }
        if (hasFinishProbe)
            raceTerrain->QueryGround(reinterpret_cast<TerrainVec3*>(&finishProbe.center), 0, 0, 0);
        if (hasStartProbe)
            raceTerrain->QueryGround(reinterpret_cast<TerrainVec3*>(&startProbe.center), 0, 0, 0);
        if (hasFinishProbe && hasStartProbe)
            UpdateTrackSeparation();
        break;
    }
    case 1:
    case 5:
        if (mode != 1 || !g_TrackGame->mode.raceSettings.randomGates) {
            sprintf(path, "%s\\%s%02d.tdf", g_TrackGame->sceneObject->sceneDirectory,
                    g_TrackGame->mode.raceSettings.trackName,
                    g_TrackGame->mode.raceSettings.trackNumber);
            FileStream* stream =
                new(__FILE__, 0x1be) FileStream(g_ResourceManager);
            if (!g_TrackGame->sceneObject->OpenArchiveFile(stream, path, "rb", (int)modelName)) {
                delete stream;
            } else if (stream) {
                {
                    raceTrack = new(__FILE__, 0x1d0) Track;
                    trackLoaded = raceTrack->LoadTrack(stream, &startProbe, &finishProbe,
                                                         &hasStartProbe, &hasFinishProbe);
                    if (!trackLoaded) {
                        Track* track = raceTrack;
                        if (track != 0) {
                            if (track->startNode != 0)
                                track->FreeReachableNodes(&track->startNode);
                            delete track;
                        }
                    }
                    delete stream;
                }
            }
        }
        trackLoaded = BuildNodeList(event);
        break;
    }
    g_TrackGame->ui->StepProgressBar();

    WriteRelativePositions(&position, &direction, event, 0);
    mode = g_TrackGame->mode.raceSettings.eventType;
    if (mode == 3 || mode == 2) {
        if (raceTrack) {
            if (!g_TrackGame->playingReplay) {
                if (hasStartProbe) {
                    direction.x = -startProbe.direction.x;
                    direction.z = -startProbe.direction.z;
                    point.x = startProbe.center.x - direction.x * 15.0f;
                    point.z = startProbe.center.z - direction.z * 15.0f;
                } else {
                    point = position;
                }
                terrain->QueryGround(reinterpret_cast<TerrainVec3*>(&point), reinterpret_cast<TerrainVec3*>(&normal), 0, 0);
                thirtySecondMan = raceScene->FindEntryByFileName("30SecMan.slt", &thirtySecondManEntry);
                if (thirtySecondMan) {
                    raceScene->RestartEntry(thirtySecondManEntry, 0, 0, 0);
                    raceScene->animationTable->entries[thirtySecondManEntry].animatesWhileHeld = 1;
                    if (thirtySecondMan->collisionObject) {
                        thirtySecondMan->collisionObject->Release();
                        thirtySecondMan->collisionObject = 0;
                    }
                }
                if (g_TrackGame->mode.raceSettings.raceMode != 0 &&
                    g_TrackGame->mode.raceSettings.raceMode != 4 && thirtySecondMan) {
                    thirtySecondMan->modelNode->SetPositionVec3(point);
                    thirtySecondMan->modelNode->SetAxesPtr((const Vec3*)&direction, (const Vec3*)&kVec3YAxis, 1, 0);
                }
                if (thirtySecondMan)
                    thirtySecondMan->Disable();
                if (hasStartProbe) {
                    point.x = startProbe.center.x;
                    point.z = startProbe.center.z;
                    direction.x = startProbe.direction.x;
                    direction.z = startProbe.direction.z;
                } else {
                    point = position;
                }
                terrain->QueryGround(reinterpret_cast<TerrainVec3*>(&point), reinterpret_cast<TerrainVec3*>(&normal), 0, 0);
                if (g_TrackGame->mode.raceSettings.eventType == 3) {
                    startGate =
                        raceScene->FindEntryByFileName("StartGate.slb", &startGateEntry);
                    if (startGate) {
                        raceScene->RestartEntry(startGateEntry, 0, 0, 0);
                        if (startGate->collisionObject) {
                            startGate->collisionObject->Release();
                            startGate->collisionObject = 0;
                        }
                    }
                    if (g_TrackGame->mode.raceSettings.raceMode != 0 &&
                        g_TrackGame->mode.raceSettings.raceMode != 4) {
                        if (startGate) {
                            startGate->modelNode->SetPositionVec3(point);
                            startGate->modelNode->SetAxesPtr((const Vec3*)&direction, (const Vec3*)&kVec3YAxis, 1, 0);
                        }
                    } else if (startGate) {
                        startGate->Disable();
                    }
                }
            }
            if (hasFinishProbe) {
                point.x = finishProbe.center.x;
                point.z = finishProbe.center.z;
                terrain->QueryGround(reinterpret_cast<TerrainVec3*>(&point), 0, 0, 0);
                direction.x = finishProbe.direction.x;
                direction.y = 0.0f;
                direction.z = finishProbe.direction.z;
            }

            sprintf(name, "%s\\%s", "Res", "MetalArch.slt");
            ArcadeObject* arch = new(__FILE__, 0x247) ArcadeObject(1);
            arch = arch->Create(renderContext, (int)lights, (int)textures, name, kVec3Zero, 0, 0,
                                               2.0f, 0.5f, 0.5f, 0);
            AppendChild(arch, -1);
            arch->arcadeModel->SetUseLighting(1);
            float scale = finishProbe.width / arch->modelWidth * 1.111f;
            arch->arcadeModel->Scale(scale, scale, scale);
            arch->arcadeModel->ResetLocalMatrices();
            arch->SetPosition(&point);
            arch->SetAxes(&direction, &kVec3YAxis, 1, 0);
            arch->BuildCollision(name, 0x6a, 0, 0, 0, 1);

            Vector3 girlOffset;
            Vector3 boxOffset;
            Vector3 turned;
            Vector3 ground;
            Vector3 girlPosition;
            Vector3 boxPosition;
            girlOffset = Vector3(scale * 9.2f, 8.0f, scale * -1.8f);
            boxOffset = Vector3(girlOffset.x - 1.22f, 4.0f, girlOffset.z + 1.2f);
            float angle = (float)atan2(direction.x, direction.z);
            D3DRMVectorRotate(&turned, &girlOffset, (Vector3*)&kVec3YAxis, angle);
            girlOffset = ScaledVector(turned, VectorLength(girlOffset));
            ground = point + girlOffset;
            terrain->QueryGround(reinterpret_cast<TerrainVec3*>(&ground), 0, 0, 0);
            float height = ground.y - point.y;
            girlPosition = point + girlOffset;
            D3DRMVectorRotate(&turned, &boxOffset, (Vector3*)&kVec3YAxis, angle);
            boxOffset = ScaledVector(turned, VectorLength(boxOffset));
            boxPosition = point + boxOffset;
            girlPosition.y += height;
            boxPosition.y += height;

            if (g_TrackGame->mode.raceSettings.raceMode != 0 &&
                g_TrackGame->mode.raceSettings.raceMode != 4) {
                sprintf(modelName, "%s\\%s", "Res", "FlagGirl.mcf");
                flagGirl = new(__FILE__, 0x26f) D3DIMSoultreeCharacter(1);
                AppendChild(flagGirl->LoadCharacter((int)renderContext, modelName, lights, textures, 1, 1),
                                      -1);
                ReplaceMeshId(flagGirl, 1, 2);
                reinterpret_cast<D3DIMSoultreeObject*>(flagGirl->modelNode)->SetUseLighting(1);
                flagGirl->modelNode->SetPositionVec3(girlPosition);
                flagGirl->modelNode->SetAxesPtr((const Vec3*)&direction, (const Vec3*)&kVec3YAxis, 1, 0);
                flagGirl->SetMotionByName("Stand");
                flagGirl->AdvanceMotionInterpolated(0.001f, 0, 0);
            }

            sprintf(name, "%s\\%s", "Res", "GirlBox.slt");
            ArcadeObject* box = new(__FILE__, 0x27c) ArcadeObject(1);
            box = box->Create(renderContext, (int)lights, (int)textures, name, kVec3Zero, 0, 0, 2.0f,
                                             0.5f, 0.5f, 0);
            AppendChild(box, -1);
            box->arcadeModel->SetUseLighting(1);
            box->SetPosition(&boxPosition);
            box->SetAxes(&direction, &kVec3YAxis, 1, 0);
            box->BuildCollision(name, 0x6a, 0, 0, 0, 1);
            g_TrackGame->eventManager->RemoveVegetationInRect(girlPosition.x - 4.0f, girlPosition.z - 4.0f,
                                                                       girlPosition.x + 4.0f, girlPosition.z + 4.0f);
        }
    } else if (mode == 0 || (mode == 4 && g_TrackGame->mode.raceSettings.stuntMode)) {
        BonusObjectManager* bonus = new(__FILE__, 0x28e) BonusObjectManager(1);
        scoreBoard = (int)bonus->SetupCounter(renderContext, (int)lights, (int)textures,
                                               (BonusBikeRaceView*)this, raceCamera);
        if (!AppendChild((GameObject*)scoreBoard, -1))
            return 0;
    }
    g_TrackGame->ui->StepProgressBar();

    SetSceneTime(0.0f);
    raceScene->holdAnimations = 1;
    if (g_TrackGame->mode.raceSettings.raceMode) {
        NumberObjectManager* numbers = new(__FILE__, 0x29b) NumberObjectManager(1);
        digitCounter = (int)numbers->SetupDigits(renderContext, (int)textures, (NumberBikeRaceView*)this, 0x1e,
                                                raceCamera, 3.0f);
        if (!AppendChild((GameObject*)digitCounter, -1))
            return 0;
    }
    if (g_TrackGame->mode.raceSettings.eventType != 4 &&
        g_TrackGame->mode.raceSettings.raceMode != 4 &&
        (g_TrackGame->playerCount > 1 || g_TrackGame->mode.raceSettings.aiRacerCount > 0)) {
        sprintf(name, "%s\\%s", "Res", "youind.slt");
        ArcadeObject* marker = new(__FILE__, 0x2a9) ArcadeObject(g_TrackGame->mode.playerIndicator);
        youIndicator = marker->Create(renderContext, (int)lights, (int)textures, name, position, 0, 0,
                                                    2.0f, 0.5f, 0.5f, 0);
        AppendChild(youIndicator, -1);
        Vector3 axis(0.0f, 1.0f, 0.0f);
        ((ArcadeObject*)youIndicator)->StartSpin(axis, 75.0f);
        ((ArcadeObject*)youIndicator)->arcadeModel->Scale(0.6f, 0.6f, 0.6f);
    }
    if (g_TrackGame->mode.raceSettings.raceMode == 4) {
        vcrFile = new(__FILE__, 0x2b6) VCRFile(1, 2);
        if (g_TrackGame->mode.ghostFile[0]) {
            vcrFile->Load(g_TrackGame->mode.ghostFile, "VCRgtemp.dat");
            ghostVcr = new(__FILE__, 0x2b9) KrustyVCR;
        }
        replayVcr = new(__FILE__, 0x2bd) KrustyVCR;
        ghostLap = 0;
    } else if (g_TrackGame->useReplayTape) {
        vcrFile = new(__FILE__, 0x2c2) VCRFile(0, 1);
        replayVcr = new(__FILE__, 0x2c3) KrustyVCR;
        if (g_TrackGame->playingReplay) {
            if (g_TrackGame->ui->playingDemo) {
                replayVcr->OpenRecording(RecorderCallback, 1, "ui\\demo.rpl", vcrFile);
                AppendChild(replayVcr, -1);
            } else {
                replayVcr->OpenRecording(RecorderCallback, 1, g_TrackGame->mode.replayFile,
                                                   vcrFile);
                AppendChild(replayVcr, -1);
            }
            savedCameraState = RaceCameraView()->cameraState;
            raceCamera->SetCameraState(1);
        }
    }
    g_TrackGame->ui->StepProgressBar();

    if (!CreateRacers(owner, lights, textures, terrain, event))
        return 0;
    g_TrackGame->ui->StepProgressBar();

    if (thirtySecondMan) {
        if (hasStartProbe) {
            localRacer->modelNode->GetPosition((Vec3*)&point);
            point.x += localRacer->heading.x * 15.0f;
            point.z += localRacer->heading.z * 15.0f;
        }
        terrain->QueryGround(reinterpret_cast<TerrainVec3*>(&point), 0, 0, 0);
        thirtySecondMan->modelNode->SetPositionVec3(point);
    }
    if (g_TrackGame->playingReplay && g_TrackGame->mode.raceSettings.raceMode != 4 &&
        g_TrackGame->useReplayTape) {
        if (g_TrackGame->ui->playingDemo) {
            DemoDlg* dialog = new(__FILE__, 0x2fb) DemoDlg;
            g_TrackGame->ui->gui->ShowDialog(dialog, 0, 1, 0, 0, 0, 0, 1);
        } else {
            g_TrackGame->ui->gui->ShowCursors(1);
            VCRDlg* dialog = new(__FILE__, 0x2ff) VCRDlg(1, "VCR.dtm");
            g_TrackGame->ui->gui->ShowDialog(dialog, 0, 1, 0, 0, 0, 0, 1);
        }
    }
    if (replayVcr && !g_TrackGame->playingReplay) {
        g_TrackGame->ui->StepProgressBar();
        if (g_TrackGame->mode.raceSettings.raceMode == 4) {
            COPY_TEXT(ghostFileRecorded, "VCRghost.dat", 0x104);
            replayVcr->OpenRecording(RecorderCallback, 0, ghostFileRecorded, vcrFile);
            if (ghostVcr) {
                COPY_TEXT(ghostFilePlayed, "VCRgtemp.dat", 0x104);
                ghostVcr->OpenRecording(RecorderCallback, 1, ghostFilePlayed, vcrFile);
                bestGhostLength = ghostVcr->RecordedLength();
                reinterpret_cast<KrustyBike*>(aiRacers[0])->SetNetRecorder(reinterpret_cast<KbRecorder*>(ghostVcr));
                if (startBeforeFinish)
                    aiRacers[0]->Disable();
                else
                    aiRacers[0]->Enable();
                AppendChild(ghostVcr, -1);
            }
        } else {
            replayVcr->OpenRecording(RecorderCallback, 0, "VCRtape.dat", vcrFile);
        }
        AppendChild(replayVcr, -1);
        isRecording = 1;
    }
    if (g_TrackGame->playerCount == 1 && g_TrackGame->mode.raceSettings.raceMode != 4) {
        int count = g_TrackGame->mode.raceSettings.aiRacerCount;
        localRacer->SetSpawnProtection(1, localRacer->GetSpawnProtectionDuration(), 0);
        for (int i = 0; i < count; i++)
            aiRacers[i]->SetSpawnProtection(1, aiRacers[i]->GetSpawnProtectionDuration(), 0);
    }
    g_TrackGame->ui->StepProgressBar();

    g_MemTagStack->Push("Audio");
    RaceSound* sound = new(__FILE__, 0x347) RaceSound(1);
    raceSound = (BikeRaceRaceSoundView*)sound->Create(renderContext, (EventBikeRaceView*)this,
                                                                      (RaceSoundKrustyBikeCameraView*)raceCamera, 6);
    AppendChild((GameObject*)raceSound, -1);
    g_MemTagStack->Push("BikeRace");
    if (g_TrackGame->mode.joystickOn) {
        g_TrackGame->controlInterface->activeJoystick->SetAcquired(0);
        g_TrackGame->controlInterface->activeJoystick->SetAutoCenter(g_TrackGame->mode.joystickOn);
        localRacer->InitForceFeedback();
        g_TrackGame->controlInterface->activeJoystick->SetAcquired(1);
    }
    if (g_TrackGame->mode.raceSettings.eventType)
        BuildStatusList(&statusList, (EventBikeRaceView*)this);
    if (ghostVcr)
        localRacer->fastestLap = bestGhostLength;
    g_TrackGame->ui->StepProgressBar();

    BikeNumberPainter* painter = new(__FILE__, 0x367) BikeNumberPainter(textures->manager);
    if (g_TrackGame->playingReplay) {
        painter->PaintNumber(reinterpret_cast<D3DIMSoultreeObject*>(localRacer->modelNode), localRacer->plateNumber);
        for (int i = 0; i < CountRaceRacers() - 1; i++)
            painter->PaintNumber(reinterpret_cast<D3DIMSoultreeObject*>(aiRacers[i]->modelNode), aiRacers[i]->plateNumber);
    } else if (g_TrackGame->playerCount > 1) {
        for (int i = 0; i < CountRaceRacers(); i++) {
            for (int j = 0; j < CountRaceRacers(); j++) {
                BikeRaceKrustyBikeView* racer = racerSlots[i];
                RacerSlot* slot = &g_TrackGame->mode.racers[j];
                if (racer->recorderId == slot->networkPlayerId && racer->aiMessageIndex == slot->racerIndex)
                    painter->PaintNumber(reinterpret_cast<D3DIMSoultreeObject*>(racer->modelNode), slot->plateNumber);
            }
        }
    } else {
        painter->PaintNumber(reinterpret_cast<D3DIMSoultreeObject*>(localRacer->modelNode), g_TrackGame->mode.plateNumber);
        if (g_TrackGame->mode.raceSettings.raceMode == 4) {
            painter->PaintNumber(reinterpret_cast<D3DIMSoultreeObject*>(aiRacers[0]->modelNode), g_TrackGame->mode.plateNumber);
        } else {
            for (int i = 0; i < g_TrackGame->mode.raceSettings.aiRacerCount; i++)
                painter->PaintNumber(reinterpret_cast<D3DIMSoultreeObject*>(aiRacers[i]->modelNode), aiRacers[i]->plateNumber);
        }
    }
    delete painter;
    g_TrackGame->ui->StepProgressBar();
    return this;
}

// A racer's skill, written into `skill`: `base` moved up or down by at most
// `range`, closer to `base` more often.
static inline void RandomSkill(float& skill, const float& base, const float& range) {
    float r = rand() * (1.0f / 32768.0f);
    skill = (float)((1.0 - exp(-r * r)) * range);
    if (rand() * (1.0f / 32768.0f) > 0.5f)
        skill = -skill;
    skill += base;
}

// The network view of start grid entry i (BikeRace reads +0x5 as the AI
// flag); the loader reads it through g_TrackGame each time.
static inline BikeRaceGridEntryView& GridPlayer(int i) {
    return ((BikeRaceGridEntryView*)g_TrackGame->mode.raceSettings.gridOrder)[i];
}

// Event AI racer n's record in the event manager; read through g_TrackGame
// each time (retail keeps only the byte offset n * 0x50 live).
static inline EventAiRacerRecord& EventAi(int n) {
    return g_TrackGame->eventManager->aiRacers[n];
}

// The front end's bike, manufacturer and rider tables, read through
// g_TrackGame at each use as retail does.
static inline BikeRaceBikeRecord& UiBike(int i) {
    return ((BikeRaceBikeRecord*)g_TrackGame->ui->bikes)[i];
}

static inline BikeRaceManufacturerRecord& UiManufacturer(int i) {
    return ((BikeRaceManufacturerRecord*)g_TrackGame->ui->manufacturers)[i];
}

static inline BikeRaceManufacturerRecord& UiRider(int i) {
    return ((BikeRaceManufacturerRecord*)g_TrackGame->ui->riders)[i];
}

// 0x00419970. Not exact (13400 of 13428 strict, linked 1.0): one prologue
// store slot, register choices at 0x00419b0d and 0x00419b39, a load/store
// order at 0x00419bf0, the SIB order of one curve copy (0x0041b124) and the
// own texture pair swapped in the frame. The order of `xor edi, edi` and
// `mov ebx, 1` at +538 moves with the unit's declaration count
// (docs/BIKERACE.md).
int BikeRace::CreateRacers(void* owner, LightManager* lights, TextureContext* textures,
                                    Terrain* terrain, RaceGateEvent* event) {
    const char* const resources = "Res";
    char path[0x104];
    char message[0x184];
    char bikeFile[0x104];
    char riderFile[0x104];
    char bikeTexture[0x40];
    char riderTexture[0x40];
    char name[0x10];
    char ownBikeTexture[0x40];
    char ownRiderTexture[0x40];
    char recordedBike[0x40];
    char recordedRider[0x40];
    char empty[0x104];
    int order[11];
    int ranking[11];
    Vector3 position;
    Vector3 direction;
    BikeEngineSetup setup;
    int playerId = 0x7fffffff;
    int recordIndex = 0;
    int id = 0;
    int i;
    float skill = aiSettings[0];
    Vector3 up(0.0f, 1.0f, 0.0f);
    empty[0] = 0;

    FileStream* stream = new(__FILE__, 0x3b6) FileStream(g_ResourceManager);
    sprintf(path, "%s\\rider.mcf", resources);
    if (!g_TrackGame->sceneObject->OpenArchiveFile(stream, path, "rb", (int)riderFile)) {
        sprintf(message, "No Rider MCF found in resources.  Aborting.\n");
        delete stream;
        Release();
        return 0;
    }

    if (g_TrackGame->playingReplay) {
        if (g_TrackGame->mode.raceSettings.networkPlayerCount) {
            racerCount = g_TrackGame->mode.raceSettings.networkPlayerCount +
                          g_TrackGame->mode.raceSettings.multiplayerAiRacerCount;
            g_TrackGame->mode.raceSettings.aiRacerCount = racerCount - 1;
            g_TrackGame->mode.raceSettings.networkPlayerCount = 0;
        } else {
            racerCount = g_TrackGame->mode.raceSettings.aiRacerCount + g_TrackGame->playerCount;
        }
    } else {
        if (g_TrackGame->mode.raceSettings.raceMode == 4)
            racerCount = 1;
        else
            racerCount = CountRaceRacers();
        recordIndex = 0;
        if (replayVcr)
            replayVcr->racerCount = racerCount;
    }
    if (g_TrackGame->playingReplay || g_TrackGame->playerCount == 1) {
        if (g_TrackGame->mode.raceSettings.raceMode == 2) {
            RankByPoints(ranking);
            for (int i = 0; i < racerCount; i++) {
                for (int j = 0; j < racerCount; j++) {
                    if (ranking[j] == i + 1) {
                        order[i] = j + 1;
                        break;
                    }
                }
            }
        } else {
            for (int i = 0; i < racerCount; i++)
                order[i] = i + 1;
        }
    }

    if (g_TrackGame->InProCircuitCareer()) {
        // The pro circuit: the player's racer, then the computer racers of
        // the career.
        TrackGame* game = g_TrackGame;
        ProCircuit* circuit = game->proCircuit;
        setup = *(BikeEngineSetup*)game->mode.garageOptions;
        setup.engineSize = circuit->racers[0].engineSize;
        setup.fourStroke = circuit->racers[0].fourStroke;
        int bikeClass = BikeClassOf(setup.engineSize);
        setup.rpmLowerLimit = game->ui->rpmLowerLimit[bikeClass];
        setup.rpmUpperLimit = game->ui->rpmUpperLimit[bikeClass];
        setup.rpmStep = (setup.rpmUpperLimit - setup.rpmLowerLimit) / 10;
        setup.weight = game->ui->weight[bikeClass];
        WriteRelativePositions(&position, &direction, event, order[0]);
        sprintf(path, "%s\\%s", resources, circuit->racers[0].bikeModel);
        if (!g_TrackGame->sceneObject->OpenArchiveFile(stream, path, "rb", (int)bikeFile)) {
            sprintf(message, "No PC Rider MCF found in resources.  Aborting.\n");
            delete stream;
            Release();
            return 0;
        }
        char riderPath[0x104];
        char circuitRiderFile[0x104];
        if (circuit->careerFlags & 2) {
            sprintf(riderPath, "%s\\%s", resources, "riderd.mcf");
        } else {
            COPY_TEXT(riderPath, riderFile, 0x104);
        }
        if (!g_TrackGame->sceneObject->OpenArchiveFile(stream, riderPath, "rb",
                                                                       (int)circuitRiderFile)) {
            sprintf(message, "No PC Rider MCF found in resources.  Aborting.\n");
            delete stream;
            Release();
            return 0;
        }
        localRacer = new(__FILE__, 0x42a) BikeRaceKrustyBikeView(1);
        reinterpret_cast<KrustyBike*>(localRacer)->Load(owner, "", bikeFile, circuitRiderFile, lights, textures, position,
                                           direction, up, terrain, g_TrackGame->controlInterface, 0, 0, this,
                                           g_TrackGame->mode.profileName, 0, 0, (const KbBikeSetup*)&setup,
                                           g_TrackGame->mode.gasGyro,
                                           g_TrackGame->mode.brakeGyro, particleManager, raceScene,
                                           replayVcr, 0, !(circuit->careerFlags & 2));
        AppendChild(localRacer, -1);
        g_TrackGame->ui->ResolveBikeFile(circuit->racers[0].bikeName, ownBikeTexture, 0x3f);
        localRacer->modelNode->ReplaceMaterialTexture(0, ownBikeTexture, textures);
        if (!(circuit->careerFlags & 2)) {
            g_TrackGame->ui->ResolveRiderFile(circuit->racers[0].riderModel, ownRiderTexture,
                                                             0x3f);
            localRacer->riderCharacter->modelNode->ReplaceMaterialTexture(0, ownRiderTexture, textures);
        }
        if (g_TrackGame->mode.raceSettings.raceMode == 1) {
            id = 1;
            aiRacers = (BikeRaceKrustyBikeView**)DebugMalloc(
                (g_TrackGame->proCircuit->racerCount - 1) * sizeof(BikeRaceKrustyBikeView*), __FILE__,
                0x44f);
            for (int i = 0; i < g_TrackGame->proCircuit->racerCount - 1; i++)
                aiRacers[i] = 0;
            for (int n = 1; n < g_TrackGame->proCircuit->racerCount; n++) {
                ProCircuitRacer* racer = &circuit->racers[n];
                WriteRelativePositions(&position, &direction, event, order[n]);
                sprintf(path, "%s\\%s", resources, racer->bikeModel);
                if (!g_TrackGame->sceneObject->OpenArchiveFile(stream, path, "rb", (int)bikeFile)) {
                    sprintf(message, "No PC Rider MCF found in resources.  Aborting.\n");
                    delete stream;
                    Release();
                    return 0;
                }
                setup = *(BikeEngineSetup*)g_TrackGame->mode.garageOptions;
                setup.engineSize = racer->engineSize;
                setup.fourStroke = racer->fourStroke;
                bikeClass = BikeClassOf(setup.engineSize);
                setup.rpmLowerLimit = g_TrackGame->ui->rpmLowerLimit[bikeClass];
                setup.rpmUpperLimit = g_TrackGame->ui->rpmUpperLimit[bikeClass];
                for (int k = 0; k < 11; k++)
                    setup.presetCurve[k] = g_TrackGame->ui->standardCurves[bikeClass][0][k];
                setup.rpmStep = (setup.rpmUpperLimit - setup.rpmLowerLimit) / 10;
                setup.weight = g_TrackGame->ui->weight[bikeClass];
                RandomSkill(skill, aiSettings[0], aiSettings[1]);
                aiRacers[n - 1] = new(__FILE__, 0x47c) BikeRaceKrustyBikeView(1);
                reinterpret_cast<KrustyBike*>(aiRacers[n - 1])->Load(owner, empty, bikeFile, riderFile, lights, textures,
                                                          position, direction, up, terrain, 0, 1, 0, this,
                                                          racer->racerName, 0, id, (const KbBikeSetup*)&setup, 1, 1, particleManager,
                                                          raceScene, replayVcr, skill, 1);
                AppendChild(aiRacers[n - 1], -1);
                aiRacers[n - 1]->modelNode->ReplaceMaterialTexture(0, racer->bikeName, textures);
                aiRacers[n - 1]->riderCharacter->modelNode->ReplaceMaterialTexture(0, racer->riderModel,
                                                                                     textures);
                id++;
            }
        }
        return 1;
    }

    int used[8];
    if (!g_TrackGame->playingReplay && g_TrackGame->playerCount > 1) {
        racerSlots = new(__FILE__, 0x4a3) BikeRaceKrustyBikeView*[racerCount];
        memset(used, 0, sizeof(used));
    }
    g_TrackGame->ui->StepProgressBar();
    char ai = 0;
    int* bikes = (int*)DebugCalloc(racerCount, 4, __FILE__, 0x4af);
    int* riders = (int*)DebugCalloc(racerCount, 4, __FILE__, 0x4b0);
    const char** names = (const char**)DebugMalloc(racerCount * 4, __FILE__, 0x4b1);
    int plate;
    int engineSize;
    int fourStroke;
    if (g_TrackGame->playingReplay) {
        // A replay: the recorded player.
        recordIndex = 1;
        replayVcr->GetRacer(&id, &ai, 0, name, bikeFile, riderFile, recordedBike, recordedRider,
                                           &engineSize, &fourStroke, &plate);
        g_TrackGame->ui->ResolveBikeFile(recordedBike, bikeTexture, 0x3f);
        g_TrackGame->ui->ResolveRiderFile(recordedRider, riderTexture, 0x3f);
    } else {
        sprintf(path, "%s\\%s", resources, g_TrackGame->mode.racerChoice.bikeModel);
        if (!g_TrackGame->sceneObject->OpenArchiveFile(stream, path, "rb", (int)bikeFile)) {
            sprintf(message, "No Rider MCF found in resources.  Aborting.\n");
            delete stream;
            Release();
            return 0;
        }
        g_TrackGame->ui->ResolveBikeFile(g_TrackGame->mode.racerChoice.bikeName,
                                                         bikeTexture, 0x3f);
        g_TrackGame->ui->ResolveRiderFile(g_TrackGame->mode.racerChoice.riderName,
                                                         riderTexture, 0x3f);
        if (!g_TrackGame->mode.raceSettings.multiplayerAiRacerCount) {
            g_TrackGame->ui->PickRandomRacers(bikes, racerCount, riders);
            g_TrackGame->ui->MakeRandomNames(names, racerCount);
        }
        strcpy(name, g_TrackGame->mode.profileName);
        plate = g_TrackGame->mode.plateNumber;
    }

    if (g_TrackGame->playingReplay || g_TrackGame->playerCount == 1) {
        // The player's racer offline (or in a replay).
        WriteRelativePositions(&position, &direction, event, order[0]);
        localRacer = new(__FILE__, 0x4ea) BikeRaceKrustyBikeView(1);
        if (!AppendChild(
                reinterpret_cast<KrustyBike*>(localRacer)->Load(
                    owner, empty, bikeFile, riderFile, lights, textures, position, direction, up, terrain,
                    g_TrackGame->controlInterface, 0, 0, this, name, 0, id,
                    (const KbBikeSetup*)g_TrackGame->mode.garageOptions,
                    g_TrackGame->mode.gasGyro, g_TrackGame->mode.brakeGyro, particleManager,
                    raceScene, replayVcr, 0, 1),
                -1)) {
            sprintf(message, "Player Bike not loaded\n");
            if (riders)
                DebugFree(riders, __FILE__, 0x4ff);
            if (bikes)
                DebugFree(bikes, __FILE__, 0x500);
            if (names)
                DebugFree(names, __FILE__, 0x501);
            delete stream;
            Release();
            return 0;
        }
        localRacer->modelNode->ReplaceMaterialTexture(0, bikeTexture, textures);
        localRacer->riderCharacter->modelNode->ReplaceMaterialTexture(0, riderTexture, textures);
        if (!g_TrackGame->mode.raceSettings.treeCollision) {
            localRacer->collisionObject->ignoreVegetation = 1;
            localRacer->frontWheel->ignoreVegetation = 1;
            localRacer->rearWheel->ignoreVegetation = 1;
        }
        localRacer->engineSize = ((BikeEngineSetup*)g_TrackGame->mode.garageOptions)->engineSize;
        localRacer->fourStroke = (char)((BikeEngineSetup*)g_TrackGame->mode.garageOptions)->fourStroke;
        localRacer->plateNumber = plate;
        if ((replayVcr && g_TrackGame->useReplayTape && !g_TrackGame->playingReplay) ||
            g_TrackGame->mode.raceSettings.raceMode == 4) {
            replayVcr->SetRacer(0, 0, recordIndex++, g_TrackGame->mode.profileName, bikeFile,
                                               riderFile, bikeTexture, riderTexture, localRacer->engineSize,
                                               (unsigned char)localRacer->fourStroke, plate);
        }
    } else {
        // A network race: this player's racer, the other players' and the
        // AI racers of other hosts.
        playerId = g_TrackGame->network->localPlayer;
        for (i = 0; i < racerCount; i++) {
            if (GridPlayer(i).networkPlayerId == playerId && !GridPlayer(i).isAi) {
                int j;
                for (j = 0; j < g_TrackGame->mode.racerCount; j++) {
                    if (GridPlayer(i).networkPlayerId == g_TrackGame->mode.racers[j].networkPlayerId && g_TrackGame->mode.racers[j].racerIndex == 0) {
                        used[j] = 1;
                        break;
                    }
                }
                WriteRelativePositions(&position, &direction, event, i + 1);
                BikeRaceKrustyBikeView* racer = new(__FILE__, 0x53b) BikeRaceKrustyBikeView(1);
                racerSlots[j] = racer;
                if (!AppendChild(
                        reinterpret_cast<KrustyBike*>(racer)->Load(
                            owner, empty, bikeFile, riderFile, lights, textures, position, direction, up, terrain,
                            g_TrackGame->controlInterface, 0, 0, this, g_TrackGame->mode.profileName, 0,
                            playerId, (const KbBikeSetup*)g_TrackGame->mode.garageOptions,
                            g_TrackGame->mode.gasGyro, g_TrackGame->mode.brakeGyro,
                            particleManager, raceScene, replayVcr, 0, 1),
                        -1)) {
                    sprintf(message, "Net Bike%d not loaded\n", j);
                    if (riders)
                        DebugFree(riders, __FILE__, 0x550);
                    if (bikes)
                        DebugFree(bikes, __FILE__, 0x551);
                    if (names)
                        DebugFree(names, __FILE__, 0x552);
                    delete stream;
                    Release();
                    return 0;
                }
                g_TrackGame->ui->ResolveBikeFile(g_TrackGame->mode.racerChoice.bikeName,
                                                                 ownBikeTexture, 0x3f);
                g_TrackGame->ui->ResolveRiderFile(g_TrackGame->mode.racerChoice.riderName,
                                                                 ownRiderTexture, 0x3f);
                if (!g_TrackGame->mode.raceSettings.treeCollision) {
                    racerSlots[j]->collisionObject->ignoreVegetation = 1;
                    racerSlots[j]->frontWheel->ignoreVegetation = 1;
                    racerSlots[j]->rearWheel->ignoreVegetation = 1;
                }
                racerSlots[j]->modelNode->ReplaceMaterialTexture(0, ownBikeTexture, textures);
                racerSlots[j]->riderCharacter->modelNode->ReplaceMaterialTexture(0, ownRiderTexture, textures);
                racerSlots[j]->plateNumber = g_TrackGame->mode.plateNumber;
                if (replayVcr && g_TrackGame->useReplayTape && !g_TrackGame->playingReplay) {
                    replayVcr->SetRacer(playerId, 0, recordIndex++, g_TrackGame->mode.profileName,
                                                       bikeFile, riderFile, ownBikeTexture, ownRiderTexture,
                                                       racerSlots[j]->engineSize,
                                                       (unsigned char)racerSlots[j]->fourStroke, g_TrackGame->mode.racers[j].plateNumber);
                }
                localRacer = racerSlots[j];
                targetRacer = j;
                break;
            }
        }
        for (i = 0; i < racerCount; i++) {
            if (GridPlayer(i).networkPlayerId == playerId || GridPlayer(i).isAi)
                continue;
            int j;
            for (j = 0; j < g_TrackGame->mode.racerCount; j++) {
                if (GridPlayer(i).networkPlayerId == g_TrackGame->mode.racers[j].networkPlayerId && g_TrackGame->mode.racers[j].racerIndex == 0) {
                    used[j] = 1;
                    break;
                }
            }
            WriteRelativePositions(&position, &direction, event, i + 1);
            setup = *(BikeEngineSetup*)g_TrackGame->mode.garageOptions;
            setup.engineSize = g_TrackGame->mode.racers[j].engineSize;
            setup.fourStroke = g_TrackGame->mode.racers[j].fourStroke;
            int bikeClass = BikeClassOf(setup.engineSize);
            setup.rpmLowerLimit = g_TrackGame->ui->rpmLowerLimit[bikeClass];
            setup.rpmUpperLimit = g_TrackGame->ui->rpmUpperLimit[bikeClass];
            for (int k = 0; k < 11; k++)
                setup.presetCurve[k] = g_TrackGame->ui->standardCurves[bikeClass][0][k];
            setup.rpmStep = (setup.rpmUpperLimit - setup.rpmLowerLimit) / 10;
            setup.weight = g_TrackGame->ui->weight[bikeClass];
            sprintf(path, "%s\\%s", resources, g_TrackGame->mode.racers[j].bikeModelName);
            BikeRaceKrustyBikeView* racer = new(__FILE__, 0x5a3) BikeRaceKrustyBikeView(1);
            racerSlots[j] = racer;
            if (!AppendChild(reinterpret_cast<KrustyBike*>(racer)->Load(
                                           owner, empty, path, riderFile, lights, textures, position, direction,
                                           up, terrain, 0, 0, 1, this, g_TrackGame->mode.racers[j].playerName, 0, g_TrackGame->mode.racers[j].networkPlayerId,
                                           (const KbBikeSetup*)&setup, 0, 1, particleManager, raceScene, replayVcr, 0, 1),
                                       -1)) {
                sprintf(message, "Net Bike%d not loaded\n", j);
            }
            g_TrackGame->ui->ResolveBikeFile(g_TrackGame->mode.racers[j].bikeName, bikeTexture, 0x3f);
            racerSlots[j]->modelNode->ReplaceMaterialTexture(0, bikeTexture, textures);
            if (g_TrackGame->GetRegistryInt("MPR", 0) == 123 && g_TrackGame->mode.racers[j].hasMprSkin) {
                racerSlots[j]->riderCharacter->modelNode->ReplaceMaterialTexture(0, "MPR.tga", textures);
            } else {
                g_TrackGame->ui->ResolveRiderFile(g_TrackGame->mode.racers[j].riderName, riderTexture, 0x3f);
                racerSlots[j]->riderCharacter->modelNode->ReplaceMaterialTexture(0, riderTexture, textures);
            }
            if (!g_TrackGame->mode.raceSettings.treeCollision) {
                racerSlots[j]->collisionObject->ignoreVegetation = 1;
                racerSlots[j]->frontWheel->ignoreVegetation = 1;
                racerSlots[j]->rearWheel->ignoreVegetation = 1;
            }
            racerSlots[j]->Disable();
            racerSlots[j]->messageDelay = g_TrackGame->mode.racers[j].lowestMessageDelay;
            if (replayVcr && g_TrackGame->useReplayTape && !g_TrackGame->playingReplay) {
                replayVcr->SetRacer(g_TrackGame->mode.racers[j].networkPlayerId, 0, recordIndex++, g_TrackGame->mode.racers[j].playerName, path,
                                                   riderFile, bikeTexture, riderTexture, racerSlots[j]->engineSize,
                                                   (unsigned char)racerSlots[j]->fourStroke, g_TrackGame->mode.racers[j].plateNumber);
            }
        }
        for (i = 0; i < racerCount; i++) {
            int other = GridPlayer(i).networkPlayerId;
            if (other == playerId || !GridPlayer(i).isAi)
                continue;
            int j;
            for (j = 0; j < racerCount; j++) {
                if (other == g_TrackGame->mode.racers[j].networkPlayerId && g_TrackGame->mode.racers[j].racerIndex != 0 && !used[j]) {
                    used[j] = 1;
                    break;
                }
            }
            WriteRelativePositions(&position, &direction, event, i + 1);
            setup = *(BikeEngineSetup*)g_TrackGame->mode.garageOptions;
            setup.engineSize = g_TrackGame->mode.racers[j].engineSize;
            setup.fourStroke = g_TrackGame->mode.racers[j].fourStroke;
            int bikeClass = BikeClassOf(setup.engineSize);
            setup.rpmLowerLimit = g_TrackGame->ui->rpmLowerLimit[bikeClass];
            setup.rpmUpperLimit = g_TrackGame->ui->rpmUpperLimit[bikeClass];
            for (int k = 0; k < 11; k++)
                setup.presetCurve[k] = g_TrackGame->ui->standardCurves[bikeClass][0][k];
            setup.rpmStep = (setup.rpmUpperLimit - setup.rpmLowerLimit) / 10;
            setup.weight = g_TrackGame->ui->weight[bikeClass];
            BikeRaceKrustyBikeView* racer = new(__FILE__, 0x61f) BikeRaceKrustyBikeView(1);
            racerSlots[j] = racer;
            if (!AppendChild(reinterpret_cast<KrustyBike*>(racer)->Load(
                                           owner, empty, g_TrackGame->mode.racers[j].bikeModelName, riderFile, lights, textures, position,
                                           direction, up, terrain, 0, (char)g_TrackGame->mode.racers[j].racerIndex, 1, this,
                                           g_TrackGame->mode.racers[j].playerName, 0, g_TrackGame->mode.racers[j].networkPlayerId, (const KbBikeSetup*)&setup, 0, 1, particleManager,
                                           raceScene, replayVcr, 0, 1),
                                       -1)) {
                sprintf(message, "Net Bike%d not loaded\n", j);
            }
            racerSlots[j]->modelNode->ReplaceMaterialTexture(0, g_TrackGame->mode.racers[j].bikeName, textures);
            racerSlots[j]->riderCharacter->modelNode->ReplaceMaterialTexture(0, g_TrackGame->mode.racers[j].riderName, textures);
            if (!g_TrackGame->mode.raceSettings.treeCollision) {
                racerSlots[j]->collisionObject->ignoreVegetation = 1;
                racerSlots[j]->frontWheel->ignoreVegetation = 1;
                racerSlots[j]->rearWheel->ignoreVegetation = 1;
            }
            racerSlots[j]->Disable();
            racerSlots[j]->messageDelay = g_TrackGame->mode.racers[j].lowestMessageDelay;
            if (replayVcr && g_TrackGame->useReplayTape && !g_TrackGame->playingReplay) {
                replayVcr->SetRacer(g_TrackGame->mode.racers[j].networkPlayerId, (char)g_TrackGame->mode.racers[j].racerIndex, recordIndex++,
                                                   g_TrackGame->mode.racers[j].playerName, g_TrackGame->mode.racers[j].bikeModelName, riderFile,
                                                   g_TrackGame->mode.racers[j].bikeName, g_TrackGame->mode.racers[j].riderName,
                                                   racerSlots[j]->engineSize,
                                                   (unsigned char)racerSlots[j]->fourStroke, g_TrackGame->mode.racers[j].plateNumber);
            }
        }
    }

    ((FollowCamera*)raceCamera)->AppendTableEntry(position, 140.0f, 5.0f, 5.0f, 2.5f, 2.3f);
    ((FollowCamera*)raceCamera)->AppendTableEntry(position, 140.0f, 5.0f, 5.0f, 2.5f, 2.3f);

    int fromEvent = 0;
    if ((g_TrackGame->playingReplay && replayVcr->racerCount > 1) || g_TrackGame->playerCount == 1) {
        if (g_TrackGame->mode.raceSettings.raceMode == 4) {
            // The ghost.
            aiRacers = new(__FILE__, 0x674) BikeRaceKrustyBikeView*[1];
            WriteRelativePositions(&position, &direction, event, order[0]);
            sprintf(bikeFile, "%s\\%s", resources, "Ghost.mcf");
            sprintf(riderFile, "%s\\GhostRider.mcf", resources);
            BikeRaceKrustyBikeView* racer = new(__FILE__, 0x67c) BikeRaceKrustyBikeView(1);
            aiRacers[0] = racer;
            if (!AppendChild(
                    reinterpret_cast<KrustyBike*>(racer)->Load(
                        owner, empty, bikeFile, riderFile, lights, textures, position, direction, up, terrain, 0, -1,
                        0, this, "Ghost", 0, 0, (const KbBikeSetup*)g_TrackGame->mode.garageOptions,
                        g_TrackGame->mode.gasGyro, g_TrackGame->mode.brakeGyro,
                        particleManager, raceScene, replayVcr, 0, 1),
                    -1)) {
                sprintf(message, "Ghost Bike not loaded\n");
            }
            replayVcr->SetRacer(0, 0, recordIndex, g_TrackGame->mode.profileName, bikeFile,
                                               riderFile, bikeTexture, riderTexture, aiRacers[0]->engineSize,
                                               (unsigned char)aiRacers[0]->fourStroke,
                                               g_TrackGame->mode.plateNumber);
            aiRacers[0]->SetSpawnProtection(1, 0, 1);
            aiRacers[0]->Disable();
        } else if (g_TrackGame->mode.raceSettings.aiRacerCount > 0) {
            // The AI racers.
            id = 1;
            aiRacers = new(__FILE__, 0x6a5) BikeRaceKrustyBikeView*[g_TrackGame->mode.raceSettings.aiRacerCount];
            for (int i = 0; i < g_TrackGame->mode.raceSettings.aiRacerCount; i++)
                aiRacers[i] = 0;
            for (int n = 0; n < g_TrackGame->mode.raceSettings.aiRacerCount; n++) {
                char aiName[0x10];
                if (g_TrackGame->playingReplay) {
                    replayVcr->GetRacer(&id, &ai, recordIndex, aiName, bikeFile, riderFile,
                                                       recordedBike, recordedRider, &engineSize, &fourStroke,
                                                       &plate);
                    setup = *(BikeEngineSetup*)g_TrackGame->mode.garageOptions;
                    g_TrackGame->ui->ResolveBikeFile(recordedBike, bikeTexture, 0x3f);
                    g_TrackGame->ui->ResolveRiderFile(recordedRider, riderTexture, 0x3f);
                } else {
                    if (g_TrackGame->mode.raceSettings.races > 0 &&
                        g_TrackGame->eventManager->racesRun > 0) {
                        fromEvent = 1;
                        COPY_TEXT(aiName, g_TrackGame->eventManager->results[n + 1].name, 0x10);
                        COPY_TEXT(bikeFile, EventAi(n).bikeModelPath, 0x104);
                    } else {
                        COPY_TEXT(aiName, names[n], 0x10);
                        sprintf(bikeFile, "%s\\%s", resources, UiManufacturer(UiBike(bikes[n]).manufacturer).modelFile);
                    }
                    ai = 1;
                    if (!g_TrackGame->sceneObject->OpenArchiveFile(stream, bikeFile, "rb",
                                                                                   (int)bikeFile)) {
                        sprintf(message, "No Net Bike MCF found in resources.  Aborting.\n");
                        delete stream;
                        Release();
                        return 0;
                    }
                    setup = *(BikeEngineSetup*)g_TrackGame->mode.garageOptions;
                    if (fromEvent) {
                        engineSize = EventAi(n).engineSize;
                        fourStroke = EventAi(n).fourStroke;
                        skill = EventAi(n).skill;
                    } else {
                        engineSize = UiBike(bikes[n]).engineSize;
                        fourStroke = UiBike(bikes[n]).fourStroke;
                        if (g_TrackGame->mode.raceSettings.eventType == 0)
                            RandomSkill(skill, aiSettings[5], aiSettings[6]);
                        else
                            RandomSkill(skill, aiSettings[0], aiSettings[1]);
                    }
                    float r = rand() * (1.0f / 32768.0f);
                    plate = 101 - (int)(r * -898.0f);
                }
                WriteRelativePositions(&position, &direction, event, order[n + 1]);
                setup.engineSize = engineSize;
                setup.fourStroke = fourStroke;
                int bikeClass = BikeClassOf(setup.engineSize);
                setup.rpmLowerLimit = g_TrackGame->ui->rpmLowerLimit[bikeClass];
                setup.rpmUpperLimit = g_TrackGame->ui->rpmUpperLimit[bikeClass];
                for (int k = 0; k < 11; k++)
                    setup.presetCurve[k] = g_TrackGame->ui->standardCurves[bikeClass][0][k];
                setup.rpmStep = (setup.rpmUpperLimit - setup.rpmLowerLimit) / 10;
                setup.weight = g_TrackGame->ui->weight[bikeClass];
                BikeRaceKrustyBikeView* racer = new(__FILE__, 0x718) BikeRaceKrustyBikeView(1);
                aiRacers[n] = racer;
                if (!AppendChild(reinterpret_cast<KrustyBike*>(racer)->Load(
                                               owner, empty, bikeFile, riderFile, lights, textures, position,
                                               direction, up, terrain, 0, ai, 0, this, aiName, 0, id, (const KbBikeSetup*)&setup, 1, 1,
                                               particleManager, raceScene, replayVcr, skill, 1),
                                           -1)) {
                    sprintf(message, "AI Bike%d not loaded\n", n);
                }
                if (g_TrackGame->playingReplay) {
                    char text[0x40];
                    COPY_TEXT(text, bikeTexture, 0x40);
                    g_TrackGame->ui->ResolveBikeFile(text, bikeTexture, 0x3f);
                    aiRacers[n]->modelNode->ReplaceMaterialTexture(0, bikeTexture, textures);
                    COPY_TEXT(text, riderTexture, 0x40);
                    g_TrackGame->ui->ResolveRiderFile(text, riderTexture, 0x3f);
                    aiRacers[n]->riderCharacter->modelNode->ReplaceMaterialTexture(0, riderTexture, textures);
                    aiRacers[n]->plateNumber = plate;
                    recordIndex++;
                } else if (fromEvent) {
                    g_TrackGame->ui->ResolveBikeFile(EventAi(n).bikeTexture, bikeTexture, 0x3f);
                    aiRacers[n]->modelNode->ReplaceMaterialTexture(0, bikeTexture, textures);
                    g_TrackGame->ui->ResolveRiderFile(EventAi(n).riderTexture, riderTexture, 0x3f);
                    aiRacers[n]->riderCharacter->modelNode->ReplaceMaterialTexture(0, riderTexture, textures);
                    aiRacers[n]->plateNumber = plate;
                } else {
                    g_TrackGame->ui->ResolveBikeFile(UiBike(bikes[n]).texture, bikeTexture, 0x3f);
                    aiRacers[n]->modelNode->ReplaceMaterialTexture(0, bikeTexture, textures);
                    g_TrackGame->ui->ResolveRiderFile(UiRider(riders[n]).modelFile, riderTexture,
                                                                     0x3f);
                    aiRacers[n]->riderCharacter->modelNode->ReplaceMaterialTexture(0, riderTexture, textures);
                    aiRacers[n]->plateNumber = plate;
                }
                if (!g_TrackGame->mode.raceSettings.treeCollision) {
                    aiRacers[n]->collisionObject->ignoreVegetation = 1;
                    aiRacers[n]->frontWheel->ignoreVegetation = 1;
                    aiRacers[n]->rearWheel->ignoreVegetation = 1;
                }
                if (replayVcr && g_TrackGame->useReplayTape && !g_TrackGame->playingReplay) {
                    replayVcr->SetRacer(id, 1, recordIndex++, aiName, path, riderFile, bikeTexture,
                                                       riderTexture, aiRacers[n]->engineSize,
                                                       (unsigned char)aiRacers[n]->fourStroke, plate);
                }
                id++;
                if (g_TrackGame->mode.raceSettings.raceMode == 2 &&
                    g_TrackGame->mode.raceSettings.races > 0 &&
                    !g_TrackGame->eventManager->racesRun) {
                    strcpy(EventAi(n).bikeTexture, bikeTexture);
                    strcpy(EventAi(n).riderTexture, riderTexture);
                    strcpy(EventAi(n).bikeModelPath, bikeFile);
                    EventAi(n).engineSize = engineSize;
                    EventAi(n).fourStroke = fourStroke;
                    EventAi(n).skill = skill;
                }
            }
        }
    } else {
        // A network race: the AI racers this host runs.
        id = 1;
        if (g_TrackGame->mode.raceSettings.aiRacerCount > 0) {
            aiRacers = new(__FILE__, 0x770) BikeRaceKrustyBikeView*[g_TrackGame->mode.raceSettings.aiRacerCount];
            for (i = 0; i < racerCount; i++) {
                int other = GridPlayer(i).networkPlayerId;
                if (other != playerId || !GridPlayer(i).isAi)
                    continue;
                int j;
                for (j = 0; j < g_TrackGame->mode.racerCount; j++) {
                    if (other == g_TrackGame->mode.racers[j].networkPlayerId && g_TrackGame->mode.racers[j].racerIndex != 0 && !used[j]) {
                        used[j] = 1;
                        break;
                    }
                }
                WriteRelativePositions(&position, &direction, event, i + 1);
                setup = *(BikeEngineSetup*)g_TrackGame->mode.garageOptions;
                setup.engineSize = g_TrackGame->mode.racers[j].engineSize;
                setup.fourStroke = g_TrackGame->mode.racers[j].fourStroke;
                int bikeClass = BikeClassOf(setup.engineSize);
                setup.rpmLowerLimit = g_TrackGame->ui->rpmLowerLimit[bikeClass];
                setup.rpmUpperLimit = g_TrackGame->ui->rpmUpperLimit[bikeClass];
                for (int k = 0; k < 11; k++)
                    setup.presetCurve[k] = g_TrackGame->ui->standardCurves[bikeClass][0][k];
                setup.rpmStep = (setup.rpmUpperLimit - setup.rpmLowerLimit) / 10;
                setup.weight = g_TrackGame->ui->weight[bikeClass];
                BikeRaceKrustyBikeView* racer = new(__FILE__, 0x78f) BikeRaceKrustyBikeView(1);
                racerSlots[j] = racer;
                if (!AppendChild(reinterpret_cast<KrustyBike*>(racer)->Load(
                                               owner, empty, g_TrackGame->mode.racers[j].bikeModelName, riderFile, lights, textures,
                                               position, direction, up, terrain, 0, (char)g_TrackGame->mode.racers[j].racerIndex, 0,
                                               this, g_TrackGame->mode.racers[j].playerName, 0, g_TrackGame->mode.racers[j].networkPlayerId, (const KbBikeSetup*)&setup, 0, 1,
                                               particleManager, raceScene, replayVcr, 0, 1),
                                           -1)) {
                    sprintf(message, "Net Bike%d not loaded\n", j);
                }
                racerSlots[j]->modelNode->ReplaceMaterialTexture(0, g_TrackGame->mode.racers[j].bikeName, textures);
                racerSlots[j]->riderCharacter->modelNode->ReplaceMaterialTexture(0, g_TrackGame->mode.racers[j].riderName, textures);
                if (!g_TrackGame->mode.raceSettings.treeCollision) {
                    racerSlots[j]->collisionObject->ignoreVegetation = 1;
                    racerSlots[j]->frontWheel->ignoreVegetation = 1;
                    racerSlots[j]->rearWheel->ignoreVegetation = 1;
                }
                if (replayVcr && g_TrackGame->useReplayTape && !g_TrackGame->playingReplay) {
                    replayVcr->SetRacer(g_TrackGame->mode.racers[j].networkPlayerId, (char)g_TrackGame->mode.racers[j].racerIndex, recordIndex++,
                                                       g_TrackGame->mode.racers[j].playerName, g_TrackGame->mode.racers[j].bikeModelName, riderFile,
                                                       g_TrackGame->mode.racers[j].bikeName, g_TrackGame->mode.racers[j].riderName,
                                                       racerSlots[j]->engineSize,
                                                       (unsigned char)racerSlots[j]->fourStroke,
                                                       g_TrackGame->mode.racers[j].plateNumber);
                }
                aiRacers[id - 1] = racerSlots[j];
                id++;
            }
        }
        // Every racer's collisions against every other racer's.
        for (int a = 0; a < racerCount; a++) {
            if (!racerSlots[a]->isRemote)
                continue;
            for (int b = 0; b < racerCount; b++) {
                if (a == b)
                    continue;
                if (racerSlots[b]->isRemote) {
                    racerSlots[a]->bikeCollision->AddIgnoredOwner(racerSlots[b]->bikeCollision);
                    racerSlots[a]->bikeCollision->AddIgnoredOwner(racerSlots[b]->bikeAndRiderCollision);
                    racerSlots[a]->bikeCollision->AddIgnoredOwner(racerSlots[b]->frontWheel);
                    racerSlots[a]->bikeCollision->AddIgnoredOwner(racerSlots[b]->rearWheel);
                    racerSlots[a]->bikeCollision->AddIgnoredOwner(racerSlots[b]->wrecker->collisionModel);
                    racerSlots[a]->bikeAndRiderCollision->AddIgnoredOwner(racerSlots[b]->bikeCollision);
                    racerSlots[a]->bikeAndRiderCollision->AddIgnoredOwner(racerSlots[b]->bikeAndRiderCollision);
                    racerSlots[a]->bikeAndRiderCollision->AddIgnoredOwner(racerSlots[b]->frontWheel);
                    racerSlots[a]->bikeAndRiderCollision->AddIgnoredOwner(racerSlots[b]->rearWheel);
                    racerSlots[a]->bikeAndRiderCollision->AddIgnoredOwner(racerSlots[b]->wrecker->collisionModel);
                }
                racerSlots[a]->frontWheel->AddIgnoredOwner(racerSlots[b]->bikeCollision);
                racerSlots[a]->frontWheel->AddIgnoredOwner(racerSlots[b]->bikeAndRiderCollision);
                racerSlots[a]->frontWheel->AddIgnoredOwner(racerSlots[b]->frontWheel);
                racerSlots[a]->frontWheel->AddIgnoredOwner(racerSlots[b]->rearWheel);
                racerSlots[a]->frontWheel->AddIgnoredOwner(racerSlots[b]->wrecker->collisionModel);
                racerSlots[a]->rearWheel->AddIgnoredOwner(racerSlots[b]->bikeCollision);
                racerSlots[a]->rearWheel->AddIgnoredOwner(racerSlots[b]->bikeAndRiderCollision);
                racerSlots[a]->rearWheel->AddIgnoredOwner(racerSlots[b]->frontWheel);
                racerSlots[a]->rearWheel->AddIgnoredOwner(racerSlots[b]->rearWheel);
                racerSlots[a]->rearWheel->AddIgnoredOwner(racerSlots[b]->wrecker->collisionModel);
                racerSlots[a]->wrecker->collisionModel->AddIgnoredOwner(racerSlots[b]->bikeCollision);
                racerSlots[a]->wrecker->collisionModel->AddIgnoredOwner(racerSlots[b]->bikeAndRiderCollision);
                racerSlots[a]->wrecker->collisionModel->AddIgnoredOwner(racerSlots[b]->frontWheel);
                racerSlots[a]->wrecker->collisionModel->AddIgnoredOwner(racerSlots[b]->rearWheel);
                racerSlots[a]->wrecker->collisionModel->AddIgnoredOwner(
                    racerSlots[b]->wrecker->collisionModel);
            }
        }
    }
    if (riders)
        DebugFree(riders, __FILE__, 0x7f7);
    if (bikes)
        DebugFree(bikes, __FILE__, 0x7f8);
    if (names)
        DebugFree(names, __FILE__, 0x7f9);
    delete stream;
    return 1;
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
    FreeGateList();
    if (statusList != 0) {
        FreeStatusList(statusList);
        statusList = 0;
    }
    if (vcrFile != 0) {
        delete vcrFile;
        vcrFile = 0;
        if (g_TrackGame->playingReplay) {
            RaceCameraView()->cameraState = savedCameraState;
            raceCamera->SaveCameraState();
        }
        g_TrackGame->playingReplay = 0;
        g_TrackGame->useReplayTape = 0;
    }
    while (gateList != 0) {
        RaceGate* node = gateList;
        gateList = node->nextGate;
        DebugFree(node, __FILE__, 0x82e);
    }
}

// 0x0041d260
int BikeRace::SetPaused(int value) {
    GameObject::SetPaused(value);
    JoystickDevice* joystick = g_TrackGame->controlInterface->activeJoystick;
    if (joystick != 0 && g_TrackGame->mode.joystickOn) {
        joystick->PauseForceFeedback(value);
    }
    return 1;
}

// 0x0041d0d0
int BikeRace::AppendNode(int index, RaceGatePose* source,
                         RaceGate*** tail, float scale, int height) {
    RaceGate* node =
        (RaceGate*)DebugCalloc(1, sizeof(RaceGate), __FILE__, 0x840);
    if (node == 0) {
        return 0;
    }
    RaceGatePose* record = &source[index];
    node->center.x = record->center.x;
    node->center.y = record->center.y;
    node->center.z = record->center.z;
    node->direction.x = record->direction.x;
    node->direction.y = record->direction.y;
    node->direction.z = record->direction.z;
    node->width = scale;
    node->height = height;
    node->halfExtents.y = 0.0f;
    node->halfExtents.x = scale * node->direction.z * 0.5f;
    node->halfExtents.z = scale * node->direction.x * -0.5f;
    **tail = node;
    *tail = &node->nextGate;
    return 1;
}

// 0x0041d170
int BikeRace::BuildNodeList(RaceGateEvent* owner) {
    int count = owner->gateCount;
    RaceGatePose* sources = owner->gateSources;
    RaceGate** tail = &gateList;
    for (int i = 0; i < count; i++) {
        RaceGateObject* entry = owner->gateEntries[i];
        if (!AppendNode(i, sources, &tail, entry->width, entry->height)) {
            return 0;
        }
    }
    return 1;
}

// 0x0041d1e0
int BikeRace::UpdateTrackSeparation() {
    if (raceTrack != 0 && raceTrack->DistanceAlong(finishProbe.trackPos, startProbe.trackPos) > 0.0f) {
        startBeforeFinish = 0;
    } else {
        startBeforeFinish = 1;
    }
    return startBeforeFinish;
}

// 0x0041d2a0
void BikeRace::SetSceneTime(float time) {
    raceScene->RestartAllEntries(time);
}

static inline float MaxFloat(float a, float b) {
    return a > b ? a : b;
}

// 0x0041d2b0
int BikeRace::Tick(float frameTime) {
    if (!g_TrackGame->uiInteractionBlocked) {
        if (RaceCameraView()->followingRacer &&
            RaceCameraView()->followedBike->outOfRace != 0) {
            NextCameraTarget(0, 1, 0);
        }
        if (!g_TrackGame->paused) {
            if (g_TrackGame->mode.joystickOn) {
                localRacer->UpdateForceFeedback();
            }
            if (g_TrackGame->mode.raceSettings.raceMode == 4) {
                unsigned short lap = localRacer->lapCount;
                if (ghostLap != lap && (!startBeforeFinish || lap > 0)) {
                    RestartReplay();
                    ghostLap = localRacer->lapCount;
                    replayTime = 0;
                    replayFrames = 0;
                    replayLength = 0;
                    replayLengthFrames = 0;
                    recordTickTime = 0;
                    recordTickCount = 0;
                    recordTickRecordCount = 0;
                    localRacer->recordStateTimer = FLT_MAX / 2;
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
                        TextQueueMessage* message = new (__FILE__, 0x8bf) TextQueueMessage(text, 3.25f);
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
                            recordTickRecordCount = 1;
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
                                if (g_TrackGame->eventManager->FindRaceMode()->auralScape != 0) {
                                    g_TrackGame->eventManager->FindRaceMode()->auralScape->Enable();
                                }
                                g_TrackGame->childRoot->EnableByClassName("SoundGroup");
                            }
                            if (g_TrackGame->mode.particlesOn) {
                                g_TrackGame->childRoot->EnableByClassName("ParticleManager");
                                g_TrackGame->childRoot->EnableByClassName("DirtParticleEmitter");
                                g_TrackGame->childRoot->EnableByClassName("DustParticleEmitter");
                                g_TrackGame->childRoot->EnableByClassName("DirtChunkParticleEmitter");
                                g_TrackGame->childRoot->EnableByClassName("SteamParticleEmitter");
                            }
                            if (g_TrackGame->mode.shadowsOn) {
                                g_TrackGame->childRoot->EnableByClassName("ProjectedShadow");
                                g_TrackGame->childRoot->EnableByClassName("TerrainShadow");
                                g_TrackGame->childRoot->EnableByClassName("D3DIMSoultreeShadow");
                            }
                        }
                    } else if (appliedReplayMode != replayMode) {
                        appliedReplayMode = replayMode;
                        if (g_TrackGame->eventManager->FindRaceMode()->auralScape != 0) {
                            g_TrackGame->eventManager->FindRaceMode()->auralScape->Disable();
                        }
                        g_TrackGame->childRoot->DisableByClassName("SoundGroup");
                        g_TrackGame->childRoot->DisableByClassName("ParticleManager");
                        g_TrackGame->childRoot->DisableByClassName("DirtParticleEmitter");
                        g_TrackGame->childRoot->DisableByClassName("DustParticleEmitter");
                        g_TrackGame->childRoot->DisableByClassName("DirtChunkParticleEmitter");
                        g_TrackGame->childRoot->DisableByClassName("SteamParticleEmitter");
                        g_TrackGame->childRoot->DisableByClassName("ProjectedShadow");
                        g_TrackGame->childRoot->DisableByClassName("TerrainShadow");
                        g_TrackGame->childRoot->DisableByClassName("D3DIMSoultreeShadow");
                    }
                }
                if (!g_TrackGame->playingReplay && isRacing) {
                    if (replayLength > recordTimeLimit) {
                        replayLength = recordTimeLimit;
                        TextQueueOverlay* overlay = g_TrackGame->eventManager->FindTextQueue();
                        char text[0x100];
                        g_TrackGame->LoadResourceString(0x14dd, text, 0x80);
                        TextQueueMessage* message = new (__FILE__, 0x912) TextQueueMessage(text, 3.25f);
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
                                recordTickRecordCount = CountRaceRacers() + 1;
                            } else {
                                recordTickRecordCount = CountRaceRacers();
                            }
                            int camera = g_TrackGame->mode.raceSettings.eventType;
                            if (camera == 0 || (camera == 4 && g_TrackGame->mode.raceSettings.stuntMode)) {
                                if (recordTickCount % 20 == 0) {
                                    scoreRecordDue = true;
                                    recordTickRecordCount *= 2;
                                } else {
                                    scoreRecordDue = false;
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
                    TimeStampMessage stamp;
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
                            racerSlots[i]->Enable();
                            for (int j = 0; j < g_TrackGame->networkAiRacerCount + g_TrackGame->playerCount; j++) {
                                if (racerSlots[j]->recorderId == id) {
                                    racerSlots[j]->Enable();
                                }
                            }
                        }
                        if (player == 0) {
                            racerSlots[i]->Disable();
                            for (int j = 0; j < g_TrackGame->networkAiRacerCount + g_TrackGame->playerCount; j++) {
                                if (racerSlots[j]->recorderId == id) {
                                    racerSlots[j]->Disable();
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
                            racerSlots[k]->SetSpawnProtection(1, racerSlots[k]->GetSpawnProtectionDuration(), 0);
                        }
                    }
                }
            } else {
                Vector3 position;
                if (isRacing) {
                    if (startGate != 0 && startGate->isEnabled &&
                        startGateTimer > 0.0f) {
                        startGateTimer -= g_TrackGame->frameTime;
                        if (startGateTimer <= 0.0f) {
                            startGateTimer = 0;
                            raceScene->RestartEntry(startGateEntry, 0.0f, 0, 0);
                            startGate->Disable();
                            if (!RaceCameraView()->followingRacer && targetEntry == startGateEntry) {
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
                        raceScene->holdAnimations = 0;
                        if (startGate != 0) {
                            raceScene->EnableAutomaticSequence(startGateEntry);
                            startGateTimer = 5.0f;
                        }
                        if (g_TrackGame->playerCount > 1) {
                            int self = g_TrackGame->network->localPlayer;
                            int count = g_TrackGame->mode.raceSettings.multiplayerAiRacerCount + g_TrackGame->playerCount;
                            for (int i = 0; i < count; i++) {
                                if (g_TrackGame->mode.racers[i].networkPlayerId == self) {
                                    racerSlots[i]->velocity = kVec3Zero;
                                    racerSlots[i]->linearSpeed = 0;
                                }
                            }
                        }
                        replayTime = 0;
                        replayFrames = 0;
                        replayLength = 0;
                        replayLengthFrames = 0;
                        if (thirtySecondMan != 0) {
                            raceScene->RestartEntry(thirtySecondManEntry, 0.0f, 0, 0);
                            thirtySecondMan->Disable();
                            if (!RaceCameraView()->followingRacer && targetEntry == thirtySecondManEntry) {
                                NextCameraTarget(1, 1, 0);
                            }
                            if (thirtySecondMan->collisionObject != 0) {
                                thirtySecondMan->collisionObject->Release();
                                thirtySecondMan->collisionObject = 0;
                            }
                        }
                        switch (g_TrackGame->mode.raceSettings.eventType) {
                        case 2:
                            chatOverlay = (ChatOverlay*)g_TrackGame->nationalEvent->chatOverlay;
                            break;
                        case 3:
                            chatOverlay = (ChatOverlay*)g_TrackGame->supercrossEvent->chatOverlay;
                            break;
                        case 0:
                            chatOverlay = (ChatOverlay*)g_TrackGame->quarryEvent->chatOverlay;
                            break;
                        case 1:
                        case 5:
                            chatOverlay = (ChatOverlay*)g_TrackGame->bajaEnduroEvent->chatOverlay;
                            break;
                        case 4:
                            chatOverlay = (ChatOverlay*)g_TrackGame->tagEvent->chatOverlay;
                            break;
                        }
                    }
                }
                GameObject::Tick(frameTime);
                int camera = g_TrackGame->mode.raceSettings.eventType;
                if (camera == 3 || camera == 2 || camera == 5 || camera == 1 || camera == 0) {
                    Track* track = raceTrack;
                    if ((track != 0 || gateList != 0 || camera == 0) && isRacing && !restartPending &&
                        localRacer != 0) {
                        int keepRacing = g_TrackGame->mode.raceSettings.raceMode == 0 ||
                                         g_TrackGame->mode.raceSettings.raceMode == 4;
                        if (!g_TrackGame->playingReplay) {
                            if (camera == 1 || camera == 5) {
                                UpdateGateRace(&statusList, gateList, g_TrackGame->frameTime,
                                                      g_TrackGame->mode.raceSettings.lapCount, keepRacing);
                            } else if (camera == 0) {
                                RankByScore(keepRacing);
                            } else {
                                UpdateLapRace(&statusList, track, g_TrackGame->frameTime, &startProbe,
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
                if (youIndicator != 0 && youIndicator->isEnabled) {
                    position = localRacer->centerOfMass;
                    position.y += 4.25f;
                    ((ArcadeObject*)youIndicator)->SetPosition(&position);
                }
                if (chatHideTimer > 0.0f && g_TrackGame->mode.chatInputLine != 1) {
                    chatHideTimer -= g_TrackGame->frameTime;
                    if (chatHideTimer <= 0.0f) {
                        chatHideTimer = 0;
                        if (chatOverlay != 0) {
                            chatOverlay->typingEnabled = 0;
                            chatOverlay->HideInputLine();
                        }
                        chatOpen = false;
                    }
                }
                if (flagGirl != 0 && localRacer->statusNode->progressPos.node != 0) {
                    int laps = g_TrackGame->mode.raceSettings.lapCount;
                    int lap = localRacer->lapCount;
                    int last = lap == laps - 1;
                    int previous = lap == laps - 2;
                    if (previous || last) {
                        float toFinish = raceTrack->DistanceAlong(localRacer->statusNode->progressPos,
                                                                            finishProbe.trackPos);
                        float fromFinish = raceTrack->DistanceAlong(finishProbe.trackPos,
                                                                              localRacer->statusNode->progressPos);
                        if (flagGirlState && localRacer->finished && fromFinish > 100.0f) {
                            ReplaceMeshId(flagGirl, 1, 2);
                            flagGirl->SetMotionByName("Stand");
                            flagGirlState = 0;
                        } else if ((previous && toFinish < 300.0f) || (last && fromFinish < 100.0f)) {
                            if (flagGirlState != 1) {
                                ReplaceMeshId(flagGirl, 1, 2);
                                flagGirlState = 1;
                                flagGirl->SetMotionByName("FlagStart");
                            } else if (flagGirl->motionStopped) {
                                flagGirl->SetMotionByName("FlagLoop");
                            }
                        } else if (flagGirlState && flagGirlState < 3 &&
                                   ((previous && toFinish > 100.0f) || (last && fromFinish > 100.0f))) {
                            flagGirl->SetMotionByName("Stand");
                            flagGirlState = 0;
                        } else if ((last && toFinish < 300.0f) || (localRacer->finished && fromFinish < 100.0f)) {
                            if (flagGirlState < 3) {
                                ReplaceMeshId(flagGirl, 2, 1);
                                flagGirlState = 3;
                                flagGirl->SetMotionByName("FlagStart");
                            } else if (flagGirl->motionStopped) {
                                flagGirl->SetMotionByName("FlagLoop");
                                flagGirlState = 4;
                            }
                        }
                    }
                    flagGirl->AdvanceMotionInterpolated(frameTime, 0, 0);
                }
            }
        } else if (replayVcr != 0) {
            replayVcr->Tick(frameTime);
        }
    }
    if (g_TrackGame->testKey && g_TrackGame->debugOverlay != 0) {
        if (lodDebugPage < 0) {
            lodDebugPage = g_TrackGame->debugOverlay->NewPage();
        }
        if (g_TrackGame->debugOverlay->shownPage == lodDebugPage) {
            char name[0x20];
            D3DIMSoultreeObject* model;
            int pass = 0;
            if ((signed char)targetKind < 0) {
                strncpy(name,
                        ((BikeRaceManufacturerRecord*)g_TrackGame->ui->manufacturers)
                            [((BikeRaceBikeRecord*)g_TrackGame->ui->bikes)
                                 [g_TrackGame->mode.racerChoice.bikeIndex].manufacturer].modelFile,
                        0x14);
                name[0x14] = 0;
                raceScene->ResolveSltPath(name);
                model = localRacer->modelNode;
            } else if (targetKind == 0) {
                strncpy(name, raceScene->staticModels->casters[targetIndex].fileName, 0x14);
                name[0x14] = 0;
                model = (D3DIMSoultreeObject*)raceScene->staticModels->casters[targetIndex].model;
            } else {
                strncpy(name, raceScene->animationTable->entries[targetIndex].fileName, 0x14);
                name[0x14] = 0;
                SceneAnimation* entry = &raceScene->animationTable->entries[targetIndex];
                if (entry->keyFramed) {
                    model = ((BikeRaceRiderView*)entry->character)->modelNode;
                } else {
                    model = entry->proceduralObject->carModel;
                }
            }
            do {
                int faces = 0;
                SoultreeLod* group = &model->lodTable[model->currentLod];
                for (int i = 0; i < group->surfaceCount; i++) {
                    faces += group->surfaces[i].faceCount;
                }
                g_TrackGame->debugOverlay->Title(lodDebugPage, "Soultree LOD (%s)", name);
                g_TrackGame->debugOverlay->Print(lodDebugPage, "Current LOD %d out of %d",
                                                                        model->currentLod + 1, model->lodCount);
                g_TrackGame->debugOverlay->Print(lodDebugPage, "Number of Faces = %d", faces);
                if ((signed char)targetKind < 0) {
                    if (g_LodDebugSelectedRow > 5) {
                        if ((pass == 1 && g_LodDebugSelectedRow - 7 > model->lodCount) || g_LodDebugSelectedRow < 2) {
                            g_TrackGame->debugOverlay->SetRowHighlight(lodDebugPage, g_LodDebugSelectedRow + 1, 0);
                            g_LodDebugSelectedRow = 2;
                        }
                    } else if ((pass == 0 && g_LodDebugSelectedRow > model->lodCount) || g_LodDebugSelectedRow < 2) {
                        g_TrackGame->debugOverlay->SetRowHighlight(lodDebugPage, g_LodDebugSelectedRow + 1, 0);
                        g_LodDebugSelectedRow = 9;
                    }
                } else if (g_LodDebugSelectedRow > model->lodCount || g_LodDebugSelectedRow < 2) {
                    g_TrackGame->debugOverlay->SetRowHighlight(lodDebugPage, g_LodDebugSelectedRow + 1, 0);
                    g_LodDebugSelectedRow = 2;
                }
                if (model->lodCount > 1) {
                    int up = g_TrackGame->controlInterface->IsDeviceButtonDown(0xd0, 0, 0x80, 0);
                    int down = g_TrackGame->controlInterface->IsDeviceButtonDown(0xc8, 0, 0x80, 0);
                    int row = g_LodDebugSelectedRow - 1;
                    if (down || up) {
                        if ((signed char)targetKind < 0) {
                            if (g_LodDebugSelectedRow > 5) {
                                if (pass == 1) {
                                    row = g_LodDebugSelectedRow - 8;
                                } else {
                                    row = 99;
                                }
                            } else if (pass == 1) {
                                row = 99;
                            }
                        }
                        switch (row) {
                        case 1:
                            if (model->lodCount > 1) {
                                if (down) {
                                    model->autoLodDistances[0] += 0.05f;
                                } else {
                                    model->autoLodDistances[0] -= 0.05f;
                                    model->autoLodDistances[0] = MaxFloat(model->autoLodDistances[0], 0.0f);
                                }
                            }
                            break;
                        case 2:
                            if (model->lodCount > 2) {
                                if (down) {
                                    model->autoLodDistances[1] += 0.05f;
                                } else {
                                    model->autoLodDistances[1] -= 0.05f;
                                    model->autoLodDistances[1] = MaxFloat(model->autoLodDistances[1], 0.0f);
                                }
                            }
                            break;
                        case 3:
                            if (model->lodCount > 3) {
                                if (down) {
                                    model->autoLodDistances[2] += 0.05f;
                                } else {
                                    model->autoLodDistances[2] -= 0.05f;
                                    model->autoLodDistances[2] = MaxFloat(model->autoLodDistances[2], 0.0f);
                                }
                            }
                            break;
                        case 4:
                            if (model->lodCount > 4) {
                                if (down) {
                                    model->autoLodDistances[3] += 0.05f;
                                } else {
                                    model->autoLodDistances[3] -= 0.05f;
                                    model->autoLodDistances[3] = MaxFloat(model->autoLodDistances[3], 0.0f);
                                }
                            }
                            break;
                        }
                    }
                    g_TrackGame->debugOverlay->Print(
                        lodDebugPage, "AutoLOD#0 %.2f", model->lodCount > 1 ? model->autoLodDistances[0] : 0.0f);
                    g_TrackGame->debugOverlay->Print(
                        lodDebugPage, "AutoLOD#1 %.2f", model->lodCount > 2 ? model->autoLodDistances[1] : 0.0f);
                    g_TrackGame->debugOverlay->Print(
                        lodDebugPage, "AutoLOD#2 %.2f", model->lodCount > 3 ? model->autoLodDistances[2] : 0.0f);
                    g_TrackGame->debugOverlay->Print(
                        lodDebugPage, "AutoLOD#3 %.2f", model->lodCount > 4 ? model->autoLodDistances[3] : 0.0f);
                    g_TrackGame->debugOverlay->SetRowHighlight(lodDebugPage, g_LodDebugSelectedRow + 1, 1);
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
            if (reinterpret_cast<KrustyBike*>(racerSlots[i])->ReceivedPositionChanged()) {
                return 1;
            }
        }
    }
    return 0;
}

// 0x0041ea60
void BikeRace::ReplaceMeshId(D3DIMSoultreeCharacter* character, int from, int to) {
    for (int i = 0; i < reinterpret_cast<D3DIMSoultreeObject*>(character->modelNode)->lodCount; i++) {
        for (int j = 0; j < reinterpret_cast<D3DIMSoultreeObject*>(character->modelNode)->lodTable[i].surfaceCount; j++) {
            for (int k = 0; k < reinterpret_cast<D3DIMSoultreeObject*>(character->modelNode)->lodTable[i].surfaces[j].materialCount; k++) {
                if (reinterpret_cast<D3DIMSoultreeObject*>(character->modelNode)->lodTable[i].surfaces[j].materialIndices[k] == from) {
                    reinterpret_cast<D3DIMSoultreeObject*>(character->modelNode)->lodTable[i].surfaces[j].materialIndices[k] = to;
                }
            }
        }
    }
}

// 0x0041eb20
int BikeRace::AimCamera(int caster) {
    if (RaceCameraView()->followingRacer) {
        targetKind = 0xff;
        targetIndex = 0;
        if (g_TrackGame->playerCount > 1) {
            RaceCameraView()->followedBike->modelNode->ClearFlag14c();
            RaceCameraView()->followedBike->riderCharacter->modelNode->ClearFlag14c();
            RaceCameraView()->StartFollowing(racerSlots[targetRacer]);
            if (raceShadow != 0) {
                raceShadow->ClearCasters();
                raceShadow->AddCaster(racerSlots[targetRacer]->modelNode);
                raceShadow->AddCaster(racerSlots[targetRacer]->riderCharacter->modelNode);
            }
            if (g_TrackGame->bajaEnduroEvent != 0) {
                BikeRaceKrustyBikeView* racer = racerSlots[targetRacer];
                g_TrackGame->bajaEnduroEvent->SelectLapGates(racer->lapGate,
                                                                          racer->nextLapGate, racer);
            }
        } else if (targetRacer == 0) {
            RaceCameraView()->followedBike->modelNode->ClearFlag14c();
            RaceCameraView()->followedBike->riderCharacter->modelNode->ClearFlag14c();
            RaceCameraView()->StartFollowing(localRacer);
            if (raceShadow != 0) {
                raceShadow->ClearCasters();
                raceShadow->AddCaster(localRacer->modelNode);
                raceShadow->AddCaster(localRacer->riderCharacter->modelNode);
            }
            if (g_TrackGame->bajaEnduroEvent != 0) {
                BikeRaceKrustyBikeView* racer = localRacer;
                g_TrackGame->bajaEnduroEvent->SelectLapGates(racer->lapGate,
                                                                          racer->nextLapGate, racer);
            }
        } else {
            RaceCameraView()->followedBike->modelNode->ClearFlag14c();
            RaceCameraView()->followedBike->riderCharacter->modelNode->ClearFlag14c();
            int index = targetRacer - 1;
            RaceCameraView()->StartFollowing(aiRacers[index]);
            if (raceShadow != 0) {
                raceShadow->ClearCasters();
                raceShadow->AddCaster(aiRacers[index]->modelNode);
                raceShadow->AddCaster(aiRacers[index]->riderCharacter->modelNode);
            }
            if (g_TrackGame->bajaEnduroEvent != 0) {
                BikeRaceKrustyBikeView* racer = aiRacers[index];
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
            RaceCameraView()->TargetShadowCaster(
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
            SceneAnimation* entry = &raceScene->animationTable->entries[targetEntry];
            if (entry->keyFramed) {
                CollisionCharacter* character = entry->character;
                if (character->isEnabled) {
                    RaceCameraView()->TargetSceneCharacter(character);
                } else {
                    return 0;
                }
            } else {
                CarProcedural* object = entry->proceduralObject;
                if (object->isEnabled) {
                    RaceCameraView()->TargetSceneObject(object);
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
inline BikeRaceKrustyBikeView* BikeRace::RacerAtIndex(int index) {
    if (g_TrackGame->playerCount == 1 && index == 0) {
        return localRacer;
    }
    if (g_TrackGame->playerCount > 1 || index < g_TrackGame->playerCount) {
        return racerSlots[index];
    }
    return aiRacers[index - g_TrackGame->playerCount];
}

// 0x0041f1d0. Not exact (887 bytes against 881, 313 strict, linked 0.829):
// retail keeps 0 in edx from the first instruction (re-zeroed after calls)
// and has three copies of the AimCamera tail call; the branches and calls
// are retail's (docs/BIKERACE.md).
void BikeRace::NextCameraTarget(int forward, int racers, int objects) {
    int caster;
    if (g_TrackGame->uiInteractionBlocked) {
        return;
    }
    BikeRaceKrustyBikeCameraView* camera = RaceCameraView();
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
        } while (RacerAtIndex(targetRacer)->outOfRace != 0);
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
            SceneAnimation* entry = &scene->animationTable->entries[targetEntry];
            if (entry->keyFramed) {
                if (entry->character->isEnabled) {
                    hidden = entry->excludeFromCameraIteration;
                } else {
                    hidden = 1;
                }
            } else if (entry->proceduralObject->isEnabled) {
                hidden = entry->excludeFromCameraIteration;
            } else {
                hidden = 1;
            }
            if (old == targetEntry) {
                BikeRaceKrustyBikeCameraView* view = RaceCameraView();
                if (view->targetCharacter != 0 || view->targetCar != 0) {
                    return;
                }
                entry = &scene->animationTable->entries[targetEntry];
                if (entry->keyFramed) {
                    if (!entry->character->isEnabled) {
                        view->followingRacer = 1;
                        return;
                    }
                } else if (!entry->proceduralObject->isEnabled) {
                    view->followingRacer = 1;
                    return;
                }
                AimCamera(caster);
                return;
            }
        } while (hidden);
    } else {
        caster = 1;
        SceneStaticModels* casters = raceScene->staticModels;
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
int BikeRace::OnChar(int character) {
    if (chatOpen && chatOverlay != 0 && chatOverlay->HandleChatKey(character, &character)) {
        if (character == 0) {
            chatHideTimer = 10.0f;
        }
        return 1;
    }
    return 0;
}

// 0x0041f5e0
int BikeRace::OnControlPress(ControlEvent* event, InputButtonState* entry) {
    int result;
    if (g_TrackGame->network != 0 && event->kind == 0) {
        g_TrackGame->controlInterface->keyboard->ModifierHolds(0xc);
    }
    if (chatOpen && chatOverlay != 0) {
        if (event->control == 1 && event->kind == 0) {
            chatHideTimer = 0.1f;
            chatOverlay->typingEnabled = 0;
            chatOpen = false;
            return 1;
        }
        if (chatOverlay->HandleChatInput(event, entry, &result)) {
            return 1;
        }
    }
    if (GameObject::OnControlPress(event, entry)) {
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
                if (!keyboard->IsButtonDown(0x2a, 0x3f, 0) &&
                    !g_TrackGame->controlInterface->keyboard->IsButtonDown(0x36, 0x3f, 0)) {
                    if (g_TrackGame->mode.chatInputLine == 2) {
                        break;
                    }
                    if (chatOverlay != 0) {
                        chatOverlay->ShowInputLine();
                        chatOverlay->typingEnabled = 1;
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
                    chatOverlay->typingEnabled = 1;
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
                TextQueueMessage* message = new (__FILE__, 0xcdd) TextQueueMessage(text, 3.25f);
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
                TextQueueMessage message(text, 1.5f);
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
                TextQueueMessage message(text, 1.5f);
                overlay->ReplaceHeadMessage(&message);
                return 1;
            }
            break;
        case 0x1f:
            if (TestInputEvent(0x1f, 0, event, 0x80)) {
                debugDraw = 1 - debugDraw;
            }
            break;
        }
    }
    if (g_TrackGame->testKey && g_TrackGame->debugOverlay != 0) {
        if (lodDebugPage < 0) {
            lodDebugPage = g_TrackGame->debugOverlay->NewPage();
        }
        int shown = g_TrackGame->debugOverlay->shownPage;
        if (shown == lodDebugPage) {

            if (TestInputEvent(0x1c, 0, event, 0x80)) {
                if (g_TrackGame->ui->gui->GetUser(0)->userCursor == 0) {
                    g_TrackGame->ui->gui->CreateCursors("ui\\cursor.tga", 0);
                }
                if (!g_TrackGame->ui->gui->GetUser(0)->userCursor->isEnabled) {
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
                        lodDebugPage, g_LodDebugSelectedRow + 1, 0);
                    g_LodDebugSelectedRow++;
                    if ((signed char)targetKind < 0) {
                        if (g_LodDebugSelectedRow > 12) {
                            g_LodDebugSelectedRow = 2;
                        } else if (g_LodDebugSelectedRow > 5 && g_LodDebugSelectedRow < 9) {
                            g_LodDebugSelectedRow = 9;
                        }
                    } else if (g_LodDebugSelectedRow > 5) {
                        g_LodDebugSelectedRow = 2;
                    }
                }
            }
            if (TestInputEvent(0, 1, event, 0x80000000)) {
                int picked = objectPicker->PickAtCursor();
                if (picked != 0) {
                    if (raceScene->staticModels != 0) {
                        for (int i = 0; i < raceScene->staticModels->casterCount; i++) {
                            PickSceneStaticModelView* caster =
                                &((PickSceneStaticModelView*)raceScene->staticModels->casters)[i];
                            if (caster->flags & 1) {
                                if (picked == caster->physicsObject->collisionObject) {
                                    RaceCameraView()->followingRacer = 1;
                                    targetCaster = i;
                                    NextCameraTarget(1, 0, 0);
                                    return 1;
                                }
                            } else if (picked == caster->collisionObject) {
                                RaceCameraView()->followingRacer = 1;
                                targetCaster = i;
                                NextCameraTarget(1, 0, 0);
                                return 1;
                            }
                        }
                    }
                    if (raceScene->animationTable != 0) {
                        for (int i = 0; i < raceScene->animationTable->entryCount; i++) {
                            SceneAnimation* entry = &raceScene->animationTable->entries[i];
                            if (entry->keyFramed) {
                                if (picked == ((PickCollisionCharacterView*)entry->character)->collisionObject) {
                                    RaceCameraView()->followingRacer = 1;
                                    targetEntry = i;
                                    NextCameraTarget(1, 0, 1);
                                    return 1;
                                }
                            } else if (picked == ((PickProceduralObjectView*)entry->proceduralObject)->collisionObject) {
                                RaceCameraView()->followingRacer = 1;
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
int BikeRace::OnNetworkMessage(int type, void* data, int from, int to, int flags) {
    int count;
    if (GameObject::OnNetworkMessage(type, data, from, to, flags)) {
        return 1;
    }
    if (g_TrackGame->uiInteractionBlocked) {
        return 1;
    }
    if (from == 0) {
        return 0;
    }
    if (type == 1) {
        RacerStateMessage* state = (RacerStateMessage*)data;
        int self = g_TrackGame->network->localPlayer;
        count = g_TrackGame->mode.racerCount;
        for (int i = 0; i < count; i++) {
            int id = g_TrackGame->mode.racers[i].networkPlayerId;
            if (id != self && id == from) {
                BikeRaceKrustyBikeView* racer = racerSlots[i];
                if (racer != 0 && racer->netIndex == state->racerIndex) {
                    racerSlots[i]->hasFullState = 1;
                    ReceivedRacerState* oldest = racerSlots[i]->receivedStates[2];
                    racerSlots[i]->receivedStates[2] = racerSlots[i]->receivedStates[1];
                    racerSlots[i]->receivedStates[1] = racerSlots[i]->receivedStates[0];
                    racerSlots[i]->receivedStates[0] = oldest;
                    racerSlots[i]->receivedStates[0]->messageFlags = flags;
                    racerSlots[i]->receivedStates[0]->state = *state;
                    racerSlots[i]->receivedStates[0]->fresh = 1;
                    racerSlots[i]->deltaBaseVelocity = state->velocity;
                    racerSlots[i]->deltaBasePosition = state->position;
                    racerSlots[i]->deltaBaseAngularVelocity = state->angularVelocity;
                    racerSlots[i]->deltaBaseRoll = state->roll;
                    racerSlots[i]->deltaBasePitch = state->pitch;
                    racerSlots[i]->deltaBaseYaw = state->yaw;
                    racerSlots[i]->deltaBaseTime = replayMode == 11 ? flags : state->time;
                    return 1;
                }
            }
        }
    } else if (type == 13) {
        RacerDeltaMessage* message = (RacerDeltaMessage*)data;
        int self = g_TrackGame->network->localPlayer;
        for (int i = 0; i < g_TrackGame->mode.racerCount; i++) {
            int id = g_TrackGame->mode.racers[i].networkPlayerId;
            if (id != self && id == from && racerSlots[i] != 0 &&
                racerSlots[i]->netIndex == message->racerIndex) {
                if (!racerSlots[i]->hasFullState) {
                    return 1;
                }
                ReceivedRacerState* oldest = racerSlots[i]->receivedStates[2];
                racerSlots[i]->receivedStates[2] = racerSlots[i]->receivedStates[1];
                racerSlots[i]->receivedStates[1] = racerSlots[i]->receivedStates[0];
                racerSlots[i]->receivedStates[0] = oldest;
                racerSlots[i]->receivedStates[0]->messageFlags = flags;
                reinterpret_cast<KrustyBike*>(racerSlots[i])->ApplyStateDelta(
                    reinterpret_cast<const KbNetDelta*>(message),
                    reinterpret_cast<KbNetState*>(racerSlots[i]->receivedStates[0]));
                racerSlots[i]->receivedStates[0]->fresh = 0;
                return 1;
            }
        }
    } else if (type == 10) {
        RacerScoreMessage* score = (RacerScoreMessage*)data;
        count = g_TrackGame->mode.racerCount;
        int self = g_TrackGame->network->localPlayer;
        for (int i = 0; i < count; i++) {
            int id = g_TrackGame->mode.racers[i].networkPlayerId;
            if (id != self && id == from) {
                BikeRaceKrustyBikeView* racer = racerSlots[i];
                if (racer != 0 && racer->netIndex == score->racerIndex) {
                    racerSlots[i]->score = score->racerScore;
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
                chatOverlay->AddMessage(name, (int)&((TimeStampMessage*)data)->timeStamp, 1);
            }
        }
        return 1;
    } else if (type == 0x84) {
        TimeStampMessage* stamp = (TimeStampMessage*)data;
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
BikeRaceKrustyBikeView* BikeRace::NextBike(int* iterator) {
    BikeRaceKrustyBikeView* racer = 0;
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
            racerSlots[i]->RemoveFromRace();
            TrackGameViewOwner* owner = g_TrackGame->tagEvent;
            if (owner != 0 && owner->followedRacer == (EventKrustyBikeView*)racerSlots[i]) {
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
        startGate->Enable();
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
        raceSound->engineGroup->SetPaused(0);
        if (raceSound->field_0x40 != 0) {
            raceSound->field_0x40->SetPaused(0);
        }
        raceSound->effectsGroup->SetPaused(0);
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
                g_TrackGame->ui->gui->ShowDialog((UIDialog*)new (__FILE__, 0x1073) VCRDlg(1, "VCR.dtm"), 0, 1, 0, 0, 0, 0, 1);
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
        flagGirlState = 0;
    }
    SetSceneTime(0.0001f);
    raceScene->holdAnimations = 1;
    for (BikeRaceKrustyBikeView* racer = NextBike(&iterator); racer != 0;
         racer = NextBike(&iterator)) {
        racer->SnapshotRespawnPose();
        racer->restarted = true;
        WriteRelativePositions(&position, &direction, g_TrackGame->bajaEnduroEvent,
                              positions[iterator - 1]);
        racer->startPosition = position;
        racer->startDirection = direction;
        reinterpret_cast<KrustyBike*>(racer)->SetNetRecorder(reinterpret_cast<KbRecorder*>(replayVcr));
        replayPaused = false;
    }
    if (g_TrackGame->tagEvent != 0 &&
        g_TrackGame->tagEvent->ball != 0) {
        g_TrackGame->tagEvent->ball->SetRecorder(replayVcr);
    }
    if (g_TrackGame->quarryEvent != 0) {
        reinterpret_cast<BaseQuarryEvent*>(g_TrackGame->quarryEvent)->ResetClock();
    }
    if (g_TrackGame->tagEvent != 0) {
        reinterpret_cast<BaseQuarryEvent*>(g_TrackGame->tagEvent)->ResetClock();
    }
    if (g_TrackGame->bajaEnduroEvent != 0) {
        g_TrackGame->bajaEnduroEvent->SelectLapGates(0, 1, RaceCameraView()->followedRacer);
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
    if (localRacer->lapCount == 1 && startBeforeFinish) {
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
                                       (unsigned char)aiRacers[0]->fourStroke,
                                       g_TrackGame->mode.plateNumber);
    AppendChild(replayVcr, -1);
    reinterpret_cast<KrustyBike*>(localRacer)->SetNetRecorder(reinterpret_cast<KbRecorder*>(replayVcr));
    if (startBeforeFinish && localRacer->lapCount == 1 && ghostFilePlayed[0] == 0) {
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
    reinterpret_cast<KrustyBike*>(aiRacers[0])->SetNetRecorder(reinterpret_cast<KbRecorder*>(ghostVcr));
    aiRacers[0]->Enable();
    aiRacers[0]->hasFullState = 0;
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
            thirtySecondMan->Enable();
            raceScene->RestartEntry(thirtySecondManEntry, 0.0f, 0, 0);
            if (thirtySecondMan->collisionObject != 0) {
                thirtySecondMan->collisionObject->Release();
                thirtySecondMan->collisionObject = 0;
            }
            raceScene->EnableAutomaticSequence(thirtySecondManEntry);
        } else {
            startCountdown = 5.2f;
        }
    }
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

// 0x004210f0. Not exact (2960 bytes against 2981, 466 strict, linked
// 0.981): the call sequence is retail's (17 out-of-line Vector3
// constructors, six scales, four dots and inverse square roots); register
// choice and scheduling differ (docs/BIKERACE.md).
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

// Inline: TrackGame+0x568's tag ball, reloaded at each use.
static inline TagBall* Ball() {
    return (TagBall*)g_TrackGame->tagEvent->ball;
}

// 0x00421d50
int BikeRace::RecordFrame(int kind, void* data, int flag, int recorderId, int* keep, int time) {
    *keep = 0;
    lastRecordKind = kind;
    if (replayPaused && kind == -1) {
        replayTime = ((VcrTickRecord*)data)->time;
        replayStepFrame = false;
        return 5;
    }
    if (g_TrackGame->uiInteractionBlocked) {
        return 0;
    }
    if (kind == -2) {
        ClearRacerParts();
        g_TrackGame->replaySpeed = 1.0f;
        if (g_TrackGame->ui->playingDemo) {
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
    } else if (kind == -3) {
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
    } else if (kind == -1) {
        VcrTickRecord* tick = (VcrTickRecord*)data;
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
    } else if (kind == 6) {
        if (replayPaused) {
            ToggleReplayPause(0);
        }
        return replayMode == 11 ? 11 : 1;
    } else if (kind == 1) {
        RacerStateMessage* state = (RacerStateMessage*)data;
        BikeRaceKrustyBikeView* racer = localRacer;
        if (recorderId == racer->recorderId && racer->netIndex == state->racerIndex) {
            if (g_TrackGame->mode.raceSettings.raceMode == 4) {
                racer = aiRacers[0];
            }
            if (racer == 0) {
                return 0;
            }
            racer->hasFullState = 1;
            ReceivedRacerState* oldest;
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
            racer->receivedStates[0]->state.time = (int)(s_UnknownStatic578e8c * 1000.0f);
            racer->receivedStates[0]->fresh = 1;
            racer->deltaBaseVelocity = state->velocity;
            racer->deltaBasePosition = state->position;
            racer->deltaBaseAngularVelocity = state->angularVelocity;
            racer->deltaBaseRoll = state->roll;
            racer->deltaBasePitch = state->pitch;
            racer->deltaBaseYaw = state->yaw;
            racer->deltaBaseTime = replayMode == 11 ? time : state->time;
            if (ghostVcr == 0) {
                racer->fastestLap = state->fastestLap;
                racer->timeBehindLeader = state->timeBehindLeader;
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
            if (aiRacers[i] != 0 && recorderId == aiRacers[i]->recorderId &&
                aiRacers[i]->netIndex == state->racerIndex) {
                aiRacers[i]->hasFullState = 1;
                ReceivedRacerState* oldest;
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
                aiRacers[i]->receivedStates[0]->state.time = (int)(s_UnknownStatic578e8c * 1000.0f);
                aiRacers[i]->receivedStates[0]->fresh = 1;
                aiRacers[i]->deltaBaseVelocity = state->velocity;
                aiRacers[i]->deltaBasePosition = state->position;
                aiRacers[i]->deltaBaseAngularVelocity = state->angularVelocity;
                aiRacers[i]->deltaBaseRoll = state->roll;
                aiRacers[i]->deltaBasePitch = state->pitch;
                aiRacers[i]->deltaBaseYaw = state->yaw;
                aiRacers[i]->deltaBaseTime = replayMode == 11 ? time : state->time;
                if (ghostVcr == 0) {
                    aiRacers[i]->fastestLap = state->fastestLap;
                    aiRacers[i]->timeBehindLeader = state->timeBehindLeader;
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
    } else if (kind == 13) {
        RacerDeltaMessage* message = (RacerDeltaMessage*)data;
        if (flag) {
            return 1;
        }
        BikeRaceKrustyBikeView* racer = localRacer;
        if (recorderId == racer->recorderId && racer->netIndex == message->racerIndex) {
            if (g_TrackGame->mode.raceSettings.raceMode == 4) {
                racer = aiRacers[0];
            }
            if (racer == 0 || !racer->hasFullState) {
                return 0;
            }
            ReceivedRacerState* oldest;
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
            reinterpret_cast<KrustyBike*>(racer)->ApplyStateDelta(
                    reinterpret_cast<const KbNetDelta*>(message),
                    reinterpret_cast<KbNetState*>(racer->receivedStates[0]));
            racer->receivedStates[0]->state.time = (int)(s_UnknownStatic578e8c * 1000.0f);
            racer->receivedStates[0]->fresh = 0;
            return 1;
        }
        int i = 0;
        if (g_TrackGame->mode.raceSettings.raceMode == 4) {
            return 1;
        }
        for (; i < g_TrackGame->mode.raceSettings.aiRacerCount; i++) {
            if (aiRacers[i] != 0 && recorderId == aiRacers[i]->recorderId &&
                aiRacers[i]->netIndex == message->racerIndex) {
                if (!aiRacers[i]->hasFullState) {
                    return 0;
                }
                ReceivedRacerState* oldest = aiRacers[i]->receivedStates[3];
                aiRacers[i]->receivedStates[3] = aiRacers[i]->receivedStates[2];
                aiRacers[i]->receivedStates[2] = aiRacers[i]->receivedStates[1];
                aiRacers[i]->receivedStates[1] = aiRacers[i]->receivedStates[0];
                aiRacers[i]->receivedStates[0] = oldest;
                aiRacers[i]->receivedStates[0]->messageFlags = time;
                reinterpret_cast<KrustyBike*>(aiRacers[i])->ApplyStateDelta(
                    reinterpret_cast<const KbNetDelta*>(message),
                    reinterpret_cast<KbNetState*>(aiRacers[i]->receivedStates[0]));
                aiRacers[i]->receivedStates[0]->state.time = (int)(s_UnknownStatic578e8c * 1000.0f);
                aiRacers[i]->receivedStates[0]->fresh = 0;
                return 1;
            }
        }
    } else if (kind == 10) {
        RacerScoreMessage* score = (RacerScoreMessage*)data;
        BikeRaceKrustyBikeView* racer = localRacer;
        if (recorderId == racer->recorderId && racer->netIndex == score->racerIndex) {
            if (racer == 0) {
                return 0;
            }
            racer->score = score->racerScore;
            *keep = 1;
            return 1;
        }
        for (int i = 0; i < g_TrackGame->mode.raceSettings.aiRacerCount; i++) {
            if (aiRacers[i] != 0 && recorderId == aiRacers[i]->recorderId &&
                aiRacers[i]->netIndex == score->racerIndex) {
                aiRacers[i]->score = score->racerScore;
                *keep = 1;
                return 1;
            }
        }
    } else if (kind == 0x11) {
        TagBallState* record = (TagBallState*)data;
        if (g_TrackGame->tagEvent != 0 && g_TrackGame->tagEvent->ball != 0) {
            Ball()->hasState = 1;
            TagBallReceivedState* oldest = Ball()->receivedStates[3];
            Ball()->receivedStates[3] = Ball()->receivedStates[2];
            Ball()->receivedStates[2] = Ball()->receivedStates[1];
            Ball()->receivedStates[1] = Ball()->receivedStates[0];
            Ball()->receivedStates[0] = oldest;
            Ball()->receivedStates[0]->recordTime = time;
            Ball()->receivedStates[0]->record = *record;
            Ball()->receivedStates[0]->record.runningTime = (int)(s_UnknownStatic578e8c * 1000.0f);
            Ball()->receivedStates[0]->fresh = 1;
            Ball()->velocity = record->velocity;
            Ball()->position = record->position;
            Ball()->angularVelocity = record->angularVelocity;
            Ball()->roll = record->roll;
            Ball()->pitch = record->pitch;
            Ball()->yaw = record->yaw;
            Ball()->runningTime = replayMode == 11 ? time : record->runningTime;
            *keep = 1;
            if (replayMode == 11) {
                Ball()->receivedStates[1]->record = Ball()->receivedStates[0]->record;
                Ball()->receivedStates[2]->record = Ball()->receivedStates[1]->record;
                Ball()->receivedStates[3]->record = Ball()->receivedStates[2]->record;
                Ball()->receivedStates[1]->recordTime = Ball()->receivedStates[0]->recordTime;
                Ball()->receivedStates[2]->recordTime = Ball()->receivedStates[1]->recordTime;
                Ball()->receivedStates[3]->recordTime = Ball()->receivedStates[2]->recordTime;
                Ball()->receivedStates[1]->fresh = 1;
                Ball()->receivedStates[2]->fresh = 1;
                Ball()->receivedStates[3]->fresh = 1;
                return 11;
            }
            return 1;
        }
    } else if (kind == 0x10) {
        if (flag) {
            return 1;
        }
        if (g_TrackGame->tagEvent != 0 && g_TrackGame->tagEvent->ball != 0 &&
            Ball()->hasState) {
            TagBallReceivedState* oldest = Ball()->receivedStates[3];
            Ball()->receivedStates[3] = Ball()->receivedStates[2];
            Ball()->receivedStates[2] = Ball()->receivedStates[1];
            Ball()->receivedStates[1] = Ball()->receivedStates[0];
            Ball()->receivedStates[0] = oldest;
            Ball()->receivedStates[0]->recordTime = time;
            Ball()->ApplyReplayDelta((TagBallStateDelta*)data,
                                                          Ball()->receivedStates[0]);
            Ball()->receivedStates[0]->record.runningTime = (int)(s_UnknownStatic578e8c * 1000.0f);
            Ball()->receivedStates[0]->fresh = 0;
            return 1;
        }
    } else if (kind == 0x87) {
        VcrFollowRecord* follow = (VcrFollowRecord*)data;
        if (g_TrackGame->tagEvent != 0) {
            g_TrackGame->tagEvent->followedRacer = 0;
            if (follow->followedRacerId != 0) {
                int iterator = 0;
                for (BikeRaceKrustyBikeView* racer = NextBike(&iterator); racer != 0;
                     racer = NextBike(&iterator)) {
                    if (racer->recorderId == follow->followedRacerId) {
                        g_TrackGame->tagEvent->followedRacer = (EventKrustyBikeView*)racer;
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
    BikeRaceKrustyBikeView* racer = aiRacers[index];
    if (racer != 0) {
        racer->receivedStates[0]->state.position = kVec3Zero;
        racer->receivedStates[1]->state.position = kVec3Zero;
        racer->receivedStates[2]->state.position = kVec3Zero;
        racer->receivedStates[3]->state.position = kVec3Zero;
    }
}

// 0x00422ec0
void BikeRace::ClearRacerParts() {
    BikeRaceKrustyBikeView* racer = localRacer;
    racer->receivedStates[0]->state.position = kVec3Zero;
    racer->receivedStates[1]->state.position = kVec3Zero;
    racer->receivedStates[2]->state.position = kVec3Zero;
    racer->receivedStates[3]->state.position = kVec3Zero;
    for (int i = 0; i < CountRaceRacers() - 1; i++) {
        racer = aiRacers[i];
        if (racer != 0) {
            racer->receivedStates[0]->state.position = kVec3Zero;
            racer->receivedStates[1]->state.position = kVec3Zero;
            racer->receivedStates[2]->state.position = kVec3Zero;
            racer->receivedStates[3]->state.position = kVec3Zero;
        }
    }
}

// 0x004230e0
int RecorderCallback(int kind, void* data, int flag, int recorderId, int* keep, int time,
                     int* milliseconds) {
    BikeRace* race = (BikeRace*)g_TrackGame->eventManager->FindRaceView();
    *milliseconds = (int)(race->replayTime * 1000.0f);
    return race->RecordFrame(kind, data, flag, recorderId, keep, time);
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
    item->next = list;
    list = item;
    while (list != 0) {
        if (list->node->flags & 4) {
            item = list;
            list = list->next;
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
                raceTerrain->QueryGround(reinterpret_cast<TerrainVec3*>(&a), 0, 0, 0);
                raceTerrain->QueryGround(reinterpret_cast<TerrainVec3*>(&b), 0, 0, 0);
                a.y += 5.0f;
                b.y += 5.0f;
                DrawLine((const Vec3*)&a, (const Vec3*)&b);
                a = *(Vector3*)&segment->edgeBX;
                b = *(Vector3*)&segment->next->edgeBX;
                raceTerrain->QueryGround(reinterpret_cast<TerrainVec3*>(&a), 0, 0, 0);
                raceTerrain->QueryGround(reinterpret_cast<TerrainVec3*>(&b), 0, 0, 0);
                a.y += 5.0f;
                b.y += 5.0f;
                DrawLine((const Vec3*)&a, (const Vec3*)&b);
                SetDrawColor(0, 0, 0xff);
                a = *(Vector3*)&segment->pointX;
                b = *(Vector3*)&segment->next->pointX;
                raceTerrain->QueryGround(reinterpret_cast<TerrainVec3*>(&a), 0, 0, 0);
                raceTerrain->QueryGround(reinterpret_cast<TerrainVec3*>(&b), 0, 0, 0);
                a.y += 5.0f;
                b.y += 5.0f;
                DrawLine((const Vec3*)&a, (const Vec3*)&b);
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
                    next->next = list;
                    list = next;
                }
            }
        }
    }
}

// 0x00423410
int BikeRace::Render3D() {
    Vector3 a;
    Vector3 b;
    if (drawChildren) {
        GameObject::Render3D();
    }
    if (debugDraw) {
        if (raceTrack != 0) {
            DrawTrack(raceTrack);
            if (hasStartProbe) {
                SetDrawColor(0xff, 0, 0xff);
                a = startProbe.center - startProbe.halfExtents;
                b = startProbe.center + startProbe.halfExtents;
                a.y += 2.5f;
                b.y += 2.5f;
                DrawLine((const Vec3*)&a, (const Vec3*)&b);
            }
            if (hasFinishProbe) {
                SetDrawColor(0, 0xff, 0xff);
                a = finishProbe.center - finishProbe.halfExtents;
                b = finishProbe.center + finishProbe.halfExtents;
                a.y += 2.5f;
                b.y += 2.5f;
                DrawLine((const Vec3*)&a, (const Vec3*)&b);
            }
        }
        if (debugDraw) {
            if (g_ObstacleDebugUpperHit) {
                SetDrawColor(0xff, 0, 0);
            } else {
                SetDrawColor(0, 0xff, 0);
            }
            DrawLine((const Vec3*)&g_ObstacleDebugUpperLine[0], (const Vec3*)&g_ObstacleDebugUpperLine[1]);
            if (g_ObstacleDebugHit) {
                SetDrawColor(0xff, 0, 0x80);
            } else {
                SetDrawColor(0, 0xff, 0x80);
            }
            DrawLine((const Vec3*)&g_ObstacleDebugLine[0], (const Vec3*)&g_ObstacleDebugLine[1]);
            if (g_ObstacleDebugLowerHit) {
                SetDrawColor(0xff, 0, 0xff);
            } else {
                SetDrawColor(0, 0xff, 0xff);
            }
            DrawLine((const Vec3*)&g_ObstacleDebugLowerLine[0], (const Vec3*)&g_ObstacleDebugLowerLine[1]);
            if (g_ObstacleDebugLeftHit) {
                SetDrawColor(0xff, 0, 0xff);
            } else {
                SetDrawColor(0, 0xff, 0xff);
            }
            DrawLine((const Vec3*)&g_ObstacleDebugLeftLine[0], (const Vec3*)&g_ObstacleDebugLeftLine[1]);
            if (g_ObstacleDebugRightHit) {
                SetDrawColor(0xff, 0, 0xff);
            } else {
                SetDrawColor(0, 0xff, 0xff);
            }
            DrawLine((const Vec3*)&g_ObstacleDebugRightLine[0], (const Vec3*)&g_ObstacleDebugRightLine[1]);
            SetDrawColor(0, 0, 0xff);
            DrawLine((const Vec3*)&g_GateDebugBoxLine[0], (const Vec3*)&g_GateDebugBoxLine[1]);
            SetDrawColor(0xff, 0, 0xff);
            DrawLine((const Vec3*)&g_GateDebugDirectionLine[0], (const Vec3*)&g_GateDebugDirectionLine[1]);
            SetDrawColor(0xff, 0, 0);
            DrawLine((const Vec3*)&g_PathTargetDebugLine[0], (const Vec3*)&g_PathTargetDebugLine[1]);
        }
    }
    return 1;
}

// 0x00423790
void BikeRace::SetDetailLevel(int level) {
    g_ActiveLodSettings = g_TrackGame->bladeRenderer ? g_BladeLodSettings : g_NonBladeLodSettings;
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
int BikeRace::OnControlRelease(ControlEvent* event, InputButtonState* entry) {
    if (TestInputEvent(6, 0, event, 0x80)) {
        drawChildren = 1 - drawChildren;
        return 1;
    }
    return 0;
}
