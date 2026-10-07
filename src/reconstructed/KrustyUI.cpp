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
void* UnknownFunction47b570(void* block, unsigned int size);

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
    void UnknownFunction4fc660(const Vector3* position);   // 0x004fc660
    void UnknownFunction4fbd10(float a, float b, float c, float d, float e, float f, float g,
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
    void UnknownFunction4a8b10(const char* motion);        // 0x004a8b10: plays a motion
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
    void UnknownFunction49e470(UnknownKrustyUILight* light);        // 0x0049e470: adds a light
    unsigned char field_0x2c[0x11c - 0x2c];
};

class UnknownKrustyUILight : public GameObject {
public:
    explicit UnknownKrustyUILight(int flags);                      // 0x0049deb0
    // 0x0049e230: slot 8, then type, colour, range, position, direction.
    UnknownKrustyUILight* UnknownFunction49e230(void* owner, int type, unsigned int color,
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
        lights->UnknownFunction49e470(light);
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
    void UnknownFunction417670(UnknownKrustyUISoultree* texture, int number); // 0x00417670
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
    int UnknownFunction485c80(const char* resource);
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
int UnknownFunction511970(int format);

// Views of the +0x48 (model), +0x50 (bike) and +0x58 (rider) lists.
struct UnknownKrustyUIModelEntry {
    char field_0x00[0x40];                    // name
    char field_0x40[0x40];                    // model file (a rider's file runs to +0xc0)
    char field_0x80[0x40];                    // UI model file
    UnknownKrustyUICharacter* field_0xc0;     // the loaded UI model (0x00498cf0)
    int field_0xc4;                           // availability (3 for the random choices)
};

struct UnknownKrustyUIBikeEntry {
    int field_0x00;                           // index into +0x48
    char field_0x04[0x44];                    // name
    char field_0x48[0x40];                    // texture
    int field_0x88;
    int field_0x8c;                           // engine size (cc)
    int field_0x90;                           // four-stroke
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
    g_UnknownGlobal56e26c->mode.UnknownFunction523580();
}

// 0x004987f0
KrustyUI::KrustyUI(int flags) : GameObject(flags) {
    field_0x2c = 0;
    field_0x30 = 0;
    field_0x34 = 0;
    field_0x38 = 0;
    field_0x3c = 0;
    field_0x48c = 0;
    field_0x490 = 0;
    field_0x464 = 0;
    field_0x40 = 0;
    field_0x44 = 0;
    field_0x494 = 0;
    field_0x498 = 0;
    field_0x4a8 = 0;
    field_0x4ac = 0;
    field_0x4b0 = 0;
    field_0x4a4 = 0;
    field_0x4a0 = 0;
    field_0x49c = 0;
    field_0x60 = 0;
    field_0x64 = 0;
}


// KrustyUI.h leaves +0x480 as bytes and types +0x46c and +0x470 as the
// selection dialogs' model views; 0x00498cf0 stores the garage character at
// +0x470, the rider at +0x46c and the rider's position (a Vector3) at +0x480.
#define KRUSTYUI_GARAGE (*(UnknownKrustyUICharacter**)&field_0x470)
#define KRUSTYUI_RIDER (*(UnknownKrustyUICharacter**)&field_0x46c)
#define KRUSTYUI_RIDER_POSITION (*(Vector3*)field_0x480)
#define KRUSTYUI_TARGET ((PCRenderTarget*)field_0x18)
#define KRUSTYUI_MODELS ((UnknownKrustyUIModelEntry*)field_0x48)

// 0x00498cf0: builds the garage scene (+0x464) on the first call with 1 or
// -1: camera, lights, the garage and rider models and their shadow; with 2
// or -1 afterwards, loads each bike model with its plate number once.
void KrustyUI::UnknownFunction498cf0(int value) {
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
    ((RenderTarget*)field_0x18)->field_0x34 = 0;
    g_UnknownGlobal56e26c->field_0x2d5_bit2 = 1;
    if (!field_0x464 && (value == 1 || value == -1)) {
        UnknownFunction5053b0(1);
        field_0x498 = 0;
        if (g_UnknownGlobal56e26c->UnknownFunction521970(0x1438, text, 0x80))
            field_0x498 = atoi(text) != 0;
        field_0x498 = g_UnknownGlobal56e26c->UnknownVirtualSlot22("AllowIME", field_0x498);
        if (field_0x498)
            field_0x2c->UnknownFunction4868b0(1);
        ((UnknownKrustyUIGuiView*)field_0x2c)->UnknownFunction485c80("global.dtm");
        field_0x474 = kVec3Zero;
        KRUSTYUI_RIDER_POSITION = kVec3Zero;
        KRUSTYUI_RIDER_POSITION.y += 0.1f;
        field_0x464 = new(__FILE__, 0x105) GameObject(1);
        PCCamera* camera = new(__FILE__, 0x108) PCCamera(1);
        field_0x468 = (Camera*)camera->UnknownVirtualSlot8(field_0x18);
        field_0x464->UnknownFunction469190(field_0x468, -1);
        field_0x468->field_0x1d8 = 0;
        field_0x468->field_0x1d4 = 0;
        UnknownKrustyUILightManager* manager = new(__FILE__, 0x111) UnknownKrustyUILightManager(1);
        lights = (UnknownKrustyUILightManager*)manager->UnknownVirtualSlot8(field_0x18);
        field_0x464->UnknownFunction469190(lights, -1);
        UnknownKrustyUILight* emitter = new(__FILE__, 0x116) UnknownKrustyUILight(1);
        light = emitter->UnknownFunction49e230(field_0x18, 6, 0x606060, 0, 0, 0, 0, 0, 0, 0, 0);
        field_0x464->UnknownFunction469190(light, -1);
        KrustyUIAddLight(lights, light);
        Vector3 position(-1800.0f, 200.0f, 0.0f);
        Vector3 direction = field_0x474 - position;
        emitter = new(__FILE__, 0x121) UnknownKrustyUILight(1);
        light = emitter->UnknownFunction49e230(field_0x18, 4, 0xffffff, &position, &direction, 0, 0, 0,
                                               0, 0, 0);
        field_0x464->UnknownFunction469190(light, -1);
        KrustyUIAddLight(lights, light);
        s488 = field_0x474;
        s488.y += 8.0f;
        s488.x += 12.0f;
        s408 = field_0x474 - s488;
        s448 = s488;
        s448.x -= 24.0f;
        s438 = field_0x474 - s448;
        emitter = new(__FILE__, 0x130) UnknownKrustyUILight(1);
        light = emitter->UnknownFunction49e230(field_0x18, 1, 0x8c8c8c, &s488, &s408, 0, 0, 0, 0, 0, 0);
        field_0x464->UnknownFunction469190(light, -1);
        KrustyUIAddLight(lights, light);
        UnknownKrustyUIContext context = *(UnknownKrustyUIContext*)&g_UnknownGlobal56e26c->field_0x1c;
        context.field_0x14 = 0;
        if (!g_UnknownGlobal56e26c->field_0x2d0) {
            // Free video memory: total less the frame buffers, or a fixed 4 MB
            // (2 MB below 800x600) off the total outside NT without the 0x400 cap.
            int memory = KRUSTYUI_TARGET->field_0x04->field_0x54;
            int free = memory - UnknownFunction511970(KRUSTYUI_TARGET->field_0x28) *
                                    (KRUSTYUI_TARGET->field_0x14 + 1) * KRUSTYUI_TARGET->field_0x10 *
                                    KRUSTYUI_TARGET->field_0x0c;
            if (g_UnknownGlobal56e26c->field_0x424.platformId != 2 &&
                !(KRUSTYUI_TARGET->field_0x04->field_0x1b8 & 0x400))
                free = (KRUSTYUI_TARGET->field_0x164 & 0x4000) ? memory - 0x400000 : memory - 0x200000;
            if (!g_UnknownGlobal56e26c->field_0x0c->field_0x9f0 && free < 0x500000)
                context.field_0x14 = 1;
        }
        KRUSTYUI_GARAGE = new(__FILE__, 0x174) UnknownKrustyUICharacter(1);
        field_0x464->UnknownFunction469190(
            KRUSTYUI_GARAGE->CharacterVirtualSlot11(field_0x18, "UIgarage.mcf", lights, &context, 1, 1), -1);
        KRUSTYUI_GARAGE->field_0x1a0->UnknownFunction4444c0(0);
        KRUSTYUI_GARAGE->field_0x1a0->UnknownFunction4fc660(&field_0x474);
        KRUSTYUI_GARAGE->field_0x1a0->UnknownFunction4fbd10(0.0f, 0.0f, -1.0f, 0.0f, 1.0f, 0.0f, 0.0f, 1);
        KRUSTYUI_GARAGE->UnknownFunction4a8b10("Spin");
        KRUSTYUI_GARAGE->UnknownFunction4a6bb0(0.03f, 0, 0);
        UnknownKrustyUIShadow* projected = new(__FILE__, 0x182) UnknownKrustyUIShadow(1);
        shadow = projected->UnknownFunction4da7b0(field_0x18, g_UnknownGlobal56e26c->field_0x3c);
        UnknownKrustyUIShadowLink* link = new(__FILE__, 0x186) UnknownKrustyUIShadowLink(1);
        shadowLink = link->UnknownFunction4468b0(field_0x18, KRUSTYUI_GARAGE->field_0x1a0, shadow);
        field_0x464->UnknownFunction469190(shadowLink, -1);
        if (shadow)
            field_0x464->UnknownFunction469190(shadow, -1);
        KRUSTYUI_RIDER = new(__FILE__, 0x194) UnknownKrustyUICharacter(1);
        field_0x464->UnknownFunction469190(
            KRUSTYUI_RIDER->CharacterVirtualSlot11(field_0x18, "UIRider.mcf", lights,
                                                   &g_UnknownGlobal56e26c->field_0x1c, 1, 1), -1);
        KRUSTYUI_RIDER->field_0x1a0->UnknownFunction4444c0(1);
        KRUSTYUI_RIDER->field_0x1a0->UnknownFunction4fc660(&KRUSTYUI_RIDER_POSITION);
        KRUSTYUI_RIDER->field_0x1a0->UnknownFunction4fbd10(0.0f, 0.0f, -1.0f, 0.0f, 1.0f, 0.0f, 0.0f, 1);
        KRUSTYUI_RIDER->UnknownFunction4a8b10("WaitR");
        KRUSTYUI_RIDER->UnknownFunction4a6bb0(0.03f, 0, 0);
        ((RenderTarget*)field_0x18)->UnknownFunction4e8cf0(0);
        pending = 1;
        done = 0;
    }
    if (pending && !done && (value == 2 || value == -1)) {
        UnknownKrustyUIPlatePainter painter(g_UnknownGlobal56e26c->field_0x1c);
        for (int i = 0; i < field_0x4c; i++) {
            sprintf(text, "Animations\\UIbike\\%s", KRUSTYUI_MODELS[i].field_0x80);
            KRUSTYUI_MODELS[i].field_0xc0 = new(__FILE__, 0x1b4) UnknownKrustyUICharacter(0);
            field_0x464->UnknownFunction469190(
                KRUSTYUI_MODELS[i].field_0xc0->CharacterVirtualSlot11(field_0x18, text, lights,
                                                                      &g_UnknownGlobal56e26c->field_0x1c,
                                                                      1, 1), -1);
            UnknownKrustyUICharacter* bike = KRUSTYUI_MODELS[i].field_0xc0;
            bike->field_0x1a0->UnknownFunction4444c0(1);
            bike->field_0x1a0->UnknownFunction4fc660(&field_0x474);
            bike->field_0x1a0->UnknownFunction4fbd10(0.0f, 0.0f, -1.0f, 0.0f, 1.0f, 0.0f, 0.0f, 1);
            bike->UnknownFunction4a8b10("WaitB");
            bike->UnknownFunction4a6bb0(0.03f, 0, 0);
            painter.UnknownFunction417670(bike->field_0x1a0, g_UnknownGlobal56e26c->mode.field_0x1bcc);
        }
        if (shadow) {
            shadow->UnknownFunction4dab00(KRUSTYUI_MODELS[0].field_0xc0->field_0x1a0);
            shadow->UnknownFunction4dab00(KRUSTYUI_RIDER->field_0x1a0);
            s468 = field_0x474;
            s468.y += 40.0f;
            s468.x += 15.0f;
            s478 = field_0x474 - s468;
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
    UnknownFunction4999b0();
    if (field_0x48)
        operator delete(field_0x48, __FILE__, 1306);
    if (field_0x50)
        operator delete(field_0x50, __FILE__, 1307);
    if (field_0x58)
        operator delete(field_0x58, __FILE__, 1308);
    if (field_0x60)
        operator delete(field_0x60, __FILE__, 1309);
}

// 0x004999b0
void KrustyUI::UnknownFunction4999b0() {
    UnknownFunction5053b0(0);
    if (!g_UnknownGlobal56e26c->field_0x2d5_bit1)
        field_0x2c->UnknownFunction485d50();
    if (field_0x464) {
        field_0x464->Release();
        field_0x464 = 0;
    }
}

// 0x0049b530: tells the "ProgressBar" control 1 (0x0047b370).
void KrustyUI::UnknownFunction49b530() {
    if (field_0x490) {
        UIProgressBar* bar = static_cast<UIProgressBar*>(field_0x490->UnknownFunction46ebf0("ProgressBar", 0));
        if (bar)
            bar->UnknownFunction47b370(1);
    }
}

// 0x0049bb80
void KrustyUI::UnknownFunction49bb80() {
    if (field_0x60)
        operator delete(field_0x60, __FILE__, 1453);
    field_0x60 = 0;
    field_0x64 = 0;
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
        g_UnknownGlobal56e26c->field_0x08->isHost = 1;
    return 0;
}

// 0x00499980: also hands the value to the +0x464 scene.
int KrustyUI::UnknownVirtualSlot25(void* value) {
    GameObject::UnknownVirtualSlot25(value);
    if (field_0x464)
        field_0x464->UnknownVirtualSlot25(value);
    return 1;
}

// 0x004999f0
void KrustyUI::UnknownFunction4999f0(GameObject* parent) {
    if (field_0x464) {
        parent->UnknownFunction469190(field_0x464, -1);
        ((RenderTarget*)field_0x18)->UnknownFunction4e8cf0(field_0x468);
    }
}

// 0x00499a20
void KrustyUI::UnknownFunction499a20() {
    if (field_0x464) {
        field_0x464->UnknownFunction4691f0();
        ((RenderTarget*)field_0x18)->UnknownFunction4e8cf0(0);
    }
}

// 0x00499b00
void KrustyUI::UnknownFunction499b00() {
    if (field_0x2c)
        field_0x2c->UnknownFunction486630(1);
}

// 0x00499b10
void KrustyUI::UnknownFunction499b10() {
    if (field_0x2c)
        field_0x2c->UnknownFunction486630(0);
}

// 0x00499b20: opens menu `menu` (each id has its dialog class).
void KrustyUI::UnknownFunction499b20(int menu) {
    UIDialog* dialog;
    if (menu == 0xd0)
        field_0x34 = field_0x3c;
    field_0x3c = menu;
    switch (menu) {
    case 100:
        dialog = new(__FILE__, 674) MainDlg;
        field_0x2c->UnknownFunction485a70(dialog, menu, 2, 0, 0, 0, 0, 1);
        g_UnknownGlobal56e26c->UnknownFunction468880();
        break;
    case 101:
        dialog = new(__FILE__, 681) SinglePlayerDlg;
        field_0x2c->UnknownFunction485a70(dialog, menu, 2, 0, 0, 0, 0, 1);
        break;
    case 102:
        dialog = new(__FILE__, 685) OptionsDlg;
        field_0x2c->UnknownFunction485a70(dialog, menu, 2, 0, 0, 0, 0, 1);
        break;
    case 104:
        if (g_UnknownGlobal56e26c->mode.field_0xa4c != g_UnknownGlobal56e26c->field_0x0c->field_0x0c)
            g_UnknownGlobal56e26c->UnknownVirtualSlot19(g_UnknownGlobal56e26c->mode.field_0xa4c);
        dialog = new(__FILE__, 695) LoadingDlg(1);
        field_0x2c->UnknownFunction485a70(dialog, menu, 2, 0, 0, 0, 0, 1);
        break;
    case 0x85c:
    case 0x85d:
        dialog = new(__FILE__, 727) HostJoinDlg;
        field_0x2c->UnknownFunction485a70(dialog, menu, 4, 0, (int)field_0x2c->UnknownFunction485df0(), 0, 0, 1);
        break;
    case 0x866:
    case 0x867:
    case 0x868:
        dialog = new(__FILE__, 733) MultiPlayerDlg;
        field_0x2c->UnknownFunction485a70(dialog, menu, 2, 0, 0, 0, 0, 1);
        break;
    case 0xd8:
        dialog = new(__FILE__, 737) SerialPopupDlg;
        field_0x2c->UnknownFunction485a70(dialog, 0xd8, 4, 0, (int)field_0x2c->UnknownFunction485df0(), 0, 0, 1);
        break;
    case 0xd9:
        dialog = new(__FILE__, 741) TCPAddressDlg;
        field_0x2c->UnknownFunction485a70(dialog, menu, 4, 0, (int)field_0x2c->UnknownFunction485df0(), 0, 0, 1);
        break;
    case 0x88f:
        if (g_UnknownGlobal56e26c->field_0x3444) {
            dialog = new(__FILE__, 747) PCCentralDlg;
            field_0x2c->UnknownFunction485a70(dialog, 0, 2, 0, 0, 0, 0, 1);
        }
        break;
    case 0x88e: {
        UnknownTrackGameObject3444* circuit = g_UnknownGlobal56e26c->field_0x3444;
        if (circuit) {
            if (circuit->field_0x1285[circuit->field_0x40].field_0x08 == circuit->field_0x44) {
                circuit->UnknownFunction4d41a0();
                UnknownFunction4d4ba0();
                return;
            }
            if (g_UnknownGlobal56e26c->mode.field_0x27f8.field_0x00 == 1) {
                dialog = new(__FILE__, 760) PCLastRaceDlg;
                field_0x2c->UnknownFunction485a70(dialog, 0x88e, 2, 0, 0, 0, 0, 1);
            } else {
                dialog = new(__FILE__, 763) PCCentralDlg;
                field_0x2c->UnknownFunction485a70(dialog, 0, 2, 0, 0, 0, 0, 1);
            }
        } else if (g_UnknownGlobal56e26c->field_0x18 == 1) {
            dialog = new(__FILE__, 767) SinglePlayerDlg;
            field_0x2c->UnknownFunction485a70(dialog, 0x88e, 2, 0, 0, 0, 0, 1);
        } else {
            dialog = new(__FILE__, 769) MultiPlayerDlg;
            field_0x2c->UnknownFunction485a70(dialog, 0x88e, 2, 0, 0, 0, 0, 1);
        }
        break;
    }
    case 0xfc:
        dialog = new(__FILE__, 774) WaitOrCallDlg;
        field_0x2c->UnknownFunction485a70(dialog, menu, 4, 0, (int)field_0x2c->UnknownFunction485df0(), 0, 0, 1);
        break;
    case 0x910:
    case 0x911:
        dialog = new(__FILE__, 788) TrackRecordDlg;
        field_0x2c->UnknownFunction485a70(dialog, menu, 2, 0, 0, 0, 0, 1);
        break;
    case 0xbb9:
    case 0xbba:
    case 0xbbb:
        dialog = new(__FILE__, 798) UserNameDlg;
        field_0x2c->UnknownFunction485a70(dialog, menu, 0xc, 0, (int)field_0x2c->UnknownFunction485df0(), 0, 0, 1);
        break;
    case 0x12f:
        dialog = new(__FILE__, 802) RemoveProfileDlg;
        field_0x2c->UnknownFunction485a70(dialog, menu, 4, 0, (int)field_0x2c->UnknownFunction485df0(), 0, 0, 1);
        break;
    case 0x1f9:
        dialog = new(__FILE__, 806) ConnectErrorDlg;
        field_0x2c->UnknownFunction485a70(dialog, 0x1f9, 4, 0, (int)field_0x2c->UnknownFunction485df0(), 0, 0, 1);
        break;
    case 0x1fa:
        dialog = new(__FILE__, 810) PlayerRemovedDlg;
        field_0x2c->UnknownFunction485a70(dialog, 0x1fa, 4, 0, (int)field_0x2c->UnknownFunction485df0(), 0, 0, 1);
        break;
    case 0x191:
        dialog = new(__FILE__, 817) ExitDlg;
        field_0x2c->UnknownFunction485a70(dialog, 0x191, 0x1c, 0, 0, 0, 0, 1);
        break;
    case 0x190:
        dialog = new(__FILE__, 822) ContinueDlg;
        field_0x2c->UnknownFunction485a70(dialog, menu, 0x1c, 0, 0, 0, 0, 1);
        break;
    }
}

// 0x0049a4a0: turns "MediaControl" on, hides the GUI and opens Exit1Dlg.
void KrustyUI::UnknownFunction49a4a0() {
    UnknownFunction468dd0("MediaControl");
    field_0x2c->UnknownFunction486630(0);
    field_0x2c->UnknownFunction485a70(new(__FILE__, 836) Exit1Dlg, 0, 2, 0, 0, 0, 0, 1);
}

// 0x0049a540: reads presets.pb's garage tables, or sets the defaults.
void KrustyUI::UnknownFunction49a540() {
    char key[128];
    UnknownParameterBlock block;
    UnknownTextureStream* stream = new(__FILE__, 851) UnknownTextureStream((int)g_UnknownResourceManager572b44);
    if (stream->UnknownFunction460f50("presets.pb", "r", 0)) {
        block.UnknownFunction4b77a0((UnknownParameterStream*)stream, 0, 1);
        for (int i = 0; i < 5; i++) {
            sprintf(key, "Category_%d", i + 1);
            block.UnknownFunction4b78f0(key);
            block.UnknownFunction4b7f10("RPMLowerLimit", 5000, &field_0x2fc[i]);
            block.UnknownFunction4b7f10("RPMUpperLimit", 10000, &field_0x310[i]);
            field_0x414[i] = (field_0x310[i] - field_0x2fc[i]) / 10;
            block.UnknownFunction4b7f10("HPSum", 400, &field_0x400[i]);
            block.UnknownFunction4b7f10("MinRange", 10, &field_0x43c[i]);
            block.UnknownFunction4b7f10("Weight", 200, &field_0x428[i]);
            int maximum = 0;
            int j;
            for (j = 0; j < 11; j++) {
                sprintf(key, "RPM%05d", field_0x2fc[i] + field_0x414[i] * j);
                block.UnknownFunction4b7f10(key, 10, &field_0x324[i][j]);
                if (maximum <= field_0x324[i][j])
                    maximum = field_0x324[i][j];
            }
            field_0x450[i] = maximum;
            for (int preset = 0; preset < 3; preset++) {
                sprintf(key, "Category_%dPreset_%d", i + 1, preset + 1);
                block.UnknownFunction4b78f0(key);
                for (j = 0; j < 11; j++) {
                    sprintf(key, "RPM%05d", field_0x2fc[i] + field_0x414[i] * j);
                    block.UnknownFunction4b7f10(key, 10, &field_0x68[i][preset][j]);
                }
            }
        }
    } else {
        for (int i = 0; i < 5; i++) {
            field_0x2fc[i] = 5000;
            field_0x310[i] = 10000;
            field_0x400[i] = 100;
            field_0x414[i] = (field_0x310[i] - field_0x2fc[i]) / 10;
            for (int preset = 0; preset < 3; preset++) {
                for (int j = 0; j < 11; j++) {
                    field_0x68[i][preset][j] = 10;
                    field_0x324[i][j] = 40;
                }
            }
        }
    }
    delete stream;
}

// 0x0049b560
int KrustyUI::UnknownFunction49b560(const char* name, char* text, int size) {
    char path[128];
    UNKNOWN_COPY_SIZED(text, name, size);
    if (g_UnknownResourceManager572b44->UnknownFunction4e9360(name, 1))
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
    if (stream->UnknownFunction460f50(path, "r", 0)) {
        delete stream;
        UNKNOWN_COPY_SIZED(text, path, size);
        return 1;
    }
    delete stream;
    int index;
    do {
        index = (int)(RandomUnit() * (field_0x54 - 1));
    } while (((UnknownKrustyUIBikeEntry*)field_0x50)[index].field_0x88
             || ((UnknownKrustyUIModelEntry*)field_0x48)[((UnknownKrustyUIBikeEntry*)field_0x50)[index].field_0x00].field_0xc4 != 3);
    UNKNOWN_COPY_SIZED(text, ((UnknownKrustyUIBikeEntry*)field_0x50)[index].field_0x48, size);
    return 0;
}

// 0x0049b7f0
int KrustyUI::UnknownFunction49b7f0(const char* name, char* text, int size) {
    char path[128];
    UNKNOWN_COPY_SIZED(text, name, size);
    if (g_UnknownResourceManager572b44->UnknownFunction4e9360(name, 1))
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
    if (stream->UnknownFunction460f50(path, "r", 0)) {
        delete stream;
        UNKNOWN_COPY_SIZED(text, path, size);
        return 1;
    }
    delete stream;
    int index;
    do {
        index = (int)(RandomUnit() * (field_0x5c - 1));
    } while (((UnknownKrustyUIModelEntry*)field_0x58)[index].field_0xc0
             || ((UnknownKrustyUIModelEntry*)field_0x58)[index].field_0xc4 != 3);
    UNKNOWN_COPY_SIZED(text, ((UnknownKrustyUIModelEntry*)field_0x58)[index].field_0x40, size);
    return 0;
}

// 0x0049ba70
int KrustyUI::UnknownFunction49ba70(int a, int b, int c, const char* name, int d, int e) {
    field_0x60 = (UnknownKrustyUIEntry*)UnknownFunction47b570(field_0x60, (field_0x64 + 1) * sizeof(UnknownKrustyUIEntry));
    field_0x60[field_0x64].field_0x00 = a;
    field_0x60[field_0x64].field_0x04 = b;
    field_0x60[field_0x64].field_0x08 = c;
    int length = strlen(name);
    int count = length > 63 ? 63 : length;
    strncpy(field_0x60[field_0x64].field_0x14, name, count);
    field_0x60[field_0x64].field_0x14[count] = 0;
    field_0x60[field_0x64].field_0x0c = d;
    field_0x60[field_0x64].field_0x10 = e;
    return ++field_0x64 - 1;
}

// 0x0049bbb0
void KrustyUI::UnknownFunction49bbb0() {
    UnknownTrackGameModeSettings* settings = &g_UnknownGlobal56e26c->mode.field_0x27f8;
    if (settings->field_0x04 == 1 && settings->field_0x2c)
        return;
    UnknownFunction49bc50(0);
    int mode = g_UnknownGlobal56e26c->mode.field_0x27f8.field_0x04;
    if (mode != 0 && mode != 4) {
        if (g_UnknownGlobal56e26c->mode.field_0x27f8.field_0x20 == 5)
            UnknownFunction49bc50(1);
        if (g_UnknownGlobal56e26c->mode.field_0x27f8.field_0x20 == 10)
            UnknownFunction49bc50(2);
    } else {
        if (g_UnknownGlobal56e26c->mode.field_0x27f8.field_0x140 == 5.0f)
            UnknownFunction49bc50(1);
        if (g_UnknownGlobal56e26c->mode.field_0x27f8.field_0x140 == 10.0f)
            UnknownFunction49bc50(2);
    }
}

// 0x0049bc50: selects high-score table `table`, names it after the track
// and adds the racers' results; saves it when one of them made the table.
void KrustyUI::UnknownFunction49bc50(int table) {
    char name[128];
    if (!g_UnknownGlobal56e26c->mode.field_0x27f8.field_0x10)
        return;
    g_UnknownGlobal56e26c->field_0x3400->field_0x00 = table;
    switch (g_UnknownGlobal56e26c->mode.field_0x27f8.field_0x04) {
    case 1:
    case 5:
        sprintf(name, "%s%d", g_UnknownGlobal56e26c->mode.field_0x27f8.field_0x36,
                g_UnknownGlobal56e26c->mode.field_0x27f8.field_0x34);
        break;
    default: {
        int length = strlen(g_UnknownGlobal56e26c->mode.field_0x27f8.field_0x36);
        int count = length > 127 ? 127 : length;
        strncpy(name, g_UnknownGlobal56e26c->mode.field_0x27f8.field_0x36, count);
        name[count] = 0;
        break;
    }
    }
    g_UnknownGlobal56e26c->field_0x3400->UnknownFunction51f0b0(
        (short)g_UnknownGlobal56e26c->mode.field_0x27f8.field_0x04, name);
    int added = 0;
    if (g_UnknownGlobal56e26c->field_0x18 == 1) {
        if (!g_UnknownGlobal56e26c->field_0x3400->UnknownFunction51f3c0(
                (short)g_UnknownGlobal56e26c->mode.field_0x27f8.field_0x04, 0))
            return;
    } else {
        for (int i = 0; i < g_UnknownGlobal56e26c->field_0x18; i++) {
            if (g_UnknownGlobal56e26c->field_0x3400->UnknownFunction51f3c0(
                    (short)g_UnknownGlobal56e26c->mode.field_0x27f8.field_0x04, i))
                added = 1;
        }
        if (!added)
            return;
    }
    g_UnknownGlobal56e26c->field_0x3400->UnknownFunction51f260(
        (short)g_UnknownGlobal56e26c->mode.field_0x27f8.field_0x04, name,
        g_UnknownGlobal56e26c->mode.UnknownFunction524100());
}

// 0x0049b020
void KrustyUI::UnknownFunction49b020(const char** names, int count) {
    int* used = (int*)DebugCalloc(36, sizeof(int), __FILE__, 1120);
    srand(UnknownFunction4bfa80());
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
    operator delete(used, __FILE__, 1157);
}
