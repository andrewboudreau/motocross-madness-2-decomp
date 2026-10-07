// Near-miss EventManager candidates, kept out of src/reconstructed until
// they match. See docs/EVENTMANAGER.md.
//
// EventManager::UnknownFunction45e710 (0x0045e710, 531 bytes): leaves the
// race for a menu. Control flow, calls, the TransDlg `new` (retail line 1064)
// and its EH state match; 454 of 533 bytes. After the `new` retail loads the
// global into eax (the short form, 2 bytes less) and the menu into edx where
// VC6 here picks ecx and eax; it also reuses ecx for TrackGame+0xc4c. Local,
// base-pointer, assignment-in-argument and declaration-placement forms do
// not change it.
//
// EventManager::CreatePodiumScene (0x0045d480, 4247 bytes): the podium
// scene. A first full draft: control flow, calls, strings, `new` lines and
// EH states follow retail, but about 500 instructions still differ: the
// frame (retail 0x460, three text buffers at +0xe8/+0x1e8/+0x2ec and a
// second message at +0x168), the zero-direction test's block order, the
// +0x3444 last-race test (retail materialises a sete), and the rotations'
// scaling temporaries. The views it needs are declared below.
#include <math.h>
#include <stdio.h>
#include <stdlib.h>

#include "../../src/reconstructed/DebugAlloc.h"
#include "../../src/reconstructed/GameUi.h"
#include "../../src/reconstructed/SoundInterface.h"
#include "../../src/reconstructed/TrackGame.h"
#include "../../src/reconstructed/UIDialog.h"
#include "../../src/reconstructed/PCCamera.h"
#include "../../src/reconstructed/TextureMap.h"
#include "../../src/reconstructed/UnknownResourceManager.h"

// EventManager.cpp's per-file vectors (0x0059af48, 0x0059af18).
static Vector3 kVec3YAxis = Vector3(0.0f, 1.0f, 0.0f);
static Vector3 kVec3ZAxis = Vector3(0.0f, 0.0f, 1.0f);

// IMM32, called through the linker's import thunk.
extern "C" void* __stdcall ImmAssociateContext(void* window, void* context);

// 0x0045e710: leaves the race for `menu`: restores the UI, the window's
// input context and 640x480x16, and shows the transition dialog.
void EventManager::UnknownFunction45e710(int menu) {
    if (!g_TrackGame->ui)
        return;
    UnknownTrackGameObject56cItem* item = g_TrackGame->ui->field_0x2c->FindInputDialog();
    if (item)
        item->UnknownFunction46ff30(0);
    if (g_TrackGame->ui->field_0x494)
        ImmAssociateContext(g_TrackGame->field_0x31c, g_TrackGame->ui->field_0x494);
    if (g_TrackGame->ui->field_0x498)
        g_TrackGame->ui->field_0x2c->EnableWindowClipper(1);
    g_TrackGame->uiInteractionBlocked = 0;
    g_TrackGame->field_0x3438 = 0;
    g_TrackGame->field_0x3434 = 0;
    if (!g_TrackGame->mode.field_0xa20)
        g_TrackGame->UnknownFunction521a40();
    UnknownDisplayMode* current = &g_TrackGame->display->field_0x10[g_TrackGame->mode.field_0xa4c];
    if (current->width != 640 || current->height != 480 || current->bitDepth != 16)
        g_TrackGame->UnknownVirtualSlot19(
            g_TrackGame->display->UnknownFunction52d250(640, 480, 16, 0, 0));
    g_TrackGame->ui->field_0x2c->CreateBackground();
    UnknownKrustyUIGuiLayer* layer = g_TrackGame->ui->field_0x2c->GetUser(0);
    layer->field_0xc0->field_0x5c = g_TrackGame->mode.field_0x6d4;
    if (!g_TrackGame->field_0x3428 && !g_TrackGame->ui->field_0x4a8 &&
        g_TrackGame->mode.field_0x27f8.field_0x00 != 0 && g_TrackGame->mode.field_0x27f8.field_0x00 != 4)
        g_TrackGame->ui->UnknownFunction49bbb0();
    g_TrackGame->ui->field_0x2c->ShowCursors(1);
    TransDlg* dialog = new(__FILE__, 1064) TransDlg;
    g_TrackGame->ui->field_0x2c->ShowDialog(dialog, 0, 2, 0, 0, 0, 0, 1);
    dialog->SetNextMenu(menu);
    ((PCSoundInterface*)g_TrackGame->soundInterface)->UnknownFunction4be9b0(0);
}

// Views for the podium scene 0x0045d480. Only what it calls is declared.
struct UnknownPodiumModel {                         // the view part's +0xa4
    unsigned char field_0x000[0x398];
    float field_0x398;                              // racer count
};
struct UnknownPodiumGrid {                          // TrackGameViewOwner+0x2c
    unsigned char field_0x00[0x8c];
    Vector3 field_0x8c;                             // podium position
    Vector3 field_0x98;                             // podium direction (0 when none)
    UnknownPodiumModel* field_0xa4;
    unsigned char field_0xa8[0xc4 - 0xa8];
    GameObject* field_0xc4;                         // sound group
};
struct UnknownPodiumOwner : public GameObject {     // TrackGameViewOwner
    UnknownPodiumGrid* field_0x2c;
    unsigned char field_0x30[0x34 - 0x30];
    UnknownKrustyBikeView* field_0x34;
    unsigned char field_0x38[0x7c - 0x38];
    GameObject* field_0x7c;                         // lights
    unsigned char field_0x80[0x94 - 0x80];
    int field_0x94;
    int field_0x98;
};
struct UnknownPodiumTrackObject {                   // the view's +0x4c
    void UnknownFunction507c10(Vector3* point, int a, int b, int c); // 0x00507c10
    unsigned char field_0x00[0x40];
    float field_0x40;                               // grid spacing
};
struct UnknownPodiumNode {
    void UnknownFunction4fc970(Vector3* position);              // 0x004fc970
    void UnknownFunction4fc4f0(Vector3* direction, Vector3* up); // 0x004fc4f0
};
struct UnknownPodiumNodeOwner {                     // the view's +0x64
    unsigned char field_0x000[0x1a0];
    UnknownPodiumNode* field_0x1a0;
};
struct UnknownPodiumViewFields {
    unsigned char field_0x00[0x4c];
    UnknownPodiumTrackObject* field_0x4c;
    unsigned char field_0x50[0x64 - 0x50];
    UnknownPodiumNodeOwner* field_0x64;
};
struct UnknownPodiumRacerModel {                    // racer +0x5c4
    unsigned char field_0x000[0x1a0];
    UnknownPodiumSoultree* field_0x1a0;
};
struct UnknownPodiumRacer {
    unsigned char field_0x000[0x5c4];
    UnknownPodiumRacerModel* field_0x5c4;
    unsigned char field_0x5c8[0x784 - 0x5c8];
    int field_0x784;                                // place
};
// Game+0x1c's resource context, as the podium characters' slot 11 takes it.
struct UnknownPodiumContext {
    TextureMapManager* field_0x00;
    int field_0x04;
    int field_0x08;
    int field_0x0c;
    int field_0x10;
    int field_0x14;
};
class UnknownPodiumSound {                          // PCAudio.h's Sound
public:
    UnknownPodiumSound(GameObject* group, int type);                    // 0x004bba10
    int UnknownFunction4bc320(const char* name, UnknownTextureStream* stream, int flags, int a,
                              int b, int c);                            // 0x004bc320
    int UnknownFunction4bc6b0(int restart, unsigned long playFlags, int preferHardware); // 0x004bc6b0
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
    void UnknownFunction4014f0(const Vector3* position);                // 0x004014f0
    void UnknownFunction401520(const Vector3* a, const Vector3* b, int c, int d); // 0x00401520
    unsigned char field_0x04[0x2c - 0x4];
    UnknownPodiumSoultree* field_0x2c;
};

extern "C" Vector3* __stdcall D3DRMVectorRotate(Vector3* result, Vector3* vector, Vector3* axis,
                                                float theta);

// |v|, exact for unit vectors.
static inline float PodiumLength(Vector3 v) {
    float squared = v.x * v.x + v.y * v.y + v.z * v.z;
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

// Whether the career's current race is its series' last (the bonus track
// is the last when there is one).
static inline int PodiumIsLastRace(UnknownTrackGameObject3444* circuit) {
    int last;
    if (circuit->field_0x1285[circuit->field_0x40].field_0x08)
        last = circuit->field_0x1285[circuit->field_0x40].field_0x08 - 1;
    else
        last = circuit->field_0x1285[circuit->field_0x40].field_0x04;
    return circuit->field_0x44 == last;
}

// 0x0045d480
int EventManager::CreatePodiumScene() {
    char name[256];
    char path[260];
    char message[388];
    UnknownTrackGameObject3444* circuit = g_TrackGame->field_0x3444;
    TrackGameViewOwner* owner = FindRaceMode();
    UnknownKrustyBikeView* view = FindRaceView();
    int iterator = 0;
    int ok;
    if (g_TrackGame->mode.field_0x27f8.field_0x00 == 2)
        ok = field_0x48 > g_TrackGame->mode.field_0x27f8.field_0x0c;
    else
        ok = 1;
    if (circuit) {
        if (!PodiumIsLastRace(circuit))
            return 0;
    } else {
        if (!ok || !*(int*)g_TrackGame->mode.field_0x6c4)
            return 0;
        if (g_TrackGame->mode.field_0x27f8.field_0x00 != 2 && !g_TrackGame->network &&
            field_0x50[0].field_0x04 > 3)
            return 0;
    }
    UnknownFunction45cdc0(1);
    PCCamera* camera = new(__FILE__, 0x276) PCCamera(1);
    podiumCamera = (Camera*)camera->UnknownVirtualSlot8(field_0x18);
    if (!view->AppendChild(podiumCamera, -1))
        return 0;
    ((RenderTarget*)field_0x18)->UnknownFunction4e8cf0(podiumCamera);
    podiumCamera->UnknownFunction469260(view->field_0x50, -1);
    UnknownPodiumGrid* grid = PODIUM_OWNER(owner)->field_0x2c;
    float rows = grid->field_0xa4->field_0x398;
    if (!((int)rows & 1))
        rows -= 1.0f;
    float column;
    float row;
    column = row = rows * 0.5f;
    Vector3 direction;
    Vector3 up;
    if (grid->field_0x98.x == 0.0f && grid->field_0x98.y == 0.0f && grid->field_0x98.z == 0.0f) {
        if (g_TrackGame->mode.field_0x27f8.field_0x04 == 3 && PODIUM_VIEW(PODIUM_OWNER(owner)->field_0x34)->field_0x64) {
            PODIUM_VIEW(PODIUM_OWNER(owner)->field_0x34)->field_0x64->field_0x1a0->UnknownFunction4fc970(&field_0x3c4);
            PODIUM_VIEW(PODIUM_OWNER(owner)->field_0x34)->field_0x64->field_0x1a0->UnknownFunction4fc4f0(&direction, &up);
        } else {
            float spacing = PODIUM_VIEW(PODIUM_OWNER(owner)->field_0x34)->field_0x4c->field_0x40;
            field_0x3c4 = Vector3((int)column * spacing * 256.0f, 0.0f, (int)row * spacing * 256.0f);
            direction = kVec3ZAxis;
        }
    } else {
        field_0x3c4 = grid->field_0x8c;
        direction = PODIUM_OWNER(owner)->field_0x2c->field_0x98;
    }
    up = kVec3YAxis;
    PODIUM_VIEW(PODIUM_OWNER(owner)->field_0x34)->field_0x4c->UnknownFunction507c10(&field_0x3c4, 0, 0, 0);
    float angle = (float)atan2(direction.x, direction.z);
    Vector3 offsets[3];
    Vector3 facings[3];
    Vector3 cameraOffset;
    Vector3 lookOffset;
    int kind = g_TrackGame->mode.UnknownFunction524100();
    if (kind >= 2 && kind <= 4) {
        offsets[0] = Vector3(0.0f, 0.383f, 0.0f);
        offsets[1] = Vector3(0.0f, 0.383f, 0.0f);
        offsets[2] = Vector3(0.0f, 0.383f, 0.0f);
        facings[0] = Vector3(0.0f, 0.0f, -1.0f);
        facings[1] = Vector3(0.0f, 0.0f, -1.0f);
        facings[2] = Vector3(0.0f, 0.0f, -1.0f);
        field_0x3c4.y += 1.5f;
        cameraOffset = Vector3(-10.0f, 15.0f, -20.0f);
        lookOffset = Vector3(5.0f, 9.0f, -10.0f);
        field_0x3d8 = field_0x3c4;
        field_0x3d8.y += 6.0f;
    } else {
        offsets[0] = Vector3(0.0f, 3.4f, -6.0f);
        offsets[1] = Vector3(-11.0f, 3.4f, -7.0f);
        offsets[2] = Vector3(10.0f, 3.4f, -9.0f);
        facings[0] = Vector3(0.0f, 0.0f, -1.0f);
        facings[1] = Vector3((float)sin(2.967059388756752), 0.0f, (float)cos(2.967059388756752));
        facings[2] = Vector3((float)sin(-2.617993578314781), 0.0f, (float)cos(-2.617993578314781));
        cameraOffset = Vector3(-25.0f, 7.0f, -15.0f);
        lookOffset = Vector3(20.0f, 13.0f, -28.0f);
        field_0x3d8 = field_0x3c4;
        field_0x3d8.y += 6.0f;
        field_0x3d8.z -= 6.0f;
    }
    Vector3 rotated;
    for (int i = 0; i < 3; i++) {
        D3DRMVectorRotate(&rotated, &offsets[i], &kVec3YAxis, angle);
        offsets[i] = rotated * PodiumLength(offsets[i]);
        D3DRMVectorRotate(&rotated, &facings[i], &kVec3YAxis, angle);
        facings[i] = rotated * PodiumLength(facings[i]);
    }
    UnknownPodiumContext context;
    context.field_0x00 = g_TrackGame->field_0x3c;
    context.field_0x04 = PODIUM_OWNER(owner)->field_0x94;
    context.field_0x08 = PODIUM_OWNER(owner)->field_0x98;
    context.field_0x0c = ((RenderTarget*)field_0x18)->field_0x28;
    context.field_0x10 = 0x115c;
    context.field_0x14 = 0;
    PODIUM_ARCADE->UnknownVirtualSlot5();
    PODIUM_ARCADE->UnknownFunction4014f0(&field_0x3c4);
    PODIUM_ARCADE->UnknownFunction401520(&direction, &up, 0, 1);
    if (PODIUM_OWNER(owner)->field_0x7c && PODIUM_OWNER(owner)->field_0x7c->field_0x25_bit0)
        PODIUM_ARCADE->field_0x2c->UnknownFunction4444c0(1);
    UnknownTextureStream* stream = new(__FILE__, 0x2f5) UnknownTextureStream((int)g_UnknownResourceManager572b44);
    podiumCharacterCount = field_0x4c < 3 ? field_0x4c : 3;
    float chance = rand() * (1.0f / 32768);
    for (int place = 1; place - 1 < podiumCharacterCount; place++) {
        UnknownEventRacer* racer = 0;
        iterator = 0;
        racer = PODIUM_OWNER(owner)->field_0x34->UnknownFunction4204e0(&iterator);
        while (racer && ((UnknownPodiumRacer*)racer)->field_0x784 != place)
            racer = PODIUM_OWNER(owner)->field_0x34->UnknownFunction4204e0(&iterator);
        if (g_TrackGame->UnknownFunction521cd0() &&
            (g_TrackGame->field_0x3444->field_0x464 & 2) &&
            racer == PODIUM_OWNER(owner)->field_0x34->field_0x38)
            sprintf(name, "%s\\Winnerd.mcf", "Res");
        else
            sprintf(name, "%s\\Winner.mcf", "Res");
        if (!g_TrackGame->sceneObject->UnknownFunction4e9cd0(stream, name, "rb", (int)path)) {
            sprintf(message, "No winner animation file found in resources.  Aborting podium scene.");
            delete stream;
            return 0;
        }
        D3DIMSoultreeCharacter*& character = podiumCharacters[place - 1];
        character = new(__FILE__, 0x31d) D3DIMSoultreeCharacter(field_0x25_bit0);
        character->CharacterVirtualSlot11(field_0x18, path, PODIUM_OWNER(owner)->field_0x7c, &context, 1, 1);
        view->AppendChild(character, -1);
        if (character) {
            if (PODIUM_OWNER(owner)->field_0x7c && PODIUM_OWNER(owner)->field_0x7c->field_0x25_bit0)
                character->field_0x1a0->UnknownFunction4444c0(1);
            int mode = g_TrackGame->mode.UnknownFunction524100();
            if (mode == 3 || mode == 2 || mode == 4) {
                if (chance < 0.5f && podiumCharacterCount != 1)
                    sprintf(name, "Podium3_%02d", place);
                else
                    sprintf(name, "Podium5_%02d", place);
            } else {
                sprintf(name, "Podium4_%02d", place);
            }
            podiumMotions[place - 1] = character->UnknownFunction4a6b30(name, 1);
            Vector3 position = field_0x3c4 + offsets[place - 1];
            character->field_0x1a0->UnknownFunction4fc660(&position);
            character->field_0x1a0->UnknownFunction4fbd70(&facings[place - 1], &kVec3YAxis, 1, 1);
            character->UnknownFunction4a8b40(podiumMotions[place - 1]);
            character->field_0x10 = 0;
            if (racer) {
                UnknownPodiumTextureEntry** texture = ((UnknownPodiumRacer*)racer)->field_0x5c4->field_0x1a0->field_0x290;
                if (texture)
                    character->field_0x1a0->UnknownFunction444c70(0, (*texture)->field_0x2c, &context);
            }
        }
    }
    delete stream;
    if (g_TrackGame->mode.field_0xa28 && g_TrackGame->mode.field_0x23a4) {
        UnknownPodiumSound* sound = new(__FILE__, 0x34e) UnknownPodiumSound(PODIUM_OWNER(owner)->field_0x2c->field_0xc4, 1);
        UnknownTextureStream* audio = new(__FILE__, 0x34f) UnknownTextureStream((int)g_UnknownResourceManager572b44);
        if (!g_TrackGame->sceneObject->UnknownFunction4e9cd0(audio, "CrowdLoop.wav", "rb", 0)) {
            sprintf(message, "CrowdLoop.wav not found in Audio.res.\n");
            delete audio;
            return 0;
        }
        sound->UnknownFunction4bc320("CrowdLoop.wav", audio, 1, 0, 0, -1);
        delete audio;
        sound->UnknownFunction4bc6b0(0, 1, 0);
    }
    D3DRMVectorRotate(&rotated, &cameraOffset, &kVec3YAxis, angle);
    podiumCameraPosition = rotated * PodiumLength(cameraOffset) + field_0x3c4;
    D3DRMVectorRotate(&rotated, &lookOffset, &kVec3YAxis, angle);
    podiumCameraTarget = rotated * PodiumLength(lookOffset) + field_0x3c4;
    podiumPanSpeed = podiumCameraTarget - podiumCameraPosition;
    podiumCamera->UnknownFunction42e9b0(&podiumCameraPosition, 0, 0, 0, 0);
    podiumCamera->UnknownVirtualSlot29(field_0x3d8);
    float x0 = field_0x3c4.x - 16.0f;
    float x1 = x0 + 32.0f;
    float z0 = field_0x3c4.z - 16.0f;
    float z1 = z0 + 32.0f;
    RemoveVegetationInRect(x0, z0, x1, z1);
    UnknownFunction45fdc0(x0, z0, x1, z1);
    field_0x440 = 0;
    g_TrackGame->UnknownFunction468880();
    return 1;
}
