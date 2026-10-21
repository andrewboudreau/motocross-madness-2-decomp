// Link the actual provider; main observes storage after normal CRT startup.
#include "../../src/reconstructed/BikeRace.h"
#include <stddef.h>
#include <stdio.h>
#include <string.h>

typedef char VectorSize[(sizeof(Vector3) == 12) ? 1 : -1];
typedef char VectorOffsets[(offsetof(Vector3, x) == 0 && offsetof(Vector3, y) == 4 && offsetof(Vector3, z) == 8) ? 1 : -1];
typedef char Size_g_ObstacleDebugLeftLine[(sizeof(g_ObstacleDebugLeftLine) == 24) ? 1 : -1];
typedef char Size_g_ObstacleDebugRightLine[(sizeof(g_ObstacleDebugRightLine) == 24) ? 1 : -1];
typedef char Size_g_ObstacleDebugLine[(sizeof(g_ObstacleDebugLine) == 24) ? 1 : -1];
typedef char Size_g_ObstacleDebugLowerLine[(sizeof(g_ObstacleDebugLowerLine) == 24) ? 1 : -1];
typedef char Size_g_PathTargetDebugLine[(sizeof(g_PathTargetDebugLine) == 24) ? 1 : -1];
typedef char Size_g_ObstacleDebugUpperLine[(sizeof(g_ObstacleDebugUpperLine) == 24) ? 1 : -1];

int main() {
    Vector3* lines[] = {g_ObstacleDebugLeftLine, g_ObstacleDebugRightLine, g_ObstacleDebugLine, g_ObstacleDebugLowerLine, g_PathTargetDebugLine, g_ObstacleDebugUpperLine};
    int* scalars[] = {&g_ObstacleDebugUpperHit, &g_ObstacleDebugHit, &g_ObstacleDebugLowerHit, &g_ObstacleDebugRightHit, &g_ObstacleDebugLeftHit, &g_LodDebugSelectedRow};
    unsigned char* cells[42];
    unsigned long expected[42];
    unsigned long patterns[] = {0, 0x80000000UL, 0x3f800000UL, 0xbf800000UL,
                               0x7f800000UL, 0x7fc12345UL, 0xffffffffUL, 0x01234567UL};
    int n = 0;
    int i, j, k;
    for (i = 0; i < 6; ++i) {
        if ((unsigned char*)&lines[i][1] - (unsigned char*)&lines[i][0] != 12) return 1;
        for (j = 0; j < 2; ++j) {
            cells[n++] = (unsigned char*)&lines[i][j].x;
            cells[n++] = (unsigned char*)&lines[i][j].y;
            cells[n++] = (unsigned char*)&lines[i][j].z;
        }
    }
    for (i = 0; i < 6; ++i) cells[n++] = (unsigned char*)scalars[i];
    if (n != 42) return 2;
    for (i = 0; i < n; ++i) {
        unsigned long value;
        memcpy(&value, cells[i], 4);
        expected[i] = (i == 41) ? 2 : 0;
        if (value != expected[i]) return 3;
        for (j = 0; j < i; ++j) {
            unsigned long a = (unsigned long)cells[i], b = (unsigned long)cells[j];
            if (a < b + 4 && b < a + 4) return 4;
        }
    }
    // Change one real storage word at a time, checking every other word too.
    // memcpy preserves all payload bits, including signed zero and NaN values.
    for (k = 0; k < 8; ++k) {
        for (i = 0; i < n; ++i) {
            expected[i] = patterns[k] ^ (unsigned long)(i * 0x01010101UL);
            memcpy(cells[i], &expected[i], 4);
            for (j = 0; j < n; ++j) {
                unsigned long value;
                memcpy(&value, cells[j], 4);
                if (value != expected[j]) return 5;
            }
        }
    }
    puts("Race debug storage: CRT initial state and 336 independent mutations passed");
    return 0;
}
