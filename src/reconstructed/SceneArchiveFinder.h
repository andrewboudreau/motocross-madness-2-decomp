#pragma once

class UnknownTextureStream;

// Object at TrackGame+0x574, deleted by slot 15. Its destructor is
// out-of-line and empty (the shared body 0x00464e90).
class UnknownTrackGameObject574 {
public:
    UnknownTrackGameObject574();              // 0x004e99d0 (defined in SceneManager.cpp)
    ~UnknownTrackGameObject574();             // 0x00464e90
    // 0x004e9a10 / 0x004e9ac0 (SceneManager.cpp): run a temporary Scene's
    // CountObjects / CountTextures (the quarry loader's estimates).
    int CountSceneObjects(int* count);
    int CountSceneTextures(unsigned long* counts, char* width, char* hasCube, int ecosystem);
    // 0x004e9ba0: opens the scene archive `name` (default sceneFile).
    UnknownTextureStream* OpenSceneArchive(const char* name);
    // 0x004e9cd0: opens `name` from the archive in `stream` (racesnd.cpp
    // reads Audio.res samples through it); 0 when not found.
    int OpenArchiveFile(UnknownTextureStream* stream, const char* name, const char* mode, int a);
    void SetSceneFileNames(const char* name); // 0x004e9f70: derived file names
    void SetSceneDirectory(char* path);   // 0x004e9b80 (EventManager 0x0045cb70)
    void SelectScene(char* name, const char* kind, int value); // 0x004e9e30
    // 0x004ea390: writes the scene name for `sceneFile` into `name`.
    void ReadCurrentSceneName(char* name, char* scene, int value);
    // 0x004ea010 (TrackRecord.cpp 0x0051ffe0): a track's display name.
    // Clears *a and *b when given; the archive's 0x00461cb0 fills them.
    void ReadSceneName(char* out, char* name, int index, const char* kind, int* a, int* b);

    // Layout from the constructor's stores; array lengths are inferred
    // from the gaps between them (and the 0x103-byte copy into +0x44).
    UnknownTextureStream* sceneArchive;       // +0x00 open scene archive
    char sceneArchiveName[0x40];              // +0x04 its name
    char sceneDirectory[0x104];               // +0x44 scene directory (SetSceneDirectory)
    char waypointPath[0x104];                 // +0x148 directory\waypoint file
    char sceneFile[0x40];                     // +0x24c scene file name
    char estFile[0x40];                       // +0x28c "%s.est"
    char trnFile[0x40];                       // +0x2cc "%s.trn"
    char cubeFile[0x40];  // +0x30C  the scene's "CubeFile" (SceneManager 0x004ebfc0); emptied by SetSceneFileNames
    char waypointFile[0x40];                  // +0x34c "%s01.wpt"
    int sceneNamed;                           // +0x38c 0 when SelectScene read "NO NAME"
    int waypointRaceCount;                    // +0x390 SceneInfo "NumberOfWaypointRaces"
};
