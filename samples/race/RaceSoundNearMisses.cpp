// Near-miss racesnd.cpp candidates, kept out of src/reconstructed until they
// match. See docs/RACESOUND.md. The canonical file is included first so the
// TU-local declarations and globals are the same.
//
// 0x004e39b0 (2348 bytes, the per-racer update): the control flow and every
// store match; retail keeps -1 in ebx through the voice-assignment loop and
// reloads 0 per branch afterwards, VC6 here keeps 0 in ebx throughout. Retail
// also reads the old field_0x10 and compares it with 6 (flags unused) before
// the two "set 6" stores; no source form tried reproduces that dead compare.
//
// 0x004e4d10 (2032 bytes, the half-buffer refill): the first half matches;
// retail keeps the remainder in a stack slot and tail-merges the
// `field_0x18 =` stores, and reloads 11025 * field_0x1298 for the second
// copy's destination, where VC6 here keeps both in registers.

#include "../../src/reconstructed/RaceSound.cpp"

// 0x004e39b0: sorts the channels by distance from the camera, gives the
// nearest four racers the engine voices, steps each channel's engine sample
// set from its throttle and speed, and plays the position and lap sounds.
int RaceSound::UnknownFunction4e39b0(float frameTime) {
    int i;
    int j;

    for (i = 0; i < field_0x94; i++) {
        if (!field_0x98[i].field_0x00->field_0x4a0) {
            field_0x98[i].field_0x4c =
                (int)UnknownFunction4e4460(&field_0x38->field_0x170, &field_0x98[i].field_0x00->field_0x00c);
        } else {
            field_0x98[i].field_0x4c = 1000000;
            field_0x98[i].field_0x04 = 0;
        }
    }
    qsort(field_0x98, field_0x94, sizeof(UnknownRaceSoundChannel), UnknownFunction4e5880);

    for (i = field_0x94 - 1; i > -1; i--) {
        if (field_0x98[i].field_0x04 && i >= 4) {
            field_0x44c[field_0x98[i].field_0x08] = 0;
            field_0x98[i].field_0x04->UnknownFunction4bc940(1);
            field_0x98[i].field_0x04 = 0;
            field_0x98[i].field_0x08 = -1;
        }
        if (!field_0x98[i].field_0x04 && i < 4) {
            for (j = 0; j < 4; j++) {
                if (!field_0x44c[j]) {
                    field_0x98[i].field_0x04 = field_0x43c[j];
                    field_0x98[i].field_0x08 = j;
                    field_0x44c[j] = 1;
                    UnknownFunction4e5780(field_0x98[i].field_0x04, 0, 0, 1, 0);
                    break;
                }
            }
        }
    }

    for (i = 0; i < field_0x94; i++) {
        field_0x434 = &field_0x98[i];
        if (field_0x434->field_0x00->field_0x479)
            field_0x434->field_0x48 = 0;
        else
            field_0x434->field_0x48 = 1;
        if (field_0x434->field_0x00->field_0x108) {
            field_0x434->field_0x3c += frameTime;
            if (field_0x434->field_0x3c >= 0.2f) {
                if (!field_0x434->field_0x00->field_0x478)
                    field_0x434->field_0x30 = 1;
                if (field_0x434->field_0x10 != 5 && field_0x434->field_0x10 != 6 && field_0x434->field_0x10 != 1) {
                    j = rand() % field_0x1164[field_0x434->field_0x0c];
                    field_0x434->field_0x38 = field_0xa98[field_0x434->field_0x0c][j];
                    field_0x434->field_0x14 = field_0xe58[field_0x434->field_0x0c][j];
                    field_0x434->field_0x18 = 0;
                    field_0x434->field_0x10 = 5;
                    field_0x434->field_0x20 = 1;
                    UnknownFunction4e4a30();
                }
                if (field_0x434->field_0x30 == 1 && field_0x434->field_0x00->field_0x478) {
                    field_0x434->field_0x30 = 0;
                    j = rand() % field_0x117c[field_0x434->field_0x0c];
                    field_0x434->field_0x38 = field_0xb88[field_0x434->field_0x0c][j];
                    field_0x434->field_0x14 = field_0xf48[field_0x434->field_0x0c][j];
                    field_0x434->field_0x18 = 0;
                    field_0x434->field_0x10 = 6;
                    field_0x434->field_0x20 = 1;
                    UnknownFunction4e4a30();
                }
            }
        } else {
            field_0x434->field_0x3c = 0;
            field_0x434->field_0x30 = 0;
            if (field_0x434->field_0x2c == 1 && !field_0x83c->UnknownFunction4bca80() &&
                field_0x434->field_0x00 == field_0x2c)
                UnknownFunction4e5780(field_0x83c, 0, 0, 0, 0);
            if (!field_0x434->field_0x00->field_0x478)
                field_0x434->field_0x34 = 1;
            if (field_0x434->field_0x48) {
                if (field_0x434->field_0x00->field_0x478) {
                    if (field_0x434->field_0x10 == 6) {
                        j = rand() % field_0x114c[field_0x434->field_0x0c];
                        field_0x434->field_0x38 = field_0x9a8[field_0x434->field_0x0c][j];
                        field_0x434->field_0x14 = field_0xd68[field_0x434->field_0x0c][j];
                        field_0x434->field_0x18 = 0;
                        field_0x434->field_0x10 = 2;
                        field_0x434->field_0x20 = 3;
                        UnknownFunction4e4a30();
                    }
                    if (field_0x434->field_0x10 == 1 || field_0x434->field_0x10 == 5) {
                        field_0x434->field_0x44 = (int)(field_0x434->field_0x00->field_0x0b8 * 0.68182f);
                        if (field_0x434->field_0x44 < field_0x47c[field_0x434->field_0x0c]) {
                            j = rand() % field_0x1134[field_0x434->field_0x0c];
                            field_0x434->field_0x38 = field_0x8b8[field_0x434->field_0x0c][j];
                            field_0x434->field_0x14 = field_0xc78[field_0x434->field_0x0c][j];
                            field_0x434->field_0x18 = 0;
                            field_0x434->field_0x10 = 2;
                            field_0x434->field_0x20 = 3;
                        } else if (field_0x434->field_0x44 < field_0x488[field_0x434->field_0x0c]) {
                            j = rand() % field_0x1140[field_0x434->field_0x0c];
                            field_0x434->field_0x38 = field_0x930[field_0x434->field_0x0c][j];
                            field_0x434->field_0x14 = field_0xcf0[field_0x434->field_0x0c][j];
                            field_0x434->field_0x18 = 0;
                            field_0x434->field_0x10 = 2;
                            field_0x434->field_0x20 = 3;
                        } else if (field_0x434->field_0x44 < field_0x494[field_0x434->field_0x0c]) {
                            j = rand() % field_0x114c[field_0x434->field_0x0c];
                            field_0x434->field_0x38 = field_0x9a8[field_0x434->field_0x0c][j];
                            field_0x434->field_0x14 = field_0xd68[field_0x434->field_0x0c][j];
                            field_0x434->field_0x18 = 0;
                            field_0x434->field_0x10 = 2;
                            field_0x434->field_0x20 = 3;
                        } else {
                            j = rand() % field_0x1158[field_0x434->field_0x0c];
                            field_0x434->field_0x38 = field_0xa20[field_0x434->field_0x0c][j];
                            field_0x434->field_0x14 = field_0xde0[field_0x434->field_0x0c][j];
                            field_0x434->field_0x18 = 0;
                            field_0x434->field_0x10 = 3;
                            field_0x434->field_0x20 = 4;
                        }
                        UnknownFunction4e4a30();
                    }
                } else if (field_0x434->field_0x10 == 2 || field_0x434->field_0x10 == 3 ||
                           field_0x434->field_0x10 == 4) {
                    j = rand() % field_0x1164[field_0x434->field_0x0c];
                    field_0x434->field_0x38 = field_0xa98[field_0x434->field_0x0c][j];
                    field_0x434->field_0x14 = field_0xe58[field_0x434->field_0x0c][j];
                    field_0x434->field_0x18 = 0;
                    field_0x434->field_0x10 = 5;
                    field_0x434->field_0x20 = 1;
                    UnknownFunction4e4a30();
                }
            } else if (field_0x434->field_0x00->field_0x478 == true &&
                       (field_0x434->field_0x34 == 1 || field_0x434->field_0x28 == 1)) {
                field_0x434->field_0x34 = 0;
                j = rand() % 1;
                field_0x434->field_0x38 = field_0xb88[field_0x434->field_0x0c][j];
                field_0x434->field_0x14 = field_0xf48[field_0x434->field_0x0c][j];
                field_0x434->field_0x18 = 0;
                field_0x434->field_0x10 = 6;
                field_0x434->field_0x20 = 1;
                UnknownFunction4e4a30();
            }
        }
        field_0x434->field_0x28 = field_0x434->field_0x48;
        field_0x434->field_0x24 = (unsigned char)field_0x434->field_0x00->field_0x478;
        field_0x434->field_0x2c = (unsigned char)field_0x434->field_0x00->field_0x108;
        UnknownFunction4e4d10();
    }

    if (g_UnknownGlobal56e26c->mode.field_0xa38 &&
        (field_0x2c->field_0x7b8 > field_0x1294 || field_0x2c->field_0x7a0 > field_0x1290))
        UnknownFunction4e5780(field_0x1204, 0, 0, 0, 0);

    for (i = 0; i < field_0x94; i++) {
        field_0x438 = field_0x98[i].field_0x00;
        if (!field_0x438->field_0x444) {
            field_0x120c[i] = 0;
            field_0x1238[i] = 0;
            field_0x1264[i] = 0;
            continue;
        }
        if (!field_0x120c[i]) {
            if (field_0x438 == field_0x2c && field_0x2c->field_0x460 == 12) {
                field_0x11b4->UnknownFunction4bc940(1);
                UnknownFunction4e5780(field_0x11b4, 0, 0, 0, 0);
            }
            if (g_UnknownGlobal56e26c->field_0x2d74 == 3) {
                if (field_0x438 == field_0x2c && field_0x438->field_0x784 == 1)
                    UnknownFunction4e5780(field_0x11f8, 0, 0, 0, 0);
                if (field_0x438 == field_0x2c && field_0x438->field_0x784 > 1 && field_0x12a4 > 30.0f) {
                    UnknownFunction4e5780(field_0x11fc, 0, 0, 0, 0);
                    field_0x12a4 = 0;
                }
            }
        }
        if (field_0x438->field_0x484 &&
            (!field_0x1238[i] || !field_0x11b8[field_0x1238[i]]->UnknownFunction4bca80()))
            field_0x1238[i] = UnknownFunction4e42e0(field_0x438);
        if (field_0x438->field_0x604->field_0xb0 &&
            (!field_0x1264[i] || !field_0x11d0[field_0x1264[i]]->UnknownFunction4bca80()))
            field_0x1264[i] = UnknownFunction4e43a0(field_0x438);
        field_0x120c[i] = 1;
        if (field_0x438 == field_0x2c && field_0x438->field_0x784 < field_0x129c) {
            if (field_0x438->field_0x784 == 1) {
                if (field_0x11f4)
                    UnknownFunction4e5780(field_0x11f4, 0, 0, 0, 0);
            } else if (field_0x12a8 > 15.0f) {
                if (rand() > 0.5f) {
                    if (field_0x11ec)
                        UnknownFunction4e5780(field_0x11ec, 0, 0, 0, 0);
                } else if (field_0x11f0)
                    UnknownFunction4e5780(field_0x11f0, 0, 0, 0, 0);
                field_0x12a8 = 0;
            }
        }
    }
    field_0x1290 = field_0x2c->field_0x7a0;
    field_0x1294 = field_0x2c->field_0x7b8;
    field_0x129c = field_0x2c->field_0x784;
    return 1;
}

// 0x004e4d10: once the play cursor has crossed into the other half, refills
// the half just played from the sample, looping it (field_0x20 0) or going
// on to a random sample of the set field_0x20 names.
void RaceSound::UnknownFunction4e4d10() {
    void* first;
    unsigned long write;
    void* second;
    unsigned long play;
    unsigned long secondBytes;
    unsigned long firstBytes;
    int half;
    int avail;
    int rest;
    int j;

    if (!field_0x434->field_0x04)
        return;
    field_0x434->field_0x04->UnknownFunction4bcd40(&play, &write);
    if (field_0x434->field_0x1c == 1) {
        if (write < (unsigned long)(16537 * field_0x1298))
            return;
        field_0x434->field_0x04->UnknownFunction4bd020(0, 0, &first, &firstBytes, &second, &secondBytes, 2);
        if (field_0x434->field_0x20 == 0) {
            half = 11025 * field_0x1298;
            avail = field_0x434->field_0x14 - field_0x434->field_0x18;
            if (half > avail) {
                rest = half - avail;
                memcpy((char*)field_0x434->field_0x50, (char*)field_0x434->field_0x38 + field_0x434->field_0x18, avail);
                memcpy((char*)field_0x434->field_0x50 + avail, field_0x434->field_0x38, rest);
                memcpy(first, field_0x434->field_0x50, 22050 * field_0x1298);
                field_0x434->field_0x18 = rest;
            } else {
                memcpy((char*)field_0x434->field_0x50, (char*)field_0x434->field_0x38 + field_0x434->field_0x18, half);
                memcpy(first, field_0x434->field_0x50, 22050 * field_0x1298);
                field_0x434->field_0x18 += half;
            }
        } else {
            half = 11025 * field_0x1298;
            avail = field_0x434->field_0x14 - field_0x434->field_0x18;
            if (half > avail) {
                rest = half - avail;
                memcpy((char*)field_0x434->field_0x50, (char*)field_0x434->field_0x38 + field_0x434->field_0x18, avail);
                switch (field_0x434->field_0x20) {
                case 1:
                    j = rand() % field_0x1128[field_0x434->field_0x0c];
                    field_0x434->field_0x38 = field_0x840[field_0x434->field_0x0c][j];
                    field_0x434->field_0x18 = 0;
                    field_0x434->field_0x14 = field_0xc00[field_0x434->field_0x0c][j];
                    field_0x434->field_0x10 = 1;
                    field_0x434->field_0x20 = 0;
                    break;
                case 3:
                    j = rand() % field_0x1158[field_0x434->field_0x0c];
                    field_0x434->field_0x38 = field_0xa20[field_0x434->field_0x0c][j];
                    field_0x434->field_0x18 = 0;
                    field_0x434->field_0x14 = field_0xde0[field_0x434->field_0x0c][j];
                    field_0x434->field_0x10 = 3;
                    field_0x434->field_0x20 = 4;
                    break;
                case 4:
                    j = rand() % field_0x1170[field_0x434->field_0x0c];
                    field_0x434->field_0x38 = field_0xb10[field_0x434->field_0x0c][j];
                    field_0x434->field_0x18 = 0;
                    field_0x434->field_0x14 = field_0xed0[field_0x434->field_0x0c][j];
                    field_0x434->field_0x10 = 4;
                    field_0x434->field_0x20 = 3;
                    break;
                }
                memcpy((char*)field_0x434->field_0x50 + avail, field_0x434->field_0x38, rest);
                memcpy(first, field_0x434->field_0x50, 22050 * field_0x1298);
                field_0x434->field_0x18 = rest;
            } else {
                memcpy((char*)field_0x434->field_0x50, (char*)field_0x434->field_0x38 + field_0x434->field_0x18, half);
                memcpy(first, field_0x434->field_0x50, 22050 * field_0x1298);
                field_0x434->field_0x18 += half;
            }
        }
        field_0x434->field_0x04->UnknownFunction4bd080(first, half, second, 0);
        field_0x434->field_0x1c = 2;
    } else if (field_0x434->field_0x1c == 2) {
        if (write < (unsigned long)(5512 * field_0x1298) || write > (unsigned long)(11025 * field_0x1298))
            return;
        field_0x434->field_0x04->UnknownFunction4bd020(0, 0, &first, &firstBytes, &second, &secondBytes, 2);
        if (field_0x434->field_0x20 == 0) {
            half = 11025 * field_0x1298;
            avail = field_0x434->field_0x14 - field_0x434->field_0x18;
            if (half > avail) {
                rest = half - avail;
                memcpy((char*)field_0x434->field_0x50 + half, (char*)field_0x434->field_0x38 + field_0x434->field_0x18, avail);
                memcpy((char*)field_0x434->field_0x50 + half + avail, field_0x434->field_0x38, rest);
                memcpy(first, field_0x434->field_0x50, 22050 * field_0x1298);
                field_0x434->field_0x18 = rest;
            } else {
                memcpy((char*)field_0x434->field_0x50 + half, (char*)field_0x434->field_0x38 + field_0x434->field_0x18, half);
                memcpy(first, field_0x434->field_0x50, 22050 * field_0x1298);
                field_0x434->field_0x18 += half;
            }
        } else {
            half = 11025 * field_0x1298;
            avail = field_0x434->field_0x14 - field_0x434->field_0x18;
            if (half > avail) {
                rest = half - avail;
                memcpy((char*)field_0x434->field_0x50 + half, (char*)field_0x434->field_0x38 + field_0x434->field_0x18, avail);
                switch (field_0x434->field_0x20) {
                case 1:
                    j = rand() % field_0x1128[field_0x434->field_0x0c];
                    field_0x434->field_0x38 = field_0x840[field_0x434->field_0x0c][j];
                    field_0x434->field_0x18 = 0;
                    field_0x434->field_0x14 = field_0xc00[field_0x434->field_0x0c][j];
                    field_0x434->field_0x10 = 1;
                    field_0x434->field_0x20 = 0;
                    break;
                case 3:
                    j = rand() % field_0x1158[field_0x434->field_0x0c];
                    field_0x434->field_0x38 = field_0xa20[field_0x434->field_0x0c][j];
                    field_0x434->field_0x18 = 0;
                    field_0x434->field_0x14 = field_0xde0[field_0x434->field_0x0c][j];
                    field_0x434->field_0x10 = 3;
                    field_0x434->field_0x20 = 4;
                    break;
                case 4:
                    j = rand() % field_0x1170[field_0x434->field_0x0c];
                    field_0x434->field_0x38 = field_0xb10[field_0x434->field_0x0c][j];
                    field_0x434->field_0x18 = 0;
                    field_0x434->field_0x14 = field_0xed0[field_0x434->field_0x0c][j];
                    field_0x434->field_0x10 = 4;
                    field_0x434->field_0x20 = 3;
                    break;
                }
                memcpy((char*)field_0x434->field_0x50 + half + avail, field_0x434->field_0x38, rest);
                memcpy(first, field_0x434->field_0x50, 22050 * field_0x1298);
                field_0x434->field_0x18 = rest;
            } else {
                memcpy((char*)field_0x434->field_0x50 + half, (char*)field_0x434->field_0x38 + field_0x434->field_0x18, half);
                memcpy(first, field_0x434->field_0x50, 22050 * field_0x1298);
                field_0x434->field_0x18 += half;
            }
        }
        field_0x434->field_0x04->UnknownFunction4bd080(first, 22050 * field_0x1298, second, 0);
        field_0x434->field_0x1c = 1;
    }
}

