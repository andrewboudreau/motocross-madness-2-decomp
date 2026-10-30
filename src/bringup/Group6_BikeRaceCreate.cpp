// Group6_BikeRaceCreate.cpp -- best-guess BikeRace::Create (0x00417ed0), body taken unchanged from
// samples/race/BikeRaceNearMisses.cpp (unreviewed; not byte-matched). Preamble as Group1_BikeRaceCamera.cpp.


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

// BikeRace.cpp's TU-local vector helpers.
static inline Vector3 operator+(const Vector3& a, const Vector3& b) {
    return Vector3(a.x + b.x, a.y + b.y, a.z + b.z);
}

static inline Vector3 operator-(const Vector3& a, const Vector3& b) {
    return Vector3(a.x - b.x, a.y - b.y, a.z - b.z);
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


// DlgProcs.h's DemoDlg (vtable 0x00550e18, 0x7f58 bytes) with the inline
// constructor the setup's `new` expands (UIDialog(1, "credits.dtm"), then
// the vtable); DlgProcs.h declares no constructor for it.
class UnknownBikeRaceDemoDlg : public UIDialog {
public:
    UnknownBikeRaceDemoDlg(int flags, const char* resource) : UIDialog(flags, resource) {}
    virtual int Tick(float frameTime);                    // 0x00455b20
    virtual int OnControlPress(UnknownControlEvent* event, UnknownInputEntry* entry);       // 0x00455ae0
};

// The length of `v` (1 without a square root for unit vectors); the setup
// restores the lengths D3DRMVectorRotate's unit results lose.
static inline float UnknownVectorLength(const Vector3& v) {
    float xx = v.x * v.x;
    float yy = v.y * v.y;
    float lengthSquared = xx + yy + v.z * v.z;
    if (lengthSquared == 1.0f)
        return 1.0f;
    return (float)sqrt(lengthSquared);
}

static inline Vector3 UnknownScaled(const Vector3& v, float scale) {
    return Vector3(v.x * scale, v.y * scale, v.z * scale);
}

// 0x00417ed0
BikeRace* BikeRace::Create(void* owner, void* particles, LightManager* lights,
                                          UnknownTextureFormatChoice* textures,
                                          UnknownBikeRaceProjector* terrain, KrustyBikeCamera* camera,
                                          Scene* scene, UnknownBikeRaceNodeOwner* event,
                                          UnknownBikeRaceShadow* shadow) {
    char name[0x104];
    char path[0x104];
    char modelName[0x104];
    char message[0x144];
    Vector3 position;
    Vector3 direction;
    Vector3 normal;
    Vector3 point;

    GameObject::SetRenderContext(owner);
    field_0x25_bit1 = 1;
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
        UnknownTextureStream* stream =
            new(__FILE__, 0x193) UnknownTextureStream(g_ResourceManager);
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
            raceTerrain->QueryGround(&finishProbe.center, 0, 0, 0);
        if (hasStartProbe)
            raceTerrain->QueryGround(&startProbe.center, 0, 0, 0);
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
            UnknownTextureStream* stream =
                new(__FILE__, 0x1be) UnknownTextureStream(g_ResourceManager);
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
                terrain->QueryGround(&point, &normal, 0, 0);
                thirtySecondMan = (UnknownBikeRaceView6c*)raceScene->FindEntryByFileName("30SecMan.slt", &thirtySecondManEntry);
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
                    thirtySecondMan->model->SetPositionVec3(point);
                    thirtySecondMan->model->SetAxesPtr(&direction, &kVec3YAxis, 1, 0);
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
                terrain->QueryGround(&point, &normal, 0, 0);
                if (g_TrackGame->mode.raceSettings.eventType == 3) {
                    startGate =
                        (UnknownBikeRaceView6c*)raceScene->FindEntryByFileName("StartGate.slb", &startGateEntry);
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
                            startGate->model->SetPositionVec3(point);
                            startGate->model->SetAxesPtr(&direction, &kVec3YAxis, 1, 0);
                        }
                    } else if (startGate) {
                        startGate->Disable();
                    }
                }
            }
            if (hasFinishProbe) {
                point.x = finishProbe.center.x;
                point.z = finishProbe.center.z;
                terrain->QueryGround(&point, 0, 0, 0);
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
            girlOffset = UnknownScaled(turned, UnknownVectorLength(girlOffset));
            ground = point + girlOffset;
            terrain->QueryGround(&ground, 0, 0, 0);
            float height = ground.y - point.y;
            girlPosition = point + girlOffset;
            D3DRMVectorRotate(&turned, &boxOffset, (Vector3*)&kVec3YAxis, angle);
            boxOffset = UnknownScaled(turned, UnknownVectorLength(boxOffset));
            boxPosition = point + boxOffset;
            girlPosition.y += height;
            boxPosition.y += height;

            if (g_TrackGame->mode.raceSettings.raceMode != 0 &&
                g_TrackGame->mode.raceSettings.raceMode != 4) {
                sprintf(modelName, "%s\\%s", "Res", "FlagGirl.mcf");
                flagGirl = (UnknownBikeRaceCharacter*)new(__FILE__, 0x26f) D3DIMSoultreeCharacter(1);
                AppendChild(flagGirl->UnknownVirtualSlot11(renderContext, modelName, lights, textures, 1, 1),
                                      -1);
                ReplaceMeshId(flagGirl, 1, 2);
                flagGirl->modelNode->SetUseLighting(1);
                flagGirl->modelNode->SetPositionVec3(girlPosition);
                flagGirl->modelNode->SetAxesPtr(&direction, &kVec3YAxis, 1, 0);
                reinterpret_cast<Character*>(flagGirl)->SetMotionByName("Stand");
                flagGirl->UnknownVirtualSlot7(0.001f, 0, 0);
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
                                               (UnknownBonusRacer*)this, (UnknownBonusCamera*)raceCamera);
        if (!AppendChild((GameObject*)scoreBoard, -1))
            return 0;
    }
    g_TrackGame->ui->StepProgressBar();

    SetSceneTime(0.0f);
    raceScene->holdAnimations = 1;
    if (g_TrackGame->mode.raceSettings.raceMode) {
        NumberObjectManager* numbers = new(__FILE__, 0x29b) NumberObjectManager(1);
        digitCounter = (int)numbers->SetupDigits(renderContext, (int)textures, (UnknownNumberRacer*)this, 0x1e,
                                                (UnknownNumberCamera*)raceCamera, 3.0f);
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
        vcrFile = new(__FILE__, 0x2b6) UnknownVcrFile(1, 2);
        if (g_TrackGame->mode.ghostFile[0]) {
            vcrFile->Load(g_TrackGame->mode.ghostFile, "VCRgtemp.dat");
            ghostVcr = new(__FILE__, 0x2b9) KrustyVCR;
        }
        replayVcr = new(__FILE__, 0x2bd) KrustyVCR;
        ghostLap = 0;
    } else if (g_TrackGame->useReplayTape) {
        vcrFile = new(__FILE__, 0x2c2) UnknownVcrFile(0, 1);
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
            savedCameraState = UnknownFunctionCameraView()->cameraState;
            raceCamera->UnknownVirtualSlot71(1);
        }
    }
    g_TrackGame->ui->StepProgressBar();

    if (!CreateRacers(owner, lights, textures, terrain, event))
        return 0;
    g_TrackGame->ui->StepProgressBar();

    if (thirtySecondMan) {
        if (hasStartProbe) {
            localRacer->modelNode->GetPosition(&point);
            point.x += localRacer->heading.x * 15.0f;
            point.z += localRacer->heading.z * 15.0f;
        }
        terrain->QueryGround(&point, 0, 0, 0);
        thirtySecondMan->model->SetPositionVec3(point);
    }
    if (g_TrackGame->playingReplay && g_TrackGame->mode.raceSettings.raceMode != 4 &&
        g_TrackGame->useReplayTape) {
        if (g_TrackGame->ui->playingDemo) {
            UnknownBikeRaceDemoDlg* dialog = new(__FILE__, 0x2fb) UnknownBikeRaceDemoDlg(1, "credits.dtm");
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
                aiRacers[0]->UnknownFunction496e20(ghostVcr);
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
        localRacer->UnknownRacerVirtualSlot50(1, localRacer->UnknownRacerVirtualSlot45(), 0);
        for (int i = 0; i < count; i++)
            aiRacers[i]->UnknownRacerVirtualSlot50(1, aiRacers[i]->UnknownRacerVirtualSlot45(), 0);
    }
    g_TrackGame->ui->StepProgressBar();

    g_MemTagStack->Push("Audio");
    RaceSound* sound = new(__FILE__, 0x347) RaceSound(1);
    raceSound = (UnknownBikeRaceViews*)sound->Create(renderContext, (UnknownKrustyBikeView*)this,
                                                                      (UnknownRaceSoundCamera*)raceCamera, 6);
    AppendChild((GameObject*)raceSound, -1);
    g_MemTagStack->Push("BikeRace");
    if (g_TrackGame->mode.joystickOn) {
        g_TrackGame->controlInterface->activeJoystick->SetAcquired(0);
        g_TrackGame->controlInterface->activeJoystick->SetAutoCenter(g_TrackGame->mode.joystickOn);
        localRacer->UnknownRacerVirtualSlot93();
        g_TrackGame->controlInterface->activeJoystick->SetAcquired(1);
    }
    if (g_TrackGame->mode.raceSettings.eventType)
        BuildStatusList(&statusList, (UnknownKrustyBikeView*)this);
    if (ghostVcr)
        localRacer->fastestLap = bestGhostLength;
    g_TrackGame->ui->StepProgressBar();

    UnknownBikeNumberPainter* painter = new(__FILE__, 0x367) UnknownBikeNumberPainter(textures->manager);
    if (g_TrackGame->playingReplay) {
        painter->PaintNumber(reinterpret_cast<D3DIMSoultreeObject*>(localRacer->modelNode), localRacer->plateNumber);
        for (int i = 0; i < UnknownFunctionRacerCount() - 1; i++)
            painter->PaintNumber(reinterpret_cast<D3DIMSoultreeObject*>(aiRacers[i]->modelNode), aiRacers[i]->plateNumber);
    } else if (g_TrackGame->playerCount > 1) {
        for (int i = 0; i < UnknownFunctionRacerCount(); i++) {
            for (int j = 0; j < UnknownFunctionRacerCount(); j++) {
                UnknownBikeRaceRacer* racer = racerSlots[i];
                UnknownTrackGameRacerSlot* slot = &g_TrackGame->mode.racers[j];
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
