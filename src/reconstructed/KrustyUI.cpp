#include "KrustyUI.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "DebugAlloc.h"
#include "DirectPlayMessages.h"
#include "GameUi.h"
#include "OptionProcs.h"
#include "Parameterblocks.h"
#include "PCCamera.h"
#include "TextureMap.h"
#include "UIDialog.h"
#include "RenderTarget.h"
#include "TrackGame.h"
#include "DlgProcs.h"
#include "InGameProcs.h"
#include "MenuId.h"
#include "NetProcs.h"
#include "ProCircuitProcs.h"
#include "SelectGamePicProcs.h"
#include "TrackRecordDlg.h"

// The four per-file vector constants (see src/krusty2/math/Math3D.h):
// 0x0067c418, 0x0067c428, 0x0067c458 and 0x0067c3f8, initialised by
// 0x0049bdd0..0x0049bf0b. 0x00498cf0 reads kVec3Zero.
static const Vector3 kVec3Zero = Vector3(0.0f, 0.0f, 0.0f);
static const Vector3 kVec3XAxis = Vector3(1.0f, 0.0f, 0.0f);
static const Vector3 kVec3YAxis = Vector3(0.0f, 1.0f, 0.0f);
static const Vector3 kVec3ZAxis = Vector3(0.0f, 0.0f, 1.0f);

// cdecl 0x0047b570 (gameui.cpp): resizes a DebugMalloc'd block.
void* ResizeDebugBlock(void* block, unsigned int size);

// Global at 0x0068a498: 36 short strings loaded from string resources
// 5000-5035 (KrustyUI 0x004988a0).
char g_UnknownStrings68a498[36][16];

// cdecl 0x005053b0, called first on shutdown with 0.
void UnknownFunction5053b0(int value);

// Views for 0x00498cf0 (the garage scene). Only what it calls is declared;
// the classes' full declarations (LightEmitter.h, the D3DIMSoultree and
// ProjectedShadow headers) are not mixed into this unit. Names provisional.

// The Soultree object at a character's +0x1a0.
struct UnknownKrustyUISoultree {
    void UnknownFunction4444c0(int value);                 // 0x004444c0
    void SetPosition(const Vector3* position);   // 0x004fc660
    void SetAxes(float a, float b, float c, float d, float e, float f, float g,
                               int h);                     // 0x004fbd10
};

// RTTI D3DIMSoultreeCharacter (0x240 bytes, constructor 0x004455b0): vfptr
// at +0, vbptr at +4, virtual GameObject base. Its primary slot 11 loads a
// model file and returns the GameObject to add to the scene.
class UnknownKrustyUICharacter : public virtual GameObject {
public:
    explicit UnknownKrustyUICharacter(int flags);
    virtual void CharacterVirtualSlot0();
    virtual void CharacterVirtualSlot1();
    virtual void CharacterVirtualSlot2();
    virtual void CharacterVirtualSlot3();
    virtual void CharacterVirtualSlot4();
    virtual void CharacterVirtualSlot5();
    virtual void CharacterVirtualSlot6();
    virtual void CharacterVirtualSlot7();
    virtual void CharacterVirtualSlot8();
    virtual void CharacterVirtualSlot9();
    virtual void CharacterVirtualSlot10();
    virtual GameObject* CharacterVirtualSlot11(void* owner, const char* name, GameObject* lights,
                                               void* context, int a, int b);
    void SetMotionByName(const char* motion);        // 0x004a8b10: plays a motion
    int UnknownFunction4a6bb0(float time, int a, int b);   // 0x004a6bb0: advances it

    unsigned char field_0x008[0x1a0 - 0x8];
    UnknownKrustyUISoultree* field_0x1a0;
    unsigned char field_0x1a4[0x214 - 0x1a4];
};

// RTTI LightManager (0x11c bytes) and LightEmitter (0xac bytes).
class UnknownKrustyUILight;
class UnknownKrustyUILightManager : public GameObject {
public:
    explicit UnknownKrustyUILightManager(int flags);               // 0x0049e390
    void AddLight(UnknownKrustyUILight* light);        // 0x0049e470: adds a light
    unsigned char field_0x2c[0x11c - 0x2c];
};

class UnknownKrustyUILight : public GameObject {
public:
    explicit UnknownKrustyUILight(int flags);                      // 0x0049deb0
    // 0x0049e230: slot 8, then type, colour, range, position, direction.
    UnknownKrustyUILight* Create(void* owner, int type, unsigned int color,
                                                const Vector3* position, const Vector3* direction,
                                                float range, int sphere, int a8, int a9, int a10,
                                                int index);
    unsigned char field_0x2c[0xac - 0x2c];
};

// Adds `light` to `lights` when there is one. An inline helper: written out
// at each of the three sites, VC6 here forms the bike loop's addresses as
// [offset + list] instead of retail's [list + offset].
static inline void KrustyUIAddLight(UnknownKrustyUILightManager* lights, UnknownKrustyUILight* light) {
    if (light)
        lights->AddLight(light);
}

// RTTI ProjectedShadow (0x134 bytes, constructor 0x004da570).
class UnknownKrustyUIShadow : public GameObject {
public:
    explicit UnknownKrustyUIShadow(int flags);
    UnknownKrustyUIShadow* UnknownFunction4da7b0(void* owner, TextureMapManager* textures); // 0x004da7b0
    void UnknownFunction4dab00(UnknownKrustyUISoultree* caster);     // 0x004dab00: adds a caster
    void UnknownFunction4dae50(const Vector3* position, const Vector3* direction, int a); // 0x004dae50
    void UnknownFunction4dace0(int value);                           // 0x004dace0
    void UnknownFunction4dae30(int value);                           // 0x004dae30
    unsigned char field_0x2c[0x134 - 0x2c];
};

// RTTI D3DIMSoultreeShadow (0x48 bytes, constructor 0x00446840).
class UnknownKrustyUIShadowLink : public GameObject {
public:
    explicit UnknownKrustyUIShadowLink(int flags);
    GameObject* UnknownFunction4468b0(void* owner, UnknownKrustyUISoultree* caster,
                                      UnknownKrustyUIShadow* shadow); // 0x004468b0
    unsigned char field_0x2c[0x48 - 0x2c];
};

// The plate number painter (SelectGamePicProcs.h's UnknownBikeNumberPainter).
class UnknownKrustyUIPlatePainter {
public:
    UnknownKrustyUIPlatePainter(TextureMapManager* textures);       // 0x00417500
    ~UnknownKrustyUIPlatePainter();                                 // 0x00417570
    void PaintNumber(UnknownKrustyUISoultree* texture, int number); // 0x00417670
    unsigned char field_0x00[0x2c];
};

// Game+0x1c..+0x33, copied as the garage model's resource context; 0x00498cf0
// sets +0x14 when little video memory is free.
struct UnknownKrustyUIContext {
    TextureMapManager* field_0x00;
    int field_0x04;
    int field_0x08;
    int field_0x0c;
    int field_0x10;
    int field_0x14;
};

// The GUI's 0x00485c80 (GUIManager.h) opens a dialog resource.
class UnknownKrustyUIGuiView {
public:
    int OpenDialogResource(const char* resource);
};

// The garage scene's static vectors have an empty destructor: retail
// registers six empty atexit handlers (0x00499920..0x00499970).
struct UnknownKrustyUIVector : public Vector3 {
    ~UnknownKrustyUIVector() {}
    UnknownKrustyUIVector& operator=(const Vector3& v) {
        Vector3::operator=(v);
        return *this;
    }
};

inline Vector3 operator-(const Vector3& a, const Vector3& b) {
    return Vector3(a.x - b.x, a.y - b.y, a.z - b.z);
}

// cdecl 0x00511970 (Tgafile.h): bytes per pixel of a pixel format.
int PixelFormatBytesPerPixel(int format);

// Views of the +0x48 (model), +0x50 (bike) and +0x58 (rider) lists.
struct UnknownKrustyUIModelEntry {
    char name[0x40];                          // +0x00: "Name"
    char modelFile[0x40];                     // +0x40: "ModelFile" (a rider's "File" runs to +0xc0)
    char uiModelFile[0x40];                   // +0x80: "UIModelFile"
    UnknownKrustyUICharacter* uiModel;        // +0xc0: the loaded UI model (0x00498cf0)
    int availability;                         // +0xc4: "Availability" (3 for the random choices)
};

struct UnknownKrustyUIBikeEntry {
    int manufacturer;                         // +0x00: index into +0x48
    char name[0x44];                          // +0x04: "BikeName_%02d"
    char texture[0x40];                       // +0x48: "BikeTexture_%02d"
    int field_0x88;
    int engineSize;                           // +0x8c: "BikeEngineSize_%02d" (cc)
    int fourStroke;                           // +0x90: "BikeFourStroke_%02d"
};

// Copies at most size - 1 characters of `source` and terminates them.
#define UNKNOWN_COPY_SIZED(destination, source, size)                       \
    strncpy(destination, source, size);                                     \
    destination[strlen(source) < size - 1 ? strlen(source) : size - 1] = 0;

// A random value in [0, 1).
static inline float RandomUnit() {
    // The cast keeps VC6 from folding the scale into a following constant
    // factor or reordering it after a variable one.
    return (float)(rand() * (1.0f / 32768));
}

// cdecl 0x0049bda0: the GUI's progress callback.
void UnknownFunction49bda0() {
    g_TrackGame->mode.SaveProfile();
}

// 0x004987f0
KrustyUI::KrustyUI(int flags) : GameObject(flags) {
    gui = 0;
    returnMenu = 0;
    previousMenu = 0;
    field_0x38 = 0;
    currentMenu = 0;
    field_0x48c = 0;
    progressDialog = 0;
    garageScene = 0;
    replayAvailable = 0;
    demoEnded = 0;
    savedInputContext = 0;
    allowIme = 0;
    field_0x4a8 = 0;
    field_0x4ac = 0;
    field_0x4b0 = 0;
    field_0x4a4 = 0;
    field_0x4a0 = 0;
    field_0x49c = 0;
    entries = 0;
    entryCount = 0;
}


// KrustyUI.h leaves +0x480 as bytes and types +0x46c and +0x470 as the
// selection dialogs' model views; 0x00498cf0 stores the garage character at
// +0x470, the rider at +0x46c and the rider's position (a Vector3) at +0x480.
#define KRUSTYUI_GARAGE (*(UnknownKrustyUICharacter**)&garageCharacter)
#define KRUSTYUI_RIDER (*(UnknownKrustyUICharacter**)&riderPreview)
#define KRUSTYUI_RIDER_POSITION (*(Vector3*)field_0x480)
#define KRUSTYUI_TARGET ((PCRenderTarget*)field_0x18)
#define KRUSTYUI_MODELS ((UnknownKrustyUIModelEntry*)manufacturers)

// 0x00498cf0: builds the garage scene (+0x464) on the first call with 1 or
// -1: camera, lights, the garage and rider models and their shadow; with 2
// or -1 afterwards, loads each bike model with its plate number once.
void KrustyUI::BuildGarageScene(int value) {
    static UnknownKrustyUIVector s408;
    static UnknownKrustyUIVector s438;
    static UnknownKrustyUIVector s448;
    static UnknownKrustyUIVector s468;
    static UnknownKrustyUIVector s478;
    static UnknownKrustyUIVector s488;
    static UnknownKrustyUILightManager* lights;
    static UnknownKrustyUILight* light;
    static UnknownKrustyUIShadow* shadow;
    static GameObject* shadowLink;
    static int pending;
    static int done;
    char text[260];

    field_0x4ac = 0;
    field_0x4a8 = 0;
    ((RenderTarget*)field_0x18)->clearTarget = 0;
    g_TrackGame->field_0x2d5_bit2 = 1;
    if (!garageScene && (value == 1 || value == -1)) {
        UnknownFunction5053b0(1);
        allowIme = 0;
        if (g_TrackGame->LoadResourceString(0x1438, text, 0x80))
            allowIme = atoi(text) != 0;
        allowIme = g_TrackGame->GetRegistryFlag("AllowIME", allowIme);
        if (allowIme)
            gui->EnableWindowClipper(1);
        ((UnknownKrustyUIGuiView*)gui)->OpenDialogResource("global.dtm");
        garageFocus = kVec3Zero;
        KRUSTYUI_RIDER_POSITION = kVec3Zero;
        KRUSTYUI_RIDER_POSITION.y += 0.1f;
        garageScene = new(__FILE__, 0x105) GameObject(1);
        PCCamera* camera = new(__FILE__, 0x108) PCCamera(1);
        garageCamera = (Camera*)camera->UnknownVirtualSlot8(field_0x18);
        garageScene->AppendChild(garageCamera, -1);
        garageCamera->field_0x1d8 = 0;
        garageCamera->field_0x1d4 = 0;
        UnknownKrustyUILightManager* manager = new(__FILE__, 0x111) UnknownKrustyUILightManager(1);
        lights = (UnknownKrustyUILightManager*)manager->UnknownVirtualSlot8(field_0x18);
        garageScene->AppendChild(lights, -1);
        UnknownKrustyUILight* emitter = new(__FILE__, 0x116) UnknownKrustyUILight(1);
        light = emitter->Create(field_0x18, 6, 0x606060, 0, 0, 0, 0, 0, 0, 0, 0);
        garageScene->AppendChild(light, -1);
        KrustyUIAddLight(lights, light);
        Vector3 position(-1800.0f, 200.0f, 0.0f);
        Vector3 direction = garageFocus - position;
        emitter = new(__FILE__, 0x121) UnknownKrustyUILight(1);
        light = emitter->Create(field_0x18, 4, 0xffffff, &position, &direction, 0, 0, 0,
                                               0, 0, 0);
        garageScene->AppendChild(light, -1);
        KrustyUIAddLight(lights, light);
        s488 = garageFocus;
        s488.y += 8.0f;
        s488.x += 12.0f;
        s408 = garageFocus - s488;
        s448 = s488;
        s448.x -= 24.0f;
        s438 = garageFocus - s448;
        emitter = new(__FILE__, 0x130) UnknownKrustyUILight(1);
        light = emitter->Create(field_0x18, 1, 0x8c8c8c, &s488, &s408, 0, 0, 0, 0, 0, 0);
        garageScene->AppendChild(light, -1);
        KrustyUIAddLight(lights, light);
        UnknownKrustyUIContext context = *(UnknownKrustyUIContext*)&g_TrackGame->field_0x1c;
        context.field_0x14 = 0;
        if (!g_TrackGame->field_0x2d0) {
            // Free video memory: total less the frame buffers, or a fixed 4 MB
            // (2 MB below 800x600) off the total outside NT without the 0x400 cap.
            int memory = KRUSTYUI_TARGET->targetDisplay->totalVideoMemory;
            int free = memory - PixelFormatBytesPerPixel(KRUSTYUI_TARGET->surfacePixelFormat) *
                                    (KRUSTYUI_TARGET->frameModulus + 1) * KRUSTYUI_TARGET->targetHeight *
                                    KRUSTYUI_TARGET->targetWidth;
            if (g_TrackGame->osVersion.platformId != 2 &&
                !(KRUSTYUI_TARGET->targetDisplay->driverCaps & 0x400))
                free = (KRUSTYUI_TARGET->deviceCaps & 0x4000) ? memory - 0x400000 : memory - 0x200000;
            if (!g_TrackGame->display->isAGP && free < 0x500000)
                context.field_0x14 = 1;
        }
        KRUSTYUI_GARAGE = new(__FILE__, 0x174) UnknownKrustyUICharacter(1);
        garageScene->AppendChild(
            KRUSTYUI_GARAGE->CharacterVirtualSlot11(field_0x18, "UIgarage.mcf", lights, &context, 1, 1), -1);
        KRUSTYUI_GARAGE->field_0x1a0->UnknownFunction4444c0(0);
        KRUSTYUI_GARAGE->field_0x1a0->SetPosition(&garageFocus);
        KRUSTYUI_GARAGE->field_0x1a0->SetAxes(0.0f, 0.0f, -1.0f, 0.0f, 1.0f, 0.0f, 0.0f, 1);
        KRUSTYUI_GARAGE->SetMotionByName("Spin");
        KRUSTYUI_GARAGE->UnknownFunction4a6bb0(0.03f, 0, 0);
        UnknownKrustyUIShadow* projected = new(__FILE__, 0x182) UnknownKrustyUIShadow(1);
        shadow = projected->UnknownFunction4da7b0(field_0x18, g_TrackGame->textureManager);
        UnknownKrustyUIShadowLink* link = new(__FILE__, 0x186) UnknownKrustyUIShadowLink(1);
        shadowLink = link->UnknownFunction4468b0(field_0x18, KRUSTYUI_GARAGE->field_0x1a0, shadow);
        garageScene->AppendChild(shadowLink, -1);
        if (shadow)
            garageScene->AppendChild(shadow, -1);
        KRUSTYUI_RIDER = new(__FILE__, 0x194) UnknownKrustyUICharacter(1);
        garageScene->AppendChild(
            KRUSTYUI_RIDER->CharacterVirtualSlot11(field_0x18, "UIRider.mcf", lights,
                                                   &g_TrackGame->field_0x1c, 1, 1), -1);
        KRUSTYUI_RIDER->field_0x1a0->UnknownFunction4444c0(1);
        KRUSTYUI_RIDER->field_0x1a0->SetPosition(&KRUSTYUI_RIDER_POSITION);
        KRUSTYUI_RIDER->field_0x1a0->SetAxes(0.0f, 0.0f, -1.0f, 0.0f, 1.0f, 0.0f, 0.0f, 1);
        KRUSTYUI_RIDER->SetMotionByName("WaitR");
        KRUSTYUI_RIDER->UnknownFunction4a6bb0(0.03f, 0, 0);
        ((RenderTarget*)field_0x18)->SetCamera(0);
        pending = 1;
        done = 0;
    }
    if (pending && !done && (value == 2 || value == -1)) {
        UnknownKrustyUIPlatePainter painter(g_TrackGame->field_0x1c);
        for (int i = 0; i < manufacturerCount; i++) {
            sprintf(text, "Animations\\UIbike\\%s", KRUSTYUI_MODELS[i].uiModelFile);
            KRUSTYUI_MODELS[i].uiModel = new(__FILE__, 0x1b4) UnknownKrustyUICharacter(0);
            garageScene->AppendChild(
                KRUSTYUI_MODELS[i].uiModel->CharacterVirtualSlot11(field_0x18, text, lights,
                                                                      &g_TrackGame->field_0x1c,
                                                                      1, 1), -1);
            UnknownKrustyUICharacter* bike = KRUSTYUI_MODELS[i].uiModel;
            bike->field_0x1a0->UnknownFunction4444c0(1);
            bike->field_0x1a0->SetPosition(&garageFocus);
            bike->field_0x1a0->SetAxes(0.0f, 0.0f, -1.0f, 0.0f, 1.0f, 0.0f, 0.0f, 1);
            bike->SetMotionByName("WaitB");
            bike->UnknownFunction4a6bb0(0.03f, 0, 0);
            painter.PaintNumber(bike->field_0x1a0, g_TrackGame->mode.plateNumber);
        }
        if (shadow) {
            shadow->UnknownFunction4dab00(KRUSTYUI_MODELS[0].uiModel->field_0x1a0);
            shadow->UnknownFunction4dab00(KRUSTYUI_RIDER->field_0x1a0);
            s468 = garageFocus;
            s468.y += 40.0f;
            s468.x += 15.0f;
            s478 = garageFocus - s468;
            // Retail passes the key light's vectors here, not the two just set.
            shadow->UnknownFunction4dae50(&s488, &s408, 3);
            shadow->UnknownFunction4dace0(0x20);
            shadow->UnknownFunction4dae30(0x20);
            pending = 0;
            done = 1;
        }
    }
}

// 0x0049b470
KrustyUI::~KrustyUI() {
    Shutdown();
    if (manufacturers)
        DebugFree(manufacturers, __FILE__, 1306);
    if (bikes)
        DebugFree(bikes, __FILE__, 1307);
    if (riders)
        DebugFree(riders, __FILE__, 1308);
    if (entries)
        DebugFree(entries, __FILE__, 1309);
}

// 0x004999b0
void KrustyUI::Shutdown() {
    UnknownFunction5053b0(0);
    if (!g_TrackGame->field_0x2d5_bit1)
        gui->CloseDialogResource();
    if (garageScene) {
        garageScene->Release();
        garageScene = 0;
    }
}

// 0x0049b530: tells the "ProgressBar" control 1 (0x0047b370).
void KrustyUI::StepProgressBar() {
    if (progressDialog) {
        UIProgressBar* bar = static_cast<UIProgressBar*>(progressDialog->UnknownFunction46ebf0("ProgressBar", 0));
        if (bar)
            bar->StepProgress(1);
    }
}

// 0x0049bb80
void KrustyUI::UnknownFunction49bb80() {
    if (entries)
        DebugFree(entries, __FILE__, 1453);
    entries = 0;
    entryCount = 0;
}

// 0x00499ad0
void KrustyUI::UnknownVirtualSlot4() {
    GameObject::UnknownVirtualSlot4();
}

// 0x00499ac0
void KrustyUI::UnknownVirtualSlot5() {
    GameObject::UnknownVirtualSlot5();
}

// 0x00499ae0
int KrustyUI::UnknownVirtualSlot10(float frameTime) {
    return GameObject::UnknownVirtualSlot10(frameTime);
}

// 0x00499af0
int KrustyUI::UnknownVirtualSlot18() {
    GameObject::UnknownVirtualSlot18();
    return 1;
}

// 0x00499a40
int KrustyUI::UnknownVirtualSlot23(UnknownControlEvent* event, UnknownInputEntry* entry) {
    field_0x4a4 = 0;
    return GameObject::UnknownVirtualSlot23(event, entry) != 0;
}

// 0x00499a70: DPSYS_HOST (this player became the session host) sets the
// network object's +0x10.
int KrustyUI::UnknownVirtualSlot24(int type, void* data, int from, int to, int flags) {
    if (GameObject::UnknownVirtualSlot24(type, data, from, to, flags))
        return 1;
    if (type == DPSYS_HOST)
        g_TrackGame->network->isHost = 1;
    return 0;
}

// 0x00499980: also hands the value to the +0x464 scene.
int KrustyUI::UnknownVirtualSlot25(void* value) {
    GameObject::UnknownVirtualSlot25(value);
    if (garageScene)
        garageScene->UnknownVirtualSlot25(value);
    return 1;
}

// 0x004999f0
void KrustyUI::ShowScene(GameObject* parent) {
    if (garageScene) {
        parent->AppendChild(garageScene, -1);
        ((RenderTarget*)field_0x18)->SetCamera(garageCamera);
    }
}

// 0x00499a20
void KrustyUI::HideScene() {
    if (garageScene) {
        garageScene->Unlink();
        ((RenderTarget*)field_0x18)->SetCamera(0);
    }
}

// 0x00499b00
void KrustyUI::ShowCursors() {
    if (gui)
        gui->ShowCursors(1);
}

// 0x00499b10
void KrustyUI::UnknownFunction499b10() {
    if (gui)
        gui->ShowCursors(0);
}

// 0x00499b20: opens menu `menu` (each id has its dialog class).
void KrustyUI::OpenMenu(int menu) {
    UIDialog* dialog;
    if (menu == 0xd0)
        previousMenu = currentMenu;
    currentMenu = menu;
    switch (menu) {
    case kMenuMain:
        dialog = new(__FILE__, 674) MainDlg;
        gui->ShowDialog(dialog, menu, 2, 0, 0, 0, 0, 1);
        g_TrackGame->ResetClock();
        break;
    case kMenuSinglePlayer:
        dialog = new(__FILE__, 681) SinglePlayerDlg;
        gui->ShowDialog(dialog, menu, 2, 0, 0, 0, 0, 1);
        break;
    case kMenuOptions:
        dialog = new(__FILE__, 685) OptionsDlg;
        gui->ShowDialog(dialog, menu, 2, 0, 0, 0, 0, 1);
        break;
    case kMenuLoading:
        if (g_TrackGame->mode.displayModeIndex != g_TrackGame->display->currentDisplayMode)
            g_TrackGame->UnknownVirtualSlot19(g_TrackGame->mode.displayModeIndex);
        dialog = new(__FILE__, 695) LoadingDlg(1);
        gui->ShowDialog(dialog, menu, 2, 0, 0, 0, 0, 1);
        break;
    case kMenuHostJoin85c:
    case kMenuHostJoin85d:
        dialog = new(__FILE__, 727) HostJoinDlg;
        gui->ShowDialog(dialog, menu, 4, 0, (int)gui->FindInputDialog(), 0, 0, 1);
        break;
    case kMenuMultiPlayer866:
    case kMenuMultiPlayer867:
    case kMenuMultiPlayer868:
        dialog = new(__FILE__, 733) MultiPlayerDlg;
        gui->ShowDialog(dialog, menu, 2, 0, 0, 0, 0, 1);
        break;
    case kMenuSerialPopup:
        dialog = new(__FILE__, 737) SerialPopupDlg;
        gui->ShowDialog(dialog, kMenuSerialPopup, 4, 0, (int)gui->FindInputDialog(), 0, 0, 1);
        break;
    case kMenuTCPAddress:
        dialog = new(__FILE__, 741) TCPAddressDlg;
        gui->ShowDialog(dialog, menu, 4, 0, (int)gui->FindInputDialog(), 0, 0, 1);
        break;
    case kMenuPCCentral:
        if (g_TrackGame->proCircuit) {
            dialog = new(__FILE__, 747) PCCentralDlg;
            gui->ShowDialog(dialog, 0, 2, 0, 0, 0, 0, 1);
        }
        break;
    case kMenu88e: {
        UnknownTrackGameObject3444* circuit = g_TrackGame->proCircuit;
        if (circuit) {
            if (circuit->schedules[circuit->series].bonusTrack == circuit->raceIndex) {
                circuit->AdvanceAfterRace();
                OpenCareerDialog();
                return;
            }
            if (g_TrackGame->mode.raceSettings.raceMode == 1) {
                dialog = new(__FILE__, 760) PCLastRaceDlg;
                gui->ShowDialog(dialog, kMenu88e, 2, 0, 0, 0, 0, 1);
            } else {
                dialog = new(__FILE__, 763) PCCentralDlg;
                gui->ShowDialog(dialog, 0, 2, 0, 0, 0, 0, 1);
            }
        } else if (g_TrackGame->playerCount == 1) {
            dialog = new(__FILE__, 767) SinglePlayerDlg;
            gui->ShowDialog(dialog, kMenu88e, 2, 0, 0, 0, 0, 1);
        } else {
            dialog = new(__FILE__, 769) MultiPlayerDlg;
            gui->ShowDialog(dialog, kMenu88e, 2, 0, 0, 0, 0, 1);
        }
        break;
    }
    case kMenuWaitOrCall:
        dialog = new(__FILE__, 774) WaitOrCallDlg;
        gui->ShowDialog(dialog, menu, 4, 0, (int)gui->FindInputDialog(), 0, 0, 1);
        break;
    case kMenuTrackRecord910:
    case kMenuTrackRecord911:
        dialog = new(__FILE__, 788) TrackRecordDlg;
        gui->ShowDialog(dialog, menu, 2, 0, 0, 0, 0, 1);
        break;
    case kMenuUserNameBb9:
    case kMenuUserNameBba:
    case kMenuUserNameBbb:
        dialog = new(__FILE__, 798) UserNameDlg;
        gui->ShowDialog(dialog, menu, 0xc, 0, (int)gui->FindInputDialog(), 0, 0, 1);
        break;
    case kMenuRemoveProfile:
        dialog = new(__FILE__, 802) RemoveProfileDlg;
        gui->ShowDialog(dialog, menu, 4, 0, (int)gui->FindInputDialog(), 0, 0, 1);
        break;
    case kMenuConnectError:
        dialog = new(__FILE__, 806) ConnectErrorDlg;
        gui->ShowDialog(dialog, kMenuConnectError, 4, 0, (int)gui->FindInputDialog(), 0, 0, 1);
        break;
    case kMenuPlayerRemoved:
        dialog = new(__FILE__, 810) PlayerRemovedDlg;
        gui->ShowDialog(dialog, kMenuPlayerRemoved, 4, 0, (int)gui->FindInputDialog(), 0, 0, 1);
        break;
    case kMenuExit:
        dialog = new(__FILE__, 817) ExitDlg;
        gui->ShowDialog(dialog, kMenuExit, 0x1c, 0, 0, 0, 0, 1);
        break;
    case kMenuContinue:
        dialog = new(__FILE__, 822) ContinueDlg;
        gui->ShowDialog(dialog, menu, 0x1c, 0, 0, 0, 0, 1);
        break;
    }
}

// 0x0049a4a0: turns "MediaControl" on, hides the GUI and opens Exit1Dlg.
void KrustyUI::OpenExitDialog() {
    DisableByClassName("MediaControl");
    gui->ShowCursors(0);
    gui->ShowDialog(new(__FILE__, 836) Exit1Dlg, 0, 2, 0, 0, 0, 0, 1);
}

// 0x0049a540: reads presets.pb's garage tables, or sets the defaults.
void KrustyUI::LoadGarageTables() {
    char key[128];
    UnknownParameterBlock block;
    UnknownTextureStream* stream = new(__FILE__, 851) UnknownTextureStream((int)g_UnknownResourceManager572b44);
    if (stream->Open("presets.pb", "r", 0)) {
        block.Attach((UnknownParameterStream*)stream, 0, 1);
        for (int i = 0; i < 5; i++) {
            sprintf(key, "Category_%d", i + 1);
            block.SelectSection(key);
            block.GetInt("RPMLowerLimit", 5000, &rpmLowerLimit[i]);
            block.GetInt("RPMUpperLimit", 10000, &rpmUpperLimit[i]);
            rpmStep[i] = (rpmUpperLimit[i] - rpmLowerLimit[i]) / 10;
            block.GetInt("HPSum", 400, &hpSum[i]);
            block.GetInt("MinRange", 10, &minRange[i]);
            block.GetInt("Weight", 200, &weight[i]);
            int maximum = 0;
            int j;
            for (j = 0; j < 11; j++) {
                sprintf(key, "RPM%05d", rpmLowerLimit[i] + rpmStep[i] * j);
                block.GetInt(key, 10, &bandMaxima[i][j]);
                if (maximum <= bandMaxima[i][j])
                    maximum = bandMaxima[i][j];
            }
            maxRange[i] = maximum;
            for (int preset = 0; preset < 3; preset++) {
                sprintf(key, "Category_%dPreset_%d", i + 1, preset + 1);
                block.SelectSection(key);
                for (j = 0; j < 11; j++) {
                    sprintf(key, "RPM%05d", rpmLowerLimit[i] + rpmStep[i] * j);
                    block.GetInt(key, 10, &standardCurves[i][preset][j]);
                }
            }
        }
    } else {
        for (int i = 0; i < 5; i++) {
            rpmLowerLimit[i] = 5000;
            rpmUpperLimit[i] = 10000;
            hpSum[i] = 100;
            rpmStep[i] = (rpmUpperLimit[i] - rpmLowerLimit[i]) / 10;
            for (int preset = 0; preset < 3; preset++) {
                for (int j = 0; j < 11; j++) {
                    standardCurves[i][preset][j] = 10;
                    bandMaxima[i][j] = 40;
                }
            }
        }
    }
    delete stream;
}

// 0x0049a8b0: reads the bike models and bikes (bikes.pb) and the riders
// (riders.pb) into the +0x48, +0x50 and +0x58 lists. Only manufacturers 3
// and 7, their second bike, and riders 42 and 44 are loaded. Each file's
// entry count is a local of its own block: the two share one frame slot and
// that puts the manufacturer index below the bike count, as retail has it.
void KrustyUI::LoadBikesAndRiders() {
    char key[256];
    UnknownParameterBlock block;
    UnknownTextureStream* stream = new(__FILE__, 913) UnknownTextureStream((int)g_UnknownResourceManager572b44);
    if (manufacturers)
        DebugFree(manufacturers, __FILE__, 918);
    if (bikes)
        DebugFree(bikes, __FILE__, 919);
    if (riders)
        DebugFree(riders, __FILE__, 920);
    bikes = 0;
    riders = 0;
    riderCount = 0;
    bikeCount = 0;
    manufacturerCount = 0;
    if (stream->Open("bikes.pb", "r", 0)) {
        block.Attach((UnknownParameterStream*)stream, 0, 1);
        block.SelectSection("Information");
        int count;
        block.GetInt("NumberOfManufacturers", 0, &count);
        for (int i = 1; i <= count; i++) {
            if (i != 7 && i != 3)
                continue;
            int manufacturerBikeCount;
            int availability;
            char name[260];
            char uiModel[260];
            char model[260];
            sprintf(key, "Manufacturer_%02d", i);
            block.SelectSection(key);
            block.GetInt("NumberOfBikes", 0, &manufacturerBikeCount);
            block.GetString("Name", "(no name)", name, -1);
            block.GetString("UIModelFile", "", uiModel, -1);
            block.GetString("ModelFile", "", model, -1);
            block.GetInt("Availability", 1, &availability);
            manufacturers = ResizeDebugBlock(manufacturers, (manufacturerCount + 1) * sizeof(UnknownKrustyUIModelEntry));
            int length = strlen(name);
            int n = length > 63 ? 63 : length;
            strncpy(((UnknownKrustyUIModelEntry*)manufacturers)[manufacturerCount].name, name, n);
            ((UnknownKrustyUIModelEntry*)manufacturers)[manufacturerCount].name[n] = 0;
            length = strlen(model);
            n = length > 63 ? 63 : length;
            strncpy(((UnknownKrustyUIModelEntry*)manufacturers)[manufacturerCount].modelFile, model, n);
            ((UnknownKrustyUIModelEntry*)manufacturers)[manufacturerCount].modelFile[n] = 0;
            length = strlen(uiModel);
            n = length > 63 ? 63 : length;
            strncpy(((UnknownKrustyUIModelEntry*)manufacturers)[manufacturerCount].uiModelFile, uiModel, n);
            ((UnknownKrustyUIModelEntry*)manufacturers)[manufacturerCount].uiModelFile[n] = 0;
            ((UnknownKrustyUIModelEntry*)manufacturers)[manufacturerCount].availability = availability;
            for (int j = 1; j <= manufacturerBikeCount; j++) {
                if (j != 2)
                    continue;
                char bikeName[260];
                char texture[260];
                sprintf(key, "BikeName_%02d", j);
                block.GetString(key, "(no name)", bikeName, -1);
                sprintf(key, "BikeTexture_%02d", j);
                block.GetString(key, "", texture, -1);
                bikes = ResizeDebugBlock(bikes, (bikeCount + 1) * sizeof(UnknownKrustyUIBikeEntry));
                ((UnknownKrustyUIBikeEntry*)bikes)[bikeCount].manufacturer = manufacturerCount;
                ((UnknownKrustyUIBikeEntry*)bikes)[bikeCount].field_0x88 = 0;
                length = strlen(bikeName);
                n = length > 63 ? 63 : length;
                strncpy(((UnknownKrustyUIBikeEntry*)bikes)[bikeCount].name, bikeName, n);
                ((UnknownKrustyUIBikeEntry*)bikes)[bikeCount].name[n] = 0;
                length = strlen(texture);
                n = length > 63 ? 63 : length;
                strncpy(((UnknownKrustyUIBikeEntry*)bikes)[bikeCount].texture, texture, n);
                ((UnknownKrustyUIBikeEntry*)bikes)[bikeCount].texture[n] = 0;
                sprintf(key, "BikeEngineSize_%02d", j);
                block.GetInt(key, 250, &((UnknownKrustyUIBikeEntry*)bikes)[bikeCount].engineSize);
                sprintf(key, "BikeFourStroke_%02d", j);
                block.GetInt(key, 0, &((UnknownKrustyUIBikeEntry*)bikes)[bikeCount].fourStroke);
                bikeCount++;
            }
            manufacturerCount++;
        }
    }
    if (stream->Open("riders.pb", "r", 0)) {
        block.Attach((UnknownParameterStream*)stream, 0, 1);
        block.SelectSection("Information");
        int count;
        block.GetInt("NumberOfRiders", 0, &count);
        for (int i = 1; i <= count; i++) {
            if (i != 42 && i != 44)
                continue;
            int availability;
            char riderName[260];
            char file[260];
            sprintf(key, "Rider_%02d", i);
            block.SelectSection(key);
            block.GetString("Name", "(no name)", riderName, -1);
            block.GetString("File", "", file, -1);
            block.GetInt("Availability", 1, &availability);
            riders = ResizeDebugBlock(riders, (riderCount + 1) * sizeof(UnknownKrustyUIModelEntry));
            ((UnknownKrustyUIModelEntry*)riders)[riderCount].uiModel = 0;
            int length = strlen(riderName);
            int n = length > 63 ? 63 : length;
            strncpy(((UnknownKrustyUIModelEntry*)riders)[riderCount].name, riderName, n);
            ((UnknownKrustyUIModelEntry*)riders)[riderCount].name[n] = 0;
            length = strlen(file);
            n = length > 127 ? 127 : length;
            strncpy(((UnknownKrustyUIModelEntry*)riders)[riderCount].modelFile, file, n);
            ((UnknownKrustyUIModelEntry*)riders)[riderCount].modelFile[n] = 0;
            ((UnknownKrustyUIModelEntry*)riders)[riderCount].availability = availability;
            riderCount++;
        }
    }
    delete stream;
}

// 0x0049b560
int KrustyUI::ResolveBikeFile(const char* name, char* text, int size) {
    char path[128];
    UNKNOWN_COPY_SIZED(text, name, size);
    if (g_UnknownResourceManager572b44->FindItem(name, 1))
        return 1;
    if (!strchr(name, '\\')) {
        sprintf(path, "%s\\%s", "Res", name);
    } else {
        int length = strlen(name);
        int count = length > 127 ? 127 : length;
        strncpy(path, name, count);
        path[count] = 0;
    }
    UnknownTextureStream* stream = new(__FILE__, 1369) UnknownTextureStream((int)g_UnknownResourceManager572b44);
    if (stream->Open(path, "r", 0)) {
        delete stream;
        UNKNOWN_COPY_SIZED(text, path, size);
        return 1;
    }
    delete stream;
    int index;
    do {
        index = (int)(RandomUnit() * (bikeCount - 1));
    } while (((UnknownKrustyUIBikeEntry*)bikes)[index].field_0x88
             || ((UnknownKrustyUIModelEntry*)manufacturers)[((UnknownKrustyUIBikeEntry*)bikes)[index].manufacturer].availability != 3);
    UNKNOWN_COPY_SIZED(text, ((UnknownKrustyUIBikeEntry*)bikes)[index].texture, size);
    return 0;
}

// 0x0049b7f0
int KrustyUI::ResolveRiderFile(const char* name, char* text, int size) {
    char path[128];
    UNKNOWN_COPY_SIZED(text, name, size);
    if (g_UnknownResourceManager572b44->FindItem(name, 1))
        return 1;
    if (!strchr(name, '\\')) {
        sprintf(path, "%s\\%s", "Res", name);
    } else {
        int length = strlen(name);
        int count = length > 127 ? 127 : length;
        strncpy(path, name, count);
        path[count] = 0;
    }
    UnknownTextureStream* stream = new(__FILE__, 1411) UnknownTextureStream((int)g_UnknownResourceManager572b44);
    if (stream->Open(path, "r", 0)) {
        delete stream;
        UNKNOWN_COPY_SIZED(text, path, size);
        return 1;
    }
    delete stream;
    int index;
    do {
        index = (int)(RandomUnit() * (riderCount - 1));
    } while (((UnknownKrustyUIModelEntry*)riders)[index].uiModel
             || ((UnknownKrustyUIModelEntry*)riders)[index].availability != 3);
    UNKNOWN_COPY_SIZED(text, ((UnknownKrustyUIModelEntry*)riders)[index].modelFile, size);
    return 0;
}

// 0x0049ba70
int KrustyUI::AddEntry(int a, int scene, int series, const char* name, int d, int e) {
    entries = (UnknownKrustyUIEntry*)ResizeDebugBlock(entries, (entryCount + 1) * sizeof(UnknownKrustyUIEntry));
    entries[entryCount].field_0x00 = a;
    entries[entryCount].scene = scene;
    entries[entryCount].series = series;
    int length = strlen(name);
    int count = length > 63 ? 63 : length;
    strncpy(entries[entryCount].fileName, name, count);
    entries[entryCount].fileName[count] = 0;
    entries[entryCount].field_0x0c = d;
    entries[entryCount].field_0x10 = e;
    return ++entryCount - 1;
}

// 0x0049bbb0
void KrustyUI::RecordRaceResults() {
    UnknownTrackGameModeSettings* settings = &g_TrackGame->mode.raceSettings;
    if (settings->eventType == 1 && settings->field_0x2c)
        return;
    RecordHighScoreTable(0);
    int mode = g_TrackGame->mode.raceSettings.eventType;
    if (mode != 0 && mode != 4) {
        if (g_TrackGame->mode.raceSettings.lapCount == 5)
            RecordHighScoreTable(1);
        if (g_TrackGame->mode.raceSettings.lapCount == 10)
            RecordHighScoreTable(2);
    } else {
        if (g_TrackGame->mode.raceSettings.field_0x140 == 5.0f)
            RecordHighScoreTable(1);
        if (g_TrackGame->mode.raceSettings.field_0x140 == 10.0f)
            RecordHighScoreTable(2);
    }
}

// 0x0049bc50: selects high-score table `table`, names it after the track
// and adds the racers' results; saves it when one of them made the table.
void KrustyUI::RecordHighScoreTable(int table) {
    char name[128];
    if (!g_TrackGame->mode.raceSettings.treeCollision)
        return;
    g_TrackGame->field_0x3400->field_0x00 = table;
    switch (g_TrackGame->mode.raceSettings.eventType) {
    case 1:
    case 5:
        sprintf(name, "%s%d", g_TrackGame->mode.raceSettings.trackName,
                g_TrackGame->mode.raceSettings.field_0x34);
        break;
    default: {
        int length = strlen(g_TrackGame->mode.raceSettings.trackName);
        int count = length > 127 ? 127 : length;
        strncpy(name, g_TrackGame->mode.raceSettings.trackName, count);
        name[count] = 0;
        break;
    }
    }
    g_TrackGame->field_0x3400->UnknownFunction51f0b0(
        (short)g_TrackGame->mode.raceSettings.eventType, name);
    int added = 0;
    if (g_TrackGame->playerCount == 1) {
        if (!g_TrackGame->field_0x3400->AddRacer(
                (short)g_TrackGame->mode.raceSettings.eventType, 0))
            return;
    } else {
        for (int i = 0; i < g_TrackGame->playerCount; i++) {
            if (g_TrackGame->field_0x3400->AddRacer(
                    (short)g_TrackGame->mode.raceSettings.eventType, i))
                added = 1;
        }
        if (!added)
            return;
    }
    g_TrackGame->field_0x3400->UnknownFunction51f260(
        (short)g_TrackGame->mode.raceSettings.eventType, name,
        g_TrackGame->mode.GetSelectedSeries());
}

// 0x0049b020
void KrustyUI::UnknownFunction49b020(const char** names, int count) {
    int* used = (int*)DebugCalloc(36, sizeof(int), __FILE__, 1120);
    srand(ReadClock());
    for (int i = count; i > 0; i--) {
        int index = (int)(RandomUnit() * 36);
        if (index >= 35)
            index = 35;
        int start = index;
        while (used[index]) {
            if (++index == 36)
                index = 0;
            if (index == start)
                break;
        }
        used[index] = 1;
        *names++ = g_UnknownStrings68a498[index];
    }
    DebugFree(used, __FILE__, 1157);
}

// 0x0049b0d0: picks `count` distinct random bikes and riders. Only models of
// availability 2 or 3 take part, and a bike must be in the player's class or
// a neighbouring one (1 and 2, 3 and 4). A first pick of an unused bike
// keeps class 1 four times in five and class 2 one time in five (any other
// class always); otherwise the next unused bike after it is taken. The
// class 1/2 test is one `||` of two `&&` pairs: if/else, goto and ternary
// forms of it lay the second arm out before the probe loop, retail after it.
void KrustyUI::UnknownFunction49b0d0(int* bikeChoices, int count, int* riderChoices) {
    int* usedBikes = (int*)DebugCalloc(bikeCount, sizeof(int), __FILE__, 1164);
    int* usedRiders = (int*)DebugCalloc(riderCount, sizeof(int), __FILE__, 1165);
    int playerClass = UnknownBikeClassOf(
        ((UnknownTrackGameModeOptionsFd8*)g_TrackGame->mode.garageOptions)->engineSize);
    srand(ReadClock());
    int i;
    for (i = 0; i < riderCount; i++) {
        int type = ((UnknownKrustyUIModelEntry*)riders)[i].availability;
        if (type < 2 || type > 3)
            usedRiders[i] = -1;
    }
    for (i = 0; i < bikeCount; i++) {
        int type = ((UnknownKrustyUIModelEntry*)manufacturers)[((UnknownKrustyUIBikeEntry*)bikes)[i].manufacturer].availability;
        if (type < 2 || type > 3) {
            usedBikes[i] = -1;
            continue;
        }
        int bikeClass = UnknownBikeClassOf(((UnknownKrustyUIBikeEntry*)bikes)[i].engineSize);
        if (playerClass == bikeClass || (playerClass == 1 && bikeClass == 2)
            || (playerClass == 2 && bikeClass == 1) || (playerClass == 3 && bikeClass == 4)
            || (playerClass == 4 && bikeClass == 3))
            continue;
        usedBikes[i] = -1;
    }
    for (int k = 0; k < count; k++) {
        int index;
        float chance;
        int bikeClass;
        do {
            index = (int)(RandomUnit() * (bikeCount - 1));
        } while (usedBikes[index] == -1);
        int start = index;
        if (!usedBikes[index]) {
            chance = RandomUnit();
            bikeClass = UnknownBikeClassOf(((UnknownKrustyUIBikeEntry*)bikes)[index].engineSize);
            switch (bikeClass) {
            case 1:
            case 2:
                if ((chance <= 0.2f && bikeClass == 2) || (chance > 0.2f && bikeClass == 1))
                    goto found;
                break;
            case 3:
            case 4:
                if (chance > 0.5f)
                    goto found;
                goto found;
            default:
                goto found;
            }
        }
        do {
            if (++index == bikeCount)
                index = 0;
            if (index == start)
                break;
        } while (usedBikes[index]);
    found:
        usedBikes[index] = 1;
        bikeChoices[k] = index;
        do {
            index = (int)(RandomUnit() * (riderCount - 1));
        } while (usedRiders[index] == -1);
        start = index;
        while (usedRiders[index]) {
            if (++index == riderCount)
                index = 0;
            if (index == start)
                break;
        }
        usedRiders[index] = 1;
        riderChoices[k] = index;
    }
    DebugFree(usedBikes, __FILE__, 1299);
    DebugFree(usedRiders, __FILE__, 1300);
}
