#include <float.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "Net.h"
#include "PCAudio.h"
#include "TextureMap.h"
#include "Tgafile.h"
#include "SceneManager.h"

// 0x004e9980: looks `name` up in a keyword table ended by an empty name.
// Only SceneManager.cpp code (0x004eb602) calls it.
int UnknownFunction4e9980(int* out, const char* name, const UnknownSceneKeyword* table)
{
    const UnknownSceneKeyword* entry = table;
    while (*entry->name != '\0') {
        if (_stricmp(name, entry->name) == 0) {
            *out = entry->value;
            return 1;
        }
        entry++;
    }
    return 0;
}

// 0x004e99d0
UnknownTrackGameObject574::UnknownTrackGameObject574()
{
    field_0x44[0] = 0;
    field_0x24c[0] = 0;
    field_0x2cc[0] = 0;
    field_0x30c[0] = 0;
    field_0x34c[0] = 0;
    field_0x148[0] = 0;
    field_0x38c = 0;
    field_0x00 = 0;
    field_0x04[0] = 0;
    field_0x390 = 0;
}

// 0x004e9a10
int UnknownTrackGameObject574::UnknownFunction4e9a10(int* flag)
{
    Scene* scene = (new(__FILE__, 115) Scene(0))->UnknownFunction4ea7e0(this, 0, 0, 0);
    if (!scene)
        return 0;
    scene->UnknownFunction4f0d20(flag);
    scene->Release();
    return 1;
}

// 0x004e9ac0
int UnknownTrackGameObject574::UnknownFunction4e9ac0(unsigned long* info, char* a, char* b, int c)
{
    info[0] = 0;
    info[1] = 0;
    info[2] = 0;
    info[3] = 0;
    Scene* scene = (new(__FILE__, 151) Scene(0))->UnknownFunction4ea7e0(this, 0, 0, 0);
    if (!scene)
        return 0;
    int result = scene->UnknownFunction4f1130(info, a, b, c);
    scene->Release();
    return result;
}

// 0x004e9b80
void UnknownTrackGameObject574::UnknownFunction4e9b80(char* path)
{
    strncpy(field_0x44, path, 0x103);
}

// 0x004e9ba0: keeps the open archive when it already is `name`.
UnknownTextureStream* UnknownTrackGameObject574::UnknownFunction4e9ba0(const char* name)
{
    char scene[0x40];
    char path[0x104];

    if (name)
        strncpy(scene, name, 0x3f);
    else
        strncpy(scene, field_0x24c, 0x3f);
    if (field_0x00 && !_stricmp(field_0x04, scene))
        return field_0x00;
    sprintf(path, "%s\\%s", field_0x44, scene);
    strcpy(strrchr(path, '.'), ".env");
    if (field_0x00) {
        g_UnknownResourceManager572b44->UnknownFunction4e9830(field_0x00);
        delete field_0x00;
    }
    field_0x00 = g_UnknownResourceManager572b44->UnknownFunction4e9030(path, 1);
    if (!field_0x00) {
        field_0x04[0] = 0;
        return 0;
    }
    int length = strlen(scene);
    int count = length > 0x3f ? 0x3f : length;
    strncpy(field_0x04, scene, count);
    field_0x04[count] = 0;
    return field_0x00;
}

// 0x004e9e30: selects scene `name`.`kind` and reads whether it is named.
void UnknownTrackGameObject574::UnknownFunction4e9e30(char* name, const char* kind, int value)
{
    char sceneName[0x40];
    char buffer[0x104];

    sprintf(buffer, "%s.%s", name, kind);
    strncpy(field_0x24c, buffer, 0x3f);
    UnknownFunction4e9f70(name);
    UnknownFunction4e9ba0(0);
    UnknownTextureStream* stream =
        new(__FILE__, 297) UnknownTextureStream((int)g_UnknownResourceManager572b44);
    if (stream->UnknownFunction460f50(field_0x24c, "r", 0)) {
        UnknownFunction4ea390(sceneName, field_0x24c, value);
        if (value && !_stricmp(sceneName, "NO NAME"))
            field_0x38c = 0;
        else
            field_0x38c = 1;
    } else {
        field_0x38c = 0;
    }
    delete stream;
}

// 0x004e9f70
void UnknownTrackGameObject574::UnknownFunction4e9f70(const char* name)
{
    char buffer[0x104];

    sprintf(buffer, "%s.trn", name);
    strncpy(field_0x2cc, buffer, 0x1f);
    strncpy(field_0x30c, "", 0x1f);
    sprintf(buffer, "%s01.wpt", name);
    strncpy(field_0x34c, buffer, 0x1f);
    sprintf(buffer, "%s.est", name);
    strncpy(field_0x28c, buffer, 0x1f);
}

// 0x004ea010: as 0x004ea390 for scene `name` (with extension `kind` when
// it has none), opened through 0x004e9ba0.
void UnknownTrackGameObject574::UnknownFunction4ea010(char* out, char* name, int index,
                                                      const char* kind, int* a, int* b)
{
    char path[0x104];
    char scene[0x104];
    char base[0x104];

    if (a)
        *a = 0;
    if (b)
        *b = 0;
    if (field_0x44[0]) {
        if (!strrchr(name, '.'))
            sprintf(scene, "%s.%s", name, kind);
        else
            strncpy(scene, name, 0x3f);
        sprintf(path, "%s\\%s", field_0x44, scene);
        UnknownParameterBlock block;
        UnknownTextureStream* archive = UnknownFunction4e9ba0(scene);
        if (archive) {
            archive->UnknownFunction461cb0(a, b);
            block.UnknownFunction4b7220((UnknownParameterStream*)archive, scene, path, 0);
            block.UnknownFunction4b78f0("SceneInfo");
            block.UnknownFunction4b7f10("NumberOfWaypointRaces", 0, &field_0x390);
            if (index) {
                if (field_0x390) {
                    UnknownTextureStream* stream = new(__FILE__, 401)
                        UnknownTextureStream((int)g_UnknownResourceManager572b44);
                    strcpy(base, scene);
                    strcpy(strrchr(base, '.'), "");
                    sprintf(path, "%s%02d.wpt", base, index);
                    strncpy(field_0x34c, path, 0x3f);
                    if (stream->UnknownFunction460f50(field_0x34c, "r", 0)) {
                        sprintf(field_0x148, "%s\\%s", field_0x44, field_0x34c);
                        sprintf(path, "WaypointRaceName%d", index);
                        if (!block.UnknownFunction4b7ec0(path, "", out, 0x3f))
                            strncpy(out, "NO NAME", 0x3f);
                    } else {
                        field_0x148[0] = 0;
                        strncpy(out, "NO NAME", 0x3f);
                    }
                    delete stream;
                } else {
                    field_0x148[0] = 0;
                    strncpy(out, "NO NAME", 0x3f);
                }
            } else {
                if (!block.UnknownFunction4b7ec0("SceneName", "", out, 0x3f))
                    strncpy(out, "NO NAME", 0x3f);
            }
        } else {
            strncpy(out, "NO NAME", 0x3f);
        }
    } else {
        strncpy(out, "NO NAME", 0x3f);
    }
}

// 0x004ea390: reads the scene name (value 0) or waypoint race `value`'s
// name from the scene's SceneInfo section; "NO NAME" when missing. The
// archive is passed through Parameterblocks.h's view of the stream class.
void UnknownTrackGameObject574::UnknownFunction4ea390(char* name, char* scene, int value)
{
    char path[0x104];
    char base[0x104];

    if (field_0x44[0]) {
        sprintf(path, "%s\\%s", field_0x44, scene);
        UnknownParameterBlock block;
        block.UnknownFunction4b7220((UnknownParameterStream*)UnknownFunction4e9ba0(0), scene, path, 1);
        block.UnknownFunction4b78f0("SceneInfo");
        block.UnknownFunction4b7f10("NumberOfWaypointRaces", 0, &field_0x390);
        if (value) {
            if (field_0x390) {
                UnknownTextureStream* stream =
                    new(__FILE__, 471) UnknownTextureStream((int)g_UnknownResourceManager572b44);
                strcpy(base, scene);
                strcpy(strrchr(base, '.'), "");
                sprintf(path, "%s%02d.wpt", base, value);
                strncpy(field_0x34c, path, 0x3f);
                if (stream->UnknownFunction460f50(field_0x34c, "r", 0)) {
                    sprintf(field_0x148, "%s\\%s", field_0x44, field_0x34c);
                    sprintf(path, "WaypointRaceName%d", value);
                    if (!block.UnknownFunction4b7ec0(path, "", name, 0x3f))
                        strncpy(name, "NO NAME", 0x3f);
                } else {
                    field_0x148[0] = 0;
                    strncpy(name, "NO NAME", 0x3f);
                }
                delete stream;
            } else {
                field_0x148[0] = 0;
                strncpy(name, "NO NAME", 0x3f);
            }
        } else {
            if (!block.UnknownFunction4b7ec0("SceneName", "", name, 0x3f))
                strncpy(name, "NO NAME", 0x3f);
        }
    } else {
        strncpy(name, "NO NAME", 0x3f);
    }
}

// 0x004ea660
Scene::Scene(int flags) : GameObject(flags)
{
    field_0x70 = Vector3(0.0f, 0.0f, 0.0f);
    field_0xa8 = 0;
    field_0xac = 0;
    field_0xb0 = 0;
    field_0xa4 = 0;
    field_0xb4 = 0;
    field_0xb8 = 0;
    field_0x30[0] = 0;
    field_0x7c8[0] = 0;
    field_0x7c = Vector3(0.0f, 0.0f, 0.0f);
    field_0xbc = 0;
    field_0xc8.Init(4, 4);
    field_0xc0 = 0;
    field_0xc4 = 0;
    field_0x88 = 0;
    field_0x6bc[0] = 0;
    field_0x7c4 = 0;
    field_0x7bc = 1;
    field_0x7c0 = 0;
}

// 0x004ea7e0
Scene* Scene::UnknownFunction4ea7e0(UnknownTrackGameObject574* manager, int value, int,
                                    char flag)
{
    field_0x2c = manager;
    field_0xc0 = value;
    field_0x88 = flag;
    field_0xc4 = new(__FILE__, 622) SoundGroup(field_0x25_bit0);
    UnknownFunction469190(field_0xc4, -1);
    return this;
}

// 0x004ea880
Scene::~Scene()
{
    if (field_0x7c4)
        fclose(field_0x7c4);
    if (field_0xac)
        delete field_0xac;
    if (field_0xb0)
        delete field_0xb0;
    if (field_0xa4)
        delete field_0xa4;
    if (field_0xb4) {
        if (field_0xb4->field_0x00 > 0) {
            for (int i = 0; i < field_0xb4->field_0x00; i++) {
                if (field_0xb4->field_0x04[i].field_0x00_bit3 &&
                    field_0xb4->field_0x04[i].field_0x1c > 0) {
                    if (field_0xb4->field_0x04[i].field_0x20)
                        delete field_0xb4->field_0x04[i].field_0x20;
                    if (field_0xb4->field_0x04[i].field_0x28)
                        delete field_0xb4->field_0x04[i].field_0x28;
                }
            }
            delete field_0xb4->field_0x04;
        }
        if (field_0xb4->field_0x08 > 0) {
            for (int i = 0; i < field_0xb4->field_0x08; i++) {
                if (field_0xb4->field_0x0c[i].field_0x00 > 0) {
                    if (field_0xb4->field_0x0c[i].field_0x08)
                        delete field_0xb4->field_0x0c[i].field_0x08;
                    if (field_0xb4->field_0x0c[i].field_0x0c)
                        delete field_0xb4->field_0x0c[i].field_0x0c;
                }
            }
            delete field_0xb4->field_0x0c;
        }
        delete field_0xb4;
    }
    if (field_0xb8) {
        delete field_0xb8->field_0x04;
        delete field_0xb8;
    }
}

// 0x004eaa60: plays every listed sound, then the GameObject slot.
void Scene::UnknownVirtualSlot5()
{
    for (int i = 0; i < field_0xc8.m_count; i++) {
        Sound* sound = field_0xc8.Get(i);
        if (sound)
            sound->UnknownFunction4bc6b0(0, 1, 0);
    }
    GameObject::UnknownVirtualSlot5();
}

// 0x004eaab0: as slot 5, before the GameObject slot 7.
void Scene::UnknownVirtualSlot7()
{
    for (int i = 0; i < field_0xc8.m_count; i++) {
        Sound* sound = field_0xc8.Get(i);
        if (sound)
            sound->UnknownFunction4bc6b0(0, 1, 0);
    }
    GameObject::UnknownVirtualSlot7();
}

// 0x004eaec0
UnknownSceneObject* Scene::UnknownFunction4eaec0(const char* path, int* index)
{
    char name[0x104];

    if (!field_0xb4) {
        if (index)
            *index = 0;
        return 0;
    }
    _splitpath(path, 0, 0, name, 0);
    for (int i = 0; i < field_0xb4->field_0x00; i++) {
        if (field_0xb4->field_0x04[i].field_0x00_bit3 &&
            !_strnicmp(field_0xb4->field_0x04[i].field_0x04->field_0x1a4, name,
                       strcspn(field_0xb4->field_0x04[i].field_0x04->field_0x1a4, "."))) {
            if (index)
                *index = i;
            return field_0xb4->field_0x04[i].field_0x04;
        }
    }
    if (index)
        *index = 0;
    return 0;
}

// 0x004eafd0
void Scene::UnknownFunction4eafd0(int index)
{
    if (field_0xb4 && index < field_0xb4->field_0x00)
        field_0xb4->field_0x04[index].field_0x00_bit2 = 1;
}

// 0x004eb000
void Scene::UnknownFunction4eb000(float time)
{
    if (field_0xb4) {
        for (int i = 0; i < field_0xb4->field_0x00; i++)
            UnknownFunction4eb040(i, time, 1, 0);
    }
}

// 0x004eb160
int Scene::UnknownFunction4eb160(Vector3* out, const char* section, const char* key,
                                 const char* def, int useDefault, UnknownParameterBlock* block)
{
    char value[0x184];

    if (!block)
        block = &field_0xdc;
    block->UnknownFunction4b78f0(section);
    if (!field_0x2c->field_0x38c) {
        block->UnknownFunction4b7ec0(key, def, value, 0x80);
        UnknownFunction4de580(section, key, value);
        out->x = (float)atof(strtok(value, ","));
        out->y = (float)atof(strtok(0, ","));
        out->z = (float)atof(strtok(0, "\n"));
        return 1;
    }
    if (block->UnknownFunction4b7ec0(key, "", value, 0x80)) {
        UnknownFunction4de580(section, key, value);
        out->x = (float)atof(strtok(value, ","));
        out->y = (float)atof(strtok(0, ","));
        out->z = (float)atof(strtok(0, "\n"));
        return 1;
    }
    if (useDefault && *def) {
        strcpy(value, def);
        UnknownFunction4de580(section, key, value);
        out->x = (float)atof(strtok(value, ","));
        out->y = (float)atof(strtok(0, ","));
        out->z = (float)atof(strtok(0, "\n"));
    }
    return 0;
}

// 0x004eb300
int Scene::UnknownFunction4eb300(unsigned long* out, const char* section, const char* key,
                                 const char* def, int, UnknownParameterBlock* block)
{
    char value[0x184];

    if (!block)
        block = &field_0xdc;
    block->UnknownFunction4b78f0(section);
    if (!field_0x2c->field_0x38c) {
        block->UnknownFunction4b7ec0(key, def, value, 0x80);
        UnknownFunction4de580(section, key, value);
        int r = atoi(strtok(value, ","));
        int g = atoi(strtok(0, ","));
        int b = atoi(strtok(0, "\n"));
        *out = ((r << 8 | g) << 8) | b;
        return 1;
    }
    if (block->UnknownFunction4b7ec0(key, def, value, 0x80)) {
        UnknownFunction4de580(section, key, value);
        int r = atoi(strtok(value, ","));
        int g = atoi(strtok(0, ","));
        int b = atoi(strtok(0, "\n"));
        *out = ((r << 8 | g) << 8) | b;
        return 1;
    }
    *out = 0;
    return 0;
}

// 0x004eb480
int Scene::UnknownFunction4eb480(char* out, int count, const char* section, const char* key,
                                 const char* def, int, UnknownParameterBlock* block)
{
    char value[0x184];

    if (!block)
        block = &field_0xdc;
    block->UnknownFunction4b78f0(section);
    if (block->UnknownFunction4b7ec0(key, def, value, 0x80)) {
        char* token = strtok(value, ",");
        for (int i = 0; i < count; i++) {
            out[i] = 0;
            if (token && (*token == 'Y' || *token == 'y' || *token == 'T' || *token == 't'
                          || *token == '1'))
                out[i] = 1;
            token = strtok(0, ",");
        }
        return 1;
    }
    if (count > 0)
        memset(out, 0, count);
    return 0;
}

// 0x004ebdb0: creates field_0xb0 from the "Fog" section. Retail range-checks
// the visibility value again where it reads the haziness.
int Scene::UnknownFunction4ebdb0()
{
    float visibility;
    float haziness;

    field_0xb0 = new(__FILE__, 1392) UnknownSceneFog;
    field_0xb0->color = 0x8080c0;
    field_0xb0->visibility = 1.0f;
    field_0xb0->haziness = 0.5f;
    int found = field_0xdc.UnknownFunction4b78f0("Fog");
    if (!field_0x2c->field_0x38c || !found) {
        UnknownFunction464e80("No Scene File or no fog data:  Using DEFAULT Fog\n");
        return 1;
    }
    visibility = 1.0f;
    if (!field_0xdc.UnknownFunction4b7f40("Visibility", 1.0f, &visibility)) {
        UnknownFunction4de580("Fog", "Visibility", "NOT FOUND using default 1.0");
    } else if (visibility < 0.0f || visibility > 1.0f) {
        UnknownFunction4de580("Fog", "Visibility",
                              "Invalid value, must be between 0.0 and 1.0, using default 1.0");
        visibility = 1.0f;
    }
    field_0xb0->visibility = visibility;
    UnknownFunction4de580("Fog", "Visibility", field_0xb0->visibility);
    if (!field_0xdc.UnknownFunction4b7f40("Haziness", 0.5f, &haziness)) {
        UnknownFunction4de580("Fog", "Haziness", "NOT FOUND using default 0.5");
    } else if (visibility < 0.0f || visibility > 1.0f) {
        UnknownFunction4de580("Fog", "Haziness",
                              "Invalid value, must be between 0.0 and 1.0, using default 0.5");
        haziness = 0.5f;
    }
    field_0xb0->haziness = haziness;
    UnknownFunction4de580("Fog", "Haziness", field_0xb0->haziness);
    if (!UnknownFunction4eb300(&field_0xb0->color, "Fog", "ColorRGB", "", 1, 0))
        return 0;
    UnknownFunction464e80("");
    return 1;
}

// 0x004eca20
int Scene::UnknownFunction4eca20()
{
    char key[0x40];
    char file[0x80];
    char name[0x104];
    char path[0x104];
    char message[0x204];

    int found = field_0xdc.UnknownFunction4b78f0("ResourceFiles");
    if (field_0x2c->field_0x38c && found) {
        int i = 1;
        sprintf(key, "ResourceFile%d", i);
        while (field_0xdc.UnknownFunction4b7ec0(key, "", file, 0x7f)) {
            sprintf(name, "%s\\%s", "Res", file);
            if (!g_UnknownGlobal56e26c->UnknownVirtualSlot18(name, path))
                return 0;
            g_UnknownResourceManager572b44->UnknownFunction4e9030(path, 0);
            sprintf(key, "ResourceFile%d", ++i);
        }
        return 1;
    }
    sprintf(message, "\nCannot find data for [%s] in %s", "ResourceFiles", field_0x7c8);
    UnknownFunction464e80(message);
    return 0;
}

// The four per-TU vector constants seen in other TUs (TrackOverlay.cpp). Their
// dynamic initializers sit at 0x004ecb70..0x004ecd5c; retail places the y and
// z ones after 0x004ecc10.
static const Vector3 s_UnknownVector689ca8 = Vector3(0.0f, 0.0f, 0.0f);
static const Vector3 s_UnknownVector689cb8 = Vector3(1.0f, 0.0f, 0.0f);
static const Vector3 s_UnknownVector689cc8 = Vector3(0.0f, 1.0f, 0.0f);
static const Vector3 s_UnknownVector689c98 = Vector3(0.0f, 0.0f, 1.0f);

// 0x004ecc10
void Scene::UnknownFunction4ecc10()
{
    field_0x6a0_bit0 = field_0xdc.UnknownFunction4b78f0("Stadium");
    if (field_0x6a0_bit0) {
        field_0xdc.UnknownFunction4b7ec0("FileName", "", field_0x6bc, -1);
        field_0xdc.UnknownFunction4b7f40("Top", 0, &field_0x6a4);
        field_0xdc.UnknownFunction4b7f40("Bottom", 0, &field_0x6a8);
        field_0xdc.UnknownFunction4b7f40("Scale", 0, &field_0x6ac);
        UnknownFunction4eb160(&field_0x6b0, "Stadium", "Position", "0.0,0.0,0.0", 0, 0);
    }
}

// 0x004edf20: attaches a new shadow receiver for `shadow` to each caster and
// adds it as a child.
void Scene::UnknownFunction4edf20(ProjectedShadow* shadow, int flags)
{
    if (shadow && field_0xb8) {
        for (int i = 0; i < field_0xb8->field_0x00; i++)
            UnknownFunction469190((new(__FILE__, 2136) D3DIMSoultreeShadow(flags))
                                      ->Attach((int)field_0x18, field_0xb8->field_0x04[i].field_0x04,
                                               shadow),
                                  -1);
    }
}

// 0x004ef9c0: reads a sound's settings from `section` and appends ".wav" to
// `file` when it has no extension yet.
void Scene::UnknownFunction4ef9c0(const char* section, char* file, int is3D,
                                  UnknownSound3DParameters* params, unsigned long* flags,
                                  float* oneShotDistance, float* randomTriggerPercent,
                                  int* oneShot, int* force2D)
{
    memset(params, 0, sizeof(UnknownSound3DParameters));
    if (!strstr(file, ".wav"))
        strcat(file, ".wav");
    field_0xdc.UnknownFunction4b78f0(section);
    *flags = is3D ? 2 : 0;
    int looping;
    field_0xdc.UnknownFunction4b7f10("SoundLooping", 0, &looping);
    if (looping)
        *flags |= 8;
    field_0xdc.UnknownFunction4b7f10("SoundOneShot", 0, oneShot);
    if (*oneShot)
        *flags |= 1;
    field_0xdc.UnknownFunction4b7f10("SoundForce2D", 0, force2D);
    field_0xdc.UnknownFunction4b7f40("SoundOneShotDistance", 20.0f, oneShotDistance);
    field_0xdc.UnknownFunction4b7f40("SoundRandomTriggerPercent", 100.0f, randomTriggerPercent);
    field_0xdc.UnknownFunction4b7f40("SoundMaxDistance", 0, &params->maxDistance);
    field_0xdc.UnknownFunction4b7f40("SoundMinDistance", 0, &params->minDistance);
    params->position = Vector3(FLT_MAX, FLT_MAX, FLT_MAX);
}

// 0x004f0040
int Scene::UnknownVirtualSlot14()
{
    if (field_0x7bc)
        return GameObject::UnknownVirtualSlot14();
    return 1;
}

// 0x004f0060: control 5 toggles field_0x7bc; other events go to GameObject.
int Scene::UnknownVirtualSlot22(UnknownControlEvent* event, UnknownInputEntry* entry)
{
    if (UnknownFunction43caa0(5, 0, event, 0x80)) {
        field_0x7bc = 1 - field_0x7bc;
        return 1;
    }
    return GameObject::UnknownVirtualSlot22(event, entry) != 0;
}

// 0x004f00c0
int Scene::UnknownVirtualSlot23(UnknownControlEvent* event, UnknownInputEntry* entry)
{
    return GameObject::UnknownVirtualSlot23(event, entry) != 0;
}

// 0x004f00e0
int UnknownFunction4f00e0(UnknownSceneTextureInfo* texture, int* counts)
{
    if (!_stricmp(texture->field_0x2c, "PROCEDURAL")) {
        counts[2]++;
        return 1;
    }
    int header0;
    int header1;
    int width = 0;
    UnknownResourceEntry* entry =
        g_UnknownResourceManager572b44->UnknownFunction4e9360(texture->field_0x2c, 0);
    if (entry) {
        entry->field_0x14->UnknownFunction461340(entry->field_0x18, 0, 0);
        if (entry->field_0x14->UnknownFunction461640(&header0, 4, 1) != 1
            || entry->field_0x14->UnknownFunction461640(&header1, 4, 1) != 1
            || entry->field_0x14->UnknownFunction461640(&width, 4, 1) != 1)
            return 0;
    } else {
        UnknownTgaFile* file = UnknownFunction511d00(texture->field_0x2c, 0,
                                                     (int)g_UnknownResourceManager572b44);
        if (!file)
            return 0;
        width = file->width;
        UnknownFunction512dd0(file);
    }
    switch (width) {
    case 0x100:
        counts[0]++;
        break;
    case 0x80:
        counts[1]++;
        break;
    case 0x40:
        counts[2]++;
        break;
    case 0x20:
        counts[3]++;
        break;
    }
    return 1;
}

// 0x004f0310: registers `name` with `manager`, taking it over from the
// global manager when that has it, else loading it from `path`; 0 when
// `manager` already has it.
int UnknownFunction4f0310(UnknownResourceManager* manager, const char* name, const char* path)
{
    UnknownResourceEntry* global = g_UnknownResourceManager572b44->UnknownFunction4e9360(name, 0);
    if (!manager->UnknownFunction4e9360(name, 0)) {
        if (global)
            manager->UnknownFunction4e96b0(name, 0);
        else
            manager->UnknownFunction4e9430(name, path);
        UnknownResourceEntry* entry = manager->UnknownFunction4e9360(name, 0);
        if (entry)
            delete entry->field_0x14;
        return 1;
    }
    return 0;
}

// 0x004f0d20: loads the scene file into field_0xdc and sets *count to its
// static model plus animation count (at least 1).
int Scene::UnknownFunction4f0d20(int* count)
{
    UnknownTextureStream* archive = field_0x2c->UnknownFunction4e9ba0(0);
    if (!archive)
        return 0;
    int animations;
    UnknownSceneResourceManager resources;
    char message[0x204];
    sprintf(field_0x7c8, "%s\\%s", field_0x2c->field_0x44, field_0x2c->field_0x24c);
    field_0xdc.UnknownFunction4b7220((UnknownParameterStream*)archive, field_0x2c->field_0x24c,
                                     field_0x7c8, 1);
    *count = 1;
    int found = field_0xdc.UnknownFunction4b78f0("StaticModels");
    if (!field_0x2c->field_0x38c || !found) {
        sprintf(message, "\nNo scene file OR cannot find data for [%s] in %s.\n\n",
                "StaticModels", field_0x7c8);
        UnknownFunction464e80(message);
    }
    if (!field_0xdc.UnknownFunction4b7f10("NumberOfStaticModels", 1, count)) {
        sprintf(message, "\nNumberOfStaticModels cannot be found under [%s] in %s.\n\n",
                "StaticModels", field_0x7c8);
        UnknownFunction464e80(message);
    }
    animations = 0;
    if (!field_0xdc.UnknownFunction4b7f10("NumberOfAnimations", 0, &animations)) {
        sprintf(message, "\nNumberOfAnimations cannot be found under [%s] in %s.\n\n",
                "Animations", field_0x7c8);
        UnknownFunction464e80(message);
    }
    *count += animations;
    *count = *count > 1 ? *count : 1;
    return 1;
}

// 0x004f0ec0: replaces `path` (a model file) with the SLT file its "General
// info" section names, prefixed with its content directory.
int Scene::UnknownFunction4f0ec0(char* path)
{
    char drive[_MAX_DRIVE];
    char name[0x104];
    char sltFile[0x108];
    char ext[_MAX_EXT];
    char content[0x108];

    UnknownTextureStream* stream =
        new(__FILE__, 3727) UnknownTextureStream((int)g_UnknownResourceManager572b44);
    char dir[_MAX_DIR];
    _splitpath(path, drive, dir, name, ext);
    strcat(name, ext);
    if (!g_UnknownResourceManager572b44->UnknownFunction4e9360(name, 0)) {
        g_UnknownResourceManager572b44->UnknownFunction4e9430(name, path);
        if (!g_UnknownResourceManager572b44->UnknownFunction4e9360(name, 0)) {
            delete stream;
            return 0;
        }
    }
    stream->UnknownFunction460f50(name, "rb", 0);
    UnknownParameterBlock block;
    block.UnknownFunction4b77a0((UnknownParameterStream*)stream, 0, 1);
    block.UnknownFunction4b78f0("General info");
    if (!block.UnknownFunction4b7b30("SLTFile", sltFile, -1)) {
        delete stream;
        return 0;
    }
    block.UnknownFunction4b7ec0("ContentDirectory", "", content, -1);
    strcpy(path, content);
    strcat(path, sltFile);
    delete stream;
    return 1;
}
