#include "../../src/reconstructed/GUIManagerCore.h"
// Near-miss EventManager candidates, kept out of src/reconstructed until
// they match. See docs/EVENTMANAGER.md.
//
// EventManager::LeaveRaceForMenu (0x0045e710, 531 bytes): leaves the
// race for a menu. Control flow, calls, the TransDlg `new` (retail line 1064)
// and its EH state match; 454 of 533 bytes. After the `new` retail loads the
// global into eax (the short form, 2 bytes less) and the menu into edx where
// VC6 here picks ecx and eax; it also reuses ecx for TrackGame+0xc4c. Local,
// base-pointer, assignment-in-argument and declaration-placement forms do
// not change it; nor do (40-form sweep) the layer store through GetUser
// directly, a layer-item local or a value local, a GUI local for
// ShowDialog or the `new` assigned inside the ShowDialog call.
//
// EventManager::CreatePodiumScene (0x0045d480, 4247 bytes): the podium
// scene; 4139 of 4247 bytes (bindings: EventManagerNearMisses.bindings.json,
// with the EH handler 0x0054ab22 the prologue pushes). Control flow, calls,
// strings, `new` lines, EH states and the frame size (0x460) follow retail.
// Shapes found: the last-race test assigned to `ok` (retail materialises the
// sete), separate `column`/`row` products, the zero direction test through
// an inline helper (it places the copy branch first), PodiumLength over a
// reference (no by-value copy) with `xx` and `yy` locals and `v.z * v.z`
// left in the sum (retail's `fld; fld st(0); fmulp` per component in the
// offsets/facings loop and the plain camera sites; plain, all-local,
// component-local, Square-helper, dot-product and three-float-helper forms
// load all three components first, and a lone `zz` local is 6 bytes longer),
// the nested InProCircuitCareer test (three Winner sprintf copies), and the
// buffers name[0x80], path[0x104], message[0x184] plus the crowd message's
// own [0x80] buffer at +0x168. Left: retail spills the z square to the
// temporary [esp+0x1c] (`fstp [t]; fstp st(0); faddp; fadd [t]`, as BikeRace
// 0x00417ed0 does) where VC6 here keeps it on the stack (`faddp st(3); fxch;
// fadd st(1); fxch` and two pops; the 11 sum, bracket, order and
// declaration forms of 2026-10-19 do not spill), so the scalar homes at
// +0x1c/+0x20 swap; the register rotation of the Winner branch (one step
// behind retail), the place counter store (retail stores 1 after the count
// test), the z component order of `podiumPosition + offsets[place - 1]` and
// the loop's two increments.
#include <math.h>
#include <stdio.h>
#include <stdlib.h>

#include "../../src/reconstructed/DebugAlloc.h"
#include "../../src/reconstructed/GameUi.h"
#include "../../src/reconstructed/SoundInterface.h"
#include "../../src/reconstructed/TrackGame.h"
#include "../../src/reconstructed/EventManager.h"
#include "../../src/reconstructed/UIDialog.h"
#include "../../src/reconstructed/PCCamera.h"
#include "../../src/reconstructed/TextureMap.h"
#include "../../src/reconstructed/UnknownResourceManager.h"

// EventManager.cpp's per-file vectors (0x0059af48, 0x0059af18).
static Vector3 kVec3YAxis = Vector3(0.0f, 1.0f, 0.0f);
static Vector3 kVec3ZAxis = Vector3(0.0f, 0.0f, 1.0f);

// IMM32, called through the linker's import thunk.
extern "C" void* __stdcall ImmAssociateContext(void* window, void* context);

#include "LeaveRaceForMenu.cpp"

// Views for the podium scene 0x0045d480. Only what it calls is declared.
struct UnknownPodiumModel {                         // the view part's +0xa4
    unsigned char field_0x000[0x398];
    float racerCount;                               // +0x398
};
struct UnknownPodiumGrid {                          // TrackGameViewOwner+0x2c
    unsigned char field_0x00[0x8c];
    Vector3 position;                               // +0x8c podium position
    Vector3 direction;                              // +0x98 podium direction (0 when none)
    UnknownPodiumModel* model;                      // +0xa4
    unsigned char field_0xa8[0xc4 - 0xa8];
    GameObject* soundGroup;                         // +0xc4
};
struct UnknownPodiumOwner : public GameObject {     // TrackGameViewOwner
    UnknownPodiumGrid* grid;                        // +0x2c
    unsigned char field_0x30[0x34 - 0x30];
    UnknownKrustyBikeView* raceView;                // +0x34
    unsigned char field_0x38[0x7c - 0x38];
    GameObject* lights;                             // +0x7c
    unsigned char field_0x80[0x94 - 0x80];
    int opaqueTextureGroup;                         // +0x94, as BaseQuarryEvent.h
    int alphaTextureGroup;                          // +0x98
};
struct UnknownPodiumTrackObject {                   // the view's +0x4c
    void QueryGround(Vector3* point, int a, int b, int c); // 0x00507c10
    unsigned char field_0x00[0x40];
    float gridSpacing;                              // +0x40
};
struct UnknownPodiumNode {
    void GetPosition(Vector3* position);              // 0x004fc970
    void GetAxes(Vector3* direction, Vector3* up); // 0x004fc4f0
};
struct UnknownPodiumNodeOwner {                     // the view's +0x64
    unsigned char field_0x000[0x1a0];
    UnknownPodiumNode* modelNode;                   // +0x1a0 the characters' model node
};
struct UnknownPodiumViewFields {
    unsigned char field_0x00[0x4c];
    UnknownPodiumTrackObject* trackObject;          // +0x4c
    unsigned char field_0x50[0x64 - 0x50];
    UnknownPodiumNodeOwner* nodeOwner;              // +0x64
};
struct UnknownPodiumRacerModel {                    // racer +0x5c4
    unsigned char field_0x000[0x1a0];
    UnknownPodiumSoultree* modelNode;             // +0x1a0
};
struct UnknownPodiumRacer {
    unsigned char field_0x000[0x5c4];
    UnknownPodiumRacerModel* riderCharacter;      // +0x5c4
    unsigned char field_0x5c8[0x784 - 0x5c8];
    int place;                                      // +0x784
};
// Game+0x1c's resource context, as the podium characters' slot 11 takes it
// (laid out like TextureMap.h's UnknownTextureFormatChoice).
struct UnknownPodiumContext {
    TextureMapManager* textureManager;              // +0x00
    int opaqueGroup;                                // +0x04
    int alphaGroup;                                 // +0x08
    int opaqueFormat;                               // +0x0c
    int alphaFormat;                                // +0x10
    int halvings;                                   // +0x14
};
class UnknownPodiumSound {                          // PCAudio.h's Sound
public:
    UnknownPodiumSound(GameObject* group, int type);                    // 0x004bba10
    int Load(const char* name, UnknownTextureStream* stream, int flags, int a,
                              int b, int c);                            // 0x004bc320
    int PlayWithOptions(int restart, unsigned long playFlags, int preferHardware); // 0x004bc6b0
    unsigned char field_0x000[0x1f8];
};
class UnknownPodiumArcade {                         // ArcadeObject (+0x3d0)
public:
    virtual void UnknownVirtualSlot0();
    virtual void UnknownVirtualSlot1();
    virtual void UnknownVirtualSlot2();
    virtual void UnknownVirtualSlot3();
    virtual void UnknownVirtualSlot4();
    virtual void UnknownVirtualSlot5();
    void SetPosition(const Vector3* position);                // 0x004014f0
    void SetAxes(const Vector3* a, const Vector3* b, int c, int d); // 0x00401520
    unsigned char field_0x04[0x2c - 0x4];
    UnknownPodiumSoultree* soultree;                // +0x2c
};

extern "C" Vector3* __stdcall D3DRMVectorRotate(Vector3* result, Vector3* vector, Vector3* axis,
                                                float theta);

// |v|, exact for unit vectors.
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

#define PODIUM_OWNER(o) ((UnknownPodiumOwner*)(o))
#define PODIUM_VIEW(v) ((UnknownPodiumViewFields*)(v))
#define PODIUM_ARCADE ((UnknownPodiumArcade*)podiumObject)
#define PODIUM_MODEL(c) ((UnknownPodiumSoultree*)(c)->modelNode)

// Whether v is the zero vector (the grid's "no podium direction").
static inline int PodiumIsZero(const Vector3& v) {
    return v.x == 0.0f && v.y == 0.0f && v.z == 0.0f;
}

// Whether the career's current race is its series' last (the bonus track
// is the last when there is one).
static inline int PodiumIsLastRace(UnknownTrackGameObject3444* circuit) {
    int last;
    if (circuit->schedules[circuit->series].bonusTrack)
        last = circuit->schedules[circuit->series].bonusTrack - 1;
    else
        last = circuit->schedules[circuit->series].trackCount;
    return circuit->raceIndex == last;
}

// 0x0045d480
int EventManager::CreatePodiumScene() {
    char name[0x80];
    char path[0x104];
    char message[0x184];
    UnknownTrackGameObject3444* circuit = g_TrackGame->proCircuit;
    TrackGameViewOwner* owner = FindRaceMode();
    UnknownKrustyBikeView* view = FindRaceView();
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
    UnknownPodiumGrid* grid = PODIUM_OWNER(owner)->grid;
    float rows = grid->model->racerCount;
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
            PODIUM_VIEW(PODIUM_OWNER(owner)->raceView)->nodeOwner->modelNode->GetPosition(&podiumPosition);
            PODIUM_VIEW(PODIUM_OWNER(owner)->raceView)->nodeOwner->modelNode->GetAxes(&direction, &up);
        } else {
            float spacing = PODIUM_VIEW(PODIUM_OWNER(owner)->raceView)->trackObject->gridSpacing;
            podiumPosition = Vector3((int)column * spacing * 256.0f, 0.0f, (int)row * spacing * 256.0f);
            direction = kVec3ZAxis;
        }
    }
    up = kVec3YAxis;
    PODIUM_VIEW(PODIUM_OWNER(owner)->raceView)->trackObject->QueryGround(&podiumPosition, 0, 0, 0);
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
    UnknownPodiumContext context;
    context.textureManager = g_TrackGame->textureManager;
    context.opaqueGroup = PODIUM_OWNER(owner)->opaqueTextureGroup;
    context.alphaGroup = PODIUM_OWNER(owner)->alphaTextureGroup;
    context.opaqueFormat = ((RenderTarget*)renderContext)->surfacePixelFormat;
    context.alphaFormat = 0x115c;
    context.halvings = 0;
    PODIUM_ARCADE->UnknownVirtualSlot5();
    PODIUM_ARCADE->SetPosition(&podiumPosition);
    PODIUM_ARCADE->SetAxes(&direction, &up, 0, 1);
    if (PODIUM_OWNER(owner)->lights && PODIUM_OWNER(owner)->lights->isEnabled)
        PODIUM_ARCADE->soultree->SetUseLighting(1);
    UnknownTextureStream* stream = new(__FILE__, 0x2f5) UnknownTextureStream(g_ResourceManager);
    podiumCharacterCount = resultCount < 3 ? resultCount : 3;
    float chance = rand() * (1.0f / 32768);
    for (int place = 1; place - 1 < podiumCharacterCount; place++) {
        UnknownEventRacer* racer = 0;
        iterator = 0;
        racer = PODIUM_OWNER(owner)->raceView->NextBike(&iterator);
        while (racer && ((UnknownPodiumRacer*)racer)->place != place)
            racer = PODIUM_OWNER(owner)->raceView->NextBike(&iterator);
        if (g_TrackGame->InProCircuitCareer()) {
            if ((g_TrackGame->proCircuit->careerFlags & 2) &&
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
        character->D3DIMVirtualSlot11((int)renderContext, path, (const SoultreeLoadDesc*)PODIUM_OWNER(owner)->lights,
                                       (int)&context, 1, 1);
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
            PODIUM_MODEL(character)->SetPosition(&position);
            PODIUM_MODEL(character)->SetAxesPtr(&facings[place - 1], &kVec3YAxis, 1, 1);
            character->SetMotion(podiumMotions[place - 1]);
            character->chr_field_0x10 = 0.0f;
            if (racer) {
                UnknownPodiumTextureEntry** texture = ((UnknownPodiumRacer*)racer)->riderCharacter->modelNode->field_0x290;
                if (texture)
                    PODIUM_MODEL(character)->ReplaceMaterialTexture(0, (*texture)->field_0x2c, &context);
            }
        }
    }
    delete stream;
    if (g_TrackGame->mode.soundOn && g_TrackGame->mode.networkGame) {
        UnknownPodiumSound* sound = new(__FILE__, 0x34e) UnknownPodiumSound(PODIUM_OWNER(owner)->grid->soundGroup, 1);
        UnknownTextureStream* audio = new(__FILE__, 0x34f) UnknownTextureStream(g_ResourceManager);
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
