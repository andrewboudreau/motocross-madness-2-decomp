#include "KrustyUI.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "DebugAlloc.h"
#include "DirectPlayMessages.h"
#include "GameUi.h"
#include "OptionProcs.h"
#include "Parameterblocks.h"
#include "TextureMap.h"
#include "UIDialog.h"
#include "RenderTarget.h"
#include "TrackGame.h"

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

// Views of the +0x48 (model), +0x50 (bike) and +0x58 (rider) lists.
struct UnknownKrustyUIModelEntry {
    char field_0x00[0x40];                    // name
    char field_0x40[0x40];                    // model file (a rider's file runs to +0xc0)
    char field_0x80[0x40];                    // UI model file
    int field_0xc0;
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
        UnknownGameUiControl* bar = field_0x490->UnknownFunction46ebf0("ProgressBar", 0);
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
