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

#include "TrackGameGlobal.h"
#include "ArcadeObject.h"
#include "D3DIMSoulTree.h"
#include "GameObject.h"
#include "MatrixUtil.h"

// 0x00460b50 (cdecl): table-driven square root (FollowCamera.h declares the
// same function).
float FastSqrt(float value);

struct ControlEvent;
struct InputButtonState;

// A racer as VisualCue sees it (the objects 0x004204e0 returns; RaceView.h's
// EventKrustyBikeView): a vfptr at +0, GameObject reached through the vbptr at
// +4. Only the +0x25 bit 0 test is used.
struct VisualCueKrustyBikeView : virtual public GameObject {
    virtual void UnknownVirtualSlot0();
    int IsCueRacerEnabled() { return isEnabled; }

    unsigned char field_0x08[0x0c - 0x08];
    Vector3 position;                         // +0x0c
};

// The camera NumberObjectManager places its digits in front of (its +0x34,
// also the ArcadeObject view whose +0x198 scales the models).
struct NumberCameraView {
    unsigned char field_0x000[0x170];
    Vector3 eyePosition;                      // +0x170
    Vector3 viewDirection;                    // +0x17c forward
    Vector3 upDirection;                      // +0x188 up
    unsigned char roll[0x198 - 0x194];  // +0x194 Camera roll (bytes kept)
    float focalLength;                        // +0x198
};

struct NumberKrustyBikeView {
    unsigned char field_0x000[0x76c];
    float offTrackTime;                       // +0x76c RaceView.h's racer offTrackTime
};

// NumberObjectManager's +0x2c. Its offsets line up with BikeRace.h's race
// view: +0x38 localRacer, +0x6c the 30SecMan entry, +0x15c startCountdown.
struct NumberBikeRaceView {
    unsigned char field_0x000[0x38];
    NumberKrustyBikeView* localRacer;      // +0x38
    unsigned char field_0x03c[0x6c - 0x3c];
    int thirtySecondMan;                      // +0x6c no digits count down while it is present
    unsigned char field_0x070[0x15c - 0x70];
    float countdown;                          // +0x15c count-down value
};

// 0x00460c00 (cdecl): 1/sqrt of `value` (table driven).
float FastInvSqrt(float value);

// A track position (RaceView.h's TrackPos: node, segment, t).
struct RunwayTrackPos {
    void* node;  // +0x0  RaceView.h/Track.h TrackPos: node
    void* segment;  // +0x4  Track.h TrackPos: segment
    float t;  // +0x8  Track.h TrackPos: t, 0..1 along the segment
};

// The track a racer's view holds at +0x48 (Track.h's Track).
struct RunwayTrackView {
    // 0x00518080: the world point of `pos`.
    int PointAt(RunwayTrackPos pos, Vector3* out);
};

struct RunwayBikeRaceView {
    unsigned char field_0x00[0x48];
    RunwayTrackView* raceTrack;            // +0x48 BikeRace's raceTrack
};

// The racer's race status node (RaceView.h's RaceStatusNode).
struct RunwayRaceStatusNodeView {
    unsigned char field_0x00[0x38];
    RunwayTrackPos placedPos;          // +0x38 (RaceView.h)
    RunwayTrackPos progressPos;        // +0x44 (RaceView.h)
};

// The racer the lights point for (NationalRace.cpp passes its view's
// +0x38; RaceView.h's EventKrustyBikeView).
struct RunwayKrustyBikeView {
    unsigned char field_0x000[0x10c];
    Vector3 trackPoint;                       // +0x10c
    Vector3 trackDirection;                   // +0x118 track direction there
    unsigned char field_0x124[0x740 - 0x124];
    RunwayBikeRaceView* bikeView;              // +0x740 (RaceView.h)
    RunwayRaceStatusNodeView* statusNode;       // +0x744 race status node (RaceView.h)
    unsigned char field_0x748[0x78c - 0x748];
    int onTrack;                              // +0x78c RaceView.h's racer onTrack
    unsigned char field_0x790[0x7a4 - 0x790];
    char finished;                            // +0x7a4
};

// The terrain (NationalRace.cpp's +0x40): 0x00507c10 drops `point` onto it.
struct RunwayTerrainView {
    void QueryGround(Vector3* point, Vector3* normal, int flatShaded, int surface);
};

// 0x4c bytes (NationalRace.cpp 0x004aa8cd): five "pointer.slt" models.
class RunwayLights : public GameObject {
public:
    explicit RunwayLights(int flags);          // 0x0048a5b0
    virtual int Tick(float frameTime);                 // 0x0048a760

    // 0x0048a600 (ret 0x10): slot 8, then loads the five lights (line 0x40).
    RunwayLights* SetupLights(void* context, int lights, int options, RunwayTerrainView* terrain);
    void SetRacer(RunwayKrustyBikeView* racer); // 0x0048ad50: sets +0x30

    float blinkTimer;                          // blink timer
    RunwayKrustyBikeView* trackedRacer;          // +0x30 the lights stand on its track (SetRacer)
    RunwayTerrainView* groundTerrain;       // +0x34 grounds the lights (SetupLights)
    ArcadeObject* lightModels[5];
};

// The object at a view's +0x50 (KrustyBikeCamera; +0x3b0 is the racer it
// follows).
struct VisualCueKrustyBikeCameraView {
    unsigned char field_0x000[0x3b0];
    RunwayKrustyBikeView* followedBike;         // +0x3b0 (RaceSound.h)
};

// The view VisualCue follows (its +0xa8; RaceView.h's EventBikeRaceView).
struct VisualCueBikeRaceView {
    // 0x004204e0: the next racer after `*iterator` (advancing it), or 0.
    VisualCueKrustyBikeView* NextBike(int* iterator);

    unsigned char field_0x000[0x38];
    RunwayKrustyBikeView* ownRacer;             // +0x38 its own racer
    unsigned char field_0x03c[0x48 - 0x3c];
    RunwayTrackView* raceTrack;            // +0x48 BikeRace's raceTrack
    unsigned char raceTerrain[0x50 - 0x4c];  // +0x4c BikeRace raceTerrain (bytes kept)
    VisualCueKrustyBikeCameraView* raceCamera;       // +0x50 BikeRace's raceCamera
    unsigned char field_0x054[0x18a - 0x54];
    bool racing;                              // +0x18a
    unsigned char field_0x18b[0x190 - 0x18b];
    bool chatOpen;                            // +0x190 the chat input line is open (BikeRace)
};

// 0xfc bytes (the allocation at 0x004e06cd).
class VisualCue : public ArcadeObject {
public:
    explicit VisualCue(int flags);             // 0x0048ad60
    virtual int Tick(float frameTime);                 // 0x0048b100
    virtual int Render3D();        // 0x0048bc10
    virtual int OnControlPress(ControlEvent* event, InputButtonState* entry);       // 0x0048afa0

    // 0x0048adf0 (ret 0x34): loads "viscue.slt"; returns this, or 0.
    VisualCue* LoadVisualCue(void* context, int lights, int options, RunwayTerrainView* terrain,
                             Vector3 position, VisualCueBikeRaceView* view, int pixels,
                             ArcadeCameraView* camera, float size, float screenXFraction,
                             float screenYFraction);
    // 0x0048af20 (ret 4): collects the view's racers.
    void CollectRacers(VisualCueBikeRaceView* view);
    int NextCueIndex();               // 0x0048bc30: the next cue index when it changed, else -1
    int CanCycleCues();               // 0x0048bc80: a cue is active and more than one racer exists

    // The cue's screen fractions (0.5f, 0.1f from its callers). A plain
    // `return screenX;` emits the same code but renumbers VC6's $L/$T labels.
    float ScreenXFraction() { return *(float*)&screenX; }
    float ScreenYFraction() { return *(float*)&screenY; }
    // The placement block slot 10 repeats in each mode.
    void PlaceCue();

    VisualCueBikeRaceView* cueView;
    RunwayTerrainView* groundTerrain;  // +0xAC  LoadVisualCue's terrain argument; slot 10 drops the cue target onto it with QueryGround
    float builtFieldOfView;                          // field of view the offsets were built for
    float screenOffsetX;                          // screen offset along the camera's x axis
    float screenOffsetY;                          // screen offset along the camera's y axis
    int cycleCues;                             // +0xbc toggled by control 0x14 (racing, chat closed, two or more cue racers); keeps the cue shown off the own racer's camera and lets event type 0 cycle racers (CanCycleCues)
    int currentRacer;                            // current racer
    int reportedRacer;                            // last reported racer
    int skippedRacer;                            // racer to skip
    VisualCueKrustyBikeView* cueRacers[11];
    int cueRacerCount;                            // racers in +0xcc
};

// The digit models "five.slt" .. "one.slt".
class NumberObjectManager : public GameObject {
public:
    explicit NumberObjectManager(int flags);   // 0x0048bca0
    virtual int Tick(float frameTime);                 // 0x0048bfc0
    virtual int Render3D();        // 0x0048c290

    // 0x0048bd00 (ret 0x18): loads the five digits (lines 0x23d, 0x240).
    NumberObjectManager* SetupDigits(void* context, int options, NumberBikeRaceView* racer, int pixels,
                                     NumberCameraView* camera, float size);

    NumberBikeRaceView* countdownRacer;       // +0x2c whose count-down the digits show
    int drawEnabled;                          // +0x30 slot 14 draws only when set (1 from the constructor and slot 10)
    NumberCameraView* digitCamera;         // +0x34 the digits show only through this camera
    ArcadeObject* digitModels[5];             // +0x38 "one.slt" .. "five.slt"
};

// The racer a BonusObjectManager follows (its +0x2c).
struct BonusSoultreeObjectView {
    // 0x004fc970: writes the part's translation into *translation.
    void GetPosition(Vector3* translation);
};

struct BonusBikeWheelView {
    unsigned char field_0x000[0x230];
    Vector3 rollDirection;                    // +0x230  Bike.h BikeWheel rollDirection (this is the localRacer's rearWheel, Bike +0x5f4)
};

struct BonusKrustyBikeView {
    unsigned char field_0x000[0x3bc];
    BonusSoultreeObjectView* modelNode;         // +0x3bc
    unsigned char field_0x3c0[0x444 - 0x3c0];
    int crashState;                           // +0x444
    unsigned char field_0x448[0x5f4 - 0x448];
    BonusBikeWheelView* rearWheel;        // +0x5f4
};

struct BonusBikeRaceView {
    unsigned char field_0x000[0x38];
    BonusKrustyBikeView* localRacer;       // +0x38 BikeRace.h's localRacer
};

// The camera a BonusObjectManager faces (its +0x38).
struct BonusCameraView {
    unsigned char field_0x000[0x16c];
    float fieldOfView;                        // +0x16c
    Vector3 eyePosition;                      // +0x170
    unsigned char field_0x17c[0x198 - 0x17c];
    float focalLength;                        // +0x198
};

// The game global at 0x0056e26c (TrackGame.h's g_TrackGame) as
// this file reads it: +0x10 is the render target, whose +0x08 is the
// current camera; +0x2d74 the camera mode; +0x3430 blocks the UI.
struct BonusRenderTargetView {
    unsigned char field_0x00[0x08];
    BonusCameraView* currentCamera;        // +0x08
};

struct CueGateKrustyBikeView {
    unsigned char field_0x000[0x7b8];
    int gateIndex;                            // +0x7b8
};

struct CueKrustyBikeCameraView {
    unsigned char field_0x000[0x3b4];
    CueGateKrustyBikeView* followedRacer;     // +0x3b4 (RaceSound.h)
};

struct CueBikeRaceView {
    unsigned char field_0x00[0x50];
    CueKrustyBikeCameraView* raceCamera;          // +0x50 BikeRace's raceCamera
};

// One 24-byte gate record of the course at the global's +0x560.
struct CueCourseGate {
    Vector3 position;  // +0x0  gate point VisualCue slot 10 (camera modes 1/5) turns the cue toward
    unsigned char field_0x0c[0x18 - 0x0c];
};

struct CueCourseEventView {
    unsigned char field_0x00[0x34];
    CueBikeRaceView* raceView;      // +0x34
    unsigned char field_0x38[0xd8 - 0x38];
    CueCourseGate gates[1];  // +0xD8  24-byte gate records indexed by the followed racer's gateIndex
};

struct CueTagCameraView {
    unsigned char field_0x000[0x25];
    unsigned char statusFlags;                // +0x25  GameObject statusFlags; bit 0 is tested here
    unsigned char field_0x026[0x3b4 - 0x26];
    VisualCueKrustyBikeView* followedRacer;     // +0x3b4 (RaceSound.h)
};

struct CueTagBallView {
    unsigned char field_0x000[0x21c];
    BonusSoultreeObjectView sceneObject;        // +0x21C  the SoultreeObject subobject; SoultreePhysicsBaseObject's own part ends at 0x21c, and GetPosition is SoultreeObject's 0x004fc970. Tier 2
};

struct CueTagEventView {
    unsigned char field_0x00[0x3c];
    CueTagCameraView* raceCamera;        // +0x3c
    unsigned char field_0x40[0xa8 - 0x40];
    VisualCueKrustyBikeView* followedRacer;     // +0xa8
    unsigned char field_0xac[0xdc - 0xac];
    CueTagBallView* ball;                  // +0xdc TrackGameViewOwner::ball (the tag ball)
};

struct CueTrackGameView {
    unsigned char field_0x0000[0x10];
    BonusRenderTargetView* renderTarget;   // +0x10
    unsigned char field_0x0014[0x560 - 0x14];
    CueCourseEventView* bajaEnduroEvent;     // +0x560
    unsigned char field_0x0564[0x568 - 0x564];
    CueTagEventView* tagEvent;            // +0x568
    unsigned char field_0x056c[0x2d74 - 0x56c];
    int cameraMode;                           // +0x2d74 camera mode 0..5
    unsigned char field_0x2d78[0x2eb4 - 0x2d78];
    int tagBall;                              // +0x2eb4 race settings' tagBall (TrackGame.h)
    unsigned char field_0x2eb8[0x3430 - 0x2eb8];
    int uiInteractionBlocked;                 // +0x3430 (TrackGame.h)
};

// One 0x30-byte key of the bonus animation table at 0x0056c820 (seven
// keys); plain floats, as the table is static data.
struct UnknownBonusKey {
    int frame;                                // +0x00
    float time;                               // +0x04
    float scale[3];  // +0x8  NextKey interpolates keyScale from it (table: uniform 2.09, 1.99, ...)
    float position[3];  // +0x14  NextKey interpolates keyPosition from it
    float rotationAxis[3];  // +0x20  NextKey interpolates keyAxis from it; paired with rotationAngle
    float rotationAngle;                      // +0x2c
};

// "Base Bonus Frame" and its digit models.
class BonusObjectManager : public GameObject {
public:
    explicit BonusObjectManager(int flags);    // 0x0048c2b0
    virtual ~BonusObjectManager();             // 0x0048c320 (deleting wrapper 0x0048c300)
    virtual int Tick(float frameTime);                 // 0x0048ccc0
    virtual int Render3D();        // 0x0048d620

    // 0x0048c330 (ret 0xc): loads the digit, "x" and decimal point models.
    int LoadDigitModels(void* context, int lights, int options);
    // 0x0048c8a0 (ret 0xc): creates the frame node (line 0x326).
    int CreateFrameNode(void* context, int lights, int options);
    // 0x0048ca60 (ret 0x14): slot 8, then both loaders; returns this, or 0.
    BonusObjectManager* SetupCounter(void* context, int lights, int options, BonusBikeRaceView* racer,
                                     BonusCameraView* camera);
    void NextKey();              // 0x0048cad0: steps to the next key
    void ShowValue(float value, float fraction); // 0x0048d1e0 (ret 8): shows a value
    void HideDigits();              // 0x0048d540: hides the shown digits

    BonusBikeRaceView* raceView;              // +0x2c the race (BikeRace passes this)
    int currentKey;                            // current key, -1 when idle
    float elapsed;                          // time since the start
    BonusCameraView* camera;               // +0x38 the race camera, then the current camera
    D3DIMSoultreeObject* baseFrame;           // "Base Bonus Frame"
    D3DIMSoultreeObject* digitModels[10][5];    // digit models by digit and place
    int shownDigits[5];                        // shown digit per place, -1 when none
    D3DIMSoultreeObject* timesModel;          // "x"
    D3DIMSoultreeObject* decimalPointModel;          // decimal point
    D3DIMSoultreeObject* fractionDigitModels[10][2];   // fraction digit models
    int shownFractionDigits[2];                        // shown fraction digits
    float keyDuration;                         // key duration
    float keyAngle;                         // angle
    float keyAngleChange;                         // angle change over the key
    Vector3 eyeToFrame;  // +0x188  slot 10: frame position minus the camera eyePosition; its xz direction orients the frame
    Vector3 keyPosition;                       // position
    Vector3 keyPositionChange;                       // position change
    Vector3 keyScale;                       // scale
    Vector3 keyScaleChange;                       // scale change
    Vector3 keyAxis;                       // axis
    Vector3 keyAxisChange;                       // axis change
    float placedFieldOfView;                   // +0x1dc the camera's fieldOfView (+0x16c) when last placed
    float targetScreenWidth;  // +0x1E0  ctor 60.0f; sizeFactor = cameraDistance * it / (focalLength * frameWidth)
    float frameWidth;  // +0x1E4  CreateFrameNode: twice the base frame's local half extent x
    float sizeFactor;                         // size factor
    float cameraDistance;  // +0x1EC  slot 10: Length(position - camera eyePosition) when the frame is re-sized
    int isResizePending;  // +0x1F0  slot 10 recomputes sizeFactor when set (then clears it); ctor 0
};
