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
int FindKeyword(int* out, const char* name, const UnknownSceneKeyword* table)
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
    sceneDirectory[0] = 0;
    sceneFile[0] = 0;
    trnFile[0] = 0;
    field_0x30c[0] = 0;
    waypointFile[0] = 0;
    waypointPath[0] = 0;
    sceneNamed = 0;
    sceneArchive = 0;
    sceneArchiveName[0] = 0;
    waypointRaceCount = 0;
}

// 0x004e9a10
int UnknownTrackGameObject574::UnknownFunction4e9a10(int* flag)
{
    Scene* scene = (new(__FILE__, 115) Scene(0))->UnknownFunction4ea7e0(this, 0, 0, 0);
    if (!scene)
        return 0;
    scene->CountObjects(flag);
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
    int result = scene->CountTextures(info, (float*)a, (int*)b, (char*)c);
    scene->Release();
    return result;
}

// 0x004e9b80
void UnknownTrackGameObject574::SetSceneDirectory(char* path)
{
    strncpy(sceneDirectory, path, 0x103);
}

// 0x004e9ba0: keeps the open archive when it already is `name`.
UnknownTextureStream* UnknownTrackGameObject574::OpenSceneArchive(const char* name)
{
    char scene[0x40];
    char path[0x104];

    if (name)
        strncpy(scene, name, 0x3f);
    else
        strncpy(scene, sceneFile, 0x3f);
    if (sceneArchive && !_stricmp(sceneArchiveName, scene))
        return sceneArchive;
    sprintf(path, "%s\\%s", sceneDirectory, scene);
    strcpy(strrchr(path, '.'), ".env");
    if (sceneArchive) {
        g_UnknownResourceManager572b44->RemoveArchiveItems(sceneArchive);
        delete sceneArchive;
    }
    sceneArchive = g_UnknownResourceManager572b44->AddArchive(path, 1);
    if (!sceneArchive) {
        sceneArchiveName[0] = 0;
        return 0;
    }
    int length = strlen(scene);
    int count = length > 0x3f ? 0x3f : length;
    strncpy(sceneArchiveName, scene, count);
    sceneArchiveName[count] = 0;
    return sceneArchive;
}

// 0x004e9cd0: opens `name`'s file name and extension through `stream`. It
// succeeds only inside an archive (an inner stream at +0x1c), then copies the
// opened name to `a` (a char buffer) when given. Otherwise it frees `stream`
// and allocates a replacement that nothing keeps.
int UnknownTrackGameObject574::OpenArchiveFile(UnknownTextureStream* stream, const char* name,
                                                     const char* mode, int a)
{
    char drive[_MAX_DRIVE];
    char file[0x104];
    char ext[_MAX_EXT];
    char dir[_MAX_DIR];
    char* out = (char*)a;

    _splitpath(name, drive, dir, file, ext);
    strcat(file, ext);
    if (stream->Open(file, mode, 0)) {
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
void UnknownTrackGameObject574::SelectScene(char* name, const char* kind, int value)
{
    char sceneName[0x40];
    char buffer[0x104];

    sprintf(buffer, "%s.%s", name, kind);
    strncpy(sceneFile, buffer, 0x3f);
    SetSceneFileNames(name);
    OpenSceneArchive(0);
    UnknownTextureStream* stream =
        new(__FILE__, 297) UnknownTextureStream((int)g_UnknownResourceManager572b44);
    if (stream->Open(sceneFile, "r", 0)) {
        ReadCurrentSceneName(sceneName, sceneFile, value);
        if (value && !_stricmp(sceneName, "NO NAME"))
            sceneNamed = 0;
        else
            sceneNamed = 1;
    } else {
        sceneNamed = 0;
    }
    delete stream;
}

// 0x004e9f70
void UnknownTrackGameObject574::SetSceneFileNames(const char* name)
{
    char buffer[0x104];

    sprintf(buffer, "%s.trn", name);
    strncpy(trnFile, buffer, 0x1f);
    strncpy(field_0x30c, "", 0x1f);
    sprintf(buffer, "%s01.wpt", name);
    strncpy(waypointFile, buffer, 0x1f);
    sprintf(buffer, "%s.est", name);
    strncpy(estFile, buffer, 0x1f);
}

// 0x004ea010: as 0x004ea390 for scene `name` (with extension `kind` when
// it has none), opened through 0x004e9ba0.
void UnknownTrackGameObject574::ReadSceneName(char* out, char* name, int index,
                                                      const char* kind, int* a, int* b)
{
    char path[0x104];
    char scene[0x104];
    char base[0x104];

    if (a)
        *a = 0;
    if (b)
        *b = 0;
    if (sceneDirectory[0]) {
        if (!strrchr(name, '.'))
            sprintf(scene, "%s.%s", name, kind);
        else
            strncpy(scene, name, 0x3f);
        sprintf(path, "%s\\%s", sceneDirectory, scene);
        UnknownParameterBlock block;
        UnknownTextureStream* archive = OpenSceneArchive(scene);
        if (archive) {
            archive->UnknownFunction461cb0(a, b);
            block.OpenFromArchives((UnknownParameterStream*)archive, scene, path, 0);
            block.SelectSection("SceneInfo");
            block.GetInt("NumberOfWaypointRaces", 0, &waypointRaceCount);
            if (index) {
                if (waypointRaceCount) {
                    UnknownTextureStream* stream = new(__FILE__, 401)
                        UnknownTextureStream((int)g_UnknownResourceManager572b44);
                    strcpy(base, scene);
                    strcpy(strrchr(base, '.'), "");
                    sprintf(path, "%s%02d.wpt", base, index);
                    strncpy(waypointFile, path, 0x3f);
                    if (stream->Open(waypointFile, "r", 0)) {
                        sprintf(waypointPath, "%s\\%s", sceneDirectory, waypointFile);
                        sprintf(path, "WaypointRaceName%d", index);
                        if (!block.GetString(path, "", out, 0x3f))
                            strncpy(out, "NO NAME", 0x3f);
                    } else {
                        waypointPath[0] = 0;
                        strncpy(out, "NO NAME", 0x3f);
                    }
                    delete stream;
                } else {
                    waypointPath[0] = 0;
                    strncpy(out, "NO NAME", 0x3f);
                }
            } else {
                if (!block.GetString("SceneName", "", out, 0x3f))
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
void UnknownTrackGameObject574::ReadCurrentSceneName(char* name, char* scene, int value)
{
    char path[0x104];
    char base[0x104];

    if (sceneDirectory[0]) {
        sprintf(path, "%s\\%s", sceneDirectory, scene);
        UnknownParameterBlock block;
        block.OpenFromArchives((UnknownParameterStream*)OpenSceneArchive(0), scene, path, 1);
        block.SelectSection("SceneInfo");
        block.GetInt("NumberOfWaypointRaces", 0, &waypointRaceCount);
        if (value) {
            if (waypointRaceCount) {
                UnknownTextureStream* stream =
                    new(__FILE__, 471) UnknownTextureStream((int)g_UnknownResourceManager572b44);
                strcpy(base, scene);
                strcpy(strrchr(base, '.'), "");
                sprintf(path, "%s%02d.wpt", base, value);
                strncpy(waypointFile, path, 0x3f);
                if (stream->Open(waypointFile, "r", 0)) {
                    sprintf(waypointPath, "%s\\%s", sceneDirectory, waypointFile);
                    sprintf(path, "WaypointRaceName%d", value);
                    if (!block.GetString(path, "", name, 0x3f))
                        strncpy(name, "NO NAME", 0x3f);
                } else {
                    waypointPath[0] = 0;
                    strncpy(name, "NO NAME", 0x3f);
                }
                delete stream;
            } else {
                waypointPath[0] = 0;
                strncpy(name, "NO NAME", 0x3f);
            }
        } else {
            if (!block.GetString("SceneName", "", name, 0x3f))
                strncpy(name, "NO NAME", 0x3f);
        }
    } else {
        strncpy(name, "NO NAME", 0x3f);
    }
}

// 0x004ea660
Scene::Scene(int flags) : GameObject(flags)
{
    defaultStartPosition = Vector3(0.0f, 0.0f, 0.0f);
    lightCount = 0;
    sceneLights = 0;
    fog = 0;
    environment = 0;
    animationTable = 0;
    staticModels = 0;
    sceneName[0] = 0;
    scenePath[0] = 0;
    defaultStartDirection = Vector3(0.0f, 0.0f, 0.0f);
    crowdPresent = 0;
    sounds.Init(4, 4);
    auralScape = 0;
    soundGroup = 0;
    field_0x88 = 0;
    stadiumFile[0] = 0;
    logFile = 0;
    visible = 1;
    field_0x7c0 = 0;
}

// 0x004ea7e0
Scene* Scene::UnknownFunction4ea7e0(UnknownTrackGameObject574* manager, int value, int,
                                    char flag)
{
    sceneManager = manager;
    auralScape = value;
    field_0x88 = flag;
    soundGroup = new(__FILE__, 622) SoundGroup(field_0x25_bit0);
    AppendChild(soundGroup, -1);
    return this;
}

// 0x004ea880
Scene::~Scene()
{
    if (logFile)
        fclose(logFile);
    if (sceneLights)
        delete sceneLights;
    if (fog)
        delete fog;
    if (environment)
        delete environment;
    if (animationTable) {
        if (animationTable->entryCount > 0) {
            for (int i = 0; i < animationTable->entryCount; i++) {
                if (animationTable->entries[i].keyFramed &&
                    animationTable->entries[i].motionCount > 0) {
                    if (animationTable->entries[i].motions)
                        delete animationTable->entries[i].motions;
                    if (animationTable->entries[i].motionSequence)
                        delete animationTable->entries[i].motionSequence;
                }
            }
            delete animationTable->entries;
        }
        if (animationTable->randomSetCount > 0) {
            for (int i = 0; i < animationTable->randomSetCount; i++) {
                if (animationTable->randomSets[i].count > 0) {
                    if (animationTable->randomSets[i].animationIndices)
                        delete animationTable->randomSets[i].animationIndices;
                    if (animationTable->randomSets[i].motionIndices)
                        delete animationTable->randomSets[i].motionIndices;
                }
            }
            delete animationTable->randomSets;
        }
        delete animationTable;
    }
    if (staticModels) {
        delete staticModels->casters;
        delete staticModels;
    }
}

// 0x004eaa60: plays every listed sound, then the GameObject slot.
void Scene::UnknownVirtualSlot5()
{
    for (int i = 0; i < sounds.m_count; i++) {
        Sound* sound = sounds.Get(i);
        if (sound)
            sound->PlayWithOptions(0, 1, 0);
    }
    GameObject::UnknownVirtualSlot5();
}

// 0x004eaab0: as slot 5, before the GameObject slot 7.
void Scene::UnknownVirtualSlot7()
{
    for (int i = 0; i < sounds.m_count; i++) {
        Sound* sound = sounds.Get(i);
        if (sound)
            sound->PlayWithOptions(0, 1, 0);
    }
    GameObject::UnknownVirtualSlot7();
}

// 0x004eaec0
UnknownSceneObject* Scene::FindEntryByFileName(const char* path, int* index)
{
    char name[0x104];

    if (!animationTable) {
        if (index)
            *index = 0;
        return 0;
    }
    _splitpath(path, 0, 0, name, 0);
    for (int i = 0; i < animationTable->entryCount; i++) {
        if (animationTable->entries[i].keyFramed &&
            !_strnicmp(animationTable->entries[i].character->field_0x1a4, name,
                       strcspn(animationTable->entries[i].character->field_0x1a4, "."))) {
            if (index)
                *index = i;
            return animationTable->entries[i].character;
        }
    }
    if (index)
        *index = 0;
    return 0;
}

// 0x004eafd0
void Scene::UnknownFunction4eafd0(int index)
{
    if (animationTable && index < animationTable->entryCount)
        animationTable->entries[index].field_0x00_bit2 = 1;
}

// 0x004eb000
void Scene::UnknownFunction4eb000(float time)
{
    if (animationTable) {
        for (int i = 0; i < animationTable->entryCount; i++)
            RestartEntry(i, time, 1, 0);
    }
}

// 0x004eb040: restarts entry `index` at `time` (at least 0.0001). A
// character entry (bit 3) switches to its motion number `motion` (0 when out
// of range) unless it is idle and `force` is clear; another entry's object
// gets the time and, unless the time was clamped, one slot 10 step.
int Scene::RestartEntry(int index, float time, int force, int motion)
{
    if (animationTable) {
        int running = 1;
        if (time < 0.0001f) {
            running = 0;
            time = 0.0001f;
        }
        UnknownSceneEntry* entry = &animationTable->entries[index];
        if (entry->keyFramed) {
            UnknownSceneObject* object = entry->character;
            if (object->field_0x0c || force) {
                if (motion >= entry->sequenceLength)
                    entry->sequenceIndex = 0;
                else
                    entry->sequenceIndex = motion;
                animationTable->entries[index].currentMotion =
                    animationTable->entries[index].motionSequence[animationTable->entries[index].sequenceIndex] - 1;
                object->SetMotion(
                    animationTable->entries[index].motions[animationTable->entries[index].currentMotion]);
                object->field_0x10 = 0;
                animationTable->entries[index].field_0x18 = object->AdvanceMotion(time, 0, 0);
                return 1;
            }
        } else {
            UnknownSceneAnimatedObject* object = (UnknownSceneAnimatedObject*)entry->proceduralObject;
            object->field_0x194 = time;
            object->field_0x5c = running;
            if (running)
                ((UnknownSceneAnimatedObject*)animationTable->entries[index].proceduralObject)
                    ->UnknownVirtualSlot10(0.0001f);
        }
    }
    return 0;
}

// 0x004eb160
int Scene::ReadVector(Vector3* out, const char* section, const char* key,
                                 const char* def, int useDefault, UnknownParameterBlock* block)
{
    char value[0x184];

    if (!block)
        block = &parameters;
    block->SelectSection(section);
    if (!sceneManager->sceneNamed) {
        block->GetString(key, def, value, 0x80);
        UnknownFunction4de580(section, key, value);
        out->x = (float)atof(strtok(value, ","));
        out->y = (float)atof(strtok(0, ","));
        out->z = (float)atof(strtok(0, "\n"));
        return 1;
    }
    if (block->GetString(key, "", value, 0x80)) {
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
int Scene::ReadColor(unsigned long* out, const char* section, const char* key,
                                 const char* def, int, UnknownParameterBlock* block)
{
    char value[0x184];

    if (!block)
        block = &parameters;
    block->SelectSection(section);
    if (!sceneManager->sceneNamed) {
        block->GetString(key, def, value, 0x80);
        UnknownFunction4de580(section, key, value);
        int r = atoi(strtok(value, ","));
        int g = atoi(strtok(0, ","));
        int b = atoi(strtok(0, "\n"));
        *out = ((r << 8 | g) << 8) | b;
        return 1;
    }
    if (block->GetString(key, def, value, 0x80)) {
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
int Scene::ReadFlags(char* out, int count, const char* section, const char* key,
                                 const char* def, int, UnknownParameterBlock* block)
{
    char value[0x184];

    if (!block)
        block = &parameters;
    block->SelectSection(section);
    if (block->GetString(key, def, value, 0x80)) {
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
int Scene::ReadLight(int index)
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
    int found = parameters.SelectSection(section);
    if (sceneManager->sceneNamed && found) {
        UnknownSceneLight* light = &sceneLights[index];
        parameters.GetString("Type", "BLANK", text, 0x100);
        if (FindKeyword(&light->type, text, s_lightTypes)) {
            UnknownFunction4de580(section, "Type", text);
            if (!ReadColor(&light->color, section, "ColorRGB", "", 1, 0))
                return 0;
            light->field_0x18 = &light->position;
            if (!ReadVector(&light->position, section, "Position", "", 0, 0)) {
                light->position = Vector3(-2000.0f, 4000.0f, 1252.0f);
                if (light->type != 6 && light->type != 7)
                    UnknownFunction4de580(section, "Position(Default)", light->field_0x18);
                else
                    light->field_0x18 = 0;
            }
            parameters.GetInt("PlaceLightAtInfinity", 0, &atInfinity);
            light->atInfinity = atInfinity;
            UnknownFunction4de580(section, "PlaceLightAtInfinity", (int)light->atInfinity);
            if (light->atInfinity) {
                light->direction = light->position * (80000.0f / Length(light->position));
                light->field_0x18 = &light->direction;
            }
            light->field_0x34 = &light->look;
            if (ReadVector(&vector, section, "LookVector", "", 0, 0)) {
                light->look = vector;
            } else if (ReadVector(&vector, section, "TargetPosition", "", 0, 0)) {
                light->look = Normalize(vector - light->position);
            } else {
                light->look = Vector3(0.0f, 0.0f, 0.0f);
                light->field_0x34 = 0;
            }
            parameters.GetFloat("Range", -1.0f, &light->range);
            float range = light->range;
            if (range == -1.0) {
                light->range = 0.0f;
                if (light->type != 6 && light->type != 7)
                    UnknownFunction4de580(section, "Range(Default)", 0.0f);
            } else {
                UnknownFunction4de580(section, "Range", range);
            }
            parameters.GetInt("ShowDebugSphere", 0, &showDebugSphere);
            light->showDebugSphere = showDebugSphere;
            UnknownFunction4de580(section, "ShowDebugSphere", (int)light->showDebugSphere);
            parameters.GetInt("CastsShadows", 0, &castsShadows);
            light->castsShadows = castsShadows;
            UnknownFunction4de580(section, "CastsShadows", (int)light->castsShadows);
            parameters.GetInt("EmitsLight", 1, &emitsLight);
            light->emitsLight = emitsLight;
            UnknownFunction4de580(section, "EmitsLight", (int)light->emitsLight);
            parameters.GetInt("HasLensFlare", 0, &hasLensFlare);
            light->hasLensFlare = hasLensFlare;
            UnknownFunction4de580(section, "HasLensFlare", (int)light->hasLensFlare);
            if (light->hasLensFlare) {
                ReadColor(&light->lensFlareColor, section, "LensFlareColorRGB", "", 0, 0);
                parameters.GetFloat("LensFlareBrightness", -1.0f, &light->lensFlareBrightness);
                float brightness = light->lensFlareBrightness;
                if (brightness == -1.0) {
                    light->lensFlareBrightness = 1.0f;
                    UnknownFunction4de580(section, "LensFlareBrightness(Default)", 1.0f);
                } else {
                    UnknownFunction4de580(section, "LensFlareBrightness", brightness);
                }
                if (ReadFlags(light->layersVisible, 5, section, "LensFlareLayersVisible",
                                          "T,F,F,F,F", 0, 0)) {
                    sprintf(text, "[%s].LensFlareLayersVisible=%s,%s,%s,%s,%s", section,
                            TF(light->layersVisible[0]), TF(light->layersVisible[1]),
                            TF(light->layersVisible[2]), TF(light->layersVisible[3]),
                            TF(light->layersVisible[4]));
                    UnknownFunction464e80(text);
                }
                if (parameters.GetString("LensFlareTextureMapName", "", light->lensFlareTexture,
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
    UnknownSceneLight* light = &sceneLights[index];
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
int Scene::ReadFog()
{
    float visibility;
    float haziness;

    fog = new(__FILE__, 1392) UnknownSceneFog;
    fog->color = 0x8080c0;
    fog->visibility = 1.0f;
    fog->haziness = 0.5f;
    int found = parameters.SelectSection("Fog");
    if (!sceneManager->sceneNamed || !found) {
        UnknownFunction464e80("No Scene File or no fog data:  Using DEFAULT Fog\n");
        return 1;
    }
    visibility = 1.0f;
    if (!parameters.GetFloat("Visibility", 1.0f, &visibility)) {
        UnknownFunction4de580("Fog", "Visibility", "NOT FOUND using default 1.0");
    } else if (visibility < 0.0f || visibility > 1.0f) {
        UnknownFunction4de580("Fog", "Visibility",
                              "Invalid value, must be between 0.0 and 1.0, using default 1.0");
        visibility = 1.0f;
    }
    fog->visibility = visibility;
    UnknownFunction4de580("Fog", "Visibility", fog->visibility);
    if (!parameters.GetFloat("Haziness", 0.5f, &haziness)) {
        UnknownFunction4de580("Fog", "Haziness", "NOT FOUND using default 0.5");
    } else if (visibility < 0.0f || visibility > 1.0f) {
        UnknownFunction4de580("Fog", "Haziness",
                              "Invalid value, must be between 0.0 and 1.0, using default 0.5");
        haziness = 0.5f;
    }
    fog->haziness = haziness;
    UnknownFunction4de580("Fog", "Haziness", fog->haziness);
    if (!ReadColor(&fog->color, "Fog", "ColorRGB", "", 1, 0))
        return 0;
    UnknownFunction464e80("");
    return 1;
}

// 0x004ebfc0: allocates environment (+0xa4) and reads the "Environment" section
// (file names relative to `directory`, the cube file to `cubeDirectory`),
// the eight terrain zones and the four reverb zones. Retail reports the
// terrain file as the ecosystem file and the particle texture as the detail
// texture.
int Scene::ReadEnvironment(const char* directory, const char* cubeDirectory)
{
    char zone[0x40];
    char text[0x204];

    int found = parameters.SelectSection("Environment");
    if (sceneManager->sceneNamed && !found) {
        sprintf(text, "\nCannot find data for [%s] in %s.\n\n", "Environment", scenePath);
        UnknownFunction464e80(text);
        return 0;
    }
    environment = new(__FILE__, 1456) UnknownSceneEnvironment;
    sprintf(environment->terrainFile, "%s\\%s", directory, sceneManager->trnFile);
    sprintf(environment->ecosystemFile, "%s\\%s", directory, sceneManager->estFile);
    if (sceneManager->field_0x30c[0])
        sprintf(environment->cubeFile, "%s\\%s", cubeDirectory, sceneManager->field_0x30c);
    else
        environment->cubeFile[0] = 0;
    strncpy(environment->particleTexture, "dirtpart.tga", 0x3f);
    strncpy(environment->detailTexture, "graynoise.tga", 0x3f);
    environment->startGateScale = 1.0f;
    if (parameters.GetString("TerrainFile", "", text, 0x100)) {
        sprintf(environment->terrainFile, "%s\\%s", directory, text);
        strncpy(sceneManager->trnFile, text, 0x3f);
        strcpy(strrchr(text, '.'), ".est");
        sprintf(environment->ecosystemFile, "%s\\%s", directory, text);
        strncpy(sceneManager->estFile, text, 0x3f);
        UnknownFunction4de580("Environment", "TerrainFile", environment->terrainFile);
        UnknownFunction4de580("Environment", "EcosystemFile", environment->terrainFile);
    } else {
        UnknownFunction4de580("Environment", "TerrainFile(Default)", environment->terrainFile);
        UnknownFunction4de580("Environment", "EcosystemFile(Default)", environment->ecosystemFile);
        if (sceneManager->sceneNamed)
            return 0;
    }
    if (parameters.GetString("EcosystemFile", "", text, 0x100)) {
        sprintf(environment->ecosystemFile, "%s\\%s", directory, text);
        strncpy(sceneManager->estFile, text, 0x3f);
        UnknownFunction4de580("Environment", "EcosystemFile", environment->terrainFile);
    }
    if (parameters.GetString("CubeFile", "", text, 0x104)) {
        sprintf(environment->cubeFile, "%s\\%s", cubeDirectory, text);
        strncpy(sceneManager->field_0x30c, text, 0x3f);
        UnknownFunction4de580("Environment", "CubeFile", environment->cubeFile);
    }
    if (parameters.GetString("ParticleTextureMapName", "", text, 0x100)) {
        strncpy(environment->particleTexture, text, 0x3f);
        UnknownFunction4de580("Environment", "ParticleTextureMapName", environment->particleTexture);
    } else if (sceneManager->sceneNamed) {
        UnknownFolded4de580("Environment", "ParticleTextureMapName", "dirtpart.tga");
    }
    if (parameters.GetString("DetailTextureMapName", "", text, 0x100)) {
        strncpy(environment->detailTexture, text, 0x3f);
        UnknownFunction4de580("Environment", "DetailTextureMapName", environment->particleTexture);
    } else if (sceneManager->sceneNamed) {
        UnknownFolded4de580("Environment", "DetailTextureMapName", "graynoise.tga");
    }
    if (!parameters.GetFloat("TerrainWidthScale", 3.0f, &environment->terrainWidthScale))
        UnknownFunction4de580("Environment", "TerrainWidthScale(Default)", environment->terrainWidthScale);
    else
        UnknownFunction4de580("Environment", "TerrainWidthScale", environment->terrainWidthScale);
    if (!parameters.GetFloat("TerrainBreadthScale", 3.0f, &environment->terrainBreadthScale))
        UnknownFunction4de580("Environment", "TerrainBreadthScale(Default)", environment->terrainBreadthScale);
    else
        UnknownFunction4de580("Environment", "TerrainBreadthScale", environment->terrainBreadthScale);
    if (!parameters.GetFloat("TerrainWidth", 5.0f, &environment->terrainWidth))
        UnknownFunction4de580("Environment", "TerrainWidth(Default)", environment->terrainWidth);
    else
        UnknownFunction4de580("Environment", "TerrainWidth", environment->terrainWidth);
    if (!parameters.GetFloat("TerrainBreadth", 5.0f, &environment->terrainBreadth))
        UnknownFunction4de580("Environment", "TerrainBreadth(Default)", environment->terrainBreadth);
    else
        UnknownFunction4de580("Environment", "TerrainBreadth", environment->terrainBreadth);
    if (!parameters.GetFloat("StartGateScale", 1.0f, &environment->startGateScale))
        UnknownFunction4de580("Environment", "StartGateScale(Default)", environment->startGateScale);
    else
        UnknownFunction4de580("Environment", "StartGateScale", environment->startGateScale);
    found = parameters.SelectSection("TerrainZone1");
    environment->surfaceFriction[0] = 1.0f;
    environment->surfaceDrag[0] = 1.0f;
    environment->surfaceTraction[0] = 1.0f;
    if (!found) {
        sprintf(text, "\nCannot find Dust/Dirt data for [%s] in %s.\n\n", "TerrainZone1", scenePath);
        UnknownFunction464e80(text);
    } else {
        int dust;
        int dirt;
        parameters.GetInt("GenerateDust", 1, &dust);
        environment->generateDust[0] = dust;
        parameters.GetInt("GenerateDirt", 1, &dirt);
        environment->generateDirt[0] = dirt;
    }
    int i;
    for (i = 2; i < 9; i++) {
        int dust;
        int dirt;
        sprintf(zone, "TerrainZone%d", i);
        if (!parameters.SelectSection(zone)) {
            sprintf(text, "\nCannot find data for [%s] in %s.\n\n", zone, scenePath);
            UnknownFunction464e80(text);
            environment->surfaceFriction[i - 1] = 1.0f;
            environment->surfaceDrag[i - 1] = 1.0f;
            environment->surfaceTraction[i - 1] = 1.0f;
            environment->generateDust[i - 1] = i != 8;
            environment->generateDirt[i - 1] = i != 8;
            continue;
        }
        if (!parameters.GetFloat("SurfaceFriction", 1.0f, &environment->surfaceFriction[i - 1])) {
            UnknownFunction4de580(zone, "SurfaceFriction", "NOT FOUND");
        } else if (environment->surfaceFriction[i - 1] > 2.0f) {
            environment->surfaceFriction[i - 1] = 2.0f;
        } else if (environment->surfaceFriction[i - 1] < 0.05f) {
            environment->surfaceFriction[i - 1] = 0.05f;
        }
        UnknownFunction4de580(zone, "SurfaceFriction", environment->surfaceFriction[i - 1]);
        if (!parameters.GetFloat("SurfaceDrag", 1.0f, &environment->surfaceDrag[i - 1])) {
            UnknownFunction4de580(zone, "SurfaceDrag", "NOT FOUND");
        } else if (environment->surfaceDrag[i - 1] > 2.0f) {
            environment->surfaceDrag[i - 1] = 2.0f;
        } else if (environment->surfaceDrag[i - 1] < 0.05f) {
            environment->surfaceDrag[i - 1] = 0.05f;
        }
        UnknownFunction4de580(zone, "SurfaceDrag", environment->surfaceDrag[i - 1]);
        if (!parameters.GetFloat("SurfaceTraction", 1.0f, &environment->surfaceTraction[i - 1])) {
            UnknownFunction4de580(zone, "SurfaceTraction", "NOT FOUND");
        } else if (environment->surfaceTraction[i - 1] > 2.0f) {
            environment->surfaceTraction[i - 1] = 2.0f;
        } else if (environment->surfaceTraction[i - 1] < 0.05f) {
            environment->surfaceTraction[i - 1] = 0.05f;
        }
        UnknownFunction4de580(zone, "SurfaceTraction", environment->surfaceTraction[i - 1]);
        parameters.GetInt("GenerateDust", 1, &dust);
        environment->generateDust[i - 1] = dust;
        UnknownFunction4de580(zone, "GenerateDust", (int)environment->generateDust[i - 1]);
        parameters.GetInt("GenerateDirt", 1, &dirt);
        environment->generateDirt[i - 1] = dirt;
        UnknownFunction4de580(zone, "GenerateDirt", (int)environment->generateDirt[i - 1]);
    }
    UnknownFunction464e80("");
    for (i = 0; i < 4; i++)
        environment->enviroID[i] = 0;
    for (i = 0; i < 4; i++) {
        sprintf(zone, "ReverbZone%d", i + 1);
        if (!parameters.SelectSection(zone)) {
            sprintf(text, "\nCannot find ReverbZone data for [%s] in %s.\n\n", zone, scenePath);
            UnknownFunction464e80(text);
        } else {
            parameters.ReadString("EnviroID", 0, environment->enviroID[i]);
        }
    }
    return 1;
}

// 0x004eca20
int Scene::OpenResourceFiles()
{
    char key[0x40];
    char file[0x80];
    char name[0x104];
    char path[0x104];
    char message[0x204];

    int found = parameters.SelectSection("ResourceFiles");
    if (sceneManager->sceneNamed && found) {
        int i = 1;
        sprintf(key, "ResourceFile%d", i);
        while (parameters.GetString(key, "", file, 0x7f)) {
            sprintf(name, "%s\\%s", "Res", file);
            if (!g_TrackGame->UnknownVirtualSlot18(name, path))
                return 0;
            g_UnknownResourceManager572b44->AddArchive(path, 0);
            sprintf(key, "ResourceFile%d", ++i);
        }
        return 1;
    }
    sprintf(message, "\nCannot find data for [%s] in %s.\n\n", "ResourceFiles", scenePath);
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
void Scene::ReadStadium()
{
    hasStadium = parameters.SelectSection("Stadium");
    if (hasStadium) {
        parameters.GetString("FileName", "", stadiumFile, -1);
        parameters.GetFloat("Top", 0, &stadiumTop);
        parameters.GetFloat("Bottom", 0, &stadiumBottom);
        parameters.GetFloat("Scale", 0, &stadiumScale);
        ReadVector(&stadiumPosition, "Stadium", "Position", "0.0,0.0,0.0", 0, 0);
    }
}

// 0x004edf20: attaches a new shadow receiver for `shadow` to each caster and
// adds it as a child.
void Scene::AttachCasterShadows(ProjectedShadow* shadow, int flags)
{
    if (shadow && staticModels) {
        for (int i = 0; i < staticModels->casterCount; i++)
            AppendChild((new(__FILE__, 2136) D3DIMSoultreeShadow(flags))
                                      ->Attach((int)field_0x18, staticModels->casters[i].model,
                                               shadow),
                                  -1);
    }
}

// 0x004ef4c0: reads the "Sounds" section: whether a crowd is present and
// the "StaticSound<n>" sections, each an emitter placed in the scene or a
// plain sound added to field_0xc8. Needs the AuralScape (field_0xc0).
int Scene::ReadSounds()
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

    if (!sceneManager->sceneNamed)
        return 0;
    parameters.SelectSection("Sounds");
    parameters.GetInt("CrowdPresent", 0, &crowdPresent);
    UnknownFunction4de580("Sounds", "CrowdPresent", crowdPresent);
    UnknownFunction464e80("");
    parameters.GetInt("NumStaticSounds", 0, &count);
    if (count > 0) {
        sprintf(position, "%f %f %f", FLT_MAX, FLT_MAX, FLT_MAX);
        for (int i = 1; i <= count; i++) {
            sprintf(section, "StaticSound%d", i);
            parameters.SelectSection(section);
            if (auralScape) {
                if (parameters.GetString("SoundResourceFile", "", file, 0x103) && strcmp(file, "")
                    && _stricmp(file, "NONE")) {
                    parameters.GetString("SoundType", "Emitter", type, 0x103);
                    if (!_stricmp(type, "Emitter")) {
                        ReadSoundSettings(section, file, 1, &params, &flags, &oneShotDistance,
                                              &randomTriggerPercent, &oneShot, &force2D);
                        if (!ReadVector(&params.position, section, "Position", position, 1, 0)) {
                            sprintf(text, "Position not available for StaticSound%d\n", i);
                            UnknownFunction464e80(text);
                        }
                        SoundEmitter* emitter =
                            (new(__FILE__, 2588)
                                 SoundEmitter((AuralScape*)auralScape, soundGroup, 1, field_0x25_bit0))
                                ->UnknownFunction402260(field_0x18, file, params, flags, oneShotDistance,
                                                        randomTriggerPercent, 0, force2D);
                        AppendChild(emitter, -1);
                        if (!emitter) {
                            sprintf(text, "\nScene::SoundEmitter(%s) not created\n", file);
                            UnknownFunction464e80(text);
                        }
                    } else if (!_stricmp(type, "Sound")) {
                        int volume;
                        if (!strstr(file, ".wav"))
                            strcat(file, ".wav");
                        Sound* sound = LoadEntrySound(soundGroup, file, 1, 1, 0, -1);
                        parameters.GetInt("SoundVolume", 0, &volume);
                        sound->SetVolume(volume, 0);
                        sounds.Add(sound);
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
    sprintf(text, "\nNumber of StaticSounds<1 in %s.\n\n", scenePath);
    UnknownFunction464e80(text);
    return 0;
}

// 0x004ef9c0: reads a sound's settings from `section` and appends ".wav" to
// `file` when it has no extension yet.
void Scene::ReadSoundSettings(const char* section, char* file, int is3D,
                                  UnknownSound3DParameters* params, unsigned long* flags,
                                  float* oneShotDistance, float* randomTriggerPercent,
                                  int* oneShot, int* force2D)
{
    memset(params, 0, sizeof(UnknownSound3DParameters));
    if (!strstr(file, ".wav"))
        strcat(file, ".wav");
    parameters.SelectSection(section);
    *flags = is3D ? 2 : 0;
    int looping;
    parameters.GetInt("SoundLooping", 0, &looping);
    if (looping)
        *flags |= 8;
    parameters.GetInt("SoundOneShot", 0, oneShot);
    if (*oneShot)
        *flags |= 1;
    parameters.GetInt("SoundForce2D", 0, force2D);
    parameters.GetFloat("SoundOneShotDistance", 20.0f, oneShotDistance);
    parameters.GetFloat("SoundRandomTriggerPercent", 100.0f, randomTriggerPercent);
    parameters.GetFloat("SoundMaxDistance", 0, &params->maxDistance);
    parameters.GetFloat("SoundMinDistance", 0, &params->minDistance);
    params->position = Vector3(FLT_MAX, FLT_MAX, FLT_MAX);
}

// 0x004f0040
int Scene::UnknownVirtualSlot14()
{
    if (visible)
        return GameObject::UnknownVirtualSlot14();
    return 1;
}

// 0x004f0060: control 5 toggles field_0x7bc; other events go to GameObject.
int Scene::UnknownVirtualSlot22(UnknownControlEvent* event, UnknownInputEntry* entry)
{
    if (TestInputEvent(5, 0, event, 0x80)) {
        visible = 1 - visible;
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
int Scene::Load(void* owner, LightManager* lights, int a3, int a4, int a5,
                                 const char* cubeDirectory, void (*progress)(int), int interval)
{
    int logProgress;
    char path[0x104];

    UnknownTextureStream* archive = sceneManager->OpenSceneArchive(0);
    if (!archive)
        return 0;
    sprintf(scenePath, "%s\\%s", sceneManager->sceneDirectory, sceneManager->sceneFile);
    parameters.OpenFromArchives((UnknownParameterStream*)archive, sceneManager->sceneFile,
                                     scenePath, 1);
    parameters.SelectSection("SceneInfo");
    if (logFile)
        fclose(logFile);
    logProgress = 0;
    parameters.GetInt("LogProgress", 0, &logProgress);
    if (logProgress) {
        sprintf(path, "%s\\%s", sceneManager->sceneDirectory, "scnmgr.log");
        logFile = fopen(path, "w");
    } else {
        logFile = 0;
    }
    sceneManager->ReadCurrentSceneName(sceneName, sceneManager->sceneFile, 0);
    UnknownFunction4de580("SceneInfo", "SceneName", sceneName);
    ReadVector(&defaultStartPosition, "SceneInfo", "DefaultStartPosition", "1152.0,100.0,1152.0", 1, 0);
    ReadVector(&defaultStartDirection, "SceneInfo", "DefaultStartDirection", "0.0,0.0,1.0", 1, 0);
    UnknownFunction464e80("");
    field_0x18 = owner;
    ReadVector(&podiumPosition, "SceneInfo", "PodiumPosition", "0.0,0.0,0.0", 1, 0);
    ReadVector(&podiumDirection, "SceneInfo", "PodiumDirection", "0.0,0.0,0.0", 1, 0);
    if (!ReadEnvironment(sceneManager->sceneDirectory, cubeDirectory))
        return 0;
    parameters.SelectSection("Lights");
    parameters.GetInt("NumberOfLights", -1, &lightCount);
    UnknownFunction4de580("Lights", "NumberOfLights", lightCount);
    UnknownFunction464e80("");
    if (!sceneManager->sceneNamed || lightCount == -1)
        lightCount = 2;
    sceneLights = new(__FILE__, 2784) UnknownSceneLight[lightCount];
    for (int i = 0; i < lightCount; i++) {
        sceneLights[i].emitter = 0;
        if (!ReadLight(i))
            return 0;
        if (sceneLights[i].emitsLight) {
            const char* flare = sceneLights[i].hasLensFlare ? sceneLights[i].lensFlareTexture : 0;
            int camera = sceneLights[i].hasLensFlare ? (int)g_TrackGame->textureManager : 0;
            sceneLights[i].emitter =
                (new(__FILE__, 2798) LightEmitter(1))
                    ->Create(field_0x18, sceneLights[i].type, sceneLights[i].color,
                                            &sceneLights[i].position, sceneLights[i].field_0x34,
                                            sceneLights[i].range, sceneLights[i].showDebugSphere, 0, camera,
                                            (int)flare, i);
            if (!sceneLights[i].emitter)
                return 0;
            lights->AddLight(sceneLights[i].emitter);
        }
    }
    ReadFog();
    OpenResourceFiles();
    ReadStadium();
    {
        int objects = ReadStaticModels(lights, a3, a4, a5, progress, interval);
        if (!ReadAnimations(lights, a4, a5, progress, interval) && !objects)
            UnknownVirtualSlot4();
    }
    ReadSounds();
    return 1;
}

// 0x004eff30: selects the detail table and applies `level` to every entry's
// object and to the shadow casters, re-enabling those without flag 2.
void Scene::SetDetailLevel(int level)
{
    g_UnknownGlobal689f18 = g_TrackGame->field_0x2d0 ? g_UnknownGlobal5744c8
                                                              : g_UnknownGlobal574428;
    if (animationTable) {
        int i;
        for (i = 0; i < animationTable->entryCount; i++) {
            if (animationTable->entries[i].keyFramed)
                animationTable->entries[i].character->field_0x1a0->ApplyLodSettings(level);
            else
                ((UnknownSceneAnimatedObject*)animationTable->entries[i].proceduralObject)
                    ->field_0x34->ApplyLodSettings(level);
        }
        for (i = 0; i < animationTable->randomSetCount; i++) {
            UnknownSceneEntry* entry = animationTable->randomSets[i].entry;
            if (entry->keyFramed)
                entry->character->field_0x1a0->ApplyLodSettings(level);
            else
                ((UnknownSceneAnimatedObject*)entry->proceduralObject)->field_0x34->ApplyLodSettings(level);
        }
    }
    if (staticModels) {
        for (int i = 0; i < staticModels->casterCount; i++) {
            if (staticModels->casters[i].model) {
                ((UnknownSceneLodObject*)staticModels->casters[i].model)->ApplyLodSettings(level);
                if (!staticModels->casters[i].useLighting)
                    ((UnknownSceneLodObject*)staticModels->casters[i].model)->UnknownFunction4444c0(0);
            }
        }
    }
}

// 0x004f00e0
int CountTexture(UnknownSceneTextureInfo* texture, int* counts)
{
    if (!_stricmp(texture->textureName, "PROCEDURAL")) {
        counts[2]++;
        return 1;
    }
    int header0;
    int header1;
    int width = 0;
    UnknownResourceEntry* entry =
        g_UnknownResourceManager572b44->FindItem(texture->textureName, 0);
    if (entry) {
        entry->archive->Seek(entry->offset, 0, 0);
        if (entry->archive->Read(&header0, 4, 1) != 1
            || entry->archive->Read(&header1, 4, 1) != 1
            || entry->archive->Read(&width, 4, 1) != 1)
            return 0;
    } else {
        UnknownTgaFile* file = OpenAndReadHeader(texture->textureName, 0,
                                                     (int)g_UnknownResourceManager572b44);
        if (!file)
            return 0;
        width = file->width;
        FreeFile(file);
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
int AddResource(UnknownResourceManager* manager, const char* name, const char* path)
{
    UnknownResourceEntry* global = g_UnknownResourceManager572b44->FindItem(name, 0);
    if (!manager->FindItem(name, 0)) {
        if (global)
            manager->AddObject(name, 0);
        else
            manager->AddLooseFile(name, path);
        UnknownResourceEntry* entry = manager->FindItem(name, 0);
        if (entry)
            delete entry->archive;
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
int CountModelTextures(const char* path, UnknownSceneResourceManager* resources, unsigned long* counts)
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
    if (!AddResource((UnknownResourceManager*)resources, name, file))
        return 1;
    UnknownResourceEntry* entry = g_UnknownResourceManager572b44->FindItem(name, 0);
    if (!entry) {
        g_UnknownResourceManager572b44->AddLooseFile(name, file);
        entry = g_UnknownResourceManager572b44->FindItem(name, 0);
    }
    if (entry) {
        entry->archive->Seek(entry->offset, 0, 0);
        entry->archive->Read(&count, 4, 1);
        SoultreeObject* node = new(__FILE__, 3447) SoultreeObject(0);
        for (int i = 0; i < count; i++) {
            entry->archive->Read(node->nodeName, 0x80, 1);
            entry->archive->Read(&node->localMatrix, 0x40, 1);
            entry->archive->Read(&node->field_0x14c, 4, 1);
            entry->archive->Read(&node->localBoundsValid, 4, 1);
            entry->archive->Read(&node->localBoundsCenter, 0xc, 1);
            entry->archive->Read(&node->localBoundsHalfExtent, 0xc, 1);
            entry->archive->Read(&node->field_0x170, 4, 1);
            entry->archive->Read(&node->subtreeBoundsCenter, 0xc, 1);
            entry->archive->Read(&node->subtreeBoundsHalfExtent, 0xc, 1);
        }
        node->Release();
        int* parents = (int*)operator new(count * 4, __FILE__, 3466);
        entry->archive->Read(parents, 4, count);
        operator delete(parents);
        entry->archive->Read(&materials, 4, 1);
        if (materials > 0) {
            SoultreeMaterial* material = new(__FILE__, 3472) SoultreeMaterial(0);
            for (int j = 0; j < materials; j++) {
                entry->archive->Read(material->textureName, 0x40, 1);
                entry->archive->Read(&material->textureFormat, 4, 1);
                entry->archive->Read(&material->colorKey, 4, 1);
                entry->archive->Read(&material->sourceBlend, 4, 1);
                entry->archive->Read(&material->destBlend, 4, 1);
                entry->archive->Read(&material->hasTextureName, 4, 1);
                entry->archive->Read(&material->useVertexColor, 4, 1);
                entry->archive->Read(&material->mipMapped, 4, 1);
                entry->archive->Read(&material->hasColorKey, 4, 1);
                entry->archive->Read(&material->hasSourceBlend, 4, 1);
                entry->archive->Read(&material->hasDestBlend, 4, 1);
                entry->archive->Read(&material->field_0xb8, 4, 1);
                entry->archive->Read(&material->clampTexture, 4, 1);
                entry->archive->Read(&material->hasAlpha, 4, 1);
                entry->archive->Read(&material->materialAlpha, 4, 1);
                entry->archive->Read(&material->textureSpeed, 4, 1);
                entry->archive->Read(&material->mappingType, 2, 1);
                entry->archive->Read(&material->field_0xcc, 4, 1);
                position = entry->archive->Tell();
                mode = entry->archive->GetTextMode();
                if (material->hasTextureName) {
                    if (!AddResource((UnknownResourceManager*)resources, material->textureName, 0))
                        return 1;
                    if (!CountTexture((UnknownSceneTextureInfo*)material, (int*)counts))
                        return 0;
                }
                entry->archive->Seek(position, 0, 0);
                entry->archive->SetTextMode(mode);
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
    if (!AddResource((UnknownResourceManager*)resources, name, file))
        return 1;
    if (!g_UnknownResourceManager572b44->FindItem(name, 0)) {
        g_UnknownResourceManager572b44->AddLooseFile(name, file);
        if (!g_UnknownResourceManager572b44->FindItem(name, 0))
            return 1;
    }
    UnknownTextureStream* stream = new(__FILE__, 3622) UnknownTextureStream((int)g_UnknownResourceManager572b44);
    stream->Open(name, "rb", 0);
    UnknownParameterBlock block;
    block.Attach((UnknownParameterStream*)stream, 0, 1);
    block.SelectSection("Materials");
    block.GetInt("NumberOfMaterials", 0, &materials);
    SoultreeMaterial material(0);
    for (int k = 0; k < materials; k++) {
        sprintf(section, "Material - %d", k);
        block.SelectSection(section);
        if (block.ReadString("TextureMap", material.textureName, -1)) {
            if (!AddResource((UnknownResourceManager*)resources, material.textureName, 0)) {
                delete stream;
                return 1;
            }
            if (!CountTexture((UnknownSceneTextureInfo*)&material, (int*)counts)) {
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
int Scene::CountObjects(int* count)
{
    UnknownTextureStream* archive = sceneManager->OpenSceneArchive(0);
    if (!archive)
        return 0;
    int animations;
    UnknownSceneResourceManager resources;
    char message[0x204];
    sprintf(scenePath, "%s\\%s", sceneManager->sceneDirectory, sceneManager->sceneFile);
    parameters.OpenFromArchives((UnknownParameterStream*)archive, sceneManager->sceneFile,
                                     scenePath, 1);
    *count = 1;
    int found = parameters.SelectSection("StaticModels");
    if (!sceneManager->sceneNamed || !found) {
        sprintf(message, "\nNo scene file OR cannot find data for [%s] in %s.\n\n",
                "StaticModels", scenePath);
        UnknownFunction464e80(message);
    }
    if (!parameters.GetInt("NumberOfStaticModels", 1, count)) {
        sprintf(message, "\nNumberOfStaticModels cannot be found under [%s] in %s.\n\n",
                "StaticModels", scenePath);
        UnknownFunction464e80(message);
    }
    animations = 0;
    if (!parameters.GetInt("NumberOfAnimations", 0, &animations)) {
        sprintf(message, "\nNumberOfAnimations cannot be found under [%s] in %s.\n\n",
                "Animations", scenePath);
        UnknownFunction464e80(message);
    }
    *count += animations;
    *count = *count > 1 ? *count : 1;
    return 1;
}

// 0x004f0ec0: replaces `path` (a model file) with the SLT file its "General
// info" section names, prefixed with its content directory.
int Scene::ResolveSltPath(char* path)
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
    if (!g_UnknownResourceManager572b44->FindItem(name, 0)) {
        g_UnknownResourceManager572b44->AddLooseFile(name, path);
        if (!g_UnknownResourceManager572b44->FindItem(name, 0)) {
            delete stream;
            return 0;
        }
    }
    stream->Open(name, "rb", 0);
    UnknownParameterBlock block;
    block.Attach((UnknownParameterStream*)stream, 0, 1);
    block.SelectSection("General info");
    if (!block.ReadString("SLTFile", sltFile, -1)) {
        delete stream;
        return 0;
    }
    block.GetString("ContentDirectory", "", content, -1);
    strcpy(path, content);
    strcat(path, sltFile);
    delete stream;
    return 1;
}

// 0x004f1130: see the declaration. Model and animation files are relative to
// "Res"; key-framed animations name an MCF file whose SLT file 0x004f0ec0
// looks up.
int Scene::CountTextures(unsigned long* counts, float* width, int* hasCube, char* ecosystem)
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

    UnknownTextureStream* archive = sceneManager->OpenSceneArchive(0);
    if (!archive)
        return 0;
    UnknownSceneResourceManager resources;
    sprintf(scenePath, "%s\\%s", sceneManager->sceneDirectory, sceneManager->sceneFile);
    parameters.OpenFromArchives((UnknownParameterStream*)archive, sceneManager->sceneFile,
                                     scenePath, 1);
    OpenResourceFiles();
    int i = 0;
    models = 0;
    int found = parameters.SelectSection("StaticModels");
    if (sceneManager->sceneNamed && found) {
        if (!parameters.GetInt("NumberOfStaticModels", -1, &models)) {
            sprintf(text, "\nNumberOfStaticModels cannot be found under [%s] in %s.\n\n", "StaticModels",
                    scenePath);
            UnknownFunction464e80(text);
        } else if (models > i) {
            for (i = 0; i < models;) {
                sprintf(model, "Model%d", ++i);
                parameters.SelectSection(model);
                if (!parameters.GetString("SLT", "", file, 0x104)) {
                    sprintf(modelMessage, "\nCannot find %s for [%s] in %s.\n\n", file, model, scenePath);
                    UnknownFunction464e80(modelMessage);
                    return 0;
                }
                sprintf(modelPath, "%s\\%s", "Res", file);
                if (!CountModelTextures(modelPath, &resources, counts))
                    return 0;
            }
        }
    } else {
        sprintf(text, "\nCannot find data for [%s] in %s.\n\n", "StaticModels", scenePath);
        UnknownFunction464e80(text);
    }
    i = 0;
    animations = 0;
    found = parameters.SelectSection("Animations");
    if (sceneManager->sceneNamed && found) {
        if (!parameters.GetInt("NumberOfAnimations", -1, &animations)) {
            sprintf(text, "\nNumberOfAnimations cannot be found under [%s] in %s.\n\n", "Animations",
                    scenePath);
            UnknownFunction464e80(text);
        } else if (animations > i) {
            for (i = 0; i < animations; i = next) {
                int keyFramed;
                next = i + 1;
                sprintf(animation, "Animation%d", next);
                parameters.SelectSection(animation);
                if (!parameters.GetString("AnimationType", "", text, 0x40)) {
                    sprintf(message, "\nScene: AnimationType not specified for anim#%d.  Using KeyFramed.\n", i);
                    UnknownFunction464e80(message);
                    keyFramed = 1;
                } else if (!_strnicmp(text, "Procedural", 3)) {
                    keyFramed = 0;
                } else {
                    keyFramed = 1;
                }
                if (keyFramed) {
                    if (!parameters.GetString("MCF", "", file, 0x104)) {
                        sprintf(message, "\nCannot find %s for [%s] in %s.\n\n", "MCF", animation, scenePath);
                        UnknownFunction464e80(message);
                        return 0;
                    }
                } else if (!parameters.GetString("SLT", "", file, 0x104)) {
                    sprintf(message, "\nCannot find %s for [%s] in %s.\n\n", "SLT", animation, scenePath);
                    UnknownFunction464e80(message);
                    return 0;
                }
                sprintf(animationPath, "%s\\%s", "Res", file);
                if (keyFramed) {
                    UnknownFunction4de580(animation, "MCF", animationPath);
                    ResolveSltPath(animationPath);
                }
                UnknownFunction4de580(animation, "SLT", animationPath);
                if (!CountModelTextures(animationPath, &resources, counts))
                    return 0;
            }
        }
    } else {
        sprintf(text, "\nCannot find data for [%s] in %s.\n\n", "Animations", scenePath);
        UnknownFunction464e80(text);
    }
    found = parameters.SelectSection("Environment");
    if (sceneManager->sceneNamed && found) {
        sprintf(ecosystem, "%s\\%s", sceneManager->sceneDirectory, sceneManager->estFile);
        if (parameters.GetString("TerrainFile", "", text, 0x100)) {
            strcpy(strrchr(text, '.'), ".est");
            sprintf(ecosystem, "%s\\%s", sceneManager->sceneDirectory, text);
        }
        if (parameters.GetString("EcosystemFile", "", text, 0x100))
            sprintf(ecosystem, "%s\\%s", sceneManager->sceneDirectory, text);
        parameters.GetFloat("TerrainWidth", 5.0f, width);
        *hasCube = parameters.GetString("CubeFile", "", text, 0x104);
        return 1;
    }
    *width = 0;
    *hasCube = 0;
    return 0;
}
