// Near-miss bikerace.cpp candidates, kept out of src/reconstructed until
// they match. The canonical file is included first so the TU-local
// declarations, statics and inline helpers are the same.
//
// 0x0041eb20 (1708 bytes; camera target): 1699 of 1710 positions match.
// Retail loads the scene-table index (+0x150) before the table pointer when
// it addresses the entry (edx/ecx swapped at the branch head as well); a
// local index, a reference, pointer arithmetic and store orders did not
// reproduce it; neither do an entry reference, a long/unsigned index,
// index[array] or the /G, /O, /Zp flag sweep.
//
// 0x0041f1d0 (881 bytes; next/previous camera target): control flow, the
// three branches and the shared tail call 0x0041eb20(caster) (the caster
// flag lives in the dead third-argument slot) are reconstructed. Retail
// keeps the constant 0 in edx from the first instruction (cmp [mem], edx,
// stores of edx) and so orders registers differently; the candidate tests
// with `test reg, reg` until the first call.
//
// 0x0041f5e0 (2390 bytes with its two jump tables; slot 23, keys): every
// case is reconstructed, but retail keeps 0 in ebx for the whole function
// (the candidate keeps the EH state -1 there), and its case-local buffers
// sit at different stack offsets (frame 0x494 against 0x594).
//
// 0x004210f0 (2981 bytes with its jump table; the start grid): every case
// is decoded and the arithmetic matches where VC6 makes the same inlining
// choices. Retail calls the out-of-line Vector3 constructor (0x00404e60)
// for the last two expansions of the mode 2/3 case and inlines
// Scale(field_0x170, 20.0f) in the mode 4 case; this candidate does the
// opposite (VC6's inline budget). Call sequence (D dot, I FastInvSqrt,
// S scale, C constructor): retail DIS..CCDISCCCCCCDISCCCCCCDISCC-CSS+,
// candidate DIS..DISCCCCCCDISCCCCCCDISCS-SS+. Statement-count and helper
// spelling variations moved either end but never both.
//
// 0x00417ed0 (6745 bytes with its jump table at 0x0041992c; the setup,
// ret 0x24): every branch, all 166 calls in retail order, EH states 0..0x10
// and the 0x4e8-byte frame with its buffers at retail's offsets
// (0x0a8/0x1ac/0x2b0/0x3b4). 2800 of 6756 positions match; aligned
// instruction ratio 0.990. What differs: retail spills z*z of each vector
// length to a stack temporary (`fstp [t]; faddp; fadd [t]`, a shape found
// nowhere else in the binary) and keeps the youind axis in the slot it
// shares with the girl block's temporaries above the angle, so the girl
// block's temporaries (and with them most esp offsets below +0x44) sit 4
// to 12 bytes apart; one copy store of the girl offset is scheduled after
// instead of before `fadd 1.2`. Helper-function, declaration-scope and
// /G5, /G6, /Ox flag variations did not move either.
//
// 0x00419970 (13428 bytes; the loader, ret 0x14): decoded completely (the
// pro circuit branch, the network player and AI racers, the offline player,
// ghost and AI racers, the collision pairing and the 10 EH states); 194 of
// retail's 195 calls in the same order (call-sequence ratio 0.967: retail
// keeps one more stream destructor tail and lays the failure tail out
// between the AI loops). Instruction ratio 0.747 with esp offsets
// normalised: retail's 0xab0-byte frame orders the locals differently from
// this candidate's 0xac4 (register choice follows), so only 1380 of 13322
// positions match.

#include "../../src/reconstructed/BikeRace.cpp"

#include <math.h>
#include <stdlib.h>

#include "../../src/reconstructed/CarProcedural.h"
#include "../../src/reconstructed/EventManager.h"
#include "../../src/reconstructed/Krusty3DObjects.h"
#include "../../src/reconstructed/MemTag.h"
#include "../../src/reconstructed/OptionProcs.h"
#include "../../src/reconstructed/RaceSound.h"
#include "../../src/reconstructed/RaceStatus.h"
#include "../../src/reconstructed/SelectGamePicProcs.h"
#include "../../src/reconstructed/TextureMap.h"

// 0x0041eb20
int BikeRace::UnknownFunction41eb20(int caster) {
    if (UnknownFunctionCameraView()->field_0x390) {
        field_0x0b0 = 0xff;
        field_0x0ac = 0;
        if (g_TrackGame->field_0x18 > 1) {
            UnknownFunctionCameraView()->field_0x3b0->field_0x3bc->UnknownFunction4fdb50();
            UnknownFunctionCameraView()->field_0x3b0->field_0x5c4->field_0x1a0->UnknownFunction4fdb50();
            UnknownFunctionCameraView()->UnknownFunctionFollow(racerSlots[field_0x14c]);
            if (field_0x05c != 0) {
                field_0x05c->UnknownFunction4dab90();
                field_0x05c->UnknownFunction4dab00(racerSlots[field_0x14c]->field_0x3bc);
                field_0x05c->UnknownFunction4dab00(racerSlots[field_0x14c]->field_0x5c4->field_0x1a0);
            }
            if (g_TrackGame->field_0x560 != 0) {
                UnknownBikeRaceRacer* racer = racerSlots[field_0x14c];
                g_TrackGame->field_0x560->UnknownFunction404df0(racer->field_0x7b8,
                                                                          racer->field_0x7bc, racer);
            }
        } else if (field_0x14c == 0) {
            UnknownFunctionCameraView()->field_0x3b0->field_0x3bc->UnknownFunction4fdb50();
            UnknownFunctionCameraView()->field_0x3b0->field_0x5c4->field_0x1a0->UnknownFunction4fdb50();
            UnknownFunctionCameraView()->UnknownFunctionFollow(localRacer);
            if (field_0x05c != 0) {
                field_0x05c->UnknownFunction4dab90();
                field_0x05c->UnknownFunction4dab00(localRacer->field_0x3bc);
                field_0x05c->UnknownFunction4dab00(localRacer->field_0x5c4->field_0x1a0);
            }
            if (g_TrackGame->field_0x560 != 0) {
                UnknownBikeRaceRacer* racer = localRacer;
                g_TrackGame->field_0x560->UnknownFunction404df0(racer->field_0x7b8,
                                                                          racer->field_0x7bc, racer);
            }
        } else {
            UnknownFunctionCameraView()->field_0x3b0->field_0x3bc->UnknownFunction4fdb50();
            UnknownFunctionCameraView()->field_0x3b0->field_0x5c4->field_0x1a0->UnknownFunction4fdb50();
            int index = field_0x14c - 1;
            UnknownFunctionCameraView()->UnknownFunctionFollow(aiRacers[index]);
            if (field_0x05c != 0) {
                field_0x05c->UnknownFunction4dab90();
                field_0x05c->UnknownFunction4dab00(aiRacers[index]->field_0x3bc);
                field_0x05c->UnknownFunction4dab00(aiRacers[index]->field_0x5c4->field_0x1a0);
            }
            if (g_TrackGame->field_0x560 != 0) {
                UnknownBikeRaceRacer* racer = aiRacers[index];
                g_TrackGame->field_0x560->UnknownFunction404df0(racer->field_0x7b8,
                                                                          racer->field_0x7bc, racer);
            }
        }
    } else {
    if (caster) {
        field_0x0b0 = 0;
        field_0x0ac = field_0x154;
        if (raceScene->field_0xb8 != 0 && raceScene->field_0xb8->field_0x00 > 0) {
            if (field_0x05c != 0) {
                field_0x05c->UnknownFunction4dab90();
            }
            UnknownFunctionCameraView()->UnknownFunctionTargetCaster(
                raceScene->field_0xb8->field_0x04[field_0x154].field_0x04);
        } else {
            UnknownFunctionCameraView()->field_0x390 = 1;
        }
    } else {
        field_0x0b0 = 1;
        field_0x0ac = field_0x150;
        if (raceScene->field_0xb4 != 0 && raceScene->field_0xb4->field_0x00 != 0) {
            if (field_0x05c != 0) {
                field_0x05c->UnknownFunction4dab90();
            }
            UnknownSceneEntry* entry = &raceScene->field_0xb4->field_0x04[field_0x150];
            if (entry->field_0x00_bit3) {
                UnknownBikeRaceSceneCharacter* character =
                    (UnknownBikeRaceSceneCharacter*)entry->field_0x04;
                if (character->field_0x25_bit0) {
                    UnknownFunctionCameraView()->UnknownFunctionTargetCharacter(character);
                } else {
                    return 0;
                }
            } else {
                GameObject* object = (GameObject*)entry->field_0x08;
                if (((UnknownBikeRaceObjectFlags*)object)->field_0x25_bit0) {
                    UnknownFunctionCameraView()->UnknownFunctionTargetObject(object);
                } else {
                    return 0;
                }
            }
        } else {
            UnknownFunctionCameraView()->field_0x390 = 1;
        }
    }
    if (UnknownFunctionCameraView()->field_0x390) {
        field_0x0b0 = 0xff;
        field_0x0ac = 0;
    }
    }
    return 1;
}

// Inline: racer `index` in camera order (own racer first, then the AI or
// remote racers).
inline UnknownBikeRaceRacer* BikeRace::UnknownFunctionRacerAt(int index) {
    int players = g_TrackGame->field_0x18;
    if (players == 1 && index == 0) {
        return localRacer;
    }
    if (players > 1 || index < players) {
        return racerSlots[index];
    }
    return aiRacers[index - players];
}

// 0x0041f1d0
void BikeRace::UnknownFunction41f1d0(int forward, int racers, int objects) {
    int caster;
    if (g_TrackGame->uiInteractionBlocked) {
        return;
    }
    UnknownBikeRaceCameraView* camera = UnknownFunctionCameraView();
    if ((camera->field_0x268 || camera->field_0x244 == 7) &&
        (camera->field_0x244 != 7 || racers || objects)) {
        return;
    }
    if (camera->field_0x390 != racers) {
        camera->field_0x390 = racers;
        if (UnknownFunction41eb20(objects == 0)) {
            return;
        }
    }
    if (racers) {
        caster = 0;
        int others;
        if (g_TrackGame->field_0x18 > 1) {
            others = g_TrackGame->field_0x3424;
        } else if (g_TrackGame->mode.field_0x27f8.field_0x00 == 4) {
            others = 1;
        } else {
            others = g_TrackGame->mode.field_0x27f8.field_0x24;
        }
        if (g_TrackGame->field_0x18 == 1 && others == 0) {
            return;
        }
        int old = field_0x14c;
        do {
            if (forward) {
                field_0x14c++;
            } else {
                field_0x14c--;
            }
            if (field_0x14c < 0) {
                field_0x14c = g_TrackGame->field_0x18 + others - 1;
            }
            if (field_0x14c >= g_TrackGame->field_0x18 + others) {
                field_0x14c = 0;
            }
            if (old == field_0x14c) {
                UnknownFunctionCameraView()->field_0x390 = 1;
                return;
            }
        } while (UnknownFunctionRacerAt(field_0x14c)->field_0x4a0 != 0);
    } else if (objects) {
        caster = 0;
        Scene* scene = raceScene;
        if (scene->field_0xb4 == 0 || scene->field_0xb4->field_0x00 <= 0) {
            UnknownFunctionCameraView()->field_0x390 = 1;
            return;
        }
        int hidden = 1;
        int old = field_0x150;
        do {
            if (forward) {
                field_0x150++;
            } else {
                field_0x150--;
            }
            if (field_0x150 < 0) {
                field_0x150 = scene->field_0xb4->field_0x00 - 1;
            }
            if (field_0x150 >= scene->field_0xb4->field_0x00) {
                field_0x150 = 0;
            }
            UnknownSceneEntry* entry = &scene->field_0xb4->field_0x04[field_0x150];
            if (entry->field_0x00_bit3) {
                if (((UnknownBikeRaceSceneCharacter*)entry->field_0x04)->field_0x25_bit0) {
                    hidden = entry->field_0x00_bit4;
                } else {
                    hidden = 1;
                }
            } else if (((UnknownBikeRaceObjectFlags*)entry->field_0x08)->field_0x25_bit0) {
                hidden = entry->field_0x00_bit4;
            } else {
                hidden = 1;
            }
            if (old == field_0x150) {
                UnknownBikeRaceCameraView* view = UnknownFunctionCameraView();
                if (view->field_0x384 != 0 || view->field_0x388 != 0) {
                    return;
                }
                entry = &scene->field_0xb4->field_0x04[field_0x150];
                if (entry->field_0x00_bit3) {
                    if (!((UnknownBikeRaceSceneCharacter*)entry->field_0x04)->field_0x25_bit0) {
                        view->field_0x390 = 1;
                        return;
                    }
                } else if (!((UnknownBikeRaceObjectFlags*)entry->field_0x08)->field_0x25_bit0) {
                    view->field_0x390 = 1;
                    return;
                }
                break;
            }
        } while (hidden);
    } else {
        caster = 1;
        UnknownSceneBuffer* casters = raceScene->field_0xb8;
        if (casters == 0 || casters->field_0x00 <= 0) {
            UnknownFunctionCameraView()->field_0x390 = 1;
            return;
        }
        int old = field_0x154;
        field_0x154 = forward ? old + 1 : old - 1;
        if (field_0x154 < 0) {
            field_0x154 = raceScene->field_0xb8->field_0x00 - 1;
        }
        if (field_0x154 >= raceScene->field_0xb8->field_0x00) {
            field_0x154 = 0;
        }
        if (old == field_0x154 && UnknownFunctionCameraView()->field_0x38c != 0) {
            return;
        }
    }
    UnknownFunction41eb20(caster);
}

// 0x0041f5e0
int BikeRace::UnknownVirtualSlot23(UnknownControlEvent* event, UnknownInputEntry* entry) {
    int result;
    if (g_TrackGame->network != 0 && event->kind == 0) {
        g_TrackGame->controlInterface->keyboard->UnknownFunction48a240(0xc);
    }
    if (field_0x190 && field_0x19c != 0) {
        if (event->control == 1 && event->kind == 0) {
            field_0x194 = 0.1f;
            field_0x19c->field_0x3dc = 0;
            field_0x190 = false;
            return 1;
        }
        if (field_0x19c->UnknownFunction51dce0(event, entry, &result)) {
            return 1;
        }
    }
    if (GameObject::UnknownVirtualSlot23(event, entry)) {
        return 1;
    }
    if (!field_0x190 && g_TrackGame->uiInteractionBlocked) {
        return 0;
    }
    if (event->kind == 0) {
        switch (event->control) {
        case 0x35:
            if (g_TrackGame->field_0x18 > 1) {
                KeyboardDevice* keyboard = g_TrackGame->controlInterface->keyboard;
                if (!keyboard->UnknownVirtualSlot5(0x2a, 0x3f, 0) &&
                    !g_TrackGame->controlInterface->keyboard->UnknownVirtualSlot5(0x36, 0x3f, 0)) {
                    if (g_TrackGame->mode.field_0x6b4 == 2) {
                        break;
                    }
                    if (field_0x19c != 0) {
                        field_0x19c->UnknownFunction51dd10();
                        field_0x19c->field_0x3dc = 1;
                    }
                    field_0x194 = 10.0f;
                    field_0x190 = true;
                    return 1;
                }
                g_TrackGame->mode.field_0x6b4++;
                if (g_TrackGame->mode.field_0x6b4 == 3) {
                    g_TrackGame->mode.field_0x6b4 = 0;
                }
                if (g_TrackGame->mode.field_0x6b4 == 1 && field_0x19c != 0) {
                    field_0x19c->UnknownFunction51dd10();
                    field_0x19c->field_0x3dc = 1;
                    field_0x190 = true;
                }
                char title[0x80];
                char value[0x80];
                char text[0x100];
                g_TrackGame->LoadResourceString(0x1428, title, 0x80);
                switch (g_TrackGame->mode.field_0x6b4) {
                case 0:
                    g_TrackGame->LoadResourceString(0x140b, value, 0x80);
                    break;
                case 1:
                    g_TrackGame->LoadResourceString(0x140a, value, 0x80);
                    break;
                case 2:
                    g_TrackGame->LoadResourceString(0x140c, value, 0x80);
                    break;
                }
                sprintf(text, "%s : %s", title, value);
                UnknownMessage* message = new (__FILE__, 0xcdd) UnknownMessage(text, 3.25f);
                TextQueueOverlay* overlay;
                switch (g_TrackGame->mode.field_0x27f8.field_0x04) {
                case 0:
                    overlay = g_TrackGame->field_0x55c->field_0x6c;
                    break;
                case 1:
                case 5:
                    overlay = g_TrackGame->field_0x560->field_0x6c;
                    break;
                case 2:
                    overlay = g_TrackGame->field_0x564->field_0x6c;
                    break;
                case 4:
                    overlay = g_TrackGame->field_0x568->field_0x6c;
                    break;
                }
                if (overlay != 0 && message != 0) {
                    overlay->UnknownFunction51b540(message);
                }
                delete message;
            }
            break;
        case 0x1b:
            if ((g_TrackGame->field_0x2d4_bit2) &&
                UnknownFunction43caa0(0x1b, 0, event, 3)) {
                UnknownFunction41f1d0(1, 0, 0);
                return 1;
            }
            if (UnknownFunction43caa0(0x1b, 0, event, 0x80)) {
                return 1;
            }
            UnknownFunction41f1d0(1, 0, 1);
            return 1;
        case 0x1a:
            if ((g_TrackGame->field_0x2d4_bit2) &&
                UnknownFunction43caa0(0x1a, 0, event, 3)) {
                UnknownFunction41f1d0(0, 0, 0);
                return 1;
            }
            if (UnknownFunction43caa0(0x1a, 0, event, 0x80)) {
                return 1;
            }
            UnknownFunction41f1d0(0, 0, 1);
            return 1;
        case 0xc9:
            if (UnknownFunction43caa0(0xc9, 0, event, 0x80000000)) {
                UnknownFunction41f1d0(1, 1, 0);
                return 1;
            }
            break;
        case 0xd1:
            if (UnknownFunction43caa0(0xd1, 0, event, 0x80000000)) {
                UnknownFunction41f1d0(0, 1, 0);
                return 1;
            }
            break;
        case 0x22:
            if (UnknownFunction43caa0(0x22, 0, event, 0xc)) {
                g_TrackGame->mode.field_0xa8c = 1 - g_TrackGame->mode.field_0xa8c;
                localRacer->field_0x5bc = g_TrackGame->mode.field_0xa8c;
                TextQueueOverlay* overlay = g_TrackGame->eventManager->FindTextQueue();
                if (overlay == 0) {
                    return 1;
                }
                char title[0x80];
                char value[0x80];
                char text[0x100];
                g_TrackGame->LoadResourceString(0x140d, title, 0x80);
                g_TrackGame->LoadResourceString(localRacer->field_0x5bc ? 0x1407 : 0x1408,
                                                             value, 0x80);
                sprintf(text, "%s %s", title, value);
                UnknownMessage message(text, 1.5f);
                overlay->UnknownFunction51b540(&message);
                return 1;
            }
            break;
        case 0x30:
            if (UnknownFunction43caa0(0x30, 0, event, 0xc)) {
                g_TrackGame->mode.field_0xa90 = 1 - g_TrackGame->mode.field_0xa90;
                localRacer->field_0x5c0 = g_TrackGame->mode.field_0xa90;
                TextQueueOverlay* overlay = g_TrackGame->eventManager->FindTextQueue();
                if (overlay == 0) {
                    return 1;
                }
                char title[0x80];
                char value[0x80];
                char text[0x100];
                g_TrackGame->LoadResourceString(0x142d, title, 0x80);
                g_TrackGame->LoadResourceString(localRacer->field_0x5c0 ? 0x1407 : 0x1408,
                                                             value, 0x80);
                sprintf(text, "%s %s", title, value);
                UnknownMessage message(text, 1.5f);
                overlay->UnknownFunction51b540(&message);
                return 1;
            }
            break;
        case 0x1f:
            if (UnknownFunction43caa0(0x1f, 0, event, 0x80)) {
                field_0x034 = 1 - field_0x034;
            }
            break;
        }
    }
    if (g_TrackGame->field_0x2d4_bit2 && g_TrackGame->debugOverlay != 0) {
        if (field_0x0a8 < 0) {
            field_0x0a8 = g_TrackGame->debugOverlay->NewPage();
        }
        if (g_TrackGame->debugOverlay->field_0x26c4 == field_0x0a8) {
            if (UnknownFunction43caa0(0x1c, 0, event, 0x80)) {
                UnknownKrustyUIGui* gui = g_TrackGame->ui->field_0x2c;
                if (gui->GetUser(0)->field_0x30 == 0) {
                    g_TrackGame->ui->field_0x2c->UnknownFunction486590("ui\\cursor.tga", 0);
                }
                if (((UnknownBikeRaceObjectFlags*)g_TrackGame->ui->field_0x2c
                         ->GetUser(0)->field_0x30)->field_0x25_bit0) {
                    g_TrackGame->debugOverlay->UnknownFunction448000(
                        field_0x0a8, g_UnknownGlobal567a88 + 1, 0);
                    g_UnknownGlobal567a88++;
                    if ((signed char)field_0x0b0 < 0) {
                        if (g_UnknownGlobal567a88 > 12) {
                            g_UnknownGlobal567a88 = 2;
                        } else if (g_UnknownGlobal567a88 > 5 && g_UnknownGlobal567a88 < 9) {
                            g_UnknownGlobal567a88 = 9;
                        }
                    } else if (g_UnknownGlobal567a88 > 5) {
                        g_UnknownGlobal567a88 = 2;
                    }
                } else if (objectPicker == 0) {
                    objectPicker = new (__FILE__, 0xed2) ObjectPicker(1);
                    objectPicker->UnknownFunction4b0210(
                        g_TrackGame->renderTarget, 0, UnknownFunction417b00,
                        (GameCursor*)g_TrackGame->ui->field_0x2c->GetUser(0)->field_0x30);
                    if (AppendChild(objectPicker, -1)) {
                        g_TrackGame->ui->UnknownFunction499b00();
                    }
                } else {
                    g_TrackGame->ui->UnknownFunction499b00();
                }
            }
            if (UnknownFunction43caa0(0, 1, event, 0x80000000)) {
                int picked = objectPicker->UnknownFunction4b04c0();
                if (picked != 0) {
                    Scene* scene = raceScene;
                    if (scene->field_0xb8 != 0) {
                        int count = scene->field_0xb8->field_0x00;
                        UnknownBikeRacePickCaster* casters =
                            (UnknownBikeRacePickCaster*)scene->field_0xb8->field_0x04;
                        for (int i = 0; i < count; i++) {
                            if (casters[i].field_0x00 & 1) {
                                if (picked == casters[i].field_0x08->field_0x128) {
                                    UnknownFunctionCameraView()->field_0x390 = 1;
                                    field_0x154 = i;
                                    UnknownFunction41f1d0(1, 0, 0);
                                    return 1;
                                }
                            } else if (picked == casters[i].field_0x0c) {
                                UnknownFunctionCameraView()->field_0x390 = 1;
                                field_0x154 = i;
                                UnknownFunction41f1d0(1, 0, 0);
                                return 1;
                            }
                        }
                    }
                    if (scene->field_0xb4 != 0) {
                        int count = scene->field_0xb4->field_0x00;
                        UnknownSceneEntry* entries = scene->field_0xb4->field_0x04;
                        for (int i = 0; i < count; i++) {
                            if (entries[i].field_0x00_bit3) {
                                if (picked == ((UnknownBikeRacePickCharacter*)entries[i].field_0x04)->field_0x210) {
                                    UnknownFunctionCameraView()->field_0x390 = 1;
                                    field_0x150 = i;
                                    UnknownFunction41f1d0(1, 0, 1);
                                    return 1;
                                }
                            } else if (picked == ((UnknownBikeRacePickObject*)entries[i].field_0x08)->field_0x3c) {
                                UnknownFunctionCameraView()->field_0x390 = 1;
                                field_0x150 = i;
                                UnknownFunction41f1d0(1, 0, 1);
                                return 1;
                            }
                        }
                    }
                }
            }
        }
    }
    return 0;
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

static inline Vector3& operator*=(Vector3& v, float s) {
    v.x *= s;
    v.y *= s;
    v.z *= s;
    return v;
}

static inline float SquareMagnitude(const Vector3& v) {
    return DotProduct(v, v);
}

static inline Vector3 Scale(const Vector3& v, float s) {
    return Vector3(s * v.x, s * v.y, s * v.z);
}

float UnknownFunction40ae30(const Vector3* a, const Vector3* b); // 0x0040ae30: a.b

static inline Vector3 Normalize(const Vector3& v) {
    float length = SquareMagnitude(v);
    if (length == 1.0f) {
        return v;
    }
    float scale = FastInvSqrt(length);
    return Scale(v, scale);
}

// 0x004210f0
void BikeRace::UnknownFunction4210f0(Vector3* a, Vector3* b, void* reference, int count) {
    if (field_0x188 && count != 0) {
        *a = Scale(gridSlotStep, (float)count) + gridFirstSlot;
        *b = gridDirection;
        return;
    }
    switch (g_TrackGame->mode.field_0x27f8.field_0x04) {
    case 2:
    case 3: {
        if (field_0x144) {
            a->x = startProbe.field_0x00.x;
            a->z = startProbe.field_0x00.z;
            a->y = 0.0f;
            b->x = startProbe.field_0x0c.x;
            b->z = startProbe.field_0x0c.z;
            b->y = 0.0f;
        } else {
            *a = raceScene->field_0x70;
            *b = Normalize(raceScene->field_0x7c);
        }
        if (count <= 0) {
            return;
        }
        float side;
        if (raceTrack->field_0x00 != 0 && raceTrack->field_0x00->field_0x08 != 0 &&
            raceTrack->field_0x00->field_0x08->field_0x2c != 0 &&
            raceTrack->field_0x00->field_0x08->field_0x2c->field_0x2c != 0) {
            TrackVec3 p0;
            TrackVec3 p1;
            raceTrack->UnknownFunction518130(raceTrack->field_0x00->field_0x08, &p0);
            raceTrack->UnknownFunction518130(raceTrack->field_0x00->field_0x08->field_0x2c, &p1);
            if (p0.z * p1.x - p1.z * p0.x <= 0.0f) {
                side = 1.0f;
            } else {
                side = -1.0f;
            }
        } else {
            side = -1.0f;
        }
        gridDirection = *b;
        *a -= Scale(*b, 4.66f);
        gridSlotStep = Vector3(b->z, 0.0f, -b->x);
        gridFirstSlot = *a - Scale(Scale(Scale(gridSlotStep, 6.75f), field_0x04c->field_0x40), side);
        gridSlotStep *= field_0x04c->field_0x40 * 15.5f / (racerCount + 1) * side;
        *a = Scale(gridSlotStep, (float)count) + gridFirstSlot;
        break;
    }
    case 0:
        *a = raceScene->field_0x70;
        *b = Normalize(raceScene->field_0x7c);
        if (count <= 0) {
            return;
        }
        gridDirection = *b;
        *a -= Scale(*b, 4.66f);
        gridSlotStep = Vector3(b->z, 0.0f, -b->x);
        gridFirstSlot = *a - Scale(gridSlotStep, 20.0f);
        gridSlotStep = Scale(Vector3(b->z, 0.0f, -b->x), 20.0f);
        *a = Scale(gridSlotStep, (float)count) + gridFirstSlot;
        break;
    case 1:
    case 5: {
        UnknownBikeRaceNodeOwner* owner = (UnknownBikeRaceNodeOwner*)reference;
        Vector3 direction = owner->field_0x0d8[owner->field_0x0ac - 1].field_0x0c;
        direction.y = 0.0f;
        *b = Normalize(direction);
        *a = owner->field_0x0d8[owner->field_0x0ac - 1].field_0x00;
        *a += Scale(*b, 8.0f);
        a->y = 0.0f;
        if (count <= 0) {
            return;
        }
        float width = owner->field_0x420[owner->field_0x0ac - 1]->field_0x50 * 1.05f;
        float spacing = width / (racerCount + 1);
        if (3.4f > spacing) {
            spacing = 3.4f;
        }
        gridDirection = *b;
        gridSlotStep = Vector3(b->z, 0.0f, -b->x);
        gridFirstSlot = *a - Scale(gridSlotStep, width * 0.48f);
        gridSlotStep = Scale(Vector3(b->z, 0.0f, -b->x), spacing);
        *a = Scale(gridSlotStep, (float)count) + gridFirstSlot;
        break;
    }
    case 4:
        *a = raceScene->field_0x70;
        *b = Normalize(raceScene->field_0x7c);
        if (count <= 0) {
            return;
        }
        gridDirection = *b;
        *a -= Scale(*b, 4.66f);
        gridSlotStep = Vector3(b->z, 0.0f, -b->x);
        gridFirstSlot = *a - Scale(gridSlotStep, 20.0f);
        gridSlotStep = Scale(Vector3(b->z, 0.0f, -b->x), 20.0f);
        *a = gridFirstSlot + Scale(gridSlotStep, (float)count);
        break;
    default:
        return;
    }
    field_0x188 = true;
}

// DlgProcs.h's DemoDlg (vtable 0x00550e18, 0x7f58 bytes) with the inline
// constructor the setup's `new` expands (UIDialog(1, "credits.dtm"), then
// the vtable); DlgProcs.h declares no constructor for it.
class UnknownBikeRaceDemoDlg : public UIDialog {
public:
    UnknownBikeRaceDemoDlg(int flags, const char* resource) : UIDialog(flags, resource) {}
    virtual int UnknownVirtualSlot10(float frameTime);    // 0x00455b20
    virtual int UnknownVirtualSlot23(UnknownControlEvent* event, UnknownInputEntry* entry); // 0x00455ae0

    unsigned char field_0x2c[0x7f58 - 0x2c];
};

// The length of `v` (1 without a square root for unit vectors); the setup
// restores the lengths D3DRMVectorRotate's unit results lose.
static inline float UnknownVectorLength(const Vector3& v) {
    float lengthSquared = v.x * v.x + v.y * v.y + v.z * v.z;
    if (lengthSquared == 1.0f)
        return 1.0f;
    return (float)sqrt(lengthSquared);
}

static inline Vector3 UnknownScaled(const Vector3& v, float scale) {
    return Vector3(v.x * scale, v.y * scale, v.z * scale);
}

// 0x00417ed0
BikeRace* BikeRace::UnknownFunction417ed0(void* owner, void* particles, LightManager* lights,
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

    GameObject::UnknownVirtualSlot8(owner);
    field_0x25_bit1 = 1;
    field_0x0c4 = 0;
    field_0x07c[0] = 50.0f;
    field_0x07c[1] = 10.0f;
    field_0x07c[2] = 40.0f;
    field_0x07c[3] = 40.0f;
    field_0x07c[4] = 60.0f;
    field_0x07c[5] = 0.0f;
    field_0x07c[6] = 100.0f;
    field_0x07c[7] = 10.0f;
    field_0x07c[8] = 10.0f;
    field_0x07c[9] = 50.0f;
    field_0x07c[10] = 100.0f;
    field_0x0b8 = 0;
    field_0x054 = (int)particles;
    raceScene = scene;
    field_0x05c = shadow;
    if (!g_TrackGame->field_0x1cc) {
        Release();
        return 0;
    }
    field_0x04c = terrain;
    raceCamera = camera;
    field_0x3f8 = false;
    field_0x3f9 = false;
    replayTime = 0;
    replayLength = 0;
    field_0x1d4 = 0;
    field_0x1d8 = 0;
    g_TrackGame->ui->UnknownFunction49b530();

    int mode = g_TrackGame->mode.field_0x27f8.field_0x04;
    switch (mode) {
    case 2:
    case 3: {
        COPY_TEXT(path, raceScene->field_0xa4->terrainFile, 0x104);
        strcpy(strrchr(path, '.'), ".tdf");
        UnknownTextureStream* stream =
            new(__FILE__, 0x193) UnknownTextureStream((int)g_UnknownResourceManager572b44);
        if (!g_TrackGame->sceneObject->UnknownFunction4e9cd0(stream, path, "rb", (int)modelName)) {
            sprintf(message, "No Track file found in env file.  Abort.\n");
            delete stream;
            Release();
            return 0;
        }
        if (stream) {
            raceTrack = new(__FILE__, 0x1a5) Track;
            trackLoaded = raceTrack->UnknownFunction515ed0(stream, &startProbe, &finishProbe, &field_0x144,
                                                             &field_0x148);
            delete stream;
        }
        if (field_0x148)
            field_0x04c->UnknownFunction507c10(&finishProbe.field_0x00, 0, 0, 0);
        if (field_0x144)
            field_0x04c->UnknownFunction507c10(&startProbe.field_0x00, 0, 0, 0);
        if (field_0x148 && field_0x144)
            UnknownFunction41d1e0();
        break;
    }
    case 1:
    case 5:
        if (mode != 1 || !g_TrackGame->mode.field_0x27f8.field_0x2c) {
            sprintf(path, "%s\\%s%02d.tdf", g_TrackGame->sceneObject->field_0x44,
                    g_TrackGame->mode.field_0x27f8.field_0x36,
                    g_TrackGame->mode.field_0x27f8.field_0x34);
            UnknownTextureStream* stream =
                new(__FILE__, 0x1be) UnknownTextureStream((int)g_UnknownResourceManager572b44);
            if (!g_TrackGame->sceneObject->UnknownFunction4e9cd0(stream, path, "rb", (int)modelName)) {
                delete stream;
            } else if (stream) {
                {
                    raceTrack = new(__FILE__, 0x1d0) Track;
                    trackLoaded = raceTrack->UnknownFunction515ed0(stream, &startProbe, &finishProbe,
                                                                     &field_0x144, &field_0x148);
                    if (!trackLoaded) {
                        Track* track = raceTrack;
                        if (track != 0) {
                            if (track->field_0x00 != 0)
                                track->UnknownFunction516870(&track->field_0x00);
                            delete track;
                        }
                    }
                    delete stream;
                }
            }
        }
        trackLoaded = UnknownFunction41d170(event);
        break;
    }
    g_TrackGame->ui->UnknownFunction49b530();

    UnknownFunction4210f0(&position, &direction, event, 0);
    mode = g_TrackGame->mode.field_0x27f8.field_0x04;
    if (mode == 3 || mode == 2) {
        if (raceTrack) {
            if (!g_TrackGame->field_0x3428) {
                if (field_0x144) {
                    direction.x = -startProbe.field_0x0c.x;
                    direction.z = -startProbe.field_0x0c.z;
                    point.x = startProbe.field_0x00.x - direction.x * 15.0f;
                    point.z = startProbe.field_0x00.z - direction.z * 15.0f;
                } else {
                    point = position;
                }
                terrain->UnknownFunction507c10(&point, &normal, 0, 0);
                field_0x06c = (UnknownBikeRaceView6c*)raceScene->UnknownFunction4eaec0("30SecMan.slt", &field_0x070);
                if (field_0x06c) {
                    raceScene->UnknownFunction4eb040(field_0x070, 0, 0, 0);
                    raceScene->field_0xb4->field_0x04[field_0x070].field_0x00_bit5 = 1;
                    if (field_0x06c->field_0x210) {
                        field_0x06c->field_0x210->Release();
                        field_0x06c->field_0x210 = 0;
                    }
                }
                if (g_TrackGame->mode.field_0x27f8.field_0x00 != 0 &&
                    g_TrackGame->mode.field_0x27f8.field_0x00 != 4 && field_0x06c) {
                    field_0x06c->field_0x1a0->UnknownFunction4fc630(point);
                    field_0x06c->field_0x1a0->UnknownFunction4fbd70(&direction, &kVec3YAxis, 1, 0);
                }
                if (field_0x06c)
                    field_0x06c->UnknownVirtualSlot4();
                if (field_0x144) {
                    point.x = startProbe.field_0x00.x;
                    point.z = startProbe.field_0x00.z;
                    direction.x = startProbe.field_0x0c.x;
                    direction.z = startProbe.field_0x0c.z;
                } else {
                    point = position;
                }
                terrain->UnknownFunction507c10(&point, &normal, 0, 0);
                if (g_TrackGame->mode.field_0x27f8.field_0x04 == 3) {
                    field_0x064 =
                        (UnknownBikeRaceView6c*)raceScene->UnknownFunction4eaec0("StartGate.slb", &field_0x068);
                    if (field_0x064) {
                        raceScene->UnknownFunction4eb040(field_0x068, 0, 0, 0);
                        if (field_0x064->field_0x210) {
                            field_0x064->field_0x210->Release();
                            field_0x064->field_0x210 = 0;
                        }
                    }
                    if (g_TrackGame->mode.field_0x27f8.field_0x00 != 0 &&
                        g_TrackGame->mode.field_0x27f8.field_0x00 != 4) {
                        if (field_0x064) {
                            field_0x064->field_0x1a0->UnknownFunction4fc630(point);
                            field_0x064->field_0x1a0->UnknownFunction4fbd70(&direction, &kVec3YAxis, 1, 0);
                        }
                    } else if (field_0x064) {
                        field_0x064->UnknownVirtualSlot4();
                    }
                }
            }
            if (field_0x148) {
                point.x = finishProbe.field_0x00.x;
                point.z = finishProbe.field_0x00.z;
                terrain->UnknownFunction507c10(&point, 0, 0, 0);
                direction.x = finishProbe.field_0x0c.x;
                direction.y = 0.0f;
                direction.z = finishProbe.field_0x0c.z;
            }

            sprintf(name, "%s\\%s", "Res", "MetalArch.slt");
            ArcadeObject* arch = new(__FILE__, 0x247) ArcadeObject(1);
            arch = arch->UnknownFunction401310(field_0x18, (int)lights, (int)textures, name, kVec3Zero, 0, 0,
                                               2.0f, 0.5f, 0.5f, 0);
            AppendChild(arch, -1);
            arch->arcadeModel->UnknownFunction4444c0(1);
            float scale = finishProbe.field_0x24 / arch->modelWidth * 1.111f;
            arch->arcadeModel->UnknownFunction4fd340(scale, scale, scale);
            arch->arcadeModel->SoultreeVirtualSlot6();
            arch->UnknownFunction4014f0(&point);
            arch->UnknownFunction401520(&direction, &kVec3YAxis, 1, 0);
            arch->UnknownFunction401540(name, 0x6a, 0, 0, 0, 1);

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
            terrain->UnknownFunction507c10(&ground, 0, 0, 0);
            float height = ground.y - point.y;
            girlPosition = point + girlOffset;
            D3DRMVectorRotate(&turned, &boxOffset, (Vector3*)&kVec3YAxis, angle);
            boxOffset = UnknownScaled(turned, UnknownVectorLength(boxOffset));
            boxPosition = point + boxOffset;
            girlPosition.y += height;
            boxPosition.y += height;

            if (g_TrackGame->mode.field_0x27f8.field_0x00 != 0 &&
                g_TrackGame->mode.field_0x27f8.field_0x00 != 4) {
                sprintf(modelName, "%s\\%s", "Res", "FlagGirl.mcf");
                flagGirl = (UnknownBikeRaceCharacter*)new(__FILE__, 0x26f) D3DIMSoultreeCharacter(1);
                AppendChild(flagGirl->UnknownVirtualSlot11(field_0x18, modelName, lights, textures, 1, 1),
                                      -1);
                UnknownFunction41ea60(flagGirl, 1, 2);
                flagGirl->field_0x1a0->UnknownFunction4444c0(1);
                flagGirl->field_0x1a0->UnknownFunction4fc630(girlPosition);
                flagGirl->field_0x1a0->UnknownFunction4fbd70(&direction, &kVec3YAxis, 1, 0);
                flagGirl->UnknownFunction4a8b10("Stand");
                flagGirl->UnknownVirtualSlot7(0.001f, 0, 0);
            }

            sprintf(name, "%s\\%s", "Res", "GirlBox.slt");
            ArcadeObject* box = new(__FILE__, 0x27c) ArcadeObject(1);
            box = box->UnknownFunction401310(field_0x18, (int)lights, (int)textures, name, kVec3Zero, 0, 0, 2.0f,
                                             0.5f, 0.5f, 0);
            AppendChild(box, -1);
            box->arcadeModel->UnknownFunction4444c0(1);
            box->UnknownFunction4014f0(&boxPosition);
            box->UnknownFunction401520(&direction, &kVec3YAxis, 1, 0);
            box->UnknownFunction401540(name, 0x6a, 0, 0, 0, 1);
            g_TrackGame->eventManager->RemoveVegetationInRect(girlPosition.x - 4.0f, girlPosition.z - 4.0f,
                                                                       girlPosition.x + 4.0f, girlPosition.z + 4.0f);
        }
    } else if (mode == 0 || (mode == 4 && g_TrackGame->mode.field_0x27f8.field_0x148)) {
        BonusObjectManager* bonus = new(__FILE__, 0x28e) BonusObjectManager(1);
        field_0x0bc = (int)bonus->UnknownFunction48ca60(field_0x18, (int)lights, (int)textures,
                                                        (UnknownBonusRacer*)this, (UnknownBonusCamera*)raceCamera);
        if (!AppendChild((GameObject*)field_0x0bc, -1))
            return 0;
    }
    g_TrackGame->ui->UnknownFunction49b530();

    UnknownFunction41d2a0(0.0f);
    raceScene->field_0x7c0 = 1;
    if (g_TrackGame->mode.field_0x27f8.field_0x00) {
        NumberObjectManager* numbers = new(__FILE__, 0x29b) NumberObjectManager(1);
        field_0x0c0 = (int)numbers->UnknownFunction48bd00(field_0x18, (int)textures, (UnknownNumberRacer*)this, 0x1e,
                                                          (UnknownNumberCamera*)raceCamera, 3.0f);
        if (!AppendChild((GameObject*)field_0x0c0, -1))
            return 0;
    }
    if (g_TrackGame->mode.field_0x27f8.field_0x04 != 4 &&
        g_TrackGame->mode.field_0x27f8.field_0x00 != 4 &&
        (g_TrackGame->field_0x18 > 1 || g_TrackGame->mode.field_0x27f8.field_0x24 > 0)) {
        sprintf(name, "%s\\%s", "Res", "youind.slt");
        ArcadeObject* marker = new(__FILE__, 0x2a9) ArcadeObject(*(int*)g_TrackGame->mode.field_0x6d0);
        field_0x060 = marker->UnknownFunction401310(field_0x18, (int)lights, (int)textures, name, position, 0, 0,
                                                    2.0f, 0.5f, 0.5f, 0);
        AppendChild(field_0x060, -1);
        Vector3 axis(0.0f, 1.0f, 0.0f);
        ((ArcadeObject*)field_0x060)->UnknownFunction4017a0(axis, 75.0f);
        ((ArcadeObject*)field_0x060)->arcadeModel->UnknownFunction4fd340(0.6f, 0.6f, 0.6f);
    }
    if (g_TrackGame->mode.field_0x27f8.field_0x00 == 4) {
        field_0x1a8 = new(__FILE__, 0x2b6) UnknownVcrFile(1, 2);
        if (g_TrackGame->mode.field_0x25ec[0]) {
            field_0x1a8->Load(g_TrackGame->mode.field_0x25ec, "VCRgtemp.dat");
            ghostVcr = new(__FILE__, 0x2b9) KrustyVCR;
        }
        replayVcr = new(__FILE__, 0x2bd) KrustyVCR;
        field_0x1e8 = 0;
    } else if (g_TrackGame->field_0x342c) {
        field_0x1a8 = new(__FILE__, 0x2c2) UnknownVcrFile(0, 1);
        replayVcr = new(__FILE__, 0x2c3) KrustyVCR;
        if (g_TrackGame->field_0x3428) {
            if (g_TrackGame->ui->field_0x4a8) {
                replayVcr->UnknownFunction49bf10(UnknownFunction4230e0, 1, "ui\\demo.rpl", field_0x1a8);
                AppendChild(replayVcr, -1);
            } else {
                replayVcr->UnknownFunction49bf10(UnknownFunction4230e0, 1, g_TrackGame->mode.field_0x26f4,
                                                   field_0x1a8);
                AppendChild(replayVcr, -1);
            }
            field_0x1ec = UnknownFunctionCameraView()->field_0x244;
            raceCamera->UnknownVirtualSlot71(1);
        }
    }
    g_TrackGame->ui->UnknownFunction49b530();

    if (!UnknownFunction419970(owner, lights, textures, terrain, event))
        return 0;
    g_TrackGame->ui->UnknownFunction49b530();

    if (field_0x06c) {
        if (field_0x144) {
            localRacer->field_0x3bc->UnknownFunction4fc970(&point);
            point.x += localRacer->field_0x088.x * 15.0f;
            point.z += localRacer->field_0x088.z * 15.0f;
        }
        terrain->UnknownFunction507c10(&point, 0, 0, 0);
        field_0x06c->field_0x1a0->UnknownFunction4fc630(point);
    }
    if (g_TrackGame->field_0x3428 && g_TrackGame->mode.field_0x27f8.field_0x00 != 4 &&
        g_TrackGame->field_0x342c) {
        if (g_TrackGame->ui->field_0x4a8) {
            UnknownBikeRaceDemoDlg* dialog = new(__FILE__, 0x2fb) UnknownBikeRaceDemoDlg(1, "credits.dtm");
            g_TrackGame->ui->field_0x2c->ShowDialog(dialog, 0, 1, 0, 0, 0, 0, 1);
        } else {
            g_TrackGame->ui->field_0x2c->ShowCursors(1);
            VCRDlg* dialog = new(__FILE__, 0x2ff) VCRDlg(1, "VCR.dtm");
            g_TrackGame->ui->field_0x2c->ShowDialog(dialog, 0, 1, 0, 0, 0, 0, 1);
        }
    }
    if (replayVcr && !g_TrackGame->field_0x3428) {
        g_TrackGame->ui->UnknownFunction49b530();
        if (g_TrackGame->mode.field_0x27f8.field_0x00 == 4) {
            COPY_TEXT(ghostFileRecorded, "VCRghost.dat", 0x104);
            replayVcr->UnknownFunction49bf10(UnknownFunction4230e0, 0, ghostFileRecorded, field_0x1a8);
            if (ghostVcr) {
                COPY_TEXT(ghostFilePlayed, "VCRgtemp.dat", 0x104);
                ghostVcr->UnknownFunction49bf10(UnknownFunction4230e0, 1, ghostFilePlayed, field_0x1a8);
                field_0x1c8 = ghostVcr->UnknownFunction49c000();
                aiRacers[0]->UnknownFunction496e20(ghostVcr);
                if (field_0x189)
                    aiRacers[0]->UnknownVirtualSlot4();
                else
                    aiRacers[0]->UnknownVirtualSlot5();
                AppendChild(ghostVcr, -1);
            }
        } else {
            replayVcr->UnknownFunction49bf10(UnknownFunction4230e0, 0, "VCRtape.dat", field_0x1a8);
        }
        AppendChild(replayVcr, -1);
        isRecording = 1;
    }
    if (g_TrackGame->field_0x18 == 1 && g_TrackGame->mode.field_0x27f8.field_0x00 != 4) {
        int count = g_TrackGame->mode.field_0x27f8.field_0x24;
        localRacer->UnknownRacerVirtualSlot50(1, localRacer->UnknownRacerVirtualSlot45(), 0);
        for (int i = 0; i < count; i++)
            aiRacers[i]->UnknownRacerVirtualSlot50(1, aiRacers[i]->UnknownRacerVirtualSlot45(), 0);
    }
    g_TrackGame->ui->UnknownFunction49b530();

    g_MemTagStack->Push("Audio");
    RaceSound* sound = new(__FILE__, 0x347) RaceSound(1);
    field_0x044 = (UnknownBikeRaceViews*)sound->Create(field_0x18, (UnknownKrustyBikeView*)this,
                                                                      (UnknownRaceSoundCamera*)raceCamera, 6);
    AppendChild((GameObject*)field_0x044, -1);
    g_MemTagStack->Push("BikeRace");
    if (g_TrackGame->mode.field_0xa88) {
        g_TrackGame->controlInterface->activeJoystick->SetAcquired(0);
        g_TrackGame->controlInterface->activeJoystick->UnknownVirtualSlot5(g_TrackGame->mode.field_0xa88);
        localRacer->UnknownRacerVirtualSlot93();
        g_TrackGame->controlInterface->activeJoystick->SetAcquired(1);
    }
    if (g_TrackGame->mode.field_0x27f8.field_0x04)
        BuildStatusList(&field_0x0c4, (UnknownKrustyBikeView*)this);
    if (ghostVcr)
        localRacer->field_0x750 = field_0x1c8;
    g_TrackGame->ui->UnknownFunction49b530();

    UnknownBikeNumberPainter* painter = new(__FILE__, 0x367) UnknownBikeNumberPainter(textures->field_0x00);
    if (g_TrackGame->field_0x3428) {
        painter->UnknownFunction417670(localRacer->field_0x3bc, localRacer->field_0x73c);
        for (int i = 0; i < UnknownFunctionRacerCount() - 1; i++)
            painter->UnknownFunction417670(aiRacers[i]->field_0x3bc, aiRacers[i]->field_0x73c);
    } else if (g_TrackGame->field_0x18 > 1) {
        for (int i = 0; i < UnknownFunctionRacerCount(); i++) {
            for (int j = 0; j < UnknownFunctionRacerCount(); j++) {
                UnknownBikeRaceRacer* racer = racerSlots[i];
                UnknownTrackGameRacerSlot* slot = &g_TrackGame->mode.field_0x1be4[j];
                if (racer->field_0x11bc == slot->field_0xd4 && racer->field_0x11c0 == slot->field_0xd8)
                    painter->UnknownFunction417670(racer->field_0x3bc, slot->field_0xc0);
            }
        }
    } else {
        painter->UnknownFunction417670(localRacer->field_0x3bc, g_TrackGame->mode.field_0x1bcc);
        if (g_TrackGame->mode.field_0x27f8.field_0x00 == 4) {
            painter->UnknownFunction417670(aiRacers[0]->field_0x3bc, g_TrackGame->mode.field_0x1bcc);
        } else {
            for (int i = 0; i < g_TrackGame->mode.field_0x27f8.field_0x24; i++)
                painter->UnknownFunction417670(aiRacers[i]->field_0x3bc, aiRacers[i]->field_0x73c);
        }
    }
    delete painter;
    g_TrackGame->ui->UnknownFunction49b530();
    return this;
}

// A racer's skill: `base` moved up or down by at most `range`, closer to
// `base` more often.
static inline float UnknownRandomSkill(float base, float range) {
    float r = rand() * (1.0f / 32768.0f);
    float offset = (float)((1.0 - exp(-r * r)) * range);
    if (rand() * (1.0f / 32768.0f) > 0.5f)
        offset = -offset;
    return base + offset;
}

// 0x00419970
int BikeRace::UnknownFunction419970(void* owner, LightManager* lights, UnknownTextureFormatChoice* textures,
                                    UnknownBikeRaceProjector* terrain, UnknownBikeRaceNodeOwner* event) {
    char path[0x104];
    char message[0x184];
    char bikeFile[0x104];
    char riderFile[0x104];
    char bikeTexture[0x40];
    char riderTexture[0x40];
    char name[0x10];
    char empty[0x104];
    int order[11];
    Vector3 position;
    Vector3 direction;
    UnknownBikeRaceBikeSetup setup;
    int playerId = 0x7fffffff;
    int recordIndex = 0;
    int id = 0;
    float skill = field_0x07c[0];
    Vector3 up(0.0f, 1.0f, 0.0f);
    empty[0] = 0;

    UnknownTextureStream* stream = new(__FILE__, 0x3b6) UnknownTextureStream((int)g_UnknownResourceManager572b44);
    sprintf(path, "%s\\rider.mcf", "Res");
    if (!g_TrackGame->sceneObject->UnknownFunction4e9cd0(stream, path, "rb", (int)riderFile)) {
        sprintf(message, "No Rider MCF found in resources.  Aborting.\n");
        delete stream;
        Release();
        return 0;
    }

    if (g_TrackGame->field_0x3428) {
        if (g_TrackGame->mode.field_0x27f8.field_0x35) {
            racerCount = g_TrackGame->mode.field_0x27f8.field_0x35 +
                          g_TrackGame->mode.field_0x27f8.field_0x28;
            g_TrackGame->mode.field_0x27f8.field_0x24 = racerCount - 1;
            g_TrackGame->mode.field_0x27f8.field_0x35 = 0;
        } else {
            racerCount = g_TrackGame->mode.field_0x27f8.field_0x24 + g_TrackGame->field_0x18;
        }
    } else {
        if (g_TrackGame->mode.field_0x27f8.field_0x00 == 4)
            racerCount = 1;
        else
            racerCount = UnknownFunctionRacerCount();
        recordIndex = 0;
        if (replayVcr)
            replayVcr->field_0x2fc = racerCount;
    }
    if (g_TrackGame->field_0x3428 || g_TrackGame->field_0x18 == 1) {
        if (g_TrackGame->mode.field_0x27f8.field_0x00 == 2) {
            int ranking[11];
            UnknownFunction417bc0(ranking);
            for (int i = 0; i < racerCount; i++) {
                for (int j = 0; j < racerCount; j++) {
                    if (ranking[j] == i + 1) {
                        order[i] = j + 1;
                        break;
                    }
                }
            }
        } else {
            for (int i = 0; i < racerCount; i++)
                order[i] = i + 1;
        }
    }

    if (g_TrackGame->UnknownFunction521cd0()) {
        // The pro circuit: the player's racer, then the computer racers of
        // the career.
        UnknownTrackGameObject3444* circuit = g_TrackGame->field_0x3444;
        setup = *(UnknownBikeRaceBikeSetup*)g_TrackGame->mode.field_0xfd8;
        setup.field_0x00 = circuit->field_0x465[0].field_0x138;
        setup.field_0x04 = circuit->field_0x465[0].field_0x13c;
        int bikeClass = UnknownBikeClassOf(setup.field_0x00);
        setup.field_0x50 = g_TrackGame->ui->field_0x2fc[bikeClass];
        setup.field_0x54 = g_TrackGame->ui->field_0x310[bikeClass];
        setup.field_0x58 = (setup.field_0x54 - setup.field_0x50) / 10;
        setup.field_0x08 = g_TrackGame->ui->field_0x428[bikeClass];
        UnknownFunction4210f0(&position, &direction, event, order[0]);
        sprintf(path, "%s\\%s", "Res", circuit->field_0x465[0].field_0x38);
        if (!g_TrackGame->sceneObject->UnknownFunction4e9cd0(stream, path, "rb", (int)bikeFile)) {
            sprintf(message, "No PC Rider MCF found in resources.  Aborting.\n");
            delete stream;
            Release();
            return 0;
        }
        char riderPath[0x104];
        char circuitRiderFile[0x104];
        if (circuit->field_0x464 & 2) {
            sprintf(riderPath, "%s\\%s", "Res", "riderd.mcf");
        } else {
            COPY_TEXT(riderPath, riderFile, 0x104);
        }
        if (!g_TrackGame->sceneObject->UnknownFunction4e9cd0(stream, riderPath, "rb",
                                                                       (int)circuitRiderFile)) {
            sprintf(message, "No PC Rider MCF found in resources.  Aborting.\n");
            delete stream;
            Release();
            return 0;
        }
        localRacer = new(__FILE__, 0x42a) UnknownBikeRaceRacer(1);
        localRacer->UnknownFunction48fc80(owner, "", bikeFile, circuitRiderFile, lights, textures, position,
                                           direction, up, terrain, g_TrackGame->controlInterface, 0, 0, this,
                                           g_TrackGame->mode.field_0x00, 0, 0, &setup,
                                           g_TrackGame->mode.field_0xa8c,
                                           g_TrackGame->mode.field_0xa90, field_0x054, raceScene,
                                           replayVcr, 0, !(circuit->field_0x464 & 2));
        AppendChild(localRacer, -1);
        char ownBikeTexture[0x40];
        g_TrackGame->ui->UnknownFunction49b560(circuit->field_0x465[0].field_0x78, ownBikeTexture, 0x3f);
        localRacer->field_0x3bc->UnknownFunction444c70(0, ownBikeTexture, (int)textures);
        if (!(circuit->field_0x464 & 2)) {
            char ownRiderTexture[0x40];
            g_TrackGame->ui->UnknownFunction49b7f0(circuit->field_0x465[0].field_0xb8, ownRiderTexture,
                                                             0x3f);
            localRacer->field_0x5c4->field_0x1a0->UnknownFunction444c70(0, ownRiderTexture, (int)textures);
        }
        if (g_TrackGame->mode.field_0x27f8.field_0x00 == 1) {
            id = 1;
            aiRacers = (UnknownBikeRaceRacer**)DebugMalloc(
                (g_TrackGame->field_0x3444->field_0x460 - 1) * sizeof(UnknownBikeRaceRacer*), __FILE__,
                0x44f);
            for (int i = 0; i < g_TrackGame->field_0x3444->field_0x460 - 1; i++)
                aiRacers[i] = 0;
            for (int n = 1; n < g_TrackGame->field_0x3444->field_0x460; n++) {
                UnknownProCircuitRacer* racer = &g_TrackGame->field_0x3444->field_0x465[n];
                UnknownFunction4210f0(&position, &direction, event, order[n]);
                sprintf(path, "%s\\%s", "Res", racer->field_0x38);
                if (!g_TrackGame->sceneObject->UnknownFunction4e9cd0(stream, path, "rb", (int)bikeFile)) {
                    sprintf(message, "No PC Rider MCF found in resources.  Aborting.\n");
                    delete stream;
                    Release();
                    return 0;
                }
                setup = *(UnknownBikeRaceBikeSetup*)g_TrackGame->mode.field_0xfd8;
                setup.field_0x00 = racer->field_0x138;
                setup.field_0x04 = racer->field_0x13c;
                bikeClass = UnknownBikeClassOf(setup.field_0x00);
                setup.field_0x50 = g_TrackGame->ui->field_0x2fc[bikeClass];
                setup.field_0x54 = g_TrackGame->ui->field_0x310[bikeClass];
                for (int k = 0; k < 11; k++)
                    setup.field_0x24[k] = g_TrackGame->ui->field_0x68[bikeClass][0][k];
                setup.field_0x58 = (setup.field_0x54 - setup.field_0x50) / 10;
                setup.field_0x08 = g_TrackGame->ui->field_0x428[bikeClass];
                skill = UnknownRandomSkill(field_0x07c[0], field_0x07c[1]);
                aiRacers[n - 1] = new(__FILE__, 0x47c) UnknownBikeRaceRacer(1);
                aiRacers[n - 1]->UnknownFunction48fc80(owner, empty, bikeFile, riderFile, lights, textures,
                                                          position, direction, up, terrain, 0, 1, 0, this,
                                                          racer->field_0xf8, 0, id, &setup, 1, 1, field_0x054,
                                                          raceScene, replayVcr, skill, 1);
                AppendChild(aiRacers[n - 1], -1);
                aiRacers[n - 1]->field_0x3bc->UnknownFunction444c70(0, racer->field_0x78, (int)textures);
                aiRacers[n - 1]->field_0x5c4->field_0x1a0->UnknownFunction444c70(0, racer->field_0xb8,
                                                                                     (int)textures);
                id++;
            }
        }
        return 1;
    }

    if (!g_TrackGame->field_0x3428 && g_TrackGame->field_0x18 > 1) {
        racerSlots = new(__FILE__, 0x4a3) UnknownBikeRaceRacer*[racerCount];
    }
    int used[8];
    memset(used, 0, sizeof(used));
    g_TrackGame->ui->UnknownFunction49b530();
    char ai = 0;
    int* bikes = (int*)DebugCalloc(racerCount, 4, __FILE__, 0x4af);
    int* riders = (int*)DebugCalloc(racerCount, 4, __FILE__, 0x4b0);
    const char** names = (const char**)DebugMalloc(racerCount * 4, __FILE__, 0x4b1);
    int plate;
    int engineSize;
    int engineKind;
    char ownRiderTexture[0x40];
    char ownBikeTexture[0x40];
    if (g_TrackGame->field_0x3428) {
        // A replay: the recorded player.
        char recordedBike[0x40];
        char recordedRider[0x40];
        recordIndex = 1;
        replayVcr->UnknownFunction49c1e0(&id, &ai, 0, name, bikeFile, riderFile, recordedBike, recordedRider,
                                           &engineSize, &engineKind, &plate);
        g_TrackGame->ui->UnknownFunction49b560(recordedBike, bikeTexture, 0x3f);
        g_TrackGame->ui->UnknownFunction49b7f0(recordedRider, riderTexture, 0x3f);
    } else {
        sprintf(path, "%s\\%s", "Res", g_TrackGame->mode.field_0x1974.field_0x00);
        if (!g_TrackGame->sceneObject->UnknownFunction4e9cd0(stream, path, "rb", (int)bikeFile)) {
            sprintf(message, "No Rider MCF found in resources.  Aborting.\n");
            delete stream;
            Release();
            return 0;
        }
        g_TrackGame->ui->UnknownFunction49b560(g_TrackGame->mode.field_0x1974.field_0x40,
                                                         bikeTexture, 0x3f);
        g_TrackGame->ui->UnknownFunction49b7f0(g_TrackGame->mode.field_0x1974.field_0x80,
                                                         riderTexture, 0x3f);
        if (!g_TrackGame->mode.field_0x27f8.field_0x28) {
            g_TrackGame->ui->UnknownFunction49b0d0(bikes, racerCount, riders);
            g_TrackGame->ui->UnknownFunction49b020(names, racerCount);
        }
        strcpy(name, g_TrackGame->mode.field_0x00);
        plate = g_TrackGame->mode.field_0x1bcc;
    }

    UnknownBikeRaceNetPlayer* players = (UnknownBikeRaceNetPlayer*)g_TrackGame->mode.field_0x27f8.field_0x14c;
    UnknownTrackGameRacerSlot* slots = g_TrackGame->mode.field_0x1be4;
    if (!g_TrackGame->field_0x3428 && g_TrackGame->field_0x18 != 1) {
        // A network race: this player's racer, the other players' and the
        // AI racers of other hosts.
        playerId = g_TrackGame->network->localPlayer;
        int i;
        for (i = 0; i < racerCount; i++) {
            if (players[i].field_0x00 == playerId && !players[i].field_0x05)
                break;
        }
        if (i < racerCount) {
            int j;
            for (j = 0; j < g_TrackGame->mode.field_0x1be0; j++) {
                if (players[i].field_0x00 == slots[j].field_0xd4 && slots[j].field_0xd8 == 0) {
                    used[j] = 1;
                    break;
                }
            }
            UnknownFunction4210f0(&position, &direction, event, i + 1);
            racerSlots[j] = new(__FILE__, 0x53b) UnknownBikeRaceRacer(1);
            if (!AppendChild(
                    racerSlots[j]->UnknownFunction48fc80(
                        owner, empty, bikeFile, riderFile, lights, textures, position, direction, up, terrain,
                        g_TrackGame->controlInterface, 0, 0, this, g_TrackGame->mode.field_0x00, 0,
                        playerId, (UnknownBikeRaceBikeSetup*)g_TrackGame->mode.field_0xfd8,
                        g_TrackGame->mode.field_0xa8c, g_TrackGame->mode.field_0xa90,
                        field_0x054, raceScene, replayVcr, 0, 1),
                    -1)) {
                sprintf(message, "Net Bike%d not loaded\n", j);
                if (riders)
                    DebugFree(riders, __FILE__, 0x550);
                if (bikes)
                    DebugFree(bikes, __FILE__, 0x551);
                if (names)
                    DebugFree(names, __FILE__, 0x552);
                delete stream;
                Release();
                return 0;
            }
            g_TrackGame->ui->UnknownFunction49b560(g_TrackGame->mode.field_0x1974.field_0x40,
                                                             ownBikeTexture, 0x3f);
            g_TrackGame->ui->UnknownFunction49b7f0(g_TrackGame->mode.field_0x1974.field_0x80,
                                                             ownRiderTexture, 0x3f);
            if (!g_TrackGame->mode.field_0x27f8.field_0x10) {
                racerSlots[j]->field_0x128->field_0x68 = 1;
                racerSlots[j]->field_0x5f0->field_0x68 = 1;
                racerSlots[j]->field_0x5f4->field_0x68 = 1;
            }
            racerSlots[j]->field_0x3bc->UnknownFunction444c70(0, ownBikeTexture, (int)textures);
            racerSlots[j]->field_0x5c4->field_0x1a0->UnknownFunction444c70(0, ownRiderTexture, (int)textures);
            racerSlots[j]->field_0x73c = g_TrackGame->mode.field_0x1bcc;
            if (replayVcr && g_TrackGame->field_0x342c && !g_TrackGame->field_0x3428) {
                replayVcr->UnknownFunction49c070(playerId, 0, recordIndex++, g_TrackGame->mode.field_0x00,
                                                   bikeFile, riderFile, ownBikeTexture, ownRiderTexture,
                                                   racerSlots[j]->field_0x738,
                                                   (unsigned char)racerSlots[j]->field_0x737, slots[j].field_0xc0);
            }
            field_0x14c = j;
            localRacer = racerSlots[j];
        }
        for (i = 0; i < racerCount; i++) {
            if (players[i].field_0x00 == playerId || players[i].field_0x05)
                continue;
            int j;
            for (j = 0; j < g_TrackGame->mode.field_0x1be0; j++) {
                if (players[i].field_0x00 == slots[j].field_0xd4 && slots[j].field_0xd8 == 0) {
                    used[j] = 1;
                    break;
                }
            }
            UnknownFunction4210f0(&position, &direction, event, i + 1);
            setup = *(UnknownBikeRaceBikeSetup*)g_TrackGame->mode.field_0xfd8;
            setup.field_0x00 = slots[j].field_0xec;
            setup.field_0x04 = slots[j].field_0xf0;
            int bikeClass = UnknownBikeClassOf(setup.field_0x00);
            setup.field_0x50 = g_TrackGame->ui->field_0x2fc[bikeClass];
            setup.field_0x54 = g_TrackGame->ui->field_0x310[bikeClass];
            for (int k = 0; k < 11; k++)
                setup.field_0x24[k] = g_TrackGame->ui->field_0x68[bikeClass][0][k];
            setup.field_0x58 = (setup.field_0x54 - setup.field_0x50) / 10;
            sprintf(path, "%s\\%s", "Res", slots[j].field_0x00);
            setup.field_0x08 = g_TrackGame->ui->field_0x428[bikeClass];
            racerSlots[j] = new(__FILE__, 0x5a3) UnknownBikeRaceRacer(1);
            if (!AppendChild(racerSlots[j]->UnknownFunction48fc80(
                                           owner, empty, path, riderFile, lights, textures, position, direction,
                                           up, terrain, 0, 0, 1, this, slots[j].field_0xdc, 0, slots[j].field_0xd4,
                                           &setup, 0, 1, field_0x054, raceScene, replayVcr, 0, 1),
                                       -1)) {
                sprintf(message, "Net Bike%d not loaded\n", j);
            }
            g_TrackGame->ui->UnknownFunction49b560(slots[j].field_0x40, bikeTexture, 0x3f);
            racerSlots[j]->field_0x3bc->UnknownFunction444c70(0, bikeTexture, (int)textures);
            if (g_TrackGame->GetRegistryInt("MPR", 0) == 123 && slots[j].field_0xf4) {
                racerSlots[j]->field_0x5c4->field_0x1a0->UnknownFunction444c70(0, "MPR.tga", (int)textures);
            } else {
                g_TrackGame->ui->UnknownFunction49b7f0(slots[j].field_0x80, riderTexture, 0x3f);
                racerSlots[j]->field_0x5c4->field_0x1a0->UnknownFunction444c70(0, riderTexture, (int)textures);
            }
            if (!g_TrackGame->mode.field_0x27f8.field_0x10) {
                racerSlots[j]->field_0x128->field_0x68 = 1;
                racerSlots[j]->field_0x5f0->field_0x68 = 1;
                racerSlots[j]->field_0x5f4->field_0x68 = 1;
            }
            racerSlots[j]->UnknownVirtualSlot4();
            racerSlots[j]->field_0x11c4 = slots[j].field_0xd0;
            if (replayVcr && g_TrackGame->field_0x342c && !g_TrackGame->field_0x3428) {
                replayVcr->UnknownFunction49c070(slots[j].field_0xd4, 0, recordIndex++, slots[j].field_0xdc, path,
                                                   riderFile, bikeTexture, riderTexture, racerSlots[j]->field_0x738,
                                                   (unsigned char)racerSlots[j]->field_0x737, slots[j].field_0xc0);
            }
        }
        for (i = 0; i < racerCount; i++) {
            int other = players[i].field_0x00;
            if (other == playerId || !players[i].field_0x05)
                continue;
            int j;
            for (j = 0; j < racerCount; j++) {
                if (other == slots[j].field_0xd4 && slots[j].field_0xd8 != 0 && !used[j]) {
                    used[j] = 1;
                    break;
                }
            }
            UnknownFunction4210f0(&position, &direction, event, i + 1);
            setup = *(UnknownBikeRaceBikeSetup*)g_TrackGame->mode.field_0xfd8;
            setup.field_0x00 = slots[j].field_0xec;
            setup.field_0x04 = slots[j].field_0xf0;
            int bikeClass = UnknownBikeClassOf(setup.field_0x00);
            setup.field_0x50 = g_TrackGame->ui->field_0x2fc[bikeClass];
            setup.field_0x54 = g_TrackGame->ui->field_0x310[bikeClass];
            for (int k = 0; k < 11; k++)
                setup.field_0x24[k] = g_TrackGame->ui->field_0x68[bikeClass][0][k];
            setup.field_0x58 = (setup.field_0x54 - setup.field_0x50) / 10;
            setup.field_0x08 = g_TrackGame->ui->field_0x428[bikeClass];
            racerSlots[j] = new(__FILE__, 0x61f) UnknownBikeRaceRacer(1);
            if (!AppendChild(racerSlots[j]->UnknownFunction48fc80(
                                           owner, empty, slots[j].field_0x00, riderFile, lights, textures, position,
                                           direction, up, terrain, 0, (char)slots[j].field_0xd8, 1, this,
                                           slots[j].field_0xdc, 0, slots[j].field_0xd4, &setup, 0, 1, field_0x054,
                                           raceScene, replayVcr, 0, 1),
                                       -1)) {
                sprintf(message, "Net Bike%d not loaded\n", j);
            }
            racerSlots[j]->field_0x3bc->UnknownFunction444c70(0, slots[j].field_0x40, (int)textures);
            racerSlots[j]->field_0x5c4->field_0x1a0->UnknownFunction444c70(0, slots[j].field_0x80, (int)textures);
            if (!g_TrackGame->mode.field_0x27f8.field_0x10) {
                racerSlots[j]->field_0x128->field_0x68 = 1;
                racerSlots[j]->field_0x5f0->field_0x68 = 1;
                racerSlots[j]->field_0x5f4->field_0x68 = 1;
            }
            racerSlots[j]->UnknownVirtualSlot4();
            racerSlots[j]->field_0x11c4 = slots[j].field_0xd0;
            if (replayVcr && g_TrackGame->field_0x342c && !g_TrackGame->field_0x3428) {
                replayVcr->UnknownFunction49c070(slots[j].field_0xd4, (char)slots[j].field_0xd8, recordIndex++,
                                                   slots[j].field_0xdc, slots[j].field_0x00, riderFile,
                                                   slots[j].field_0x40, slots[j].field_0x80,
                                                   racerSlots[j]->field_0x738,
                                                   (unsigned char)racerSlots[j]->field_0x737, slots[j].field_0xc0);
            }
        }
    } else {
        // The player's racer offline (or in a replay).
        UnknownFunction4210f0(&position, &direction, event, order[0]);
        localRacer = new(__FILE__, 0x4ea) UnknownBikeRaceRacer(1);
        if (!AppendChild(
                localRacer->UnknownFunction48fc80(
                    owner, empty, bikeFile, riderFile, lights, textures, position, direction, up, terrain,
                    g_TrackGame->controlInterface, 0, 0, this, name, 0, id,
                    (UnknownBikeRaceBikeSetup*)g_TrackGame->mode.field_0xfd8,
                    g_TrackGame->mode.field_0xa8c, g_TrackGame->mode.field_0xa90, field_0x054,
                    raceScene, replayVcr, 0, 1),
                -1)) {
            sprintf(message, "Player Bike not loaded\n");
            if (riders)
                DebugFree(riders, __FILE__, 0x4ff);
            if (bikes)
                DebugFree(bikes, __FILE__, 0x500);
            if (names)
                DebugFree(names, __FILE__, 0x501);
            delete stream;
            Release();
            return 0;
        }
        localRacer->field_0x3bc->UnknownFunction444c70(0, bikeTexture, (int)textures);
        localRacer->field_0x5c4->field_0x1a0->UnknownFunction444c70(0, riderTexture, (int)textures);
        if (!g_TrackGame->mode.field_0x27f8.field_0x10) {
            localRacer->field_0x128->field_0x68 = 1;
            localRacer->field_0x5f0->field_0x68 = 1;
            localRacer->field_0x5f4->field_0x68 = 1;
        }
        localRacer->field_0x738 = ((UnknownBikeRaceBikeSetup*)g_TrackGame->mode.field_0xfd8)->field_0x00;
        localRacer->field_0x737 = (char)((UnknownBikeRaceBikeSetup*)g_TrackGame->mode.field_0xfd8)->field_0x04;
        localRacer->field_0x73c = plate;
        if ((replayVcr && g_TrackGame->field_0x342c && !g_TrackGame->field_0x3428) ||
            g_TrackGame->mode.field_0x27f8.field_0x00 == 4) {
            replayVcr->UnknownFunction49c070(0, 0, recordIndex++, g_TrackGame->mode.field_0x00, bikeFile,
                                               riderFile, bikeTexture, riderTexture, localRacer->field_0x738,
                                               (unsigned char)localRacer->field_0x737, plate);
        }
    }

    ((FollowCamera*)raceCamera)->UnknownFunction463520(position, 140.0f, 5.0f, 5.0f, 2.5f, 2.3f);
    ((FollowCamera*)raceCamera)->UnknownFunction463520(position, 140.0f, 5.0f, 5.0f, 2.5f, 2.3f);

    int fromEvent = 0;
    if ((g_TrackGame->field_0x3428 && replayVcr->field_0x2fc > 1) || g_TrackGame->field_0x18 == 1) {
        if (g_TrackGame->mode.field_0x27f8.field_0x00 == 4) {
            // The ghost.
            aiRacers = new(__FILE__, 0x674) UnknownBikeRaceRacer*[1];
            UnknownFunction4210f0(&position, &direction, event, order[0]);
            sprintf(bikeFile, "%s\\%s", "Res", "Ghost.mcf");
            sprintf(riderFile, "%s\\GhostRider.mcf", "Res");
            aiRacers[0] = new(__FILE__, 0x67c) UnknownBikeRaceRacer(1);
            if (!AppendChild(
                    aiRacers[0]->UnknownFunction48fc80(
                        owner, empty, bikeFile, riderFile, lights, textures, position, direction, up, terrain, 0, -1,
                        0, this, "Ghost", 0, 0, (UnknownBikeRaceBikeSetup*)g_TrackGame->mode.field_0xfd8,
                        g_TrackGame->mode.field_0xa8c, g_TrackGame->mode.field_0xa90,
                        field_0x054, raceScene, replayVcr, 0, 1),
                    -1)) {
                sprintf(message, "Ghost Bike not loaded\n");
            }
            replayVcr->UnknownFunction49c070(0, 0, recordIndex, g_TrackGame->mode.field_0x00, bikeFile,
                                               riderFile, bikeTexture, riderTexture, aiRacers[0]->field_0x738,
                                               (unsigned char)aiRacers[0]->field_0x737,
                                               g_TrackGame->mode.field_0x1bcc);
            aiRacers[0]->UnknownRacerVirtualSlot50(1, 0, 1);
            aiRacers[0]->UnknownVirtualSlot4();
        } else if (g_TrackGame->mode.field_0x27f8.field_0x24 > 0) {
            // The AI racers.
            id = 1;
            aiRacers = new(__FILE__, 0x6a5) UnknownBikeRaceRacer*[g_TrackGame->mode.field_0x27f8.field_0x24];
            for (int i = 0; i < g_TrackGame->mode.field_0x27f8.field_0x24; i++)
                aiRacers[i] = 0;
            UnknownBikeRaceEventAi* eventAi = (UnknownBikeRaceEventAi*)g_TrackGame->eventManager->field_0x444;
            for (int n = 0; n < g_TrackGame->mode.field_0x27f8.field_0x24; n++) {
                char aiName[0x10];
                if (g_TrackGame->field_0x3428) {
                    char recordedBike[0x40];
                    char recordedRider[0x40];
                    replayVcr->UnknownFunction49c1e0(&id, &ai, recordIndex, aiName, bikeFile, riderFile,
                                                       recordedBike, recordedRider, &engineSize, &engineKind,
                                                       &plate);
                    setup = *(UnknownBikeRaceBikeSetup*)g_TrackGame->mode.field_0xfd8;
                    g_TrackGame->ui->UnknownFunction49b560(recordedBike, bikeTexture, 0x3f);
                    g_TrackGame->ui->UnknownFunction49b7f0(recordedRider, riderTexture, 0x3f);
                } else {
                    if (g_TrackGame->mode.field_0x27f8.field_0x0c > 0 &&
                        g_TrackGame->eventManager->field_0x48 > 0) {
                        fromEvent = 1;
                        COPY_TEXT(aiName, g_TrackGame->eventManager->field_0x50[n + 1].field_0x40, 0x10);
                        COPY_TEXT(bikeFile, eventAi[n].field_0x80, 0x104);
                    } else {
                        COPY_TEXT(aiName, names[n], 0x10);
                        UnknownBikeRaceUiChoice* choices = (UnknownBikeRaceUiChoice*)g_TrackGame->ui->field_0x50;
                        UnknownBikeRaceUiModel* models = (UnknownBikeRaceUiModel*)g_TrackGame->ui->field_0x48;
                        sprintf(bikeFile, "%s\\%s", "Res", models[choices[bikes[n]].field_0x00].field_0x40);
                    }
                    ai = 1;
                    if (!g_TrackGame->sceneObject->UnknownFunction4e9cd0(stream, bikeFile, "rb",
                                                                                   (int)bikeFile)) {
                        sprintf(message, "No Net Bike MCF found in resources.  Aborting.\n");
                        delete stream;
                        Release();
                        return 0;
                    }
                    setup = *(UnknownBikeRaceBikeSetup*)g_TrackGame->mode.field_0xfd8;
                    if (fromEvent) {
                        engineSize = eventAi[n].field_0xc0;
                        engineKind = eventAi[n].field_0xc4;
                        skill = eventAi[n].field_0xc8;
                    } else {
                        UnknownBikeRaceUiChoice* choices = (UnknownBikeRaceUiChoice*)g_TrackGame->ui->field_0x50;
                        engineSize = choices[bikes[n]].field_0x8c;
                        engineKind = choices[bikes[n]].field_0x90;
                        if (g_TrackGame->mode.field_0x27f8.field_0x04 == 0)
                            skill = UnknownRandomSkill(field_0x07c[5], field_0x07c[6]);
                        else
                            skill = UnknownRandomSkill(field_0x07c[0], field_0x07c[1]);
                    }
                    float r = rand() * (1.0f / 32768.0f);
                    plate = 101 - (int)(r * -898.0f);
                }
                UnknownFunction4210f0(&position, &direction, event, order[n + 1]);
                setup.field_0x00 = engineSize;
                setup.field_0x04 = engineKind;
                int bikeClass = UnknownBikeClassOf(setup.field_0x00);
                setup.field_0x50 = g_TrackGame->ui->field_0x2fc[bikeClass];
                setup.field_0x54 = g_TrackGame->ui->field_0x310[bikeClass];
                for (int k = 0; k < 11; k++)
                    setup.field_0x24[k] = g_TrackGame->ui->field_0x68[bikeClass][0][k];
                setup.field_0x58 = (setup.field_0x54 - setup.field_0x50) / 10;
                setup.field_0x08 = g_TrackGame->ui->field_0x428[bikeClass];
                aiRacers[n] = new(__FILE__, 0x718) UnknownBikeRaceRacer(1);
                if (!AppendChild(aiRacers[n]->UnknownFunction48fc80(
                                               owner, empty, bikeFile, riderFile, lights, textures, position,
                                               direction, up, terrain, 0, ai, 0, this, aiName, 0, id, &setup, 1, 1,
                                               field_0x054, raceScene, replayVcr, skill, 1),
                                           -1)) {
                    sprintf(message, "AI Bike%d not loaded\n", n);
                }
                if (g_TrackGame->field_0x3428) {
                    char text[0x40];
                    COPY_TEXT(text, bikeTexture, 0x40);
                    g_TrackGame->ui->UnknownFunction49b560(text, bikeTexture, 0x3f);
                    aiRacers[n]->field_0x3bc->UnknownFunction444c70(0, bikeTexture, (int)textures);
                    COPY_TEXT(text, riderTexture, 0x40);
                    g_TrackGame->ui->UnknownFunction49b7f0(text, riderTexture, 0x3f);
                    aiRacers[n]->field_0x5c4->field_0x1a0->UnknownFunction444c70(0, riderTexture, (int)textures);
                    aiRacers[n]->field_0x73c = plate;
                    recordIndex++;
                } else if (fromEvent) {
                    g_TrackGame->ui->UnknownFunction49b560(eventAi[n].field_0x40, bikeTexture, 0x3f);
                    aiRacers[n]->field_0x3bc->UnknownFunction444c70(0, bikeTexture, (int)textures);
                    g_TrackGame->ui->UnknownFunction49b7f0(eventAi[n].field_0x00, riderTexture, 0x3f);
                    aiRacers[n]->field_0x5c4->field_0x1a0->UnknownFunction444c70(0, riderTexture, (int)textures);
                    aiRacers[n]->field_0x73c = plate;
                } else {
                    UnknownBikeRaceUiChoice* choices = (UnknownBikeRaceUiChoice*)g_TrackGame->ui->field_0x50;
                    UnknownBikeRaceUiModel* riderModels = (UnknownBikeRaceUiModel*)g_TrackGame->ui->field_0x58;
                    g_TrackGame->ui->UnknownFunction49b560(choices[bikes[n]].field_0x48, bikeTexture, 0x3f);
                    aiRacers[n]->field_0x3bc->UnknownFunction444c70(0, bikeTexture, (int)textures);
                    g_TrackGame->ui->UnknownFunction49b7f0(riderModels[riders[n]].field_0x40, riderTexture,
                                                                     0x3f);
                    aiRacers[n]->field_0x5c4->field_0x1a0->UnknownFunction444c70(0, riderTexture, (int)textures);
                    aiRacers[n]->field_0x73c = plate;
                }
                if (!g_TrackGame->mode.field_0x27f8.field_0x10) {
                    aiRacers[n]->field_0x128->field_0x68 = 1;
                    aiRacers[n]->field_0x5f0->field_0x68 = 1;
                    aiRacers[n]->field_0x5f4->field_0x68 = 1;
                }
                if (replayVcr && g_TrackGame->field_0x342c && !g_TrackGame->field_0x3428) {
                    replayVcr->UnknownFunction49c070(id, 1, recordIndex++, aiName, path, riderFile, bikeTexture,
                                                       riderTexture, aiRacers[n]->field_0x738,
                                                       (unsigned char)aiRacers[n]->field_0x737, plate);
                }
                id++;
                if (g_TrackGame->mode.field_0x27f8.field_0x00 == 2 &&
                    g_TrackGame->mode.field_0x27f8.field_0x0c > 0 &&
                    !g_TrackGame->eventManager->field_0x48) {
                    strcpy(eventAi[n].field_0x40, bikeTexture);
                    strcpy(eventAi[n].field_0x00, riderTexture);
                    strcpy(eventAi[n].field_0x80, bikeFile);
                    eventAi[n].field_0xc0 = engineSize;
                    eventAi[n].field_0xc4 = engineKind;
                    eventAi[n].field_0xc8 = skill;
                }
            }
        }
    } else {
        // A network race: the AI racers this host runs.
        id = 1;
        if (g_TrackGame->mode.field_0x27f8.field_0x24 > 0) {
            aiRacers = new(__FILE__, 0x770) UnknownBikeRaceRacer*[g_TrackGame->mode.field_0x27f8.field_0x24];
            for (int i = 0; i < racerCount; i++) {
                int other = players[i].field_0x00;
                if (other != playerId || !players[i].field_0x05)
                    continue;
                int j;
                for (j = 0; j < g_TrackGame->mode.field_0x1be0; j++) {
                    if (other == slots[j].field_0xd4 && slots[j].field_0xd8 != 0 && !used[j]) {
                        used[j] = 1;
                        break;
                    }
                }
                UnknownFunction4210f0(&position, &direction, event, i + 1);
                setup = *(UnknownBikeRaceBikeSetup*)g_TrackGame->mode.field_0xfd8;
                setup.field_0x00 = slots[j].field_0xec;
                setup.field_0x04 = slots[j].field_0xf0;
                int bikeClass = UnknownBikeClassOf(setup.field_0x00);
                setup.field_0x50 = g_TrackGame->ui->field_0x2fc[bikeClass];
                setup.field_0x54 = g_TrackGame->ui->field_0x310[bikeClass];
                for (int k = 0; k < 11; k++)
                    setup.field_0x24[k] = g_TrackGame->ui->field_0x68[bikeClass][0][k];
                setup.field_0x58 = (setup.field_0x54 - setup.field_0x50) / 10;
                setup.field_0x08 = g_TrackGame->ui->field_0x428[bikeClass];
                racerSlots[j] = new(__FILE__, 0x78f) UnknownBikeRaceRacer(1);
                if (!AppendChild(racerSlots[j]->UnknownFunction48fc80(
                                               owner, empty, slots[j].field_0x00, riderFile, lights, textures,
                                               position, direction, up, terrain, 0, (char)slots[j].field_0xd8, 0,
                                               this, slots[j].field_0xdc, 0, slots[j].field_0xd4, &setup, 0, 1,
                                               field_0x054, raceScene, replayVcr, 0, 1),
                                           -1)) {
                    sprintf(message, "Net Bike%d not loaded\n", j);
                }
                racerSlots[j]->field_0x3bc->UnknownFunction444c70(0, slots[j].field_0x40, (int)textures);
                racerSlots[j]->field_0x5c4->field_0x1a0->UnknownFunction444c70(0, slots[j].field_0x80, (int)textures);
                if (!g_TrackGame->mode.field_0x27f8.field_0x10) {
                    racerSlots[j]->field_0x128->field_0x68 = 1;
                    racerSlots[j]->field_0x5f0->field_0x68 = 1;
                    racerSlots[j]->field_0x5f4->field_0x68 = 1;
                }
                if (replayVcr && g_TrackGame->field_0x342c && !g_TrackGame->field_0x3428) {
                    replayVcr->UnknownFunction49c070(slots[j].field_0xd4, (char)slots[j].field_0xd8, recordIndex++,
                                                       slots[j].field_0xdc, slots[j].field_0x00, riderFile,
                                                       slots[j].field_0x40, slots[j].field_0x80,
                                                       racerSlots[j]->field_0x738,
                                                       (unsigned char)racerSlots[j]->field_0x737,
                                                       slots[j].field_0xc0);
                }
                aiRacers[id - 1] = racerSlots[j];
                id++;
            }
        }
        // Every racer's collisions against every other racer's.
        for (int a = 0; a < racerCount; a++) {
            if (!racerSlots[a]->field_0x735)
                continue;
            for (int b = 0; b < racerCount; b++) {
                if (a == b)
                    continue;
                if (racerSlots[b]->field_0x735) {
                    racerSlots[a]->field_0x1418->UnknownFunction439410(racerSlots[b]->field_0x1418);
                    racerSlots[a]->field_0x1418->UnknownFunction439410(racerSlots[b]->field_0x1414);
                    racerSlots[a]->field_0x1418->UnknownFunction439410(racerSlots[b]->field_0x5f0);
                    racerSlots[a]->field_0x1418->UnknownFunction439410(racerSlots[b]->field_0x5f4);
                    racerSlots[a]->field_0x1418->UnknownFunction439410(racerSlots[b]->field_0x604->field_0x38);
                    racerSlots[a]->field_0x1414->UnknownFunction439410(racerSlots[b]->field_0x1418);
                    racerSlots[a]->field_0x1414->UnknownFunction439410(racerSlots[b]->field_0x1414);
                    racerSlots[a]->field_0x1414->UnknownFunction439410(racerSlots[b]->field_0x5f0);
                    racerSlots[a]->field_0x1414->UnknownFunction439410(racerSlots[b]->field_0x5f4);
                    racerSlots[a]->field_0x1414->UnknownFunction439410(racerSlots[b]->field_0x604->field_0x38);
                }
                racerSlots[a]->field_0x5f0->UnknownFunction439410(racerSlots[b]->field_0x1418);
                racerSlots[a]->field_0x5f0->UnknownFunction439410(racerSlots[b]->field_0x1414);
                racerSlots[a]->field_0x5f0->UnknownFunction439410(racerSlots[b]->field_0x5f0);
                racerSlots[a]->field_0x5f0->UnknownFunction439410(racerSlots[b]->field_0x5f4);
                racerSlots[a]->field_0x5f0->UnknownFunction439410(racerSlots[b]->field_0x604->field_0x38);
                racerSlots[a]->field_0x5f4->UnknownFunction439410(racerSlots[b]->field_0x1418);
                racerSlots[a]->field_0x5f4->UnknownFunction439410(racerSlots[b]->field_0x1414);
                racerSlots[a]->field_0x5f4->UnknownFunction439410(racerSlots[b]->field_0x5f0);
                racerSlots[a]->field_0x5f4->UnknownFunction439410(racerSlots[b]->field_0x5f4);
                racerSlots[a]->field_0x5f4->UnknownFunction439410(racerSlots[b]->field_0x604->field_0x38);
                racerSlots[a]->field_0x604->field_0x38->UnknownFunction439410(racerSlots[b]->field_0x1418);
                racerSlots[a]->field_0x604->field_0x38->UnknownFunction439410(racerSlots[b]->field_0x1414);
                racerSlots[a]->field_0x604->field_0x38->UnknownFunction439410(racerSlots[b]->field_0x5f0);
                racerSlots[a]->field_0x604->field_0x38->UnknownFunction439410(racerSlots[b]->field_0x5f4);
                racerSlots[a]->field_0x604->field_0x38->UnknownFunction439410(
                    racerSlots[b]->field_0x604->field_0x38);
            }
        }
    }
    if (riders)
        DebugFree(riders, __FILE__, 0x7f7);
    if (bikes)
        DebugFree(bikes, __FILE__, 0x7f8);
    if (names)
        DebugFree(names, __FILE__, 0x7f9);
    delete stream;
    return 1;
}
