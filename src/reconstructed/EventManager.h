#pragma once

#include "GameObject.h"
#include "MatrixUtil.h"
#include "RaceView.h"

class ArcadeObject;
class TextQueueOverlay;
struct TrackGameViewOwner;
struct UnknownKrustyBikeView;

// 0x50-byte entry; EventManager keeps 11 at +0x50.
class Camera;

// 8-byte record sorted with 0x0045e930.
struct UnknownEventRanking {
    UnknownEventRacer* racer;
    float value;
};

// 8-byte record sorted with TrackOverlay's 0x005199f0 (higher value first).
struct UnknownEventScore {
    float value;
    UnknownEventRacer* racer;
};
int CompareScoresDescending(const void* a, const void* b); // cdecl 0x005199f0

// 16-byte record sorted with 0x0045d3d0.
struct UnknownEventStanding {
    int points;                                    // the result's points (EventManager.cpp)
    float field_0x04;
    float field_0x08;
    UnknownEventRacer* racer;                      // its +0x5e0 name breaks ties
};

// cdecl 0x0045d3d0: qsort order for standings (more points first, then
// lower +0x04 and +0x08, then the racer's name).
int CompareStandings(const void* a, const void* b);  // 0x0045d3d0

// cdecl 0x0045e930: qsort order for rankings (higher value first, then
// higher +0x7a0, +0x790 and +0x744->+0x0c).
int CompareRankings(const void* a, const void* b);  // 0x0045e930

// cdecl 0x0045cb20: progress callback that 0x0045cb70 and 0x0045cdc0 pass
// by address.
void LoadProgressCallback(int* step);  // 0x0045cb20

// cdecl 0x0045fbb0: qsort order for unsigned values.
int CompareUnsigned(const void* a, const void* b);  // 0x0045fbb0

// Minimal view of RTTI classes Character : virtual GameObject and
// D3DIMSoultreeCharacter : Character, declared in full in
// src/krusty2/motion/D3DIMSoultreeCharacter.h, whose header tree cannot be
// mixed with this one. 0x0045d480 constructs up to three (0x240 bytes,
// constructor 0x004455b0 writes D3DIMSoultreeCharacter's vtables) into
// EventManager+0x424; slot 10 calls their primary slot 7.
class Character : public virtual GameObject {
public:
    virtual void CharacterVirtualSlot0();
    virtual void CharacterVirtualSlot1();
    virtual void CharacterVirtualSlot2();
    virtual void CharacterVirtualSlot3(int a);
    virtual void CharacterVirtualSlot4(int a, int b);
    virtual void CharacterVirtualSlot5(int a, int b);
    virtual void CharacterVirtualSlot6(int a, int b);
    // 0x004a70c0 (ret 0xc): adds `time` (fld of the first argument) to +0x10.
    virtual int CharacterVirtualSlot7(float time, int a, int b);
};
// The Soultree object at a podium character's +0x1a0 (D3DIMSoulTree.h's
// D3DIMSoultreeObject methods, seen from the podium scene).
struct UnknownPodiumTextureEntry {
    unsigned char field_0x00[0x2c];
    char field_0x2c[4];                            // texture name
};
struct UnknownPodiumSoultree {
    void SetUseLighting(int value);                      // 0x004444c0
    void SetPosition(const Vector3* position);        // 0x004fc660
    void SetAxesPtr(const Vector3* look, const Vector3* up, int a, int b); // 0x004fbd70
    void ReplaceMaterialTexture(int index, const char* name, void* context); // 0x00444c70

    unsigned char field_0x000[0x290];
    UnknownPodiumTextureEntry** field_0x290;       // the racer model's texture
};

struct Motion;

class D3DIMSoultreeCharacter : public Character {
public:
    explicit D3DIMSoultreeCharacter(int flags);    // 0x004455b0 (0x240 bytes)
    virtual void CharacterVirtualSlot8();
    virtual void CharacterVirtualSlot9();
    virtual void CharacterVirtualSlot10();
    // Loads the model file `name`; returns the GameObject to add.
    virtual GameObject* CharacterVirtualSlot11(void* owner, const char* name, GameObject* lights,
                                               void* context, int a, int b);
    Motion* FindMotion(const char* name, int a); // 0x004a6b30: finds a motion
    void SetMotion(Motion* motion);    // 0x004a8b40: plays it

    unsigned char field_0x008[0x10 - 0x8];
    int motionTime;                           // +0x10  Character +0x10 motion time (slot 7 adds time; SceneManager.h's view names it the same)
    unsigned char field_0x014[0x1a0 - 0x14];
    UnknownPodiumSoultree* modelNode;         // +0x1a0
    unsigned char field_0x1a4[0x214 - 0x1a4];
};

// Motion record (src/krusty2/motion): 0x004a6b30's result, 0x004a8b40's argument.

// cdecl 0x004aef40: deletes TrackGame's NetworkInterface (+0x08), sets +0x18
// to 1 and clears +0x3424, +0x2da5 and +0x2d98. EventManager slot 10 calls
// it once when +0x3c is set.
void EndNetworkGame();  // 0x004aef40

struct UnknownEventEntry {
    UnknownEventEntry();                           // 0x0045c830 (resets through 0x0045c840)
    void Reset();                  // 0x0045c840: reset

    // 0x0045c8b0: copies the racer's id, name and the fields noted below
    // (dword moves; int/float types are from other readers). With racer
    // +0x4a0 set it clears them instead and sets position 99.
    void CopyFromRacer(UnknownEventRacer* racer);  // 0x0045c8b0

    int playerId;                                  // +0x00: network player id (racer +0x11bc)
    int position;                                  // +0x04: finishing position (99 when racer +0x4a0 is set)
    float fastestLap;                              // +0x08: racer +0x750 (ZoneReport "FastestLap"); TrackRecord.cpp 0x0051f3c0 loads it as a float
    int largestStuntPoints;                        // +0x0c: racer +0x788 (ZoneReport "LargestSingleStuntPoints")
    int field_0x10;                                // racer +0x758
    float totalRunningTime;                        // +0x14: racer +0x754 (ZoneReport "TotalrunningTime")
    float bestTagRun;                              // +0x18: racer +0x760 bestTagRun
    float totalTagRun;                             // +0x1c: racer +0x764 totalTagRun
    int finished;                                  // +0x20: racer +0x7a4 (1 when TrackGame+0x2d74 is 0 or 4)
    int outOfRace;                            // +0x24 racer +0x4a0
    int points;                                    // +0x28: championship points (AddChampionshipPoints)
    float score;                                   // +0x2c: racer +0x768 (score)
    unsigned char aiIndex;                    // +0x30 racer +0x11c0; counted in TrackGame+0x3424 (else Game+0x18)
    unsigned char field_0x31[0x34 - 0x31];
    int field_0x34[3];                             // racer +0x7ac..+0x7b4
    char name[16];                                 // +0x40: racer name (racer +0x5e0)
};

// RTTI: EventManager : GameObject (vtable 0x0055259c; 0xd08 bytes, the size
// TrackGame slot 4 allocates). Its constructor 0x0045c9e0 writes the vtable.
// Only the members TrackGame and KrustyBikeCamera call are declared.
class EventManager : public GameObject {
public:
    explicit EventManager(int flags);              // 0x0045c9e0
    virtual ~EventManager();                       // 0x0045cae0 (deleting wrapper 0x0045cac0)
    virtual GameObject* SetRenderContext(void* value);    // 0x0045caf0
    virtual int Tick(float frameTime);                     // 0x0045f200: per-frame update
    // 0x0045f3a0: while UI interaction is blocked (TrackGame+0x3430), control 1,
    // 0x1c or 0x39 (keyboard) or any joystick press resumes it.
    virtual int OnControlRelease(UnknownControlEvent* event, UnknownInputEntry* entry);
    virtual int OnControlPress(UnknownControlEvent* event, UnknownInputEntry* entry);       // 0x0045f440
    // 0x0045f490: network messages. NetworkInterface 0x004aced0 (through Game
    // slot 17) passes a NetMessage's +0x00, data (+0x14), +0x04, +0x08 and
    // +0x0c, which 0x004aacc0 stores from its `from`, `to` and `flags`.
    virtual int OnNetworkMessage(int type, void* data, int from, int to, int flags);

    // The first race-mode object of TrackGame+0x558..+0x568 present, its
    // +0x34 view and its +0x6c target.
    TrackGameViewOwner* FindRaceMode();            // 0x0045d2b0
    UnknownKrustyBikeView* FindRaceView();         // 0x0045d2f0
    TextQueueOverlay* FindTextQueue();             // 0x0045d340
    // 0x0045d270: Enable() on all three; LoadingDlg's timer (0x00451020)
    // calls it as it ends the loading dialog.
    void EnableRaceObjects();
    int HasRaceMode();                   // 0x0045d390: whether any mode is present
    void ResetEntries();                  // 0x0045e520: resets the entries
    // 0x0045e550: once every remote racer is ready, ends the network wait.
    void WaitForRemoteRacers(float frameTime);  // 0x0045e550
    // 0x0045f180: adds the championship points for `racer`'s position.
    void AddChampionshipPoints(UnknownEventRacer* racer, int* points);  // 0x0045f180
    // 0x0045e600: starts the end-of-race block (or finishes at once), and
    // once it has run its course clears it and moves to the results.
    void EndRace();
    void CloseRace(int mode);          // 0x0045cdc0
    // 0x0045d480: creates the camera (+0x3d4) and up to three characters
    // (+0x424) from "%s\\Winner.mcf" with "Podium3/4/5_%02d" motions (also
    // "CrowdLoop.wav"); nonzero when slot 10 should run that scene.
    int CreatePodiumScene();  // 0x0045d480
    void LeaveRaceForMenu(int menu);          // 0x0045e710
    void RankRacers();                             // 0x0045e9d0
    void CheckRaceOver(float frameTime);           // 0x0045eef0
    void SendRacerResults();                       // 0x0045f9a0
    int LoadEvent();                   // 0x0045cb70
    void RemovePlayer(int player);        // 0x0045fbd0
    // 0x0045fce0: removes every Vegetation in the rectangle from the
    // collision quadtree (up to 1000).
    void RemoveVegetationInRect(float x0, float z0, float x1, float z1);  // 0x0045fce0
    // 0x0045fdc0: Disable() (GameObject slot 4) on every CollisionObject in
    // the rectangle of the collision quadtree.
    void DisableCollisionObjectsInRect(float x0, float z0, float x1, float z1);

    // Passed to NetworkInterface 0x004ac8d0 (+0x7c, +0x80). Its keep-alive
    // thread 0x004af6a0 drops a player not heard from for +0x7c seconds and
    // Sleeps +0x80 * 1000 ms between keep-alive (0x4b) sends.
    float keepAliveTimeout;                        // "KeepAliveTimeout" (slot 8), default 20
    float keepAliveInterval;                       // 1.0
    int waitingForRacers;                          // +0x34: 0x0045e550 runs each frame (set once the keep-alive starts)
    int keepAliveStarted;                          // +0x38: NetworkInterface StartKeepAlive was called
    int endNetworkGamePending;                     // +0x3c: slot 10 calls EndNetworkGame once
    int anyRacerFinished;                          // +0x40: a racer has +0x7a4 set (0x0045eef0)
    int allRacersFinished;                         // +0x44: every racer has +0x7a4 or +0x4a0 set (0x0045eef0)
    unsigned char racesRun;                        // +0x48: RankRacers counts mode-2 races; dialogs show it as "%d/%d"
    int resultCount;                               // +0x4c: entries filled by 0x0045e9d0
    UnknownEventEntry results[11];                 // +0x50: one per racer, copied by RankRacers
    float endTimer;                                // +0x3c0: seconds since a racer finished (0x0045eef0); the "End Timer" debug row (TrackOverlay.cpp)
    Vector3 podiumPosition;                        // +0x3c4: where 0x0045d480 places the podium; characters stand at offsets from it
    ArcadeObject* podiumObject;                     // the podium (0x0045d480)
    Camera* podiumCamera;                           // a PCCamera (0x0045d480; constructor 0x004bed80)
    Vector3 podiumFocus;                           // +0x3d8: podiumPosition + (0, 6, 0), or + (0, 6, -6) in the other layout; 0x0045d480 and slot 10 aim podiumCamera (slot 29) at it
    Vector3 podiumCameraPosition;                           // podium camera position (0x0045d480)
    Vector3 podiumCameraTarget;                           // its look target
    unsigned char field_0x3fc[0x414 - 0x3fc];
    Vector3 podiumPanSpeed;                           // pan speed (per 7 seconds)
    int podiumCharacterCount;                      // min(+0x4c, 3) (0x0045d480)
    D3DIMSoultreeCharacter* podiumCharacters[3];
    Motion* podiumMotions[3];                      // "Podium3/4/5_%02d", one per character
    int field_0x43c;
    int resumeArmed;                               // +0x440: armed by slot 23, consumed by slot 22
    unsigned char field_0x444[0xd08 - 0x444];
};
