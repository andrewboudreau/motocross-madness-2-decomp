// SelectGamePicProcs.cpp (__FILE__ 0x00573c6c): the multiplayer lobby
// dialogs. Extent 0x004f17a0..0x004f975b: the first __FILE__ user is
// 0x004f17a0 (line 0xb4 at 0x004f19f3), the last allocation 0x004f8047;
// MPEventDlg, MPBikeRiderDlg, MPOptionsDlg and MultiPlayerDlg's vtables
// point into it, their type descriptors (0x00573e58...) sit in its .data
// after the __FILE__ literal, and its unwind funclets are
// 0x0054dfd6..0x0054e156. The vector initializers 0x004f9620..0x004f975b
// close it; SelectiveGravityModel.cpp starts at 0x004f9760. Names are
// provisional.

#include "SelectGamePicProcs.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "BackgroundImage.h"
#include "Camera.h"
#include "DebugAlloc.h"
#include "../krusty2/math/FastMath.h"
#include "MatrixUtil.h"
#include "Net.h"
#include "TextureMap.h"
#include "Track.h"
#include "TrackGame.h"

// The four vector constants many retail files declare: 0x00689ce8,
// 0x00689cf8, 0x00689db0 and 0x00689cd8. Their initializers
// (0x004f9620..0x004f975b) follow this file's last function rather than
// preceding its first; only this file's code (0x004f7c1f, 0x004f8940,
// 0x004f8a90) reads them, so they are this file's copies.
static const Vector3 kVec3Zero = Vector3(0.0f, 0.0f, 0.0f);
static const Vector3 kVec3XAxis = Vector3(1.0f, 0.0f, 0.0f);
static const Vector3 kVec3YAxis = Vector3(0.0f, 1.0f, 0.0f);
static const Vector3 kVec3ZAxis = Vector3(0.0f, 0.0f, 1.0f);

int g_UnknownGlobal689df4;
int g_UnknownGlobal689df8;
int g_UnknownGlobal689dfc;

// The chat message (types 0x85 and 0x92): 0x4e bytes are sent.
struct UnknownChatMessage {
    int field_0x00;
    char field_0x04[0x4a];
};

// The player removal message (type 0x8e).
struct UnknownKickMessage {
    int field_0x00;
    int field_0x04;                           // player id
};

// 0x004f20d0
void MultiPlayerDlg::UnknownFunction4f20d0() {
    int player = g_UnknownGlobal56e26c->field_0x08->field_0x0c;
    g_UnknownGlobal56e26c->field_0x18 = g_UnknownGlobal56e26c->mode.field_0x27f8.field_0x35;
    g_UnknownGlobal56e26c->field_0x3424 = g_UnknownGlobal56e26c->mode.field_0x27f8.field_0x28;
    g_UnknownGlobal56e26c->mode.field_0x1be0 = g_UnknownGlobal56e26c->field_0x3424 + g_UnknownGlobal56e26c->field_0x18;
    for (int i = 0; i < g_UnknownGlobal56e26c->field_0x18 - 1; i++) {
        g_UnknownGlobal56e26c->mode.field_0x1be4[i + 1].field_0xd4 = g_UnknownGlobal689d08[i].field_0x04;
        g_UnknownGlobal56e26c->mode.field_0x1be4[i + 1].field_0xd8 = 0;
        int length = strlen(g_UnknownGlobal689d08[i].field_0x08);
        int count = length > 0xf ? 0xf : length;
        strncpy(g_UnknownGlobal56e26c->mode.field_0x1be4[i + 1].field_0xdc, g_UnknownGlobal689d08[i].field_0x08, count);
        g_UnknownGlobal56e26c->mode.field_0x1be4[i + 1].field_0xdc[count] = 0;
    }
    g_UnknownGlobal56e26c->mode.field_0x1be4[0].field_0xd4 = player;
    {
        int length = strlen(g_UnknownGlobal56e26c->mode.field_0x00);
        int count = length > 0xf ? 0xf : length;
        strncpy(g_UnknownGlobal56e26c->mode.field_0x1be4[0].field_0xdc, g_UnknownGlobal56e26c->mode.field_0x00, count);
        g_UnknownGlobal56e26c->mode.field_0x1be4[0].field_0xdc[count] = 0;
    }
    g_UnknownGlobal56e26c->mode.field_0x1be4[0].field_0xd8 = 0;
    g_UnknownGlobal56e26c->mode.field_0x1be4[0].UnknownFunction521fb0(
        g_UnknownGlobal56e26c->mode.field_0x1974.field_0x00, g_UnknownGlobal56e26c->mode.field_0x1974.field_0x40,
        g_UnknownGlobal56e26c->mode.field_0x1974.field_0x80);
    g_UnknownGlobal56e26c->mode.field_0x1be4[0].field_0xc4 =
        g_UnknownGlobal56e26c->mode.field_0x1974.field_0xc4;
    if (g_UnknownGlobal56e26c->mode.field_0x1bd0 > 0 && g_UnknownGlobal56e26c->mode.field_0x1bd0 < 0x65)
        g_UnknownGlobal56e26c->mode.field_0x1bcc = g_UnknownGlobal56e26c->mode.field_0x1bd0;
    g_UnknownGlobal56e26c->mode.field_0x1be4[0].field_0xc0 = g_UnknownGlobal56e26c->mode.field_0x1bcc;
    int racer = g_UnknownGlobal689df8 + 1;
    for (int j = 0; j < 8; j++) {
        for (int k = 0; k < field_0x7f68.field_0x00[j].field_0x08; k++) {
            g_UnknownGlobal56e26c->mode.field_0x1be4[racer].field_0xd4 = field_0x7f68.field_0x00[j].field_0x00;
            g_UnknownGlobal56e26c->mode.field_0x1be4[racer].field_0xd8 = racer;
            racer++;
        }
        if (field_0x7f68.field_0x00[j].field_0x00 == player)
            g_UnknownGlobal56e26c->mode.field_0x27f8.field_0x24 = field_0x7f68.field_0x00[j].field_0x08;
    }
    g_UnknownGlobal56e26c->field_0x08->UnknownFunction4ac510(0);
    if (g_UnknownGlobal56e26c->ui->field_0x2c) {
        if (g_UnknownGlobal56e26c->mode.field_0x2dbc) {
            if (g_UnknownGlobal56e26c->mode.field_0x27f8.field_0x04 != 4) {
                g_UnknownGlobal56e26c->field_0x342c = 1;
                g_UnknownGlobal56e26c->field_0x3428 = 0;
            } else {
                g_UnknownGlobal56e26c->mode.field_0x2dbc = 0;
            }
        }
        UnknownFunction4536e0();
        UnknownVirtualSlot26();
    }
}

// 0x00573c0c
static int s_UnknownStatic573c0c = 2;

// 0x004f2b90
void MultiPlayerDlg::UnknownFunction4f2b90() {
    UnknownLobbySlotTable* slots = &field_0x7f68;
    g_UnknownGlobal56e26c->mode.field_0x1bd8 = slots->UnknownFunction4f2fe0(g_UnknownGlobal689df8 + 1, g_UnknownGlobal56e26c->mode.field_0x27f8.field_0x28);
    if (g_UnknownGlobal56e26c->mode.field_0x1bd8 < g_UnknownGlobal56e26c->mode.field_0x27f8.field_0x28)
        g_UnknownGlobal56e26c->mode.field_0x27f8.field_0x28 = g_UnknownGlobal56e26c->mode.field_0x1bd8;
    if (field_0x7f58) {
        UnknownGameUiControl* list = field_0x7f58->UnknownFunction46ebf0("OpponentsListBox", 0);
        if (list && list->field_0x1ec - 1 != g_UnknownGlobal56e26c->mode.field_0x1bd8) {
            int selection = list->UnknownFunction476950();
            list->UnknownFunction4775f0();
            for (int i = 0; i <= g_UnknownGlobal56e26c->mode.field_0x1bd8; i++) {
                char text[0x80];
                sprintf(text, "%d", i);
                list->UnknownFunction476d80(text, i, 0);
            }
            if (g_UnknownGlobal56e26c->mode.field_0x1bd8 < selection)
                list->UnknownFunction476a60(g_UnknownGlobal56e26c->mode.field_0x1bd8);
            else
                list->UnknownFunction476a60(selection);
        }
    }
    if (++s_UnknownStatic573c0c >= 2) {
        g_UnknownGlobal56e26c->mode.field_0x27f8.field_0x35 = g_UnknownGlobal689df8 + 1;
        g_UnknownGlobal56e26c->field_0x18 = g_UnknownGlobal56e26c->mode.field_0x27f8.field_0x35;
        g_UnknownGlobal56e26c->field_0x3424 = g_UnknownGlobal56e26c->mode.field_0x27f8.field_0x28;
        g_UnknownGlobal56e26c->mode.field_0x1be0 = g_UnknownGlobal56e26c->field_0x3424 + g_UnknownGlobal56e26c->field_0x18;
        if (field_0x7f58)
            field_0x7f58->UnknownFunction46ecc0(1);
        if (field_0x7f64)
            field_0x7f64->UnknownFunction46ecc0(1);
        TrackGame* game = g_UnknownGlobal56e26c;
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
        message.field_0x09 = game->mode.field_0x27f8.field_0x34;
        memcpy(message.field_0x54, slots->field_0x00, sizeof(message.field_0x54));
        message.field_0x14[0] = 0;
        strncat(message.field_0x14, game->mode.field_0x27f8.field_0x36, 0x3f);
        g_UnknownGlobal56e26c->field_0x08->UnknownFunction4ac830(0xf, &message, sizeof(message),
                                                                   g_UnknownGlobal56e26c->field_0x08->field_0x0c, 0);
        s_UnknownStatic573c0c = 0;
    }
}

// 0x004f2ec0
int MultiPlayerDlg::UnknownFunction4f2ec0() {
    UnknownGameUiControl* button = UnknownFunction46ebf0("ButRdyUser", 1);
    button->UnknownFunction470d40(0x808080);
    return g_UnknownGlobal56e26c->field_0x08->UnknownFunction4ac5e0(g_UnknownGlobal56e26c->mode.field_0x00) != 0;
}

// 0x004f2f00
UnknownLobbySlotTable::UnknownLobbySlotTable() {
    for (int i = 0; i < 8; i++) {
        field_0x00[i].field_0x00 = 0;
        field_0x00[i].field_0x04 = 0;
        field_0x00[i].field_0x08 = 0;
    }
}

// 0x004f2f20
void UnknownLobbySlotTable::UnknownFunction4f2f20(int id, int value) {
    int i;
    for (i = 0; i < 8; i++) {
        if (field_0x00[i].field_0x00 == id) {
            if (value != -1)
                field_0x00[i].field_0x04 = value;
            return;
        }
    }
    for (i = 0; i < 8; i++) {
        if (field_0x00[i].field_0x00 == 0) {
            field_0x00[i].field_0x00 = id;
            if (value != -1)
                field_0x00[i].field_0x04 = value;
            return;
        }
    }
}

// 0x004f2f80
void UnknownLobbySlotTable::UnknownFunction4f2f80(int id) {
    for (int i = 0; i < 8; i++) {
        if (field_0x00[i].field_0x00 == id) {
            field_0x00[i].field_0x00 = 0;
            field_0x00[i].field_0x04 = 0;
            field_0x00[i].field_0x08 = 0;
            return;
        }
    }
}

// 0x004f2fc0: qsort order, by the request, smallest first.
static int UnknownFunction4f2fc0(const void* a, const void* b) {
    int x = ((const UnknownLobbySlot*)a)->field_0x04;
    int y = ((const UnknownLobbySlot*)b)->field_0x04;
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
        field_0x00[i].field_0x08 = 0;
        total += field_0x00[i].field_0x04;
    }
    if (total + count >= 8)
        total = 8 - count;
    if (limit) {
        int granted = 0;
        int more;
        do {
            more = 0;
            for (int j = 0; j < count; j++) {
                if (field_0x00[j].field_0x04 > field_0x00[j].field_0x08) {
                    field_0x00[j].field_0x08++;
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
void UnknownFunction4f3080(int a, const char* picture, const char* listName, int* value, UIDialog* dialog, int append) {
    char name[0x80];
    if (!dialog)
        dialog = (UIDialog*)g_UnknownGlobal56e26c->ui->field_0x2c->UnknownFunction485df0();
    name[0] = 0;
    switch (g_UnknownGlobal56e26c->mode.field_0x27f8.field_0x04) {
    case 3:
        UnknownFunction4f17a0(g_UnknownGlobal56e26c->mode.field_0x25e4, "Teraform\\SX", "*.env", "env", a, picture, listName,
                              name, value, dialog, 0, 0);
        break;
    case 1:
        UnknownFunction4f17a0(g_UnknownGlobal56e26c->mode.field_0x25e4, "Teraform\\Baja", "*.env", "env", a, picture, listName,
                              name, value, dialog, g_UnknownGlobal56e26c->mode.field_0x9f4[1], 0);
        break;
    case 5:
        UnknownFunction4f17a0(g_UnknownGlobal56e26c->mode.field_0x25e4, "Teraform\\Enduro", "*.env", "env", a, picture,
                              listName, name, value, dialog, g_UnknownGlobal56e26c->mode.field_0x9f4[5], 0);
        break;
    case 0:
    case 4:
        UnknownFunction4f17a0(g_UnknownGlobal56e26c->mode.field_0x25e4, (const char*)g_UnknownGlobal56e26c->mode.field_0x6a0,
                              "*.env", "env", a, picture, listName, name, value, dialog, 0, append);
        break;
    case 2:
        UnknownFunction4f17a0(g_UnknownGlobal56e26c->mode.field_0x25e4, "Teraform\\National", "*.env", "env", a, picture,
                              listName, name, value, dialog, 0, 0);
        break;
    }
}

// 0x004f3260: enables the controls for the ready state.
void MultiPlayerDlg::UnknownFunction4f3260() {
    if (g_UnknownGlobal56e26c->field_0x08->isHost) {
        if (field_0x7f58)
            field_0x7f58->UnknownFunction4f69d0();
        if (!strcmp(g_UnknownGlobal56e26c->mode.field_0x27f8.field_0x36, ""))
            UnknownFunction46ea80(0x514, 1);
        UnknownFunction46ea80(0x3e8, 1);
    } else {
        if (UnknownBikeClassOf(((UnknownOptGarageSettings*)g_UnknownGlobal56e26c->mode.field_0xfd8)->field_0x00) >
                g_UnknownGlobal56e26c->mode.field_0x27f8.field_0x1c &&
            g_UnknownGlobal689df4)
            UnknownFunction4f57c0(1);
        UnknownFunction46ea80(0x514, 0);
        UnknownFunction46ea80(0x3e8, 0);
    }
    if (g_UnknownGlobal689df4) {
        UnknownFunction46ebf0("Options", 1)->UnknownVirtualSlot49(0);
        UnknownFunction46ebf0("Joystick", 1)->UnknownVirtualSlot49(0);
        UnknownFunction46ebf0("Help", 1)->UnknownVirtualSlot49(0);
        if (field_0x7f5c) {
            field_0x7f5c->UnknownFunction46ebf0("ButWrench", 0)->UnknownVirtualSlot49(0);
            field_0x7f5c->UnknownFunction46ebf0("DDLBikes", 0)->UnknownVirtualSlot49(0);
            field_0x7f5c->UnknownFunction46ebf0("BikeLeft", 0)->UnknownFunction470660(0, 1);
            field_0x7f5c->UnknownFunction46ebf0("BikeRight", 0)->UnknownFunction470660(0, 1);
            field_0x7f5c->UnknownFunction46ebf0("DDLEngineSize", 0)->UnknownVirtualSlot49(0);
            field_0x7f5c->UnknownFunction46ebf0("DDLEngineSize", 0)->UnknownInlineButton()->UnknownFunction470660(0, 1);
            if (g_UnknownGlobal56e26c->field_0x08->isHost) {
                field_0x7f5c->UnknownFunction46ebf0("LargestOpponentDropDown", 0)->UnknownVirtualSlot49(0);
                field_0x7f5c->UnknownFunction46ebf0("LargestOpponentDropDown", 0)
                    ->UnknownInlineButton()
                    ->UnknownFunction470660(0, 1);
            }
        }
    } else {
        UnknownFunction46ebf0("Options", 1)->UnknownVirtualSlot49(1);
        UnknownFunction46ebf0("Joystick", 1)->UnknownVirtualSlot49(1);
        UnknownFunction46ebf0("Help", 1)->UnknownVirtualSlot49(1);
        if (field_0x7f5c) {
            field_0x7f5c->UnknownFunction46ebf0("ButWrench", 0)->UnknownVirtualSlot49(1);
            field_0x7f5c->UnknownFunction46ebf0("DDLBikes", 0)->UnknownVirtualSlot49(1);
            field_0x7f5c->UnknownFunction46ebf0("BikeLeft", 0)->UnknownFunction470660(1, 1);
            field_0x7f5c->UnknownFunction46ebf0("BikeRight", 0)->UnknownFunction470660(1, 1);
            field_0x7f5c->UnknownFunction46ebf0("DDLEngineSize", 0)->UnknownVirtualSlot49(1);
            field_0x7f5c->UnknownFunction46ebf0("DDLEngineSize", 0)->UnknownInlineButton()->UnknownFunction470660(1, 1);
            if (g_UnknownGlobal56e26c->field_0x08->isHost) {
                field_0x7f5c->UnknownFunction46ebf0("LargestOpponentDropDown", 0)->UnknownVirtualSlot49(1);
                field_0x7f5c->UnknownFunction46ebf0("LargestOpponentDropDown", 0)
                    ->UnknownInlineButton()
                    ->UnknownFunction470660(1, 1);
            }
        }
    }
}

// 0x004f3620
void MultiPlayerDlg::UnknownFunction4f3620() {
    char text[0x80];
    char name[0x80];
    if (g_UnknownGlobal56e26c->field_0x08->isHost) {
        UnknownFunction46eb30(0x514, 1);
        UnknownFunction46ebf0("Start", 0)->UnknownVirtualSlot49(1);
    } else {
        UnknownFunction46eb30(0x514, 0);
    }
    UnknownFunction4f3260();
    g_UnknownGlobal56e26c->UnknownFunction521970(0x610, text, 0x80);
    UnknownGameUiControl* button = UnknownFunction46ebf0("ButRdyUser", 1);
    button->UnknownFunction470b20(text);
    if (g_UnknownGlobal56e26c->field_0x08->isHost)
        g_UnknownGlobal56e26c->UnknownFunction521970(0x925, text, 0x80);
    for (int i = 0; i < 7; i++) {
        sprintf(name, "ButRdyPlayer%d", i + 2);
        button = UnknownFunction46ebf0(name, 1);
        button->UnknownFunction470b20(text);
    }
}

// 0x004f38a0
void MultiPlayerDlg::UnknownFunction4f38a0() {
    UnknownChatMessage message;
    UnknownGameUiControl* edit = UnknownFunction46ebf0("EditChat", 0xb);
    int length = strlen("");
    int count = length > 0x49 ? 0x49 : length;
    strncpy(message.field_0x04, "", count);
    message.field_0x04[count] = 0;
    edit->UnknownFunction473ef0(message.field_0x04, 0x4a);
    if (strcmp(message.field_0x04, "")) {
        NetworkInterface* network = g_UnknownGlobal56e26c->field_0x08;
        int player = network->field_0x0c;
        network->UnknownFunction4ac830(0x85, &message, 0x4e, player, 0);
        edit->UnknownFunction473da0("");
        UnknownFunction4f3720(player, message.field_0x04);
    }
    field_0x34->UnknownFunction487790((UnknownGuiControl*)edit, 0, 0);
}

// 0x004f3980
void MultiPlayerDlg::UnknownFunction4f3980(const char* text) {
    UnknownChatMessage message;
    int length = strlen(text);
    int count = length > 0x49 ? 0x49 : length;
    strncpy(message.field_0x04, text, count);
    message.field_0x04[count] = 0;
    g_UnknownGlobal56e26c->field_0x08->UnknownFunction4ac830(
        0x92, &message, 0x4e, g_UnknownGlobal56e26c->field_0x08->field_0x0c, 0);
    UnknownFunction4f3720(-1, message.field_0x04);
    field_0x34->UnknownFunction487790((UnknownGuiControl*)UnknownFunction46ebf0("EditChat", 0xb), 0, 0);
}

// 0x004f3a10
void MultiPlayerDlg::UnknownFunction4f3a10(int index) {
    int player = g_UnknownGlobal689d08[index].field_0x04;
    if (player && player != g_UnknownGlobal56e26c->field_0x08->field_0x0c) {
        UnknownKickMessage message;
        NetworkInterface* network = g_UnknownGlobal56e26c->field_0x08;
        message.field_0x04 = player;
        network->UnknownFunction4ac830(0x8e, &message, sizeof(message), network->field_0x0c, player);
        field_0x7f68.UnknownFunction4f2f80(message.field_0x04);
    }
}

// 0x004f57c0: reports why the ready state was cleared.
void MultiPlayerDlg::UnknownFunction4f57c0(int reason) {
    char text[0x80];
    char value[0x80];
    char message[0x80];
    switch (reason) {
    case 1:
        g_UnknownGlobal56e26c->UnknownFunction521970(0x146a, text, 0x80);
        sprintf(value, text, g_UnknownGlobal56cb6c[g_UnknownGlobal56e26c->mode.field_0x27f8.field_0x1c]);
        if (g_UnknownGlobal56e26c->mode.field_0x1bd4)
            sprintf(value, text, g_UnknownGlobal56cb6c[2]);
        g_UnknownGlobal56e26c->UnknownFunction521970(0x14d1, text, 0x80);
        sprintf(message, text, value);
        break;
    case 2: {
        g_UnknownGlobal56e26c->UnknownFunction521970(0x14d2, text, 0x80);
        int length = strlen(text);
        int count = length > 0x7f ? 0x7f : length;
        strncpy(message, text, count);
        message[count] = 0;
        break;
    }
    case 3: {
        g_UnknownGlobal56e26c->UnknownFunction521970(0x14da, text, 0x80);
        int length = strlen(text);
        int count = length > 0x7f ? 0x7f : length;
        strncpy(message, text, count);
        message[count] = 0;
        break;
    }
    }
    UnknownFunction4f3720(-1, message);
    g_UnknownGlobal689df4 = 0;
    if (reason == 2) {
        g_UnknownGlobal56e26c->UnknownFunction521970(0x14d4, text, 0x80);
        sprintf(value, text, g_UnknownGlobal56e26c->mode.field_0x00);
        UnknownFunction4f3980(value);
    } else if (reason == 3) {
        g_UnknownGlobal56e26c->UnknownFunction521970(0x14db, text, 0x80);
        sprintf(value, text, g_UnknownGlobal56e26c->mode.field_0x00);
        UnknownFunction4f3980(value);
    }
    UnknownFunction46ebf0("ButRdyUser", 1)->UnknownFunction470d40(0x808080);
}

// 0x004f5a00
void MultiPlayerDlg::UnknownFunction4f5a00(int page) {
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
            g_UnknownGlobal56e26c->ui->field_0x2c->UnknownFunction485a70(field_0x7f58, 0, 1, (int)&area, (int)this,
                                                                         0, 0, 1);
        }
        break;
    case 1:
        if (!field_0x7f5c) {
            field_0x7f5c = new(__FILE__, 0x7aa) MPBikeRiderDlg;
            g_UnknownGlobal56e26c->ui->field_0x2c->UnknownFunction485a70(field_0x7f5c, 0, 1, (int)&area, (int)this,
                                                                         0, 0, 1);
        }
        break;
    case 2:
        if (!field_0x7f60) {
            field_0x7f60 = new(__FILE__, 0x7b0) MPRaceInfoDlg;
            g_UnknownGlobal56e26c->ui->field_0x2c->UnknownFunction485a70(field_0x7f60, 0, 1, (int)&area, (int)this,
                                                                         0, 0, 1);
        }
        break;
    case 3:
        if (!field_0x7f64) {
            field_0x7f64 = new(__FILE__, 0x7b6) MPOptionsDlg;
            g_UnknownGlobal56e26c->ui->field_0x2c->UnknownFunction485a70(field_0x7f64, 0, 1, (int)&area, (int)this,
                                                                         0, 0, 1);
        }
        break;
    }
}

// 0x004f5ca0
void MultiPlayerDlg::UnknownFunction4f5ca0() {
    int value;
    int index;
    char scene[0x104];
    char path[0x104];
    UnknownFunction4f3260();
    if (!strcmp(field_0x7fc8, g_UnknownGlobal56e26c->mode.field_0x27f8.field_0x36) && field_0x80cc == g_UnknownGlobal56e26c->mode.field_0x27f8.field_0x34)
        return;
    g_UnknownGlobal56e26c->mode.UnknownFunction523b70(scene);
    g_UnknownGlobal56e26c->mode.UnknownFunction523a60((int)scene, g_UnknownGlobal56e26c->mode.field_0x27f8.field_0x36, "env", path);
    index = 0;
    value = 0;
    g_UnknownGlobal56e26c->sceneObject->UnknownFunction4e9b80(path);
    if (g_UnknownGlobal56e26c->mode.UnknownFunction524100() == 1 || g_UnknownGlobal56e26c->mode.UnknownFunction524100() == 5)
        g_UnknownGlobal56e26c->sceneObject->UnknownFunction4ea010(field_0x80d0, g_UnknownGlobal56e26c->mode.field_0x27f8.field_0x36,
                                                                  g_UnknownGlobal56e26c->mode.field_0x27f8.field_0x34, "scn", &index,
                                                                  &value);
    else
        g_UnknownGlobal56e26c->sceneObject->UnknownFunction4ea010(field_0x80d0, g_UnknownGlobal56e26c->mode.field_0x27f8.field_0x36, 0,
                                                                  "scn", &index, &value);
    if (!_stricmp("no name", field_0x80d0)) {
        g_UnknownGlobal689dfc = 2;
        if (g_UnknownGlobal689df4)
            UnknownFunction4f57c0(2);
        g_UnknownGlobal56e26c->UnknownFunction521970(0x143b, field_0x80d0, 0x80);
    } else if (value != g_UnknownGlobal56e26c->mode.field_0x27f8.field_0x13c) {
        g_UnknownGlobal689dfc = 3;
        if (g_UnknownGlobal689df4)
            UnknownFunction4f57c0(3);
    } else {
        g_UnknownGlobal689dfc = 0;
    }
    if (field_0x7f58)
        field_0x7f58->UnknownFunction4f75d0();
    {
        int length = strlen(g_UnknownGlobal56e26c->mode.field_0x27f8.field_0x36);
        int count = length > 0x103 ? 0x103 : length;
        strncpy(field_0x7fc8, g_UnknownGlobal56e26c->mode.field_0x27f8.field_0x36, count);
        field_0x7fc8[count] = 0;
    }
    field_0x80cc = g_UnknownGlobal56e26c->mode.field_0x27f8.field_0x34;
    {
        int length = strlen(field_0x80d0);
        int count = length > 0x7f ? 0x7f : length;
        strncpy(g_UnknownGlobal56e26c->mode.field_0x6f4[g_UnknownGlobal56e26c->mode.field_0x27f8.field_0x04], field_0x80d0, count);
        g_UnknownGlobal56e26c->mode.field_0x6f4[g_UnknownGlobal56e26c->mode.field_0x27f8.field_0x04][count] = 0;
    }
}

// 0x004f5f30
int MPEventDlg::UnknownVirtualSlot24(int type, void* data, int from, int to, int flags) {
    char text[0x80];
    if (type == 0x101) {
        int count = g_UnknownGlobal56e26c->mode.field_0x27f8.field_0x28;
        field_0x7f58 = 0;
        UnknownFunction46ecc0(0);
        UnknownFunction4f6e60();
        field_0x7f58 = 1;
        UnknownFunction46ecc0(0);
        UnknownGameUiControl* list = UnknownFunction46ebf0("OpponentsListBox", 3);
        if (g_UnknownGlobal56e26c->mode.field_0x27f8.field_0x04 != 4 && list->field_0x1ec - 1 != g_UnknownGlobal56e26c->mode.field_0x1bd8) {
            list->UnknownFunction4775f0();
            for (int i = 0; i <= g_UnknownGlobal56e26c->mode.field_0x1bd8; i++) {
                sprintf(text, "%d", i);
                list->UnknownFunction476d80(text, i, 0);
            }
            if (g_UnknownGlobal56e26c->mode.field_0x1bd8 < count)
                list->UnknownFunction476a60(g_UnknownGlobal56e26c->mode.field_0x1bd8);
            else
                list->UnknownFunction476a60(count);
        }
    } else if (from && type == 0xf) {
        UnknownFunction4f6070();
        UnknownFunction4f69d0();
    }
    return 0;
}

// 0x004f6070
void MPEventDlg::UnknownFunction4f6070() {
    char text[0x80];
    MultiPlayerDlg* parent = field_0x2c;
    UnknownGameUiControl* list = UnknownFunction46ebf0("DDLTextBox", 6)->field_0x1fc;
    list->UnknownFunction4775f0();
    list->UnknownFunction476d80(parent->field_0x80d0, 0, 0);
    list->UnknownFunction476a60(0);
    list = UnknownFunction46ebf0("EventTypeDropDown", 6)->field_0x1fc;
    list->UnknownFunction476b30(g_UnknownGlobal56e26c->mode.field_0x27f8.field_0x04);
    list = UnknownFunction46ebf0("RaceModeDropDown", 6)->field_0x1fc;
    list->UnknownFunction476b30(g_UnknownGlobal56e26c->mode.field_0x27f8.field_0x00);
    if (g_UnknownGlobal56e26c->mode.field_0x27f8.field_0x04 == 4) {
        UnknownFunction46ebf0("OpponentsControlBox", 5)->UnknownFunction470660(0, 1);
        UnknownGameUiControl* check = UnknownFunction46ebf0("ChkTagBall", 2);
        check->UnknownFunction470660(1, 1);
        check->UnknownFunction478cf0(g_UnknownGlobal56e26c->mode.field_0x27f8.field_0x144);
        check = UnknownFunction46ebf0("ChkStuntMode", 2);
        check->UnknownFunction470660(1, 1);
        check->UnknownFunction478cf0(g_UnknownGlobal56e26c->mode.field_0x27f8.field_0x148);
    } else {
        UnknownFunction46ebf0("OpponentsControlBox", 5)->UnknownFunction470660(1, 1);
        UnknownGameUiControl* opponents = UnknownFunction46ebf0("OpponentsListBox", 3);
        if (opponents) {
            opponents->UnknownFunction4775f0();
            UnknownFunction451380(g_UnknownGlobal56e26c->mode.field_0x27f8.field_0x28, text);
            opponents->UnknownFunction476d80(text, g_UnknownGlobal56e26c->mode.field_0x27f8.field_0x28, 0);
        }
        UnknownFunction46ebf0("ChkTagBall", 2)->UnknownFunction470660(0, 1);
        UnknownFunction46ebf0("ChkStuntMode", 2)->UnknownFunction470660(0, 1);
    }
    if (g_UnknownGlobal56e26c->mode.field_0x27f8.field_0x00 != 2)
        g_UnknownGlobal56e26c->mode.field_0x10ec = 0;
    if (g_UnknownGlobal56e26c->mode.field_0x27f8.field_0x04 != 0 && g_UnknownGlobal56e26c->mode.field_0x27f8.field_0x04 != 4) {
        UnknownGameUiControl* label = UnknownFunction46ebf0("LapsLabel", 0xc);
        label->UnknownFunction470a80(g_UnknownGlobal56e26c->field_0x420, 0x929);
        UnknownGameUiControl* laps = UnknownFunction46ebf0("LapsListBox", 3);
        laps->UnknownFunction4775f0();
        UnknownFunction451380(g_UnknownGlobal56e26c->mode.field_0x27f8.field_0x20, text);
        laps->UnknownFunction476d80(text, g_UnknownGlobal56e26c->mode.field_0x27f8.field_0x20, 0);
    } else {
        UnknownGameUiControl* label = UnknownFunction46ebf0("LapsLabel", 0xc);
        label->UnknownFunction470a80(g_UnknownGlobal56e26c->field_0x420, 0x92f);
        UnknownGameUiControl* laps = UnknownFunction46ebf0("LapsListBox", 3);
        laps->UnknownFunction4775f0();
        UnknownFunction518640(text, g_UnknownGlobal56e26c->mode.field_0x27f8.field_0x140 * 60.0f);
        laps->UnknownFunction476d80(text, (int)g_UnknownGlobal56e26c->mode.field_0x27f8.field_0x140, 0);
    }
    if (g_UnknownGlobal56e26c->mode.field_0x1bd0 > 0 && g_UnknownGlobal56e26c->mode.field_0x1bd0 < 0x65)
        g_UnknownGlobal56e26c->mode.field_0x1bcc = g_UnknownGlobal56e26c->mode.field_0x1bd0;
}

// 0x004f6370
void MPEventDlg::UnknownVirtualSlot29(UnknownDialogEvent* event) {
    char text[0x80];
    char number[0x80];
    switch (event->field_0x08) {
    case 5: {
        field_0x7f58 = 0;
        UnknownRaceSettings* settings = (UnknownRaceSettings*)&g_UnknownGlobal56e26c->mode.field_0x27f8.field_0x00;
        UnknownGameUiControl* list = UnknownFunction46ebf0("EventTypeDropDown", 6)->field_0x1fc;
        g_UnknownGlobal56e26c->UnknownFunction521970(0x13ed, text, 0x80);
        list->UnknownFunction476d80(text, 1, 0);
        g_UnknownGlobal56e26c->UnknownFunction521970(0x13ee, text, 0x80);
        list->UnknownFunction476d80(text, 0, 0);
        g_UnknownGlobal56e26c->UnknownFunction521970(0x13f0, text, 0x80);
        list->UnknownFunction476d80(text, 5, 0);
        g_UnknownGlobal56e26c->UnknownFunction521970(0x13ef, text, 0x80);
        list->UnknownFunction476d80(text, 2, 0);
        g_UnknownGlobal56e26c->UnknownFunction521970(0x13f2, text, 0x80);
        list->UnknownFunction476d80(text, 3, 0);
        g_UnknownGlobal56e26c->UnknownFunction521970(0x13f1, text, 0x80);
        list->UnknownFunction476b30(settings->field_0x04);
        list = UnknownFunction46ebf0("RaceModeDropDown", 6)->field_0x1fc;
        g_UnknownGlobal56e26c->UnknownFunction521970(0x13f3, text, 0x80);
        list->UnknownFunction476d80(text, 0, 0);
        g_UnknownGlobal56e26c->UnknownFunction521970(0x13f4, text, 0x80);
        list->UnknownFunction476d80(text, 1, 0);
        g_UnknownGlobal56e26c->UnknownFunction521970(0x13f5, text, 0x80);
        list->UnknownFunction476d80(text, 2, 1);
        list->UnknownFunction476a60(settings->field_0x00);
        UnknownFunction46ebf0("ChkTagBall", 0)->UnknownVirtualSlot54(&settings->field_0x144);
        UnknownFunction46ebf0("ChkStuntMode", 0)->UnknownVirtualSlot54(&settings->field_0x148);
        int count = g_UnknownGlobal56e26c->mode.field_0x27f8.field_0x28;
        UnknownFunction46ecc0(0);
        UnknownFunction4f6e60();
        UnknownFunction4f69d0();
        UnknownGameUiControl* opponents = UnknownFunction46ebf0("OpponentsListBox", 3);
        if (g_UnknownGlobal56e26c->field_0x08->isHost && settings->field_0x04 != 4 &&
            opponents->field_0x1ec - 1 != g_UnknownGlobal56e26c->mode.field_0x1bd8) {
            opponents->UnknownFunction4775f0();
            for (int i = 0; i <= g_UnknownGlobal56e26c->mode.field_0x1bd8; i++) {
                sprintf(number, "%d", i);
                opponents->UnknownFunction476d80(number, i, 0);
            }
            if (g_UnknownGlobal56e26c->mode.field_0x1bd8 < count)
                opponents->UnknownFunction476a60(g_UnknownGlobal56e26c->mode.field_0x1bd8);
            else
                opponents->UnknownFunction476a60(count);
        }
        if (!g_UnknownGlobal56e26c->field_0x08->isHost)
            UnknownFunction4f75d0();
        break;
    }
    case 1:
        if (!_stricmp("TrackLeft", event->field_0x04)) {
            UnknownGameUiControl* list = UnknownFunction46ebf0("DDLTextBox", 6)->field_0x1fc;
            int rows = list->field_0x1ec;
            list->UnknownFunction476a60((list->UnknownFunction476950() + rows - 1) % rows);
            list->UnknownVirtualSlot66(0);
        } else if (!_stricmp("TrackRight", event->field_0x04)) {
            UnknownGameUiControl* list = UnknownFunction46ebf0("DDLTextBox", 6)->field_0x1fc;
            int rows = list->field_0x1ec;
            list->UnknownFunction476a60((list->UnknownFunction476950() + 1) % rows);
            list->UnknownVirtualSlot66(0);
        } else if (!_stricmp("RacesLeftArrow", event->field_0x04) || !_stricmp("RacesRightArrow", event->field_0x04)) {
            g_UnknownGlobal56e26c->mode.field_0x27f8.field_0x0c = UnknownFunction46ebf0("RacesListBox", 3)->UnknownFunction4768d0(-1);
        }
        break;
    case 2:
        if (!_stricmp("RaceModeDropDown", event->field_0x04)) {
            UnknownGameUiControl* list = UnknownFunction46ebf0("RaceModeDropDown", 6)->field_0x1fc;
            g_UnknownGlobal56e26c->mode.field_0x27f8.field_0x00 = list->UnknownFunction4768d0(-1);
            UnknownFunction4f69d0();
            if (event->field_0x14->UnknownFunction4768d0(-1) != 2) {
                g_UnknownGlobal56e26c->mode.field_0x10ec = 0;
                g_UnknownGlobal56e26c->field_0x08->UnknownFunction4ac510(1);
            }
        } else if (!_stricmp("EventTypeDropDown", event->field_0x04)) {
            UnknownFunction46ecc0(1);
            UnknownFunction4f6e60();
            UnknownFunction4f69d0();
        } else if (!_stricmp("DDLTextBox", event->field_0x04)) {
            int index = event->field_0x14->UnknownFunction4768d0(-1);
            UnknownRaceSettings* settings = (UnknownRaceSettings*)&g_UnknownGlobal56e26c->mode.field_0x27f8.field_0x00;
            g_UnknownGlobal56e26c->mode.UnknownFunction5240e0(g_UnknownGlobal56e26c->ui->field_0x60[index].field_0x08);
            {
                char* name = g_UnknownGlobal56e26c->ui->field_0x60[index].field_0x14;
                int length = strlen(name);
                int count = length > 0xff ? 0xff : length;
                strncpy(settings->field_0x36, name, count);
                settings->field_0x36[count] = 0;
            }
            settings->field_0x34 = g_UnknownGlobal56e26c->ui->field_0x60[index].field_0x04;
            settings->field_0x138 = g_UnknownGlobal56e26c->ui->field_0x60[index].field_0x0c;
            settings->field_0x13c = g_UnknownGlobal56e26c->ui->field_0x60[index].field_0x10;
            strcpy(g_UnknownGlobal56e26c->mode.field_0x6f4[g_UnknownGlobal56e26c->mode.field_0x27f8.field_0x04],
                   event->field_0x14->UnknownFunction476d20(-1));
        }
        break;
    case 6:
        if (g_UnknownGlobal56e26c->mode.field_0x27f8.field_0x04 != 4) {
            UnknownGameUiControl* opponents = UnknownFunction46ebf0("OpponentsListBox", 3);
            g_UnknownGlobal56e26c->mode.field_0x27f8.field_0x28 = opponents->UnknownFunction4768d0(-1);
        } else
            g_UnknownGlobal56e26c->mode.field_0x27f8.field_0x28 = 0;
        break;
    }
}

// 0x004f69d0: enables the controls the player may change.
void MPEventDlg::UnknownFunction4f69d0() {
    char text[0x20];
    GameObjectIterator controls(field_0x7f3c, 1, "UIControl");
    int enable = g_UnknownGlobal56e26c->field_0x08->isHost && !(g_UnknownGlobal56e26c->mode.field_0x1bd4 & 1);
    for (GameObject* control = controls.Next(); control; control = controls.Next())
        ((UnknownGameUiControl*)control)->UnknownVirtualSlot49(enable);
    if (enable) {
        UnknownFunction46eb30(0x141, 1);
        UnknownFunction46ebf0("EventTypeDropDown", 6)->UnknownInlineButton()->UnknownFunction470660(1, 1);
        UnknownFunction46ebf0("RaceModeDropDown", 6)->UnknownInlineButton()->UnknownFunction470660(1, 1);
    } else {
        UnknownFunction46eb30(0x141, 0);
        UnknownFunction46ebf0("EventTypeDropDown", 6)->UnknownInlineButton()->UnknownFunction470660(0, 1);
        UnknownFunction46ebf0("RaceModeDropDown", 6)->UnknownInlineButton()->UnknownFunction470660(0, 1);
        if (g_UnknownGlobal56e26c->field_0x08->isHost && !(g_UnknownGlobal56e26c->mode.field_0x1bd4 & 2)) {
            UnknownFunction46ebf0("DDLTextBox", 6)->UnknownVirtualSlot49(1);
            UnknownFunction46ebf0("TrackLeft", 0)->UnknownVirtualSlot49(1);
            UnknownFunction46ebf0("TrackLeft", 0)->UnknownFunction470660(1, 1);
            UnknownFunction46ebf0("TrackRight", 0)->UnknownVirtualSlot49(1);
            UnknownFunction46ebf0("TrackRight", 0)->UnknownFunction470660(1, 1);
        }
    }
    UnknownGameUiControl* list = UnknownFunction46ebf0("RaceModeDropDown", 6)->field_0x1fc;
    int mode = list->UnknownFunction4768d0(-1);
    UnknownGameUiControl* lapsBox = UnknownFunction46ebf0("LapsControlBox", 5);
    UnknownGameUiControl* racesBox = UnknownFunction46ebf0("RacesControlBox", 5);
    switch (mode) {
    case 0:
        lapsBox->UnknownFunction470660(0, 1);
        racesBox->UnknownFunction470660(0, 1);
        break;
    case 1:
        lapsBox->UnknownFunction470660(1, 1);
        racesBox->UnknownFunction470660(0, 1);
        break;
    case 2:
        lapsBox->UnknownFunction470660(1, 1);
        racesBox->UnknownFunction470660(1, 1);
        break;
    }
    UnknownGameUiControl* racesLeft = UnknownFunction46ebf0("RacesLeftArrow", 9);
    UnknownGameUiControl* racesRight = UnknownFunction46ebf0("RacesRightArrow", 0xa);
    UnknownGameUiControl* opponentsLeft = UnknownFunction46ebf0("OpponentsLeftArrow", 0);
    UnknownGameUiControl* opponentsRight = UnknownFunction46ebf0("OpponentsRightArrow", 0);
    UnknownGameUiControl* racesLabel = UnknownFunction46ebf0("RacesLabel", 0xc);
    UnknownGameUiControl* races = UnknownFunction46ebf0("RacesListBox", 3);
    UnknownGameUiControl* opponents = UnknownFunction46ebf0("OpponentsListBox", 3);
    int selection;
    if (g_UnknownGlobal56e26c->field_0x08->isHost && races->field_0x1ec)
        selection = races->UnknownFunction4768d0(-1);
    else
        selection = g_UnknownGlobal56e26c->mode.field_0x27f8.field_0x0c;
    races->UnknownFunction4775f0();
    for (int i = 3; i <= 7; i += 2) {
        sprintf(text, "%d", i);
        races->UnknownFunction476d80(text, i - 1, 0);
    }
    races->UnknownFunction476b30(selection);
    if (g_UnknownGlobal56e26c->mode.field_0x27f8.field_0x00 == 2 && g_UnknownGlobal56e26c->mode.field_0x10ec) {
        if (g_UnknownGlobal56e26c->field_0x08->isHost) {
            opponentsLeft->UnknownFunction470660(0, 1);
            opponentsRight->UnknownFunction470660(0, 1);
            opponents->UnknownVirtualSlot49(0);
            racesLeft->UnknownFunction470660(0, 1);
            racesRight->UnknownFunction470660(0, 1);
        }
        racesLabel->UnknownFunction470a80(g_UnknownGlobal56e26c->field_0x420, 0x92b);
        sprintf(text, "%d/%d", g_UnknownGlobal56e26c->eventManager->field_0x48 + 1, g_UnknownGlobal56e26c->mode.field_0x27f8.field_0x0c + 1);
        races->UnknownFunction4775f0();
        races->UnknownFunction476d80(text, 0, 0);
    } else {
        if (g_UnknownGlobal56e26c->field_0x08->isHost) {
            opponentsLeft->UnknownFunction470660(1, 1);
            opponentsRight->UnknownFunction470660(1, 1);
            opponents->UnknownVirtualSlot49(1);
            racesLeft->UnknownFunction470660(1, 1);
            racesRight->UnknownFunction470660(1, 1);
        }
        racesLabel->UnknownFunction470a80(g_UnknownGlobal56e26c->field_0x420, 0x92a);
    }
}

// 0x004f6e60
void MPEventDlg::UnknownFunction4f6e60() {
    char text[0x100];
    if (g_UnknownGlobal56e26c->field_0x08->isHost)
        g_UnknownGlobal689dfc = 0;
    UnknownGameUiControl* list = UnknownFunction46ebf0("EventTypeDropDown", 6)->field_0x1fc;
    int type = list->UnknownFunction4768d0(-1);
    int& eventType = g_UnknownGlobal56e26c->mode.field_0x27f8.field_0x04;
    eventType = type;
    UnknownFunction46ebf0("OpponentsListBox", 3);
    UnknownGameUiControl* laps = UnknownFunction46ebf0("LapsListBox", 3);
    UnknownFunction46ebf0("RacesListBox", 3);
    UnknownGameUiControl* label = UnknownFunction46ebf0("LapsLabel", 0xc);
    label->UnknownFunction470a80(g_UnknownGlobal56e26c->field_0x420, 0x929);
    if (type != 4) {
        UnknownFunction46ebf0("OpponentsControlBox", 5)->UnknownFunction470660(1, 1);
        UnknownFunction46ebf0("ChkTagBall", 2)->UnknownFunction470660(0, 1);
        UnknownFunction46ebf0("ChkStuntMode", 2)->UnknownFunction470660(0, 1);
    } else {
        UnknownFunction46ebf0("OpponentsControlBox", 5)->UnknownFunction470660(0, 1);
        UnknownFunction46ebf0("ChkTagBall", 2)->UnknownFunction470660(1, 1);
        UnknownFunction46ebf0("ChkStuntMode", 2)->UnknownFunction470660(1, 1);
    }
    if (type != 0 && type != 4) {
        laps->UnknownFunction4775f0();
        for (int i = type == 2 || type == 3 ? 2 : 1; i <= 5; i++) {
            sprintf(text, "%d", i);
            laps->UnknownFunction476d80(text, i, 0);
        }
        laps->UnknownFunction476b30(g_UnknownGlobal56e26c->mode.field_0x27f8.field_0x20);
    } else {
        label->UnknownFunction470a80(g_UnknownGlobal56e26c->field_0x420, 0x92f);
        laps->UnknownFunction4775f0();
        for (int i = 2; i <= 5; i++) {
            UnknownFunction518640(text, i * 60.0f);
            laps->UnknownFunction476d80(text, i, 0);
        }
        laps->UnknownFunction476b30((int)g_UnknownGlobal56e26c->mode.field_0x27f8.field_0x140);
    }
    if (g_UnknownGlobal56e26c->field_0x08->isHost) {
        if (eventType == 0) {
            g_UnknownGlobal56e26c->mode.UnknownFunction5240e0(0);
            UnknownFunction4f3080(0, "PictureBox", "DDLTextBox", 0, this, 0);
            g_UnknownGlobal56e26c->mode.UnknownFunction5240e0(5);
            UnknownFunction4f3080(0, "PictureBox", "DDLTextBox", 0, this, 1);
        } else if (eventType == 4) {
            g_UnknownGlobal56e26c->mode.UnknownFunction5240e0(4);
            UnknownFunction4f3080(0, "PictureBox", "DDLTextBox", 0, this, 0);
            g_UnknownGlobal56e26c->mode.UnknownFunction5240e0(1);
            UnknownFunction4f3080(0, "PictureBox", "DDLTextBox", 0, this, 1);
            g_UnknownGlobal56e26c->mode.UnknownFunction5240e0(5);
            UnknownFunction4f3080(0, "PictureBox", "DDLTextBox", 0, this, 1);
            g_UnknownGlobal56e26c->mode.UnknownFunction5240e0(0);
            UnknownFunction4f3080(0, "PictureBox", "DDLTextBox", 0, this, 1);
        } else {
            g_UnknownGlobal56e26c->mode.UnknownFunction5240e0(type);
            UnknownFunction4f3080(0, "PictureBox", "DDLTextBox", 0, this, 0);
        }
        list = UnknownFunction46ebf0("DDLTextBox", 6)->field_0x1fc;
        list->UnknownFunction476ad0(g_UnknownGlobal56e26c->mode.field_0x6f4[type]);
    } else {
        MultiPlayerDlg* parent = field_0x2c;
        list = UnknownFunction46ebf0("DDLTextBox", 6)->field_0x1fc;
        list->UnknownFunction4775f0();
        list->UnknownFunction476d80(parent->field_0x80d0, 0, 0);
        list->UnknownFunction476a60(0);
    }
    g_UnknownGlobal56e26c->mode.UnknownFunction5240e0(g_UnknownGlobal56e26c->mode.field_0x27f8.field_0x08);
}

// 0x004f7210
void MPEventDlg::UnknownVirtualSlot31(int apply) {
    UnknownRaceSettings* settings = (UnknownRaceSettings*)&g_UnknownGlobal56e26c->mode.field_0x27f8.field_0x00;
    if (!g_UnknownGlobal56e26c->field_0x08 || !g_UnknownGlobal56e26c->field_0x08->isHost)
        return;
    if (apply) {
        if (settings->field_0x04 != 0 && settings->field_0x04 != 4)
            settings->field_0x20 = UnknownFunction46ebf0("LapsListBox", 3)->UnknownFunction4768d0(-1);
        else
            settings->field_0x140 = (float)UnknownFunction46ebf0("LapsListBox", 3)->UnknownFunction4768d0(-1);
        if (!g_UnknownGlobal56e26c->mode.field_0x10ec)
            settings->field_0x0c = UnknownFunction46ebf0("RacesListBox", 3)->UnknownFunction4768d0(-1);
        strcpy(settings->field_0x36, "");
        UnknownGameUiControl* list = UnknownFunction46ebf0("DDLTextBox", 6)->field_0x1fc;
        int index = list->UnknownFunction4768d0(-1);
        {
            char* name = g_UnknownGlobal56e26c->ui->field_0x60[index].field_0x14;
            int length = strlen(name);
            int count = length > 0xff ? 0xff : length;
            strncpy(settings->field_0x36, name, count);
            settings->field_0x36[count] = 0;
        }
        g_UnknownGlobal56e26c->mode.UnknownFunction5240e0(g_UnknownGlobal56e26c->ui->field_0x60[index].field_0x08);
        settings->field_0x34 = g_UnknownGlobal56e26c->ui->field_0x60[index].field_0x04;
        settings->field_0x138 = g_UnknownGlobal56e26c->ui->field_0x60[index].field_0x0c;
        settings->field_0x13c = g_UnknownGlobal56e26c->ui->field_0x60[index].field_0x10;
        strcpy(g_UnknownGlobal56e26c->mode.field_0x6f4[g_UnknownGlobal56e26c->mode.field_0x27f8.field_0x04], list->UnknownFunction476d20(-1));
        g_UnknownGlobal56e26c->mode.field_0x9f4[g_UnknownGlobal56e26c->mode.field_0x27f8.field_0x04] = g_UnknownGlobal56e26c->mode.field_0x27f8.field_0x34;
        list = UnknownFunction46ebf0("EventTypeDropDown", 6)->field_0x1fc;
        settings->field_0x04 = list->UnknownFunction4768d0(-1);
        list = UnknownFunction46ebf0("RaceModeDropDown", 6)->field_0x1fc;
        settings->field_0x00 = list->UnknownFunction4768d0(-1);
        UnknownGameUiControl* opponents = UnknownFunction46ebf0("OpponentsListBox", 3);
        if (settings->field_0x04 != 4)
            g_UnknownGlobal56e26c->mode.field_0x27f8.field_0x28 = opponents->UnknownFunction4768d0(-1);
        else
            g_UnknownGlobal56e26c->mode.field_0x27f8.field_0x28 = 0;
    } else {
        UnknownFunction46ebf0("RacesListBox", 3)->UnknownFunction476b30(settings->field_0x0c);
        UnknownGameUiControl* laps = UnknownFunction46ebf0("LapsListBox", 3);
        if (settings->field_0x04 != 0 && settings->field_0x04 != 4)
            laps->UnknownFunction476b30(settings->field_0x20);
        else
            laps->UnknownFunction476b30((int)settings->field_0x140);
        UnknownGameUiControl* list = UnknownFunction46ebf0("EventTypeDropDown", 6)->field_0x1fc;
        list->UnknownFunction476b30(g_UnknownGlobal56e26c->mode.field_0x27f8.field_0x04);
        list = UnknownFunction46ebf0("RaceModeDropDown", 6)->field_0x1fc;
        list->UnknownFunction476b30(g_UnknownGlobal56e26c->mode.field_0x27f8.field_0x00);
        if (g_UnknownGlobal56e26c->field_0x08->field_0x14 && field_0x7f58) {
            list = UnknownFunction46ebf0("DDLTextBox", 6)->field_0x1fc;
            for (int i = 0; i < list->field_0x1ec; i++) {
                int row = list->UnknownFunction4768d0(i);
                if (!_stricmp(g_UnknownGlobal56e26c->ui->field_0x60[row].field_0x14, settings->field_0x36)) {
                    list->UnknownFunction476b30(row);
                    return;
                }
            }
        }
    }
}

// 0x004f75d0
void MPEventDlg::UnknownFunction4f75d0() {
    char path[0x104];
    UnknownGameUiControl* picture = UnknownFunction46ebf0("PictureBox", 3);
    picture->UnknownFunction4775f0();
    g_UnknownGlobal56e26c->mode.UnknownFunction523a60(g_UnknownGlobal56e26c->mode.field_0x6a0, g_UnknownGlobal56e26c->mode.field_0x27f8.field_0x36,
                                                      "env", path);
    UnknownFunction4f7640(picture, path, g_UnknownGlobal56e26c->mode.field_0x27f8.field_0x36);
}

// 0x004f7640: loads the track's picture from `directory` (else the
// "unarts.tga" placeholder).
void MPEventDlg::UnknownFunction4f7640(UnknownGameUiControl* picture, const char* directory, const char* name) {
    char file[0x104];
    char path[0x104];
    char found[0x104];
    if (!picture)
        return;
    if (!*directory) {
        sprintf(path, "%s\\unarts.tga", "ui");
        g_UnknownGlobal56e26c->UnknownVirtualSlot18(path, file);
        picture->UnknownFunction477110(file, 0, 1, 0);
        return;
    }
    UnknownTextureStream* stream = new(__FILE__, 0xa72) UnknownTextureStream((int)g_UnknownResourceManager572b44);
    if (g_UnknownGlobal56e26c->mode.field_0x27f8.field_0x04 == 1 || g_UnknownGlobal56e26c->mode.field_0x27f8.field_0x04 == 5)
        sprintf(file, "%s%02ds.tga", name, g_UnknownGlobal56e26c->mode.field_0x27f8.field_0x34);
    else if (g_UnknownGlobal56e26c->mode.UnknownFunction524100() == 1 || g_UnknownGlobal56e26c->mode.UnknownFunction524100() == 5)
        sprintf(file, "%s01s.tga", name);
    else
        sprintf(file, "%ss.tga", name);
    sprintf(path, "%s\\%s", directory, file);
    if (g_UnknownGlobal56e26c->sceneObject->UnknownFunction4e9cd0(stream, path, "rb", (int)file)) {
        picture->UnknownFunction477110(file, 0, 1, 0);
    } else if (g_UnknownGlobal56e26c->UnknownVirtualSlot18(path, found)) {
        picture->UnknownFunction477110(found, 0, 1, 0);
    } else {
        sprintf(path, "%s\\unarts.tga", "ui");
        g_UnknownGlobal56e26c->UnknownVirtualSlot18(path, file);
        picture->UnknownFunction477110(file, 0, 1, 0);
    }
    if (stream)
        delete stream;
}

// 0x004f8570
void MPBikeRiderDlg::UnknownFunction4f8570(int number) {
    UnknownBikeNumberPainter painter(g_UnknownGlobal56e26c->field_0x1c);
    for (int i = 0; i < g_UnknownGlobal56e26c->ui->field_0x4c; i++)
        painter.UnknownFunction417670(
            ((UnknownKrustyUIModel*)g_UnknownGlobal56e26c->ui->field_0x48)[i].field_0xc0->field_0x1a0, number);
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
    if (!g_UnknownGlobal56e26c->field_0x08->isHost) {
        UnknownGameUiControl* list = UnknownFunction46ebf0("LargestOpponentDropDown", 6)->field_0x1fc;
        list->UnknownFunction476b30(g_UnknownGlobal56e26c->mode.field_0x27f8.field_0x1c);
        if (UnknownBikeClassOf(((UnknownOptGarageSettings*)g_UnknownGlobal56e26c->mode.field_0xfd8)->field_0x00) >
                g_UnknownGlobal56e26c->mode.field_0x27f8.field_0x1c &&
            g_UnknownGlobal689df4)
            field_0x2c->UnknownFunction4f57c0(1);
    }
}

// 0x004f8700
void MPBikeRiderDlg::UnknownFunction4f8700() {
    if (g_UnknownGlobal56e26c->field_0x08->isHost) {
        UnknownFunction46ebf0("LargestOpponentDropDown", 0)->UnknownVirtualSlot49(1);
        UnknownFunction46ebf0("LargestOpponentDropDown", 6)->UnknownInlineButton()->UnknownFunction470660(1, 1);
    } else {
        UnknownFunction46ebf0("LargestOpponentDropDown", 0)->UnknownVirtualSlot49(0);
        UnknownFunction46ebf0("LargestOpponentDropDown", 6)->UnknownInlineButton()->UnknownFunction470660(0, 1);
    }
}

// 0x004f8780
int MPBikeRiderDlg::UnknownVirtualSlot23(UnknownControlEvent* event, UnknownInputEntry* entry) {
    if (event->kind == 1 && event->control == 0) {
        RECT area = field_0x7f8c;
        area.left += 0x32;
        area.top += 0x1e;
        area.right -= 0x32;
        area.bottom -= 0x1e;
        if (PtInRect(&area, field_0x34->field_0x2c->field_0xa4)) {
            field_0x7f78 = 1;
            field_0x7f7c = 1;
        }
    }
    return UIDialog::UnknownVirtualSlot23(event, entry);
}

// 0x004f91e0
void MPBikeRiderDlg::UnknownVirtualSlot31(int apply) {
    if (!apply)
        return;
    {
        int length = strlen("");
        int count = length > 0x3f ? 0x3f : length;
        strncpy(g_UnknownGlobal56e26c->mode.field_0x1974.field_0x00, "", count);
        g_UnknownGlobal56e26c->mode.field_0x1974.field_0x00[count] = 0;
    }
    UnknownGameUiControl* list = UnknownFunction46ebf0("DDLBikes", 6)->field_0x1fc;
    int bike = list->UnknownFunction4768d0(-1);
    {
        UnknownKrustyUIBike* bikes = (UnknownKrustyUIBike*)g_UnknownGlobal56e26c->ui->field_0x50;
        char* name = ((UnknownKrustyUIModel*)g_UnknownGlobal56e26c->ui->field_0x48)[bikes[bike].field_0x00].field_0x40;
        int length = strlen(name);
        int count = length > 0x3f ? 0x3f : length;
        strncpy(g_UnknownGlobal56e26c->mode.field_0x1974.field_0x00, name, count);
        g_UnknownGlobal56e26c->mode.field_0x1974.field_0x00[count] = 0;
    }
    {
        char* name = ((UnknownKrustyUIBike*)g_UnknownGlobal56e26c->ui->field_0x50)[bike].field_0x48;
        int length = strlen(name);
        int count = length > 0x3f ? 0x3f : length;
        strncpy(g_UnknownGlobal56e26c->mode.field_0x1974.field_0x40, name, count);
        g_UnknownGlobal56e26c->mode.field_0x1974.field_0x40[count] = 0;
    }
    {
        int length = strlen("");
        int count = length > 0x3f ? 0x3f : length;
        strncpy(g_UnknownGlobal56e26c->mode.field_0x1974.field_0x80, "", count);
        g_UnknownGlobal56e26c->mode.field_0x1974.field_0x80[count] = 0;
    }
    list = UnknownFunction46ebf0("DDLRiders", 6)->field_0x1fc;
    int rider = list->UnknownFunction4768d0(-1);
    {
        char* name = ((UnknownKrustyUIModel*)g_UnknownGlobal56e26c->ui->field_0x58)[rider].field_0x40;
        int length = strlen(name);
        int count = length > 0x3f ? 0x3f : length;
        strncpy(g_UnknownGlobal56e26c->mode.field_0x1974.field_0x80, name, count);
        g_UnknownGlobal56e26c->mode.field_0x1974.field_0x80[count] = 0;
    }
}

// 0x004f93c0
void MPOptionsDlg::UnknownVirtualSlot29(UnknownDialogEvent* event) {
    char text[0x80];
    switch (event->field_0x08) {
    case 1:
        if (!_stricmp("ChkRecordRace", event->field_0x04))
            event->field_0x14->UnknownVirtualSlot59(1);
        break;
    case 5: {
        UnknownFunction46ebf0("FastFinishesCheckBox", 0)->UnknownVirtualSlot54(&g_UnknownGlobal56e26c->mode.field_0x27f8.field_0x18);
        UnknownFunction46ebf0("ChkTreeCollision", 0)->UnknownVirtualSlot54(&g_UnknownGlobal56e26c->mode.field_0x27f8.field_0x10);
        UnknownFunction46ebf0("ChkRiderCollision", 0)->UnknownVirtualSlot54(&g_UnknownGlobal56e26c->mode.field_0x27f8.field_0x14);
        UnknownFunction46ebf0("RadLODEasy", 0)->UnknownVirtualSlot54(&g_UnknownGlobal56e26c->mode.field_0x94);
        UnknownFunction46ebf0("ChkRecordRace", 0)->UnknownFunction470660(0, 1);
        UnknownFunction46ebf0("EditMaxAIBikes", 0xb)->UnknownFunction473f30("0123456789");
        UnknownGameUiControl* edit = UnknownFunction46ebf0("EditMaxAIBikes", 0xb);
        edit->UnknownFunction473da0(_itoa(g_UnknownGlobal56e26c->mode.field_0x6dc, text, 10));
        UnknownFunction46ecc0(0);
        UnknownFunction4f95b0();
        break;
    }
    case 6:
    {
        UnknownGameUiControl* edit = UnknownFunction46ebf0("EditMaxAIBikes", 0xb);
        g_UnknownGlobal56e26c->mode.field_0x6dc = atoi(edit->UnknownFunction473ef0(text, 0x80));
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
    UnknownFunction46ecc0(0);
}

// 0x004f95b0
void MPOptionsDlg::UnknownFunction4f95b0() {
    UnknownFunction46ea80(0x3f3, g_UnknownGlobal56e26c->field_0x08->isHost &&
                                     !g_UnknownGlobal56e26c->mode.field_0x1bd4);
    if (g_UnknownGlobal56e26c->mode.field_0x27f8.field_0x04 == 4)
        UnknownFunction46ebf0("ChkRecordRace", 0)->UnknownVirtualSlot49(0);
    else
        UnknownFunction46ebf0("ChkRecordRace", 0)->UnknownVirtualSlot49(1);
}
