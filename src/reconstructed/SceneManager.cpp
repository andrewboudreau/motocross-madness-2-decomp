#include <float.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "AuralScape.h"
#include "LightEmitter.h"
#include "Net.h"
#include "PCAudio.h"
#include "TextureMap.h"
#include "Tgafile.h"
#include "SceneManager.h"
#include "../krusty2/motion/CollisionCharacter.h"
#include "CarProcedural.h"

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
SceneArchiveFinder::SceneArchiveFinder()
{
    sceneDirectory[0] = 0;
    sceneFile[0] = 0;
    trnFile[0] = 0;
    cubeFile[0] = 0;
    waypointFile[0] = 0;
    waypointPath[0] = 0;
    sceneNamed = 0;
    sceneArchive = 0;
    sceneArchiveName[0] = 0;
    waypointRaceCount = 0;
}

// 0x004e9a10
int SceneArchiveFinder::CountSceneObjects(int* count)
{
    Scene* scene = (new(__FILE__, 115) Scene(0))->AttachSceneManager(this, 0, 0, 0);
    if (!scene)
        return 0;
    scene->CountObjects(count);
    scene->Release();
    return 1;
}

// 0x004e9ac0
int SceneArchiveFinder::CountSceneTextures(unsigned long* counts, char* width, char* hasCube,
                                                  int ecosystem)
{
    counts[0] = 0;
    counts[1] = 0;
    counts[2] = 0;
    counts[3] = 0;
    Scene* scene = (new(__FILE__, 151) Scene(0))->AttachSceneManager(this, 0, 0, 0);
    if (!scene)
        return 0;
    int result = scene->CountTextures(counts, (float*)width, (int*)hasCube, (char*)ecosystem);
    scene->Release();
    return result;
}

// 0x004e9b80
void SceneArchiveFinder::SetSceneDirectory(char* path)
{
    strncpy(sceneDirectory, path, 0x103);
}

// 0x004e9ba0: keeps the open archive when it already is `name`.
FileStream* SceneArchiveFinder::OpenSceneArchive(const char* name)
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
        g_ResourceManager->RemoveArchiveItems(sceneArchive);
        delete sceneArchive;
    }
    sceneArchive = g_ResourceManager->AddArchive(path, 1);
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
int SceneArchiveFinder::OpenArchiveFile(FileStream* stream, const char* name,
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
        if (stream->innerStream) {
            if (out)
                strcpy(out, file);
            return 1;
        }
        delete stream;
        stream = new(__FILE__, 268) FileStream(g_ResourceManager);
        if (out)
            *out = 0;
    } else {
        if (out)
            *out = 0;
    }
    return 0;
}

// 0x004e9e30: selects scene `name`.`kind` and reads whether it is named.
void SceneArchiveFinder::SelectScene(char* name, const char* kind, int value)
{
    char sceneName[0x40];
    char buffer[0x104];

    sprintf(buffer, "%s.%s", name, kind);
    strncpy(sceneFile, buffer, 0x3f);
    SetSceneFileNames(name);
    OpenSceneArchive(0);
    FileStream* stream =
        new(__FILE__, 297) FileStream(g_ResourceManager);
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
void SceneArchiveFinder::SetSceneFileNames(const char* name)
{
    char buffer[0x104];

    sprintf(buffer, "%s.trn", name);
    strncpy(trnFile, buffer, 0x1f);
    strncpy(cubeFile, "", 0x1f);
    sprintf(buffer, "%s01.wpt", name);
    strncpy(waypointFile, buffer, 0x1f);
    sprintf(buffer, "%s.est", name);
    strncpy(estFile, buffer, 0x1f);
}

// 0x004ea010: as 0x004ea390 for scene `name` (with extension `kind` when
// it has none), opened through 0x004e9ba0.
void SceneArchiveFinder::ReadSceneName(char* out, char* name, int index,
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
        ParameterBlock block;
        FileStream* archive = OpenSceneArchive(scene);
        if (archive) {
            archive->GetFileTimes(a, b);
            block.OpenFromArchives((FileStream*)archive, scene, path, 0);
            block.SelectSection("SceneInfo");
            block.GetInt("NumberOfWaypointRaces", 0, &waypointRaceCount);
            if (index) {
                if (waypointRaceCount) {
                    FileStream* stream = new(__FILE__, 401)
                        FileStream(g_ResourceManager);
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
void SceneArchiveFinder::ReadCurrentSceneName(char* name, char* scene, int value)
{
    char path[0x104];
    char base[0x104];

    if (sceneDirectory[0]) {
        sprintf(path, "%s\\%s", sceneDirectory, scene);
        ParameterBlock block;
        block.OpenFromArchives((FileStream*)OpenSceneArchive(0), scene, path, 1);
        block.SelectSection("SceneInfo");
        block.GetInt("NumberOfWaypointRaces", 0, &waypointRaceCount);
        if (value) {
            if (waypointRaceCount) {
                FileStream* stream =
                    new(__FILE__, 471) FileStream(g_ResourceManager);
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
    groundProbeMask = 0;
    stadiumFile[0] = 0;
    logFile = 0;
    visible = 1;
    holdAnimations = 0;
}

// 0x004ea7e0
Scene* Scene::AttachSceneManager(SceneArchiveFinder* manager, int value, int,
                                    char probeMask)
{
    sceneManager = manager;
    auralScape = value;
    groundProbeMask = probeMask;
    soundGroup = new(__FILE__, 622) SoundGroup(isEnabled);
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
void Scene::Enable()
{
    for (int i = 0; i < sounds.m_count; i++) {
        Sound* sound = sounds.Get(i);
        if (sound)
            sound->PlayWithOptions(0, 1, 0);
    }
    GameObject::Enable();
}

// 0x004eaab0: as slot 5, before the GameObject slot 7.
void Scene::OnEnable()
{
    for (int i = 0; i < sounds.m_count; i++) {
        Sound* sound = sounds.Get(i);
        if (sound)
            sound->PlayWithOptions(0, 1, 0);
    }
    GameObject::OnEnable();
}

// 0x004eaec0
CollisionCharacter* Scene::FindEntryByFileName(const char* path, int* index)
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
            !_strnicmp(animationTable->entries[i].character->sltFileName, name,
                       strcspn(animationTable->entries[i].character->sltFileName, "."))) {
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
void Scene::EnableAutomaticSequence(int index)
{
    if (animationTable && index < animationTable->entryCount)
        animationTable->entries[index].automaticSequence = 1;
}

// 0x004eb000
void Scene::RestartAllEntries(float time)
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
        SceneAnimation* entry = &animationTable->entries[index];
        if (entry->keyFramed) {
            CollisionCharacter* object = entry->character;
            if (object->motionStopped || force) {
                if (motion >= entry->sequenceLength)
                    entry->sequenceIndex = 0;
                else
                    entry->sequenceIndex = motion;
                animationTable->entries[index].currentMotion =
                    animationTable->entries[index].motionSequence[animationTable->entries[index].sequenceIndex] - 1;
                object->SetMotion(
                    (Motion*)animationTable->entries[index].motions[animationTable->entries[index].currentMotion]);
                object->motionTime = 0;
                animationTable->entries[index].motionFrame = object->AdvanceMotion(time, 0, 0);
                return 1;
            }
        } else {
            CarProcedural* object = entry->proceduralObject;
            object->pathTime = time;
            object->skipSmoothing = running;
            if (running)
                animationTable->entries[index].proceduralObject->Tick(0.0001f);
        }
    }
    return 0;
}

// 0x004eb160
int Scene::ReadVector(Vector3* out, const char* section, const char* key,
                                 const char* def, int useDefault, ParameterBlock* block)
{
    char value[0x184];

    if (!block)
        block = &parameters;
    block->SelectSection(section);
    if (!sceneManager->sceneNamed) {
        block->GetString(key, def, value, 0x80);
        LogParameter(section, key, value);
        out->x = (float)atof(strtok(value, ","));
        out->y = (float)atof(strtok(0, ","));
        out->z = (float)atof(strtok(0, "\n"));
        return 1;
    }
    if (block->GetString(key, "", value, 0x80)) {
        LogParameter(section, key, value);
        out->x = (float)atof(strtok(value, ","));
        out->y = (float)atof(strtok(0, ","));
        out->z = (float)atof(strtok(0, "\n"));
        return 1;
    }
    if (useDefault && *def) {
        strcpy(value, def);
        LogParameter(section, key, value);
        out->x = (float)atof(strtok(value, ","));
        out->y = (float)atof(strtok(0, ","));
        out->z = (float)atof(strtok(0, "\n"));
    }
    return 0;
}

// 0x004eb300
int Scene::ReadColor(unsigned long* out, const char* section, const char* key,
                                 const char* def, int, ParameterBlock* block)
{
    char value[0x184];

    if (!block)
        block = &parameters;
    block->SelectSection(section);
    if (!sceneManager->sceneNamed) {
        block->GetString(key, def, value, 0x80);
        LogParameter(section, key, value);
        int r = atoi(strtok(value, ","));
        int g = atoi(strtok(0, ","));
        int b = atoi(strtok(0, "\n"));
        *out = ((r << 8 | g) << 8) | b;
        return 1;
    }
    if (block->GetString(key, def, value, 0x80)) {
        LogParameter(section, key, value);
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
                                 const char* def, int, ParameterBlock* block)
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
        SceneLight* light = &sceneLights[index];
        parameters.GetString("Type", "BLANK", text, 0x100);
        if (FindKeyword(&light->type, text, s_lightTypes)) {
            LogParameter(section, "Type", text);
            if (!ReadColor(&light->color, section, "ColorRGB", "", 1, 0))
                return 0;
            light->effectivePosition = &light->position;
            if (!ReadVector(&light->position, section, "Position", "", 0, 0)) {
                light->position = Vector3(-2000.0f, 4000.0f, 1252.0f);
                if (light->type != 6 && light->type != 7)
                    LogParameter(section, "Position(Default)", light->effectivePosition);
                else
                    light->effectivePosition = 0;
            }
            parameters.GetInt("PlaceLightAtInfinity", 0, &atInfinity);
            light->atInfinity = atInfinity;
            LogParameter(section, "PlaceLightAtInfinity", (int)light->atInfinity);
            if (light->atInfinity) {
                light->direction = light->position * (80000.0f / Length(light->position));
                light->effectivePosition = &light->direction;
            }
            light->emitterDirection = &light->look;
            if (ReadVector(&vector, section, "LookVector", "", 0, 0)) {
                light->look = vector;
            } else if (ReadVector(&vector, section, "TargetPosition", "", 0, 0)) {
                light->look = Normalize(vector - light->position);
            } else {
                light->look = Vector3(0.0f, 0.0f, 0.0f);
                light->emitterDirection = 0;
            }
            parameters.GetFloat("Range", -1.0f, &light->range);
            float range = light->range;
            if (range == -1.0) {
                light->range = 0.0f;
                if (light->type != 6 && light->type != 7)
                    LogParameter(section, "Range(Default)", 0.0f);
            } else {
                LogParameter(section, "Range", range);
            }
            parameters.GetInt("ShowDebugSphere", 0, &showDebugSphere);
            light->showDebugSphere = showDebugSphere;
            LogParameter(section, "ShowDebugSphere", (int)light->showDebugSphere);
            parameters.GetInt("CastsShadows", 0, &castsShadows);
            light->castsShadows = castsShadows;
            LogParameter(section, "CastsShadows", (int)light->castsShadows);
            parameters.GetInt("EmitsLight", 1, &emitsLight);
            light->emitsLight = emitsLight;
            LogParameter(section, "EmitsLight", (int)light->emitsLight);
            parameters.GetInt("HasLensFlare", 0, &hasLensFlare);
            light->hasLensFlare = hasLensFlare;
            LogParameter(section, "HasLensFlare", (int)light->hasLensFlare);
            if (light->hasLensFlare) {
                ReadColor(&light->lensFlareColor, section, "LensFlareColorRGB", "", 0, 0);
                parameters.GetFloat("LensFlareBrightness", -1.0f, &light->lensFlareBrightness);
                float brightness = light->lensFlareBrightness;
                if (brightness == -1.0) {
                    light->lensFlareBrightness = 1.0f;
                    LogParameter(section, "LensFlareBrightness(Default)", 1.0f);
                } else {
                    LogParameter(section, "LensFlareBrightness", brightness);
                }
                if (ReadFlags(light->layersVisible, 5, section, "LensFlareLayersVisible",
                                          "T,F,F,F,F", 0, 0)) {
                    sprintf(text, "[%s].LensFlareLayersVisible=%s,%s,%s,%s,%s", section,
                            TF(light->layersVisible[0]), TF(light->layersVisible[1]),
                            TF(light->layersVisible[2]), TF(light->layersVisible[3]),
                            TF(light->layersVisible[4]));
                    LogMessage(text);
                }
                if (parameters.GetString("LensFlareTextureMapName", "", light->lensFlareTexture,
                                                      0x3f))
                    LogParameter(section, "LensFlareTextureMapName", light->lensFlareTexture);
                else
                    strncpy(light->lensFlareTexture, "overlay\\lensflare.tga", 0x3f);
            }
            LogMessage("");
            return 1;
        }
        LogParameter(section, "Type", text);
        return 0;
    }
    SceneLight* light = &sceneLights[index];
    if (index == 0) {
        light->type = 6;
        light->color = 0x5a5a5a;
        light->effectivePosition = 0;
        light->look = Vector3(0.0f, 0.0f, 0.0f);
        light->emitterDirection = 0;
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
        light->emitterDirection = 0;
        light->look = Vector3(0.0f, 0.0f, 0.0f);
        light->effectivePosition = &light->position;
        light->range = 0;
        light->showDebugSphere = 0;
        light->emitsLight = 1;
        light->castsShadows = 1;
        light->hasLensFlare = 0;
    }
    LogMessage("No Scene File or no light data:  Use DEFAULT Lights (1 Ambient, 1 ObjectPoint "
                          "shadowcaster\n");
    return 1;
}

// 0x004ebdb0: creates fog from the "Fog" section. Retail range-checks
// the visibility value again where it reads the haziness.
int Scene::ReadFog()
{
    float visibility;
    float haziness;

    fog = new(__FILE__, 1392) SceneFog;
    fog->color = 0x8080c0;
    fog->visibility = 1.0f;
    fog->haziness = 0.5f;
    int found = parameters.SelectSection("Fog");
    if (!sceneManager->sceneNamed || !found) {
        LogMessage("No Scene File or no fog data:  Using DEFAULT Fog\n");
        return 1;
    }
    visibility = 1.0f;
    if (!parameters.GetFloat("Visibility", 1.0f, &visibility)) {
        LogParameter("Fog", "Visibility", "NOT FOUND using default 1.0");
    } else if (visibility < 0.0f || visibility > 1.0f) {
        LogParameter("Fog", "Visibility",
                              "Invalid value, must be between 0.0 and 1.0, using default 1.0");
        visibility = 1.0f;
    }
    fog->visibility = visibility;
    LogParameter("Fog", "Visibility", fog->visibility);
    if (!parameters.GetFloat("Haziness", 0.5f, &haziness)) {
        LogParameter("Fog", "Haziness", "NOT FOUND using default 0.5");
    } else if (visibility < 0.0f || visibility > 1.0f) {
        LogParameter("Fog", "Haziness",
                              "Invalid value, must be between 0.0 and 1.0, using default 0.5");
        haziness = 0.5f;
    }
    fog->haziness = haziness;
    LogParameter("Fog", "Haziness", fog->haziness);
    if (!ReadColor(&fog->color, "Fog", "ColorRGB", "", 1, 0))
        return 0;
    LogMessage("");
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
        LogMessage(text);
        return 0;
    }
    environment = new(__FILE__, 1456) SceneEnvironment;
    sprintf(environment->terrainFile, "%s\\%s", directory, sceneManager->trnFile);
    sprintf(environment->ecosystemFile, "%s\\%s", directory, sceneManager->estFile);
    if (sceneManager->cubeFile[0])
        sprintf(environment->cubeFile, "%s\\%s", cubeDirectory, sceneManager->cubeFile);
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
        LogParameter("Environment", "TerrainFile", environment->terrainFile);
        LogParameter("Environment", "EcosystemFile", environment->terrainFile);
    } else {
        LogParameter("Environment", "TerrainFile(Default)", environment->terrainFile);
        LogParameter("Environment", "EcosystemFile(Default)", environment->ecosystemFile);
        if (sceneManager->sceneNamed)
            return 0;
    }
    if (parameters.GetString("EcosystemFile", "", text, 0x100)) {
        sprintf(environment->ecosystemFile, "%s\\%s", directory, text);
        strncpy(sceneManager->estFile, text, 0x3f);
        LogParameter("Environment", "EcosystemFile", environment->terrainFile);
    }
    if (parameters.GetString("CubeFile", "", text, 0x104)) {
        sprintf(environment->cubeFile, "%s\\%s", cubeDirectory, text);
        strncpy(sceneManager->cubeFile, text, 0x3f);
        LogParameter("Environment", "CubeFile", environment->cubeFile);
    }
    if (parameters.GetString("ParticleTextureMapName", "", text, 0x100)) {
        strncpy(environment->particleTexture, text, 0x3f);
        LogParameter("Environment", "ParticleTextureMapName", environment->particleTexture);
    } else if (sceneManager->sceneNamed) {
        LogDefaultParameter("Environment", "ParticleTextureMapName", "dirtpart.tga");
    }
    if (parameters.GetString("DetailTextureMapName", "", text, 0x100)) {
        strncpy(environment->detailTexture, text, 0x3f);
        LogParameter("Environment", "DetailTextureMapName", environment->particleTexture);
    } else if (sceneManager->sceneNamed) {
        LogDefaultParameter("Environment", "DetailTextureMapName", "graynoise.tga");
    }
    if (!parameters.GetFloat("TerrainWidthScale", 3.0f, &environment->terrainWidthScale))
        LogParameter("Environment", "TerrainWidthScale(Default)", environment->terrainWidthScale);
    else
        LogParameter("Environment", "TerrainWidthScale", environment->terrainWidthScale);
    if (!parameters.GetFloat("TerrainBreadthScale", 3.0f, &environment->terrainBreadthScale))
        LogParameter("Environment", "TerrainBreadthScale(Default)", environment->terrainBreadthScale);
    else
        LogParameter("Environment", "TerrainBreadthScale", environment->terrainBreadthScale);
    if (!parameters.GetFloat("TerrainWidth", 5.0f, &environment->terrainWidth))
        LogParameter("Environment", "TerrainWidth(Default)", environment->terrainWidth);
    else
        LogParameter("Environment", "TerrainWidth", environment->terrainWidth);
    if (!parameters.GetFloat("TerrainBreadth", 5.0f, &environment->terrainBreadth))
        LogParameter("Environment", "TerrainBreadth(Default)", environment->terrainBreadth);
    else
        LogParameter("Environment", "TerrainBreadth", environment->terrainBreadth);
    if (!parameters.GetFloat("StartGateScale", 1.0f, &environment->startGateScale))
        LogParameter("Environment", "StartGateScale(Default)", environment->startGateScale);
    else
        LogParameter("Environment", "StartGateScale", environment->startGateScale);
    found = parameters.SelectSection("TerrainZone1");
    environment->surfaceFriction[0] = 1.0f;
    environment->surfaceDrag[0] = 1.0f;
    environment->surfaceTraction[0] = 1.0f;
    if (!found) {
        sprintf(text, "\nCannot find Dust/Dirt data for [%s] in %s.\n\n", "TerrainZone1", scenePath);
        LogMessage(text);
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
            LogMessage(text);
            environment->surfaceFriction[i - 1] = 1.0f;
            environment->surfaceDrag[i - 1] = 1.0f;
            environment->surfaceTraction[i - 1] = 1.0f;
            environment->generateDust[i - 1] = i != 8;
            environment->generateDirt[i - 1] = i != 8;
            continue;
        }
        if (!parameters.GetFloat("SurfaceFriction", 1.0f, &environment->surfaceFriction[i - 1])) {
            LogParameter(zone, "SurfaceFriction", "NOT FOUND");
        } else if (environment->surfaceFriction[i - 1] > 2.0f) {
            environment->surfaceFriction[i - 1] = 2.0f;
        } else if (environment->surfaceFriction[i - 1] < 0.05f) {
            environment->surfaceFriction[i - 1] = 0.05f;
        }
        LogParameter(zone, "SurfaceFriction", environment->surfaceFriction[i - 1]);
        if (!parameters.GetFloat("SurfaceDrag", 1.0f, &environment->surfaceDrag[i - 1])) {
            LogParameter(zone, "SurfaceDrag", "NOT FOUND");
        } else if (environment->surfaceDrag[i - 1] > 2.0f) {
            environment->surfaceDrag[i - 1] = 2.0f;
        } else if (environment->surfaceDrag[i - 1] < 0.05f) {
            environment->surfaceDrag[i - 1] = 0.05f;
        }
        LogParameter(zone, "SurfaceDrag", environment->surfaceDrag[i - 1]);
        if (!parameters.GetFloat("SurfaceTraction", 1.0f, &environment->surfaceTraction[i - 1])) {
            LogParameter(zone, "SurfaceTraction", "NOT FOUND");
        } else if (environment->surfaceTraction[i - 1] > 2.0f) {
            environment->surfaceTraction[i - 1] = 2.0f;
        } else if (environment->surfaceTraction[i - 1] < 0.05f) {
            environment->surfaceTraction[i - 1] = 0.05f;
        }
        LogParameter(zone, "SurfaceTraction", environment->surfaceTraction[i - 1]);
        parameters.GetInt("GenerateDust", 1, &dust);
        environment->generateDust[i - 1] = dust;
        LogParameter(zone, "GenerateDust", (int)environment->generateDust[i - 1]);
        parameters.GetInt("GenerateDirt", 1, &dirt);
        environment->generateDirt[i - 1] = dirt;
        LogParameter(zone, "GenerateDirt", (int)environment->generateDirt[i - 1]);
    }
    LogMessage("");
    for (i = 0; i < 4; i++)
        environment->enviroID[i] = 0;
    for (i = 0; i < 4; i++) {
        sprintf(zone, "ReverbZone%d", i + 1);
        if (!parameters.SelectSection(zone)) {
            sprintf(text, "\nCannot find ReverbZone data for [%s] in %s.\n\n", zone, scenePath);
            LogMessage(text);
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
            if (!g_TrackGame->FindDataFile(name, path))
                return 0;
            g_ResourceManager->AddArchive(path, 0);
            sprintf(key, "ResourceFile%d", ++i);
        }
        return 1;
    }
    sprintf(message, "\nCannot find data for [%s] in %s.\n\n", "ResourceFiles", scenePath);
    LogMessage(message);
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

// SoultreeMaterial.h comes in here, after the earlier scene and light parsers:
// declaration order affects VC6's ReadLight code.  The static-model loader and the
// texture-accounting functions below construct and inspect materials.
#include "SoultreeMaterial.h"

// The particle emitters a physics model may carry (0x004ecd60) and the
// collision object of a static model: DustParticleEmitter, DirtChunkParticleEmitter
// and DirtSprayParticleEmitter (ParticleEmitters.h) and CollisionObject
// (src/krusty2/collision).
#include "ParticleEmitters.h"
#include "../krusty2/collision/CollisionObject.h"

// The physics part of a "PhysicsObject" model (0x524 bytes, constructor
// 0x005037c0): slot 37 attaches a particle emitter, slot 40 loads the model and
// returns the GameObject to add.
#include "../krusty2/soultree/SoultreePhysicsObject.h"

// 0x0043a330 (cdecl, CollisionPoint.cpp): adds a collision point at
// `position` to `points` (`*count` of `capacity`).
void* AddCollisionPoint(int capacity, void** points, const Vector3* position, void* owner, float threshold,
                        int* count, float friction, int surfaceOwner);
// 0x004b5d00 (cdecl): the look and up vectors of a heading and pitch.
int OrientationAxesFromAngles(Vector3* look, Vector3* up, float heading, float pitch, float roll);

// An "ObjectShape" (0x24 bytes) or "EmitterType<n>" (0x1c bytes) keyword.
struct SceneObjectShapeKeyword {
    char name[0x20];
    int value;
};
struct SceneEmitterTypeKeyword {
    char name[0x18];
    int value;
};

// The value of keyword `name` (0 when not listed).
static inline int FindShape(const char* name, const SceneObjectShapeKeyword* table)
{
    for (int i = 0; i < 2; i++) {
        if (!_stricmp(name, table[i].name))
            return table[i].value;
    }
    return 0;
}
static inline int FindEmitter(const char* name, const SceneEmitterTypeKeyword* table)
{
    for (int i = 0; i < 4; i++) {
        if (!_stricmp(name, table[i].name))
            return table[i].value;
    }
    return 0;
}

// 0x004ecd60: see the declaration. A model whose ".seg" name is the
// stadium's gets no collision object; its ".col" file is used when the
// archive has one.
int Scene::ReadStaticModels(LightManager* lights, int particleManager, int textureManager, const TextureContext* modelTextures, void (*progress)(int),
                                 int interval)
{
    int models;
    int count;
    int next;
    int stadium;
    int points;
    int emitters;
    int minPoints;
    int physics;
    int collision;
    int useLighting;
    int oneShot;
    int force2D;
    unsigned long flags;
    float radius;
    float heading;
    float pitch;
    float friction;
    float oneShotDistance;
    float randomTriggerPercent;
    Vector3 position;
    Vector3 look;
    Vector3 up;
    DS3DBuffer params;
    char model[0x44];
    char key[0x40];
    char text[0x204];
    char slt[0x108];
    char sound[0x104];
    char sltPath[0x104];
    char message[0x204];
    char path[0x104];
    char colPath[0x104];

    models = 0;
    count = 0;
    staticModels = 0;
    int found = parameters.SelectSection("StaticModels");
    if (sceneManager->sceneNamed && found) {
        if (!parameters.GetInt("NumberOfStaticModels", -1, &models)) {
            sprintf(text, "\nNumberOfStaticModels cannot be found under [%s] in %s.\n\n", "StaticModels",
                    scenePath);
            LogMessage(text);
            return 0;
        }
        if (models > 0) {
    FileStream* stream =
        new(__FILE__, 1749) FileStream(g_ResourceManager);
    staticModels = new(__FILE__, 1751) SceneStaticModels;
    staticModels->casterCount = models;
    staticModels->casters = new(__FILE__, 1753) SceneStaticModel[staticModels->casterCount];
    for (int i = 0; i < staticModels->casterCount; i = next) {
        if (i % interval == interval - 1 && progress)
            progress(0);
        next = i + 1;
        sprintf(model, "Model%d", next);
        parameters.SelectSection(model);
        if (!parameters.GetString("SLT", "", slt, 0x104)) {
            sprintf(message, "\nCannot find %s for [%s] in %s.\n\n", slt, model, scenePath);
            LogMessage(message);
            return 0;
        }
        strcpy(path, slt);
        stadium = 0;
        strcpy(strrchr(path, '.'), ".seg");
        if (!_stricmp(stadiumFile, path))
            stadium = 1;
        sprintf(sltPath, "%s\\%s", "Res", slt);
        strcpy(strrchr(slt, '.'), ".col");
        sprintf(path, "%s\\%s", "Res", slt);
        if (!g_TrackGame->sceneObject->OpenArchiveFile(stream, path, "rb", (int)colPath)) {
            strcpy(strrchr(slt, '.'), "");
            colPath[0] = 0;
        }
        ReadVector(&position, model, "Position", "0.0,0.0,0.0", 0, 0);
        if (parameters.GetFloat("Heading", -999.0f, &heading)) {
            parameters.GetFloat("Pitch", 0, &pitch);
            OrientationAxesFromAngles(&look, &up, heading * 0.01745329f, pitch * 0.01745329f, 0.0f);
        } else {
            ReadVector(&look, model, "LookVector", "0.0,0.0,1.0", 0, 0);
            ReadVector(&up, model, "UpVector", "0.0,1.0,0.0", 0, 0);
        }
        staticModels->casters[i].collisionObject = 0;
        parameters.GetInt("PhysicsObject", 0, &physics);
        if (physics) {
            parameters.GetInt("NumberOfCollisionPoints", 0, &points);
            parameters.GetInt("NumberOfEmitters", 0, &emitters);
            parameters.GetInt("MinCollisionPointsToRestOn", 1, &minPoints);
            parameters.GetFloat("Radius", 1.0f, &radius);
            parameters.GetString("ObjectShape", "", text, 0x80);
            SceneObjectShapeKeyword shapes[2] = {{"Box", 0}, {"Sphere", 1}};
            int shape = FindShape(text, shapes);
            if (minPoints >= points - 1)
                minPoints = points - 1;
            SoultreePhysicsObject* object = new(__FILE__, 1845) SoultreePhysicsObject(1);
            AppendChild(object->LoadPhysicsModel((int)renderContext, sltPath, lights,
                                                               modelTextures, *reinterpret_cast<Vec3*>(&position),
                                                               *reinterpret_cast<Vec3*>(&look), *reinterpret_cast<Vec3*>(&up),
                                                               0, (void*)particleManager, 5.0f, points, emitters, (SoultreeSlot1f0*)this,
                                                               0.033333f, 10, 0.025f, 0.05f, radius, shape,
                                                               minPoints, groundProbeMask, 1),
                                  -1);
            staticModels->casters[i].physicsObject = object;
            staticModels->casters[i].model = (D3DIMSoultreeObject*)object;
            staticModels->casters[i].physics = 1;
            if (points > 0) {
                for (int p = 0; p < points; p++) {
                    sprintf(key, "CollisionPoint%d", p + 1);
                    ReadVector(&position, model, key, "0.0,0.0,0.0", 0, 0);
                    sprintf(key, "FrictionCoefficient%d", p + 1);
                    parameters.GetFloat(key, 0.6f, &friction);
                    AddCollisionPoint(points, (void**)object->collisionPoints, &position,
                                      object->SoultreePhysicsBaseObject::sceneNode, 0,
                                      &object->collisionPointCount,
                                      friction, (int)this);
                }
            if (particleManager) {
                int dust = 0;
                int chunk = 0;
                int spray = 0;
                int nextEmitter;
                for (int e = 0; e < emitters; e = nextEmitter) {
                    nextEmitter = e + 1;
                    sprintf(key, "EmitterType%d", nextEmitter);
                    parameters.GetString(key, "", text, 0x17);
                    SceneEmitterTypeKeyword types[4] = {
                        {"DustEmitter", 1}, {"DirtChunkEmitter", 2}, {"DirtSprayEmitter", 3},
                        {"ExhaustEmitter", 4},
                    };
                    switch (FindEmitter(text, types)) {
                    case 1:
                        if (dust) {
                            sprintf(message, "\nOnly ONE %s for [%s] allowed.\n\n", text, model);
                            LogMessage(message);
                        } else {
                            DustParticleEmitter* emitter = new(__FILE__, 1917) DustParticleEmitter(1);
                            D3DIMSoultreeObject* node = object;
                            node->AppendChild((GameObject*)emitter->Create(renderContext, (ParticleManager*)particleManager), -1);
                            object->AddAttachment(1, emitter, (SoultreeAttachTarget*)*object->collisionPoints, 0);
                            dust = 1;
                        }
                        break;
                    case 2:
                        if (chunk) {
                            sprintf(message, "\nOnly ONE %s for [%s] allowed.\n\n", text, model);
                            LogMessage(message);
                        } else {
                            DirtChunkParticleEmitter* emitter = new(__FILE__, 1927) DirtChunkParticleEmitter(1);
                            D3DIMSoultreeObject* node = object;
                            node->AppendChild((GameObject*)emitter->Create(renderContext, (ParticleManager*)particleManager), -1);
                            object->AddAttachment(2, emitter, (SoultreeAttachTarget*)*object->collisionPoints, 0);
                            chunk = 1;
                        }
                        break;
                    case 3:
                        if (spray) {
                            sprintf(message, "\nOnly ONE %s for [%s] allowed.\n\n", text, model);
                            LogMessage(message);
                        } else {
                            DirtSprayParticleEmitter* emitter = new(__FILE__, 1937) DirtSprayParticleEmitter(1);
                            D3DIMSoultreeObject* node = object;
                            node->AppendChild((GameObject*)emitter->Create(renderContext, (ParticleManager*)particleManager), -1);
                            object->AddAttachment(3, emitter, (SoultreeAttachTarget*)*object->collisionPoints, 0);
                            spray = 1;
                        }
                        break;
                    case 4:
                        break;
                    default:
                        sprintf(message, "\nUnknown EmitterType (%s) for [%s].\n\n", text, model);
                        LogMessage(message);
                        break;
                    }
                }
            }
            }
        } else {
            staticModels->casters[i].physicsObject = 0;
            D3DIMSoultreeObject* object = new(__FILE__, 1956) D3DIMSoultreeObject(1);
            staticModels->casters[i].model = object;
            AppendChild(object->LoadModel(renderContext, sltPath, (int)lights, (int)modelTextures, 1), -1);
            int length = strcspn(sltPath, ".");
            count = length < 0x14 ? length : 0x14;
            strncpy(staticModels->casters[i].fileName, sltPath, count);
            staticModels->casters[i].fileName[count] = 0;
            ((D3DIMSoultreeObject*)staticModels->casters[i].model)->SetPositionVec3(position);
            ((D3DIMSoultreeObject*)staticModels->casters[i].model)->SetAxesPtr(reinterpret_cast<const Vec3*>(&look), reinterpret_cast<const Vec3*>(&up), 1, 0);
            parameters.GetInt("Collision", 1, &collision);
            staticModels->casters[i].collisionObject = 0;
            if (collision && !stadium
                && ((D3DIMSoultreeObject*)staticModels->casters[i].model)->CountNodes() == 1) {
                g_MemTagStack->Push("Collision");
                staticModels->casters[i].collisionObject = new(__FILE__, 1974) CollisionObject(1);
                staticModels->casters[i].collisionObject->Configure(renderContext, 1, 0, 1);
                staticModels->casters[i].collisionObject->ownerObject =
                    (D3DIMSoultreeObject*)staticModels->casters[i].model;
                if (!colPath[0])
                    staticModels->casters[i].collisionObject->SetHullShape(
                        (D3DIMSoultreeObject*)staticModels->casters[i].model, 0, 0, 0, 0);
                else
                    staticModels->casters[i].collisionObject->LoadShape(
                        (D3DIMSoultreeObject*)staticModels->casters[i].model, colPath);
                staticModels->casters[i].collisionObject->SnapPlacement();
                ((D3DIMSoultreeObject*)staticModels->casters[i].model)
                    ->AppendChild(staticModels->casters[i].collisionObject, -1);
                g_MemTagStack->Push("Scene");
            }
            staticModels->casters[i].physics = 0;
        }
        parameters.GetInt("UseLighting", 1, &useLighting);
        staticModels->casters[i].useLighting = useLighting;
        ((D3DIMSoultreeObject*)staticModels->casters[i].model)
            ->SetUseLighting(staticModels->casters[i].useLighting);
        if (auralScape) {
            parameters.GetString("SoundResourceFile", "", sound, 0x103);
            if (strcmp(sound, "") && _stricmp(sound, "NONE")) {
                ReadSoundSettings(model, sound, 1, &params, &flags, &oneShotDistance, &randomTriggerPercent,
                                      &oneShot, &force2D);
                SoultreeSoundEmitter* emitter =
                    (new(__FILE__, 2015)
                         SoultreeSoundEmitter((AuralScape*)auralScape, soundGroup, 1, isEnabled))
                        ->CreateFollowing(renderContext, sound, "",
                                                (SoultreeObject*)staticModels->casters[i].model,
                                                params, flags, oneShotDistance, randomTriggerPercent, 0,
                                                force2D);
                AppendChild(emitter, -1);
                if (!emitter) {
                    sprintf(text, "\nScene::SoundEmitter(%s) not created\n", sound);
                    LogMessage(text);
                }
            }
        } else {
            sprintf(text, "\nScene:  No AuralScape, so NO Static Model SOUNDS!\n");
            LogMessage(text);
        }
    }
    delete stream;
    LogMessage("");
    return 1;
        }
        sprintf(text, "\nNumber of models<1 in %s.\n\n", "StaticModels", scenePath);
        LogMessage(text);
        return 0;
    }
    sprintf(text, "\nCannot find data for [%s] in %s.\n\n", "StaticModels", scenePath);
    LogMessage(text);
    return 0;
}


// 0x004edf20: attaches a new shadow receiver for `shadow` to each caster and
// adds it as a child.
void Scene::AttachCasterShadows(ProjectedShadow* shadow, int flags)
{
    if (shadow && staticModels) {
        for (int i = 0; i < staticModels->casterCount; i++)
            AppendChild((new(__FILE__, 2136) D3DIMSoultreeShadow(flags))
                                      ->Attach((int)renderContext, staticModels->casters[i].model,
                                               shadow),
                                  -1);
    }
}

// 0x004edfe0: see the declaration. Reads "Animations": an entry per
// "Animation<n>", either a key-framed character (MCF file, motions and a
// motion sequence) or a procedural car (SLT and VUE files), each with an
// optional sound emitter; then the "RandomSet<n>" sections.
// Not exact (5289 of 5335 bytes: instructions, calls, EH states and the 0xcfc frame match;
// retail lets the random-set entries offset share fps's home and keeps i's to itself, and
// reloads motionCount as [ebx + ecx + 0x1c]; docs/SCENEMANAGER.md lists the forms tried).
int Scene::ReadAnimations(LightManager* lights, int textureManager, const TextureContext* modelTextures, void (*progress)(int), int interval)
{
    float fps;
    int animations;
    int i;
    int frontWheelsTurn;
    float lagDistance;
    int next;
    int tires;
    int useLighting;
    int exclude;
    char name[0x40];
    Vector3 up;
    Vector3 look;
    char motion[0x14];
    char number[0x18];
    char setName[0x44];
    char key[0x44];
    char message[0x204];
    char file[0x108];
    char vue[0x40];
    char text[0x204];
    DS3DBuffer params;
    char colPath[0x104];
    char sound[0x184];
    char sequence[0x80];
    char attach[0x84];
    char list[0x84];
    char sltPath[0x104];
    char vuePath[0x104];

    animations = 0;
    int found = parameters.SelectSection("Animations");
    if (sceneManager->sceneNamed && found) {
        if (!parameters.GetInt("NumberOfAnimations", -1, &animations)) {
            sprintf(text, "\nNumberOfAnimations cannot be found under [%s] in %s.\n\n", "Animations",
                    scenePath);
            LogMessage(text);
            return 0;
        }
        if (animations > 0) {
    animationTable = new(__FILE__, 2178) SceneAnimationTable;
    animationTable->entryCount = animations;
    LogParameter("Animations", "NumberOfAnimations", animations);
    animationTable->entries = new(__FILE__, 2181) SceneAnimation[animations];
    for (i = 0; i < animationTable->entryCount; i = next) {
        float heading;
        float pitch;
        if (i % interval == interval - 1 && progress)
            progress(0);
        animationTable->entries[i].inRandomSet = 0;
        next = i + 1;
        sprintf(name, "Animation%d", next);
        parameters.SelectSection(name);
        if (!parameters.GetString("AnimationType", "", text, 0x40)) {
            sprintf(message, "\nScene: AnimationType not specified for anim#%d.  Using KeyFramed.\n", i);
            LogMessage(message);
            animationTable->entries[i].keyFramed = 1;
        } else if (!_strnicmp(text, "Procedural", 3)) {
            animationTable->entries[i].keyFramed = 0;
            if (!parameters.GetString("VUE", "", vue, 0x40)) {
                sprintf(message, "\nCannot find %s for [%s] in %s.\n\n", "VUE", name, scenePath);
                LogMessage(message);
                return 0;
            }
            sprintf(text, "%s\\%s", "Res", vue);
            FileStream* stream =
                new(__FILE__, 2216) FileStream(g_ResourceManager);
            if (!g_TrackGame->sceneObject->OpenArchiveFile(stream, text, "rb", (int)vuePath)) {
                sprintf(sound, "No procedural vue file found in resources.  Aborting.\n");
                delete stream;
                return 0;
            }
            delete stream;
            LogParameter(name, "VUE", vuePath);
            if (!parameters.GetFloat("FPS", 30.0f, &fps))
                LogParameter(name, "FPS(Default)", fps);
            else
                LogParameter(name, "FPS", fps);
            if (!parameters.GetFloat("LagDistance", 0, &lagDistance))
                LogParameter(name, "LagDistance(Default)", lagDistance);
            else
                LogParameter(name, "LagDistance", lagDistance);
            if (!parameters.GetInt("FrontWheelsTurn", 1, &frontWheelsTurn)) {
                LogParameter(name, "FrontWheelsTurn(Default)", frontWheelsTurn);
                if (lagDistance == 0)
                    frontWheelsTurn = 1;
                else
                    frontWheelsTurn = 0;
            } else {
                LogParameter(name, "FrontWheelsTurn", frontWheelsTurn);
            }
            if (!parameters.GetInt("NumberOfTires", 4, &tires))
                LogParameter(name, "NumberOfTires(Default)", tires);
            else
                LogParameter(name, "NumberOfTires", tires);
        } else {
            animationTable->entries[i].keyFramed = 1;
        }
        animationTable->entries[i].animatesWhileHeld = 0;
        if (animationTable->entries[i].keyFramed && !parameters.GetString("MCF", "", file, 0x104)) {
            sprintf(message, "\nCannot find %s for [%s] in %s.\n\n", "MCF", name, scenePath);
            LogMessage(message);
            return 0;
        }
        if (!animationTable->entries[i].keyFramed && !parameters.GetString("SLT", "", file, 0x104)) {
            sprintf(message, "\nCannot find %s for [%s] in %s.\n\n", "SLT", name, scenePath);
            LogMessage(message);
            return 0;
        }
        sprintf(sltPath, "%s\\%s", "Res", file);
        if (animationTable->entries[i].keyFramed)
            ResolveSltPath(file);
        int length = strcspn(file, ".");
        int count = length < 0x14 ? length : 0x14;
        strncpy(animationTable->entries[i].fileName, file, count);
        animationTable->entries[i].fileName[count] = 0;
        length = strlen(file);
        int size = length > 0x103 ? 0x103 : length;
        strncpy(colPath, file, size);
        colPath[size] = 0;
        strcpy(strrchr(colPath, '.'), ".col");
        FileStream* stream =
            new(__FILE__, 2281) FileStream(g_ResourceManager);
        sprintf(text, "%s\\%s", "Res", colPath);
        if (!g_TrackGame->sceneObject->OpenArchiveFile(stream, text, "rb", (int)colPath))
            colPath[0] = 0;
        delete stream;
        if (!ReadVector(&animationTable->entries[i].position, name, "Position", "0.0,0.0,0.0", 0, 0))
            ReadVector(&animationTable->entries[i].position, name, "Offset", "0.0,0.0,0.0", 0, 0);
        if (animationTable->entries[i].keyFramed) {
            CollisionCharacter* character = new(__FILE__, 2300) CollisionCharacter(1);
            animationTable->entries[i].character = character;
            if (!AppendChild(character->Load((int)renderContext, sltPath, colPath, lights, modelTextures, 1, 1),
                                       -1)) {
                sprintf(message, "\nScene: Cannot create character from %s.\n\n", file);
                LogMessage(message);
                return 0;
            }
        }
        if (!animationTable->entries[i].keyFramed) {
            CarProcedural* car = new(__FILE__, 2305) CarProcedural(1);
            animationTable->entries[i].proceduralObject = car;
            if (!AppendChild(car->Create(renderContext, sltPath, colPath, (int)lights, (int)modelTextures, 0, vuePath,
                                                                 &animationTable->entries[i].position, fps,
                                                                 frontWheelsTurn, lagDistance, 1.0f, tires, 0, 0, 0),
                                       -1)) {
                sprintf(message, "\nScene: Cannot create procedural model from %s.\n\n", file);
                LogMessage(message);
                return 0;
            }
        }
        g_MemTagStack->Push("Scene");
        parameters.GetInt("UseLighting", 1, &useLighting);
        int axes = 0;
        if (parameters.GetFloat("Heading", -999.0f, &heading)) {
            parameters.GetFloat("Pitch", 0, &pitch);
            OrientationAxesFromAngles(&look, &up, heading * 0.01745329f, pitch * 0.01745329f, 0.0f);
            axes = 1;
        } else if (ReadVector(&look, name, "LookVector", "0.0,0.0,1.0", 0, 0)) {
            ReadVector(&up, name, "UpVector", "0.0,1.0,0.0", 0, 0);
            axes = 1;
        }
        parameters.GetInt("ExcludeFromCameraIteration", 0, &exclude);
        animationTable->entries[i].excludeFromCameraIteration = exclude;
        if (animationTable->entries[i].keyFramed) {
            int automatic;
            LogParameter(name, "MCF", file);
            animationTable->entries[i].character->modelNode->SetPosition(*reinterpret_cast<const Vec3*>(&animationTable->entries[i].position));
            if (axes)
                animationTable->entries[i].character->modelNode->SetAxesPtr(reinterpret_cast<const Vec3*>(&look), reinterpret_cast<const Vec3*>(&up), 1, 0);
            static_cast<D3DIMSoultreeObject*>(animationTable->entries[i].character->modelNode)->SetUseLighting(useLighting);
            parameters.GetInt("UseBlendedKeys", 1, &automatic);
            animationTable->entries[i].useBlendedKeys = automatic;
            parameters.GetInt("NumberOfMotions", 0, &animationTable->entries[i].motionCount);
            animationTable->entries[i].motions = 0;
            animationTable->entries[i].motionSequence = 0;
            if (animationTable->entries[i].motionCount > 0) {
                SceneAnimationTable* table = animationTable;
                animationTable->entries[i].motions = new(__FILE__, 2357) void*[table->entries[i].motionCount];
                for (int m = 0; m < animationTable->entries[i].motionCount; m++) {
                    sprintf(key, "MotionToPlay%d", m + 1);
                    if (!parameters.GetString(key, "", motion, 0x10)) {
                        sprintf(message, "\nCannot find %s for [%s] in %s.\n\n", key, name, scenePath);
                        LogMessage(message);
                        return 0;
                    }
                    LogParameter(name, key, motion);
                    animationTable->entries[i].motions[m] =
                        animationTable->entries[i].character->FindMotion(motion, 1);
                }
                if (parameters.GetString("MotionSequence", "", sequence, 0x40)) {
                    char* token = strtok(sequence, ",");
                    int j = 0;
                    parameters.GetString("NumberInSequence", "", number, 0x10);
                    animationTable->entries[i].sequenceLength = atoi(number);
                    if (animationTable->entries[i].sequenceLength) {
                        animationTable->entries[i].motionSequence =
                            new(__FILE__, 2379) char[animationTable->entries[i].sequenceLength];
                        for (; token && j < animationTable->entries[i].sequenceLength; j++) {
                            animationTable->entries[i].motionSequence[j] = atoi(token);
                            token = strtok(0, ",");
                        }
                        parameters.GetInt("AutomaticSequence", 1, &automatic);
                        animationTable->entries[i].automaticSequence = automatic;
                        animationTable->entries[i].sequenceIndex = animationTable->entries[i].motionSequence[0];
                        if (animationTable->entries[i].sequenceIndex >= animationTable->entries[i].sequenceLength)
                            animationTable->entries[i].sequenceIndex = 0;
                        animationTable->entries[i].currentMotion = animationTable->entries[i].sequenceIndex;
                        RestartEntry(i, animationTable->entries[i].sequenceIndex, 0, 0);
                    } else {
                        animationTable->entries[i].sequenceIndex = 0;
                        sprintf(message, "\n%s for [%s] in %s is ZERO!\n\n", "NumberInSequence", name, scenePath);
                        LogMessage(message);
                        return 0;
                    }
                } else {
                    animationTable->entries[i].currentMotion = 0;
                    animationTable->entries[i].sequenceLength = 0;
                    sprintf(message, "\n%s for [%s] in %s is ZERO!\n\n", "MotionSequence", name, scenePath);
                    LogMessage(message);
                    return 0;
                }
            }
        } else {
            LogParameter(name, "SLT", sltPath);
            animationTable->entries[i].motionCount = 1;
            animationTable->entries[i].proceduralObject->carModel->SetUseLighting(useLighting);
            if (axes)
                animationTable->entries[i].proceduralObject->carModel->SetAxesPtr(reinterpret_cast<const Vec3*>(&look), reinterpret_cast<const Vec3*>(&up), 1, 0);
        }
        if (animationTable->entries[i].motionCount > 0) {
            if (auralScape) {
                parameters.GetString("SoundResourceFile", "", sound, 0x103);
                if (strcmp(sound, "") && _stricmp(sound, "NONE")) {
                    unsigned long flags;
                    float oneShotDistance;
                    float randomTriggerPercent;
                    int oneShot;
                    int force2D;
                    ReadSoundSettings(name, sound, 0, &params, &flags, &oneShotDistance, &randomTriggerPercent,
                                          &oneShot, &force2D);
                    parameters.GetString("SoundAttachToObject", "", attach, 0x40);
                    D3DIMSoultreeObject* node;
                    if (animationTable->entries[i].keyFramed)
                        node = static_cast<D3DIMSoultreeObject*>(animationTable->entries[i].character->modelNode);
                    else
                        node = animationTable->entries[i].proceduralObject->carModel;
                    SoultreeSoundEmitter* emitter =
                        (new(__FILE__, 2433)
                             SoultreeSoundEmitter((AuralScape*)auralScape, soundGroup, 1, isEnabled))
                            ->CreateFollowing(renderContext, sound, attach, node, params,
                                                    flags, oneShotDistance, randomTriggerPercent, 0, force2D);
                    if (!emitter) {
                        sprintf(text, "\nScene::SoundEmitter(%s) not created\n", sound);
                        LogMessage(text);
                    } else {
                        AppendChild(emitter, -1);
                    }
                }
            } else {
                sprintf(text, "\nScene:  No AuralScape, so NO Animation SOUNDS!\n");
                LogMessage(text);
            }
        }
    }
    parameters.SelectSection("Animations");
    parameters.GetInt("NumberOfRandomSets", 0, &animationTable->randomSetCount);
    if (animationTable->randomSetCount > 0) {
        animationTable->randomSets = new(__FILE__, 2466) SceneRandomSet[animationTable->randomSetCount];
        for (i = 0; i < animationTable->randomSetCount; i++) {
            animationTable->randomSets[i].animationIndices = 0;
            animationTable->randomSets[i].motionIndices = 0;
            sprintf(setName, "RandomSet%d", i + 1);
            if (!parameters.SelectSection(setName)) {
                sprintf(message, "\nCannot find [%s] in %s.\n\n", setName, scenePath);
                LogMessage(message);
                return 0;
            }
            if (!parameters.GetString("RandomSet", "", list, 0x80)) {
                sprintf(message, "\nCannot find %s for [%s] in %s.\n\n", "RandomSet", setName, scenePath);
                LogMessage(message);
                return 0;
            }
            char* token = strtok(list, ",");
            int j = 0;
            animationTable->randomSets[i].entry = 0;
            parameters.GetString("NumberInSequence", 0, (char*)&animationTable->randomSets[i].count, -1);
            if (animationTable->randomSets[i].count) {
                animationTable->randomSets[i].animationIndices = new(__FILE__, 2491) signed char[animationTable->randomSets[i].count];
                animationTable->randomSets[i].motionIndices = new(__FILE__, 2492) signed char[animationTable->randomSets[i].count];
                for (; token && j < animationTable->randomSets[i].count; j++) {
                    int dot = strcspn(token, ".");
                    if (!dot || dot == (int)strlen(token)) {
                        sprintf(message, "\nInvalid Animation or Motion #%d in [%s] in %s.\n\n", j, setName,
                                scenePath);
                        LogMessage(message);
                        return 0;
                    }
                    token[dot] = 0;
                    animationTable->randomSets[i].animationIndices[j] = atoi(token) - 1;
                    animationTable->entries[animationTable->randomSets[i].animationIndices[j]].inRandomSet = 1;
                    if (animationTable->entries[i].keyFramed)
                        animationTable->entries[animationTable->randomSets[i].animationIndices[j]].character->Disable();
                    else
                        animationTable->entries[animationTable->randomSets[i].animationIndices[j]].proceduralObject->Disable();
                    animationTable->randomSets[i].motionIndices[j] = atoi(token + dot + 1) - 1;
                    token = strtok(0, ",");
                }
                animationTable->randomSets[i].entry = animationTable->entries;
            }
        }
    } else {
        animationTable->randomSets = 0;
    }
    LogMessage("");
    return 1;
        }
        sprintf(text, "\nNumber of animations<1 in %s.\n\n", "Animations", scenePath);
        LogMessage(text);
        return 0;
    }
    sprintf(text, "\nCannot find data for [%s] in %s.\n\n", "Animations", scenePath);
    LogMessage(text);
    return 0;
}

// 0x004ef4c0: reads the "Sounds" section: whether a crowd is present and
// the "StaticSound<n>" sections, each an emitter placed in the scene or a
// plain sound added to sounds. Needs the AuralScape (auralScape).
int Scene::ReadSounds()
{
    int count;
    int oneShot;
    int force2D;
    unsigned long flags;
    float oneShotDistance;
    float randomTriggerPercent;
    DS3DBuffer params;
    char file[0x104];
    char text[0x204];
    char position[0x80];
    char type[0x104];
    char section[0x204];

    if (!sceneManager->sceneNamed)
        return 0;
    parameters.SelectSection("Sounds");
    parameters.GetInt("CrowdPresent", 0, &crowdPresent);
    LogParameter("Sounds", "CrowdPresent", crowdPresent);
    LogMessage("");
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
                            LogMessage(text);
                        }
                        SoundEmitter* emitter =
                            (new(__FILE__, 2588)
                                 SoundEmitter((AuralScape*)auralScape, soundGroup, 1, isEnabled))
                                ->Create(renderContext, file, params, flags, oneShotDistance,
                                                        randomTriggerPercent, 0, force2D);
                        AppendChild(emitter, -1);
                        if (!emitter) {
                            sprintf(text, "\nScene::SoundEmitter(%s) not created\n", file);
                            LogMessage(text);
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
                LogMessage(text);
            }
        }
        LogMessage("");
        return 1;
    }
    sprintf(text, "\nNumber of StaticSounds<1 in %s.\n\n", scenePath);
    LogMessage(text);
    return 0;
}

// 0x004ef9c0: reads a sound's settings from `section` and appends ".wav" to
// `file` when it has no extension yet.
void Scene::ReadSoundSettings(const char* section, char* file, int is3D,
                                  DS3DBuffer* params, unsigned long* flags,
                                  float* oneShotDistance, float* randomTriggerPercent,
                                  int* oneShot, int* force2D)
{
    memset(params, 0, sizeof(DS3DBuffer));
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
int Scene::Render3D()
{
    if (visible)
        return GameObject::Render3D();
    return 1;
}

// 0x004f0060: control 5 toggles visible; other events go to GameObject.
int Scene::OnControlRelease(ControlEvent* event, InputButtonState* entry)
{
    if (TestInputEvent(5, 0, event, 0x80)) {
        visible = 1 - visible;
        return 1;
    }
    return GameObject::OnControlRelease(event, entry) != 0;
}

// 0x004f00c0
int Scene::OnControlPress(ControlEvent* event, InputButtonState* entry)
{
    return GameObject::OnControlPress(event, entry) != 0;
}

// 0x004efb20: loads the scene file into parameters and reads the
// "SceneInfo" positions, the environment and the "Lights" section, creating
// an emitter in `lights` for each light that emits; then the fog, resource
// files, stadium and the 0x004ecd60/0x004edfe0/0x004ef4c0 parts. Returns 0
// when a part fails.
int Scene::Load(void* owner, LightManager* lights, int particleManager, int textureManager, const TextureContext* modelTextures,
                                 const char* cubeDirectory, void (*progress)(int), int interval)
{
    int logProgress;
    char path[0x104];

    FileStream* archive = sceneManager->OpenSceneArchive(0);
    if (!archive)
        return 0;
    sprintf(scenePath, "%s\\%s", sceneManager->sceneDirectory, sceneManager->sceneFile);
    parameters.OpenFromArchives((FileStream*)archive, sceneManager->sceneFile,
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
    LogParameter("SceneInfo", "SceneName", sceneName);
    ReadVector(&defaultStartPosition, "SceneInfo", "DefaultStartPosition", "1152.0,100.0,1152.0", 1, 0);
    ReadVector(&defaultStartDirection, "SceneInfo", "DefaultStartDirection", "0.0,0.0,1.0", 1, 0);
    LogMessage("");
    renderContext = owner;
    ReadVector(&podiumPosition, "SceneInfo", "PodiumPosition", "0.0,0.0,0.0", 1, 0);
    ReadVector(&podiumDirection, "SceneInfo", "PodiumDirection", "0.0,0.0,0.0", 1, 0);
    if (!ReadEnvironment(sceneManager->sceneDirectory, cubeDirectory))
        return 0;
    parameters.SelectSection("Lights");
    parameters.GetInt("NumberOfLights", -1, &lightCount);
    LogParameter("Lights", "NumberOfLights", lightCount);
    LogMessage("");
    if (!sceneManager->sceneNamed || lightCount == -1)
        lightCount = 2;
    sceneLights = new(__FILE__, 2784) SceneLight[lightCount];
    for (int i = 0; i < lightCount; i++) {
        sceneLights[i].emitter = 0;
        if (!ReadLight(i))
            return 0;
        if (sceneLights[i].emitsLight) {
            const char* flare = sceneLights[i].hasLensFlare ? sceneLights[i].lensFlareTexture : 0;
            int flareTextureManager = sceneLights[i].hasLensFlare ? (int)g_TrackGame->textureManager : 0;
            sceneLights[i].emitter =
                (new(__FILE__, 2798) LightEmitter(1))
                    ->Create(renderContext, sceneLights[i].type, sceneLights[i].color,
                                            &sceneLights[i].position, sceneLights[i].emitterDirection,
                                            sceneLights[i].range, sceneLights[i].showDebugSphere, 0, flareTextureManager,
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
        int objects = ReadStaticModels(lights, particleManager, textureManager, modelTextures, progress, interval);
        if (!ReadAnimations(lights, textureManager, modelTextures, progress, interval) && !objects)
            Disable();
    }
    ReadSounds();
    return 1;
}

// 0x004eff30: selects the detail table and applies `level` to every entry's
// object and to the shadow casters, re-enabling those without flag 2.
void Scene::SetDetailLevel(int level)
{
    g_ActiveLodSettings = g_TrackGame->bladeRenderer ? g_BladeLodSettings
                                                     : g_NonBladeLodSettings;
    if (animationTable) {
        int i;
        for (i = 0; i < animationTable->entryCount; i++) {
            if (animationTable->entries[i].keyFramed)
                static_cast<D3DIMSoultreeObject*>(animationTable->entries[i].character->modelNode)->ApplyLodSettings(level);
            else
                animationTable->entries[i].proceduralObject->carModel->ApplyLodSettings(level);
        }
        for (i = 0; i < animationTable->randomSetCount; i++) {
            SceneAnimation* entry = animationTable->randomSets[i].entry;
            if (entry->keyFramed)
                static_cast<D3DIMSoultreeObject*>(entry->character->modelNode)->ApplyLodSettings(level);
            else
                entry->proceduralObject->carModel->ApplyLodSettings(level);
        }
    }
    if (staticModels) {
        for (int i = 0; i < staticModels->casterCount; i++) {
            if (staticModels->casters[i].model) {
                reinterpret_cast<D3DIMSoultreeObject*>(staticModels->casters[i].model)->ApplyLodSettings(level);
                if (!staticModels->casters[i].useLighting)
                    reinterpret_cast<D3DIMSoultreeObject*>(staticModels->casters[i].model)->SetUseLighting(0);
            }
        }
    }
}

// 0x004f00e0
int CountTexture(SceneTextureRecord* texture, int* counts)
{
    if (!_stricmp(texture->textureName, "PROCEDURAL")) {
        counts[2]++;
        return 1;
    }
    int header0;
    int header1;
    int width = 0;
    UnknownResourceEntry* entry =
        g_ResourceManager->FindItem(texture->textureName, 0);
    if (entry) {
        entry->archive->Seek(entry->offset, 0, 0);
        if (entry->archive->Read(&header0, 4, 1) != 1
            || entry->archive->Read(&header1, 4, 1) != 1
            || entry->archive->Read(&width, 4, 1) != 1)
            return 0;
    } else {
        TgaFile* file = OpenAndReadHeader(texture->textureName, 0,
                                                     (int)g_ResourceManager);
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
int AddResource(ResourceManager* manager, const char* name, const char* path)
{
    UnknownResourceEntry* global = g_ResourceManager->FindItem(name, 0);
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
    if (!AddResource(resources, name, file))
        return 1;
    UnknownResourceEntry* entry = g_ResourceManager->FindItem(name, 0);
    if (!entry) {
        g_ResourceManager->AddLooseFile(name, file);
        entry = g_ResourceManager->FindItem(name, 0);
    }
    if (entry) {
        entry->archive->Seek(entry->offset, 0, 0);
        entry->archive->Read(&count, 4, 1);
        SoultreeObject* node = new(__FILE__, 3447) SoultreeObject(0);
        for (int i = 0; i < count; i++) {
            entry->archive->Read(node->name, 0x80, 1);
            entry->archive->Read(&node->localMatrix, 0x40, 1);
            entry->archive->Read(&node->hidden, 4, 1);
            entry->archive->Read(&node->localBoundsValid, 4, 1);
            entry->archive->Read(&node->localBoundsCenter, 0xc, 1);
            entry->archive->Read(&node->localBoundsHalfExtent, 0xc, 1);
            entry->archive->Read(&node->subtreeBoundsValid, 4, 1);
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
                entry->archive->Read(&material->texelRatio, 4, 1);
                position = entry->archive->Tell();
                mode = entry->archive->GetTextMode();
                if (material->hasTextureName) {
                    if (!AddResource(resources, material->textureName, 0))
                        return 1;
                    if (!CountTexture((SceneTextureRecord*)material, (int*)counts))
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
    if (!AddResource(resources, name, file))
        return 1;
    if (!g_ResourceManager->FindItem(name, 0)) {
        g_ResourceManager->AddLooseFile(name, file);
        if (!g_ResourceManager->FindItem(name, 0))
            return 1;
    }
    FileStream* stream = new(__FILE__, 3622) FileStream(g_ResourceManager);
    stream->Open(name, "rb", 0);
    ParameterBlock block;
    block.Attach((FileStream*)stream, 0, 1);
    block.SelectSection("Materials");
    block.GetInt("NumberOfMaterials", 0, &materials);
    SoultreeMaterial material(0);
    for (int k = 0; k < materials; k++) {
        sprintf(section, "Material - %d", k);
        block.SelectSection(section);
        if (block.ReadString("TextureMap", material.textureName, -1)) {
            if (!AddResource(resources, material.textureName, 0)) {
                delete stream;
                return 1;
            }
            if (!CountTexture((SceneTextureRecord*)&material, (int*)counts)) {
                delete stream;
                return 0;
            }
        }
    }
    delete stream;
    return 1;
}

// 0x004f0d20: loads the scene file into parameters and sets *count to its
// static model plus animation count (at least 1).
int Scene::CountObjects(int* count)
{
    FileStream* archive = sceneManager->OpenSceneArchive(0);
    if (!archive)
        return 0;
    int animations;
    UnknownSceneResourceManager resources;
    char message[0x204];
    sprintf(scenePath, "%s\\%s", sceneManager->sceneDirectory, sceneManager->sceneFile);
    parameters.OpenFromArchives((FileStream*)archive, sceneManager->sceneFile,
                                     scenePath, 1);
    *count = 1;
    int found = parameters.SelectSection("StaticModels");
    if (!sceneManager->sceneNamed || !found) {
        sprintf(message, "\nNo scene file OR cannot find data for [%s] in %s.\n\n",
                "StaticModels", scenePath);
        LogMessage(message);
    }
    if (!parameters.GetInt("NumberOfStaticModels", 1, count)) {
        sprintf(message, "\nNumberOfStaticModels cannot be found under [%s] in %s.\n\n",
                "StaticModels", scenePath);
        LogMessage(message);
    }
    animations = 0;
    if (!parameters.GetInt("NumberOfAnimations", 0, &animations)) {
        sprintf(message, "\nNumberOfAnimations cannot be found under [%s] in %s.\n\n",
                "Animations", scenePath);
        LogMessage(message);
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

    FileStream* stream =
        new(__FILE__, 3727) FileStream(g_ResourceManager);
    char dir[_MAX_DIR];
    _splitpath(path, drive, dir, name, ext);
    strcat(name, ext);
    if (!g_ResourceManager->FindItem(name, 0)) {
        g_ResourceManager->AddLooseFile(name, path);
        if (!g_ResourceManager->FindItem(name, 0)) {
            delete stream;
            return 0;
        }
    }
    stream->Open(name, "rb", 0);
    ParameterBlock block;
    block.Attach((FileStream*)stream, 0, 1);
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

    FileStream* archive = sceneManager->OpenSceneArchive(0);
    if (!archive)
        return 0;
    UnknownSceneResourceManager resources;
    sprintf(scenePath, "%s\\%s", sceneManager->sceneDirectory, sceneManager->sceneFile);
    parameters.OpenFromArchives((FileStream*)archive, sceneManager->sceneFile,
                                     scenePath, 1);
    OpenResourceFiles();
    int i = 0;
    models = 0;
    int found = parameters.SelectSection("StaticModels");
    if (sceneManager->sceneNamed && found) {
        if (!parameters.GetInt("NumberOfStaticModels", -1, &models)) {
            sprintf(text, "\nNumberOfStaticModels cannot be found under [%s] in %s.\n\n", "StaticModels",
                    scenePath);
            LogMessage(text);
        } else if (models > i) {
            for (i = 0; i < models;) {
                sprintf(model, "Model%d", ++i);
                parameters.SelectSection(model);
                if (!parameters.GetString("SLT", "", file, 0x104)) {
                    sprintf(modelMessage, "\nCannot find %s for [%s] in %s.\n\n", file, model, scenePath);
                    LogMessage(modelMessage);
                    return 0;
                }
                sprintf(modelPath, "%s\\%s", "Res", file);
                if (!CountModelTextures(modelPath, &resources, counts))
                    return 0;
            }
        }
    } else {
        sprintf(text, "\nCannot find data for [%s] in %s.\n\n", "StaticModels", scenePath);
        LogMessage(text);
    }
    i = 0;
    animations = 0;
    found = parameters.SelectSection("Animations");
    if (sceneManager->sceneNamed && found) {
        if (!parameters.GetInt("NumberOfAnimations", -1, &animations)) {
            sprintf(text, "\nNumberOfAnimations cannot be found under [%s] in %s.\n\n", "Animations",
                    scenePath);
            LogMessage(text);
        } else if (animations > i) {
            for (i = 0; i < animations; i = next) {
                int keyFramed;
                next = i + 1;
                sprintf(animation, "Animation%d", next);
                parameters.SelectSection(animation);
                if (!parameters.GetString("AnimationType", "", text, 0x40)) {
                    sprintf(message, "\nScene: AnimationType not specified for anim#%d.  Using KeyFramed.\n", i);
                    LogMessage(message);
                    keyFramed = 1;
                } else if (!_strnicmp(text, "Procedural", 3)) {
                    keyFramed = 0;
                } else {
                    keyFramed = 1;
                }
                if (keyFramed) {
                    if (!parameters.GetString("MCF", "", file, 0x104)) {
                        sprintf(message, "\nCannot find %s for [%s] in %s.\n\n", "MCF", animation, scenePath);
                        LogMessage(message);
                        return 0;
                    }
                } else if (!parameters.GetString("SLT", "", file, 0x104)) {
                    sprintf(message, "\nCannot find %s for [%s] in %s.\n\n", "SLT", animation, scenePath);
                    LogMessage(message);
                    return 0;
                }
                sprintf(animationPath, "%s\\%s", "Res", file);
                if (keyFramed) {
                    LogParameter(animation, "MCF", animationPath);
                    ResolveSltPath(animationPath);
                }
                LogParameter(animation, "SLT", animationPath);
                if (!CountModelTextures(animationPath, &resources, counts))
                    return 0;
            }
        }
    } else {
        sprintf(text, "\nCannot find data for [%s] in %s.\n\n", "Animations", scenePath);
        LogMessage(text);
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
