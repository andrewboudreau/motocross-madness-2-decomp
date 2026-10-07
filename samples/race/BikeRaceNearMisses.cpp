// Near-miss bikerace.cpp candidates, kept out of src/reconstructed until
// they match. The canonical file is included first so the TU-local
// declarations, statics and inline helpers are the same.
//
// 0x0041eb20 (1708 bytes; camera target): 1699 of 1710 positions match.
// Retail loads the scene-table index (+0x150) before the table pointer when
// it addresses the entry (edx/ecx swapped at the branch head as well); a
// local index, a reference, pointer arithmetic and store orders did not
// reproduce it.
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

#include "../../src/reconstructed/BikeRace.cpp"

// 0x0041eb20
int BikeRace::UnknownFunction41eb20(int caster) {
    if (UnknownFunctionCameraView()->field_0x390) {
        field_0x0b0 = 0xff;
        field_0x0ac = 0;
        if (g_UnknownGlobal56e26c->field_0x18 > 1) {
            UnknownFunctionCameraView()->field_0x3b0->field_0x3bc->UnknownFunction4fdb50();
            UnknownFunctionCameraView()->field_0x3b0->field_0x5c4->field_0x1a0->UnknownFunction4fdb50();
            UnknownFunctionCameraView()->UnknownFunctionFollow(field_0x03c[field_0x14c]);
            if (field_0x05c != 0) {
                field_0x05c->UnknownFunction4dab90();
                field_0x05c->UnknownFunction4dab00(field_0x03c[field_0x14c]->field_0x3bc);
                field_0x05c->UnknownFunction4dab00(field_0x03c[field_0x14c]->field_0x5c4->field_0x1a0);
            }
            if (g_UnknownGlobal56e26c->field_0x560 != 0) {
                UnknownBikeRaceRacer* racer = field_0x03c[field_0x14c];
                g_UnknownGlobal56e26c->field_0x560->UnknownFunction404df0(racer->field_0x7b8,
                                                                          racer->field_0x7bc, racer);
            }
        } else if (field_0x14c == 0) {
            UnknownFunctionCameraView()->field_0x3b0->field_0x3bc->UnknownFunction4fdb50();
            UnknownFunctionCameraView()->field_0x3b0->field_0x5c4->field_0x1a0->UnknownFunction4fdb50();
            UnknownFunctionCameraView()->UnknownFunctionFollow(field_0x038);
            if (field_0x05c != 0) {
                field_0x05c->UnknownFunction4dab90();
                field_0x05c->UnknownFunction4dab00(field_0x038->field_0x3bc);
                field_0x05c->UnknownFunction4dab00(field_0x038->field_0x5c4->field_0x1a0);
            }
            if (g_UnknownGlobal56e26c->field_0x560 != 0) {
                UnknownBikeRaceRacer* racer = field_0x038;
                g_UnknownGlobal56e26c->field_0x560->UnknownFunction404df0(racer->field_0x7b8,
                                                                          racer->field_0x7bc, racer);
            }
        } else {
            UnknownFunctionCameraView()->field_0x3b0->field_0x3bc->UnknownFunction4fdb50();
            UnknownFunctionCameraView()->field_0x3b0->field_0x5c4->field_0x1a0->UnknownFunction4fdb50();
            int index = field_0x14c - 1;
            UnknownFunctionCameraView()->UnknownFunctionFollow(field_0x040[index]);
            if (field_0x05c != 0) {
                field_0x05c->UnknownFunction4dab90();
                field_0x05c->UnknownFunction4dab00(field_0x040[index]->field_0x3bc);
                field_0x05c->UnknownFunction4dab00(field_0x040[index]->field_0x5c4->field_0x1a0);
            }
            if (g_UnknownGlobal56e26c->field_0x560 != 0) {
                UnknownBikeRaceRacer* racer = field_0x040[index];
                g_UnknownGlobal56e26c->field_0x560->UnknownFunction404df0(racer->field_0x7b8,
                                                                          racer->field_0x7bc, racer);
            }
        }
    } else {
    if (caster) {
        field_0x0b0 = 0;
        field_0x0ac = field_0x154;
        if (field_0x058->field_0xb8 != 0 && field_0x058->field_0xb8->field_0x00 > 0) {
            if (field_0x05c != 0) {
                field_0x05c->UnknownFunction4dab90();
            }
            UnknownFunctionCameraView()->UnknownFunctionTargetCaster(
                field_0x058->field_0xb8->field_0x04[field_0x154].field_0x04);
        } else {
            UnknownFunctionCameraView()->field_0x390 = 1;
        }
    } else {
        field_0x0b0 = 1;
        field_0x0ac = field_0x150;
        if (field_0x058->field_0xb4 != 0 && field_0x058->field_0xb4->field_0x00 != 0) {
            if (field_0x05c != 0) {
                field_0x05c->UnknownFunction4dab90();
            }
            UnknownSceneEntry* entry = &field_0x058->field_0xb4->field_0x04[field_0x150];
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
    int players = g_UnknownGlobal56e26c->field_0x18;
    if (players == 1 && index == 0) {
        return field_0x038;
    }
    if (players > 1 || index < players) {
        return field_0x03c[index];
    }
    return field_0x040[index - players];
}

// 0x0041f1d0
void BikeRace::UnknownFunction41f1d0(int forward, int racers, int objects) {
    int caster;
    if (g_UnknownGlobal56e26c->uiInteractionBlocked) {
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
        if (g_UnknownGlobal56e26c->field_0x18 > 1) {
            others = g_UnknownGlobal56e26c->field_0x3424;
        } else if (g_UnknownGlobal56e26c->mode.field_0x27f8.field_0x00 == 4) {
            others = 1;
        } else {
            others = g_UnknownGlobal56e26c->mode.field_0x27f8.field_0x24;
        }
        if (g_UnknownGlobal56e26c->field_0x18 == 1 && others == 0) {
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
                field_0x14c = g_UnknownGlobal56e26c->field_0x18 + others - 1;
            }
            if (field_0x14c >= g_UnknownGlobal56e26c->field_0x18 + others) {
                field_0x14c = 0;
            }
            if (old == field_0x14c) {
                UnknownFunctionCameraView()->field_0x390 = 1;
                return;
            }
        } while (UnknownFunctionRacerAt(field_0x14c)->field_0x4a0 != 0);
    } else if (objects) {
        caster = 0;
        Scene* scene = field_0x058;
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
        UnknownSceneBuffer* casters = field_0x058->field_0xb8;
        if (casters == 0 || casters->field_0x00 <= 0) {
            UnknownFunctionCameraView()->field_0x390 = 1;
            return;
        }
        int old = field_0x154;
        field_0x154 = forward ? old + 1 : old - 1;
        if (field_0x154 < 0) {
            field_0x154 = field_0x058->field_0xb8->field_0x00 - 1;
        }
        if (field_0x154 >= field_0x058->field_0xb8->field_0x00) {
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
    if (g_UnknownGlobal56e26c->field_0x08 != 0 && event->kind == 0) {
        g_UnknownGlobal56e26c->field_0x14->keyboard->UnknownFunction48a240(0xc);
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
    if (!field_0x190 && g_UnknownGlobal56e26c->uiInteractionBlocked) {
        return 0;
    }
    if (event->kind == 0) {
        switch (event->control) {
        case 0x35:
            if (g_UnknownGlobal56e26c->field_0x18 > 1) {
                KeyboardDevice* keyboard = g_UnknownGlobal56e26c->field_0x14->keyboard;
                if (!keyboard->UnknownVirtualSlot5(0x2a, 0x3f, 0) &&
                    !g_UnknownGlobal56e26c->field_0x14->keyboard->UnknownVirtualSlot5(0x36, 0x3f, 0)) {
                    if (g_UnknownGlobal56e26c->mode.field_0x6b4 == 2) {
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
                g_UnknownGlobal56e26c->mode.field_0x6b4++;
                if (g_UnknownGlobal56e26c->mode.field_0x6b4 == 3) {
                    g_UnknownGlobal56e26c->mode.field_0x6b4 = 0;
                }
                if (g_UnknownGlobal56e26c->mode.field_0x6b4 == 1 && field_0x19c != 0) {
                    field_0x19c->UnknownFunction51dd10();
                    field_0x19c->field_0x3dc = 1;
                    field_0x190 = true;
                }
                char title[0x80];
                char value[0x80];
                char text[0x100];
                g_UnknownGlobal56e26c->UnknownFunction521970(0x1428, title, 0x80);
                switch (g_UnknownGlobal56e26c->mode.field_0x6b4) {
                case 0:
                    g_UnknownGlobal56e26c->UnknownFunction521970(0x140b, value, 0x80);
                    break;
                case 1:
                    g_UnknownGlobal56e26c->UnknownFunction521970(0x140a, value, 0x80);
                    break;
                case 2:
                    g_UnknownGlobal56e26c->UnknownFunction521970(0x140c, value, 0x80);
                    break;
                }
                sprintf(text, "%s : %s", title, value);
                UnknownMessage* message = new (__FILE__, 0xcdd) UnknownMessage(text, 3.25f);
                TextQueueOverlay* overlay;
                switch (g_UnknownGlobal56e26c->mode.field_0x27f8.field_0x04) {
                case 0:
                    overlay = g_UnknownGlobal56e26c->field_0x55c->field_0x6c;
                    break;
                case 1:
                case 5:
                    overlay = g_UnknownGlobal56e26c->field_0x560->field_0x6c;
                    break;
                case 2:
                    overlay = g_UnknownGlobal56e26c->field_0x564->field_0x6c;
                    break;
                case 4:
                    overlay = g_UnknownGlobal56e26c->field_0x568->field_0x6c;
                    break;
                }
                if (overlay != 0 && message != 0) {
                    overlay->UnknownFunction51b540(message);
                }
                delete message;
            }
            break;
        case 0x1b:
            if ((g_UnknownGlobal56e26c->field_0x2d4_bit2) &&
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
            if ((g_UnknownGlobal56e26c->field_0x2d4_bit2) &&
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
                g_UnknownGlobal56e26c->mode.field_0xa8c = 1 - g_UnknownGlobal56e26c->mode.field_0xa8c;
                field_0x038->field_0x5bc = g_UnknownGlobal56e26c->mode.field_0xa8c;
                TextQueueOverlay* overlay = g_UnknownGlobal56e26c->eventManager->UnknownFunction45d340();
                if (overlay == 0) {
                    return 1;
                }
                char title[0x80];
                char value[0x80];
                char text[0x100];
                g_UnknownGlobal56e26c->UnknownFunction521970(0x140d, title, 0x80);
                g_UnknownGlobal56e26c->UnknownFunction521970(field_0x038->field_0x5bc ? 0x1407 : 0x1408,
                                                             value, 0x80);
                sprintf(text, "%s %s", title, value);
                UnknownMessage message(text, 1.5f);
                overlay->UnknownFunction51b540(&message);
                return 1;
            }
            break;
        case 0x30:
            if (UnknownFunction43caa0(0x30, 0, event, 0xc)) {
                g_UnknownGlobal56e26c->mode.field_0xa90 = 1 - g_UnknownGlobal56e26c->mode.field_0xa90;
                field_0x038->field_0x5c0 = g_UnknownGlobal56e26c->mode.field_0xa90;
                TextQueueOverlay* overlay = g_UnknownGlobal56e26c->eventManager->UnknownFunction45d340();
                if (overlay == 0) {
                    return 1;
                }
                char title[0x80];
                char value[0x80];
                char text[0x100];
                g_UnknownGlobal56e26c->UnknownFunction521970(0x142d, title, 0x80);
                g_UnknownGlobal56e26c->UnknownFunction521970(field_0x038->field_0x5c0 ? 0x1407 : 0x1408,
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
    if (g_UnknownGlobal56e26c->field_0x2d4_bit2 && g_UnknownGlobal56e26c->field_0x38 != 0) {
        if (field_0x0a8 < 0) {
            field_0x0a8 = g_UnknownGlobal56e26c->field_0x38->NewPage();
        }
        if (g_UnknownGlobal56e26c->field_0x38->field_0x26c4 == field_0x0a8) {
            if (UnknownFunction43caa0(0x1c, 0, event, 0x80)) {
                UnknownKrustyUIGui* gui = g_UnknownGlobal56e26c->ui->field_0x2c;
                if (gui->UnknownFunction486540(0)->field_0x30 == 0) {
                    g_UnknownGlobal56e26c->ui->field_0x2c->UnknownFunction486590("ui\\cursor.tga", 0);
                }
                if (((UnknownBikeRaceObjectFlags*)g_UnknownGlobal56e26c->ui->field_0x2c
                         ->UnknownFunction486540(0)->field_0x30)->field_0x25_bit0) {
                    g_UnknownGlobal56e26c->field_0x38->UnknownFunction448000(
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
                } else if (field_0x0b4 == 0) {
                    field_0x0b4 = new (__FILE__, 0xed2) ObjectPicker(1);
                    field_0x0b4->UnknownFunction4b0210(
                        g_UnknownGlobal56e26c->field_0x10, 0, UnknownFunction417b00,
                        (GameCursor*)g_UnknownGlobal56e26c->ui->field_0x2c->UnknownFunction486540(0)->field_0x30);
                    if (UnknownFunction469190(field_0x0b4, -1)) {
                        g_UnknownGlobal56e26c->ui->UnknownFunction499b00();
                    }
                } else {
                    g_UnknownGlobal56e26c->ui->UnknownFunction499b00();
                }
            }
            if (UnknownFunction43caa0(0, 1, event, 0x80000000)) {
                int picked = field_0x0b4->UnknownFunction4b04c0();
                if (picked != 0) {
                    Scene* scene = field_0x058;
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
        *a = Scale(field_0x170, (float)count) + field_0x164;
        *b = field_0x17c;
        return;
    }
    switch (g_UnknownGlobal56e26c->mode.field_0x27f8.field_0x04) {
    case 2:
    case 3: {
        if (field_0x144) {
            a->x = field_0x0cc.field_0x00.x;
            a->z = field_0x0cc.field_0x00.z;
            a->y = 0.0f;
            b->x = field_0x0cc.field_0x0c.x;
            b->z = field_0x0cc.field_0x0c.z;
            b->y = 0.0f;
        } else {
            *a = field_0x058->field_0x70;
            *b = Normalize(field_0x058->field_0x7c);
        }
        if (count <= 0) {
            return;
        }
        float side;
        if (field_0x048->field_0x00 != 0 && field_0x048->field_0x00->field_0x08 != 0 &&
            field_0x048->field_0x00->field_0x08->field_0x2c != 0 &&
            field_0x048->field_0x00->field_0x08->field_0x2c->field_0x2c != 0) {
            TrackVec3 p0;
            TrackVec3 p1;
            field_0x048->UnknownFunction518130(field_0x048->field_0x00->field_0x08, &p0);
            field_0x048->UnknownFunction518130(field_0x048->field_0x00->field_0x08->field_0x2c, &p1);
            if (p0.z * p1.x - p1.z * p0.x <= 0.0f) {
                side = 1.0f;
            } else {
                side = -1.0f;
            }
        } else {
            side = -1.0f;
        }
        field_0x17c = *b;
        *a -= Scale(*b, 4.66f);
        field_0x170 = Vector3(b->z, 0.0f, -b->x);
        field_0x164 = *a - Scale(Scale(Scale(field_0x170, 6.75f), field_0x04c->field_0x40), side);
        field_0x170 *= field_0x04c->field_0x40 * 15.5f / (field_0x158 + 1) * side;
        *a = Scale(field_0x170, (float)count) + field_0x164;
        break;
    }
    case 0:
        *a = field_0x058->field_0x70;
        *b = Normalize(field_0x058->field_0x7c);
        if (count <= 0) {
            return;
        }
        field_0x17c = *b;
        *a -= Scale(*b, 4.66f);
        field_0x170 = Vector3(b->z, 0.0f, -b->x);
        field_0x164 = *a - Scale(field_0x170, 20.0f);
        field_0x170 = Scale(Vector3(b->z, 0.0f, -b->x), 20.0f);
        *a = Scale(field_0x170, (float)count) + field_0x164;
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
        float spacing = width / (field_0x158 + 1);
        if (3.4f > spacing) {
            spacing = 3.4f;
        }
        field_0x17c = *b;
        field_0x170 = Vector3(b->z, 0.0f, -b->x);
        field_0x164 = *a - Scale(field_0x170, width * 0.48f);
        field_0x170 = Scale(Vector3(b->z, 0.0f, -b->x), spacing);
        *a = Scale(field_0x170, (float)count) + field_0x164;
        break;
    }
    case 4:
        *a = field_0x058->field_0x70;
        *b = Normalize(field_0x058->field_0x7c);
        if (count <= 0) {
            return;
        }
        field_0x17c = *b;
        *a -= Scale(*b, 4.66f);
        field_0x170 = Vector3(b->z, 0.0f, -b->x);
        field_0x164 = *a - Scale(field_0x170, 20.0f);
        field_0x170 = Scale(Vector3(b->z, 0.0f, -b->x), 20.0f);
        *a = field_0x164 + Scale(field_0x170, (float)count);
        break;
    default:
        return;
    }
    field_0x188 = true;
}
