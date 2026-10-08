// GameUiHelpers.cpp -- the helpers at 0x0047b670..0x0047bb20, see
// GameUiHelpers.h for the evidence.

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <windows.h>

#include "GameUiHelpers.h"

#include "BikeRace.h"

// 0x0047b670
int UnknownFunction47b670(Vector3* from, Vector3* to, UnknownBikeRaceNode* gate, int direction)
{
    float u;
    float t;
    float dy;
    float p0x, p0y, d0x, d0y, p1x, p1y, d1x, d1y;

    if (!from || !to || !gate) {
        return 0;
    }
    if (to->x == 0.0f && to->y == 0.0f && to->z == 0.0f) {
        return 0;
    }
    p0x = to->x;
    p0y = to->z;
    d0x = from->x - to->x;
    d0y = from->z - to->z;
    p1x = gate->field_0x00.x - gate->field_0x18.x;
    p1y = gate->field_0x00.z - gate->field_0x18.z;
    d1x = gate->field_0x18.x * 2.0f;
    d1y = gate->field_0x18.z * 2.0f;
    if (!UnknownFunction47b800(p0x, p0y, d0x, d0y, p1x, p1y, d1x, d1y, &t, &u)) {
        return 0;
    }
    if (t < 0.0f || t > 1.0f) {
        return 0;
    }
    if (u < 0.0f || u > 1.0f) {
        return 0;
    }
    dy = from->y - to->y;
    // +0x28 holds the gate's height as a float (BikeRace.h declares it int
    // because 0x0041d0d0 copies it from an int argument).
    if (dy * t + to->y - gate->field_0x00.y > *(float*)&gate->field_0x28) {
        return 0;
    }
    if (direction) {
        if ((from->z - to->z) * gate->field_0x0c.z + (from->x - to->x) * gate->field_0x0c.x +
                dy * gate->field_0x0c.y < 0.0f) {
            return 0;
        }
    }
    return 1;
}

// 0x0047b800
int UnknownFunction47b800(float p0x, float p0y, float d0x, float d0y,
                          float p1x, float p1y, float d1x, float d1y,
                          float* t, float* u)
{
    if (!t || !u) {
        return 0;
    }
    *t = d0y * d1x - d0x * d1y;
    if (*t == 0.0f) {
        return 0;
    }
    *t = ((p1y - p0y) * d1x + (p0x - p1x) * d1y) / *t;
    if (d1x != 0.0f) {
        *u = (*t * d0x + p0x - p1x) / d1x;
    } else {
        *u = (*t * d0y + p0y - p1y) / d1y;
    }
    return 1;
}

// 0x0047b8a0
float UnknownFunction47b8a0(const char* section, const char* key, double defaultValue,
                            const char* path)
{
    char text[0x100];

    GetPrivateProfileStringA(section, key, "NONE", text, sizeof(text), path);
    if (strcmp(text, "NONE") == 0) {
        return defaultValue;
    }
    return atof(text);
}

// 0x0047b930
void UnknownFunction47b930(const char* section, const char* key, const char* defaultValue,
                           char* buffer, int size, const char* file)
{
    char directory[200];
    char path[260];

    if (!strstr(file, "\\")) {
        GetCurrentDirectoryA(260, directory);
        sprintf(path, "%s\\%s", directory, file);
    } else {
        strcpy(path, file);
    }
    GetPrivateProfileStringA(section, key, defaultValue, buffer, size, path);
}

// 0x0047b9e0
int UnknownFunction47b9e0(const char* section, const char* key, int defaultValue,
                          const char* file)
{
    char directory[200];
    char path[260];

    if (!strstr(file, "\\")) {
        GetCurrentDirectoryA(260, directory);
        sprintf(path, "%s\\%s", directory, file);
    } else {
        strcpy(path, file);
    }
    return GetPrivateProfileIntA(section, key, defaultValue, path);
}

// 0x0047ba80
void UnknownFunction47ba80(const char* section, const char* key, const char* value,
                           const char* file)
{
    char directory[200];
    char path[260];

    if (!strstr(file, "\\")) {
        GetCurrentDirectoryA(260, directory);
        sprintf(path, "%s\\%s", directory, file);
    } else {
        strcpy(path, file);
    }
    WritePrivateProfileStringA(section, key, value, path);
}
