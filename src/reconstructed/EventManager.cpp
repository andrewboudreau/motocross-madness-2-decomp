#include "BikeRaceCore.h"
#include "GUIManagerCore.h"
#include "EventManager.h"

#include <windows.h>
#include <imm.h>
#include <float.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../krusty2/broadphase/Terrain.h"
#include "ArcadeObject.h"
#include "Camera.h"
#include "D3DIMSoulTree.h"
#include "DebugAlloc.h"
#include "DirectPlayMessages.h"
#include "ControlInterface.h"
#include "GameUi.h"
#include "MenuId.h"
#include "QuarryEvent.h"
#include "TrackGame.h"
#include "SoundInterface.h"
#include "UIDialog.h"
#include "PCAudio.h"
#include "SoultreeMaterial.h"
#include "PCCamera.h"
#include "TextureMap.h"
#include "ResourceEntry.h"

// EventManager.cpp's per-file vector set (0x0059af28, 0x0059af38,
// 0x0059af48 and 0x0059af18), initialised by 0x0045fe40..0x0045ff7b; the
// podium scene 0x0045d480 reads the y and z axes. Not const: the podium
// passes the y axis to D3DRMVectorRotate (LPD3DVECTOR).
static Vector3 kVec3Zero = Vector3(0.0f, 0.0f, 0.0f);
static Vector3 kVec3XAxis = Vector3(1.0f, 0.0f, 0.0f);
static Vector3 kVec3YAxis = Vector3(0.0f, 1.0f, 0.0f);
static Vector3 kVec3ZAxis = Vector3(0.0f, 0.0f, 1.0f);

// Characters kept in recording file names.
#define FILE_NAME_CHARACTERS "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789_-()$#"

// Network messages handled by slot 24 (the layout depends on the type).
// For DPSYS_DESTROYPLAYERORGROUP the fields are DPMSG_DESTROYPLAYERORGROUP's
// dwType, dwPlayerType and dpId.
struct UnknownEventPlayerMessage {
    int header;                                    // +0x00: type (DPMSG dwType for system messages)
    int player;                                    // +0x04: player (types 0xcc and 0x8e)
    int dpId;                                      // +0x08: DPSYS_DESTROYPLAYERORGROUP
};
struct RacerResultsMessage {                  // type 0x86
    int header;                                    // +0x00: type byte and sequence
    char racerIndex;                               // +0x04: the sender's racer index
    char finished;                                 // +0x05: racer +0x7a4
    float score;                                   // +0x08: racer +0x768
    float totalRunningTime;                        // +0x0c: racer +0x754
    float fastestLap;                              // +0x10: racer +0x750
    int largestStuntPoints;                        // +0x14: racer +0x788
    int totalTrickTime;                            // +0x18: racer +0x758
    float bestTagRun;                              // +0x1c: racer bestTagRun
    float totalTagRun;                             // +0x20: racer totalTagRun
};

// 0x0045c830
RacerResult::RacerResult() {
    Reset();
}

// 0x0045c840: an empty entry in position 1.
void RacerResult::Reset() {
    playerId = 0;
    position = 1;
    fastestLap = 0;
    totalRunningTime = 0;
    points = 0;
    score = 0;
    aiIndex = 0;
    largestStuntPoints = 0;
    totalTrickTime = 0;
    bestTagRun = 0;
    totalTagRun = 0;
    finished = 0;
    strcpy(name, "");
    outOfRace = 0;
    repairDamage = 0;
    injuryDamage = 0;
    stuntEarnings = 0;
}

// 0x0045c8b0: the racer's results. A racer with +0x4a0 set is listed in
// position 99 with nothing else; in modes 0 and 4 every racer counts as
// finished.
void RacerResult::CopyFromRacer(EventKrustyBikeView* racer) {
    playerId = racer->networkPlayerId;
    int length = strlen(racer->racerName);
    int count = length > 15 ? 15 : length;
    strncpy(name, racer->racerName, count);
    name[count] = 0;
    outOfRace = racer->outOfRace;
    if (!outOfRace) {
        score = racer->score;
        position = racer->finishPosition;
        fastestLap = racer->fastestLap;
        totalRunningTime = racer->totalRunningTime;
        largestStuntPoints = racer->largestStuntPoints;
        totalTrickTime = racer->totalTrickTime;
        bestTagRun = racer->bestTagRun;
        totalTagRun = racer->totalTagRun;
        aiIndex = racer->aiIndex;
        repairDamage = racer->repairDamage;
        injuryDamage = racer->injuryDamage;
        stuntEarnings = racer->stuntEarnings;
        if (g_TrackGame->mode.raceSettings.eventType == 0 || g_TrackGame->mode.raceSettings.eventType == 4)
            finished = 1;
        else
            finished = racer->hasFinished;
    } else {
        score = 0;
        position = 99;
        fastestLap = 0;
        totalRunningTime = 0;
        largestStuntPoints = 0;
        totalTrickTime = 0;
        bestTagRun = 0;
        totalTagRun = 0;
        aiIndex = 0;
        repairDamage = 0;
        injuryDamage = 0;
        stuntEarnings = 0;
        finished = 0;
    }
}

// 0x0045c9e0
EventManager::EventManager(int flags) : GameObject(flags) {
    waitingForRacers = 0;
    keepAliveStarted = 0;
    endNetworkGamePending = 0;
    anyRacerFinished = 0;
    allRacersFinished = 0;
    racesRun = 0;
    resultCount = 0;
    endTimer = 0;
    keepAliveTimeout = 20.0f;
    keepAliveInterval = 1.0f;
    resumeArmed = 0;
    podiumCamera = 0;
    podiumObject = 0;
    podiumPosition = Vector3(-1000.0f, -1000.0f, -1000.0f);
}

// 0x0045cae0
EventManager::~EventManager() {}

// 0x0045caf0
GameObject* EventManager::SetRenderContext(void* context) {
    GameObject::SetRenderContext(context);
    keepAliveTimeout = (float)g_TrackGame->GetRegistryInt("KeepAliveTimeout", 20);
    return this;
}

// 0x0045cb20: adds *step to KrustyUI's "ProgressBar"; without a step it
// calls KrustyUI 0x0049b530. The Game pointer is re-read at each use: a
// KrustyUI local gives the same code but with the two registers of the
// final add swapped.
void LoadProgressCallback(int* step) {
    if (step) {
        if (g_TrackGame->ui->progressDialog) {
            UIControl* bar =
                g_TrackGame->ui->progressDialog->FindControl("ProgressBar", 0);
            static_cast<UIProgressBar*>(bar)->stepCount += *step;
        }
    } else {
        g_TrackGame->ui->StepProgressBar();
    }
}

// 0x0059af54: seconds the event has run with no input dialog open;
// LoadEvent (0x0045cb70) clears it and CheckRaceOver ends the race at 315.
float g_EventTimeoutClock;

// 0x0045cb70: resets the event, loads the track's environment and scene
// and, for mode 0 (BaseQuarryEvent) or 2 (NationalRace), creates the race
// object and adds it to the second root; 1 when a race-mode object exists.
int EventManager::LoadEvent() {
    char path[260];
    g_EventTimeoutClock = 0;
    endTimer = 0;
    keepAliveStarted = 0;
    waitingForRacers = 0;
    for (int i = 0; i < 8; i++)
        g_TrackGame->mode.racers[i].isReady = 0;
    podiumCharacterCount = 0;
    for (int j = 0; j < 3; j++)
        podiumCharacters[j] = 0;
    podiumCamera = 0;
    podiumObject = 0;
    g_TrackGame->ui->StepProgressBar();
    g_TrackGame->mode.FindFileDirectory(g_TrackGame->mode.currentSeriesDirectory,
                                                      g_TrackGame->mode.raceSettings.trackName, "env", path);
    g_TrackGame->sceneObject->SetSceneDirectory(path);
    switch (g_TrackGame->mode.raceSettings.eventType) {
    case 0:
        g_TrackGame->sceneObject->SelectScene(g_TrackGame->mode.raceSettings.trackName, "scn", 0);
        g_TrackGame->quarryEvent = (TrackGameViewOwner*)(new(__FILE__, 252) BaseQuarryEvent(1))
            ->Create(g_TrackGame->renderTarget, LoadProgressCallback);
        if (!g_TrackGame->childRoot->AppendChild(g_TrackGame->quarryEvent, -1))
            return 0;
        break;
    case 2:
        g_TrackGame->sceneObject->SelectScene(g_TrackGame->mode.raceSettings.trackName, "scn", 0);
        g_TrackGame->nationalEvent = (TrackGameViewOwner*)(new(__FILE__, 270) NationalRace(1))
            ->Create(g_TrackGame->renderTarget, LoadProgressCallback);
        if (!g_TrackGame->childRoot->AppendChild(g_TrackGame->nationalEvent, -1))
            return 0;
        break;
    }
    return FindRaceMode() != 0;
}

// 0x0045cdc0: ends the race. With `mode` 1 (0x0045d480, before the podium
// scene) it only calls slot 4 on the race objects and racers. Otherwise it saves the replay (Record\\<scene>_<racer>_
// <date>_<time>.vcr, filtered to file-name characters) and, in mode 4, the
// ghost (.gho), then releases the first race-mode object.
void EventManager::CloseRace(int mode) {
    char date[32];
    char scene[64];
    char name[260];
    char filtered[260];
    g_TrackGame->ui->demoEnded = 0;
    EventBikeRaceView* view = FindRaceView();
    TrackGameViewOwner* owner = FindRaceMode();
    g_TrackGame->mode.SaveProfile();
    if (mode == 1) {
        if (owner) {
            if (owner->radarOverlay)
                owner->radarOverlay->Disable();
            if (owner->statsOverlay)
                owner->statsOverlay->Disable();
            if (owner->instrumentOverlay)
                owner->instrumentOverlay->Disable();
            if (owner->chatOverlay)
                owner->chatOverlay->Disable();
        }
        view->raceCamera->Disable();
        if (view->raceSound)
            view->raceSound->Disable();
        if (view->youIndicator)
            view->youIndicator->Disable();
        int iterator = 0;
        EventKrustyBikeView* racer;
        while ((racer = reinterpret_cast<EventKrustyBikeView*>(reinterpret_cast<BikeRace*>(view)->NextBike(&iterator))) != 0)
            racer->Disable();
        return;
    }
    if (view) {
        if (g_TrackGame->mode.saveReplay && !g_TrackGame->playingReplay &&
            g_TrackGame->mode.raceSettings.raceMode != 4) {
            char description[32];
            char time[32];
            g_TrackGame->sceneObject->ReadCurrentSceneName(
                scene, g_TrackGame->sceneObject->sceneFile, 0);
            GetDateFormatA(0, 0, 0, "yyyyMMdd", date, sizeof(date));
            GetTimeFormatA(0, 0, 0, "HHmm", time, sizeof(time));
            sprintf(name, "%s_%s_%s_%s", scene, view->localRacer->racerName, date, time);
            int length = strlen(name);
            int count = 0;
            filtered[0] = 0;
            for (int i = 0; i < length; i++) {
                if (strchr(FILE_NAME_CHARACTERS, name[i]))
                    filtered[count++] = name[i];
            }
            filtered[count] = 0;
            sprintf(name, "Record\\%s.vcr", filtered);
            GetDateFormatA(0, 0, 0, "M/d/yyyy", date, sizeof(date));
            GetTimeFormatA(0, 0, 0, "h:mm tt", time, sizeof(time));
            sprintf(description, "%s %s", date, time);
            reinterpret_cast<BikeRace*>(view)->SaveReplay(name, description);
        }
        if (g_TrackGame->mode.raceSettings.raceMode == 4 && g_TrackGame->mode.saveGhost) {
            char time[32];
            g_TrackGame->sceneObject->ReadCurrentSceneName(
                scene, g_TrackGame->sceneObject->sceneFile, 0);
            GetDateFormatA(0, 0, 0, "yyyyMMdd", date, sizeof(date));
            GetTimeFormatA(0, 0, 0, "HHmm", time, sizeof(time));
            sprintf(name, "%s_%s_%s_%s", scene, view->localRacer->racerName, date, time);
            int length = strlen(name);
            int count = 0;
            filtered[0] = 0;
            for (int i = 0; i < length; i++) {
                if (strchr(FILE_NAME_CHARACTERS, name[i]))
                    filtered[count++] = name[i];
            }
            filtered[count] = 0;
            sprintf(name, "Record\\%s.gho", filtered);
            if (view->vcrFile)
                view->vcrFile->Save(name);
        }
    }
    if (g_TrackGame->supercrossEvent) {
        g_TrackGame->supercrossEvent->Unlink();
        g_TrackGame->supercrossEvent->Release();
        g_TrackGame->supercrossEvent = 0;
    } else if (g_TrackGame->quarryEvent) {
        g_TrackGame->quarryEvent->Unlink();
        g_TrackGame->quarryEvent->Release();
        g_TrackGame->quarryEvent = 0;
    } else if (g_TrackGame->bajaEnduroEvent) {
        g_TrackGame->bajaEnduroEvent->Unlink();
        g_TrackGame->bajaEnduroEvent->Release();
        g_TrackGame->bajaEnduroEvent = 0;
    } else if (g_TrackGame->nationalEvent) {
        g_TrackGame->nationalEvent->Unlink();
        g_TrackGame->nationalEvent->Release();
        g_TrackGame->nationalEvent = 0;
    } else if (g_TrackGame->tagEvent) {
        g_TrackGame->tagEvent->Unlink();
        g_TrackGame->tagEvent->Release();
        g_TrackGame->tagEvent = 0;
    }
}

// 0x0045d270
void EventManager::EnableRaceObjects() {
    TrackGameViewOwner* owner = FindRaceMode();
    if (!owner)
        return;
    owner->Enable();
    EventBikeRaceView* view = FindRaceView();
    if (view)
        view->Enable();
    TextQueueOverlay* target = FindTextQueue();
    if (target)
        target->Enable();
}

// 0x0045d2b0
TrackGameViewOwner* EventManager::FindRaceMode() {
    if (g_TrackGame->supercrossEvent)
        return g_TrackGame->supercrossEvent;
    if (g_TrackGame->quarryEvent)
        return g_TrackGame->quarryEvent;
    if (g_TrackGame->bajaEnduroEvent)
        return g_TrackGame->bajaEnduroEvent;
    if (g_TrackGame->nationalEvent)
        return g_TrackGame->nationalEvent;
    return g_TrackGame->tagEvent;
}

// 0x0045d2f0
EventBikeRaceView* EventManager::FindRaceView() {
    if (g_TrackGame->supercrossEvent)
        return g_TrackGame->supercrossEvent->raceView;
    if (g_TrackGame->quarryEvent)
        return g_TrackGame->quarryEvent->raceView;
    if (g_TrackGame->bajaEnduroEvent)
        return g_TrackGame->bajaEnduroEvent->raceView;
    if (g_TrackGame->nationalEvent)
        return g_TrackGame->nationalEvent->raceView;
    if (g_TrackGame->tagEvent)
        return g_TrackGame->tagEvent->raceView;
    return 0;
}

// 0x0045d340
TextQueueOverlay* EventManager::FindTextQueue() {
    if (g_TrackGame->supercrossEvent)
        return g_TrackGame->supercrossEvent->textQueue;
    if (g_TrackGame->quarryEvent)
        return g_TrackGame->quarryEvent->textQueue;
    if (g_TrackGame->bajaEnduroEvent)
        return g_TrackGame->bajaEnduroEvent->textQueue;
    if (g_TrackGame->nationalEvent)
        return g_TrackGame->nationalEvent->textQueue;
    if (g_TrackGame->tagEvent)
        return g_TrackGame->tagEvent->textQueue;
    return 0;
}

// 0x0045d390
int EventManager::HasRaceMode() {
    if (g_TrackGame->supercrossEvent || g_TrackGame->quarryEvent ||
        g_TrackGame->bajaEnduroEvent || g_TrackGame->nationalEvent ||
        g_TrackGame->tagEvent)
        return 1;
    return 0;
}

// 0x0045d3d0
int CompareStandings(const void* a, const void* b) {
    const RacerStanding* first = (const RacerStanding*)a;
    const RacerStanding* second = (const RacerStanding*)b;
    if (first->points > second->points)
        return -1;
    if (first->points == second->points) {
        if (first->field_0x04 < second->field_0x04)
            return -1;
        if (first->field_0x04 == second->field_0x04) {
            if (first->field_0x08 < second->field_0x08)
                return -1;
            if (first->field_0x08 == second->field_0x08)
                return strcmp(first->racer->racerName, second->racer->racerName);
        }
    }
    return 1;
}

// Views for the podium scene 0x0045d480. Only what it calls is declared.
struct PodiumSceneEnvironmentView {                         // the view part's +0xa4
    unsigned char field_0x000[0x398];
    float terrainBreadth;                           // +0x398 SceneEnvironment terrainBreadth
};
struct PodiumSceneView {                          // TrackGameViewOwner+0x2c
    unsigned char field_0x00[0x8c];
    Vector3 position;                               // +0x8c podium position
    Vector3 direction;                              // +0x98 podium direction (0 when none)
    PodiumSceneEnvironmentView* model;                      // +0xa4
    unsigned char field_0xa8[0xc4 - 0xa8];
    GameObject* soundGroup;                         // +0xc4
};
struct PodiumBaseQuarryEventView : public GameObject {     // TrackGameViewOwner
    PodiumSceneView* grid;                        // +0x2c
    unsigned char skyCube[0x34 - 0x30];  // +0x30 BaseQuarryEvent skyCube (bytes kept)
    EventBikeRaceView* raceView;                // +0x34
    unsigned char field_0x38[0x7c - 0x38];
    LightManager* lights;                           // +0x7c
    unsigned char field_0x80[0x94 - 0x80];
    int opaqueTextureGroup;                         // +0x94, as BaseQuarryEvent.h
    int alphaTextureGroup;                          // +0x98
};
struct PodiumStartGateView {                     // the view's +0x64
    unsigned char field_0x000[0x1a0];
    SoultreeObject* modelNode;                      // +0x1a0 the characters' model node
};
struct PodiumBikeRaceView {
    unsigned char field_0x00[0x4c];
    Terrain* trackObject;                           // +0x4c
    unsigned char field_0x50[0x64 - 0x50];
    PodiumStartGateView* nodeOwner;              // +0x64
};
struct PodiumRiderView {                    // racer +0x5c4
    unsigned char field_0x000[0x1a0];
    D3DIMSoultreeObject* modelNode;                 // +0x1a0
};
struct PodiumKrustyBikeView {
    unsigned char field_0x000[0x5c4];
    PodiumRiderView* riderCharacter;      // +0x5c4
    unsigned char loadInfo[0x5e0 - 0x5c8];    // +0x5c8 Bike loadTextures (a TextureContext; bytes kept)
    char riderName[16];                       // +0x5e0 Bike riderName
    void* frontWheel;                         // +0x5f0 Bike frontWheel
    void* rearWheel;                          // +0x5f4 Bike rearWheel
    void* leftBarContact;                     // +0x5f8 Bike leftBarContact
    void* rightBarContact;                    // +0x5fc Bike rightBarContact
    int tailContact;                          // +0x600 Bike tailContact
    void* wrecker;                            // +0x604 Bike wrecker
    unsigned char field_0x608[0x734 - 0x608];
    char aiKind;                              // +0x734 KrustyBike aiKind
    char isRemote;                            // +0x735 KrustyBike isRemote
    char isGhost;                             // +0x736 KrustyBike isGhost
    char fourStroke;                          // +0x737 KrustyBike fourStroke
    int engineSize;                           // +0x738 KrustyBike engineSize
    int plateNumber;                          // +0x73c KrustyBike plateNumber
    void* race;                               // +0x740 KrustyBike race
    void* statusNode;                         // +0x744 KrustyBike statusNode
    int finishTimeStamp;                      // +0x748 KrustyBike finishTimeStamp
    float lastLapTime;                        // +0x74c KrustyBike lastLapTime
    float fastestLap;                         // +0x750 KrustyBike fastestLap
    float totalRunningTime;                   // +0x754 KrustyBike totalRunningTime
    float totalTrickTime;                     // +0x758 KrustyBike totalTrickTime
    float tagRun;                             // +0x75c KrustyBike tagRun
    float bestTagRun;                         // +0x760 KrustyBike bestTagRun
    float totalTagRun;                        // +0x764 KrustyBike totalTagRun
    float score;                              // +0x768 KrustyBike score
    float offTrackTime;                       // +0x76c KrustyBike offTrackTime
    float timeBehindLeader;                   // +0x770 KrustyBike timeBehindLeader
    float lapStartTime;                       // +0x774 KrustyBike lapStartTime
    float aiCrashImpactLimit;                 // +0x778 KrustyBike aiCrashImpactLimit
    void* heapBufferA;                        // +0x77c KrustyBike heapBufferA
    void* heapBufferB;                        // +0x780 KrustyBike heapBufferB
    int place;                                      // +0x784
};

extern "C" Vector3* __stdcall D3DRMVectorRotate(Vector3* result, Vector3* vector, Vector3* axis,
                                                float theta);

// |v|, exact for unit vectors. Sums in retail's order, (x*x + y*y) + z*z;
// retail rounds z*z to a float temporary (0x0045dc05), which at /O2
// without /Op VC6 keeps on the x87 stack, so the two agree at 24-bit
// precision.
static inline float PodiumLength(const Vector3& v) {
    float xx = v.x * v.x;
    float yy = v.y * v.y;
    float squared = xx + yy + v.z * v.z;
    if (squared == 1.0f)
        return 1.0f;
    return (float)sqrt(squared);
}

static inline Vector3 operator+(const Vector3& a, const Vector3& b) {
    return Vector3(a.x + b.x, a.y + b.y, a.z + b.z);
}
static inline Vector3 operator-(const Vector3& a, const Vector3& b) {
    return Vector3(a.x - b.x, a.y - b.y, a.z - b.z);
}

#define PODIUM_OWNER(o) ((PodiumBaseQuarryEventView*)(o))
#define PODIUM_VIEW(v) ((PodiumBikeRaceView*)(v))
#define PODIUM_MODEL(c) ((D3DIMSoultreeObject*)(c)->modelNode)

// Whether v is the zero vector (the grid's "no podium direction").
static inline int PodiumIsZero(const Vector3& v) {
    return v.x == 0.0f && v.y == 0.0f && v.z == 0.0f;
}

// Whether the career's current race is its series' last (the bonus track
// is the last when there is one).
static inline int PodiumIsLastRace(ProCircuit* circuit) {
    int last;
    if (circuit->schedules[circuit->series].bonusTrack)
        last = circuit->schedules[circuit->series].bonusTrack - 1;
    else
        last = circuit->schedules[circuit->series].trackCount;
    return circuit->raceIndex == last;
}

// 0x0045d480. Not exact (4167 of 4247 bytes: retail spills the z square
// to a temporary where VC6 keeps it on the x87 stack, so the scalar homes
// at +0x1c/+0x20 swap; the place counter store and increments differ;
// docs/EVENTMANAGER.md forms tried)
// Builds the podium scene: the podium, a Winner.mcf character for each of
// the first three places, the crowd loop and the camera.
int EventManager::CreatePodiumScene() {
    char name[0x80];
    char path[0x104];
    char message[0x184];
    ProCircuit* circuit = g_TrackGame->proCircuit;
    TrackGameViewOwner* owner = FindRaceMode();
    EventBikeRaceView* view = FindRaceView();
    int iterator = 0;
    int ok;
    if (g_TrackGame->mode.raceSettings.raceMode == 2)
        ok = racesRun > g_TrackGame->mode.raceSettings.races;
    else
        ok = 1;
    if (circuit) {
        ok = PodiumIsLastRace(circuit);
        if (!ok)
            return 0;
    } else {
        if (!ok || !g_TrackGame->mode.showPodiums)
            return 0;
        if (g_TrackGame->mode.raceSettings.raceMode != 2 && !g_TrackGame->network &&
            results[0].position > 3)
            return 0;
    }
    CloseRace(1);
    PCCamera* camera = new(__FILE__, 0x276) PCCamera(1);
    podiumCamera = (Camera*)camera->SetRenderContext(renderContext);
    if (!view->AppendChild(podiumCamera, -1))
        return 0;
    ((RenderTarget*)renderContext)->SetCamera(podiumCamera);
    podiumCamera->InsertBefore(view->raceCamera, -1);
    PodiumSceneView* grid = PODIUM_OWNER(owner)->grid;
    float rows = grid->model->terrainBreadth;
    if (!((int)rows & 1))
        rows -= 1.0f;
    float column = rows * 0.5f;
    float row = rows * 0.5f;
    Vector3 direction;
    Vector3 up;
    if (!PodiumIsZero(grid->direction)) {
        podiumPosition = grid->position;
        direction = PODIUM_OWNER(owner)->grid->direction;
    } else {
        if (g_TrackGame->mode.raceSettings.eventType == 3 && PODIUM_VIEW(PODIUM_OWNER(owner)->raceView)->nodeOwner) {
            PODIUM_VIEW(PODIUM_OWNER(owner)->raceView)->nodeOwner->modelNode->GetPosition((Vec3*)&podiumPosition);
            PODIUM_VIEW(PODIUM_OWNER(owner)->raceView)->nodeOwner->modelNode->GetAxes((Vec3*)&direction, (Vec3*)&up);
        } else {
            float spacing = PODIUM_VIEW(PODIUM_OWNER(owner)->raceView)->trackObject->gridCellSize;
            podiumPosition = Vector3((int)column * spacing * 256.0f, 0.0f, (int)row * spacing * 256.0f);
            direction = kVec3ZAxis;
        }
    }
    up = kVec3YAxis;
    PODIUM_VIEW(PODIUM_OWNER(owner)->raceView)->trackObject->QueryGround((TerrainVec3*)&podiumPosition, 0, 0, 0);
    float angle = (float)atan2(direction.x, direction.z);
    Vector3 offsets[3];
    Vector3 facings[3];
    Vector3 cameraOffset;
    Vector3 lookOffset;
    int kind = g_TrackGame->mode.GetSelectedSeries();
    if (kind >= 2 && kind <= 4) {
        offsets[0] = Vector3(0.0f, 0.383f, 0.0f);
        offsets[1] = Vector3(0.0f, 0.383f, 0.0f);
        offsets[2] = Vector3(0.0f, 0.383f, 0.0f);
        facings[0] = Vector3(0.0f, 0.0f, -1.0f);
        facings[1] = Vector3(0.0f, 0.0f, -1.0f);
        facings[2] = Vector3(0.0f, 0.0f, -1.0f);
        podiumPosition.y += 1.5f;
        cameraOffset = Vector3(-10.0f, 15.0f, -20.0f);
        lookOffset = Vector3(5.0f, 9.0f, -10.0f);
        podiumFocus = podiumPosition;
        podiumFocus.y += 6.0f;
    } else {
        offsets[0] = Vector3(0.0f, 3.4f, -6.0f);
        offsets[1] = Vector3(-11.0f, 3.4f, -7.0f);
        offsets[2] = Vector3(10.0f, 3.4f, -9.0f);
        facings[0] = Vector3(0.0f, 0.0f, -1.0f);
        facings[1] = Vector3((float)sin(2.967059388756752), 0.0f, (float)cos(2.967059388756752));
        facings[2] = Vector3((float)sin(-2.617993578314781), 0.0f, (float)cos(-2.617993578314781));
        cameraOffset = Vector3(-25.0f, 7.0f, -15.0f);
        lookOffset = Vector3(20.0f, 13.0f, -28.0f);
        podiumFocus = podiumPosition;
        podiumFocus.y += 6.0f;
        podiumFocus.z -= 6.0f;
    }
    Vector3 rotated;
    for (int i = 0; i < 3; i++) {
        D3DRMVectorRotate(&rotated, &offsets[i], &kVec3YAxis, angle);
        offsets[i] = rotated * PodiumLength(offsets[i]);
        D3DRMVectorRotate(&rotated, &facings[i], &kVec3YAxis, angle);
        facings[i] = rotated * PodiumLength(facings[i]);
    }
    TextureContext context;
    context.manager = g_TrackGame->textureManager;
    context.opaqueGroup = (ManagedTextureGroup*)PODIUM_OWNER(owner)->opaqueTextureGroup;
    context.alphaGroup = (ManagedTextureGroup*)PODIUM_OWNER(owner)->alphaTextureGroup;
    context.opaqueFormat = ((RenderTarget*)renderContext)->surfacePixelFormat;
    context.alphaFormat = 0x115c;
    context.halvings = 0;
    podiumObject->Enable();
    podiumObject->SetPosition(&podiumPosition);
    podiumObject->SetAxes(&direction, &up, 0, 1);
    if (PODIUM_OWNER(owner)->lights && PODIUM_OWNER(owner)->lights->isEnabled)
        podiumObject->arcadeModel->SetUseLighting(1);
    FileStream* stream = new(__FILE__, 0x2f5) FileStream(g_ResourceManager);
    podiumCharacterCount = resultCount < 3 ? resultCount : 3;
    float chance = rand() * (1.0f / 32768);
    for (int place = 1; place - 1 < podiumCharacterCount; place++) {
        EventKrustyBikeView* racer = 0;
        iterator = 0;
        racer = reinterpret_cast<EventKrustyBikeView*>(reinterpret_cast<BikeRace*>(PODIUM_OWNER(owner)->raceView)->NextBike(&iterator));
        while (racer && ((PodiumKrustyBikeView*)racer)->place != place)
            racer = reinterpret_cast<EventKrustyBikeView*>(reinterpret_cast<BikeRace*>(PODIUM_OWNER(owner)->raceView)->NextBike(&iterator));
        if (g_TrackGame->InProCircuitCareer()) {
            ProCircuit* circuit = g_TrackGame->proCircuit;
            if ((circuit->careerFlags & 2) &&
                racer == PODIUM_OWNER(owner)->raceView->localRacer)
                sprintf(name, "%s\\Winnerd.mcf", "Res");
            else
                sprintf(name, "%s\\Winner.mcf", "Res");
        } else {
            sprintf(name, "%s\\Winner.mcf", "Res");
        }
        if (!g_TrackGame->sceneObject->OpenArchiveFile(stream, name, "rb", (int)path)) {
            sprintf(message, "No winner animation file found in resources.  Aborting.\n");
            delete stream;
            return 0;
        }
        D3DIMSoultreeCharacter*& character = podiumCharacters[place - 1];
        character = new(__FILE__, 0x31d) D3DIMSoultreeCharacter(isEnabled);
        character->LoadCharacter((int)renderContext, path, PODIUM_OWNER(owner)->lights,
                                       &context, 1, 1);
        view->AppendChild(character, -1);
        if (character) {
            if (PODIUM_OWNER(owner)->lights && PODIUM_OWNER(owner)->lights->isEnabled)
                PODIUM_MODEL(character)->SetUseLighting(1);
            int mode = g_TrackGame->mode.GetSelectedSeries();
            if (mode == 3 || mode == 2 || mode == 4) {
                if (chance < 0.5f && podiumCharacterCount != 1)
                    sprintf(name, "Podium3_%02d", place);
                else
                    sprintf(name, "Podium5_%02d", place);
            } else {
                sprintf(name, "Podium4_%02d", place);
            }
            podiumMotions[place - 1] = character->FindMotion(name, 1);
            Vector3 position = podiumPosition + offsets[place - 1];
            PODIUM_MODEL(character)->SetPosition(*(Vec3*)&position);
            PODIUM_MODEL(character)->SetAxesPtr((Vec3*)&facings[place - 1], (Vec3*)&kVec3YAxis, 1, 1);
            character->SetMotion(podiumMotions[place - 1]);
            character->motionTime = 0.0f;
            if (racer) {
                SoultreeMaterial** texture = ((PodiumKrustyBikeView*)racer)->riderCharacter->modelNode->materialTable;
                if (texture)
                    PODIUM_MODEL(character)->ReplaceMaterialTexture(0, (*texture)->textureName, &context);
            }
        }
    }
    delete stream;
    if (g_TrackGame->mode.soundOn && g_TrackGame->mode.networkGame) {
        Sound* sound = new(__FILE__, 0x34e) Sound((SoundGroup*)PODIUM_OWNER(owner)->grid->soundGroup, 1);
        FileStream* audio = new(__FILE__, 0x34f) FileStream(g_ResourceManager);
        if (!g_TrackGame->sceneObject->OpenArchiveFile(audio, "CrowdLoop.wav", "rb", 0)) {
            char text[0x80];
            sprintf(text, "CrowdLoop.wav not found in Audio.res.\n");
            delete audio;
            return 0;
        }
        sound->Load("CrowdLoop.wav", audio, 1, 0, 0, -1);
        delete audio;
        sound->PlayWithOptions(0, 1, 0);
    }
    D3DRMVectorRotate(&rotated, &cameraOffset, &kVec3YAxis, angle);
    podiumCameraPosition = rotated * PodiumLength(cameraOffset) + podiumPosition;
    D3DRMVectorRotate(&rotated, &lookOffset, &kVec3YAxis, angle);
    podiumCameraTarget = rotated * PodiumLength(lookOffset) + podiumPosition;
    podiumPanSpeed = podiumCameraTarget - podiumCameraPosition;
    podiumCamera->SetFrame(&podiumCameraPosition, 0, 0, 0, 0);
    podiumCamera->LookAt(podiumFocus);
    float x0 = podiumPosition.x - 16.0f;
    float x1 = x0 + 32.0f;
    float z0 = podiumPosition.z - 16.0f;
    float z1 = z0 + 32.0f;
    RemoveVegetationInRect(x0, z0, x1, z1);
    DisableCollisionObjectsInRect(x0, z0, x1, z1);
    resumeArmed = 0;
    g_TrackGame->ResetClock();
    return 1;
}

// 0x0045e520
void EventManager::ResetEntries() {
    for (int i = 0; i < 11; i++)
        results[i].Reset();
    racesRun = 0;
    g_TrackGame->mode.raceSettings.races = 0;
}

// 0x0045e550
void EventManager::WaitForRemoteRacers(float) {
    if (!waitingForRacers)
        return;
    int ready = 1;
    int local = g_TrackGame->network->localPlayer;
    for (int i = 0; i < g_TrackGame->mode.racerCount; i++) {
        int player = g_TrackGame->mode.racers[i].networkPlayerId;
        if (player != local) {
            NetPlayer* connected = g_TrackGame->network->FindPlayer(player);
            if (!g_TrackGame->mode.racers[i].isReady && connected)
                ready = 0;
        }
    }
    if (ready) {
        g_TrackGame->network->StopKeepAlive();
        waitingForRacers = 0;
        EndRace();
        if (g_TrackGame->networkGameObject)
            g_TrackGame->networkGameObject->ReportRaceToZone();
    }
}

// 0x0045e600
void EventManager::EndRace() {
    if (g_TrackGame->network && waitingForRacers)
        return;
    if (!g_TrackGame->uiInteractionBlocked && !g_TrackGame->podiumFinished) {
        RankRacers();
        EventBikeRaceView* view = FindRaceView();
        if (view)
            view->Enable();
        if (CreatePodiumScene()) {
            g_TrackGame->uiInteractionBlocked = 1;
            g_TrackGame->uiBlockedSeconds = 0;
            return;
        }
        g_TrackGame->podiumFinished = 1;
        g_TrackGame->uiInteractionBlocked = 0;
        return;
    }
    if (g_TrackGame->uiInteractionBlocked && !g_TrackGame->podiumFinished)
        return;
    if (!g_TrackGame->uiInteractionBlocked && g_TrackGame->podiumFinished) {
        g_TrackGame->podiumFinished = 0;
        if (g_TrackGame->network &&
            (g_TrackGame->mode.raceSettings.raceMode != 2 || racesRun > g_TrackGame->mode.raceSettings.races))
            g_TrackGame->network->SetSessionJoinable(1);
        waitingForRacers = 0;
        CloseRace(2);
        LeaveRaceForMenu(g_TrackGame->playerCount == 1 ? kMenu88e : kMenuMultiPlayer868);
    }
}

// The tool tip's on/off switch (ToolTip +0x5c).
static inline void EnableToolTips(KrustyUIToolTipView* tip, int enabled) {
    tip->enabled = enabled;
}

// 0x0045e710: leaves the race for `menu`: restores the UI, the window's
// input context and 640x480x16, and shows the transition dialog.
// The tool tip option goes through the inline setter: its parameter is a
// named local, so the option read takes no turn in VC6's eax/ecx/edx rotation
// of expression temporaries. Stored with a plain assignment the read lands in
// edx and every later scratch register is one step off retail's.
void EventManager::LeaveRaceForMenu(int menu) {
    if (!g_TrackGame->ui)
        return;
    UIDialog* item = g_TrackGame->ui->gui->FindInputDialog();
    if (item)
        item->EndDialog(0);
    if (g_TrackGame->ui->savedInputContext)
        ImmAssociateContext((HWND)g_TrackGame->windowHandle, (HIMC)g_TrackGame->ui->savedInputContext);
    if (g_TrackGame->ui->allowIme)
        g_TrackGame->ui->gui->EnableWindowClipper(1);
    g_TrackGame->uiInteractionBlocked = 0;
    g_TrackGame->podiumFinished = 0;
    g_TrackGame->uiBlockedSeconds = 0;
    if (!g_TrackGame->mode.cdAudioOn)
        g_TrackGame->StopCDAudio();
    DisplayMode* current = &g_TrackGame->display->displayModes[g_TrackGame->mode.displayModeIndex];
    if (current->width != 640 || current->height != 480 || current->bitDepth != 16)
        g_TrackGame->SwitchDisplayMode(
            g_TrackGame->display->FindDisplayMode(640, 480, 16, 0, 0));
    g_TrackGame->ui->gui->CreateBackground();
    KrustyUIGuiUser* layer = g_TrackGame->ui->gui->GetUser(0);
    EnableToolTips((KrustyUIToolTipView*)layer->userToolTip, g_TrackGame->mode.toolTips);
    if (!g_TrackGame->playingReplay && !g_TrackGame->ui->playingDemo &&
        g_TrackGame->mode.raceSettings.raceMode != 0 && g_TrackGame->mode.raceSettings.raceMode != 4)
        g_TrackGame->ui->RecordRaceResults();
    g_TrackGame->ui->gui->ShowCursors(1);
    TransDlg* dialog = new(__FILE__, 1064) TransDlg;
    g_TrackGame->ui->gui->ShowDialog(dialog, 0, 2, 0, 0, 0, 0, 1);
    dialog->SetNextMenu(menu);
    ((PCSoundInterface*)g_TrackGame->soundInterface)->SetPrimaryVolume(0);
}

// 0x0045e930
int CompareRankings(const void* a, const void* b) {
    const RacerTimeRank* first = (const RacerTimeRank*)a;
    const RacerTimeRank* second = (const RacerTimeRank*)b;
    if (first->value < second->value)
        return -1;
    if (first->value == second->value) {
        if (first->racer->lapCount > second->racer->lapCount)
            return -1;
        if (first->racer->lapCount == second->racer->lapCount) {
            if (first->racer->gatesPassed > second->racer->gatesPassed)
                return -1;
            if (first->racer->gatesPassed == second->racer->gatesPassed) {
                if (first->racer->statusNode->gateDistance < second->racer->statusNode->gateDistance)
                    return -1;
                if (first->racer->statusNode->gateDistance == second->racer->statusNode->gateDistance)
                    return 0;
            }
        }
    }
    return 1;
}

// 0x0045e9d0: ranks the racers by mode, copies them into the entries and
// awards points; in mode 2 it also orders the standings and copies each
// racer's place to its player's entry.
void EventManager::RankRacers() {
    EventBikeRaceView* view = FindRaceView();
    switch (g_TrackGame->mode.raceSettings.raceMode) { // a one-case switch: `sub eax, 2; jne`
    case 2:
        racesRun++;
    }
    int count = 0;
    int iterator = 0;
    EventKrustyBikeView* racer;
    switch (g_TrackGame->mode.raceSettings.eventType) {
    case 1:
    case 2:
    case 3:
    case 5: {
        if (g_TrackGame->playerCount > 1) {
            RacerTimeRank rankings[11];
            int ranked = 0;
            int next = 0;
            while ((racer = reinterpret_cast<EventKrustyBikeView*>(reinterpret_cast<BikeRace*>(view)->NextBike(&next))) != 0) {
                if (!racer->outOfRace) {
                    rankings[ranked].value = racer->totalRunningTime;
                    rankings[ranked].racer = racer;
                    ranked++;
                }
            }
            qsort(rankings, ranked, sizeof(rankings[0]), CompareRankings);
            for (int i = 0; i < ranked; i++)
                rankings[i].racer->finishPosition = i + 1;
        }
        iterator = 0;
        for (racer = reinterpret_cast<EventKrustyBikeView*>(reinterpret_cast<BikeRace*>(view)->NextBike(&iterator)); racer; racer = reinterpret_cast<EventKrustyBikeView*>(reinterpret_cast<BikeRace*>(view)->NextBike(&iterator))) {
            results[count].CopyFromRacer(racer);
            if ((g_TrackGame->mode.raceSettings.raceMode == 2 || g_TrackGame->proCircuit) &&
                !racer->outOfRace)
                AddChampionshipPoints(racer, &results[count].points);
            count++;
        }
        break;
    }
    case 0: {
        RacerScoreRank scores[11];
        int scored = 0;
        int next = 0;
        while ((racer = reinterpret_cast<EventKrustyBikeView*>(reinterpret_cast<BikeRace*>(view)->NextBike(&next))) != 0) {
            if (!racer->outOfRace) {
                scores[scored].value = racer->score;
                scores[scored].racer = racer;
                scored++;
            }
        }
        qsort(scores, scored, sizeof(scores[0]), CompareScoresDescending);
        for (int i = 0; i < scored; i++)
            scores[i].racer->finishPosition = i + 1;
        next = 0;
        for (racer = reinterpret_cast<EventKrustyBikeView*>(reinterpret_cast<BikeRace*>(view)->NextBike(&next)); racer; racer = reinterpret_cast<EventKrustyBikeView*>(reinterpret_cast<BikeRace*>(view)->NextBike(&next))) {
            results[count].CopyFromRacer(racer);
            if (g_TrackGame->mode.raceSettings.raceMode == 2 && !racer->outOfRace)
                AddChampionshipPoints(racer, &results[count].points);
            count++;
        }
        break;
    }
    case 4:
        if (g_TrackGame->mode.raceSettings.stuntMode) {
            RacerScoreRank scores[11];
            int scored = 0;
            int next = 0;
            while ((racer = reinterpret_cast<EventKrustyBikeView*>(reinterpret_cast<BikeRace*>(view)->NextBike(&next))) != 0) {
                if (!racer->outOfRace) {
                    scores[scored].value = racer->score;
                    scores[scored].racer = racer;
                    scored++;
                }
            }
            qsort(scores, scored, sizeof(scores[0]), CompareScoresDescending);
            for (int i = 0; i < scored; i++)
                scores[i].racer->finishPosition = i + 1;
            next = 0;
            for (racer = reinterpret_cast<EventKrustyBikeView*>(reinterpret_cast<BikeRace*>(view)->NextBike(&next)); racer; racer = reinterpret_cast<EventKrustyBikeView*>(reinterpret_cast<BikeRace*>(view)->NextBike(&next))) {
                results[count].CopyFromRacer(racer);
                if (g_TrackGame->mode.raceSettings.raceMode == 2 && !racer->outOfRace)
                    AddChampionshipPoints(racer, &results[count].points);
                count++;
            }
        } else {
            RacerScoreRank scores[11];
            int scored = 0;
            int next = 0;
            while ((racer = reinterpret_cast<EventKrustyBikeView*>(reinterpret_cast<BikeRace*>(view)->NextBike(&next))) != 0) {
                if (!racer->outOfRace) {
                    scores[scored].value = racer->totalTagRun;
                    scores[scored].racer = racer;
                    scored++;
                }
            }
            qsort(scores, scored, sizeof(scores[0]), CompareScoresDescending);
            for (int i = 0; i < scored; i++)
                scores[i].racer->finishPosition = i + 1;
            next = 0;
            for (racer = reinterpret_cast<EventKrustyBikeView*>(reinterpret_cast<BikeRace*>(view)->NextBike(&next)); racer; racer = reinterpret_cast<EventKrustyBikeView*>(reinterpret_cast<BikeRace*>(view)->NextBike(&next))) {
                results[count].CopyFromRacer(racer);
                if (g_TrackGame->mode.raceSettings.raceMode == 2 && !racer->outOfRace)
                    AddChampionshipPoints(racer, &results[count].points);
                count++;
            }
        }
        break;
    }
    resultCount = count;
    if (g_TrackGame->mode.raceSettings.raceMode == 2) {
        RacerStanding standings[11];
        int next = 0;
        int standing = 0;
        while ((racer = reinterpret_cast<EventKrustyBikeView*>(reinterpret_cast<BikeRace*>(view)->NextBike(&next))) != 0) {
            if (!racer->outOfRace) {
                standings[standing].points = results[standing].points;
                standings[standing].racer = racer;
                standing++;
            }
        }
        qsort(standings, standing, sizeof(standings[0]), CompareStandings);
        next = 0;
        while ((racer = reinterpret_cast<EventKrustyBikeView*>(reinterpret_cast<BikeRace*>(view)->NextBike(&next))) != 0) {
            int i;
            for (i = 0; i < standing; i++) {
                if (standings[i].racer == racer) {
                    racer->finishPosition = i + 1;
                    break;
                }
            }
            for (i = 0; i < standing; i++) {
                if (results[i].playerId == racer->networkPlayerId && results[i].aiIndex == racer->aiIndex) {
                    results[i].position = racer->finishPosition;
                    break;
                }
            }
        }
    }
}

// 0x0045eef0: decides when the race is over (the frame time argument is
// unused; Game+0x2f0 is read instead). After 315 seconds without an open
// menu item it always ends; otherwise by mode (TrackGame+0x2d74).
void EventManager::CheckRaceOver(float frameTime) {
    if (!g_TrackGame->ui->gui->FindInputDialog())
        g_EventTimeoutClock += g_TrackGame->frameTime;
    if (g_EventTimeoutClock >= 315.0f) {
        SendRacerResults();
        EndRace();
    }
    TrackGameViewOwner* owner = FindRaceMode();
    if (g_TrackGame->mode.raceSettings.raceMode == 0 || g_TrackGame->mode.raceSettings.raceMode == 4 || !owner ||
        !owner->isEnabled)
        return;
    EventBikeRaceView* view = FindRaceView();
    if (!view || !view->isRacing)
        return;
    switch (g_TrackGame->mode.raceSettings.eventType) {
    case 1:
    case 2:
    case 3:
    case 5:
        if (g_TrackGame->network) {
            anyRacerFinished = 0;
            allRacersFinished = 1;
            int iterator = 0;
            EventKrustyBikeView* racer;
            while ((racer = reinterpret_cast<EventKrustyBikeView*>(reinterpret_cast<BikeRace*>(view)->NextBike(&iterator))) != 0) {
                if (racer->hasFinished)
                    anyRacerFinished = 1;
                else if (!racer->outOfRace)
                    allRacersFinished = 0;
            }
            if (anyRacerFinished)
                endTimer += g_TrackGame->frameTime;
            // VC6 merges this call pair with the one after the switch.
            float limit = g_TrackGame->mode.raceSettings.shortTimeLimit ? 30.0f : 120.0f;
            if (allRacersFinished || limit < endTimer) {
                SendRacerResults();
                EndRace();
            }
            return;
        } else {
            EventKrustyBikeView* racer = view->localRacer;
            if (!racer->hasFinished)
                return;
            if (racer->finishPosition == 1) {
                if (racer->hasFinished >= 3)
                    EndRace();
                return;
            }
            if (racer->hasFinished >= 3 && ReadClock() - racer->finishTimeStamp >= 3000)
                EndRace();
            return;
        }
        break;
    case 0:
        if (g_TrackGame->quarryEvent->elapsedMinutes < g_TrackGame->mode.raceSettings.timeLimitMinutes)
            return;
        break;
    case 4:
        if (g_TrackGame->tagEvent->elapsedMinutes < g_TrackGame->mode.raceSettings.timeLimitMinutes)
            return;
        if (owner->followedRacer == owner->raceView->localRacer) {
            owner->followedRacer->totalTagRun += owner->followedRacer->tagRun;
            // Compared through locals, kept as the best run.
            float best = owner->followedRacer->bestTagRun;
            float lap = owner->followedRacer->tagRun;
            owner->followedRacer->bestTagRun = best > lap ? owner->followedRacer->bestTagRun
                                                        : owner->followedRacer->tagRun;
            owner->followedRacer->tagRun = 0;
        }
        break;
    default:
        return;
    }
    SendRacerResults();
    EndRace();
}

// 0x0045f180
void EventManager::AddChampionshipPoints(EventKrustyBikeView* racer, int* points) {
    int table[10];
    table[0] = 20;
    table[1] = 17;
    table[2] = 15;
    table[3] = 12;
    table[4] = 10;
    table[5] = 8;
    table[6] = 6;
    table[7] = 5;
    table[8] = 3;
    table[9] = 1;
    if (racer->finishPosition >= 1 && racer->finishPosition <= 10)
        *points += table[racer->finishPosition - 1];
}

// 0x0045f200: per-frame update. While UI interaction is blocked it advances
// the block timer, ticks the podium characters and pans the camera; after 7
// seconds it lifts the block.
int EventManager::Tick(float frameTime) {
    GameObject::Tick(frameTime);
    if (endNetworkGamePending) {
        EndNetworkGame();
        endNetworkGamePending = 0;
    }
    if (waitingForRacers)
        WaitForRemoteRacers(frameTime);
    else if (!g_TrackGame->playingReplay && !g_TrackGame->uiInteractionBlocked &&
             HasRaceMode())
        CheckRaceOver(frameTime);
    if (g_TrackGame->uiInteractionBlocked) {
        g_TrackGame->uiBlockedSeconds += frameTime;
        for (int i = 0; i < podiumCharacterCount; i++)
            podiumCharacters[i]->AdvanceMotionInterpolated(frameTime, 0, 0);
        podiumCameraPosition += frameTime * podiumPanSpeed * (1.0f / 7);
        podiumCamera->SetFrame(&podiumCameraPosition, 0, 0, 0, 0);
        podiumCamera->LookAt(podiumFocus);
        if (!g_TrackGame->podiumFinished && g_TrackGame->uiBlockedSeconds > 7.0f) {
            g_TrackGame->uiInteractionBlocked = 0;
            g_TrackGame->podiumFinished = 1;
            EndRace();
        }
    }
    return 1;
}

// 0x0045f3a0
int EventManager::OnControlRelease(ControlEvent* event, InputButtonState* entry) {
    if (GameObject::OnControlRelease(event, entry))
        return 1;
    if (g_TrackGame->uiInteractionBlocked && !g_TrackGame->network &&
        (event->kind == 2 || event->kind == 0 && event->control == 1 ||
         event->kind == 0 && event->control == 0x1c || event->kind == 0 && event->control == 0x39) &&
        resumeArmed) {
        resumeArmed = 0;
        g_TrackGame->uiInteractionBlocked = 0;
        g_TrackGame->podiumFinished = 1;
        EndRace();
        return 1;
    }
    return 0;
}

// 0x0045f440
int EventManager::OnControlPress(ControlEvent* event, InputButtonState* entry) {
    if (GameObject::OnControlPress(event, entry))
        return 1;
    if (g_TrackGame->uiInteractionBlocked)
        resumeArmed = 1;
    return 0;
}

// 0x0045f490: network messages. DPSYS_DESTROYPLAYERORGROUP and 0x89 mark a
// player done (and in mode 2 without a race-mode object remove it), 0x86
// updates a remote racer, 0xcc reports a player leaving (NetProcs sends it
// for a keep-alive timeout) and 0x8e the host ending the event.
int EventManager::OnNetworkMessage(int type, void* data, int from, int to, int flags) {
    if (GameObject::OnNetworkMessage(type, data, from, to, flags))
        return 1;
    UnknownEventPlayerMessage* message = (UnknownEventPlayerMessage*)data;
    char name[16];
    char text[128];
    char line[260];
    if (type == DPSYS_DESTROYPLAYERORGROUP) {
        for (int i = 0; i < g_TrackGame->mode.racerCount; i++) {
            if (g_TrackGame->mode.racers[i].networkPlayerId == message->dpId)
                g_TrackGame->mode.racers[i].isReady = 1;
        }
        if (g_TrackGame->mode.raceSettings.raceMode == 2 && !FindRaceMode())
            RemovePlayer(message->dpId);
    } else if (from) {
        if (type == 0x89) {
            for (int i = 0; i < g_TrackGame->mode.racerCount; i++) {
                if (g_TrackGame->mode.racers[i].networkPlayerId == from)
                    g_TrackGame->mode.racers[i].isReady = 1;
            }
            if (g_TrackGame->mode.raceSettings.raceMode == 2 && !FindRaceMode())
                RemovePlayer(from);
        } else if (type == 0x86) {
            RacerResultsMessage* update = (RacerResultsMessage*)data;
            EventBikeRaceView* view = FindRaceView();
            if (!view || g_TrackGame->uiInteractionBlocked)
                return 0;
            for (int i = 0; i < g_TrackGame->mode.racerCount; i++) {
                if (g_TrackGame->mode.racers[i].networkPlayerId == from &&
                    g_TrackGame->mode.racers[i].racerIndex == update->racerIndex) {
                    g_TrackGame->mode.racers[i].isReady = 1;
                    if (view->racerSlots[i]) {
                        view->racerSlots[i]->score = update->score;
                        view->racerSlots[i]->fastestLap = update->fastestLap;
                        view->racerSlots[i]->hasFinished = update->finished;
                        view->racerSlots[i]->largestStuntPoints = update->largestStuntPoints;
                        view->racerSlots[i]->totalTrickTime = update->totalTrickTime;
                        view->racerSlots[i]->bestTagRun = update->bestTagRun;
                        view->racerSlots[i]->totalTagRun = update->totalTagRun;
                        if (view->racerSlots[i]->hasFinished) {
                            view->racerSlots[i]->totalRunningTime = update->totalRunningTime;
                            // Always false here; retail still tests it.
                            if (!view->racerSlots[i]->hasFinished)
                                view->racerSlots[i]->finishTimeStamp = ReadClock();
                        } else {
                            view->racerSlots[i]->totalRunningTime = FLT_MAX;
                            view->racerSlots[i]->finishTimeStamp = 0x7ffffffe;
                        }
                    }
                }
            }
            if (keepAliveStarted)
                return 0;
            if (g_TrackGame->mode.raceSettings.eventType == 4) {
                TrackGameViewOwner* owner = FindRaceMode();
                if (owner->followedRacer == owner->raceView->localRacer) {
                    owner->followedRacer->totalTagRun += owner->followedRacer->tagRun;
                    // Compared through locals, kept as the best run.
                    float best = owner->followedRacer->bestTagRun;
                    float lap = owner->followedRacer->tagRun;
                    owner->followedRacer->bestTagRun = best > lap ? owner->followedRacer->bestTagRun
                                                                : owner->followedRacer->tagRun;
                    owner->followedRacer->tagRun = 0;
                }
            }
            SendRacerResults();
            EndRace();
        } else if (type == 0xcc) {
            TextQueueOverlay* target = g_TrackGame->eventManager->FindTextQueue();
            if (target) {
                g_TrackGame->LoadResourceString(0x13d7, text, sizeof(text));
                if (g_TrackGame->network->GetPlayerName(message->player, name)) {
                    sprintf(line, "%s %s", name, text);
                    TextQueueMessage notice(line, 3.25f);
                    target->ReplaceHeadMessage(&notice);
                }
            }
            if (message->player == g_TrackGame->network->localPlayer) {
                g_TrackGame->network->StopKeepAlive();
                endNetworkGamePending = 1;
            } else {
                for (int i = 0; i < g_TrackGame->mode.racerCount; i++) {
                    if (message->player == g_TrackGame->mode.racers[i].networkPlayerId) {
                        g_TrackGame->mode.racers[i].hasLeft = 1;
                        g_TrackGame->mode.racers[i].isReady = 1;
                    }
                }
            }
            EventBikeRaceView* view = g_TrackGame->eventManager->FindRaceView();
            if (view)
                reinterpret_cast<BikeRace*>(view)->HandlePlayerLeft(message->player);
        } else if (type == 0x8e) {
            if (message->player == g_TrackGame->network->localPlayer) {
                if (FindRaceMode()) {
                    TextQueueOverlay* target = FindTextQueue();
                    if (target) {
                        g_TrackGame->LoadResourceString(0x13d1, line, 128); // capped like `text`
                        TextQueueMessage notice(line, 3.25f);
                        target->ReplaceHeadMessage(&notice);
                    }
                } else {
                    ResetEntries();
                }
                endNetworkGamePending = 1;
            }
        }
    }
    return 0;
}

// 0x0045f9a0: once, sends the local racer's state and then each AI racer's
// (type 0x86), and starts the network wait.
void EventManager::SendRacerResults() {
    if (!g_TrackGame->network || keepAliveStarted)
        return;
    EventBikeRaceView* view = FindRaceView();
    if (!view->localRacer->hasFinished) {
        view->localRacer->totalRunningTime = FLT_MAX;
        view->localRacer->finishTimeStamp = 0x7ffffffe;
    }
    RacerResultsMessage message;
    message.score = view->localRacer->score;
    message.totalRunningTime = view->localRacer->totalRunningTime;
    message.fastestLap = view->localRacer->fastestLap;
    message.finished = view->localRacer->hasFinished;
    message.largestStuntPoints = view->localRacer->largestStuntPoints;
    message.totalTrickTime = view->localRacer->totalTrickTime;
    message.bestTagRun = view->localRacer->bestTagRun;
    message.totalTagRun = view->localRacer->totalTagRun;
    message.racerIndex = 0;
    g_TrackGame->network->Send(
        0x86, &message, sizeof(message), g_TrackGame->network->localPlayer, 0);
    for (int i = 0; i < g_TrackGame->mode.raceSettings.aiRacerCount; i++) {
        if (!view->aiRacers[i]->hasFinished) {
            view->aiRacers[i]->totalRunningTime = FLT_MAX;
            view->aiRacers[i]->finishTimeStamp = 0x7ffffffe;
        }
        message.score = view->aiRacers[i]->score;
        message.totalRunningTime = view->aiRacers[i]->totalRunningTime;
        message.fastestLap = view->aiRacers[i]->fastestLap;
        message.finished = view->aiRacers[i]->hasFinished;
        message.largestStuntPoints = view->aiRacers[i]->largestStuntPoints;
        message.totalTrickTime = view->aiRacers[i]->totalTrickTime;
        message.bestTagRun = view->aiRacers[i]->bestTagRun;
        message.totalTagRun = view->aiRacers[i]->totalTagRun;
        message.racerIndex = view->aiRacers[i]->aiIndex;
        g_TrackGame->network->Send(
            0x86, &message, sizeof(message), g_TrackGame->network->localPlayer, 0);
    }
    keepAliveStarted = 1;
    g_TrackGame->network->StartKeepAlive(keepAliveTimeout, keepAliveInterval);
    view->Disable();
    waitingForRacers = 1;
}

// 0x0045fbb0
int CompareUnsigned(const void* a, const void* b) {
    unsigned int first = *(const unsigned int*)a;
    unsigned int second = *(const unsigned int*)b;
    if (first < second)
        return -1;
    return first != second;
}

// 0x0045fbd0: removes `player`, moving the last entry and record into its
// place, then re-sorts the entries.
void EventManager::RemovePlayer(int player) {
    if (g_TrackGame->playerCount < 2)
        return;
    for (int i = 0; i < g_TrackGame->mode.racerCount; i++) {
        if (results[i].playerId == player) {
            if (!results[i].aiIndex)
                g_TrackGame->playerCount--;
            else
                g_TrackGame->networkAiRacerCount--;
            results[i] = results[g_TrackGame->mode.racerCount - 1];
            if (FindRaceMode())
                g_TrackGame->mode.racers[i] =
                    g_TrackGame->mode.racers[g_TrackGame->mode.racerCount - 1];
            g_TrackGame->mode.raceSettings.networkPlayerCount--;
        }
    }
    if (g_TrackGame->mode.racerCount)
        qsort(results, g_TrackGame->mode.racerCount - 1, sizeof(RacerResult), CompareUnsigned);
}

// The quadtree members 0x0045fce0 and 0x0045fdc0 use the krusty2 QuadTree,
// QuadTreeObject and CollisionObject (through ArcadeObject.h's Wrecker.h).
// RTTI: Vegetation : QuadTreeObject (vtable 0x005524f8); Vegetation.h
// declares it over EcoSystem's quadtree view, so the cast here names a
// local Vegetation on the krusty2 base. It overrides ComputeQuadTreeCode
// (0x00456720, ret 4) with the object's quadtree code.
class Vegetation : public QuadTreeObject {};

// 0x0045fce0
void EventManager::RemoveVegetationInRect(float x0, float z0, float x1, float z1) {
    struct {
        Vegetation* object;
        unsigned int code;
    } found[1000];
    int count = 0;
    if (g_collisionQuadTree && g_collisionQuadTree->BeginQuery(x0, z0, x1, z1)) {
        QuadTreeObject* object;
        while ((object = g_collisionQuadTree->NextObject()) != 0) {
            if (count < 1000) {
                found[count].object = dynamic_cast<Vegetation*>(object);
                if (found[count].object) {
                    found[count].code = found[count].object->ComputeQuadTreeCode(g_collisionQuadTree);
                    count++;
                }
            }
        }
        g_collisionQuadTree->EndQuery();
        for (int i = 0; i < count; i++)
            g_collisionQuadTree->Remove(found[i].object, found[i].code);
    }
}

// 0x0045fdc0
void EventManager::DisableCollisionObjectsInRect(float x0, float z0, float x1, float z1) {
    if (g_collisionQuadTree && g_collisionQuadTree->BeginQuery(x0, z0, x1, z1)) {
        QuadTreeObject* object;
        while ((object = g_collisionQuadTree->NextObject()) != 0) {
            GameObject* graphics = dynamic_cast<CollisionObject*>(object);
            if (graphics)
                graphics->Disable();
        }
        g_collisionQuadTree->EndQuery();
    }
}
