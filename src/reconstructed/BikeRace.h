#pragma once

#include <float.h>

#include "FollowCamera.h"
#include "KrustyBikeCamera.h"
#include "DebugAlloc.h"
#include "DebugOverlay.h"
#include "ObjectPicker.h"
#include "InGameProcs.h"
#include "JoystickDevice.h"
#include "KeyboardDevice.h"
#include "MatrixUtil.h"
#include "RaceStatus.h"
#include "Recorder.h"
#include "SceneManager.h"
#include "Track.h"
#include "TrackGame.h"
#include "Wrecker.h"

// Reconstruction of part of D:\aardvark\VC\krusty2\bikerace.cpp (literal
// __FILE__ at 0x00567c74; bikerace.h's at 0x00567de0).
//
// Confirmed (RTTI): BikeRace : GraphicsTest : GameObject : BaseObject, all
// single non-virtual inheritance; vtable 0x00550d24 (COL 0x0055ac68). It
// overrides slot 0 (deleting destructor 0x00417eb0, core 0x0041cf30) and
// slots 10, 14, 16, 20, 22, 23 and 24. The constructor 0x00417c30 is the
// only writer of the vtable besides the destructor.
//
// Strong inference: RaceView.h's UnknownKrustyBikeView is a view of the same
// object (EventManager 0x0045d2f0 returns it; its fields +0x38..+0x3f9 are
// the ones BikeRace's constructor initialises, and its non-virtual methods
// 0x0041f1d0..0x00423790 lie in this file). It keeps its own symbols because
// many reconstructed callers bind them.
//
// Extent (strong inference): the BikeRace code runs from the constructor's
// helpers at 0x00417b00 to slot 22 (0x00423880..0x004238bb); GR_BitString's
// code (0x004238c0..0x00423f6d) and BlockAllocator.cpp follow. Whether the
// plate number painter (0x00417500..0x00417aff, between BikeCamera's code
// and 0x00417b00) belongs to this file is open. Member names are
// provisional.

class CarProceduralCollision;
class LightManager;
struct UnknownTextureFormatChoice;
struct UnknownBikeRaceBikeSetup;
struct UnknownBikeRaceMeshGroup;
struct UnknownBikeRaceRacerInfo;
struct UnknownBikeRaceSceneCharacter;
struct UnknownBikeRaceMeshItem;
struct UnknownBikeRaceRacerPart;
struct UnknownBikeRaceNetMessage13;
struct UnknownBikeRaceView6c;

// One of KrustyVCR's eight 8-byte records at +0x25c (cleared by its inline
// constructor).
struct UnknownKrustyVcrRecord {
    UnknownKrustyVcrRecord() {
        networkPlayerId = 0;
        field_0x04 = 0;
    }
    int networkPlayerId;  // +0x0  same 8-byte grid entry as UnknownBikeRaceNetPlayer (+0 compared with racers' networkPlayerId)
    char field_0x04;
};

// One racer of a recording (KrustyVCR+0x300, 0x124 bytes; 0x0049c070 writes
// one and 0x0049c1e0 reads it back). The strings are copied with their
// lengths clamped to the array size less one.
struct UnknownKrustyVcrRacer {
    int networkId;                            // +0x000 network id
    char aiIndex;                             // +0x004 AI index
    char name[0x10];                          // +0x005 name
    char bikeFile[0x40];  // +0x15  SetRacer model argument (Res\Ghost.mcf); the loader reads it back into bikeFile
    char riderFile[0x40];  // +0x55  SetRacer rider argument (Res\GhostRider.mcf); the loader reads it back into riderFile
    char bikeName[0x40];  // +0x95  bike name; the loader passes it back to KrustyUI::ResolveBikeFile
    char riderName[0x40];  // +0xD5  rider name; the loader passes it back to KrustyUI::ResolveRiderFile
    int engineSize;                           // +0x118 engine size
    int engineKind;                           // +0x11c engine kind
    int plateNumber;  // +0x120  SetRacer's last argument is TrackGameMode plateNumber / slots[j].plateNumber; read back into plate
};

// RTTI: KrustyVCR : VCRInterface (vtable 0x00550e9c, 0xf8c bytes). Its
// constructor is inline at bikerace.cpp's `new` sites (lines 0x2b9, 0x10e7,
// 0x1103 and others), which also emit its implicit destructor 0x00419960
// and the deleting wrapper 0x00419940. BikeRace keeps the replay being
// recorded or played at +0x1a0 and the ghost at +0x1a4. Its methods sit
// after krustyui.cpp's code; only the members used here are declared.
class KrustyVCR : public VCRInterface {
public:
    KrustyVCR() {}

    // 0x0049bf10: opens `name` in `file` through `callback` for recording
    // (mode 0) or playback (mode 1); 0 on failure.
    int OpenRecording(UnknownRecorderCallback callback, int mode, char* name,
                              UnknownVcrFile* file);
    float RecordedLength();            // 0x0049c000: the recorded length
    void SetReplayLength(float time, int flag); // 0x0049bff0
    // 0x0049c010: saves the recording with its length and description.
    void SaveRecording(float length, char* description, int flag);
    // 0x0049c070: describes a racer of the recording: its network id, AI
    // index (a char), record slot, name, model files, bike and rider names,
    // engine size and kind and plate number. GetRacer (0x0049c1e0) copies
    // record `index`'s fields back out.
    void SetRacer(int id, char ai, int slot, char* name, char* model,
                               char* rider, char* bikeName, char* riderName, int engineSize,
                               int engineKind, int value);
    void GetRacer(int* id, char* ai, int index, char* name, char* model, char* rider,
                               char* bikeName, char* riderName, int* engineSize, int* engineKind,
                               int* value);

    float replayLength;                       // +0x0d8 SetReplayLength (BikeRace slot 10 each frame)
    // +0xdc..+0xf8c (0xeb0 bytes) is the block 0x0049bf10 and 0x0049c010 hand
    // to the recorder.
    char magic[8];                            // +0x0dc "MCMVCR" (0x0049bf10, mode 0)
    char version[8];                          // +0x0e4 version "2.0" plus 0x1a (the string at 0x0056da64)
    char description[0x20];                   // +0x0ec description (0x0049c010)
    float timePlayed;                         // +0x10c the time played (0x00421d50)
    // +0x110 TrackGame+0x2d70's 0x1ec bytes, the race settings from
    // raceMode on (copied by 0x0049bf10, mode 0).
    unsigned char raceSettings[0x25c - 0x110];
    UnknownKrustyVcrRecord gridEntries[8];  // +0x25C  raceSettings+0x14c (TrackGame+0x2ebc) copied with the settings: per grid slot player id (UnknownBikeRaceNetPlayer)
    unsigned char field_0x29c[0x2fc - 0x29c];
    int racerCount;                           // +0x2fc racers recorded (the loader)
    UnknownKrustyVcrRacer racers[11];         // +0x300 0x0049c070 / 0x0049c1e0
};

// The model at +0x1a0 of BikeRace+0x74's object: +0x274 groups at +0x28c
// whose items list ids (0x0041ea60 swaps one id for another; bikerace.cpp
// passes 1 and 2 around its "FlagStart"/"FlagLoop" motions).
struct UnknownBikeRaceMesh {
    // D3DIMSoultreeObject's methods (D3DIMSoulTree.h), as bikerace.cpp's
    // setup places the flag girl and the scene's start objects.
    void SetUseLighting(int value);    // 0x004444c0
    void SetPositionVec3(Vector3 position); // 0x004fc630: sets the translation
    void SetAxesPtr(const Vector3* look, const Vector3* up, int a, int b); // 0x004fbd70

    unsigned char field_0x000[0x274];
    int groupCount;                           // +0x274 group count
    unsigned char field_0x278[0x28c - 0x278];
    UnknownBikeRaceMeshGroup* levelGroups;  // +0x28C  per-level groups (+0x274 of them), as UnknownBikeRaceModel::levelGroups / D3DIMSoulTree lodTable
};

class UnknownBikeRaceCharacter {
public:
    virtual void UnknownVirtualSlot0();
    virtual void UnknownVirtualSlot1();
    virtual void UnknownVirtualSlot2();
    virtual void UnknownVirtualSlot3();
    virtual void UnknownVirtualSlot4();
    virtual void UnknownVirtualSlot5();
    virtual void UnknownVirtualSlot6();
    // Slot 7: per frame (BikeRace slot 10 passes the frame time).
    virtual int UnknownVirtualSlot7(float frameTime, int a, int b);
    virtual void UnknownVirtualSlot8();
    virtual void UnknownVirtualSlot9();
    virtual void UnknownVirtualSlot10();
    // Slot 11 (D3DIMSoultreeCharacter's CharacterVirtualSlot11): loads the
    // model file `name`; returns the GameObject to add.
    virtual GameObject* UnknownVirtualSlot11(void* owner, const char* name, LightManager* lights,
                                             UnknownTextureFormatChoice* textures, int a, int b);

    // 0x004a8b10: plays a motion ("Stand", "FlagStart", "FlagLoop").
    void SetMotionByName(const char* motion);

    unsigned char field_0x004[0x0c - 0x04];
    int motionFinished;                       // +0x00c the motion finished (slot 10 then loops "FlagLoop")
    unsigned char field_0x010[0x1a0 - 0x10];
    UnknownBikeRaceMesh* modelNode;           // +0x1a0
};

// The object at BikeRace+0x44: three GameObjects at +0x3c..+0x44 that a
// restart turns on (slot 16 with 0).
struct UnknownBikeRaceViews {
    unsigned char field_0x00[0x3c];
    GameObject* engineGroup;  // +0x3C  RaceSound+0x3c engineGroup (RaceSound.h); a restart re-enables it
    GameObject* field_0x40;
    GameObject* effectsGroup;  // +0x44  RaceSound+0x44 effectsGroup (RaceSound.h); a restart re-enables it
};

// An entry of the mesh's +0x28c table (8 bytes).
struct UnknownBikeRaceMeshGroup {
    int itemCount;                            // +0x00 item count
    UnknownBikeRaceMeshItem* surfaces;  // +0x4  itemCount surfaces (D3DIMSoulTree UnknownSoultreeLod::surfaces); ReplaceMeshId walks their ids
};

// An item of a mesh group (0x38 bytes).
struct UnknownBikeRaceMeshItem {
    unsigned char field_0x00[0x0c];
    int faceCount;                            // +0x0c face count
    unsigned char field_0x10[0x20 - 0x10];
    int idCount;                              // +0x20 id count
    int* ids;                                 // +0x24 ids
    unsigned char field_0x28[0x38 - 0x28];
};

// The object at BikeRace+0x19c (TrackOverlay.cpp's 0x0051da30 runs it).
class UnknownBikeRaceOverlay19c {
public:
    // 0x0051dce0: the chat line's input; nonzero when it took the event.
    int HandleChatInput(UnknownControlEvent* event, UnknownInputEntry* entry, int* result);
    void ShowInputLine();             // 0x0051dd10
    void HideInputLine();             // 0x0051dd40: closes the chat line
    void AddMessage(char* name, unsigned int* text, int flag); // 0x0051dd70

    unsigned char field_0x000[0x3dc];
    int typing;                               // +0x3dc typing
    int HandleChatKey(int value, int* result); // 0x0051da30
};


// A racer's model parts as 0x00423790 reaches them: the racer's +0x3bc and
// its rider's +0x1a0 take a detail level through 0x004451e0
// (D3DIMSoultreeModifier.cpp's code).
class UnknownBikeRaceModel {
public:
    void ApplyLodSettings(int level);    // 0x004451e0
    void SetLod(int value);    // 0x00444d00
    void SetLowestLod(int value);    // 0x00444d40: hidden for the followed racer
    void ClearFlag14c();             // 0x004fdb50
    void GetPosition(Vector3* position); // 0x004fc970: the translation
    // 0x00444c70 (D3DIMSoulTree.cpp): replaces texture `index` with `name`.
    int ReplaceMaterialTexture(int index, const char* name, int textures);

    unsigned char field_0x000[0x274];
    int detailLevelCount;                     // +0x274 detail level count
    unsigned char field_0x278[0x27c - 0x278];
    int currentDetailLevel;                   // +0x27c current detail level
    float* detailDistances;                   // +0x280 automatic detail distances (slot 10's debug page)
    unsigned char field_0x284[0x28c - 0x284];
    UnknownBikeRaceMeshGroup* levelGroups;    // +0x28c per level
};

struct UnknownBikeRaceRider {
    unsigned char field_0x000[0x1a0];
    UnknownBikeRaceModel* modelNode;          // +0x1a0
};

class BikeRace;
class UnknownBikeRaceProjector;

// The object at a racer's +0x604: its +0x38 collision is paired with the
// other racers' (the loader 0x00419970).
struct UnknownBikeRaceRacerPart604 {
    unsigned char field_0x00[0x38];
    CarProceduralCollision* collisionModel;  // +0x38  Wrecker+0x38 collisionModel (Wrecker.h); the loader pairs it with other racers' collisions
};

// A bike's engine setup (0x5c bytes): the loader copies TrackGameMode
// +0xfd8 (TrackGame+0x1550) and fills it from KrustyUI's garage tables for
// the bike class of the engine size.
struct UnknownBikeRaceBikeSetup {
    int engineSize;                           // +0x00 engine size (racer +0x738)
    int engineKind;                           // +0x04 engine kind (racer +0x737)
    int weight;                               // +0x08 KrustyUI "Weight"
    unsigned char field_0x0c[0x24 - 0x0c];
    int presetCurve[11];                      // +0x24 KrustyUI's preset 0 curve
    int rpmLowerLimit;                        // +0x50 "RPMLowerLimit"
    int rpmUpperLimit;                        // +0x54 "RPMUpperLimit"
    int rpmStep;                              // +0x58 a tenth of their range
};

// EventManager+0x444: one 0xcc-byte record per AI racer, kept through a
// series (mode 2) and reused while EventManager+0x48 is set.
struct UnknownBikeRaceEventAi {
    char riderTexture[0x40];                  // +0x00 rider texture name
    char bikeTexture[0x40];                   // +0x40 bike texture name
    char bikeModelPath[0x40];                 // +0x80 bike model path
    int engineSize;                           // +0xc0 engine size
    int engineKind;                           // +0xc4 engine kind
    float skill;                              // +0xc8 skill
};

// TrackGame+0x2ebc (race settings +0x14c): a network player's id and its
// +5 byte, set for the AI racers a host runs.
struct UnknownBikeRaceNetPlayer {
    int networkPlayerId;  // +0x0  compared with localPlayer and slots[j].networkPlayerId (loader, grid order)
    char field_0x04;
    char isAi;  // +0x5  set for a grid slot holding an AI racer of that player's host (racerIndex != 0)
    unsigned char field_0x06[2];
};

// The object at a racer's +0x740 (its view, BikeRace): +0x18c set keeps the
// racer's model at full detail when the camera follows another one.
struct UnknownBikeRaceRacerInfo {
    unsigned char field_0x000[0x18c];
    char forceHighLod;                        // +0x18c BikeRace's "ForceHighLOD"
};

// A racer as BikeRace's own code reaches it (its primary vtable at +0 and,
// as KrustyBike's RTTI shows, a virtual GameObject base through +4);
// UnknownEventRacer (RaceView.h) is the same object seen from EventManager.
class UnknownBikeRaceRacer : virtual public GameObject {
public:
    // 0x0048fa60: KrustyBike's constructor (0x1638 bytes; bikerace.cpp's
    // loader `new`s it with 1 and the most-derived flag).
    explicit UnknownBikeRaceRacer(int flags);
    virtual void UnknownRacerVirtualSlot0();
    virtual void UnknownRacerVirtualSlot1();
    virtual void UnknownRacerVirtualSlot2();
    virtual void UnknownRacerVirtualSlot3();
    virtual void UnknownRacerVirtualSlot4();
    virtual void UnknownRacerVirtualSlot5();
    virtual void UnknownRacerVirtualSlot6();
    virtual void UnknownRacerVirtualSlot7();
    virtual void UnknownRacerVirtualSlot8();
    virtual void UnknownRacerVirtualSlot9();
    virtual void UnknownRacerVirtualSlot10();
    virtual void UnknownRacerVirtualSlot11();
    virtual void UnknownRacerVirtualSlot12();
    virtual void UnknownRacerVirtualSlot13();
    virtual void UnknownRacerVirtualSlot14();
    virtual void UnknownRacerVirtualSlot15();
    virtual void UnknownRacerVirtualSlot16();
    virtual void UnknownRacerVirtualSlot17();
    virtual void UnknownRacerVirtualSlot18();
    virtual void UnknownRacerVirtualSlot19();
    virtual void UnknownRacerVirtualSlot20();
    virtual void UnknownRacerVirtualSlot21();
    virtual void UnknownRacerVirtualSlot22();
    virtual void UnknownRacerVirtualSlot23();
    virtual void UnknownRacerVirtualSlot24();
    virtual void UnknownRacerVirtualSlot25();
    virtual void UnknownRacerVirtualSlot26();
    virtual void UnknownRacerVirtualSlot27();
    virtual void UnknownRacerVirtualSlot28();
    virtual void UnknownRacerVirtualSlot29();
    virtual void UnknownRacerVirtualSlot30();
    virtual void UnknownRacerVirtualSlot31();
    virtual void UnknownRacerVirtualSlot32();
    virtual void UnknownRacerVirtualSlot33();
    virtual void UnknownRacerVirtualSlot34();
    virtual void UnknownRacerVirtualSlot35();
    virtual void UnknownRacerVirtualSlot36();
    virtual void UnknownRacerVirtualSlot37();
    virtual void UnknownRacerVirtualSlot38();
    virtual void UnknownRacerVirtualSlot39();
    virtual void UnknownRacerVirtualSlot40();
    virtual void UnknownRacerVirtualSlot41();
    virtual void UnknownRacerVirtualSlot42();
    virtual void UnknownRacerVirtualSlot43();
    // Its own slot 44 (0x00420590 calls it for a player who left).
    virtual void UnknownRacerVirtualSlot44();
    virtual float UnknownRacerVirtualSlot45(); // 0x0041d2b0 passes its result to slot 50 (KrustyBike's ret 0)
    virtual void UnknownRacerVirtualSlot46();
    virtual void UnknownRacerVirtualSlot47();
    virtual void UnknownRacerVirtualSlot48();
    virtual void UnknownRacerVirtualSlot49();
    virtual void UnknownRacerVirtualSlot50(int value, float time, int c); // KrustyBike's ret 0xc
    virtual void UnknownRacerVirtualSlot51();
    virtual void UnknownRacerVirtualSlot52();
    virtual void UnknownRacerVirtualSlot53();
    virtual void UnknownRacerVirtualSlot54();
    virtual void UnknownRacerVirtualSlot55();
    virtual void UnknownRacerVirtualSlot56();
    virtual void UnknownRacerVirtualSlot57();
    virtual void UnknownRacerVirtualSlot58();
    virtual void UnknownRacerVirtualSlot59();
    virtual void UnknownRacerVirtualSlot60();
    virtual void UnknownRacerVirtualSlot61();
    virtual void UnknownRacerVirtualSlot62();
    virtual void UnknownRacerVirtualSlot63();
    virtual void UnknownRacerVirtualSlot64();
    virtual void UnknownRacerVirtualSlot65();
    virtual void UnknownRacerVirtualSlot66();
    virtual void UnknownRacerVirtualSlot67();
    virtual void UnknownRacerVirtualSlot68();
    virtual void UnknownRacerVirtualSlot69();
    virtual void UnknownRacerVirtualSlot70();
    virtual void UnknownRacerVirtualSlot71();
    virtual void UnknownRacerVirtualSlot72();
    virtual void UnknownRacerVirtualSlot73();
    virtual void UnknownRacerVirtualSlot74();
    virtual void UnknownRacerVirtualSlot75();
    virtual void UnknownRacerVirtualSlot76();
    virtual void UnknownRacerVirtualSlot77();
    virtual void UnknownRacerVirtualSlot78();
    virtual void UnknownRacerVirtualSlot79();
    virtual void UnknownRacerVirtualSlot80();
    virtual void UnknownRacerVirtualSlot81();
    virtual void UnknownRacerVirtualSlot82();
    virtual void UnknownRacerVirtualSlot83();
    virtual void UnknownRacerVirtualSlot84();
    virtual void UnknownRacerVirtualSlot85();
    virtual void UnknownRacerVirtualSlot86();
    virtual void UnknownRacerVirtualSlot87();
    virtual void UnknownRacerVirtualSlot88();
    virtual void UnknownRacerVirtualSlot89();
    virtual void UnknownRacerVirtualSlot90();
    virtual void UnknownRacerVirtualSlot91();
    virtual void UnknownRacerVirtualSlot92();
    virtual void UnknownRacerVirtualSlot93();
    virtual void UnknownRacerVirtualSlot94(); // slot 10 calls it while TrackGameMode+0xa88 is set

    int UnknownFunction495c00();              // 0x00495c00 (KrustyBike.cpp's code)
    // 0x0048fc80 (KrustyBike.cpp's code, ret 0x7c): loads the bike and
    // rider models and sets the racer up; returns the GameObject to add.
    GameObject* UnknownFunction48fc80(void* owner, const char* a, const char* bikeFile, const char* riderFile,
                                      LightManager* lights, UnknownTextureFormatChoice* textures,
                                      Vector3 position, Vector3 direction, Vector3 up,
                                      UnknownBikeRaceProjector* terrain, ControlInterface* control, char ai,
                                      int remote, BikeRace* race, const char* name, int b, int id,
                                      UnknownBikeRaceBikeSetup* setup, int c, int d, int particles,
                                      Scene* scene, KrustyVCR* vcr, float skill, int e);

    // Its own slot 43 (0x00420650 calls it on each racer on a restart).
    void UnknownFunction496e20(KrustyVCR* vcr); // 0x00496e20 (KrustyBike.cpp's code)
    // 0x004933e0 (KrustyBike.cpp's code): applies message 13 to a state.
    void UnknownFunction4933e0(UnknownBikeRaceNetMessage13* message, UnknownBikeRaceRacerPart* part);

    unsigned char field_0x008[0x18 - 0x08];
    Vector3 carriedFlagPosition;              // +0x018 the flag carried above it (slot 10)
    unsigned char field_0x024[0x64 - 0x24];
    Vector3 velocity;                         // +0x064 velocity (cleared for a remote player's racer)
    unsigned char field_0x070[0x88 - 0x70];
    Vector3 heading;                          // +0x088 heading (the setup puts the scene's 30SecMan ahead of it)
    unsigned char field_0x094[0xbc - 0x94];
    int linearSpeed;  // +0xBC  SoultreePhysicsBaseObject+0xbc linearSpeed; zeroed with velocity at a network start
    unsigned char field_0x0c0[0x109 - 0xc0];
    bool restarted;                           // +0x109 set on a restart
    unsigned char field_0x10a[0x10c - 0x10a];
    Vector3 startPosition;                    // +0x10c start position (0x004210f0)
    Vector3 startDirection;                   // +0x118 start direction
    unsigned char field_0x124[0x128 - 0x124];
    CarProceduralCollision* collisionObject;  // +0x128  SoultreePhysicsBaseObject+0x128 collisionObject; its +0x68 is set without tree collision (the loader)
    unsigned char field_0x12c[0x3bc - 0x12c];
    UnknownBikeRaceModel* modelNode;          // +0x3bc
    unsigned char field_0x3c0[0x4a0 - 0x3c0];
    int outOfRace;                            // +0x4a0 out of the race (0x0041f1d0 skips it)
    unsigned char field_0x4a4[0x5bc - 0x4a4];
    int gasGyro;                              // +0x5bc TrackGameMode gasGyro copy (slot 23 key 0x22)
    int brakeGyro;                            // +0x5c0 TrackGameMode brakeGyro copy (slot 23 key 0x30)
    UnknownBikeRaceRider* riderCharacter;     // +0x5c4
    unsigned char field_0x5c8[0x5f0 - 0x5c8];
    CarProceduralCollision* frontWheel;       // +0x5f0 paired with the other racers' (the loader)
    CarProceduralCollision* rearWheel;        // +0x5f4
    unsigned char field_0x5f8[0x604 - 0x5f8];
    UnknownBikeRaceRacerPart604* wrecker;  // +0x604  Bike+0x604 object (SetupWrecker with rider.col, StartAnimatedWreck: Wrecker); its collision model is paired
    unsigned char field_0x608[0x734 - 0x608];
    unsigned char netIndex;                   // +0x734 racer index in network messages
    char isRemote;  // +0x735  KrustyBike loader's remote argument (1 for other players' bikes); gates the +0x1414/+0x1418 pairing
    unsigned char isGhost;  // +0x736  KrustyBike+0x736 isGhost (KrustyBike.h: set for loader kind 0xff)
    char engineKind;                          // +0x737 engine kind
    int engineSize;                           // +0x738 engine size
    int plateNumber;                          // +0x73c plate number (the setup paints it)
    UnknownBikeRaceRacerInfo* bikeView;       // +0x740 the racer's view (RaceView.h)
    UnknownEventRacerPart* statusNode;        // +0x744 race status node (RaceView.h)
    unsigned char field_0x748[0x74c - 0x748];
    float lastLapTime;                        // +0x74c (UnknownEventRacer's last lap time)
    float fastestLap;                         // +0x750 (0x0045c8b0 copies it to the entry's ZoneReport "FastestLap")
    unsigned char field_0x754[0x768 - 0x754];
    float score;                              // +0x768 score (network message 10)
    unsigned char field_0x76c[0x770 - 0x76c];
    int timeBehindLeader;  // +0x770  KrustyBike +0x770 timeBehindLeader; replayed from message 1's +0x2c
    unsigned char field_0x774[0x7a0 - 0x774];
    unsigned short lapCount;                  // +0x7a0 laps (UnknownEventRacer)
    unsigned char field_0x7a2[0x7a4 - 0x7a2];
    char finished;                            // +0x7a4 finished
    unsigned char field_0x7a5[0x7b8 - 0x7a5];
    int lapGate;                              // +0x7b8 RaceView.h's racer lapGate
    int nextLapGate;                          // +0x7bc
    unsigned char field_0x7c0[0x11b8 - 0x7c0];
    unsigned char hasFullState;  // +0x11B8  set when a message 1 state arrives; message 13 deltas are ignored until it is set
    unsigned char field_0x11b9[0x11bc - 0x11b9];
    int recorderId;                           // +0x11bc recorder id (0x00421d50 matches records by it)
    char aiMessageIndex;                      // +0x11c0 AI racer's index in its messages (RaceView.h)
    unsigned char field_0x11c1[0x11c4 - 0x11c1];
    float messageDelay;                       // +0x11c4 the player's message delay (racer slot +0xd0)
    UnknownBikeRaceRacerPart* receivedStates[4]; // +0x11c8 received network states, newest first
    unsigned char field_0x11d8[0x1358 - 0x11d8];
    Vector3 deltaBaseVelocity;  // +0x1358  base state message 13 deltas accumulate into (0x004933e0); set from message 1 velocity (+0x3c)
    Vector3 deltaBasePosition;  // +0x1364  message 13 delta base; set from message 1 position (+0x08)
    Vector3 deltaBaseAngularVelocity;  // +0x1370  message 13 delta base; set from message 1 angular velocity (+0x30)
    int deltaBaseRoll;  // +0x137C  message 13 delta base; set from message 1 roll (+0x14)
    int deltaBasePitch;  // +0x1380  message 13 delta base; set from message 1 pitch (+0x18)
    int deltaBaseYaw;  // +0x1384  message 13 delta base; set from message 1 yaw (+0x1c)
    int deltaBaseTime;  // +0x1388  message 13 delta base ms time (message 1 +0x4c); 0x004933e0 adds each delta's step
    unsigned char field_0x138c[0x13f8 - 0x138c];
    float recordStateTimer;  // +0x13F8  KrustyBike recordState(+0x1390).timer: s since the last full record; FLT_MAX/2 forces one
    unsigned char field_0x13fc[0x1414 - 0x13fc];
    CarProceduralCollision* altBodyA;  // +0x1414  KrustyBike+0x1414 altBodyA (KrustyBike.h); paired between remote racers by the loader
    CarProceduralCollision* altBodyB;  // +0x1418  KrustyBike+0x1418 altBodyB (KrustyBike.h); paired between remote racers by the loader
    unsigned char field_0x141c[0x160c - 0x141c]; // the GameObject virtual base follows (0x1638 bytes)
};

// A racer's network state (0x58 bytes, network message 1).
struct UnknownBikeRaceNetState {
    char field_0x00;
    char field_0x01;
    unsigned char field_0x02[0x04 - 0x02];
    int field_0x04;
    Vector3 position;  // +0x8  KbBikeMessage+0x08 position; copied to racer deltaBasePosition (+0x1364)
    int roll;  // +0x14  KbBikeMessage+0x14 roll; copied raw to racer +0x137c
    int pitch;  // +0x18  KbBikeMessage+0x18 pitch; copied raw to racer +0x1380
    int yaw;  // +0x1C  KbBikeMessage+0x1c yaw; copied raw to racer +0x1384
    unsigned char field_0x20[0x28 - 0x20];
    float fastestLap;  // +0x28  RecordFrame copies it to racer fastestLap (+0x750, as KbBikeMessage+0x28)
    int timeBehindLeader;  // +0x2C  KbBikeMessage+0x2c timeBehindLeader; replayed into racer timeBehindLeader (+0x770)
    Vector3 angularVelocity;  // +0x30  KbBikeMessage+0x30 angularVelocity; copied to racer +0x1370
    Vector3 velocity;  // +0x3C  KbBikeMessage+0x3c velocity; copied to racer +0x1358
    unsigned char field_0x48[0x4c - 0x48];
    int time;  // +0x4C  KbBikeMessage+0x4c time (ms); RecordFrame overwrites it with replay seconds * 1000
    unsigned char field_0x50[0x54 - 0x50];
    char racerIndex;                          // +0x54 racer index
    unsigned char field_0x55[0x58 - 0x55];
};

// Network message 13 (KrustyBike.cpp's 0x004933e0 applies it).
struct UnknownBikeRaceNetMessage13 {
    unsigned char field_0x00[0x16];
    char racerIndex;                          // +0x16 racer index
};

// Network message 10: a racer's score.
struct UnknownBikeRaceNetScore {
    char field_0x00;
    char racerIndex;                          // +0x01 racer index
    unsigned char field_0x02[0x04 - 0x02];
    float racerScore;  // +0x4  message 10 (KbBikePing+0x04 = KrustyBike+0x768 score); copied to racer score
};

// Network messages 0x84 (a time stamp at +4) and 0x85 (text at +4).
struct UnknownBikeRaceNetStamp {
    int field_0x00;
    unsigned int timeStamp;                   // +0x04 sender's time stamp
};

// One of a racer's three received states (+0x11c8, newest first).
struct UnknownBikeRaceRacerPart {
    UnknownBikeRaceNetState state;            // +0x00 the received state
    int messageFlags;                         // +0x58 the message's flags
    int fresh;                                // +0x5c fresh
};



// A 0x3c-byte gate: a box at +0x00 facing +0x0c with half extents +0x18,
// placed on the track at +0x2c. The course gates are DebugCalloc'd by
// 0x0041d0d0 (bikerace.cpp line 0x840) and chained through +0x38 from
// BikeRace+0xc8; the start and finish probes (BikeRace+0xcc and +0x108) are
// the same structure (BikeRace's constructor clears their +0x38). gameui.cpp
// 0x0047b670 tests crossings of either kind (RaceStatus.cpp), and slot 14
// draws the probes' boxes; 0x0041d1e0 measures the track distance between
// the probes' positions.
struct UnknownBikeRaceNode {
    Vector3 center;                           // +0x00 centre
    Vector3 direction;                        // +0x0c direction
    Vector3 halfExtents;                      // +0x18 half extents (across the direction for gates)
    float width;  // +0x24  AppendNode's scale; halfExtents = scale*dir*0.5; Create scales the finish arch by it / modelWidth
    int field_0x28;
    TrackPos trackPos;                        // +0x2c position along the track (DistanceAlong/PlaceNear operand, 0x004e6a50)
    UnknownBikeRaceNode* nextGate;            // +0x38 next gate
};

// The 0x18-byte source records 0x0041d0d0 copies.
struct UnknownBikeRaceNodeSource {
    Vector3 center;  // +0x0  AppendNode copies it into the node's center
    Vector3 direction;  // +0xC  AppendNode copies it into the node's direction
};

// The object 0x0041d170 walks: +0xac counts the entries at +0x420, and
// +0xd8 holds the records.
struct UnknownBikeRaceNodeEntry {
    unsigned char field_0x00[0x50];
    float width;  // +0x50  BuildNodeList passes it as AppendNode's scale (the node's width)
    int field_0x54;
};

struct UnknownBikeRaceNodeOwner {
    unsigned char field_0x000[0xac];
    int gateCount;  // +0xAC  BuildNodeList's loop bound (one gate node per entry)
    unsigned char field_0x0b0[0xd8 - 0xb0];
    UnknownBikeRaceNodeSource gateSources[1];  // +0xD8  BuildNodeList hands the records to AppendNode (center/direction per gate)
    unsigned char field_0x0f0[0x420 - 0xf0];
    UnknownBikeRaceNodeEntry* gateEntries[1];  // +0x420  BuildNodeList reads entry i's width and value
};

// The object at +0x210 of BikeRace+0x6c's object (the 30SecMan): a second base at +0xc
// holds the GameObject part that 0x00421050 releases (slot 2).
class UnknownBikeRaceColliderBase {
public:
    virtual void UnknownVirtualSlot0();
    int field_0x04;
    int field_0x08;
};

class UnknownBikeRaceCollider : public UnknownBikeRaceColliderBase, public GameObject {
};

// The object at BikeRace+0x64/+0x6c (StartGate.slb, 30SecMan.slt): like a racer, it reaches GameObject through
// a vbptr at +4 (0x00421050 calls slot 5 there).
struct UnknownBikeRaceView6c : virtual public GameObject {
    virtual void UnknownVirtualSlot0();
    unsigned char field_0x008[0x1a0 - 0x08];
    UnknownBikeRaceMesh* model;               // +0x1a0 model (the setup places it)
    unsigned char field_0x1a4[0x210 - 0x1a4];
    UnknownBikeRaceCollider* collisionObject; // +0x210 krusty2 CollisionCharacter::collisionObject
};

// The object at BikeRace+0x4c: 0x00507c10 moves a point (the track debug
// drawing passes each end point with three zeros).
class UnknownBikeRaceProjector {
public:
    // 0x00507c10 (Terrain::QueryGround): puts `point` on the ground; the
    // ground normal goes to `normal` when given.
    void QueryGround(Vector3* point, Vector3* normal, int a, unsigned char* b);

    unsigned char field_0x00[0x40];
    float gridCellSize;  // +0x40  Terrain+0x40 gridCellSize (Terrain.h); scales the start grid (0x004210f0)
};

// A GameObject's +0x25 flags, which GameObject.h keeps protected
// (0x0041eb20 tests bit 0 of scene objects).
struct UnknownBikeRaceObjectFlags {
    unsigned char field_0x00[0x25];
    unsigned char isEnabled : 1;              // +0x25
};

// A scene character (Scene+0xb4 entries with bit 3 set): it reaches its
// GameObject part, seen here only for its flags, through a vbptr at +4.
struct UnknownBikeRaceSceneCharacter : virtual public UnknownBikeRaceObjectFlags {
    virtual void UnknownVirtualSlot0();
};

// The members of BikeRace+0x50's KrustyBikeCamera that bikerace.cpp
// reads and writes (FollowCamera's +0x244 and +0x274, VehicleCamera's
// +0x384..+0x394, BikeCamera's +0x3b0 and KrustyBikeCamera's +0x3b4, which
// those headers keep protected and type by their own readers). The racer
// pointers are the racers this file follows.
struct UnknownBikeRaceCameraView {
    unsigned char field_0x000[0x244];
    int cameraState;                          // +0x244 camera state
    unsigned char field_0x248[0x268 - 0x248];
    int overrideActive;                       // +0x268 override active
    unsigned char field_0x26c[0x274 - 0x26c];
    unsigned char isSettled;  // +0x274  FollowCamera isSettled (the comment above names FollowCamera's +0x274)
    unsigned char field_0x275[0x384 - 0x275];
    UnknownBikeRaceSceneCharacter* targetCharacter;  // +0x384  VehicleCamera+0x384 target; UnknownFunctionTargetCharacter stores its scene character
    GameObject* targetCar;  // +0x388  VehicleCamera+0x388 targetCar; UnknownFunctionTargetObject stores its scene procedural object
    void* shadowCaster;                       // +0x38c a scene shadow caster
    unsigned char followingRacer;             // +0x390 following a racer
    unsigned char targetPointCached;          // +0x391 VehicleCamera 0x0052bc50; cleared on a new target
    unsigned char targetHeadingCached;        // +0x392 VehicleCamera 0x0052bac0; cleared on a new target
    UnknownBikeRaceRacer* vehicle;            // +0x394 VehicleCamera's followed vehicle
    unsigned char field_0x398[0x3b0 - 0x398];
    UnknownBikeRaceRacer* followedBike;       // +0x3b0 BikeCamera's bike (RaceSound.h)
    UnknownBikeRaceRacer* followedRacer;      // +0x3b4 KrustyBikeCamera's racer (RaceSound.h)

    // Inline (three times in 0x0041eb20): follows `racer`, showing the
    // previous racer's model again and hiding the new one's.
    void UnknownFunctionFollow(UnknownBikeRaceRacer* racer) {
        if (followedBike != 0 && !racer->bikeView->forceHighLod) {
            followedBike->modelNode->SetLowestLod(1);
            followedBike->riderCharacter->modelNode->SetLowestLod(1);
            if (followedBike->modelNode->currentDetailLevel == 0) {
                followedBike->modelNode->SetLod(1);
            }
            if (followedBike->riderCharacter->modelNode->currentDetailLevel == 0) {
                followedBike->riderCharacter->modelNode->SetLod(1);
            }
        }
        vehicle = racer;
        targetPointCached = 0;
        targetHeadingCached = 0;
        followingRacer = true;
        shadowCaster = 0;
        targetCharacter = 0;
        targetCar = 0;
        followedBike = racer;
        isSettled = 0;
        followedRacer = racer;
        if (!racer->bikeView->forceHighLod) {
            racer->modelNode->SetLowestLod(0);
            followedBike->riderCharacter->modelNode->SetLowestLod(0);
        }
        isSettled = 0;
    }

    // Inline (0x0041eb20): the camera targets one scene object instead.
    void UnknownFunctionTargetCaster(void* caster) {
        isSettled = 0;
        shadowCaster = caster;
        targetPointCached = 0;
        targetHeadingCached = 0;
        followingRacer = false;
        targetCharacter = 0;
        targetCar = 0;
    }
    void UnknownFunctionTargetCharacter(UnknownBikeRaceSceneCharacter* character) {
        targetCharacter = character;
        isSettled = 0;
        targetPointCached = 0;
        targetHeadingCached = 0;
        followingRacer = false;
        targetCar = 0;
        shadowCaster = 0;
    }
    void UnknownFunctionTargetObject(GameObject* object) {
        targetCar = object;
        isSettled = 0;
        targetPointCached = 0;
        targetHeadingCached = 0;
        followingRacer = false;
        targetCharacter = 0;
        shadowCaster = 0;
    }
};

// The ProjectedShadow at BikeRace+0x5c (ProjectedShadow.cpp's code).
class UnknownBikeRaceShadow {
public:
    void AddCaster(UnknownBikeRaceModel* model); // 0x004dab00: adds a caster
    void ClearCasters();             // 0x004dab90: clears the casters
};

// What slot 23 compares a pick against: a Scene+0xb8 caster's model (+0x08,
// whose +0x128 is its collision) or its +0x0c, and a Scene+0xb4 entry's
// character (+0x210) or object (+0x3c).
struct UnknownBikeRacePickModel {
    unsigned char field_0x000[0x128];
    int collisionObject;  // +0x128  physics object's collision object (SoultreePhysicsBaseObject+0x128); compared with the pick
};

struct UnknownBikeRacePickCaster {
    unsigned char flags;  // +0x0  UnknownSceneCaster+0 bits; bit 0 physics: a physics object at +0x08
    unsigned char field_0x01[0x08 - 0x01];
    UnknownBikeRacePickModel* physicsObject;  // +0x8  UnknownSceneCaster+0x08 physicsObject
    int collisionObject;  // +0xC  UnknownSceneCaster+0x0c collisionObject; compared with the pick
    unsigned char field_0x10[0x28 - 0x10];
};

struct UnknownBikeRacePickCharacter {
    unsigned char field_0x000[0x210];
    int collisionObject;                      // +0x210 krusty2 CollisionCharacter::collisionObject
};

struct UnknownBikeRacePickObject {
    unsigned char field_0x00[0x3c];
    int collisionObject;  // +0x3C  procedural scene object's collision; slot 23 compares it with ObjectPicker::PickAtCursor
};

// The replay recorder's record kinds -1 (a tick) carry the time played and
// the tick count.
struct UnknownBikeRaceVcrTick {
    float time;                               // +0x00 recorded length at the tick
    int tickCount;                            // +0x04 ticks recorded so far
};

// Record kind 0x11: the ghost's state (0x3c bytes). Kind 0x10's deltas
// (UnknownBikeRaceGhostDelta) accumulate into the ghost's copy of it.
struct UnknownBikeRaceGhostRecord {
    short field_0x00;
    unsigned short timeStep;                  // +0x02 time step (0x004a9aa0)
    Vector3 field_0x04;                       // ghost +0x5e0
    float field_0x10;                         // ghost +0x5f8
    float field_0x14;                         // ghost +0x5fc
    float field_0x18;                         // ghost +0x600
    Vector3 field_0x1c;                       // ghost +0x5ec
    Vector3 position;                         // +0x28 ghost +0x5d4
    int runningTime;                          // +0x34 ghost +0x604
    int field_0x38;
};

// Record kind 0x10: byte deltas of the ghost's state, 15/64 per unit for the
// vectors at +0x01 and +0x0b and pi/64 for the rest (the same layout as the
// network message 13 that KrustyBike.cpp's 0x004933e0 applies).
struct UnknownBikeRaceGhostDelta {
    char field_0x00;
    signed char delta1[3];                    // +0x01 x 15/64 -> ghost +0x5e0
    unsigned char step;                       // +0x04 bit 0: units of 8, bits 1-7: count
    signed char delta3[3];                    // +0x05 x pi/64 -> ghost +0x5f8..+0x600
    signed char delta2[3];                    // +0x08 x pi/64 -> ghost +0x5ec
    signed char delta0[3];                    // +0x0b x 15/64 -> ghost +0x5d4
};

// One of the ghost's four received states (newest first).
struct UnknownBikeRaceGhostPart {
    UnknownBikeRaceGhostRecord record;        // +0x00 the kind 0x11 record
    int recordTime;                           // +0x3c the record's time
    int fresh;                                // +0x40 fresh
};

// Record kind 0x87: the racer the ghost follows (by its +0x11bc id).
struct UnknownBikeRaceVcrFollow {
    int field_0x00;                           // not written by 0x004a9d20
    char field_0x04;                          // 0x004a9d20 clears the byte
    unsigned char field_0x05[0x08 - 0x05];
    int followedRacerId;  // +0x8  FollowRacer sends followedRacer->networkPlayerId; RecordFrame matches it to racer recorderId
};

// TrackGame+0x568's +0xdc object as the replay recorder callback reaches it
// (TrackGame.h keeps the type for 0x004a9d10's symbol): the ghost bike.
class UnknownBikeRaceGhost {
public:
    // 0x004a9aa0 (FollowRacer.cpp): applies record kind 0x10 to a state (as
    // 0x004933e0).
    void ApplyReplayDelta(const UnknownBikeRaceGhostDelta* delta, UnknownBikeRaceGhostPart* part);

    unsigned char field_0x000[0x4f8];
    int hasState;                             // +0x4f8 has a state
    UnknownBikeRaceGhostPart* receivedStates[4]; // +0x4fc newest first (0x00421d50 rotates them)
    unsigned char field_0x50c[0x5d4 - 0x50c];
    Vector3 position;                         // +0x5d4 position
    Vector3 field_0x5e0;
    Vector3 field_0x5ec;
    float field_0x5f8;
    float field_0x5fc;
    float field_0x600;
    int runningTime;                          // +0x604 running time
};

// KrustyUI+0x48 records (0xc8 bytes; slot 10's debug page names the
// player's bike model from +0x40) and the KrustyUI+0x50 records (0x94
// bytes) whose +0 indexes them.
struct UnknownBikeRaceUiModel {
    unsigned char field_0x00[0x40];
    char modelFile[0xc8 - 0x40];  // +0x40  KrustyUI entry "ModelFile"/"File" (UnknownKrustyUIModelEntry::modelFile)
};

struct UnknownBikeRaceUiChoice {
    int manufacturer;  // +0x0  KrustyUI bike entry +0: index into the manufacturers (+0x48)
    unsigned char field_0x04[0x48 - 0x04];
    char bikeName[0x44];                      // +0x48 bike name (the loader's AI racers)
    int engineSize;                           // +0x8c engine size
    int engineKind;                           // +0x90 engine kind
};

// Game+0x34, the second root object, seen for GameObject's name switches
// 0x00468dd0 and 0x00468f10 (protected in GameObject.h), which slot 10
// applies to the sound and effect children by name.
class UnknownBikeRaceRoot {
public:
    void DisableByClassName(const char* name);
    void EnableByClassName(const char* name);
};

// The debug lines' row counter (0x00567a88).
extern int g_UnknownGlobal567a88;

class BikeRace : public GraphicsTest {
public:
    explicit BikeRace(int flags);             // 0x00417c30
    virtual ~BikeRace();                      // 0x0041cf30 (deleting wrapper 0x00417eb0)
    // 0x0041d2b0: per frame: the replay recorder, the race start, the
    // flag girl and the debug page.
    virtual int Tick(float frameTime);
    // 0x00423410: debug drawing (the track, the probes and the debug lines
    // other files leave in globals) while +0x34 is set.
    virtual int UnknownVirtualSlot14();
    virtual int SetPaused(int value);            // 0x0041d260
    virtual int OnChar(int value);               // 0x0041f590
    // 0x00420040: network messages 1 (racer state), 13, 10 (score), 0x85
    // (chat text) and 0x84 (time stamps).
    virtual int OnNetworkMessage(int type, void* data, int from, int to, int flags);
    // 0x0041f5e0: keys: chat, camera targets, debug toggles and the debug
    // object picker.
    virtual int OnControlPress(UnknownControlEvent* event, UnknownInputEntry* entry);
    // 0x00423880: control 6 (modifier 0x80) toggles drawChildren (+0x3fd).
    virtual int OnControlRelease(UnknownControlEvent* event, UnknownInputEntry* entry);

    UnknownBikeRaceCameraView* UnknownFunctionCameraView() {
        return (UnknownBikeRaceCameraView*)raceCamera;
    }

    // Inline (bikerace.h line 0xd3; the destructor 0x0041cf30): frees the
    // +0xc8 gate list.
    void UnknownFunctionFreeNodes() {
        if (gateList != 0) {
            while (gateList != 0) {
                UnknownBikeRaceNode* node = gateList;
                gateList = node->nextGate;
                DebugFree(node, __FILE__, 0xd3);
            }
            gateList = 0;
        }
    }

    // 0x00417ed0 (ret 0x24; QuarryStuntEvent.cpp's loader calls it after
    // the constructor): sets the race up in `owner`'s scene: the track file
    // and its probes, the start objects, the flag girl, the number and bonus
    // overlays, the replay recorder, the loader 0x00419970, the dialogs,
    // the sound and the plate numbers. Returns this, or 0 after Release.
    BikeRace* Create(void* owner, void* particles, LightManager* lights,
                                    UnknownTextureFormatChoice* textures, UnknownBikeRaceProjector* terrain,
                                    KrustyBikeCamera* camera, Scene* scene, UnknownBikeRaceNodeOwner* event,
                                    UnknownBikeRaceShadow* shadow);
    // 0x00419970 (ret 0x14): loads the rider models and creates the racers;
    // 0 on failure.
    int CreateRacers(void* owner, LightManager* lights, UnknownTextureFormatChoice* textures,
                              UnknownBikeRaceProjector* terrain, UnknownBikeRaceNodeOwner* event);
    // 0x00417bc0: the racer numbers 1..+0x158 ordered by championship points.
    void RankByPoints(int* order);
    // 0x0041d0d0: appends a node for `source[index]` at `*tail`.
    int AppendNode(int index, UnknownBikeRaceNodeSource* source,
                   UnknownBikeRaceNode*** tail, float scale, int value);
    // 0x0041d170: one node per entry of `owner`; 0 when an allocation fails.
    int BuildNodeList(UnknownBikeRaceNodeOwner* owner);
    // 0x0041d1e0: sets +0x189 from the track distance between +0xf8 and +0x134.
    int UpdateTrackSeparation();
    void SetSceneTime(float time);   // 0x0041d2a0: the scene's time
    int AnyRacerRemote();              // 0x0041ea10: whether a racer is remote
    // 0x0041ea60: replaces id `from` with `to` in `character`'s mesh groups.
    void ReplaceMeshId(UnknownBikeRaceCharacter* character, int from, int to);
    // 0x0041eb20: points the camera at racer +0x14c (when it follows a
    // racer) or at scene object +0x154 / +0x150; 0 when that one is hidden.
    int AimCamera(int caster);
    UnknownBikeRaceRacer* UnknownFunctionRacerAt(int index);
    // 0x0041f1d0: the next (`forward`) or previous camera target: a racer
    // (`racers`), a scene object or a shadow caster.
    void NextCameraTarget(int forward, int racers, int objects);
    void ToggleReplayPause(int pauseOnly); // 0x0041f550: pauses, or with 0 toggles, +0x3f8
    UnknownBikeRaceRacer* NextBike(int* iterator); // 0x004204e0
    void HandlePlayerLeft(int player);   // 0x00420590
    void SaveReplay(char* path, char* description); // 0x00420b00: saves the recording, copies VCRtape.dat to `path`
    // 0x00420650 (InGameProcs.cpp ExitDlg): restarts the race; `mode` 0
    // plays the replay just recorded, otherwise records a new one.
    void RestartRace(int mode, char* path, char* description);
    void RestartReplay();             // 0x00420bd0: restarts the replay and ghost
    // 0x004210f0: writes two positions relative to `reference`.
    void WriteRelativePositions(Vector3* a, Vector3* b, void* reference, int flags);
    void ResetStartCountdown();             // 0x00421050
    // 0x00421d50: the recorder callback's work (0x004230e0 forwards to it).
    int RecordFrame(int a, void* data, int flag, int b, int* keep, int time);
    void ClearRacerParts();             // 0x00422ec0: zeroes every racer's parts
    void DrawTrack(Track* track); // 0x00423140: draws the track
    void ClearAiRacerParts(int index);    // 0x00423040: zeroes an AI racer's parts
    void SetDetailLevel(int level);    // 0x00423790: detail level

    unsigned char debugDraw;                  // +0x34 toggled by control 0x1f (modifier 0x80): slot 14 draws the track, probes and debug lines
    unsigned char field_0x035[0x38 - 0x35];
    UnknownBikeRaceRacer* localRacer;        // its own racer
    UnknownBikeRaceRacer** racerSlots;       // all racers, by racer slot
    UnknownBikeRaceRacer** aiRacers;       // AI racers (TrackGame+0x2d94 of them)
    UnknownBikeRaceViews* raceSound;          // +0x44 the RaceSound object (RaceSound::Create in the setup)
    Track* raceTrack;
    UnknownBikeRaceProjector* raceTerrain;  // +0x4C  Create's terrain argument; QueryGround (Terrain 0x00507c10) puts probes/debug lines on the ground; grid scale
    KrustyBikeCamera* raceCamera;            // the race camera
    int particleManager;  // +0x54  Create's particles argument (QuarryStuntEvent passes its particleManager); forwarded to every racer loader
    Scene* raceScene;
    UnknownBikeRaceShadow* raceShadow;  // +0x5C  Create's shadow argument (ProjectedShadow); camera targeting clears it and adds the target's casters
    GameObject* youIndicator;                 // +0x60 the spinning "youind.slt" marker over the local racer
    UnknownBikeRaceView6c* startGate;         // +0x64 the scene's "StartGate.slb" entry; reset by a restart (slot 5)
    int startGateEntry;                       // +0x68 its scene animation entry index
    UnknownBikeRaceView6c* thirtySecondMan;   // +0x6c the scene's "30SecMan.slt" entry
    int thirtySecondManEntry;                 // +0x70 its scene animation entry index
    UnknownBikeRaceCharacter* flagGirl;    // the flag girl (0x0041ea60's argument)
    int flagGirlState;  // +0x78  flag girl motion state: 0 Stand, 1 first FlagStart, 3 second FlagStart, 4 FlagLoop (slot 10)
    float aiSettings[11];  // +0x7C  Create's 11 constants: [0..1]/[5..6] AI skill ranges, [8..10] AI stunt chance % by difficulty
    int lodDebugPage;                         // +0x0a8
    int targetIndex;                          // +0xac caster or scene entry index the camera targets
    unsigned char targetKind;                 // +0xb0 camera target: 0xff a racer, 0 a static caster, 1 a scene entry
    unsigned char field_0x0b1[0xb4 - 0xb1];
    ObjectPicker* objectPicker;                // debug object picker
    int forceAiStep;  // +0xB8  KrustyBike slot 49 takes the BikeAI step 0x00413200 when aiKind or this is set; Create clears it
    int scoreBoard;                           // +0xbc score board: BonusObjectManager::SetupCounter's object
    int digitCounter;                         // +0xc0 NumberObjectManager::SetupDigits's object
    UnknownEventRacerPart* statusList;  // +0xC4  race status list: BuildStatusList/FreeStatusList/UpdateLapRace/UpdateGateRace operand
    UnknownBikeRaceNode* gateList;            // +0xc8 gate nodes linked through nextGate
    UnknownBikeRaceNode startProbe;          // start probe
    UnknownBikeRaceNode finishProbe;          // finish probe
    int hasStartProbe;                        // +0x144 LoadTrack found the start probe (debug draw)
    int hasFinishProbe;                       // +0x148 LoadTrack found the finish probe (debug draw)
    int targetRacer;                          // +0x14c racer the camera cycles to (0 local, then the AI racers)
    int targetEntry;                          // +0x150 scene animation entry the camera cycles to
    int targetCaster;                         // +0x154 static model caster the camera cycles to
    int racerCount;                          // racer count
    float startCountdown;                     // +0x15c: seconds until the start (8.2 with the 30SecMan, else 5.2)
    float startGateTimer;                     // +0x160 set to 5 at the start; at 0 the start gate entry restarts (slot 10)
    Vector3 gridFirstSlot;                      // start grid: first slot (0x004210f0)
    Vector3 gridSlotStep;                      // start grid: step between slots
    Vector3 gridDirection;                      // start grid: direction
    bool gridReady;                           // +0x188 WriteRelativePositions computed the start grid (cleared by a restart)
    unsigned char startBeforeFinish;  // +0x189  UpdateTrackSeparation: set unless finish->start track distance > 0; lap 1 is then a lead-in
    bool isRacing;                         // racing
    bool restartPending;                      // +0x18b set by RestartRace; stops the race update and clears isRacing next frame
    unsigned char forceHighLod;                // "ForceHighLOD"
    unsigned char trackLoaded;                // the track file loaded (the setup)
    bool networkStartPending;                   // +0x18e set when playerCount > 1; cleared when the per-racer network check passes and the countdown resets
    bool startStampSent;                      // +0x18f the 0x84 start time stamp was sent
    bool chatOpen;                            // +0x190 the chat input line is open and takes keys
    unsigned char field_0x191[0x194 - 0x191];
    float chatHideTimer;                      // +0x194 seconds until the chat input line hides (10 after a message)
    int chatInputLineSetting;  // +0x198  the constructor copies TrackGameMode chatInputLine
    UnknownBikeRaceOverlay19c* chatOverlay;   // +0x19c
    KrustyVCR* replayVcr;                   // replay being recorded or played
    KrustyVCR* ghostVcr;                   // ghost
    UnknownVcrFile* vcrFile;                  // +0x1a8 VCR file the replay and ghost recordings open
    float recordTickTime;                     // +0x1ac the tick record slot 10 queues (+0x1ac..+0x1b7): replay length
    int recordTickCount;                      // +0x1b0 ticks queued since the replay restarted
    int recordTickRecordCount;  // +0x1B4  tick record's 3rd dword: records per tick (racers, +1 tag ball, x2 with score records)
    float replayTime;                        // replay time
    float replayLength;                        // replay length
    float recorderTickElapsed;                        // seconds since the last recorder tick (slot 10)
    float replaySeekTarget;                        // replay seek target, -1 when none
    float bestGhostLength;                    // +0x1c8 best ghost lap length (FLT_MAX until one is recorded); seeds the racer's fastestLap
    float ghostTimeLimit;                        // "VCRGhostTimeLimit"
    float recordTimeLimit;                        // "VCRRecordTimeLimit"
    int replayFrames;                        // +0x1d4 frames played into replayTime (advanced by the replay speed)
    int replayLengthFrames;                  // +0x1d8 frames recorded into replayLength
    int replayMode;                          // replay mode
    int appliedReplayMode;                   // +0x1e0 replay mode whose sound state was last applied
    int lastRecordKind;  // +0x1E4  RecordFrame stores its record kind (-1 tick, -2 tape end, -3, 1, 13, 0x87...)
    int ghostLap;                            // +0x1e8 lap count when the ghost lap restarted (raceMode 4); ButLeft asks to save the ghost lap when > 0
    int savedCameraState;                    // +0x1ec camera state saved while a replay plays
    char ghostFilePlayed[0x104];                  // ghost file played
    char ghostFileRecorded[0x104];                  // ghost file recorded ("VCRghost.dat"/"VCRgtemp.dat")
    bool replayPaused;                         // +0x3f8: ToggleReplayPause
    bool replayStepFrame;                      // +0x3f9: ButFrame: advance one frame
    bool recorderTickDue;                     // +0x3fa a short record packet is due this frame
    bool scoreRecordDue;  // +0x3FB  set every 20th recorder tick (event 0/stunt); KrustyBike 0x00492670 then records message 10 (score)
    unsigned char isRecording;                // recording (0x00420650)
    unsigned char drawChildren;               // +0x3fd 1 by the constructor, toggled by control 6 (slot 22): slot 14 runs GameObject's draw only when set
};

// 0x00417b00: the debug picker's hit callback (slot 23 passes it to
// 0x004b0210); stores what was hit in the current race's picker.
void PickerHitCallback(CollisionObject* self, CollisionObject* other);
// Debug line end points and their flags, drawn by slot 14; BikeAI.cpp
// (0x00415ac5..0x00416686), RaceStatus.cpp (0x004e5b38..) and 0x0040e30e
// write them.
extern int g_UnknownGlobal577a00;
extern int g_UnknownGlobal578da8;
extern int g_UnknownGlobal578dac;
extern int g_UnknownGlobal578df8;
extern int g_UnknownGlobal578dd8;
extern Vector3 g_UnknownGlobal578de0[2];
extern Vector3 g_UnknownGlobal577a30[2];
extern Vector3 g_UnknownGlobal577a48[2];
extern Vector3 g_UnknownGlobal5779d8[2];
extern Vector3 g_UnknownGlobal577a18[2];
extern Vector3 g_UnknownGlobal689c30[2];
extern Vector3 g_UnknownGlobal689c48[2];
extern Vector3 g_UnknownGlobal578dc0[2];

// 0x004230e0 (cdecl; an UnknownRecorderCallback, its address is passed at
// 0x0041918a and later): the replay time in milliseconds, then 0x00421d50.
int RecorderCallback(int a, void* data, int flag, int b, int* keep, int time,
                     int* milliseconds);
// 0x00417b30 (cdecl): sorts `keys` in descending order, moving `values`
// along with them.
void SortDescending(int* keys, int* values, int count);
