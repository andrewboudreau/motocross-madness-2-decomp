#pragma once

// Complete canonical BikeRace declaration; helper classes remain in BikeRace.h.
#include "../krusty2/core/GraphicsTest.h"
#include "BikeRaceNode.h"

class LightManager;
class KrustyBikeCamera;
class Scene;
class ObjectPicker;
class ChatOverlay;
class KrustyVCR;
class UnknownVcrFile;
class UnknownBikeRaceRacer;
class UnknownBikeRaceProjector;
class UnknownBikeRaceShadow;
class UnknownBikeRaceCharacter;
struct UnknownTextureFormatChoice;
struct UnknownBikeRaceCameraView;
struct UnknownBikeRaceNodeOwner;
struct UnknownBikeRaceNodeSource;
struct UnknownBikeRaceViews;
struct UnknownBikeRaceView6c;
struct UnknownEventRacerPart;

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
    void UnknownFunctionFreeNodes();

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
    int RecordFrame(int kind, void* data, int flag, int recorderId, int* keep, int time);
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
    ChatOverlay* chatOverlay;   // +0x19c
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
