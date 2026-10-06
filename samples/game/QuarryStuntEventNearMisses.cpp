// Near-miss QuarryStuntEvent.cpp candidates (BaseQuarryEvent), kept out of
// src/reconstructed/QuarryStuntEvent.cpp until they match.
//
// BaseQuarryEvent::UnknownFunction4e0560 (0x004e0560, 311 bytes): the race
// memory estimate. Control flow, both counting loops and the final sum match;
// retail keeps the two counts in edi/esi and reloads the TrackGame global
// after the first loop (its register goes to the hoisted model kind), while
// VC6 here keeps the global in a register and spills the first count. Local
// ui/limit/threshold, pointer-walk and declaration-order forms, and compiling
// it in the file's context, do not change it.
//
// BaseQuarryEvent::UnknownVirtualSlot12 (0x004e14a0, 1756 bytes): the
// "Atmosphere" debug page (fog and light colours with the up/down keys). The
// switch, its jump table, the fog-kind strcpy chain and all prints match;
// the prologue differs: retail loads s_DebugPage into ecx and reloads
// TrackGame+0x38 for NewPage (VC6 here reuses ecx from the null test and
// puts the page in edx), and after the first ControlInterface call retail
// loads the global before moving the result to edi. Comparison order, a
// local page, nested/early-return tests, an inline overlay accessor and
// declaration placement do not change it.
#include <stdio.h>
#include <string.h>

#include "../../src/reconstructed/QuarryEvent.h"

#include "../../src/reconstructed/ControlInterface.h"
#include "../../src/reconstructed/DebugOverlay.h"
#include "../../src/reconstructed/LightEmitter.h"
#include "../../src/reconstructed/TrackGame.h"

// 0x00572150 and 0x00689bbc (QuarryStuntEvent.cpp's debug page and row).
static int s_DebugPage = -1;
static int s_DebugRow;

// The bike model table at KrustyUI+0x48 (0xc8-byte records) and the choices
// at +0x50 (0x94 bytes, a model index first); +0x58 holds more 0xc8-byte
// model records.
struct UnknownQuarryBikeModel {
    unsigned char field_0x00[0xc4];
    int field_0xc4;                           // model kind
};

struct UnknownQuarryBikeChoice {
    int field_0x00;                           // model index
    unsigned char field_0x04[0x94 - 0x04];
};

// 0x004e0560
int BaseQuarryEvent::UnknownFunction4e0560() {
    int players = 0;
    int opponents = 0;
    if (g_UnknownGlobal56e26c->UnknownFunction521cd0()) {
        int i;
        for (i = 0; i < g_UnknownGlobal56e26c->ui->field_0x54; i++) {
            if (((UnknownQuarryBikeModel*)g_UnknownGlobal56e26c->ui->field_0x48)[((UnknownQuarryBikeChoice*)g_UnknownGlobal56e26c->ui->field_0x50)[i].field_0x00].field_0xc4 <= g_UnknownGlobal56e26c->field_0x3444->field_0x48)
                players++;
        }
        for (i = 0; i < g_UnknownGlobal56e26c->ui->field_0x5c; i++) {
            if (((UnknownQuarryBikeModel*)g_UnknownGlobal56e26c->ui->field_0x58)[i].field_0xc4 <= g_UnknownGlobal56e26c->field_0x3444->field_0x48)
                opponents++;
        }
        if (players >= g_UnknownGlobal56e26c->field_0x3444->field_0x460)
            players = g_UnknownGlobal56e26c->field_0x3444->field_0x460;
        if (opponents >= g_UnknownGlobal56e26c->field_0x3444->field_0x460)
            opponents = g_UnknownGlobal56e26c->field_0x3444->field_0x460;
    } else {
        if (g_UnknownGlobal56e26c->field_0x18 == 1) {
            if (g_UnknownGlobal56e26c->mode.field_0x27f8.field_0x00 == 4)
                opponents = 1;
            else
                opponents = g_UnknownGlobal56e26c->mode.field_0x27f8.field_0x24 + 1;
        } else {
            opponents = g_UnknownGlobal56e26c->mode.field_0x27f8.field_0x28 + g_UnknownGlobal56e26c->field_0x18;
        }
        players = opponents;
    }
    return opponents * 0x80000 / 3 + opponents * 0x8000 / 3 + players * 0x80000 / 3 + 0x80000 / 3 +
           (g_UnknownGlobal56e26c->field_0x560 ? 2 * (0x80000 / 3) : 3 * (0x80000 / 3));
}

// 0x004e14a0
int BaseQuarryEvent::UnknownVirtualSlot12() {
    GameObject::UnknownVirtualSlot12();
    if (g_UnknownGlobal56e26c->field_0x2d4_bit2 && g_UnknownGlobal56e26c->field_0x38) {
        if (s_DebugPage < 0)
            s_DebugPage = g_UnknownGlobal56e26c->field_0x38->NewPage();
        if (g_UnknownGlobal56e26c->field_0x38->field_0x26c4 == s_DebugPage) {
            char atmosphere[0x40];
            int down = g_UnknownGlobal56e26c->field_0x14->UnknownVirtualSlot3(0xd0, 0, 0x80, 0);
            int up = g_UnknownGlobal56e26c->field_0x14->UnknownVirtualSlot3(0xc8, 0, 0x80, 0);
            if (up || down) {
                unsigned int sun = field_0x80->UnknownFunction49dfd0();
                unsigned int ambient = field_0x84->UnknownFunction49dfd0();
                Fog* fog = field_0x88;
                float visibility;
                float haziness;
                unsigned int color;
                int value;
                if (fog) {
                    visibility = fog->field_0x38;
                    haziness = fog->field_0x40;
                    color = fog->field_0x2c;
                } else {
                    visibility = 0.0f;
                    haziness = 0.0f;
                    color = 0;
                }
                switch (s_DebugRow) {
                case 0:
                    if (up) {
                        visibility += 0.05f;
                        if (visibility > 1.0f)
                            visibility = 1.0f;
                    } else if (down) {
                        visibility -= 0.05f;
                        if (visibility < 0.0f)
                            visibility = 0.0f;
                    }
                    break;
                case 1:
                    if (up) {
                        haziness -= 0.05f;
                        if (haziness < 0.0f)
                            haziness = 0.0f;
                    } else if (down) {
                        haziness += 0.05f;
                        if (haziness > 1.0f)
                            haziness = 1.0f;
                    }
                    break;
                case 2:
                    value = (color >> 16) & 0xff;
                    if (up) {
                        if (++value > 0xff)
                            value = 0xff;
                    } else if (down) {
                        if (--value < 0)
                            value = 0;
                    }
                    color = (color & 0xff00ffff) | (value << 16);
                    break;
                case 3:
                    value = (color >> 8) & 0xff;
                    if (up) {
                        if (++value > 0xff)
                            value = 0xff;
                    } else if (down) {
                        if (--value < 0)
                            value = 0;
                    }
                    color = (color & 0xffff00ff) | (value << 8);
                    break;
                case 4:
                    value = color & 0xff;
                    if (up) {
                        if (++value > 0xff)
                            value = 0xff;
                    } else if (down) {
                        if (--value < 0)
                            value = 0;
                    }
                    color = (color & 0xffffff00) | value;
                    break;
                case 5:
                    value = (sun >> 16) & 0xff;
                    if (up) {
                        if (++value > 0xff)
                            value = 0xff;
                    } else if (down) {
                        if (--value < 0)
                            value = 0;
                    }
                    sun = (sun & 0xff00ffff) | (value << 16);
                    break;
                case 6:
                    value = (sun >> 8) & 0xff;
                    if (up) {
                        if (++value > 0xff)
                            value = 0xff;
                    } else if (down) {
                        if (--value < 0)
                            value = 0;
                    }
                    sun = (sun & 0xffff00ff) | (value << 8);
                    break;
                case 7:
                    value = sun & 0xff;
                    if (up) {
                        if (++value > 0xff)
                            value = 0xff;
                    } else if (down) {
                        if (--value < 0)
                            value = 0;
                    }
                    sun = (sun & 0xffffff00) | value;
                    break;
                case 8:
                    value = (ambient >> 16) & 0xff;
                    if (up) {
                        if (++value > 0xff)
                            value = 0xff;
                    } else if (down) {
                        if (--value < 0)
                            value = 0;
                    }
                    ambient = (ambient & 0xff00ffff) | (value << 16);
                    break;
                case 9:
                    value = (ambient >> 8) & 0xff;
                    if (up) {
                        if (++value > 0xff)
                            value = 0xff;
                    } else if (down) {
                        if (--value < 0)
                            value = 0;
                    }
                    ambient = (ambient & 0xffff00ff) | (value << 8);
                    break;
                case 10:
                    value = ambient & 0xff;
                    if (up) {
                        if (++value > 0xff)
                            value = 0xff;
                    } else if (down) {
                        if (--value < 0)
                            value = 0;
                    }
                    ambient = (ambient & 0xffffff00) | value;
                    break;
                }
                if (fog)
                    fog->UnknownFunction4627a0(color, visibility, haziness);
                field_0x80->UnknownFunction49e020(sun);
                field_0x84->UnknownFunction49e020(ambient);
                if (field_0x48)
                    field_0x48->UnknownFunction45a9a0();
            }
            if (field_0x88) {
                if (field_0x88->field_0x44 == 0x100)
                    strcpy(atmosphere, "(Table Fog)");
                else if (field_0x88->field_0x44 == 0x80)
                    strcpy(atmosphere, "(Vertex Fog)");
                else if (field_0x88->field_0x44 == 0x10000)
                    strcpy(atmosphere, "(Range Fog)");
            } else {
                strcpy(atmosphere, "(No Fog)");
            }
            g_UnknownGlobal56e26c->field_0x38->UnknownFunction447fa0(s_DebugPage, "Atmosphere %s", atmosphere);
            if (field_0x88) {
                g_UnknownGlobal56e26c->field_0x38->UnknownFunction447f40(s_DebugPage, "Visibility %f", field_0x88->field_0x38);
                g_UnknownGlobal56e26c->field_0x38->UnknownFunction447f40(s_DebugPage, "Haziness %f", field_0x88->field_0x40);
                g_UnknownGlobal56e26c->field_0x38->UnknownFunction447f40(s_DebugPage, "Fog R      %d", (field_0x88->field_0x2c >> 16) & 0xff);
                g_UnknownGlobal56e26c->field_0x38->UnknownFunction447f40(s_DebugPage, "Fog G      %d", (field_0x88->field_0x2c >> 8) & 0xff);
                g_UnknownGlobal56e26c->field_0x38->UnknownFunction447f40(s_DebugPage, "Fog B      %d", field_0x88->field_0x2c & 0xff);
            }
            g_UnknownGlobal56e26c->field_0x38->UnknownFunction447f40(s_DebugPage, "SunLight R %d", (field_0x80->UnknownFunction49dfd0() >> 16) & 0xff);
            g_UnknownGlobal56e26c->field_0x38->UnknownFunction447f40(s_DebugPage, "SunLight G %d", (field_0x80->UnknownFunction49dfd0() >> 8) & 0xff);
            g_UnknownGlobal56e26c->field_0x38->UnknownFunction447f40(s_DebugPage, "SunLight B %d", field_0x80->UnknownFunction49dfd0() & 0xff);
            g_UnknownGlobal56e26c->field_0x38->UnknownFunction447f40(s_DebugPage, "Ambient  R %d", (field_0x84->UnknownFunction49dfd0() >> 16) & 0xff);
            g_UnknownGlobal56e26c->field_0x38->UnknownFunction447f40(s_DebugPage, "Ambient  G %d", (field_0x84->UnknownFunction49dfd0() >> 8) & 0xff);
            g_UnknownGlobal56e26c->field_0x38->UnknownFunction447f40(s_DebugPage, "Ambient  B %d", field_0x84->UnknownFunction49dfd0() & 0xff);
            g_UnknownGlobal56e26c->field_0x38->UnknownFunction448000(s_DebugPage, s_DebugRow + 1, 1);
        }
    }
    return 0;
}
