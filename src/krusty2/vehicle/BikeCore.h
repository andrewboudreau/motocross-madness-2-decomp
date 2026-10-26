#pragma once

// Canonical Bike declaration, shared without implementation collaborator views.
#include "VehicleCore.h"
#include "BikeLoaderRecord.h"

struct BikeA5C4;
struct BikeA5F8;
struct BikeA60C;
struct BikeA640;
struct BikeA644;
struct BikeDirtChunkEmitter;
struct BikeDustEmitter;
struct BikeSteamEmitter;
struct BikeWheel;
class Wrecker;

class Bike : public Vehicle {
public:
    explicit Bike(int flags);       // 0x00407700, ret 8 = flags + hidden most-derived flag
    virtual ~Bike();              // vbase vtable 0x0055085c slot 0 -> 0x0040ca40

    // overrides of Vehicle slots (signatures: SoultreePhysicsBaseObject.h, Vehicle.h)
    virtual void UnknownVirtualSlot1(float extraWeight);
    virtual int UnknownVirtualSlot5(int arg);
    virtual void UnknownVirtualSlot7(const Vec3* force);
    virtual void UnknownVirtualSlot8();
    virtual Vec3 UnknownVirtualSlot16(const Vec3* localTorque);
    virtual void UnknownVirtualSlot29(int lastStep);
    virtual float UnknownVirtualSlot32();
    virtual int UnknownVirtualSlot33(const Vec3* newPosition, const Vec3* newForward, const Vec3* newUp,
                                     const Vec3* settleUp, int keepPlacement, float extraWeight);
    virtual void UnknownVirtualSlot38(int notify, int eventKind, void* event);
    virtual int UnknownVirtualSlot39(float dt);
    virtual void UnknownVirtualSlot41();
    virtual int UnknownVirtualSlot42();
    virtual void UnknownVirtualSlot44();
    virtual Vec3* UnknownVirtualSlot46(Vec3* out, float t);
    virtual float UnknownVirtualSlot47(float arg);
    virtual void UnknownVirtualSlot49(float arg);
    virtual float UnknownVirtualSlot53();
    virtual Vec3* UnknownVirtualSlot54(Vec3* out);
    virtual Vec3* UnknownVirtualSlot55(Vec3* out, Vec3* pos);
    virtual void UnknownVirtualSlot56(Vec3* forward, int updateLeanCos, Vec3* up);
    virtual float UnknownVirtualSlot57();
    virtual float UnknownVirtualSlot59();
    virtual float UnknownVirtualSlot61(float arg);
    virtual int UnknownVirtualSlot62(float arg);
    virtual void UnknownVirtualSlot65(float arg);
    virtual int UnknownVirtualSlot66();
    virtual void UnknownVirtualSlot67();
    virtual void UnknownVirtualSlot71(int nowAirborne);
    virtual void UnknownVirtualSlot72(Vec3* out, VehicleWheel* wheel);
    virtual float UnknownVirtualSlot73(const Vec3* a, const Vec3* b);
    virtual float UnknownVirtualSlot75();
    virtual Vec3 UnknownVirtualSlot76(const Vec3* arm, const Vec3* force);
    virtual int UnknownVirtualSlot89(float frame);
    virtual void UnknownVirtualSlot90(int* steps, float frame);
    virtual void UnknownVirtualSlot91();
    virtual void UnknownVirtualSlot92(const Vec3* a, const Vec3* b);
    virtual void UnknownVirtualSlot96();

    // slots introduced by Bike
    virtual void UnknownVirtualSlot97();
    virtual int UnknownVirtualSlot98();
    virtual int UnknownVirtualSlot99(float dt, float noseOverLimit, int impact, float impulse);
    virtual int UnknownVirtualSlot100(float dt, int impact, float impulse);
    // Primary vtable 0x00550900 slot 101 is CRT _purecall (0x00534cfe).
    // KrustyBike supplies the concrete override at 0x00496d10.
    virtual void UnknownVirtualSlot101() = 0;
    virtual void UnknownVirtualSlot102(float arg);

    // 0x004079c0 (6745 bytes, `ret 0xa0` = 40 argument dwords): the bike loader.  Runs
    // Vehicle::LoadVehicle, builds the rider character, the handlebar part, the two wheels
    // with their shocks, the three contact points, the particle emitters and the pose handles.
    // Argument names are tier 3 except where a callee's parameter fixes them; a19 is unused.
    GameObject* LoadBike(int context, const char* engineName, const char* modelPath, const char* riderModel,
                         const SoultreeLoadDesc* desc, const BikeA5C8* info, Vec3 position,
                         Vec3 forward, Vec3 up, void* terrainProbe, VehicleInputMap* map,
                         const char* name, int a19, int defaultEngine, int* torqueTable,
                         int rpmLow, int rpmHigh, int rpmStep, float frontSpring, float frontDamper,
                         float rearSpring, float rearDamper, float frontRatio, float rearRatio,
                         int gasGyroOption, int brakeGyroOption, void* linkedObject, void* device, VehicleAxis* steer,
                         VehicleAxis* lean, VehicleAxis* throttle, SoultreeSlot1f0* track,
                         float weight, int createCollision);

    // 0x0040a520 (2272 bytes): integrates a steering torque from the angular velocity, the
    // front wheel's contact and the two +0x5f8/+0x5fc contacts, turns it by the steer state
    // and returns the normalised projection (0 when nothing drives it).
    float Method_0x0040a520();

    // Inherited members whose canonical types are still generic are viewed through
    // casts at the use site (MIGRATION.md rule 6): field_0x12c as BikeElem**, field_0x1f4
    // as BikeA1F4*.  Inline accessor
    // functions were tried and cost the exact match of slot 8 (0x00406750): VC6 then
    // loads field_0x42c before the float arguments instead of after them.

    int gasGyro;  // +0x5BC  0 from the constructor, LoadBike's gasGyroOption: TrackGameMode gasGyro (key 0x22, reset 0)
    int brakeGyro;  // +0x5C0  1 from the constructor, LoadBike's brakeGyroOption: TrackGameMode brakeGyro (key 0x30, reset 1)
    BikeA5C4* riderCharacter;  // +0x5c4 slot 8/89 (0x406ae0): calls Method_0x004a8c50(poseA, poseB, poseParam, w) on it and riderCharacter->c_0x1a0 scene node; slot 97 passes it with "rider.col" to the 0x604 object
    BikeA5C8 field_0x5c8;         // +0x5c8 copy of the loader's info block (when given)
    char riderName[16];           // +0x5e0 ctor strcpy from global 0x00577738 (empty string in the image); tier 3 name
    BikeWheel* frontWheel;       // +0x5f0 front wheel (largest config z, see 0x4079c0 loop at 0x408705); tier 3
    BikeWheel* rearWheel;       // +0x5f4 rear wheel (smallest config z, 0x408728); tier 3
    BikeA5F8* leftBarContact;      // +0x5f8 cleared in the ctor (0x00407700); LoadBike adds it at (-1.305, 0.225, -0.228) on the "Handlebars" node; Method_0x0040a520 reads it
    BikeA5F8* rightBarContact;     // +0x5fc cleared in the ctor; the mirror point (+1.305, 0.225, -0.228) on the "Handlebars" node
    int tailContact;                          // +0x600  the loader's contact point at the tail (0, 3.345, -3.206) on modelNode, beside the handlebar contacts
    Wrecker* wrecker;  // +0x604  Wrecker (SetupWrecker with rider.col, StartAnimatedWreck); RaceView/BikeRace +0x604 wrecker
    Vehicle* linkedVehicle;  // +0x608 cleared in ctor and reset (lines near linkedVehicle = 0), set reciprocally in the slot 38 pairing code: linkedVehicle = other; ((Bike*)other)->linkedVehicle = this
    BikeA60C* handlebars;          // +0x60c BikeA60C("Handlebars", model, steerState); the steer axis runs from the front wheel to its node; cleared in the ctor; ~Bike calls 0x00464e90 on it, then operator delete
    Vec3 steerTorqueGains;  // +0x610  per-axis gains of the steering torque integrator (Method_0x0040a520)
    Vec3 steerTorque;  // +0x61C  integrated steering torque (Method_0x0040a520); decays by 0.99 (or -0.02/-0.1 at the limit) per step
    float driveSlideAngle;                    // +0x628  slot 61: rear-wheel drive term while all wheels touch and the bike leans, decayed to 0 over 0.75 s; returned to slot 49, which hands it to slot 46 (provisional)
    float driveSlideAngleStep;                // +0x62C  slot 61: per-step decay of driveSlideAngle (-1.333 * fixedStepTime * angle)
    int driveSlideStepsLeft;                  // +0x630  slot 61: steps left in the 0.75 s decay ((int)(0.75 / fixedStepTime - 0.5))
    int field_0x634;
    int field_0x638;
    float crossupScale;                       // +0x63C  KrustyBike slot 96: CrossupSlider (TrackGame+0xfe4) pct * 0.4 + 0.8, 1.0 for AI bikes
    BikeA640* steerAxis;  // +0x640 slot 41 region: steer = (steerAxis->l_0x0 - 0.5f) * 4.0f; slot 89 maps (0.75 - l_0x0)*3.99 to a pose index
    BikeA644* poseSmoother;  // +0x644 slot 1 and slot 89: object with value/rate/ratio/limits that is advanced toward field_0x504 each frame with rate limiting (see BikeA644)
    float leanPoseMin;  // +0x648 slot 89: lean > leanPoseMin selects pose; poseLeanBlend = (lean - leanPoseMin) / (leanPoseMax - leanPoseMin)
    float leanPoseMax;  // +0x64c slot 89: upper lean bound; divisor (leanPoseMax - leanPoseMin) of the pose lean blend
    int poseIndex;  // +0x650 slot 89/61: current pose index (set to 10/11+idx, used to index riderPoseHandles[] and bikePoseHandles[], clamped to 13)
    float poseBlend;  // +0x654 slot 89: fractional part of the pose parameter ((0.75-l)*3.99 - idx); slot 61 weight w = 1 - poseBlend
    int poseState;  // +0x658 slot 89/61: paired with poseIndex (10 = lean pose); compared against 10 in slot 61 and set to idx*2+4
    float poseParam;  // +0x65c slot 61: passed as time/weight arg to Method_0x004a8c50 and Method_0x004a8bf0; slot 89 state machine writes it from the smoothed value (x = t * -0.25)
    float poseLeanBlend;  // +0x660 slot 89: (lean - min)/(max - min) pose lean blend; slot 61: (speed*0.0310559+1)*0.5
    float backUpPhase;                        // +0x664  KrustyBike slot 102 advances it mod 1 as the BackUpBottom pose time (riderPoseHandles[17]); slot 86 ramps the back-up push off it
    float backUpPhaseRate;                    // +0x668  KrustyBike slot 102: per-frame advance of backUpPhase, max(|forwardSpeed|, 1) * dt * 0.55371
    int riderPoseHandles[18];  // +0x66c slot 61: riderPoseHandles[idx], riderPoseHandles[next] passed to riderCharacter->Method_0x004a8c50 (rider-side pose ids, 18 entries)
    int bikePoseHandles[18];  // +0x6b4 slot 61: bikePoseHandles[idx], [next] passed to D3DIMSoultreeCharacter::Method_0x004a8c50 (bike-side pose ids, 18 entries)
    int crossoverPose;                        // +0x6FC  slot 89 sets it while airborne with both wheels clear and picks poses 11..13 (CrosOv1..3); slot 41 clears it when slot 89 leaves that branch (tier 2)
    int frontWheelLifted;                     // +0x700  = wheelsInContact > 0 and frontWheel->penetration < -2.5 (front wheel clear while another touches); slot 62 returns 2 on it
    float poseSmoothTime;                     // +0x704  0.55; the poseSmoother's smoothing time (constructor argument, reloaded when the crossover pose starts)
    float sideLieThreshold;  // +0x708 slot 38 tail: compared with |dot(kVec3YAxis, field_0xa0)|; if smaller 'bike is lying on its side' branch
    float field_0x70c;             // +0x70c loader: 0.85f
    float crashGraceTime;                     // +0x710  the loader sets crashTimerReload - 0.5; slot 39 clears wrecker +0xac while crashTimer is still above it (provisional)
    float wobbleTime;  // +0x714 slot 91 wobble: wobbleTime += field_0x13c each frame, cubed (t*t*t*0.3578) to cap amplitude, halved on sign flip, zeroed by slot 71
    float wobbleSign;  // +0x718 slot 91 wobble: sign of field_0x2c; compared with new sign to detect flip; set in slot 71 from sign of field_0x48
    float wobbleOffset;  // +0x71c slot 91 wobble: added into field_0xd8.z, decays *0.9 per frame, reloaded from field_0xd8.z on flip/slot 71
    int wobbleStage;  // +0x720 slot 91 wobble: cycles 1..2 on each sign flip, loop active only while 1..2; slot 71 sets to 1
    float leanSpeedThreshold;                 // +0x724  32.0; slot 57 returns savedRoll as the lean while not crashed and prevSpeed is below it (provisional)
    BikeDustEmitter* dustEmitter;  // +0x728  BikeDustEmitter built by the loader (Bike.cpp new line 0x70f), attached as type 1 on the rear wheel
    BikeDirtChunkEmitter* dirtChunkEmitter;  // +0x72C  BikeDirtChunkEmitter built by the loader (line 0x710), attached as type 2 on the rear wheel
    BikeSteamEmitter* steamEmitter;  // +0x730  BikeSteamEmitter built by the loader (line 0x711), attached as type 4 at (0, 2.5, -2.75); own data ends at 0x734, the compiler places the vtordisp there
};

typedef char bike_assert_sizeof[(sizeof(Bike) == 0x738 + 0x2c) ? 1 : -1];
