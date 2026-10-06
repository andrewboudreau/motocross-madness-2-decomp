#pragma once

#include <float.h>

#include "FollowCamera.h"
#include "KrustyBikeCamera.h"
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
        field_0x00 = 0;
        field_0x04 = 0;
    }
    int field_0x00;
    char field_0x04;
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
    int UnknownFunction49bf10(UnknownRecorderCallback callback, int mode, char* name,
                              UnknownVcrFile* file);
    float UnknownFunction49c000();            // 0x0049c000: the recorded length
    // 0x0049c010: saves the recording with its length and description.
    void UnknownFunction49c010(float length, char* description, int flag);
    // 0x0049c070: describes the ghost rider (model files, rider and bike
    // names, engine size and kind).
    void UnknownFunction49c070(int a, int b, int c, TrackGameMode* mode, char* model,
                               char* rider, char* riderName, char* bikeName, int engineSize,
                               int engineKind, int value);

    unsigned char field_0x0d8[0x25c - 0xd8];
    UnknownKrustyVcrRecord field_0x25c[8];
    unsigned char field_0x29c[0xf8c - 0x29c];
};

// The model at +0x1a0 of BikeRace+0x74's object: +0x274 groups at +0x28c
// whose items list ids (0x0041ea60 swaps one id for another; bikerace.cpp
// passes 1 and 2 around its "FlagStart"/"FlagLoop" motions).
struct UnknownBikeRaceMesh {
    unsigned char field_0x000[0x274];
    int field_0x274;                          // group count
    unsigned char field_0x278[0x28c - 0x278];
    UnknownBikeRaceMeshGroup* field_0x28c;
};

class UnknownBikeRaceCharacter {
public:
    void UnknownFunction4a8b10(const char* motion); // 0x004a8b10: plays a motion ("Stand")

    unsigned char field_0x000[0x1a0];
    UnknownBikeRaceMesh* field_0x1a0;
};

// The object at BikeRace+0x44: three GameObjects at +0x3c..+0x44 that a
// restart turns on (slot 16 with 0).
struct UnknownBikeRaceViews {
    unsigned char field_0x00[0x3c];
    GameObject* field_0x3c;
    GameObject* field_0x40;
    GameObject* field_0x44;
};

// An entry of the mesh's +0x28c table (8 bytes).
struct UnknownBikeRaceMeshGroup {
    int field_0x00;                           // item count
    UnknownBikeRaceMeshItem* field_0x04;
};

// An item of a mesh group (0x38 bytes).
struct UnknownBikeRaceMeshItem {
    unsigned char field_0x00[0x20];
    int field_0x20;                           // id count
    int* field_0x24;                          // ids
    unsigned char field_0x28[0x38 - 0x28];
};

// The object at BikeRace+0x19c (TrackOverlay.cpp's 0x0051da30 runs it).
class UnknownBikeRaceOverlay19c {
public:
    // 0x0051dce0: the chat line's input; nonzero when it took the event.
    int UnknownFunction51dce0(UnknownControlEvent* event, UnknownInputEntry* entry, int* result);
    void UnknownFunction51dd10();             // 0x0051dd10
    void UnknownFunction51dd70(char* name, unsigned int* text, int flag); // 0x0051dd70

    unsigned char field_0x000[0x3dc];
    int field_0x3dc;                          // typing
    int UnknownFunction51da30(int value, int* result); // 0x0051da30
};


// A racer's model parts as 0x00423790 reaches them: the racer's +0x3bc and
// its rider's +0x1a0 take a detail level through 0x004451e0
// (D3DIMSoultreeModifier.cpp's code).
class UnknownBikeRaceModel {
public:
    void UnknownFunction4451e0(int level);    // 0x004451e0
    void UnknownFunction444d00(int value);    // 0x00444d00
    void UnknownFunction444d40(int value);    // 0x00444d40: hidden for the followed racer
    void UnknownFunction4fdb50();             // 0x004fdb50

    unsigned char field_0x000[0x27c];
    int field_0x27c;
};

struct UnknownBikeRaceRider {
    unsigned char field_0x000[0x1a0];
    UnknownBikeRaceModel* field_0x1a0;
};

// The object at a racer's +0x740: +0x18c set keeps the racer's model shown
// when the camera follows it.
struct UnknownBikeRaceRacerInfo {
    unsigned char field_0x000[0x18c];
    char field_0x18c;
};

// A racer as BikeRace's own code reaches it (its primary vtable at +0 and,
// as KrustyBike's RTTI shows, a virtual GameObject base through +4);
// UnknownEventRacer (RaceView.h) is the same object seen from EventManager.
class UnknownBikeRaceRacer : virtual public GameObject {
public:
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

    int UnknownFunction495c00();              // 0x00495c00 (KrustyBike.cpp's code)

    // Its own slot 43 (0x00420650 calls it on each racer on a restart).
    void UnknownFunction496e20(KrustyVCR* vcr); // 0x00496e20 (KrustyBike.cpp's code)
    // 0x004933e0 (KrustyBike.cpp's code): applies message 13 to a state.
    void UnknownFunction4933e0(UnknownBikeRaceNetMessage13* message, UnknownBikeRaceRacerPart* part);

    unsigned char field_0x008[0x109 - 0x08];
    bool field_0x109;                         // set on a restart
    unsigned char field_0x10a[0x10c - 0x10a];
    Vector3 field_0x10c;                      // start position (0x004210f0)
    Vector3 field_0x118;                      // start direction
    unsigned char field_0x124[0x3bc - 0x124];
    UnknownBikeRaceModel* field_0x3bc;
    unsigned char field_0x3c0[0x4a0 - 0x3c0];
    int field_0x4a0;                          // out of the race (0x0041f1d0 skips it)
    unsigned char field_0x4a4[0x5bc - 0x4a4];
    int field_0x5bc;                          // TrackGame+0x1004 copy (slot 23 key 0x22)
    int field_0x5c0;                          // TrackGame+0x1008 copy (slot 23 key 0x30)
    UnknownBikeRaceRider* field_0x5c4;
    unsigned char field_0x5c8[0x734 - 0x5c8];
    unsigned char field_0x734;                // racer index in network messages
    unsigned char field_0x735[0x737 - 0x735];
    char field_0x737;                         // engine kind
    int field_0x738;                          // engine size
    unsigned char field_0x73c[0x740 - 0x73c];
    UnknownBikeRaceRacerInfo* field_0x740;
    unsigned char field_0x744[0x74c - 0x744];
    float field_0x74c;
    float field_0x750;
    unsigned char field_0x754[0x768 - 0x754];
    float field_0x768;                        // score (network message 10)
    unsigned char field_0x76c[0x7a0 - 0x76c];
    unsigned short field_0x7a0;
    unsigned char field_0x7a2[0x7b8 - 0x7a2];
    int field_0x7b8;
    int field_0x7bc;
    unsigned char field_0x7c0[0x11b8 - 0x7c0];
    unsigned char field_0x11b8;
    unsigned char field_0x11b9[0x11c8 - 0x11b9];
    UnknownBikeRaceRacerPart* field_0x11c8[4];
    unsigned char field_0x11d8[0x1358 - 0x11d8];
    Vector3 field_0x1358;
    Vector3 field_0x1364;
    Vector3 field_0x1370;
    int field_0x137c;
    int field_0x1380;
    int field_0x1384;
    int field_0x1388;
};

// A racer's network state (0x58 bytes, network message 1).
struct UnknownBikeRaceNetState {
    char field_0x00;
    char field_0x01;
    unsigned char field_0x02[0x04 - 0x02];
    int field_0x04;
    Vector3 field_0x08;
    int field_0x14;
    int field_0x18;
    int field_0x1c;
    unsigned char field_0x20[0x30 - 0x20];
    Vector3 field_0x30;
    Vector3 field_0x3c;
    unsigned char field_0x48[0x4c - 0x48];
    int field_0x4c;
    unsigned char field_0x50[0x54 - 0x50];
    char field_0x54;                          // racer index
    unsigned char field_0x55[0x58 - 0x55];
};

// Network message 13 (KrustyBike.cpp's 0x004933e0 applies it).
struct UnknownBikeRaceNetMessage13 {
    unsigned char field_0x00[0x16];
    char field_0x16;                          // racer index
};

// Network message 10: a racer's score.
struct UnknownBikeRaceNetScore {
    char field_0x00;
    char field_0x01;                          // racer index
    unsigned char field_0x02[0x04 - 0x02];
    float field_0x04;
};

// Network messages 0x84 (a time stamp at +4) and 0x85 (text at +4).
struct UnknownBikeRaceNetStamp {
    int field_0x00;
    unsigned int field_0x04;                  // sender's time stamp
};

// One of a racer's three received states (+0x11c8, newest first).
struct UnknownBikeRaceRacerPart {
    UnknownBikeRaceNetState field_0x00;
    int field_0x58;                           // the message's flags
    int field_0x5c;                           // fresh
};



// A 0x3c-byte list node (DebugCalloc'd by 0x0041d0d0, bikerace.cpp line
// 0x840) chained through +0x38 from BikeRace+0xc8.
struct UnknownBikeRaceNode {
    Vector3 field_0x00;                       // copied from the source record
    Vector3 field_0x0c;
    float field_0x18;
    int field_0x1c;
    float field_0x20;
    float field_0x24;
    int field_0x28;
    unsigned char field_0x2c[0x38 - 0x2c];
    UnknownBikeRaceNode* field_0x38;
};

// The 0x18-byte source records 0x0041d0d0 copies.
struct UnknownBikeRaceNodeSource {
    Vector3 field_0x00;
    Vector3 field_0x0c;
};

// The object 0x0041d170 walks: +0xac counts the entries at +0x420, and
// +0xd8 holds the records.
struct UnknownBikeRaceNodeEntry {
    unsigned char field_0x00[0x50];
    float field_0x50;
    int field_0x54;
};

struct UnknownBikeRaceNodeOwner {
    unsigned char field_0x000[0xac];
    int field_0x0ac;
    unsigned char field_0x0b0[0xd8 - 0xb0];
    UnknownBikeRaceNodeSource field_0x0d8[1];
    unsigned char field_0x0f0[0x420 - 0xf0];
    UnknownBikeRaceNodeEntry* field_0x420[1];
};

// The object at +0x210 of BikeRace+0x6c's object: a second base at +0xc
// holds the GameObject part that 0x00421050 releases (slot 2).
class UnknownBikeRaceColliderBase {
public:
    virtual void UnknownVirtualSlot0();
    int field_0x04;
    int field_0x08;
};

class UnknownBikeRaceCollider : public UnknownBikeRaceColliderBase, public GameObject {
};

// The object at BikeRace+0x6c: like a racer, it reaches GameObject through
// a vbptr at +4 (0x00421050 calls slot 5 there).
struct UnknownBikeRaceView6c : virtual public GameObject {
    virtual void UnknownVirtualSlot0();
    unsigned char field_0x008[0x210 - 0x08];
    UnknownBikeRaceCollider* field_0x210;
};

void operator delete(void* p, const char* file, int line);

// The object at BikeRace+0x4c: 0x00507c10 moves a point (the track debug
// drawing passes each end point with three zeros).
class UnknownBikeRaceProjector {
public:
    void UnknownFunction507c10(Vector3* point, int a, int b, int c); // 0x00507c10
};

// A 0x3c-byte track probe (BikeRace+0xcc and +0x108): slot 14 draws the
// box at +0x00 with half extents +0x18; 0x0041d1e0 measures the track
// distance between the two probes' positions (+0x2c). BikeRace's
// constructor clears +0x38.
struct UnknownBikeRaceProbe {
    Vector3 field_0x00;                       // centre
    unsigned char field_0x0c[0x18 - 0x0c];
    Vector3 field_0x18;                       // half extents
    unsigned char field_0x24[0x2c - 0x24];
    TrackPos field_0x2c;
    int field_0x38;
};

// A GameObject's +0x25 flags, which GameObject.h keeps protected
// (0x0041eb20 tests bit 0 of scene objects).
struct UnknownBikeRaceObjectFlags {
    unsigned char field_0x00[0x25];
    unsigned char field_0x25_bit0 : 1;
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
    int field_0x244;                          // camera state
    unsigned char field_0x248[0x268 - 0x248];
    int field_0x268;                          // override active
    unsigned char field_0x26c[0x274 - 0x26c];
    unsigned char field_0x274;
    unsigned char field_0x275[0x384 - 0x275];
    UnknownBikeRaceSceneCharacter* field_0x384;
    GameObject* field_0x388;
    void* field_0x38c;                        // a scene shadow caster
    unsigned char field_0x390;                // following a racer
    unsigned char field_0x391;
    unsigned char field_0x392;
    UnknownBikeRaceRacer* field_0x394;
    unsigned char field_0x398[0x3b0 - 0x398];
    UnknownBikeRaceRacer* field_0x3b0;
    UnknownBikeRaceRacer* field_0x3b4;

    // Inline (three times in 0x0041eb20): follows `racer`, showing the
    // previous racer's model again and hiding the new one's.
    void UnknownFunctionFollow(UnknownBikeRaceRacer* racer) {
        if (field_0x3b0 != 0 && !racer->field_0x740->field_0x18c) {
            field_0x3b0->field_0x3bc->UnknownFunction444d40(1);
            field_0x3b0->field_0x5c4->field_0x1a0->UnknownFunction444d40(1);
            if (field_0x3b0->field_0x3bc->field_0x27c == 0) {
                field_0x3b0->field_0x3bc->UnknownFunction444d00(1);
            }
            if (field_0x3b0->field_0x5c4->field_0x1a0->field_0x27c == 0) {
                field_0x3b0->field_0x5c4->field_0x1a0->UnknownFunction444d00(1);
            }
        }
        field_0x394 = racer;
        field_0x391 = 0;
        field_0x392 = 0;
        field_0x390 = true;
        field_0x38c = 0;
        field_0x384 = 0;
        field_0x388 = 0;
        field_0x3b0 = racer;
        field_0x274 = 0;
        field_0x3b4 = racer;
        if (!racer->field_0x740->field_0x18c) {
            racer->field_0x3bc->UnknownFunction444d40(0);
            field_0x3b0->field_0x5c4->field_0x1a0->UnknownFunction444d40(0);
        }
        field_0x274 = 0;
    }

    // Inline (0x0041eb20): the camera targets one scene object instead.
    void UnknownFunctionTargetCaster(void* caster) {
        field_0x274 = 0;
        field_0x38c = caster;
        field_0x391 = 0;
        field_0x392 = 0;
        field_0x390 = false;
        field_0x384 = 0;
        field_0x388 = 0;
    }
    void UnknownFunctionTargetCharacter(UnknownBikeRaceSceneCharacter* character) {
        field_0x384 = character;
        field_0x274 = 0;
        field_0x391 = 0;
        field_0x392 = 0;
        field_0x390 = false;
        field_0x388 = 0;
        field_0x38c = 0;
    }
    void UnknownFunctionTargetObject(GameObject* object) {
        field_0x388 = object;
        field_0x274 = 0;
        field_0x391 = 0;
        field_0x392 = 0;
        field_0x390 = false;
        field_0x384 = 0;
        field_0x38c = 0;
    }
};

// The ProjectedShadow at BikeRace+0x5c (ProjectedShadow.cpp's code).
class UnknownBikeRaceShadow {
public:
    void UnknownFunction4dab00(UnknownBikeRaceModel* model); // 0x004dab00: adds a caster
    void UnknownFunction4dab90();             // 0x004dab90: clears the casters
};

// What slot 23 compares a pick against: a Scene+0xb8 caster's model (+0x08,
// whose +0x128 is its collision) or its +0x0c, and a Scene+0xb4 entry's
// character (+0x210) or object (+0x3c).
struct UnknownBikeRacePickModel {
    unsigned char field_0x000[0x128];
    int field_0x128;
};

struct UnknownBikeRacePickCaster {
    unsigned char field_0x00;                 // bit 0: a model at +0x08
    unsigned char field_0x01[0x08 - 0x01];
    UnknownBikeRacePickModel* field_0x08;
    int field_0x0c;
    unsigned char field_0x10[0x28 - 0x10];
};

struct UnknownBikeRacePickCharacter {
    unsigned char field_0x000[0x210];
    int field_0x210;
};

struct UnknownBikeRacePickObject {
    unsigned char field_0x00[0x3c];
    int field_0x3c;
};

// The debug lines' row counter (0x00567a88).
extern int g_UnknownGlobal567a88;

class BikeRace : public GraphicsTest {
public:
    explicit BikeRace(int flags);             // 0x00417c30
    virtual ~BikeRace();                      // 0x0041cf30 (deleting wrapper 0x00417eb0)
    // 0x00423410: debug drawing (the track, the probes and the debug lines
    // other files leave in globals) while +0x34 is set.
    virtual int UnknownVirtualSlot14();
    virtual int UnknownVirtualSlot16(int value); // 0x0041d260
    virtual int UnknownVirtualSlot20(int value); // 0x0041f590
    // 0x00420040: network messages 1 (racer state), 13, 10 (score), 0x85
    // (chat text) and 0x84 (time stamps).
    virtual int UnknownVirtualSlot24(int type, void* data, int from, int to, int flags);
    // 0x0041f5e0: keys: chat, camera targets, debug toggles and the debug
    // object picker.
    virtual int UnknownVirtualSlot23(UnknownControlEvent* event, UnknownInputEntry* entry);
    // 0x00423880: control 0x80 (any modifier) toggles +0x3fd.
    virtual int UnknownVirtualSlot22(UnknownControlEvent* event, UnknownInputEntry* entry);

    UnknownBikeRaceCameraView* UnknownFunctionCameraView() {
        return (UnknownBikeRaceCameraView*)field_0x050;
    }

    // Inline (bikerace.h line 0xd3; the destructor 0x0041cf30): frees the
    // +0xc8 nodes.
    void UnknownFunctionFreeNodes() {
        if (field_0x0c8 != 0) {
            while (field_0x0c8 != 0) {
                UnknownBikeRaceNode* node = field_0x0c8;
                field_0x0c8 = node->field_0x38;
                operator delete(node, __FILE__, 0xd3);
            }
            field_0x0c8 = 0;
        }
    }

    // 0x00417bc0: the racer numbers 1..+0x158 ordered by championship points.
    void UnknownFunction417bc0(int* order);
    // 0x0041d0d0: appends a node for `source[index]` at `*tail`.
    int UnknownFunction41d0d0(int index, UnknownBikeRaceNodeSource* source,
                              UnknownBikeRaceNode*** tail, float scale, int value);
    // 0x0041d170: one node per entry of `owner`; 0 when an allocation fails.
    int UnknownFunction41d170(UnknownBikeRaceNodeOwner* owner);
    // 0x0041d1e0: sets +0x189 from the track distance between +0xf8 and +0x134.
    int UnknownFunction41d1e0();
    void UnknownFunction41d2a0(float time);   // 0x0041d2a0: the scene's time
    int UnknownFunction41ea10();              // 0x0041ea10: whether a racer is remote
    // 0x0041ea60: replaces id `from` with `to` in `character`'s mesh groups.
    void UnknownFunction41ea60(UnknownBikeRaceCharacter* character, int from, int to);
    // 0x0041eb20: points the camera at racer +0x14c (when it follows a
    // racer) or at scene object +0x154 / +0x150; 0 when that one is hidden.
    int UnknownFunction41eb20(int caster);
    UnknownBikeRaceRacer* UnknownFunctionRacerAt(int index);
    // 0x0041f1d0: the next (`forward`) or previous camera target: a racer
    // (`racers`), a scene object or a shadow caster.
    void UnknownFunction41f1d0(int forward, int racers, int objects);
    void UnknownFunction41f550(int value);    // 0x0041f550: toggles +0x3f8
    UnknownBikeRaceRacer* UnknownFunction4204e0(int* iterator); // 0x004204e0
    void UnknownFunction420590(int player);   // 0x00420590
    void UnknownFunction420b00(char* path, char* description); // 0x00420b00
    // 0x00420650 (InGameProcs.cpp ExitDlg): restarts the race; `mode` 0
    // plays the replay just recorded, otherwise records a new one.
    void UnknownFunction420650(int mode, char* path, char* description);
    void UnknownFunction420bd0();             // 0x00420bd0: restarts the replay and ghost
    // 0x004210f0: writes two positions relative to `reference`.
    void UnknownFunction4210f0(Vector3* a, Vector3* b, void* reference, int flags);
    void UnknownFunction421050();             // 0x00421050
    // 0x00421d50: the recorder callback's work (0x004230e0 forwards to it).
    int UnknownFunction421d50(int a, void* data, int flag, int b, int* keep, int time);
    void UnknownFunction422ec0();             // 0x00422ec0: zeroes every racer's parts
    void UnknownFunction423140(Track* track); // 0x00423140: draws the track
    void UnknownFunction423040(int index);    // 0x00423040: zeroes an AI racer's parts
    void UnknownFunction423790(int level);    // 0x00423790: detail level

    unsigned char field_0x034;
    unsigned char field_0x035[0x38 - 0x35];
    UnknownBikeRaceRacer* field_0x038;        // its own racer
    UnknownBikeRaceRacer** field_0x03c;       // all racers, by racer slot
    UnknownBikeRaceRacer** field_0x040;       // AI racers (TrackGame+0x2d94 of them)
    UnknownBikeRaceViews* field_0x044;
    Track* field_0x048;
    UnknownBikeRaceProjector* field_0x04c;
    KrustyBikeCamera* field_0x050;            // the race camera
    int field_0x054;
    Scene* field_0x058;
    UnknownBikeRaceShadow* field_0x05c;
    GameObject* field_0x060;
    UnknownBikeRaceView6c* field_0x064;       // reset by a restart (slot 5)
    int field_0x068;
    UnknownBikeRaceView6c* field_0x06c;
    int field_0x070;
    UnknownBikeRaceCharacter* field_0x074;    // the flag girl (0x0041ea60's argument)
    int field_0x078;
    float field_0x07c[11];
    int field_0x0a8;
    int field_0x0ac;
    unsigned char field_0x0b0;
    unsigned char field_0x0b1[0xb4 - 0xb1];
    ObjectPicker* field_0x0b4;                // debug object picker
    int field_0x0b8;
    int field_0x0bc;
    int field_0x0c0;
    UnknownEventRacerPart* field_0x0c4;
    UnknownBikeRaceNode* field_0x0c8;
    UnknownBikeRaceProbe field_0x0cc;
    UnknownBikeRaceProbe field_0x108;
    int field_0x144;
    int field_0x148;
    int field_0x14c;
    int field_0x150;
    int field_0x154;
    int field_0x158;                          // racer count
    float field_0x15c;
    int field_0x160;
    unsigned char field_0x164[0x188 - 0x164];
    bool field_0x188;
    unsigned char field_0x189;
    bool field_0x18a;                         // racing
    bool field_0x18b;
    unsigned char field_0x18c;                // "ForceHighLOD"
    bool field_0x18d;
    bool field_0x18e;
    bool field_0x18f;
    bool field_0x190;
    unsigned char field_0x191[0x194 - 0x191];
    float field_0x194;
    int field_0x198;
    UnknownBikeRaceOverlay19c* field_0x19c;
    KrustyVCR* field_0x1a0;                   // replay being recorded or played
    KrustyVCR* field_0x1a4;                   // ghost
    UnknownVcrFile* field_0x1a8;
    int field_0x1ac;
    int field_0x1b0;
    int field_0x1b4;
    float field_0x1b8;                        // replay time
    float field_0x1bc;                        // replay length
    int field_0x1c0;
    float field_0x1c4;                        // replay seek target, -1 when none
    float field_0x1c8;
    float field_0x1cc;                        // "VCRGhostTimeLimit"
    float field_0x1d0;                        // "VCRRecordTimeLimit"
    int field_0x1d4;
    int field_0x1d8;
    int field_0x1dc;                          // replay mode
    int field_0x1e0;
    int field_0x1e4;
    int field_0x1e8;
    int field_0x1ec;
    char field_0x1f0[0x104];                  // ghost file played
    char field_0x2f4[0x104];                  // ghost file recorded ("VCRghost.dat"/"VCRgtemp.dat")
    bool field_0x3f8;
    bool field_0x3f9;
    bool field_0x3fa;
    bool field_0x3fb;
    unsigned char field_0x3fc;                // recording (0x00420650)
    unsigned char field_0x3fd;                // toggled by slot 22
};

// The global at 0x00689f18 (D3DIMSoultreeModifier.cpp's code reads it):
// 0x00423790 points it at one of two tables, by Game+0x2d0.
extern unsigned char* g_UnknownGlobal689f18;
extern unsigned char g_UnknownGlobal5744c8[];
extern unsigned char g_UnknownGlobal574428[];

// 0x00417b00: the debug picker's hit callback (slot 23 passes it to
// 0x004b0210); stores what was hit in the current race's picker.
void UnknownFunction417b00(CollisionObject* self, CollisionObject* other);
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
int UnknownFunction4230e0(int a, void* data, int flag, int b, int* keep, int time,
                          int* milliseconds);
// 0x00417b30 (cdecl): sorts `keys` in descending order, moving `values`
// along with them.
void UnknownFunction417b30(int* keys, int* values, int count);
