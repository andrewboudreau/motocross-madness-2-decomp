#include <float.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "AuralScape.h"
#include "LightEmitter.h"
#include "Net.h"
#include "PCAudio.h"
#include "SoultreeMaterial.h"
#include "TextureMap.h"
#include "Tgafile.h"
#include "SceneManager.h"

float FastInvSqrt(float x); // 0x00460c00



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
    int result = scene->UnknownFunction4f1130(info, (float*)a, (int*)b, (char*)c);
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

// 0x004e9cd0: opens `name`'s file name and extension through `stream`. It
// succeeds only inside an archive (an inner stream at +0x1c), then copies the
// opened name to `a` (a char buffer) when given. Otherwise it frees `stream`
// and allocates a replacement that nothing keeps.
int UnknownTrackGameObject574::UnknownFunction4e9cd0(UnknownTextureStream* stream, const char* name,
                                                     const char* mode, int a)
{
    char drive[_MAX_DRIVE];
    char file[0x104];
    char ext[_MAX_EXT];
    char dir[_MAX_DIR];
    char* out = (char*)a;

    _splitpath(name, drive, dir, file, ext);
    strcat(file, ext);
    if (stream->UnknownFunction460f50(file, mode, 0)) {
        if (stream->field_0x1c) {
            if (out)
                strcpy(out, file);
            return 1;
        }
        delete stream;
        stream = new(__FILE__, 268) UnknownTextureStream((int)g_UnknownResourceManager572b44);
        if (out)
            *out = 0;
    } else {
        if (out)
            *out = 0;
    }
    return 0;
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

// 0x004eb040: restarts entry `index` at `time` (at least 0.0001). A
// character entry (bit 3) switches to its motion number `motion` (0 when out
// of range) unless it is idle and `force` is clear; another entry's object
// gets the time and, unless the time was clamped, one slot 10 step.
int Scene::UnknownFunction4eb040(int index, float time, int force, int motion)
{
    if (field_0xb4) {
        int running = 1;
        if (time < 0.0001f) {
            running = 0;
            time = 0.0001f;
        }
        UnknownSceneEntry* entry = &field_0xb4->field_0x04[index];
        if (entry->field_0x00_bit3) {
            UnknownSceneObject* object = entry->field_0x04;
            if (object->field_0x0c || force) {
                if (motion >= entry->field_0x25)
                    entry->field_0x26 = 0;
                else
                    entry->field_0x26 = motion;
                field_0xb4->field_0x04[index].field_0x24 =
                    field_0xb4->field_0x04[index].field_0x28[field_0xb4->field_0x04[index].field_0x26] - 1;
                object->UnknownFunction4a8b40(
                    field_0xb4->field_0x04[index].field_0x20[field_0xb4->field_0x04[index].field_0x24]);
                object->field_0x10 = 0;
                field_0xb4->field_0x04[index].field_0x18 = object->UnknownFunction4a6bb0(time, 0, 0);
                return 1;
            }
        } else {
            UnknownSceneAnimatedObject* object = (UnknownSceneAnimatedObject*)entry->field_0x08;
            object->field_0x194 = time;
            object->field_0x5c = running;
            if (running)
                ((UnknownSceneAnimatedObject*)field_0xb4->field_0x04[index].field_0x08)
                    ->UnknownVirtualSlot10(0.0001f);
        }
    }
    return 0;
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

// The keywords of a light's "Type" (0x00572c28).
static const UnknownSceneKeyword s_lightTypes[] = {
    {"Undefined", 0}, {"VertexPoint", 1}, {"ObjectPoint", 2}, {"PointNoise", 3},
    {"Directional", 4}, {"Spot", 5}, {"Ambient", 6}, {"AmbientNoise", 7}, {"", 0},
};

// |v|, 0 and 1 exactly.
static inline float Length(const Vector3& v)
{
    float y = v.y;
    float x = v.x;
    float z = v.z;
    float squared = z * z + (x * x + y * y);
    if (squared == 0.0f)
        return 0.0f;
    if (squared == 1.0f)
        return 1.0f;
    return 1.0f / FastInvSqrt(squared);
}

static inline Vector3 operator-(const Vector3& a, const Vector3& b)
{
    Vector3 r(a.x - b.x, a.y - b.y, a.z - b.z);
    return r;
}

// v scaled to unit length (unchanged when it already is).
static inline Vector3 Normalize(const Vector3& v)
{
    float squared = v.z * v.z + (v.y * v.y + v.x * v.x);
    if (squared == 1.0f)
        return v;
    return v * FastInvSqrt(squared);
}

// "T" or "F" for a flag.
static inline const char* TF(char c)
{
    return c ? "T" : "F";
}

// 0x004eb570: reads section "Light<index + 1>" into light `index`. Without
// a scene file or section, light 0 becomes the default ambient light and
// the others a shadow-casting point light far away.
int Scene::UnknownFunction4eb570(int index)
{
    char section[16];
    char text[0x204];
    Vector3 vector;
    int atInfinity;
    int showDebugSphere;
    int castsShadows;
    int emitsLight;
    int hasLensFlare;

    sprintf(section, "Light%d", index + 1);
    int found = field_0xdc.UnknownFunction4b78f0(section);
    if (field_0x2c->field_0x38c && found) {
        UnknownSceneLight* light = &field_0xac[index];
        field_0xdc.UnknownFunction4b7ec0("Type", "BLANK", text, 0x100);
        if (UnknownFunction4e9980(&light->type, text, s_lightTypes)) {
            UnknownFunction4de580(section, "Type", text);
            if (!UnknownFunction4eb300(&light->color, section, "ColorRGB", "", 1, 0))
                return 0;
            light->field_0x18 = &light->position;
            if (!UnknownFunction4eb160(&light->position, section, "Position", "", 0, 0)) {
                light->position = Vector3(-2000.0f, 4000.0f, 1252.0f);
                if (light->type != 6 && light->type != 7)
                    UnknownFunction4de580(section, "Position(Default)", light->field_0x18);
                else
                    light->field_0x18 = 0;
            }
            field_0xdc.UnknownFunction4b7f10("PlaceLightAtInfinity", 0, &atInfinity);
            light->atInfinity = atInfinity;
            UnknownFunction4de580(section, "PlaceLightAtInfinity", (int)light->atInfinity);
            if (light->atInfinity) {
                light->direction = light->position * (80000.0f / Length(light->position));
                light->field_0x18 = &light->direction;
            }
            light->field_0x34 = &light->look;
            if (UnknownFunction4eb160(&vector, section, "LookVector", "", 0, 0)) {
                light->look = vector;
            } else if (UnknownFunction4eb160(&vector, section, "TargetPosition", "", 0, 0)) {
                light->look = Normalize(vector - light->position);
            } else {
                light->look = Vector3(0.0f, 0.0f, 0.0f);
                light->field_0x34 = 0;
            }
            field_0xdc.UnknownFunction4b7f40("Range", -1.0f, &light->range);
            float range = light->range;
            if (range == -1.0) {
                light->range = 0.0f;
                if (light->type != 6 && light->type != 7)
                    UnknownFunction4de580(section, "Range(Default)", 0.0f);
            } else {
                UnknownFunction4de580(section, "Range", range);
            }
            field_0xdc.UnknownFunction4b7f10("ShowDebugSphere", 0, &showDebugSphere);
            light->showDebugSphere = showDebugSphere;
            UnknownFunction4de580(section, "ShowDebugSphere", (int)light->showDebugSphere);
            field_0xdc.UnknownFunction4b7f10("CastsShadows", 0, &castsShadows);
            light->castsShadows = castsShadows;
            UnknownFunction4de580(section, "CastsShadows", (int)light->castsShadows);
            field_0xdc.UnknownFunction4b7f10("EmitsLight", 1, &emitsLight);
            light->emitsLight = emitsLight;
            UnknownFunction4de580(section, "EmitsLight", (int)light->emitsLight);
            field_0xdc.UnknownFunction4b7f10("HasLensFlare", 0, &hasLensFlare);
            light->hasLensFlare = hasLensFlare;
            UnknownFunction4de580(section, "HasLensFlare", (int)light->hasLensFlare);
            if (light->hasLensFlare) {
                UnknownFunction4eb300(&light->lensFlareColor, section, "LensFlareColorRGB", "", 0, 0);
                field_0xdc.UnknownFunction4b7f40("LensFlareBrightness", -1.0f, &light->lensFlareBrightness);
                float brightness = light->lensFlareBrightness;
                if (brightness == -1.0) {
                    light->lensFlareBrightness = 1.0f;
                    UnknownFunction4de580(section, "LensFlareBrightness(Default)", 1.0f);
                } else {
                    UnknownFunction4de580(section, "LensFlareBrightness", brightness);
                }
                if (UnknownFunction4eb480(light->layersVisible, 5, section, "LensFlareLayersVisible",
                                          "T,F,F,F,F", 0, 0)) {
                    sprintf(text, "[%s].LensFlareLayersVisible=%s,%s,%s,%s,%s", section,
                            TF(light->layersVisible[0]), TF(light->layersVisible[1]),
                            TF(light->layersVisible[2]), TF(light->layersVisible[3]),
                            TF(light->layersVisible[4]));
                    UnknownFunction464e80(text);
                }
                if (field_0xdc.UnknownFunction4b7ec0("LensFlareTextureMapName", "", light->lensFlareTexture,
                                                      0x3f))
                    UnknownFunction4de580(section, "LensFlareTextureMapName", light->lensFlareTexture);
                else
                    strncpy(light->lensFlareTexture, "overlay\\lensflare.tga", 0x3f);
            }
            UnknownFunction464e80("");
            return 1;
        }
        UnknownFunction4de580(section, "Type", text);
        return 0;
    }
    UnknownSceneLight* light = &field_0xac[index];
    if (index == 0) {
        light->type = 6;
        light->color = 0x5a5a5a;
        light->field_0x18 = 0;
        light->look = Vector3(0.0f, 0.0f, 0.0f);
        light->field_0x34 = 0;
        light->range = 0;
        light->showDebugSphere = 0;
        light->atInfinity = 0;
        light->emitsLight = 1;
        light->castsShadows = 0;
        light->hasLensFlare = 0;
    } else {
        light->position = Vector3(-2000.0f, 4000.0f, 1252.0f);
        light->type = 2;
        light->color = 0xc8c8c8;
        light->atInfinity = 0;
        light->direction = light->position * (80000.0f / Length(light->position));
        light->field_0x34 = 0;
        light->look = Vector3(0.0f, 0.0f, 0.0f);
        light->field_0x18 = &light->position;
        light->range = 0;
        light->showDebugSphere = 0;
        light->emitsLight = 1;
        light->castsShadows = 1;
        light->hasLensFlare = 0;
    }
    UnknownFunction464e80("No Scene File or no light data:  Use DEFAULT Lights (1 Ambient, 1 ObjectPoint "
                          "shadowcaster\n");
    return 1;
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

// 0x004ebfc0: allocates field_0xa4 and reads the "Environment" section
// (file names relative to `directory`, the cube file to `cubeDirectory`),
// the eight terrain zones and the four reverb zones. Retail reports the
// terrain file as the ecosystem file and the particle texture as the detail
// texture.
int Scene::UnknownFunction4ebfc0(const char* directory, const char* cubeDirectory)
{
    char zone[0x40];
    char text[0x204];

    int found = field_0xdc.UnknownFunction4b78f0("Environment");
    if (field_0x2c->field_0x38c && !found) {
        sprintf(text, "\nCannot find data for [%s] in %s.\n\n", "Environment", field_0x7c8);
        UnknownFunction464e80(text);
        return 0;
    }
    field_0xa4 = new(__FILE__, 1456) UnknownSceneEnvironment;
    sprintf(field_0xa4->terrainFile, "%s\\%s", directory, field_0x2c->field_0x2cc);
    sprintf(field_0xa4->ecosystemFile, "%s\\%s", directory, field_0x2c->field_0x28c);
    if (field_0x2c->field_0x30c[0])
        sprintf(field_0xa4->cubeFile, "%s\\%s", cubeDirectory, field_0x2c->field_0x30c);
    else
        field_0xa4->cubeFile[0] = 0;
    strncpy(field_0xa4->particleTexture, "dirtpart.tga", 0x3f);
    strncpy(field_0xa4->detailTexture, "graynoise.tga", 0x3f);
    field_0xa4->startGateScale = 1.0f;
    if (field_0xdc.UnknownFunction4b7ec0("TerrainFile", "", text, 0x100)) {
        sprintf(field_0xa4->terrainFile, "%s\\%s", directory, text);
        strncpy(field_0x2c->field_0x2cc, text, 0x3f);
        strcpy(strrchr(text, '.'), ".est");
        sprintf(field_0xa4->ecosystemFile, "%s\\%s", directory, text);
        strncpy(field_0x2c->field_0x28c, text, 0x3f);
        UnknownFunction4de580("Environment", "TerrainFile", field_0xa4->terrainFile);
        UnknownFunction4de580("Environment", "EcosystemFile", field_0xa4->terrainFile);
    } else {
        UnknownFunction4de580("Environment", "TerrainFile(Default)", field_0xa4->terrainFile);
        UnknownFunction4de580("Environment", "EcosystemFile(Default)", field_0xa4->ecosystemFile);
        if (field_0x2c->field_0x38c)
            return 0;
    }
    if (field_0xdc.UnknownFunction4b7ec0("EcosystemFile", "", text, 0x100)) {
        sprintf(field_0xa4->ecosystemFile, "%s\\%s", directory, text);
        strncpy(field_0x2c->field_0x28c, text, 0x3f);
        UnknownFunction4de580("Environment", "EcosystemFile", field_0xa4->terrainFile);
    }
    if (field_0xdc.UnknownFunction4b7ec0("CubeFile", "", text, 0x104)) {
        sprintf(field_0xa4->cubeFile, "%s\\%s", cubeDirectory, text);
        strncpy(field_0x2c->field_0x30c, text, 0x3f);
        UnknownFunction4de580("Environment", "CubeFile", field_0xa4->cubeFile);
    }
    if (field_0xdc.UnknownFunction4b7ec0("ParticleTextureMapName", "", text, 0x100)) {
        strncpy(field_0xa4->particleTexture, text, 0x3f);
        UnknownFunction4de580("Environment", "ParticleTextureMapName", field_0xa4->particleTexture);
    } else if (field_0x2c->field_0x38c) {
        UnknownFolded4de580("Environment", "ParticleTextureMapName", "dirtpart.tga");
    }
    if (field_0xdc.UnknownFunction4b7ec0("DetailTextureMapName", "", text, 0x100)) {
        strncpy(field_0xa4->detailTexture, text, 0x3f);
        UnknownFunction4de580("Environment", "DetailTextureMapName", field_0xa4->particleTexture);
    } else if (field_0x2c->field_0x38c) {
        UnknownFolded4de580("Environment", "DetailTextureMapName", "graynoise.tga");
    }
    if (!field_0xdc.UnknownFunction4b7f40("TerrainWidthScale", 3.0f, &field_0xa4->terrainWidthScale))
        UnknownFunction4de580("Environment", "TerrainWidthScale(Default)", field_0xa4->terrainWidthScale);
    else
        UnknownFunction4de580("Environment", "TerrainWidthScale", field_0xa4->terrainWidthScale);
    if (!field_0xdc.UnknownFunction4b7f40("TerrainBreadthScale", 3.0f, &field_0xa4->terrainBreadthScale))
        UnknownFunction4de580("Environment", "TerrainBreadthScale(Default)", field_0xa4->terrainBreadthScale);
    else
        UnknownFunction4de580("Environment", "TerrainBreadthScale", field_0xa4->terrainBreadthScale);
    if (!field_0xdc.UnknownFunction4b7f40("TerrainWidth", 5.0f, &field_0xa4->terrainWidth))
        UnknownFunction4de580("Environment", "TerrainWidth(Default)", field_0xa4->terrainWidth);
    else
        UnknownFunction4de580("Environment", "TerrainWidth", field_0xa4->terrainWidth);
    if (!field_0xdc.UnknownFunction4b7f40("TerrainBreadth", 5.0f, &field_0xa4->terrainBreadth))
        UnknownFunction4de580("Environment", "TerrainBreadth(Default)", field_0xa4->terrainBreadth);
    else
        UnknownFunction4de580("Environment", "TerrainBreadth", field_0xa4->terrainBreadth);
    if (!field_0xdc.UnknownFunction4b7f40("StartGateScale", 1.0f, &field_0xa4->startGateScale))
        UnknownFunction4de580("Environment", "StartGateScale(Default)", field_0xa4->startGateScale);
    else
        UnknownFunction4de580("Environment", "StartGateScale", field_0xa4->startGateScale);
    found = field_0xdc.UnknownFunction4b78f0("TerrainZone1");
    field_0xa4->surfaceFriction[0] = 1.0f;
    field_0xa4->surfaceDrag[0] = 1.0f;
    field_0xa4->surfaceTraction[0] = 1.0f;
    if (!found) {
        sprintf(text, "\nCannot find Dust/Dirt data for [%s] in %s.\n\n", "TerrainZone1", field_0x7c8);
        UnknownFunction464e80(text);
    } else {
        int dust;
        int dirt;
        field_0xdc.UnknownFunction4b7f10("GenerateDust", 1, &dust);
        field_0xa4->generateDust[0] = dust;
        field_0xdc.UnknownFunction4b7f10("GenerateDirt", 1, &dirt);
        field_0xa4->generateDirt[0] = dirt;
    }
    int i;
    for (i = 2; i < 9; i++) {
        int dust;
        int dirt;
        sprintf(zone, "TerrainZone%d", i);
        if (!field_0xdc.UnknownFunction4b78f0(zone)) {
            sprintf(text, "\nCannot find data for [%s] in %s.\n\n", zone, field_0x7c8);
            UnknownFunction464e80(text);
            field_0xa4->surfaceFriction[i - 1] = 1.0f;
            field_0xa4->surfaceDrag[i - 1] = 1.0f;
            field_0xa4->surfaceTraction[i - 1] = 1.0f;
            field_0xa4->generateDust[i - 1] = i != 8;
            field_0xa4->generateDirt[i - 1] = i != 8;
            continue;
        }
        if (!field_0xdc.UnknownFunction4b7f40("SurfaceFriction", 1.0f, &field_0xa4->surfaceFriction[i - 1])) {
            UnknownFunction4de580(zone, "SurfaceFriction", "NOT FOUND");
        } else if (field_0xa4->surfaceFriction[i - 1] > 2.0f) {
            field_0xa4->surfaceFriction[i - 1] = 2.0f;
        } else if (field_0xa4->surfaceFriction[i - 1] < 0.05f) {
            field_0xa4->surfaceFriction[i - 1] = 0.05f;
        }
        UnknownFunction4de580(zone, "SurfaceFriction", field_0xa4->surfaceFriction[i - 1]);
        if (!field_0xdc.UnknownFunction4b7f40("SurfaceDrag", 1.0f, &field_0xa4->surfaceDrag[i - 1])) {
            UnknownFunction4de580(zone, "SurfaceDrag", "NOT FOUND");
        } else if (field_0xa4->surfaceDrag[i - 1] > 2.0f) {
            field_0xa4->surfaceDrag[i - 1] = 2.0f;
        } else if (field_0xa4->surfaceDrag[i - 1] < 0.05f) {
            field_0xa4->surfaceDrag[i - 1] = 0.05f;
        }
        UnknownFunction4de580(zone, "SurfaceDrag", field_0xa4->surfaceDrag[i - 1]);
        if (!field_0xdc.UnknownFunction4b7f40("SurfaceTraction", 1.0f, &field_0xa4->surfaceTraction[i - 1])) {
            UnknownFunction4de580(zone, "SurfaceTraction", "NOT FOUND");
        } else if (field_0xa4->surfaceTraction[i - 1] > 2.0f) {
            field_0xa4->surfaceTraction[i - 1] = 2.0f;
        } else if (field_0xa4->surfaceTraction[i - 1] < 0.05f) {
            field_0xa4->surfaceTraction[i - 1] = 0.05f;
        }
        UnknownFunction4de580(zone, "SurfaceTraction", field_0xa4->surfaceTraction[i - 1]);
        field_0xdc.UnknownFunction4b7f10("GenerateDust", 1, &dust);
        field_0xa4->generateDust[i - 1] = dust;
        UnknownFunction4de580(zone, "GenerateDust", (int)field_0xa4->generateDust[i - 1]);
        field_0xdc.UnknownFunction4b7f10("GenerateDirt", 1, &dirt);
        field_0xa4->generateDirt[i - 1] = dirt;
        UnknownFunction4de580(zone, "GenerateDirt", (int)field_0xa4->generateDirt[i - 1]);
    }
    UnknownFunction464e80("");
    for (i = 0; i < 4; i++)
        field_0xa4->enviroID[i] = 0;
    for (i = 0; i < 4; i++) {
        sprintf(zone, "ReverbZone%d", i + 1);
        if (!field_0xdc.UnknownFunction4b78f0(zone)) {
            sprintf(text, "\nCannot find ReverbZone data for [%s] in %s.\n\n", zone, field_0x7c8);
            UnknownFunction464e80(text);
        } else {
            field_0xdc.UnknownFunction4b7b30("EnviroID", 0, field_0xa4->enviroID[i]);
        }
    }
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

// 0x004ef4c0: reads the "Sounds" section: whether a crowd is present and
// the "StaticSound<n>" sections, each an emitter placed in the scene or a
// plain sound added to field_0xc8. Needs the AuralScape (field_0xc0).
int Scene::UnknownFunction4ef4c0()
{
    int count;
    int oneShot;
    int force2D;
    unsigned long flags;
    float oneShotDistance;
    float randomTriggerPercent;
    UnknownSound3DParameters params;
    char file[0x104];
    char text[0x204];
    char position[0x80];
    char type[0x104];
    char section[0x204];

    if (!field_0x2c->field_0x38c)
        return 0;
    field_0xdc.UnknownFunction4b78f0("Sounds");
    field_0xdc.UnknownFunction4b7f10("CrowdPresent", 0, &field_0xbc);
    UnknownFunction4de580("Sounds", "CrowdPresent", field_0xbc);
    UnknownFunction464e80("");
    field_0xdc.UnknownFunction4b7f10("NumStaticSounds", 0, &count);
    if (count > 0) {
        sprintf(position, "%f %f %f", FLT_MAX, FLT_MAX, FLT_MAX);
        for (int i = 1; i <= count; i++) {
            sprintf(section, "StaticSound%d", i);
            field_0xdc.UnknownFunction4b78f0(section);
            if (field_0xc0) {
                if (field_0xdc.UnknownFunction4b7ec0("SoundResourceFile", "", file, 0x103) && strcmp(file, "")
                    && _stricmp(file, "NONE")) {
                    field_0xdc.UnknownFunction4b7ec0("SoundType", "Emitter", type, 0x103);
                    if (!_stricmp(type, "Emitter")) {
                        UnknownFunction4ef9c0(section, file, 1, &params, &flags, &oneShotDistance,
                                              &randomTriggerPercent, &oneShot, &force2D);
                        if (!UnknownFunction4eb160(&params.position, section, "Position", position, 1, 0)) {
                            sprintf(text, "Position not available for StaticSound%d\n", i);
                            UnknownFunction464e80(text);
                        }
                        SoundEmitter* emitter =
                            (new(__FILE__, 2588)
                                 SoundEmitter((AuralScape*)field_0xc0, field_0xc4, 1, field_0x25_bit0))
                                ->UnknownFunction402260(field_0x18, file, params, flags, oneShotDistance,
                                                        randomTriggerPercent, 0, force2D);
                        UnknownFunction469190(emitter, -1);
                        if (!emitter) {
                            sprintf(text, "\nScene::SoundEmitter(%s) not created\n", file);
                            UnknownFunction464e80(text);
                        }
                    } else if (!_stricmp(type, "Sound")) {
                        int volume;
                        if (!strstr(file, ".wav"))
                            strcat(file, ".wav");
                        Sound* sound = UnknownFunction4bb890(field_0xc4, file, 1, 1, 0, -1);
                        field_0xdc.UnknownFunction4b7f10("SoundVolume", 0, &volume);
                        sound->UnknownFunction4bcbe0(volume, 0);
                        field_0xc8.Add(sound);
                    }
                }
            } else {
                sprintf(text, "\nScene:  No AuralScape, so NO Static Model SOUNDS!\n");
                UnknownFunction464e80(text);
            }
        }
        UnknownFunction464e80("");
        return 1;
    }
    sprintf(text, "\nNumber of StaticSounds<1 in %s.\n\n", field_0x7c8);
    UnknownFunction464e80(text);
    return 0;
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

// 0x004efb20: loads the scene file into field_0xdc and reads the
// "SceneInfo" positions, the environment and the "Lights" section, creating
// an emitter in `lights` for each light that emits; then the fog, resource
// files, stadium and the 0x004ecd60/0x004edfe0/0x004ef4c0 parts. Returns 0
// when a part fails.
int Scene::UnknownFunction4efb20(void* owner, LightManager* lights, int a3, int a4, int a5,
                                 const char* cubeDirectory, void (*progress)(int), int interval)
{
    int logProgress;
    char path[0x104];

    UnknownTextureStream* archive = field_0x2c->UnknownFunction4e9ba0(0);
    if (!archive)
        return 0;
    sprintf(field_0x7c8, "%s\\%s", field_0x2c->field_0x44, field_0x2c->field_0x24c);
    field_0xdc.UnknownFunction4b7220((UnknownParameterStream*)archive, field_0x2c->field_0x24c,
                                     field_0x7c8, 1);
    field_0xdc.UnknownFunction4b78f0("SceneInfo");
    if (field_0x7c4)
        fclose(field_0x7c4);
    logProgress = 0;
    field_0xdc.UnknownFunction4b7f10("LogProgress", 0, &logProgress);
    if (logProgress) {
        sprintf(path, "%s\\%s", field_0x2c->field_0x44, "scnmgr.log");
        field_0x7c4 = fopen(path, "w");
    } else {
        field_0x7c4 = 0;
    }
    field_0x2c->UnknownFunction4ea390(field_0x30, field_0x2c->field_0x24c, 0);
    UnknownFunction4de580("SceneInfo", "SceneName", field_0x30);
    UnknownFunction4eb160(&field_0x70, "SceneInfo", "DefaultStartPosition", "1152.0,100.0,1152.0", 1, 0);
    UnknownFunction4eb160(&field_0x7c, "SceneInfo", "DefaultStartDirection", "0.0,0.0,1.0", 1, 0);
    UnknownFunction464e80("");
    field_0x18 = owner;
    UnknownFunction4eb160(&field_0x8c, "SceneInfo", "PodiumPosition", "0.0,0.0,0.0", 1, 0);
    UnknownFunction4eb160(&field_0x98, "SceneInfo", "PodiumDirection", "0.0,0.0,0.0", 1, 0);
    if (!UnknownFunction4ebfc0(field_0x2c->field_0x44, cubeDirectory))
        return 0;
    field_0xdc.UnknownFunction4b78f0("Lights");
    field_0xdc.UnknownFunction4b7f10("NumberOfLights", -1, &field_0xa8);
    UnknownFunction4de580("Lights", "NumberOfLights", field_0xa8);
    UnknownFunction464e80("");
    if (!field_0x2c->field_0x38c || field_0xa8 == -1)
        field_0xa8 = 2;
    field_0xac = new(__FILE__, 2784) UnknownSceneLight[field_0xa8];
    for (int i = 0; i < field_0xa8; i++) {
        field_0xac[i].field_0x00 = 0;
        if (!UnknownFunction4eb570(i))
            return 0;
        if (field_0xac[i].emitsLight) {
            const char* flare = field_0xac[i].hasLensFlare ? field_0xac[i].lensFlareTexture : 0;
            int camera = field_0xac[i].hasLensFlare ? (int)g_UnknownGlobal56e26c->field_0x3c : 0;
            field_0xac[i].field_0x00 =
                (new(__FILE__, 2798) LightEmitter(1))
                    ->UnknownFunction49e230(field_0x18, field_0xac[i].type, field_0xac[i].color,
                                            &field_0xac[i].position, field_0xac[i].field_0x34,
                                            field_0xac[i].range, field_0xac[i].showDebugSphere, 0, camera,
                                            (int)flare, i);
            if (!field_0xac[i].field_0x00)
                return 0;
            lights->UnknownFunction49e470(field_0xac[i].field_0x00);
        }
    }
    UnknownFunction4ebdb0();
    UnknownFunction4eca20();
    UnknownFunction4ecc10();
    {
        int objects = UnknownFunction4ecd60(lights, a3, a4, a5, progress, interval);
        if (!UnknownFunction4edfe0(lights, a4, a5, progress, interval) && !objects)
            UnknownVirtualSlot4();
    }
    UnknownFunction4ef4c0();
    return 1;
}

// 0x004eff30: selects the detail table and applies `level` to every entry's
// object and to the shadow casters, re-enabling those without flag 2.
void Scene::UnknownFunction4eff30(int level)
{
    g_UnknownGlobal689f18 = g_UnknownGlobal56e26c->field_0x2d0 ? g_UnknownGlobal5744c8
                                                              : g_UnknownGlobal574428;
    if (field_0xb4) {
        int i;
        for (i = 0; i < field_0xb4->field_0x00; i++) {
            if (field_0xb4->field_0x04[i].field_0x00_bit3)
                field_0xb4->field_0x04[i].field_0x04->field_0x1a0->UnknownFunction4451e0(level);
            else
                ((UnknownSceneAnimatedObject*)field_0xb4->field_0x04[i].field_0x08)
                    ->field_0x34->UnknownFunction4451e0(level);
        }
        for (i = 0; i < field_0xb4->field_0x08; i++) {
            UnknownSceneEntry* entry = field_0xb4->field_0x0c[i].field_0x04;
            if (entry->field_0x00_bit3)
                entry->field_0x04->field_0x1a0->UnknownFunction4451e0(level);
            else
                ((UnknownSceneAnimatedObject*)entry->field_0x08)->field_0x34->UnknownFunction4451e0(level);
        }
    }
    if (field_0xb8) {
        for (int i = 0; i < field_0xb8->field_0x00; i++) {
            if (field_0xb8->field_0x04[i].field_0x04) {
                ((UnknownSceneLodObject*)field_0xb8->field_0x04[i].field_0x04)->UnknownFunction4451e0(level);
                if (!field_0xb8->field_0x04[i].useLighting)
                    ((UnknownSceneLodObject*)field_0xb8->field_0x04[i].field_0x04)->UnknownFunction4444c0(0);
            }
        }
    }
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

// Copies at most 0x103 characters of `from` and terminates them.
static inline void CopyPathPart(char* to, const char* from)
{
    int length = strlen(from);
    int count = length > 0x103 ? 0x103 : length;
    strncpy(to, from, count);
    to[count] = 0;
}

// 0x004f0390: see the declaration. The compiled ".slb" file holds the node
// records (skipped) and the materials; without it the ".slt" text file's
// "Material - <n>" sections name the textures. 0 only when a texture cannot
// be measured.
int UnknownFunction4f0390(const char* path, UnknownSceneResourceManager* resources, unsigned long* counts)
{
    int materials;
    char drive[_MAX_DRIVE];
    int count;
    int position;
    char mode;
    char name[0x104];
    char file[0x104];
    char fname[_MAX_FNAME];
    char section[0x80];
    char dir[_MAX_DIR];
    char ext[_MAX_EXT];

    if (!*path)
        return 1;
    _splitpath(path, drive, dir, fname, ext);
    CopyPathPart(name, fname);
    strcat(name, ".slb");
    CopyPathPart(file, drive);
    strcat(file, dir);
    strcat(file, fname);
    strcat(file, ".slb");
    if (!UnknownFunction4f0310((UnknownResourceManager*)resources, name, file))
        return 1;
    UnknownResourceEntry* entry = g_UnknownResourceManager572b44->UnknownFunction4e9360(name, 0);
    if (!entry) {
        g_UnknownResourceManager572b44->UnknownFunction4e9430(name, file);
        entry = g_UnknownResourceManager572b44->UnknownFunction4e9360(name, 0);
    }
    if (entry) {
        entry->field_0x14->UnknownFunction461340(entry->field_0x18, 0, 0);
        entry->field_0x14->UnknownFunction461640(&count, 4, 1);
        SoultreeObject* node = new(__FILE__, 3447) SoultreeObject(0);
        for (int i = 0; i < count; i++) {
            entry->field_0x14->UnknownFunction461640(node->field_0x038, 0x80, 1);
            entry->field_0x14->UnknownFunction461640(&node->field_0x0b8, 0x40, 1);
            entry->field_0x14->UnknownFunction461640(&node->field_0x14c, 4, 1);
            entry->field_0x14->UnknownFunction461640(&node->field_0x154, 4, 1);
            entry->field_0x14->UnknownFunction461640(&node->field_0x158, 0xc, 1);
            entry->field_0x14->UnknownFunction461640(&node->field_0x164, 0xc, 1);
            entry->field_0x14->UnknownFunction461640(&node->field_0x170, 4, 1);
            entry->field_0x14->UnknownFunction461640(&node->field_0x174, 0xc, 1);
            entry->field_0x14->UnknownFunction461640(&node->field_0x180, 0xc, 1);
        }
        node->Release();
        int* parents = (int*)operator new(count * 4, __FILE__, 3466);
        entry->field_0x14->UnknownFunction461640(parents, 4, count);
        operator delete(parents);
        entry->field_0x14->UnknownFunction461640(&materials, 4, 1);
        if (materials > 0) {
            SoultreeMaterial* material = new(__FILE__, 3472) SoultreeMaterial(0);
            for (int j = 0; j < materials; j++) {
                entry->field_0x14->UnknownFunction461640(material->field_0x2c, 0x40, 1);
                entry->field_0x14->UnknownFunction461640(&material->field_0x8c, 4, 1);
                entry->field_0x14->UnknownFunction461640(&material->field_0x90, 4, 1);
                entry->field_0x14->UnknownFunction461640(&material->field_0x94, 4, 1);
                entry->field_0x14->UnknownFunction461640(&material->field_0x98, 4, 1);
                entry->field_0x14->UnknownFunction461640(&material->field_0x9c, 4, 1);
                entry->field_0x14->UnknownFunction461640(&material->field_0xa0, 4, 1);
                entry->field_0x14->UnknownFunction461640(&material->field_0xa4, 4, 1);
                entry->field_0x14->UnknownFunction461640(&material->field_0xa8, 4, 1);
                entry->field_0x14->UnknownFunction461640(&material->field_0xac, 4, 1);
                entry->field_0x14->UnknownFunction461640(&material->field_0xb0, 4, 1);
                entry->field_0x14->UnknownFunction461640(&material->field_0xb8, 4, 1);
                entry->field_0x14->UnknownFunction461640(&material->field_0xb4, 4, 1);
                entry->field_0x14->UnknownFunction461640(&material->field_0xbc, 4, 1);
                entry->field_0x14->UnknownFunction461640(&material->field_0xc0, 4, 1);
                entry->field_0x14->UnknownFunction461640(&material->field_0xc4, 4, 1);
                entry->field_0x14->UnknownFunction461640(&material->field_0xc8, 2, 1);
                entry->field_0x14->UnknownFunction461640(&material->field_0xcc, 4, 1);
                position = entry->field_0x14->UnknownFunction461600();
                mode = entry->field_0x14->UnknownFunction43e9e0();
                if (material->field_0x9c) {
                    if (!UnknownFunction4f0310((UnknownResourceManager*)resources, material->field_0x2c, 0))
                        return 1;
                    if (!UnknownFunction4f00e0((UnknownSceneTextureInfo*)material, (int*)counts))
                        return 0;
                }
                entry->field_0x14->UnknownFunction461340(position, 0, 0);
                entry->field_0x14->UnknownFunction43e9b0(mode);
            }
            material->Release();
        }
        return 1;
    }
    CopyPathPart(name, fname);
    strcat(name, ".slt");
    CopyPathPart(file, drive);
    strcat(file, dir);
    strcat(file, fname);
    strcat(file, ".slt");
    if (!UnknownFunction4f0310((UnknownResourceManager*)resources, name, file))
        return 1;
    if (!g_UnknownResourceManager572b44->UnknownFunction4e9360(name, 0)) {
        g_UnknownResourceManager572b44->UnknownFunction4e9430(name, file);
        if (!g_UnknownResourceManager572b44->UnknownFunction4e9360(name, 0))
            return 1;
    }
    UnknownTextureStream* stream = new(__FILE__, 3622) UnknownTextureStream((int)g_UnknownResourceManager572b44);
    stream->UnknownFunction460f50(name, "rb", 0);
    UnknownParameterBlock block;
    block.UnknownFunction4b77a0((UnknownParameterStream*)stream, 0, 1);
    block.UnknownFunction4b78f0("Materials");
    block.UnknownFunction4b7f10("NumberOfMaterials", 0, &materials);
    SoultreeMaterial material(0);
    for (int k = 0; k < materials; k++) {
        sprintf(section, "Material - %d", k);
        block.UnknownFunction4b78f0(section);
        if (block.UnknownFunction4b7b30("TextureMap", material.field_0x2c, -1)) {
            if (!UnknownFunction4f0310((UnknownResourceManager*)resources, material.field_0x2c, 0)) {
                delete stream;
                return 1;
            }
            if (!UnknownFunction4f00e0((UnknownSceneTextureInfo*)&material, (int*)counts)) {
                delete stream;
                return 0;
            }
        }
    }
    delete stream;
    return 1;
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

// 0x004f1130: see the declaration. Model and animation files are relative to
// "Res"; key-framed animations name an MCF file whose SLT file 0x004f0ec0
// looks up.
int Scene::UnknownFunction4f1130(unsigned long* counts, float* width, int* hasCube, char* ecosystem)
{
    int models;
    int animations;
    int next;
    char animation[0x40];
    char model[0x40];
    char text[0x204];
    char file[0x108];
    char animationPath[0x104];
    char message[0x204];
    char modelPath[0x108];
    char modelMessage[0x204];

    UnknownTextureStream* archive = field_0x2c->UnknownFunction4e9ba0(0);
    if (!archive)
        return 0;
    UnknownSceneResourceManager resources;
    sprintf(field_0x7c8, "%s\\%s", field_0x2c->field_0x44, field_0x2c->field_0x24c);
    field_0xdc.UnknownFunction4b7220((UnknownParameterStream*)archive, field_0x2c->field_0x24c,
                                     field_0x7c8, 1);
    UnknownFunction4eca20();
    int i = 0;
    models = 0;
    int found = field_0xdc.UnknownFunction4b78f0("StaticModels");
    if (field_0x2c->field_0x38c && found) {
        if (!field_0xdc.UnknownFunction4b7f10("NumberOfStaticModels", -1, &models)) {
            sprintf(text, "\nNumberOfStaticModels cannot be found under [%s] in %s.\n\n", "StaticModels",
                    field_0x7c8);
            UnknownFunction464e80(text);
        } else if (models > i) {
            for (i = 0; i < models;) {
                sprintf(model, "Model%d", ++i);
                field_0xdc.UnknownFunction4b78f0(model);
                if (!field_0xdc.UnknownFunction4b7ec0("SLT", "", file, 0x104)) {
                    sprintf(modelMessage, "\nCannot find %s for [%s] in %s.\n\n", file, model, field_0x7c8);
                    UnknownFunction464e80(modelMessage);
                    return 0;
                }
                sprintf(modelPath, "%s\\%s", "Res", file);
                if (!UnknownFunction4f0390(modelPath, &resources, counts))
                    return 0;
            }
        }
    } else {
        sprintf(text, "\nCannot find data for [%s] in %s.\n\n", "StaticModels", field_0x7c8);
        UnknownFunction464e80(text);
    }
    i = 0;
    animations = 0;
    found = field_0xdc.UnknownFunction4b78f0("Animations");
    if (field_0x2c->field_0x38c && found) {
        if (!field_0xdc.UnknownFunction4b7f10("NumberOfAnimations", -1, &animations)) {
            sprintf(text, "\nNumberOfAnimations cannot be found under [%s] in %s.\n\n", "Animations",
                    field_0x7c8);
            UnknownFunction464e80(text);
        } else if (animations > i) {
            for (i = 0; i < animations; i = next) {
                int keyFramed;
                next = i + 1;
                sprintf(animation, "Animation%d", next);
                field_0xdc.UnknownFunction4b78f0(animation);
                if (!field_0xdc.UnknownFunction4b7ec0("AnimationType", "", text, 0x40)) {
                    sprintf(message, "\nScene: AnimationType not specified for anim#%d.  Using KeyFramed.\n", i);
                    UnknownFunction464e80(message);
                    keyFramed = 1;
                } else if (!_strnicmp(text, "Procedural", 3)) {
                    keyFramed = 0;
                } else {
                    keyFramed = 1;
                }
                if (keyFramed) {
                    if (!field_0xdc.UnknownFunction4b7ec0("MCF", "", file, 0x104)) {
                        sprintf(message, "\nCannot find %s for [%s] in %s.\n\n", "MCF", animation, field_0x7c8);
                        UnknownFunction464e80(message);
                        return 0;
                    }
                } else if (!field_0xdc.UnknownFunction4b7ec0("SLT", "", file, 0x104)) {
                    sprintf(message, "\nCannot find %s for [%s] in %s.\n\n", "SLT", animation, field_0x7c8);
                    UnknownFunction464e80(message);
                    return 0;
                }
                sprintf(animationPath, "%s\\%s", "Res", file);
                if (keyFramed) {
                    UnknownFunction4de580(animation, "MCF", animationPath);
                    UnknownFunction4f0ec0(animationPath);
                }
                UnknownFunction4de580(animation, "SLT", animationPath);
                if (!UnknownFunction4f0390(animationPath, &resources, counts))
                    return 0;
            }
        }
    } else {
        sprintf(text, "\nCannot find data for [%s] in %s.\n\n", "Animations", field_0x7c8);
        UnknownFunction464e80(text);
    }
    found = field_0xdc.UnknownFunction4b78f0("Environment");
    if (field_0x2c->field_0x38c && found) {
        sprintf(ecosystem, "%s\\%s", field_0x2c->field_0x44, field_0x2c->field_0x28c);
        if (field_0xdc.UnknownFunction4b7ec0("TerrainFile", "", text, 0x100)) {
            strcpy(strrchr(text, '.'), ".est");
            sprintf(ecosystem, "%s\\%s", field_0x2c->field_0x44, text);
        }
        if (field_0xdc.UnknownFunction4b7ec0("EcosystemFile", "", text, 0x100))
            sprintf(ecosystem, "%s\\%s", field_0x2c->field_0x44, text);
        field_0xdc.UnknownFunction4b7f40("TerrainWidth", 5.0f, width);
        *hasCube = field_0xdc.UnknownFunction4b7ec0("CubeFile", "", text, 0x104);
        return 1;
    }
    *width = 0;
    *hasCube = 0;
    return 0;
}
