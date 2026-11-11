// KrustyBikeTrickPick.cpp -- KrustyBike::Fn_0048D780 (0x0048d780, 291 B, ret 0xc), the AI
// trick pick that BikeAI's racing line 0x00414370 calls.  Status: best-guess-unreviewed.  The
// game build links it with /QIfist (config/game_build.json candidate_sources) in place of the
// src/bringup/Group3_KrustyBikeAISupport.cpp copy; the owning translation unit is not known
// (the function sits among KrustyBike's methods, but no source string is referenced).
//
// Picks a trick that fits in `t` seconds.  The table 0x0056cca0 holds 12 rows of 0x50 bytes in
// decreasing clip length: {minimum time, all count, first-group count, second-group count,
// 16 trick ids}.  The first row whose time is below t is used; `group` 0 draws from all its
// tricks, 1 from the first group, 2 from the second group (the first when the second is
// empty).  An AI bike (aiKind, +0x734) also sets its trick hold time (+0x152c) from the slack
// left after the clip (0x0056cc60 clip lengths); it fails when that slack is under 0.1 s.
// Returns 1 with *out the trick id, else 0.
//
// Checked against the retail disassembly: the row search (`fcomp; test ah, 0x41`, so
// t > minTime) and the `row < 0` exit, the group switch (0, 1, 2, else fail), the second-group
// fallback, the index `rand() * (1/32768) * count - 0.5` rounded by retail's inline fistp
// helper at 0x0048d834 (floor of the product, ties to even), the aiKind byte, the slack test
// (`test ah, 1`: slack < 0.1 fails) and the hold-time product order.  Both tables equal
// retail's 0x0056cc60..0x0056d060 bytes.  The index is in [-0.5, count - 0.5) with count <= 16,
// so FistpFloatToInt's 64-bit store equals retail's 32-bit one.  Differentially tested with
// tools/emu_cases/krusty_trick_pick.py: 0 of 6000 runs differ (53- and 24-bit precision).
//
// Shape (linked_accuracy masked, /QIfist): 0.773, 110 against 97 instructions.  The named
// `pick` and `hold` locals keep retail's product order (rand * 1/32768 first); written as one
// expression VC6 multiplies by count first and folds 1/32768 * 0.8 into one constant (0.757,
// and the hold time then rounds differently in its last bit).  `rand() / 32768.0f` and an
// inline Random01 helper compile the same as the single expression.  Retail's ebp frame comes
// from its inline asm; the frame pragma below keeps it (0.804).  What remains is the shared
// failure epilogue.
#include <stdlib.h>

#include "vehicle/KrustyBike.h"
#include "../../common/FistpFloatToInt.h"

struct KbTrickRow {
    float minTime;
    int allCount;
    int firstCount;
    int secondCount;
    int tricks[16];
};

static const KbTrickRow s_kbAiTrickRows[12] = {   // 0x0056cca0
    {2.4000001f, 16, 8, 8, {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15}},
    {2.3333001f, 15, 7, 8, {0, 1, 2, 3, 4, 5, 6, 8, 9, 10, 11, 12, 13, 14, 15, 0}},
    {2.2665999f, 12, 5, 7, {0, 1, 2, 4, 5, 8, 9, 10, 12, 13, 14, 15, 0, 0, 0, 0}},
    {2.1333001f, 11, 4, 7, {0, 1, 4, 5, 8, 9, 10, 12, 13, 14, 15, 0, 0, 0, 0, 0}},
    {1.9375f, 8, 4, 4, {0, 1, 4, 5, 8, 9, 10, 14, 15, 0, 0, 0, 0, 0, 0, 0}},
    {1.8f, 9, 4, 5, {0, 1, 4, 5, 8, 9, 10, 15, 0, 0, 0, 0, 0, 0, 0, 0}},
    {1.7647001f, 7, 4, 3, {0, 1, 4, 5, 8, 10, 15, 0, 0, 0, 0, 0, 0, 0, 0, 0}},
    {1.7222000f, 6, 4, 2, {0, 1, 4, 5, 8, 15, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}},
    {1.6667000f, 4, 4, 0, {0, 1, 4, 5, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}},
    {1.6111000f, 3, 3, 0, {0, 1, 5, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}},
    {1.5f, 2, 2, 0, {1, 5, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}},
    {1.3f, 1, 1, 0, {1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}},
};

static const float s_kbAiTrickLength[16] = {      // 0x0056cc60 clip lengths (s)
    1.6111f, 1.3f, 2.2666f, 2.3333f, 1.6667f, 1.5f, 2.3333f, 2.4f,
    1.7222f, 1.8f, 1.7647f, 2.3333f, 2.1333f, 2.1333f, 1.9375f, 1.7222f,
};

// Retail's inline-asm fistp helper gives this function an EBP frame; the
// pragma keeps the frame pointer for it alone (docs/GAME_BUILD.md#frame-pointer-units).
#pragma optimize("y", off)
int KrustyBike::Fn_0048D780(int* out, float t, int group)
{
    int row = -1;
    int i;
    for (i = 0; i < 12; i++) {
        if (t > s_kbAiTrickRows[i].minTime) {
            row = i;
            break;
        }
    }
    if (row < 0)
        return 0;
    int first = 0;
    int count;
    switch (group) {
    case 0:
        count = s_kbAiTrickRows[row].allCount;
        break;
    case 1:
        count = s_kbAiTrickRows[row].firstCount;
        break;
    case 2:
        if (s_kbAiTrickRows[row].secondCount == 0) {
            count = s_kbAiTrickRows[row].firstCount;
        } else {
            first = s_kbAiTrickRows[row].firstCount;
            count = s_kbAiTrickRows[row].secondCount;
        }
        break;
    default:
        return 0;
    }
    int index;
    float pick = rand() * (1.0f / 32768.0f);
    FistpFloatToInt(&index, pick * count - 0.5f);
    int trick = s_kbAiTrickRows[row].tricks[index + first];
    *out = trick;
    if (aiKind) {
        float slack = t - s_kbAiTrickLength[trick];
        if (slack < 0.1f)
            return 0;
        float hold = rand() * (1.0f / 32768.0f);
        aiTrickHoldLeft = hold * slack * 0.8f;
    }
    return 1;
}
#pragma optimize("", on)
