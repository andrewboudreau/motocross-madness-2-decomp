// SelectGamePicProcs.cpp (__FILE__ 0x00573c6c): the multiplayer lobby
// dialogs. Extent 0x004f1740..0x004f975b: the player-record initializers
// 0x004f1740..0x004f1793 open it, the first __FILE__ user is
// 0x004f17a0 (line 0xb4 at 0x004f19f3), the last allocation 0x004f8047;
// MPEventDlg, MPBikeRiderDlg, MPOptionsDlg and MultiPlayerDlg's vtables
// point into it, their type descriptors (0x00573e58...) sit in its .data
// after the __FILE__ literal, and its unwind funclets are
// 0x0054dfd6..0x0054e156. The vector initializers 0x004f9620..0x004f975b
// close it; SelectiveGravityModel.cpp starts at 0x004f9760. Names are
// provisional.

#include "SelectGamePicProcs.h"

#include "DialogEventKind.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "BackgroundImage.h"
#include "Camera.h"
#include "DebugAlloc.h"
#include "EventManager.h"
#include "../krusty2/math/FastMath.h"
#include "MatrixUtil.h"
#include "Net.h"
#include "TextureMap.h"
#include "Track.h"
#include "TrackGame.h"

// A truncating copy into a `size`-byte buffer (as in DlgProcs.cpp).
#define COPY_TEXT(dest, source, size)                              \
    {                                                              \
        int length = strlen(source);                               \
        int copied = length > (size) - 1 ? (size) - 1 : length;    \
        strncpy(dest, source, copied);                             \
        (dest)[copied] = 0;                                        \
    }

// The four vector constants many retail files declare: 0x00689ce8,
// 0x00689cf8, 0x00689db0 and 0x00689cd8. Their initializers
// (0x004f9620..0x004f975b) follow this file's last function rather than
// preceding its first; only this file's code (0x004f7c1f, 0x004f8940,
// 0x004f8a90) reads them, so they are this file's copies.
static const Vector3 kVec3Zero = Vector3(0.0f, 0.0f, 0.0f);
static const Vector3 kVec3XAxis = Vector3(1.0f, 0.0f, 0.0f);
static const Vector3 kVec3YAxis = Vector3(0.0f, 1.0f, 0.0f);
static const Vector3 kVec3ZAxis = Vector3(0.0f, 0.0f, 1.0f);

// Seven player records (0x18 bytes each, 0x00689d08). The constructor stub
// 0x004f1740, its body 0x004f1750, the atexit thunk 0x004f1770 and the
// destructor 0x004f1780 run the vector constructor/destructor iterators with
// PlayerInfoType's constructor (0x004adcc0) and destructor (0x00523c80).
// Ownership (docs/INITIALIZERS.md): .CRT$XCU lists 0x004f1740 right after
// this file's vector initializers 0x004f9620..0x004f9710 and apart from
// SceneManager.cpp's, the array lies inside this file's .bss block, and
// only this file's code reads it.
PlayerInfoType g_UnknownGlobal689d08[7];

// d3drm.dll's D3DRMVectorRotate (declared as in FollowCamera.h): rotates
// `vector` about `axis` by `theta`.
extern "C" Vector3* __stdcall D3DRMVectorRotate(Vector3* result, Vector3* vector, Vector3* axis,
                                                float theta);

// Inline vector helpers of the bike views (as in DlgProcs.cpp).
static inline Vector3 UnknownVectorDifference(const Vector3& a, const Vector3& b) {
    return Vector3(a.x - b.x, a.y - b.y, a.z - b.z);
}

static inline Vector3 UnknownVectorSum(const Vector3& a, const Vector3& b) {
    return Vector3(a.x + b.x, a.y + b.y, a.z + b.z);
}

static inline Vector3& operator*=(Vector3& v, float scale) {
    v.x *= scale;
    v.y *= scale;
    v.z *= scale;
    return v;
}

int g_UnknownGlobal689dbc[7];
int g_UnknownGlobal689dd8[7];
int g_UnknownGlobal689df4;
int g_UnknownGlobal689df8;
int g_UnknownGlobal689dfc;

// The chat message (types 0x85 and 0x92): 0x4e bytes are sent.
struct UnknownChatMessage {
    int field_0x00;
    char field_0x04[0x4a];
};

// A player's state (type 2).
struct UnknownPlayerStateMessage {
    unsigned char field_0x00;
    unsigned char field_0x01;                 // ready
    char field_0x02[0x40];                    // bike name (racer slot +0x40)
    char field_0x42[0x40];                    // bike model name (racer slot +0x00)
    char field_0x82[0x40];                    // rider name (racer slot +0x80)
    unsigned char field_0xc2;                 // racer slot +0xc4
    unsigned char field_0xc3;                 // requested racers
    int field_0xc4;                           // send time
    int field_0xc8;                           // racer slot +0xec
    unsigned char field_0xcc;                 // racer slot +0xf0
    unsigned char field_0xcd;                 // racer slot +0xf4
    int field_0xd0;                           // racer slot +0xc0
};

// The player removal message (type 0x8e).
struct UnknownKickMessage {
    int field_0x00;
    int field_0x04;                           // player id
};

// A racer's random grid key (12 bytes; 0x004f2340 sorts them).
struct UnknownGridDraw {
    int field_0x00;                           // player id
    float field_0x04;                         // random key
    char field_0x08;                          // racer index
};

// 0x004f2080: qsort order of the grid entries, by +0x04, largest first
// (0x004f2340, in the near-miss sample, sorts with it and 0x004f20a0).
int UnknownFunction4f2080(const void* a, const void* b) {
    unsigned char x = ((const UnknownTrackGameModeEntry*)a)->field_0x04;
    unsigned char y = ((const UnknownTrackGameModeEntry*)b)->field_0x04;
    if (x > y)
        return -1;
    return x != y;
}

// 0x004f20a0: qsort order of the grid draws, by the key, largest first.
int UnknownFunction4f20a0(const void* a, const void* b) {
    const UnknownGridDraw* x = (const UnknownGridDraw*)a;
    const UnknownGridDraw* y = (const UnknownGridDraw*)b;
    if (x->field_0x04 > y->field_0x04)
        return -1;
    if (x->field_0x04 == y->field_0x04)
        return 0;
    return 1;
}

// 0x004f20d0
void MultiPlayerDlg::FillRacerSlots() {
    int player = g_TrackGame->network->localPlayer;
    g_TrackGame->field_0x18 = g_TrackGame->mode.field_0x27f8.field_0x35;
    g_TrackGame->field_0x3424 = g_TrackGame->mode.field_0x27f8.field_0x28;
    g_TrackGame->mode.field_0x1be0 = g_TrackGame->field_0x3424 + g_TrackGame->field_0x18;
    for (int i = 0; i < g_TrackGame->field_0x18 - 1; i++) {
        g_TrackGame->mode.field_0x1be4[i + 1].field_0xd4 = g_UnknownGlobal689d08[i].id;
        g_TrackGame->mode.field_0x1be4[i + 1].field_0xd8 = 0;
        int length = strlen(g_UnknownGlobal689d08[i].name);
        int count = length > 0xf ? 0xf : length;
        strncpy(g_TrackGame->mode.field_0x1be4[i + 1].field_0xdc, g_UnknownGlobal689d08[i].name, count);
        g_TrackGame->mode.field_0x1be4[i + 1].field_0xdc[count] = 0;
    }
    g_TrackGame->mode.field_0x1be4[0].field_0xd4 = player;
    {
        int length = strlen(g_TrackGame->mode.field_0x00);
        int count = length > 0xf ? 0xf : length;
        strncpy(g_TrackGame->mode.field_0x1be4[0].field_0xdc, g_TrackGame->mode.field_0x00, count);
        g_TrackGame->mode.field_0x1be4[0].field_0xdc[count] = 0;
    }
    g_TrackGame->mode.field_0x1be4[0].field_0xd8 = 0;
    g_TrackGame->mode.field_0x1be4[0].UnknownFunction521fb0(
        g_TrackGame->mode.field_0x1974.field_0x00, g_TrackGame->mode.field_0x1974.field_0x40,
        g_TrackGame->mode.field_0x1974.field_0x80);
    g_TrackGame->mode.field_0x1be4[0].field_0xc4 =
        g_TrackGame->mode.field_0x1974.field_0xc4;
    if (g_TrackGame->mode.field_0x1bd0 > 0 && g_TrackGame->mode.field_0x1bd0 < 0x65)
        g_TrackGame->mode.field_0x1bcc = g_TrackGame->mode.field_0x1bd0;
    g_TrackGame->mode.field_0x1be4[0].field_0xc0 = g_TrackGame->mode.field_0x1bcc;
    int racer = g_UnknownGlobal689df8 + 1;
    for (int j = 0; j < 8; j++) {
        for (int k = 0; k < field_0x7f68.field_0x00[j].grantedRacers; k++) {
            g_TrackGame->mode.field_0x1be4[racer].field_0xd4 = field_0x7f68.field_0x00[j].playerId;
            g_TrackGame->mode.field_0x1be4[racer].field_0xd8 = racer;
            racer++;
        }
        if (field_0x7f68.field_0x00[j].playerId == player)
            g_TrackGame->mode.field_0x27f8.field_0x24 = field_0x7f68.field_0x00[j].grantedRacers;
    }
    g_TrackGame->network->SetSessionJoinable(0);
    if (g_TrackGame->ui->field_0x2c) {
        if (g_TrackGame->mode.field_0x2dbc) {
            if (g_TrackGame->mode.field_0x27f8.field_0x04 != 4) {
                g_TrackGame->field_0x342c = 1;
                g_TrackGame->field_0x3428 = 0;
            } else {
                g_TrackGame->mode.field_0x2dbc = 0;
            }
        }
        UnknownFunction4536e0();
        UnknownVirtualSlot26();
    }
}

// 0x00573c0c
static int s_UnknownStatic573c0c = 2;

// 0x004f2b90
void MultiPlayerDlg::SendSettings() {
    UnknownLobbySlotTable* slots = &field_0x7f68;
    g_TrackGame->mode.field_0x1bd8 = slots->UnknownFunction4f2fe0(g_UnknownGlobal689df8 + 1, g_TrackGame->mode.field_0x27f8.field_0x28);
    if (g_TrackGame->mode.field_0x1bd8 < g_TrackGame->mode.field_0x27f8.field_0x28)
        g_TrackGame->mode.field_0x27f8.field_0x28 = g_TrackGame->mode.field_0x1bd8;
    if (field_0x7f58) {
        UIListBox* list = static_cast<UIListBox*>(field_0x7f58->FindControl("OpponentsListBox", 0));
        if (list && list->rowCount - 1 != g_TrackGame->mode.field_0x1bd8) {
            int selection = list->GetSelectedRow();
            list->RemoveAllRows();
            for (int i = 0; i <= g_TrackGame->mode.field_0x1bd8; i++) {
                char text[0x80];
                sprintf(text, "%d", i);
                list->AddRow(text, i, 0);
            }
            if (g_TrackGame->mode.field_0x1bd8 < selection)
                list->SelectRow(g_TrackGame->mode.field_0x1bd8);
            else
                list->SelectRow(selection);
        }
    }
    if (++s_UnknownStatic573c0c >= 2) {
        g_TrackGame->mode.field_0x27f8.field_0x35 = g_UnknownGlobal689df8 + 1;
        g_TrackGame->field_0x18 = g_TrackGame->mode.field_0x27f8.field_0x35;
        g_TrackGame->field_0x3424 = g_TrackGame->mode.field_0x27f8.field_0x28;
        g_TrackGame->mode.field_0x1be0 = g_TrackGame->field_0x3424 + g_TrackGame->field_0x18;
        if (field_0x7f58)
            field_0x7f58->UpdateBoundValues(1);
        if (field_0x7f64)
            field_0x7f64->UpdateBoundValues(1);
        TrackGame* game = g_TrackGame;
        UnknownLobbySettingsMessage message;
        message.minutes = (int)game->mode.field_0x27f8.field_0x140;
        message.eventType = game->mode.field_0x27f8.field_0x04;
        message.raceMode = game->mode.field_0x27f8.field_0x00;
        message.field_0x2d78 = game->mode.field_0x27f8.field_0x08;
        message.laps = game->mode.field_0x27f8.field_0x20;
        message.opponents = game->mode.field_0x27f8.field_0x28;
        message.players = game->mode.field_0x27f8.field_0x35;
        message.races = game->mode.field_0x27f8.field_0x0c;
        message.tagBall = game->mode.field_0x27f8.field_0x144;
        message.stuntMode = game->mode.field_0x27f8.field_0x148;
        message.treeCollision = game->mode.field_0x27f8.field_0x10;
        message.riderCollision = game->mode.field_0x27f8.field_0x14;
        message.fastFinishes = game->mode.field_0x27f8.field_0x18;
        message.bikeClass = game->mode.field_0x27f8.field_0x1c;
        message.field_0x0c = game->mode.field_0x27f8.field_0x138;
        message.field_0x10 = game->mode.field_0x27f8.field_0x13c;
        message.detail = game->mode.field_0x94;
        message.trackNumber = game->mode.field_0x27f8.field_0x34;
        memcpy(message.slots, slots->field_0x00, sizeof(message.slots));
        message.trackName[0] = 0;
        strncat(message.trackName, game->mode.field_0x27f8.field_0x36, 0x3f);
        g_TrackGame->network->Send(0xf, &message, sizeof(message),
                                                                   g_TrackGame->network->localPlayer, 0);
        s_UnknownStatic573c0c = 0;
    }
}

// 0x004f2ec0
int MultiPlayerDlg::UnknownFunction4f2ec0() {
    UIControl* button = FindControl("ButRdyUser", 1);
    button->SetFontColor(0x808080);
    return g_TrackGame->network->CreateLocalPlayer(g_TrackGame->mode.field_0x00) != 0;
}

// 0x004f2f00
UnknownLobbySlotTable::UnknownLobbySlotTable() {
    for (int i = 0; i < 8; i++) {
        field_0x00[i].playerId = 0;
        field_0x00[i].requestedRacers = 0;
        field_0x00[i].grantedRacers = 0;
    }
}

// 0x004f2f20
void UnknownLobbySlotTable::SetSlot(int id, int value) {
    int i;
    for (i = 0; i < 8; i++) {
        if (field_0x00[i].playerId == id) {
            if (value != -1)
                field_0x00[i].requestedRacers = value;
            return;
        }
    }
    for (i = 0; i < 8; i++) {
        if (field_0x00[i].playerId == 0) {
            field_0x00[i].playerId = id;
            if (value != -1)
                field_0x00[i].requestedRacers = value;
            return;
        }
    }
}

// 0x004f2f80
void UnknownLobbySlotTable::FreeSlot(int id) {
    for (int i = 0; i < 8; i++) {
        if (field_0x00[i].playerId == id) {
            field_0x00[i].playerId = 0;
            field_0x00[i].requestedRacers = 0;
            field_0x00[i].grantedRacers = 0;
            return;
        }
    }
}

// 0x004f2fc0: qsort order, by the request, smallest first.
static int UnknownFunction4f2fc0(const void* a, const void* b) {
    int x = ((const UnknownLobbySlot*)a)->requestedRacers;
    int y = ((const UnknownLobbySlot*)b)->requestedRacers;
    if (x < y)
        return -1;
    return x != y;
}

// 0x004f2fe0: grants the first `count` slots up to `limit` racers in
// turn; returns the requested total, at most 8 - `count`.
int UnknownLobbySlotTable::UnknownFunction4f2fe0(int count, int limit) {
    if (count >= 8)
        return 0;
    qsort(field_0x00, count, sizeof(UnknownLobbySlot), UnknownFunction4f2fc0);
    int total = 0;
    for (int i = 0; i < 8; i++) {
        field_0x00[i].grantedRacers = 0;
        total += field_0x00[i].requestedRacers;
    }
    if (total + count >= 8)
        total = 8 - count;
    if (limit) {
        int granted = 0;
        int more;
        do {
            more = 0;
            for (int j = 0; j < count; j++) {
                if (field_0x00[j].requestedRacers > field_0x00[j].grantedRacers) {
                    field_0x00[j].grantedRacers++;
                    granted++;
                    more = 1;
                    if (granted >= limit)
                        return total;
                }
            }
        } while (more);
    }
    return total;
}

// 0x004f3080
void FillTrackList(int a, const char* picture, const char* listName, int* value, UIDialog* dialog, int append) {
    char name[0x80];
    if (!dialog)
        dialog = (UIDialog*)g_TrackGame->ui->field_0x2c->FindInputDialog();
    name[0] = 0;
    switch (g_TrackGame->mode.field_0x27f8.field_0x04) {
    case 3:
        FillFileList(g_TrackGame->mode.field_0x25e4, "Teraform\\SX", "*.env", "env", a, picture, listName,
                     name, value, dialog, 0, 0);
        break;
    case 1:
        FillFileList(g_TrackGame->mode.field_0x25e4, "Teraform\\Baja", "*.env", "env", a, picture, listName,
                     name, value, dialog, g_TrackGame->mode.field_0x9f4[1], 0);
        break;
    case 5:
        FillFileList(g_TrackGame->mode.field_0x25e4, "Teraform\\Enduro", "*.env", "env", a, picture,
                     listName, name, value, dialog, g_TrackGame->mode.field_0x9f4[5], 0);
        break;
    case 0:
    case 4:
        FillFileList(g_TrackGame->mode.field_0x25e4, (const char*)g_TrackGame->mode.field_0x6a0,
                     "*.env", "env", a, picture, listName, name, value, dialog, 0, append);
        break;
    case 2:
        FillFileList(g_TrackGame->mode.field_0x25e4, "Teraform\\National", "*.env", "env", a, picture,
                     listName, name, value, dialog, 0, 0);
        break;
    }
}

// 0x004f3260: enables the controls for the ready state.
void MultiPlayerDlg::UnknownFunction4f3260() {
    if (g_TrackGame->network->isHost) {
        if (field_0x7f58)
            field_0x7f58->UnknownFunction4f69d0();
        if (!strcmp(g_TrackGame->mode.field_0x27f8.field_0x36, ""))
            EnableGroup(0x514, 1);
        EnableGroup(0x3e8, 1);
    } else {
        if (UnknownBikeClassOf(((UnknownOptGarageSettings*)g_TrackGame->mode.field_0xfd8)->engineSize) >
                g_TrackGame->mode.field_0x27f8.field_0x1c &&
            g_UnknownGlobal689df4)
            UnknownFunction4f57c0(1);
        EnableGroup(0x514, 0);
        EnableGroup(0x3e8, 0);
    }
    if (g_UnknownGlobal689df4) {
        FindControl("Options", 1)->UnknownVirtualSlot49(0);
        FindControl("Joystick", 1)->UnknownVirtualSlot49(0);
        FindControl("Help", 1)->UnknownVirtualSlot49(0);
        if (field_0x7f5c) {
            field_0x7f5c->FindControl("ButWrench", 0)->UnknownVirtualSlot49(0);
            field_0x7f5c->FindControl("DDLBikes", 0)->UnknownVirtualSlot49(0);
            field_0x7f5c->FindControl("BikeLeft", 0)->Show(0, 1);
            field_0x7f5c->FindControl("BikeRight", 0)->Show(0, 1);
            field_0x7f5c->FindControl("DDLEngineSize", 0)->UnknownVirtualSlot49(0);
            static_cast<UIDropDownList*>(field_0x7f5c->FindControl("DDLEngineSize", 0))->UnknownInlineButton()->Show(0, 1);
            if (g_TrackGame->network->isHost) {
                field_0x7f5c->FindControl("LargestOpponentDropDown", 0)->UnknownVirtualSlot49(0);
                static_cast<UIDropDownList*>(field_0x7f5c->FindControl("LargestOpponentDropDown", 0))->UnknownInlineButton()
                    ->Show(0, 1);
            }
        }
    } else {
        FindControl("Options", 1)->UnknownVirtualSlot49(1);
        FindControl("Joystick", 1)->UnknownVirtualSlot49(1);
        FindControl("Help", 1)->UnknownVirtualSlot49(1);
        if (field_0x7f5c) {
            field_0x7f5c->FindControl("ButWrench", 0)->UnknownVirtualSlot49(1);
            field_0x7f5c->FindControl("DDLBikes", 0)->UnknownVirtualSlot49(1);
            field_0x7f5c->FindControl("BikeLeft", 0)->Show(1, 1);
            field_0x7f5c->FindControl("BikeRight", 0)->Show(1, 1);
            field_0x7f5c->FindControl("DDLEngineSize", 0)->UnknownVirtualSlot49(1);
            static_cast<UIDropDownList*>(field_0x7f5c->FindControl("DDLEngineSize", 0))->UnknownInlineButton()->Show(1, 1);
            if (g_TrackGame->network->isHost) {
                field_0x7f5c->FindControl("LargestOpponentDropDown", 0)->UnknownVirtualSlot49(1);
                static_cast<UIDropDownList*>(field_0x7f5c->FindControl("LargestOpponentDropDown", 0))->UnknownInlineButton()
                    ->Show(1, 1);
            }
        }
    }
}

// 0x004f3620
void MultiPlayerDlg::UnknownFunction4f3620() {
    char text[0x80];
    char name[0x80];
    if (g_TrackGame->network->isHost) {
        ShowGroup(0x514, 1);
        FindControl("Start", 0)->UnknownVirtualSlot49(1);
    } else {
        ShowGroup(0x514, 0);
    }
    UnknownFunction4f3260();
    g_TrackGame->LoadResourceString(0x610, text, 0x80);
    UIControl* button = FindControl("ButRdyUser", 1);
    button->SetText(text);
    if (g_TrackGame->network->isHost)
        g_TrackGame->LoadResourceString(0x925, text, 0x80);
    for (int i = 0; i < 7; i++) {
        sprintf(name, "ButRdyPlayer%d", i + 2);
        button = FindControl(name, 1);
        button->SetText(text);
    }
}

// 0x004f38a0
void MultiPlayerDlg::SendChatLine() {
    UnknownChatMessage message;
    UIEditBox* edit = static_cast<UIEditBox*>(FindControl("EditChat", 0xb));
    int length = strlen("");
    int count = length > 0x49 ? 0x49 : length;
    strncpy(message.field_0x04, "", count);
    message.field_0x04[count] = 0;
    edit->GetEditText(message.field_0x04, 0x4a);
    if (strcmp(message.field_0x04, "")) {
        NetworkInterface* network = g_TrackGame->network;
        int player = network->localPlayer;
        network->Send(0x85, &message, 0x4e, player, 0);
        edit->SetEditText("");
        AddChatLine(player, message.field_0x04);
    }
    guiUser->UnknownFunction487790((UnknownGuiControl*)edit, 0, 0);
}

// 0x004f3980
void MultiPlayerDlg::SendSystemChatLine(const char* text) {
    UnknownChatMessage message;
    int length = strlen(text);
    int count = length > 0x49 ? 0x49 : length;
    strncpy(message.field_0x04, text, count);
    message.field_0x04[count] = 0;
    g_TrackGame->network->Send(
        0x92, &message, 0x4e, g_TrackGame->network->localPlayer, 0);
    AddChatLine(-1, message.field_0x04);
    guiUser->UnknownFunction487790((UnknownGuiControl*)FindControl("EditChat", 0xb), 0, 0);
}

// 0x004f3a10
void MultiPlayerDlg::RemoveLobbyPlayer(int index) {
    int player = g_UnknownGlobal689d08[index].id;
    if (player && player != g_TrackGame->network->localPlayer) {
        UnknownKickMessage message;
        NetworkInterface* network = g_TrackGame->network;
        message.field_0x04 = player;
        network->Send(0x8e, &message, sizeof(message), network->localPlayer, player);
        field_0x7f68.FreeSlot(message.field_0x04);
    }
}

// 0x00689e04: only written (0x004f4520).
int g_UnknownGlobal689e04;

// 0x004f4520: the lobby's dialog procedure.
void MultiPlayerDlg::UnknownVirtualSlot29(UnknownDialogEvent* event) {
    char name[0x10];
    char text[0x80];
    UnknownPlayerStateMessage state;
    char prompt[1024];
    int i;
    switch (event->kind) {
    case kDialogInit: {
        field_0x7f58 = 0;
        field_0x7f5c = 0;
        field_0x7f60 = 0;
        field_0x7f64 = 0;
        field_0x80d0[0] = 0;
        UIMultiState* tab = static_cast<UIMultiState*>(FindControl("EventTab", 4));
        tab->SetStateTextFromResource(0, g_TrackGame->resourceInstance, 0x13e3);
        tab->SetStateTextFromResource(1, g_TrackGame->resourceInstance, 0x13e3);
        tab->SetTextAlign(0x22);
        tab = static_cast<UIMultiState*>(FindControl("BikeRiderTab", 4));
        tab->SetStateTextFromResource(0, g_TrackGame->resourceInstance, 0x13e4);
        tab->SetStateTextFromResource(1, g_TrackGame->resourceInstance, 0x13e4);
        tab->SetTextAlign(0x22);
        tab = static_cast<UIMultiState*>(FindControl("RaceInfoTab", 4));
        tab->SetStateTextFromResource(0, g_TrackGame->resourceInstance, 0x13e5);
        tab->SetStateTextFromResource(1, g_TrackGame->resourceInstance, 0x13e5);
        tab->SetTextAlign(0x22);
        tab = static_cast<UIMultiState*>(FindControl("OptionsTab", 4));
        tab->SetStateTextFromResource(0, g_TrackGame->resourceInstance, 0x1468);
        tab->SetStateTextFromResource(1, g_TrackGame->resourceInstance, 0x1468);
        tab->SetTextAlign(0x22);
        memcpy(&g_TrackGame->mode.field_0x27f8, &g_TrackGame->mode.field_0x2bd0,
               sizeof(g_TrackGame->mode.field_0x27f8));
        memcpy(g_TrackGame->mode.field_0xfd8, g_TrackGame->mode.field_0x1090,
               sizeof(g_TrackGame->mode.field_0xfd8));
        memcpy(&g_TrackGame->mode.field_0x1974, g_TrackGame->mode.field_0x1b04,
               sizeof(g_TrackGame->mode.field_0x1974));
        strcpy(builtForTrack, "");
        field_0x80cc = 0;
        g_TrackGame->fullNetPacketIntervalSeconds =
            g_TrackGame->GetRegistryInt("IntervalBetweenFullPacketsMS", -1000) * 0.001f;
        if (g_TrackGame->fullNetPacketIntervalSeconds < 0.0f)
            g_TrackGame->fullNetPacketIntervalSeconds = 2.0f;
        else if (g_TrackGame->fullNetPacketIntervalSeconds < 2.0)
            g_TrackGame->fullNetPacketIntervalSeconds = 2.0f;
        else if (g_TrackGame->fullNetPacketIntervalSeconds > 4.0f)
            g_TrackGame->fullNetPacketIntervalSeconds = 4.0f;
        g_TrackGame->shortNetPacketIntervalSeconds =
            g_TrackGame->GetRegistryInt("IntervalBetweenShortPacketsMS", -1000) * 0.001f;
        if (g_TrackGame->shortNetPacketIntervalSeconds < 0.0f) {
            int connection = g_TrackGame->network->providerKind;
        char address[0x80];
        char line[0x80];
            if (connection == 1 || connection == 2 || connection == 8)
                g_TrackGame->shortNetPacketIntervalSeconds = 0.067f;
            else
                g_TrackGame->shortNetPacketIntervalSeconds = 0.2f;
        } else if (g_TrackGame->shortNetPacketIntervalSeconds < 0.067f) {
            g_TrackGame->shortNetPacketIntervalSeconds = 0.067f;
        } else if (g_TrackGame->shortNetPacketIntervalSeconds > 0.2f) {
            g_TrackGame->shortNetPacketIntervalSeconds = 0.2f;
        }
        s_UnknownStatic573c0c = 2;
        g_TrackGame->mode.ResetNetworkRace();
        if (openingMenu != 0x868)
            g_TrackGame->eventManager->ResetEntries();
        g_TrackGame->mode.field_0x27f8.field_0x2c = 0;
        g_UnknownGlobal689dfc = g_TrackGame->network->isHost == 0;
        g_UnknownGlobal689e04 = 2;
        if (g_TrackGame->mode.field_0x27f8.field_0x04 == 4) {
            g_UnknownGlobal689e04 = 0;
            g_TrackGame->mode.field_0x27f8.field_0x00 = 0;
        } else {
            g_UnknownGlobal689e04 = 2;
        }
        UIListBox* chat = static_cast<UIListBox*>(FindControl("ListChat", 0));
        chat->SetSelectable(0);
        chat->SetTextAlign(0x11);
        UIEditBox* edit = (UIEditBox*)FindControl("EditChat", 0);
        edit->SetCapacity(0x3c);
        edit->SetTextAlign(0x11);
        edit->SetBackgroundColor(0xffffff);
        if (g_TrackGame->ui->field_0x3c == 0x866 && !g_TrackGame->network->lobbyConnected)
            UnknownFunction4f2ec0();
        g_UnknownGlobal689df4 = 0;
        UIControl* user = FindControl("UserName", 0xc);
        g_TrackGame->network->GetPlayerName(g_TrackGame->network->localPlayer, name);
        user->SetText(name);
        user->SetTextDrop(0);
        user->SetTextAlign(0x12);
        UIControl* ready = FindControl("ButRdyUser", 0);
        ready->SetFontColor(0x808080);
        ready->SetTextAlign(0x12);
        for (i = 0; i < 7; i++) {
            sprintf(text, "ButRdyPlayer%d", i + 2);
            UIControl* button = FindControl(text, 0);
            button->SetFontColor(0x808080);
            button->SetTextAlign(0x12);
        }
        for (i = 0; i < 7; i++) {
            sprintf(text, "NamePlayer%d", i + 2);
            UIControl* label = FindControl(text, 0);
            label->SetTextDrop(0);
            label->SetTextAlign(0x12);
            g_UnknownGlobal689dbc[i] = 0;
            g_UnknownGlobal689dd8[i] = 0;
        }
        UnknownFunction4f3620();
        chat = static_cast<UIListBox*>(FindControl("ListChat", 0));
        chat->SetSelectable(0);
        EnableGroup(0x258, 0);
        UnknownFunction4f3260();
        if (g_TrackGame->network->lobbyConnected) {
            if (g_TrackGame->mode.field_0x1bd4 == -1) {
                g_TrackGame->networkGameObject->UnknownFunction49c2f0(&g_TrackGame->mode.field_0x1bd4);
                g_TrackGame->networkGameObject->UnknownFunction49c600();
                if (field_0x7f58) {
                    field_0x7f58->UpdateBoundValues(0);
                    field_0x7f58->ApplyEventType();
                    field_0x7f58->UpdateBoundValues(0);
                }
            }
        } else {
            g_TrackGame->mode.field_0x1bd4 = 0;
        }
        if (openingMenu == 0x88e || openingMenu == 0x868) {
            static_cast<UIRadioButton*>(FindControl("RaceInfoTab", 4))->SelectInGroup(0);
            ShowPage(2);
        } else {
            static_cast<UIRadioButton*>(FindControl("EventTab", 4))->SelectInGroup(0);
            ShowPage(0);
        }
        textColors[0] = 0xfeb97a;
        int connection = g_TrackGame->network->providerKind;
        char address[0x80];
        char line[0x80];
        if ((connection == 4 || connection == 2) && g_TrackGame->network->isHost) {
            g_TrackGame->LoadResourceString(0x14b9, text, 0x80);
            g_TrackGame->network->GetLocalAddress(address);
            sprintf(line, "%s %s", text, address);
            AddChatLine(-1, line);
        }
        AddTimer(0, 0x3e8, 0);
        guiUser->UnknownFunction487790((UnknownGuiControl*)FindControl("EditChat", 0xb), 0, 0);
        break;
    }
    case kDialogCommand:
        if (!_stricmp("EventTab", event->controlName)) {
            ShowPage(0);
        } else if (!_stricmp("BikeRiderTab", event->controlName)) {
            ShowPage(1);
        } else if (!_stricmp("RaceInfoTab", event->controlName)) {
            ShowPage(2);
        } else if (!_stricmp("OptionsTab", event->controlName)) {
            ShowPage(3);
        } else if (!_stricmp("Back", event->controlName)) {
            if (field_0x7f58)
                field_0x7f58->UpdateBoundValues(1);
            if (field_0x7f5c)
                field_0x7f5c->UpdateBoundValues(1);
            UpdateBoundValues(1);
            memcpy(&g_TrackGame->mode.field_0x2bd0, &g_TrackGame->mode.field_0x27f8,
                   sizeof(g_TrackGame->mode.field_0x2bd0));
            memcpy(g_TrackGame->mode.field_0x1090, g_TrackGame->mode.field_0xfd8,
                   sizeof(g_TrackGame->mode.field_0x1090));
            memcpy(g_TrackGame->mode.field_0x1b04, &g_TrackGame->mode.field_0x1974,
                   sizeof(g_TrackGame->mode.field_0x1b04));
            if (g_TrackGame->network->lobbyConnected) {
                EndDialog(0);
                g_TrackGame->ui->OpenExitDialog();
                return;
            }
            g_TrackGame->network->DestroyLocalPlayer();
            EndNetworkGame();
            event->dialog->EndDialog(0);
            event->handled = 1;
            g_TrackGame->ui->OpenMenu(100);
        } else if (!_stricmp("ButRdyUser", event->controlName)) {
            int tooBig = UnknownBikeClassOf(UNKNOWN_GARAGE_SETTINGS->engineSize) > g_TrackGame->mode.field_0x27f8.field_0x1c;
            if (g_TrackGame->mode.field_0x1bd4)
                tooBig = UnknownBikeClassOf(UNKNOWN_GARAGE_SETTINGS->engineSize) > 2;
            if (!tooBig) {
                if (!g_UnknownGlobal689dfc)
                    g_UnknownGlobal689df4 = !g_UnknownGlobal689df4;
            } else {
                UnknownFunction4f57c0(1);
            }
            if (g_UnknownGlobal689dfc == 2 || g_UnknownGlobal689dfc == 3)
                UnknownFunction4f57c0(g_UnknownGlobal689dfc);
            UIControl* button = FindControl("ButRdyUser", 0);
            if (g_UnknownGlobal689df4) {
                char format[0x80];
                char line[0x80];
                UnknownFunction4f3260();
                button->SetFontColor(0xffffff);
                g_TrackGame->LoadResourceString(0x14d3, format, 0x80);
                sprintf(line, format, g_TrackGame->mode.field_0x00);
                SendSystemChatLine(line);
            } else {
                button->SetFontColor(0x808080);
            }
        } else if (!_stricmp("Start", event->controlName)) {
            if (g_UnknownGlobal689df4) {
                UpdateBoundValues(1);
                if (field_0x7f58)
                    field_0x7f58->UpdateBoundValues(1);
                if (field_0x7f5c)
                    field_0x7f5c->UpdateBoundValues(1);
                if (field_0x7f64)
                    field_0x7f64->UpdateBoundValues(1);
                if (field_0x7f58)
                    field_0x7f58->UpdateBoundValues(1);
                if (g_TrackGame->mode.field_0x27f8.field_0x08 == -1) {
                    ChoiceDlg* dialog = new(__FILE__, 0x6a4) ChoiceDlg;
                    guiManager->ShowDialog((UIDialog*)dialog, 0, 4, 0, (UIDialog*)this, 0, 0, 1);
                    strcpy(prompt, "You must choose one of the following trial version tracks:\n"
                                   "Stunt Event: Donner Pass, or\n"
                                   "Nationals Race: A Voodoo Basin\n");
                    dialog->SetTextsOrResources("Trial Version", 0, prompt, 0, 0, 0, 0, 0x13e9, 0, 0);
                    dialog->FindControl("TxtPrompt", 0)->field_0x1e8 = 1;
                    return;
                }
                memcpy(&g_TrackGame->mode.field_0x2bd0, &g_TrackGame->mode.field_0x27f8,
                       sizeof(g_TrackGame->mode.field_0x2bd0));
                memcpy(g_TrackGame->mode.field_0x1090, g_TrackGame->mode.field_0xfd8,
                       sizeof(g_TrackGame->mode.field_0x1090));
                memcpy(g_TrackGame->mode.field_0x1b04, &g_TrackGame->mode.field_0x1974,
                       sizeof(g_TrackGame->mode.field_0x1b04));
                SendStartMessage(1);
            }
        } else if (!_stricmp("ButChatSend", event->controlName)) {
            SendChatLine();
            guiUser->UnknownFunction487790((UnknownGuiControl*)FindControl("EditChat", 0xb), 0, 0);
        } else if (!_stricmp("ButRdyPlayer2", event->controlName)) {
            RemoveLobbyPlayer(0);
        } else if (!_stricmp("ButRdyPlayer3", event->controlName)) {
            RemoveLobbyPlayer(1);
        } else if (!_stricmp("ButRdyPlayer4", event->controlName)) {
            RemoveLobbyPlayer(2);
        } else if (!_stricmp("ButRdyPlayer5", event->controlName)) {
            RemoveLobbyPlayer(3);
        } else if (!_stricmp("ButRdyPlayer6", event->controlName)) {
            RemoveLobbyPlayer(4);
        } else if (!_stricmp("ButRdyPlayer7", event->controlName)) {
            RemoveLobbyPlayer(5);
        } else if (!_stricmp("ButRdyPlayer8", event->controlName)) {
            RemoveLobbyPlayer(6);
        } else if (!_stricmp("Options", event->controlName)) {
            OptionsDlg* dialog = new(__FILE__, 0x6d5) OptionsDlg;
            guiManager->ShowDialog((UIDialog*)dialog, 0, 2, 0, (UIDialog*)this, 0, 0, 1);
        } else if (!_stricmp("Joystick", event->controlName)) {
            OptionsDlg* dialog = new(__FILE__, 0x6d8) OptionsDlg;
            guiManager->ShowDialog((UIDialog*)dialog, 0, 2, 0, (UIDialog*)this, 1, 0, 1);
        } else if (!_stricmp("Help", event->controlName)) {
            g_TrackGame->mode.OpenHelp("MCM2HELP", 0);
        }
        if (_stricmp("Back", event->controlName) && _stricmp("Start", event->controlName))
            guiUser->UnknownFunction487790((UnknownGuiControl*)FindControl("EditChat", 0xb), 0, 0);
        break;
    case kDialogEditDone:
        _stricmp("EditChat", event->controlName);
        break;
    case 11:
        if (event->key == 0x2f) {
            UnknownGuiControl* focus = guiUser->focusControl;
            if (focus != (UnknownGuiControl*)FindControl("EditChat", 0))
                guiUser->UnknownFunction487790((UnknownGuiControl*)FindControl("EditChat", 0xb), 0, 0);
        }
        if (event->key == 0xd) {
            FindControl("ButChatSend", 0)->UnknownVirtualSlot58(2, 0);
            SendChatLine();
        }
        break;
    case kDialogTimer: {
        for (i = 0; i < 7; i++)
            g_UnknownGlobal689d08[i].UnknownVirtualSlot1();
        int index = 0;
        g_UnknownGlobal689df8 = 0;
        int me = g_TrackGame->network->localPlayer;
        NetPlayer* player;
        while ((player = g_TrackGame->network->NextPlayer(&index)) != 0) {
            if (player->id != me) {
                g_UnknownGlobal689d08[g_UnknownGlobal689df8].id = player->id;
                COPY_TEXT(g_UnknownGlobal689d08[g_UnknownGlobal689df8].name, player->name, 0x10);
                field_0x7f68.SetSlot(player->id, -1);
                g_UnknownGlobal689df8++;
            }
        }
        for (i = 0; i < 7; i++) {
            sprintf(text, "NamePlayer%d", i + 2);
            UIControl* label = FindControl(text, 0);
            label->SetText(g_UnknownGlobal689d08[i].name);
        }
        for (i = 0; i < 7; i++) {
            sprintf(text, "ButRdyPlayer%d", i + 2);
            UIControl* button = FindControl(text, 0);
            if (!g_UnknownGlobal689d08[i].name[0])
                button->SetFontColor(0x808080);
        }
        if (field_0x7f5c)
            field_0x7f5c->UpdateBoundValues(1);
        UnknownFunction4f3620();
        if (g_TrackGame->network->isHost) {
            int allReady = 1;
            UIControl* start = FindControl("Start", 0);
            for (i = 0; i < g_UnknownGlobal689df8; i++) {
                if (!g_UnknownGlobal689dbc[i])
                    allReady = 0;
            }
            if (g_UnknownGlobal689df4 && allReady && g_UnknownGlobal689df8 >= 1)
                start->UnknownVirtualSlot49(1);
            else
                start->UnknownVirtualSlot49(0);
            field_0x7f68.SetSlot(me, g_TrackGame->mode.field_0x6dc);
            SendSettings();
        }
        state.field_0x01 = g_UnknownGlobal689df4;
        COPY_TEXT(state.field_0x42, g_TrackGame->mode.field_0x1974.field_0x00, 0x40);
        COPY_TEXT(state.field_0x02, g_TrackGame->mode.field_0x1974.field_0x40, 0x40);
        COPY_TEXT(state.field_0x82, g_TrackGame->mode.field_0x1974.field_0x80, 0x40);
        state.field_0xc2 = g_TrackGame->mode.field_0x1974.field_0xc4;
        state.field_0xc3 = g_TrackGame->mode.field_0x6dc;
        state.field_0xc8 = UNKNOWN_GARAGE_SETTINGS->engineSize;
        state.field_0xcc = UNKNOWN_GARAGE_SETTINGS->field_0x04;
        if (g_TrackGame->mode.field_0x1bd0 > 0 && g_TrackGame->mode.field_0x1bd0 < 0x65)
            g_TrackGame->mode.field_0x1bcc = g_TrackGame->mode.field_0x1bd0;
        state.field_0xd0 = g_TrackGame->mode.field_0x1bcc;
        state.field_0xc4 = ReadClock();
        g_TrackGame->network->Send(2, &state, sizeof(state),
                                                               g_TrackGame->network->localPlayer, 0);
        break;
    }
    case 9:
        if (event->code == 0x51) {
            if (field_0x7f5c) {
                field_0x7f5c->UpdateBoundValues(0);
                field_0x7f5c->FillBikeRiderLists();
                field_0x7f5c->ApplyChosenRider();
                field_0x7f5c->ApplyChosenBike();
            }
        } else if (event->code == 0x66) {
            static_cast<UIRadioButton*>(FindControl("EventTab", 4))->SelectInGroup(0);
            ShowPage(0);
        }
        break;
    case kDialogClose:
        RemoveTimers(0);
        break;
    }
}

// 0x004f57c0: reports why the ready state was cleared.
void MultiPlayerDlg::UnknownFunction4f57c0(int reason) {
    char text[0x80];
    char value[0x80];
    char message[0x80];
    switch (reason) {
    case 1:
        g_TrackGame->LoadResourceString(0x146a, text, 0x80);
        sprintf(value, text, g_UnknownGlobal56cb6c[g_TrackGame->mode.field_0x27f8.field_0x1c]);
        if (g_TrackGame->mode.field_0x1bd4)
            sprintf(value, text, g_UnknownGlobal56cb6c[2]);
        g_TrackGame->LoadResourceString(0x14d1, text, 0x80);
        sprintf(message, text, value);
        break;
    case 2: {
        g_TrackGame->LoadResourceString(0x14d2, text, 0x80);
        int length = strlen(text);
        int count = length > 0x7f ? 0x7f : length;
        strncpy(message, text, count);
        message[count] = 0;
        break;
    }
    case 3: {
        g_TrackGame->LoadResourceString(0x14da, text, 0x80);
        int length = strlen(text);
        int count = length > 0x7f ? 0x7f : length;
        strncpy(message, text, count);
        message[count] = 0;
        break;
    }
    }
    AddChatLine(-1, message);
    g_UnknownGlobal689df4 = 0;
    if (reason == 2) {
        g_TrackGame->LoadResourceString(0x14d4, text, 0x80);
        sprintf(value, text, g_TrackGame->mode.field_0x00);
        SendSystemChatLine(value);
    } else if (reason == 3) {
        g_TrackGame->LoadResourceString(0x14db, text, 0x80);
        sprintf(value, text, g_TrackGame->mode.field_0x00);
        SendSystemChatLine(value);
    }
    FindControl("ButRdyUser", 1)->SetFontColor(0x808080);
}

// 0x004f5a00
void MultiPlayerDlg::ShowPage(int page) {
    RECT area = {0x11, 0x23, 0x1aa, 0x111};
    if (field_0x7f58 && page != 0) {
        field_0x7f58->Release();
        field_0x7f58 = 0;
    }
    if (field_0x7f5c && page != 1) {
        field_0x7f5c->Release();
        field_0x7f5c = 0;
    }
    if (field_0x7f60 && page != 2) {
        field_0x7f60->Release();
        field_0x7f60 = 0;
    }
    if (field_0x7f64 && page != 3) {
        field_0x7f64->Release();
        field_0x7f64 = 0;
    }
    switch (page) {
    case 0:
        if (!field_0x7f58) {
            field_0x7f58 = new(__FILE__, 0x7a4) MPEventDlg;
            g_TrackGame->ui->field_0x2c->ShowDialog(field_0x7f58, 0, 1, (int)&area, (int)this,
                                                                         0, 0, 1);
        }
        break;
    case 1:
        if (!field_0x7f5c) {
            field_0x7f5c = new(__FILE__, 0x7aa) MPBikeRiderDlg;
            g_TrackGame->ui->field_0x2c->ShowDialog(field_0x7f5c, 0, 1, (int)&area, (int)this,
                                                                         0, 0, 1);
        }
        break;
    case 2:
        if (!field_0x7f60) {
            field_0x7f60 = new(__FILE__, 0x7b0) MPRaceInfoDlg;
            g_TrackGame->ui->field_0x2c->ShowDialog(field_0x7f60, 0, 1, (int)&area, (int)this,
                                                                         0, 0, 1);
        }
        break;
    case 3:
        if (!field_0x7f64) {
            field_0x7f64 = new(__FILE__, 0x7b6) MPOptionsDlg;
            g_TrackGame->ui->field_0x2c->ShowDialog(field_0x7f64, 0, 1, (int)&area, (int)this,
                                                                         0, 0, 1);
        }
        break;
    }
}

// 0x004f5ca0
void MultiPlayerDlg::FollowTrackChange() {
    int value;
    int index;
    char scene[0x104];
    char path[0x104];
    UnknownFunction4f3260();
    if (!strcmp(builtForTrack, g_TrackGame->mode.field_0x27f8.field_0x36) && field_0x80cc == g_TrackGame->mode.field_0x27f8.field_0x34)
        return;
    g_TrackGame->mode.UnknownFunction523b70(scene);
    g_TrackGame->mode.FindFileDirectory((int)scene, g_TrackGame->mode.field_0x27f8.field_0x36, "env", path);
    index = 0;
    value = 0;
    g_TrackGame->sceneObject->UnknownFunction4e9b80(path);
    if (g_TrackGame->mode.UnknownFunction524100() == 1 || g_TrackGame->mode.UnknownFunction524100() == 5)
        g_TrackGame->sceneObject->UnknownFunction4ea010(field_0x80d0, g_TrackGame->mode.field_0x27f8.field_0x36,
                                                                  g_TrackGame->mode.field_0x27f8.field_0x34, "scn", &index,
                                                                  &value);
    else
        g_TrackGame->sceneObject->UnknownFunction4ea010(field_0x80d0, g_TrackGame->mode.field_0x27f8.field_0x36, 0,
                                                                  "scn", &index, &value);
    if (!_stricmp("no name", field_0x80d0)) {
        g_UnknownGlobal689dfc = 2;
        if (g_UnknownGlobal689df4)
            UnknownFunction4f57c0(2);
        g_TrackGame->LoadResourceString(0x143b, field_0x80d0, 0x80);
    } else if (value != g_TrackGame->mode.field_0x27f8.field_0x13c) {
        g_UnknownGlobal689dfc = 3;
        if (g_UnknownGlobal689df4)
            UnknownFunction4f57c0(3);
    } else {
        g_UnknownGlobal689dfc = 0;
    }
    if (field_0x7f58)
        field_0x7f58->ShowTrackPicture();
    {
        int length = strlen(g_TrackGame->mode.field_0x27f8.field_0x36);
        int count = length > 0x103 ? 0x103 : length;
        strncpy(builtForTrack, g_TrackGame->mode.field_0x27f8.field_0x36, count);
        builtForTrack[count] = 0;
    }
    field_0x80cc = g_TrackGame->mode.field_0x27f8.field_0x34;
    {
        int length = strlen(field_0x80d0);
        int count = length > 0x7f ? 0x7f : length;
        strncpy(g_TrackGame->mode.field_0x6f4[g_TrackGame->mode.field_0x27f8.field_0x04], field_0x80d0, count);
        g_TrackGame->mode.field_0x6f4[g_TrackGame->mode.field_0x27f8.field_0x04][count] = 0;
    }
}

// 0x004f5f30
int MPEventDlg::UnknownVirtualSlot24(int type, void* data, int from, int to, int flags) {
    char text[0x80];
    if (type == 0x101) {
        int count = g_TrackGame->mode.field_0x27f8.field_0x28;
        field_0x7f58 = 0;
        UpdateBoundValues(0);
        ApplyEventType();
        field_0x7f58 = 1;
        UpdateBoundValues(0);
        UIListBox* list = static_cast<UIListBox*>(FindControl("OpponentsListBox", 3));
        if (g_TrackGame->mode.field_0x27f8.field_0x04 != 4 && list->rowCount - 1 != g_TrackGame->mode.field_0x1bd8) {
            list->RemoveAllRows();
            for (int i = 0; i <= g_TrackGame->mode.field_0x1bd8; i++) {
                sprintf(text, "%d", i);
                list->AddRow(text, i, 0);
            }
            if (g_TrackGame->mode.field_0x1bd8 < count)
                list->SelectRow(g_TrackGame->mode.field_0x1bd8);
            else
                list->SelectRow(count);
        }
    } else if (from && type == 0xf) {
        ShowHostSettings();
        UnknownFunction4f69d0();
    }
    return 0;
}

// 0x004f6070
void MPEventDlg::ShowHostSettings() {
    char text[0x80];
    MultiPlayerDlg* parent = static_cast<MultiPlayerDlg*>(parentDialog);
    UIListBox* list = static_cast<UIDropDownList*>(FindControl("DDLTextBox", 6))->listPart;
    list->RemoveAllRows();
    list->AddRow(parent->field_0x80d0, 0, 0);
    list->SelectRow(0);
    list = static_cast<UIDropDownList*>(FindControl("EventTypeDropDown", 6))->listPart;
    list->SelectRowByData(g_TrackGame->mode.field_0x27f8.field_0x04);
    list = static_cast<UIDropDownList*>(FindControl("RaceModeDropDown", 6))->listPart;
    list->SelectRowByData(g_TrackGame->mode.field_0x27f8.field_0x00);
    if (g_TrackGame->mode.field_0x27f8.field_0x04 == 4) {
        FindControl("OpponentsControlBox", 5)->Show(0, 1);
        UIMultiState* check = static_cast<UIMultiState*>(FindControl("ChkTagBall", 2));
        check->Show(1, 1);
        check->SetCurrentState(g_TrackGame->mode.field_0x27f8.field_0x144);
        check = static_cast<UIMultiState*>(FindControl("ChkStuntMode", 2));
        check->Show(1, 1);
        check->SetCurrentState(g_TrackGame->mode.field_0x27f8.field_0x148);
    } else {
        FindControl("OpponentsControlBox", 5)->Show(1, 1);
        UIListBox* opponents = static_cast<UIListBox*>(FindControl("OpponentsListBox", 3));
        if (opponents) {
            opponents->RemoveAllRows();
            UnknownFunction451380(g_TrackGame->mode.field_0x27f8.field_0x28, text);
            opponents->AddRow(text, g_TrackGame->mode.field_0x27f8.field_0x28, 0);
        }
        FindControl("ChkTagBall", 2)->Show(0, 1);
        FindControl("ChkStuntMode", 2)->Show(0, 1);
    }
    if (g_TrackGame->mode.field_0x27f8.field_0x00 != 2)
        g_TrackGame->mode.field_0x10ec = 0;
    if (g_TrackGame->mode.field_0x27f8.field_0x04 != 0 && g_TrackGame->mode.field_0x27f8.field_0x04 != 4) {
        UIControl* label = FindControl("LapsLabel", 0xc);
        label->SetTextFromResource(g_TrackGame->resourceInstance, 0x929);
        UIListBox* laps = static_cast<UIListBox*>(FindControl("LapsListBox", 3));
        laps->RemoveAllRows();
        UnknownFunction451380(g_TrackGame->mode.field_0x27f8.field_0x20, text);
        laps->AddRow(text, g_TrackGame->mode.field_0x27f8.field_0x20, 0);
    } else {
        UIControl* label = FindControl("LapsLabel", 0xc);
        label->SetTextFromResource(g_TrackGame->resourceInstance, 0x92f);
        UIListBox* laps = static_cast<UIListBox*>(FindControl("LapsListBox", 3));
        laps->RemoveAllRows();
        UnknownFunction518640(text, g_TrackGame->mode.field_0x27f8.field_0x140 * 60.0f);
        laps->AddRow(text, (int)g_TrackGame->mode.field_0x27f8.field_0x140, 0);
    }
    if (g_TrackGame->mode.field_0x1bd0 > 0 && g_TrackGame->mode.field_0x1bd0 < 0x65)
        g_TrackGame->mode.field_0x1bcc = g_TrackGame->mode.field_0x1bd0;
}

// 0x004f6370
void MPEventDlg::UnknownVirtualSlot29(UnknownDialogEvent* event) {
    char text[0x80];
    char number[0x80];
    switch (event->kind) {
    case kDialogInit: {
        field_0x7f58 = 0;
        UnknownRaceSettings* settings = (UnknownRaceSettings*)&g_TrackGame->mode.field_0x27f8.field_0x00;
        UIListBox* list = static_cast<UIDropDownList*>(FindControl("EventTypeDropDown", 6))->listPart;
        g_TrackGame->LoadResourceString(0x13ed, text, 0x80);
        list->AddRow(text, 1, 0);
        g_TrackGame->LoadResourceString(0x13ee, text, 0x80);
        list->AddRow(text, 0, 0);
        g_TrackGame->LoadResourceString(0x13f0, text, 0x80);
        list->AddRow(text, 5, 0);
        g_TrackGame->LoadResourceString(0x13ef, text, 0x80);
        list->AddRow(text, 2, 0);
        g_TrackGame->LoadResourceString(0x13f2, text, 0x80);
        list->AddRow(text, 3, 0);
        g_TrackGame->LoadResourceString(0x13f1, text, 0x80);
        list->SelectRowByData(settings->eventType);
        list = static_cast<UIDropDownList*>(FindControl("RaceModeDropDown", 6))->listPart;
        g_TrackGame->LoadResourceString(0x13f3, text, 0x80);
        list->AddRow(text, 0, 0);
        g_TrackGame->LoadResourceString(0x13f4, text, 0x80);
        list->AddRow(text, 1, 0);
        g_TrackGame->LoadResourceString(0x13f5, text, 0x80);
        list->AddRow(text, 2, 1);
        list->SelectRow(settings->raceMode);
        FindControl("ChkTagBall", 0)->UnknownVirtualSlot54(&settings->tagBall);
        FindControl("ChkStuntMode", 0)->UnknownVirtualSlot54(&settings->stuntMode);
        int count = g_TrackGame->mode.field_0x27f8.field_0x28;
        UpdateBoundValues(0);
        ApplyEventType();
        UnknownFunction4f69d0();
        UIListBox* opponents = static_cast<UIListBox*>(FindControl("OpponentsListBox", 3));
        if (g_TrackGame->network->isHost && settings->eventType != 4 &&
            opponents->rowCount - 1 != g_TrackGame->mode.field_0x1bd8) {
            opponents->RemoveAllRows();
            for (int i = 0; i <= g_TrackGame->mode.field_0x1bd8; i++) {
                sprintf(number, "%d", i);
                opponents->AddRow(number, i, 0);
            }
            if (g_TrackGame->mode.field_0x1bd8 < count)
                opponents->SelectRow(g_TrackGame->mode.field_0x1bd8);
            else
                opponents->SelectRow(count);
        }
        if (!g_TrackGame->network->isHost)
            ShowTrackPicture();
        break;
    }
    case kDialogCommand:
        if (!_stricmp("TrackLeft", event->controlName)) {
            UIListBox* list = static_cast<UIDropDownList*>(FindControl("DDLTextBox", 6))->listPart;
            int rows = list->rowCount;
            list->SelectRow((list->GetSelectedRow() + rows - 1) % rows);
            list->UnknownVirtualSlot66(0);
        } else if (!_stricmp("TrackRight", event->controlName)) {
            UIListBox* list = static_cast<UIDropDownList*>(FindControl("DDLTextBox", 6))->listPart;
            int rows = list->rowCount;
            list->SelectRow((list->GetSelectedRow() + 1) % rows);
            list->UnknownVirtualSlot66(0);
        } else if (!_stricmp("RacesLeftArrow", event->controlName) || !_stricmp("RacesRightArrow", event->controlName)) {
            g_TrackGame->mode.field_0x27f8.field_0x0c = static_cast<UIListBox*>(FindControl("RacesListBox", 3))->GetRowData(-1);
        }
        break;
    case kDialogListSelect:
        if (!_stricmp("RaceModeDropDown", event->controlName)) {
            UIListBox* list = static_cast<UIDropDownList*>(FindControl("RaceModeDropDown", 6))->listPart;
            g_TrackGame->mode.field_0x27f8.field_0x00 = list->GetRowData(-1);
            UnknownFunction4f69d0();
            if (static_cast<UIListBox*>(event->control)->GetRowData(-1) != 2) {
                g_TrackGame->mode.field_0x10ec = 0;
                g_TrackGame->network->SetSessionJoinable(1);
            }
        } else if (!_stricmp("EventTypeDropDown", event->controlName)) {
            UpdateBoundValues(1);
            ApplyEventType();
            UnknownFunction4f69d0();
        } else if (!_stricmp("DDLTextBox", event->controlName)) {
            int index = static_cast<UIListBox*>(event->control)->GetRowData(-1);
            UnknownRaceSettings* settings = (UnknownRaceSettings*)&g_TrackGame->mode.field_0x27f8.field_0x00;
            g_TrackGame->mode.UnknownFunction5240e0(g_TrackGame->ui->field_0x60[index].field_0x08);
            {
                char* name = g_TrackGame->ui->field_0x60[index].field_0x14;
                int length = strlen(name);
                int count = length > 0xff ? 0xff : length;
                strncpy(settings->trackName, name, count);
                settings->trackName[count] = 0;
            }
            settings->trackNumber = g_TrackGame->ui->field_0x60[index].field_0x04;
            settings->field_0x138 = g_TrackGame->ui->field_0x60[index].field_0x0c;
            settings->field_0x13c = g_TrackGame->ui->field_0x60[index].field_0x10;
            strcpy(g_TrackGame->mode.field_0x6f4[g_TrackGame->mode.field_0x27f8.field_0x04],
                   static_cast<UIListBox*>(event->control)->GetRowText(-1));
        }
        break;
    case kDialogClose:
        if (g_TrackGame->mode.field_0x27f8.field_0x04 != 4) {
            UIListBox* opponents = static_cast<UIListBox*>(FindControl("OpponentsListBox", 3));
            g_TrackGame->mode.field_0x27f8.field_0x28 = opponents->GetRowData(-1);
        } else
            g_TrackGame->mode.field_0x27f8.field_0x28 = 0;
        break;
    }
}

// 0x004f69d0: enables the controls the player may change.
void MPEventDlg::UnknownFunction4f69d0() {
    char text[0x20];
    GameObjectIterator controls(controlContainer, 1, "UIControl");
    int enable = g_TrackGame->network->isHost && !(g_TrackGame->mode.field_0x1bd4 & 1);
    for (GameObject* control = controls.Next(); control; control = controls.Next())
        ((UIControl*)control)->UnknownVirtualSlot49(enable);
    if (enable) {
        ShowGroup(0x141, 1);
        static_cast<UIDropDownList*>(FindControl("EventTypeDropDown", 6))->UnknownInlineButton()->Show(1, 1);
        static_cast<UIDropDownList*>(FindControl("RaceModeDropDown", 6))->UnknownInlineButton()->Show(1, 1);
    } else {
        ShowGroup(0x141, 0);
        static_cast<UIDropDownList*>(FindControl("EventTypeDropDown", 6))->UnknownInlineButton()->Show(0, 1);
        static_cast<UIDropDownList*>(FindControl("RaceModeDropDown", 6))->UnknownInlineButton()->Show(0, 1);
        if (g_TrackGame->network->isHost && !(g_TrackGame->mode.field_0x1bd4 & 2)) {
            FindControl("DDLTextBox", 6)->UnknownVirtualSlot49(1);
            FindControl("TrackLeft", 0)->UnknownVirtualSlot49(1);
            FindControl("TrackLeft", 0)->Show(1, 1);
            FindControl("TrackRight", 0)->UnknownVirtualSlot49(1);
            FindControl("TrackRight", 0)->Show(1, 1);
        }
    }
    UIListBox* list = static_cast<UIDropDownList*>(FindControl("RaceModeDropDown", 6))->listPart;
    int mode = list->GetRowData(-1);
    UIControl* lapsBox = FindControl("LapsControlBox", 5);
    UIControl* racesBox = FindControl("RacesControlBox", 5);
    switch (mode) {
    case 0:
        lapsBox->Show(0, 1);
        racesBox->Show(0, 1);
        break;
    case 1:
        lapsBox->Show(1, 1);
        racesBox->Show(0, 1);
        break;
    case 2:
        lapsBox->Show(1, 1);
        racesBox->Show(1, 1);
        break;
    }
    UIControl* racesLeft = FindControl("RacesLeftArrow", 9);
    UIControl* racesRight = FindControl("RacesRightArrow", 0xa);
    UIControl* opponentsLeft = FindControl("OpponentsLeftArrow", 0);
    UIControl* opponentsRight = FindControl("OpponentsRightArrow", 0);
    UIControl* racesLabel = FindControl("RacesLabel", 0xc);
    UIListBox* races = static_cast<UIListBox*>(FindControl("RacesListBox", 3));
    UIControl* opponents = FindControl("OpponentsListBox", 3);
    int selection;
    if (g_TrackGame->network->isHost && races->rowCount)
        selection = races->GetRowData(-1);
    else
        selection = g_TrackGame->mode.field_0x27f8.field_0x0c;
    races->RemoveAllRows();
    for (int i = 3; i <= 7; i += 2) {
        sprintf(text, "%d", i);
        races->AddRow(text, i - 1, 0);
    }
    races->SelectRowByData(selection);
    if (g_TrackGame->mode.field_0x27f8.field_0x00 == 2 && g_TrackGame->mode.field_0x10ec) {
        if (g_TrackGame->network->isHost) {
            opponentsLeft->Show(0, 1);
            opponentsRight->Show(0, 1);
            opponents->UnknownVirtualSlot49(0);
            racesLeft->Show(0, 1);
            racesRight->Show(0, 1);
        }
        racesLabel->SetTextFromResource(g_TrackGame->resourceInstance, 0x92b);
        sprintf(text, "%d/%d", g_TrackGame->eventManager->field_0x48 + 1, g_TrackGame->mode.field_0x27f8.field_0x0c + 1);
        races->RemoveAllRows();
        races->AddRow(text, 0, 0);
    } else {
        if (g_TrackGame->network->isHost) {
            opponentsLeft->Show(1, 1);
            opponentsRight->Show(1, 1);
            opponents->UnknownVirtualSlot49(1);
            racesLeft->Show(1, 1);
            racesRight->Show(1, 1);
        }
        racesLabel->SetTextFromResource(g_TrackGame->resourceInstance, 0x92a);
    }
}

// 0x004f6e60
void MPEventDlg::ApplyEventType() {
    char text[0x100];
    if (g_TrackGame->network->isHost)
        g_UnknownGlobal689dfc = 0;
    UIListBox* list = static_cast<UIDropDownList*>(FindControl("EventTypeDropDown", 6))->listPart;
    int type = list->GetRowData(-1);
    int& eventType = g_TrackGame->mode.field_0x27f8.field_0x04;
    eventType = type;
    FindControl("OpponentsListBox", 3);
    UIListBox* laps = static_cast<UIListBox*>(FindControl("LapsListBox", 3));
    FindControl("RacesListBox", 3);
    UIControl* label = FindControl("LapsLabel", 0xc);
    label->SetTextFromResource(g_TrackGame->resourceInstance, 0x929);
    if (type != 4) {
        FindControl("OpponentsControlBox", 5)->Show(1, 1);
        FindControl("ChkTagBall", 2)->Show(0, 1);
        FindControl("ChkStuntMode", 2)->Show(0, 1);
    } else {
        FindControl("OpponentsControlBox", 5)->Show(0, 1);
        FindControl("ChkTagBall", 2)->Show(1, 1);
        FindControl("ChkStuntMode", 2)->Show(1, 1);
    }
    if (type != 0 && type != 4) {
        laps->RemoveAllRows();
        for (int i = type == 2 || type == 3 ? 2 : 1; i <= 5; i++) {
            sprintf(text, "%d", i);
            laps->AddRow(text, i, 0);
        }
        laps->SelectRowByData(g_TrackGame->mode.field_0x27f8.field_0x20);
    } else {
        label->SetTextFromResource(g_TrackGame->resourceInstance, 0x92f);
        laps->RemoveAllRows();
        for (int i = 2; i <= 5; i++) {
            UnknownFunction518640(text, i * 60.0f);
            laps->AddRow(text, i, 0);
        }
        laps->SelectRowByData((int)g_TrackGame->mode.field_0x27f8.field_0x140);
    }
    if (g_TrackGame->network->isHost) {
        if (eventType == 0) {
            g_TrackGame->mode.UnknownFunction5240e0(0);
            FillTrackList(0, "PictureBox", "DDLTextBox", 0, this, 0);
            g_TrackGame->mode.UnknownFunction5240e0(5);
            FillTrackList(0, "PictureBox", "DDLTextBox", 0, this, 1);
        } else if (eventType == 4) {
            g_TrackGame->mode.UnknownFunction5240e0(4);
            FillTrackList(0, "PictureBox", "DDLTextBox", 0, this, 0);
            g_TrackGame->mode.UnknownFunction5240e0(1);
            FillTrackList(0, "PictureBox", "DDLTextBox", 0, this, 1);
            g_TrackGame->mode.UnknownFunction5240e0(5);
            FillTrackList(0, "PictureBox", "DDLTextBox", 0, this, 1);
            g_TrackGame->mode.UnknownFunction5240e0(0);
            FillTrackList(0, "PictureBox", "DDLTextBox", 0, this, 1);
        } else {
            g_TrackGame->mode.UnknownFunction5240e0(type);
            FillTrackList(0, "PictureBox", "DDLTextBox", 0, this, 0);
        }
        list = static_cast<UIDropDownList*>(FindControl("DDLTextBox", 6))->listPart;
        list->SelectRowByText(g_TrackGame->mode.field_0x6f4[type]);
    } else {
        MultiPlayerDlg* parent = static_cast<MultiPlayerDlg*>(parentDialog);
        list = static_cast<UIDropDownList*>(FindControl("DDLTextBox", 6))->listPart;
        list->RemoveAllRows();
        list->AddRow(parent->field_0x80d0, 0, 0);
        list->SelectRow(0);
    }
    g_TrackGame->mode.UnknownFunction5240e0(g_TrackGame->mode.field_0x27f8.field_0x08);
}

// 0x004f7210
void MPEventDlg::UnknownVirtualSlot31(int apply) {
    UnknownRaceSettings* settings = (UnknownRaceSettings*)&g_TrackGame->mode.field_0x27f8.field_0x00;
    if (!g_TrackGame->network || !g_TrackGame->network->isHost)
        return;
    if (apply) {
        if (settings->eventType != 0 && settings->eventType != 4)
            settings->laps = static_cast<UIListBox*>(FindControl("LapsListBox", 3))->GetRowData(-1);
        else
            settings->minutes = (float)static_cast<UIListBox*>(FindControl("LapsListBox", 3))->GetRowData(-1);
        if (!g_TrackGame->mode.field_0x10ec)
            settings->races = static_cast<UIListBox*>(FindControl("RacesListBox", 3))->GetRowData(-1);
        strcpy(settings->trackName, "");
        UIListBox* list = static_cast<UIDropDownList*>(FindControl("DDLTextBox", 6))->listPart;
        int index = list->GetRowData(-1);
        {
            char* name = g_TrackGame->ui->field_0x60[index].field_0x14;
            int length = strlen(name);
            int count = length > 0xff ? 0xff : length;
            strncpy(settings->trackName, name, count);
            settings->trackName[count] = 0;
        }
        g_TrackGame->mode.UnknownFunction5240e0(g_TrackGame->ui->field_0x60[index].field_0x08);
        settings->trackNumber = g_TrackGame->ui->field_0x60[index].field_0x04;
        settings->field_0x138 = g_TrackGame->ui->field_0x60[index].field_0x0c;
        settings->field_0x13c = g_TrackGame->ui->field_0x60[index].field_0x10;
        strcpy(g_TrackGame->mode.field_0x6f4[g_TrackGame->mode.field_0x27f8.field_0x04], list->GetRowText(-1));
        g_TrackGame->mode.field_0x9f4[g_TrackGame->mode.field_0x27f8.field_0x04] = g_TrackGame->mode.field_0x27f8.field_0x34;
        list = static_cast<UIDropDownList*>(FindControl("EventTypeDropDown", 6))->listPart;
        settings->eventType = list->GetRowData(-1);
        list = static_cast<UIDropDownList*>(FindControl("RaceModeDropDown", 6))->listPart;
        settings->raceMode = list->GetRowData(-1);
        UIListBox* opponents = static_cast<UIListBox*>(FindControl("OpponentsListBox", 3));
        if (settings->eventType != 4)
            g_TrackGame->mode.field_0x27f8.field_0x28 = opponents->GetRowData(-1);
        else
            g_TrackGame->mode.field_0x27f8.field_0x28 = 0;
    } else {
        static_cast<UIListBox*>(FindControl("RacesListBox", 3))->SelectRowByData(settings->races);
        UIListBox* laps = static_cast<UIListBox*>(FindControl("LapsListBox", 3));
        if (settings->eventType != 0 && settings->eventType != 4)
            laps->SelectRowByData(settings->laps);
        else
            laps->SelectRowByData((int)settings->minutes);
        UIListBox* list = static_cast<UIDropDownList*>(FindControl("EventTypeDropDown", 6))->listPart;
        list->SelectRowByData(g_TrackGame->mode.field_0x27f8.field_0x04);
        list = static_cast<UIDropDownList*>(FindControl("RaceModeDropDown", 6))->listPart;
        list->SelectRowByData(g_TrackGame->mode.field_0x27f8.field_0x00);
        if (g_TrackGame->network->lobbyConnected && field_0x7f58) {
            list = static_cast<UIDropDownList*>(FindControl("DDLTextBox", 6))->listPart;
            for (int i = 0; i < list->rowCount; i++) {
                int row = list->GetRowData(i);
                if (!_stricmp(g_TrackGame->ui->field_0x60[row].field_0x14, settings->trackName)) {
                    list->SelectRowByData(row);
                    return;
                }
            }
        }
    }
}

// 0x004f75d0
void MPEventDlg::ShowTrackPicture() {
    char path[0x104];
    UIListBox* picture = static_cast<UIListBox*>(FindControl("PictureBox", 3));
    picture->RemoveAllRows();
    g_TrackGame->mode.FindFileDirectory(g_TrackGame->mode.field_0x6a0, g_TrackGame->mode.field_0x27f8.field_0x36,
                                                      "env", path);
    UnknownFunction4f7640(picture, path, g_TrackGame->mode.field_0x27f8.field_0x36);
}

// 0x004f7640: loads the track's picture from `directory` (else the
// "unarts.tga" placeholder).
void MPEventDlg::UnknownFunction4f7640(UIControl* picture, const char* directory, const char* name) {
    char file[0x104];
    char path[0x104];
    char found[0x104];
    if (!picture)
        return;
    if (!*directory) {
        sprintf(path, "%s\\unarts.tga", "ui");
        g_TrackGame->UnknownVirtualSlot18(path, file);
        static_cast<UIListBox*>(picture)->AddImageFileRow(file, 0, 1, 0);
        return;
    }
    UnknownTextureStream* stream = new(__FILE__, 0xa72) UnknownTextureStream((int)g_UnknownResourceManager572b44);
    if (g_TrackGame->mode.field_0x27f8.field_0x04 == 1 || g_TrackGame->mode.field_0x27f8.field_0x04 == 5)
        sprintf(file, "%s%02ds.tga", name, g_TrackGame->mode.field_0x27f8.field_0x34);
    else if (g_TrackGame->mode.UnknownFunction524100() == 1 || g_TrackGame->mode.UnknownFunction524100() == 5)
        sprintf(file, "%s01s.tga", name);
    else
        sprintf(file, "%ss.tga", name);
    sprintf(path, "%s\\%s", directory, file);
    if (g_TrackGame->sceneObject->UnknownFunction4e9cd0(stream, path, "rb", (int)file)) {
        static_cast<UIListBox*>(picture)->AddImageFileRow(file, 0, 1, 0);
    } else if (g_TrackGame->UnknownVirtualSlot18(path, found)) {
        static_cast<UIListBox*>(picture)->AddImageFileRow(found, 0, 1, 0);
    } else {
        sprintf(path, "%s\\unarts.tga", "ui");
        g_TrackGame->UnknownVirtualSlot18(path, file);
        static_cast<UIListBox*>(picture)->AddImageFileRow(file, 0, 1, 0);
    }
    if (stream)
        delete stream;
}

// 0x004f8570
void MPBikeRiderDlg::PaintPlateNumber(int number) {
    UnknownBikeNumberPainter painter(g_TrackGame->field_0x1c);
    for (int i = 0; i < g_TrackGame->ui->field_0x4c; i++)
        painter.UnknownFunction417670(
            ((UnknownKrustyUIModel*)g_TrackGame->ui->field_0x48)[i].field_0xc0->plateTexture, number);
}

// 0x004f8620
int MPBikeRiderDlg::UnknownVirtualSlot24(int type, void* data, int from, int to, int flags) {
    if (type == 0x101) {
        UnknownFunction4f8700();
    } else if (from && type == 0xf) {
        UnknownFunction4f8650();
        UnknownFunction4f8700();
    }
    return 0;
}

// 0x004f8650
void MPBikeRiderDlg::UnknownFunction4f8650() {
    if (!g_TrackGame->network->isHost) {
        UIListBox* list = static_cast<UIDropDownList*>(FindControl("LargestOpponentDropDown", 6))->listPart;
        list->SelectRowByData(g_TrackGame->mode.field_0x27f8.field_0x1c);
        if (UnknownBikeClassOf(((UnknownOptGarageSettings*)g_TrackGame->mode.field_0xfd8)->engineSize) >
                g_TrackGame->mode.field_0x27f8.field_0x1c &&
            g_UnknownGlobal689df4)
            static_cast<MultiPlayerDlg*>(parentDialog)->UnknownFunction4f57c0(1);
    }
}

// 0x004f8700
void MPBikeRiderDlg::UnknownFunction4f8700() {
    if (g_TrackGame->network->isHost) {
        FindControl("LargestOpponentDropDown", 0)->UnknownVirtualSlot49(1);
        static_cast<UIDropDownList*>(FindControl("LargestOpponentDropDown", 6))->UnknownInlineButton()->Show(1, 1);
    } else {
        FindControl("LargestOpponentDropDown", 0)->UnknownVirtualSlot49(0);
        static_cast<UIDropDownList*>(FindControl("LargestOpponentDropDown", 6))->UnknownInlineButton()->Show(0, 1);
    }
}

// 0x004f8780
int MPBikeRiderDlg::UnknownVirtualSlot23(UnknownControlEvent* event, UnknownInputEntry* entry) {
    if (event->kind == 1 && event->control == 0) {
        RECT area = previewArea;
        area.left += 0x32;
        area.top += 0x1e;
        area.right -= 0x32;
        area.bottom -= 0x1e;
        if (PtInRect(&area, guiUser->pointerDevice->pointerPosition)) {
            field_0x7f78 = 1;
            field_0x7f7c = 1;
        }
    }
    return UIDialog::UnknownVirtualSlot23(event, entry);
}


// 0x004f8d20
void MPBikeRiderDlg::ApplyChosenBike() {
    char text[12];
    UIListBox* bikes = static_cast<UIDropDownList*>(FindControl("DDLBikes", 6))->listPart;
    KrustyUI* ui = g_TrackGame->ui;
    UnknownKrustyUIBike* bike = &((UnknownKrustyUIBike*)ui->field_0x50)[bikes->GetRowData(-1)];
    UnknownKrustyUIModel* model = &((UnknownKrustyUIModel*)ui->field_0x48)[bike->model];
    int i;
    // Retail re-reads TrackGame's KrustyUI after each model is hidden.
    for (i = 0; i < ui->field_0x4c; i++) {
        ((UnknownKrustyUIModel*)ui->field_0x48)[i].field_0xc0->UnknownVirtualSlot4();
        ui = g_TrackGame->ui;
    }
    model->field_0xc0->UnknownVirtualSlot5();
    field_0x7f84 = 1;
    if (bike->field_0x88) {
        UIControl* engine = FindControl("DDLEngineSize", 0);
        if (!engine->field_0x70) {
            engine->Show(1, 1);
            FindControl("TxtEngineSize", 0)->Show(1, 1);
            if (dialogBackground)
                dialogBackground->UnknownFunction404da0();
            if (g_UnknownGlobal689df4)
                engine->UnknownVirtualSlot49(0);
        }
    } else {
        UIControl* engine = FindControl("DDLEngineSize", 0);
        if (engine->field_0x70) {
            engine->Show(0, 1);
            FindControl("TxtEngineSize", 0)->Show(0, 1);
            if (dialogBackground)
                dialogBackground->UnknownFunction404da0();
        }
    }
    g_TrackGame->mode.field_0x1974.field_0xc4 = bikes->GetRowData(-1);
    if (bike->field_0x88) {
        UIListBox* sizes = static_cast<UIDropDownList*>(FindControl("DDLEngineSize", 6))->listPart;
        int size = sizes->GetRowData(-1);
        UNKNOWN_GARAGE_SETTINGS->engineSize = g_UnknownGlobal56cb6c[size];
        UNKNOWN_GARAGE_SETTINGS->field_0x04 = size == 2 || size == 4 ? 1 : 0;
        UNKNOWN_APPLY_BIKE_CLASS(size, i);
        ((UnknownKrustyUIBike*)g_TrackGame->ui->field_0x50)[g_TrackGame->mode.field_0x1974.field_0xc4]
            .engineSize = UNKNOWN_GARAGE_SETTINGS->engineSize;
        ((UnknownKrustyUIBike*)g_TrackGame->ui->field_0x50)[g_TrackGame->mode.field_0x1974.field_0xc4]
            .field_0x90 = UNKNOWN_GARAGE_SETTINGS->field_0x04;
    } else {
        int row = bikes->GetRowData(-1);
        UNKNOWN_GARAGE_SETTINGS->engineSize = ((UnknownKrustyUIBike*)g_TrackGame->ui->field_0x50)[row].engineSize;
        UNKNOWN_GARAGE_SETTINGS->field_0x04 = ((UnknownKrustyUIBike*)g_TrackGame->ui->field_0x50)[row].field_0x90;
        int bikeClass = UnknownBikeClassOf(UNKNOWN_GARAGE_SETTINGS->engineSize);
        UNKNOWN_APPLY_BIKE_CLASS(bikeClass, i);
    }
    UIEditBox* plate = static_cast<UIEditBox*>(FindControl("EditPlateNumber", 0xb));
    if (g_TrackGame->mode.field_0x1bd0 > 0 && g_TrackGame->mode.field_0x1bd0 < 101) {
        g_TrackGame->mode.field_0x1bcc = g_TrackGame->mode.field_0x1bd0;
        sprintf(text, "%d", g_TrackGame->mode.field_0x1bcc);
    } else {
        _itoa(g_TrackGame->mode.field_0x1bcc, text, 10);
    }
    plate->SetEditText(text);
}

// 0x004f91e0
void MPBikeRiderDlg::UnknownVirtualSlot31(int apply) {
    if (!apply)
        return;
    {
        int length = strlen("");
        int count = length > 0x3f ? 0x3f : length;
        strncpy(g_TrackGame->mode.field_0x1974.field_0x00, "", count);
        g_TrackGame->mode.field_0x1974.field_0x00[count] = 0;
    }
    UIListBox* list = static_cast<UIDropDownList*>(FindControl("DDLBikes", 6))->listPart;
    int bike = list->GetRowData(-1);
    {
        UnknownKrustyUIBike* bikes = (UnknownKrustyUIBike*)g_TrackGame->ui->field_0x50;
        char* name = ((UnknownKrustyUIModel*)g_TrackGame->ui->field_0x48)[bikes[bike].model].modelName;
        int length = strlen(name);
        int count = length > 0x3f ? 0x3f : length;
        strncpy(g_TrackGame->mode.field_0x1974.field_0x00, name, count);
        g_TrackGame->mode.field_0x1974.field_0x00[count] = 0;
    }
    {
        char* name = ((UnknownKrustyUIBike*)g_TrackGame->ui->field_0x50)[bike].field_0x48;
        int length = strlen(name);
        int count = length > 0x3f ? 0x3f : length;
        strncpy(g_TrackGame->mode.field_0x1974.field_0x40, name, count);
        g_TrackGame->mode.field_0x1974.field_0x40[count] = 0;
    }
    {
        int length = strlen("");
        int count = length > 0x3f ? 0x3f : length;
        strncpy(g_TrackGame->mode.field_0x1974.field_0x80, "", count);
        g_TrackGame->mode.field_0x1974.field_0x80[count] = 0;
    }
    list = static_cast<UIDropDownList*>(FindControl("DDLRiders", 6))->listPart;
    int rider = list->GetRowData(-1);
    {
        char* name = ((UnknownKrustyUIModel*)g_TrackGame->ui->field_0x58)[rider].modelName;
        int length = strlen(name);
        int count = length > 0x3f ? 0x3f : length;
        strncpy(g_TrackGame->mode.field_0x1974.field_0x80, name, count);
        g_TrackGame->mode.field_0x1974.field_0x80[count] = 0;
    }
}

// 0x004f93c0
void MPOptionsDlg::UnknownVirtualSlot29(UnknownDialogEvent* event) {
    char text[0x80];
    switch (event->kind) {
    case kDialogCommand:
        if (!_stricmp("ChkRecordRace", event->controlName))
            event->control->UnknownVirtualSlot59(1);
        break;
    case kDialogInit: {
        FindControl("FastFinishesCheckBox", 0)->UnknownVirtualSlot54(&g_TrackGame->mode.field_0x27f8.field_0x18);
        FindControl("ChkTreeCollision", 0)->UnknownVirtualSlot54(&g_TrackGame->mode.field_0x27f8.field_0x10);
        FindControl("ChkRiderCollision", 0)->UnknownVirtualSlot54(&g_TrackGame->mode.field_0x27f8.field_0x14);
        FindControl("RadLODEasy", 0)->UnknownVirtualSlot54(&g_TrackGame->mode.field_0x94);
        FindControl("ChkRecordRace", 0)->Show(0, 1);
        static_cast<UIEditBox*>(FindControl("EditMaxAIBikes", 0xb))->SetAcceptedCharacters("0123456789");
        UIEditBox* edit = static_cast<UIEditBox*>(FindControl("EditMaxAIBikes", 0xb));
        edit->SetEditText(_itoa(g_TrackGame->mode.field_0x6dc, text, 10));
        UpdateBoundValues(0);
        UnknownFunction4f95b0();
        break;
    }
    case kDialogClose:
    {
        UIEditBox* edit = static_cast<UIEditBox*>(FindControl("EditMaxAIBikes", 0xb));
        g_TrackGame->mode.field_0x6dc = atoi(edit->GetEditText(text, 0x80));
    }
        break;
    }
}

// 0x004f9570
int MPOptionsDlg::UnknownVirtualSlot24(int type, void* data, int from, int to, int flags) {
    if (from && type == 0xf) {
        UnknownFunction4f95a0();
        UnknownFunction4f95b0();
    }
    return 0;
}

// 0x004f95a0
void MPOptionsDlg::UnknownFunction4f95a0() {
    UpdateBoundValues(0);
}

// 0x004f95b0
void MPOptionsDlg::UnknownFunction4f95b0() {
    EnableGroup(0x3f3, g_TrackGame->network->isHost &&
                                     !g_TrackGame->mode.field_0x1bd4);
    if (g_TrackGame->mode.field_0x27f8.field_0x04 == 4)
        FindControl("ChkRecordRace", 0)->UnknownVirtualSlot49(0);
    else
        FindControl("ChkRecordRace", 0)->UnknownVirtualSlot49(1);
}
