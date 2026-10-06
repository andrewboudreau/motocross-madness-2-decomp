#pragma once

// Krusty3DObjects.h -- reconstruction of part of
// D:\aardvark\VC\krusty2\Krusty3DObjects.cpp (literal __FILE__ at
// 0x0056c98c). Evidence, extent and the per-function table are in
// docs/KRUSTY3DOBJECTS.md.
//
// Confirmed (RTTI, all single non-virtual inheritance at offset 0):
//   RunwayLights : GameObject          vtable 0x0055429c, COL 0x0055cee0
//   VisualCue : ArcadeObject : GameObject  vtable 0x00554320, COL 0x0055cf38
//   NumberObjectManager : GameObject   vtable 0x00554390, COL 0x0055cf88
//   BonusObjectManager : GameObject    vtable 0x00554400, COL 0x0055cfd8
// Member and method names are provisional (tier 3).
//
// src/reconstructed/QuarryEvent.h keeps QuarryStuntEvent.cpp's own partial
// view of VisualCue (it reads the cue's +0x48/+0x4c as floats); the two
// headers are never included together.

#include "ArcadeObject.h"
#include "D3DIMSoulTree.h"
#include "GameObject.h"
#include "MatrixUtil.h"

// 0x00460b50 (cdecl): table-driven square root (FollowCamera.h declares the
// same function).
float UnknownFunction460b50(float value);

struct UnknownControlEvent;
struct UnknownInputEntry;

// A racer as VisualCue sees it (the objects 0x004204e0 returns; RaceView.h's
// UnknownEventRacer): a vfptr at +0, GameObject reached through the vbptr at
// +4. Only the +0x25 bit 0 test is used.
struct UnknownVisualCueRacer : virtual public GameObject {
    virtual void UnknownVirtualSlot0();
    int UnknownIsEnabled() { return field_0x25_bit0; }

    unsigned char field_0x08[0x0c - 0x08];
    Vector3 field_0x0c;                // position
};

// The camera NumberObjectManager places its digits in front of (its +0x34,
// also the ArcadeObject view whose +0x198 scales the models).
struct UnknownNumberCamera {
    unsigned char field_0x000[0x170];
    Vector3 field_0x170;               // position
    Vector3 field_0x17c;               // forward
    Vector3 field_0x188;               // up
    unsigned char field_0x194[0x198 - 0x194];
    float field_0x198;
};

struct UnknownNumberRacerState {
    unsigned char field_0x000[0x76c];
    float field_0x76c;
};

// NumberObjectManager's +0x2c.
struct UnknownNumberRacer {
    unsigned char field_0x000[0x38];
    UnknownNumberRacerState* field_0x38;
    unsigned char field_0x03c[0x6c - 0x3c];
    int field_0x6c;
    unsigned char field_0x070[0x15c - 0x70];
    float field_0x15c;                 // count-down value
};

// 0x00460c00 (cdecl): 1/sqrt of `value` (table driven).
float UnknownFunction460c00(float value);

// A track position (RaceView.h's TrackPos: node, segment, t).
struct UnknownRunwayTrackPos {
    void* field_0x00;
    void* field_0x04;
    float field_0x08;
};

// The track a racer's view holds at +0x48 (Track.h's Track).
struct UnknownRunwayTrack {
    // 0x00518080: the world point of `pos`.
    int UnknownFunction518080(UnknownRunwayTrackPos pos, Vector3* out);
};

struct UnknownRunwayView {
    unsigned char field_0x00[0x48];
    UnknownRunwayTrack* field_0x48;
};

// The racer's race status node (RaceView.h's UnknownEventRacerPart).
struct UnknownRunwayRacerPart {
    unsigned char field_0x00[0x38];
    UnknownRunwayTrackPos field_0x38;
    UnknownRunwayTrackPos field_0x44;
};

// The racer the lights point for (NationalRace.cpp passes its view's
// +0x38; RaceView.h's UnknownEventRacer).
struct UnknownRunwayRacer {
    unsigned char field_0x000[0x10c];
    Vector3 field_0x10c;               // track point
    Vector3 field_0x118;               // track direction there
    unsigned char field_0x124[0x740 - 0x124];
    UnknownRunwayView* field_0x740;
    UnknownRunwayRacerPart* field_0x744;
    unsigned char field_0x748[0x78c - 0x748];
    int field_0x78c;
    unsigned char field_0x790[0x7a4 - 0x790];
    char field_0x7a4;                  // finished
};

// The terrain (NationalRace.cpp's +0x40): 0x00507c10 drops `point` onto it.
struct UnknownRunwayTerrain {
    void UnknownFunction507c10(Vector3* point, Vector3* normal, int a, int b);
};

// 0x4c bytes (NationalRace.cpp 0x004aa8cd): five "pointer.slt" models.
class RunwayLights : public GameObject {
public:
    explicit RunwayLights(int flags);          // 0x0048a5b0
    virtual int UnknownVirtualSlot10(float frameTime); // 0x0048a760

    // 0x0048a600 (ret 0x10): slot 8, then loads the five lights (line 0x40).
    RunwayLights* UnknownFunction48a600(void* value, int a, int b, UnknownRunwayTerrain* terrain);
    void UnknownFunction48ad50(UnknownRunwayRacer* racer); // 0x0048ad50: sets +0x30

    float field_0x2c;                          // blink timer
    UnknownRunwayRacer* field_0x30;
    UnknownRunwayTerrain* field_0x34;
    ArcadeObject* field_0x38[5];
};

// The object at a view's +0x50 (KrustyBikeCamera; +0x3b0 is the racer it
// follows).
struct UnknownVisualCueCamera {
    unsigned char field_0x000[0x3b0];
    UnknownRunwayRacer* field_0x3b0;
};

// The view VisualCue follows (its +0xa8; RaceView.h's UnknownKrustyBikeView).
struct UnknownVisualCueView {
    // 0x004204e0: the next racer after `*iterator` (advancing it), or 0.
    UnknownVisualCueRacer* UnknownFunction4204e0(int* iterator);

    unsigned char field_0x000[0x38];
    UnknownRunwayRacer* field_0x38;    // its own racer
    unsigned char field_0x03c[0x48 - 0x3c];
    UnknownRunwayTrack* field_0x48;
    unsigned char field_0x04c[0x50 - 0x4c];
    UnknownVisualCueCamera* field_0x50;
    unsigned char field_0x054[0x18a - 0x54];
    bool field_0x18a;                  // racing
    unsigned char field_0x18b[0x190 - 0x18b];
    bool field_0x190;
};

// 0xfc bytes (the allocation at 0x004e06cd).
class VisualCue : public ArcadeObject {
public:
    explicit VisualCue(int flags);             // 0x0048ad60
    virtual int UnknownVirtualSlot10(float frameTime); // 0x0048b100
    virtual int UnknownVirtualSlot14();        // 0x0048bc10
    virtual int UnknownVirtualSlot23(UnknownControlEvent* event, UnknownInputEntry* entry); // 0x0048afa0

    // 0x0048adf0 (ret 0x34): loads "viscue.slt"; returns this, or 0.
    VisualCue* UnknownFunction48adf0(void* value, int a, int b, UnknownRunwayTerrain* terrain, Vector3 position,
                                     UnknownVisualCueView* view, int pixels, UnknownArcadeView* camera,
                                     float size, float c, float d);
    // 0x0048af20 (ret 4): collects the view's racers.
    void UnknownFunction48af20(UnknownVisualCueView* view);
    int UnknownFunction48bc30();               // 0x0048bc30: the next cue index when it changed, else -1
    int UnknownFunction48bc80();               // 0x0048bc80

    // ArcadeObject.h types +0x48/+0x4c as int (0x00401310's `c` and `d`);
    // the cue stores screen fractions there (0.5f, 0.1f from its callers).
    float UnknownScreenX() { return *(float*)&field_0x48; }
    float UnknownScreenY() { return *(float*)&field_0x4c; }
    // The placement block slot 10 repeats in each mode.
    void UnknownPlace();

    UnknownVisualCueView* field_0xa8;
    UnknownRunwayTerrain* field_0xac;
    float field_0xb0;                          // field of view the offsets were built for
    float field_0xb4;                          // screen offset along the camera's x axis
    float field_0xb8;                          // screen offset along the camera's y axis
    int field_0xbc;                            // toggled by control 0x14
    int field_0xc0;                            // current racer
    int field_0xc4;                            // last reported racer
    int field_0xc8;                            // racer to skip
    UnknownVisualCueRacer* field_0xcc[11];
    int field_0xf8;                            // racers in +0xcc
};

// The digit models "five.slt" .. "one.slt".
class NumberObjectManager : public GameObject {
public:
    explicit NumberObjectManager(int flags);   // 0x0048bca0
    virtual int UnknownVirtualSlot10(float frameTime); // 0x0048bfc0
    virtual int UnknownVirtualSlot14();        // 0x0048c290

    // 0x0048bd00 (ret 0x18): loads the five digits (lines 0x23d, 0x240).
    NumberObjectManager* UnknownFunction48bd00(void* value, int b, UnknownNumberRacer* racer, int pixels,
                                               UnknownNumberCamera* camera, float size);

    UnknownNumberRacer* field_0x2c;
    int field_0x30;
    UnknownNumberCamera* field_0x34;
    ArcadeObject* field_0x38[5];
};

// The racer a BonusObjectManager follows (its +0x2c).
struct UnknownBonusRacerPart {
    // 0x004fc970: writes the part's translation into *translation.
    void UnknownFunction4fc970(Vector3* translation);
};

struct UnknownBonusRacerModel {
    unsigned char field_0x000[0x230];
    Vector3 field_0x230;
};

struct UnknownBonusRacerState {
    unsigned char field_0x000[0x3bc];
    UnknownBonusRacerPart* field_0x3bc;
    unsigned char field_0x3c0[0x444 - 0x3c0];
    int field_0x444;
    unsigned char field_0x448[0x5f4 - 0x448];
    UnknownBonusRacerModel* field_0x5f4;
};

struct UnknownBonusRacer {
    unsigned char field_0x000[0x38];
    UnknownBonusRacerState* field_0x38;
};

// The camera a BonusObjectManager faces (its +0x38).
struct UnknownBonusCamera {
    unsigned char field_0x000[0x16c];
    float field_0x16c;
    Vector3 field_0x170;               // position
    unsigned char field_0x17c[0x198 - 0x17c];
    float field_0x198;
};

// The game global at 0x0056e26c (TrackGame.h's g_UnknownGlobal56e26c) as
// this file reads it: +0x10 is the render target, whose +0x08 is the
// current camera; +0x2d74 the camera mode; +0x3430 blocks the UI.
struct UnknownBonusRenderTarget {
    unsigned char field_0x00[0x08];
    UnknownBonusCamera* field_0x08;
};

struct UnknownKrustyRacerRef {
    unsigned char field_0x000[0x7b8];
    int field_0x7b8;                   // gate index
};

struct UnknownKrustyCamera {
    unsigned char field_0x000[0x3b4];
    UnknownKrustyRacerRef* field_0x3b4;
};

struct UnknownKrustyCameraHolder {
    unsigned char field_0x00[0x50];
    UnknownKrustyCamera* field_0x50;
};

// One 24-byte gate record of the course at the global's +0x560.
struct UnknownKrustyGate {
    Vector3 field_0x00;
    unsigned char field_0x0c[0x18 - 0x0c];
};

struct UnknownKrustyCourse {
    unsigned char field_0x00[0x34];
    UnknownKrustyCameraHolder* field_0x34;
    unsigned char field_0x38[0xd8 - 0x38];
    UnknownKrustyGate field_0xd8[1];
};

struct UnknownKrustyFollowed {
    unsigned char field_0x000[0x25];
    unsigned char field_0x25;
    unsigned char field_0x026[0x3b4 - 0x26];
    UnknownVisualCueRacer* field_0x3b4;
};

struct UnknownKrustyNode {
    unsigned char field_0x000[0x21c];
    UnknownBonusRacerPart field_0x21c;
};

struct UnknownKrustyPlayer {
    unsigned char field_0x00[0x3c];
    UnknownKrustyFollowed* field_0x3c;
    unsigned char field_0x40[0xa8 - 0x40];
    UnknownVisualCueRacer* field_0xa8;
    unsigned char field_0xac[0xdc - 0xac];
    UnknownKrustyNode* field_0xdc;
};

struct UnknownKrustyGame {
    unsigned char field_0x0000[0x10];
    UnknownBonusRenderTarget* field_0x10;
    unsigned char field_0x0014[0x560 - 0x14];
    UnknownKrustyCourse* field_0x560;
    unsigned char field_0x0564[0x568 - 0x564];
    UnknownKrustyPlayer* field_0x568;
    unsigned char field_0x056c[0x2d74 - 0x56c];
    int field_0x2d74;                  // camera mode 0..5
    unsigned char field_0x2d78[0x2eb4 - 0x2d78];
    int field_0x2eb4;
    unsigned char field_0x2eb8[0x3430 - 0x2eb8];
    int field_0x3430;
};
extern UnknownKrustyGame* g_UnknownKrustyGame56e26c;

// One 0x30-byte key of the bonus animation table at 0x0056c820 (seven
// keys); plain floats, as the table is static data.
struct UnknownBonusKey {
    int field_0x00;                    // frame
    float field_0x04;                  // time
    float field_0x08[3];               // scale
    float field_0x14[3];               // position
    float field_0x20[3];               // rotation axis
    float field_0x2c;                  // rotation angle
};

// "Base Bonus Frame" and its digit models.
class BonusObjectManager : public GameObject {
public:
    explicit BonusObjectManager(int flags);    // 0x0048c2b0
    virtual ~BonusObjectManager();             // 0x0048c320 (deleting wrapper 0x0048c300)
    virtual int UnknownVirtualSlot10(float frameTime); // 0x0048ccc0
    virtual int UnknownVirtualSlot14();        // 0x0048d620

    // 0x0048c330 (ret 0xc): loads the digit, "x" and decimal point models.
    int UnknownFunction48c330(void* value, int a, int b);
    // 0x0048c8a0 (ret 0xc): creates the frame node (line 0x326).
    int UnknownFunction48c8a0(void* value, int a, int b);
    // 0x0048ca60 (ret 0x14): slot 8, then both loaders; returns this, or 0.
    BonusObjectManager* UnknownFunction48ca60(void* value, int a, int b, UnknownBonusRacer* racer,
                                              UnknownBonusCamera* camera);
    void UnknownFunction48cad0();              // 0x0048cad0: steps to the next key
    void UnknownFunction48d1e0(float value, float fraction); // 0x0048d1e0 (ret 8): shows a value
    void UnknownFunction48d540();              // 0x0048d540: hides the shown digits

    UnknownBonusRacer* field_0x2c;
    int field_0x30;                            // current key, -1 when idle
    float field_0x34;                          // time since the start
    UnknownBonusCamera* field_0x38;
    D3DIMSoultreeObject* field_0x3c;           // "Base Bonus Frame"
    D3DIMSoultreeObject* field_0x40[10][5];    // digit models by digit and place
    int field_0x108[5];                        // shown digit per place, -1 when none
    D3DIMSoultreeObject* field_0x11c;          // "x"
    D3DIMSoultreeObject* field_0x120;          // decimal point
    D3DIMSoultreeObject* field_0x124[10][2];   // fraction digit models
    int field_0x174[2];                        // shown fraction digits
    float field_0x17c;                         // key duration
    float field_0x180;                         // angle
    float field_0x184;                         // angle change over the key
    Vector3 field_0x188;
    Vector3 field_0x194;                       // position
    Vector3 field_0x1a0;                       // position change
    Vector3 field_0x1ac;                       // scale
    Vector3 field_0x1b8;                       // scale change
    Vector3 field_0x1c4;                       // axis
    Vector3 field_0x1d0;                       // axis change
    float field_0x1dc;                         // camera +0x16c when last placed
    float field_0x1e0;
    float field_0x1e4;                         // twice the frame's half height
    float field_0x1e8;                         // size factor
    float field_0x1ec;
    int field_0x1f0;
};
