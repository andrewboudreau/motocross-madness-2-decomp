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

class BikeRace;
class KrustyBike;
class Camera;
class LightManager;
class RenderTarget;
class Terrain;
class Track;
struct RaceStatusNode;
struct TextureContext;
struct ControlEvent;
struct InputButtonState;

// 0x00460c00 (cdecl): 1/sqrt of `value` (table driven).
float FastInvSqrt(float value);

// 0x4c bytes (NationalRace.cpp 0x004aa8cd): five "pointer.slt" models.
class RunwayLights : public GameObject {
public:
    explicit RunwayLights(int flags);          // 0x0048a5b0
    virtual int Tick(float frameTime);                 // 0x0048a760

    // 0x0048a600 (ret 0x10): slot 8, then loads the five lights (line 0x40).
    RunwayLights* SetupLights(void* context, const LightManager* lights, const TextureContext* textures, Terrain* terrain);
    void SetRacer(KrustyBike* racer);      // 0x0048ad50: sets +0x30

    float blinkTimer;                          // blink timer
    KrustyBike* trackedRacer;                 // +0x30 the lights stand on its track (SetRacer; NationalRace passes its race's localRacer)
    Terrain* groundTerrain;                 // +0x34 grounds the lights (SetupLights)
    ArcadeObject* lightModels[5];
};

class KrustyBikeCamera;

// 0xfc bytes (the allocation at 0x004e06cd).
class VisualCue : public ArcadeObject {
public:
    explicit VisualCue(int flags);             // 0x0048ad60
    virtual int Tick(float frameTime);                 // 0x0048b100
    virtual int Render3D();        // 0x0048bc10
    virtual int OnControlPress(ControlEvent* event, InputButtonState* entry);       // 0x0048afa0

    // 0x0048adf0 (ret 0x34): loads "viscue.slt"; returns this, or 0.
    VisualCue* LoadVisualCue(void* context, const LightManager* lights, const TextureContext* textures, Terrain* terrain,
                             Vector3 position, BikeRace* view, int pixels,
                             Camera* camera, float size, float screenXFraction,
                             float screenYFraction);
    // 0x0048af20 (ret 4): collects the race's racers (BikeRace::NextBike).
    void CollectRacers(BikeRace* view);
    int NextCueIndex();               // 0x0048bc30: the next cue index when it changed, else -1
    int CanCycleCues();               // 0x0048bc80: a cue is active and more than one racer exists

    // The cue's screen fractions (0.5f, 0.1f from its callers).
    float ScreenXFraction() { return screenX; }
    float ScreenYFraction() { return screenY; }
    // The placement block slot 10 repeats in each mode.
    void PlaceCue();

    BikeRace* cueView;                        // +0xa8 the race the cue follows (LoadVisualCue, CollectRacers)
    Terrain* groundTerrain;            // +0xAC  LoadVisualCue's terrain argument; slot 10 drops the cue target onto it with QueryGround
    float builtFieldOfView;                          // field of view the offsets were built for
    float screenOffsetX;                          // screen offset along the camera's x axis
    float screenOffsetY;                          // screen offset along the camera's y axis
    int cycleCues;                             // +0xbc toggled by control 0x14 (racing, chat closed, two or more cue racers); keeps the cue shown off the own racer's camera and lets event type 0 cycle racers (CanCycleCues)
    int currentRacer;                            // current racer
    int reportedRacer;                            // last reported racer
    int skippedRacer;                            // racer to skip
    KrustyBike* cueRacers[11];
    int cueRacerCount;                            // racers in +0xcc
};

// The digit models "five.slt" .. "one.slt".
class NumberObjectManager : public GameObject {
public:
    explicit NumberObjectManager(int flags);   // 0x0048bca0
    virtual int Tick(float frameTime);                 // 0x0048bfc0
    virtual int Render3D();        // 0x0048c290

    // 0x0048bd00 (ret 0x18): loads the five digits (lines 0x23d, 0x240).
    NumberObjectManager* SetupDigits(void* context, const TextureContext* textures, BikeRace* racer, int pixels,
                                     Camera* camera, float size);

    BikeRace* countdownRacer;                 // +0x2c whose count-down the digits show
    int drawEnabled;                          // +0x30 slot 14 draws only when set (1 from the constructor and slot 10)
    Camera* digitCamera;         // +0x34 the digits show only through this camera
    ArcadeObject* digitModels[5];             // +0x38 "one.slt" .. "five.slt"
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
    int LoadDigitModels(void* context, const LightManager* lights, const TextureContext* textures);
    // 0x0048c8a0 (ret 0xc): creates the frame node (line 0x326).
    int CreateFrameNode(void* context, const LightManager* lights, const TextureContext* textures);
    // 0x0048ca60 (ret 0x14): slot 8, then both loaders; returns this, or 0.
    BonusObjectManager* SetupCounter(void* context, const LightManager* lights, const TextureContext* textures, BikeRace* racer,
                                     Camera* camera);
    void NextKey();              // 0x0048cad0: steps to the next key
    void ShowValue(float value, float fraction); // 0x0048d1e0 (ret 8): shows a value
    void HideDigits();              // 0x0048d540: hides the shown digits

    BikeRace* raceView;                       // +0x2c the race (BikeRace passes this)
    int currentKey;                            // current key, -1 when idle
    float elapsed;                          // time since the start
    Camera* camera;               // +0x38 the race camera, then the current camera
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
