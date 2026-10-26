#pragma once

// Canonical Vehicle declaration, shared without implementation collaborator views.
#include <stddef.h>
#include "../soultree/SoultreePhysicsCharacterCore.h"
#define VEH_CHECK_OFFSET(cls, member, off) \
    typedef char veh_assert_##member##_##off[(offsetof(cls, member) == (off)) ? 1 : -1]

struct VehicleAxis;
struct VehicleInputMap;
struct VehicleSmoother;
struct VehicleSparkEmitter;
struct VehicleSpeedState;
struct VehicleTicker;
struct VehicleWheel;
struct VehicleWheelAux;
class SteeringControl;
typedef SteeringControl VehicleSteerState;

class Vehicle : public SoultreePhysicsCharacter {
public:
    Vehicle();
    explicit Vehicle(int flags);                 // 0x005257a0 (ret 8: flags + hidden most-derived flag)
    virtual ~Vehicle();                          // vbase deleting dtor 0x0052b630
    virtual int Tick(float dt);                     // 0x0052a830 via vtordisp thunk 0x0040cab0

    // ---- overrides of inherited slots (order as in the vtable) ----
    virtual void UnknownVirtualSlot0(float extraWeight);
    virtual void UnknownVirtualSlot1(float extraWeight);                               // 0x00525550
    virtual void UnknownVirtualSlot3(const Vec3* normal, const Vec3* relVel, const Vec3* arm,
                                     const Vec3* angScale, int eventKind, int scaleBits,
                                     float* impulseOut);                   // 0x005264c0
    virtual void UnknownVirtualSlot4(const Vec3* normal, Vec3* velA, const Vec3* worldAngVelA,
                                     const Vec3* armA, int eventKind, float invMassB, int nodeB,
                                     const Vec3* velB, const Vec3* worldAngVelB,
                                     const Vec3* armB, Vec3* invInertiaB, Vec3* angVelB,
                                     int velBOrNull, float* impulseOut, float scale);
    virtual void UnknownVirtualSlot7(const Vec3* force);
    // Slots 18..20 take the canonical 40-byte record; the bodies view it as
    // VehicleImpactEvent (same offsets: +0x04/+0x08/+0x0c sinks, +0x24 resetTrail).
    virtual void UnknownVirtualSlot18(SoultreeAttachment* arg);
    virtual void UnknownVirtualSlot19(SoultreeAttachment* arg);
    virtual void UnknownVirtualSlot20(SoultreeAttachment* arg);
    virtual void UnknownVirtualSlot21();
    virtual int  UnknownVirtualSlot23();
    virtual int  UnknownVirtualSlot24();
    virtual int  UnknownVirtualSlot25();
    virtual int  UnknownVirtualSlot28(int arg);
    virtual int  UnknownVirtualSlot33(const Vec3* newPosition, const Vec3* newForward, const Vec3* newUp,
                                      const Vec3* settleUp, int keepPlacement, float extraWeight);   // 0x00527430
    virtual void UnknownVirtualSlot34();
    virtual void UnknownVirtualSlot35(int a, int b);
    virtual void UnknownVirtualSlot36();
    virtual void UnknownVirtualSlot38(int notify, int eventKind, void* event);
    virtual int  UnknownVirtualSlot39(float dt);
    virtual int  UnknownVirtualSlot42();

    // ---- slots introduced by Vehicle (43..96) ----
    virtual void UnknownVirtualSlot43();
    virtual void UnknownVirtualSlot44();
    virtual float UnknownVirtualSlot45();
    virtual Vec3* UnknownVirtualSlot46(Vec3* out, float arg);
    // pure virtual: entry is _purecall (0x00534cfe).  Bike's override 0x0040cbb0 is
    // `ret 4` and returns a float (tier 1 via its byte match), hence float(float).
    virtual float UnknownVirtualSlot47(float arg) = 0;
    virtual void UnknownVirtualSlot48();
    // 0x0052a940 forwards its argument unchanged to slots 9 (float dt), 39, 64 and 65,
    // so it and they take the float time step (tier 2).
    virtual void UnknownVirtualSlot49(float dt);
    virtual void UnknownVirtualSlot50(int a, float b, int c);
    virtual int UnknownVirtualSlot51();
    virtual int UnknownVirtualSlot52();
    virtual float UnknownVirtualSlot53();
    virtual Vec3* UnknownVirtualSlot54(Vec3* out);
    virtual Vec3* UnknownVirtualSlot55(Vec3* out, Vec3* pos);
    virtual void UnknownVirtualSlot56(Vec3* a, int b, Vec3* c);
    virtual float UnknownVirtualSlot57();
    virtual void UnknownVirtualSlot58(Vec3* settleUp, int keepPlacement);
    virtual float UnknownVirtualSlot59();
    virtual void UnknownVirtualSlot60(float a, float b, int c);
    virtual float UnknownVirtualSlot61(float arg);
    virtual int UnknownVirtualSlot62(float arg);
    virtual void UnknownVirtualSlot63(float dt);
    // void: 0x00528e50 leaves eax as whatever the last store used (no return value is
    // materialised) and the only caller, slot 49, ignores eax; byte-exact as void (tier 2).
    virtual void UnknownVirtualSlot64(float dt);
    virtual void UnknownVirtualSlot65(float dt);  // entry is the shared empty stub 0x00464e80
    virtual int UnknownVirtualSlot66() = 0;  // pure virtual: entry is _purecall (0x00534cfe)
    virtual void UnknownVirtualSlot67();
    virtual int UnknownVirtualSlot68(int* out);
    virtual void UnknownVirtualSlot69();
    virtual int UnknownVirtualSlot70(float arg);
    virtual void UnknownVirtualSlot71(int arg);
    virtual void UnknownVirtualSlot72(Vec3* out, VehicleWheel* wheel);
    virtual float UnknownVirtualSlot73(const Vec3* a, const Vec3* b);  // only read (Bike 0x00406090)
    virtual float UnknownVirtualSlot74(Vec3* point, Vec3* dir, float c, float d);
    virtual float UnknownVirtualSlot75();
    // ret 0xc = hidden result pointer + two arguments; KrustyBike 0x00491ca0 byte-matches
    // as a by-value Vec3 return (tier 1 for the ABI shape, tier 2 for the Vec3 pointers).
    virtual Vec3 UnknownVirtualSlot76(const Vec3* a, const Vec3* b);
    virtual int UnknownVirtualSlot77(int arg);
    virtual int UnknownVirtualSlot78();
    virtual int UnknownVirtualSlot79();
    virtual int UnknownVirtualSlot80();
    virtual int UnknownVirtualSlot81();
    virtual int UnknownVirtualSlot82();
    virtual int UnknownVirtualSlot83(VehicleWheel* wheel);
    // Returns the input-map query (KrustyBike 0x00491630 uses the result; the Vehicle
    // body 0x00529250 leaves it in eax) (tier 2).
    virtual int UnknownVirtualSlot84(int a, int b);
    virtual void UnknownVirtualSlot85();
    virtual void UnknownVirtualSlot86();
    virtual void UnknownVirtualSlot87();  // entry is the shared empty stub 0x00464e90
    virtual int UnknownVirtualSlot88();
    virtual int UnknownVirtualSlot89(float arg);      // Bike 0x0040ae50 passes a float (tier 2)
    virtual void UnknownVirtualSlot90(int* a, float b);
    virtual void UnknownVirtualSlot91();  // entry is the shared empty stub 0x00464e90
    virtual void UnknownVirtualSlot92(const Vec3* a, const Vec3* b);
    virtual void UnknownVirtualSlot93();
    virtual void UnknownVirtualSlot94();  // entry is the shared empty stub 0x00464e90
    virtual void UnknownVirtualSlot95();
    virtual void UnknownVirtualSlot96();

    // 0x00526830 (next to slot 70): stores the crash answer in crashState and, when crashed,
    // clears the controls and records the heading; returns it (tier 3 name).
    int CheckCrash();
    // 0x00525e20 (`ret 0x94` = 37 argument dwords; not in any vtable: Bike's loader 0x004079c0
    // calls it directly).  Forwards the arguments named after slot 40's parameters (context is
    // its renderContext; a5 is Bike's info record) to SoultreePhysicsCharacter slot 40 with
    // (0.02f, 100, 0.001f, 0.1f, 3) and the track byte; the other arguments configure the
    // vehicle (names tier 3).
    GameObject* LoadVehicle(int context, const char* engineName, const char* modelPath,
                            const SoultreeLoadDesc* desc, int a5, Vec3 startPosition, Vec3 startForward,
                            Vec3 startUp, void* terrainProbe, VehicleInputMap* map, void* linkedObject,
                            float weight, float arg18, float steerScale, void* device, int wheelCap,
                            int extraContacts, int attachmentSlots, int earlyCap, int lateCap, int defaultEngine,
                            int* torqueTable, int rpmLow, int rpmHigh, int rpmStep, float gearArg,
                            VehicleAxis* steer, VehicleAxis* lean, VehicleAxis* throttle,
                            SoultreeSlot1f0* trackObject, int createCollision);
    int Method_00478FE0();      // 0x00478fe0, shared `xor eax,eax; ret` stub (direct call from CheckCrash)
    int AddEarlyTicker(VehicleTicker* ticker);                  // 0x00525cb0, append to earlyTickers
    int AddLateTicker(VehicleTicker* ticker);                   // 0x00525cf0, append to lateTickers
    int AddWheel(VehicleWheel* wheel, int a2, int a3, int a4, VehicleWheelAux* aux);   // 0x00525d30, appends a wheel and registers its contact point
    // 0x00528400: normalized average of the wheels' contact normals (+0xe4); returns out.
    Vec3* GetAverageGroundNormal(Vec3* out);
    int   UpdateWheelRampLevels();       // 0x00529280, ramps each wheel's rampLevel by its slot 83 answer
    void  PlaceWheels();                 // 0x00528eb0, retracts the shocks and probes each wheel against the ground
    void  UpdateWheelsInContact();       // 0x00529a20, updates in-contact wheels, resets the aux ramp of the others
    void  Method_00527A20(float* speed, Vec3* worldTorque, Vec3* force);   // 0x00527a20
    void  Method_005293E0(float* speed);                         // 0x005293e0
    void  Method_00529450(Vec3* force, Vec3* localTorque);           // 0x00529450
    void  AccumulateWheelContacts(Vec3* force, Vec3* localTorque, float torqueScale);  // 0x00529c20, folds in-contact wheel vectors into the accumulators
    // 0x00525c60: calls virtual slot 0 of every object in the two owned arrays (+0x554/+0x55c, +0x560/+0x568).
    void RunTickers();
    void ResetWheelContacts();           // 0x00525a90, resets every wheel's contact state and vectors

    // Inherited members whose canonical types are still separate classes are viewed
    // through casts at the use site (MIGRATION.md rule 6): field_0x12c as
    // VehicleContact**, field_0x128 as
    // VehicleContactSet*, field_0x1f0 as VehicleMaterialSet*.  Inline accessor functions
    // were tried; VC6 schedules their loads differently (slots 4, 19, 35 and 49 lost
    // bytes, Bike slot 8 lost its exact match), so the casts are written out.

    float forwardSpeed;  // +0x434  velocity along bodyForward (BikeAI dot; UpdateContactPatch out); KrustyBike compares it with 60 mph in ft/s
    float referenceSpeed;                     // +0x438  LoadVehicle sets 180; Bike slot 59 scales the steer limit by prevSpeed / referenceSpeed; the bike cameras use it in place of 180 (tier 2)
    float turnRate;  // +0x43c = turnAngle * field_0x140 (1/dt) in slot 72: velocity deflection angle per second; reset to 0 whenever no wheel drives it
    int movingForward;  // +0x440 slot 85 passes +dt when set and -dt when clear to every wheel; slot 74 flips the sign of its result on it; slot 1 resets it to 1
    int crashState;  // +0x444 nonzero disables input (slots 52/62/64/60 test ==0), switches solver vectors (+0xf0, 0.1 scale, 1,1,1 mask); Bike slot 99 writes it and runs the crash timer 0x454 while set
    int crashDirection;          // +0x448 Bike slot 99 (0x00409b30) stores small codes (1..5)
    float impactAccelLimit;                   // +0x44C  LoadVehicle sets 200; Bike slot 99 reports a crash when impulse * invMass > a * impactAccelLimit (tier 2)
    float speedGainLimit;  // +0x450 slot 70: true when (speed field_0xbc - previous speed field_0xb8) < arg * speedGainLimit
    // 0x454/0x458: Bike slot 39 (0x0040a1f0) decrements 0x454 by the frame time and
    // compares it with 0.0f/-2.0f, so both are floats (tier 2; slot 69 copies 0x458).
    float crashTimer;  // +0x454 Bike slot 39 decrements it by the frame time while crashed and compares with 0/-0.1/-2.0; slots 1/67 clear it
    float crashTimerReload;  // +0x458 slot 69 copies it into crashTimer
    float crashYaw;  // +0x45C  CheckCrash stores savedYaw here on a crash; VehicleCamera slot 39 returns it
    int crashReason;  // +0x460 Bike slot 99 stores reason codes (2,4,6,7,8,10,11,13) beside crashDirection
    int crashedUpsideDown;  // +0x464  CheckCrash stores savedUp.y < 0 on a crash; VehicleCamera slot 39 turns the camera by pi
    VehicleInputMap* inputMap;  // +0x468 object whose virtual slots 2/3 are queried with action ids 0x0e/0x1d/0x1f/0x2d/0xc8/0xd0 (slots 77-84); slot 39 also uses its key table
    void* inputDevice;  // +0x46c tested non-null together with throttleAxis before the throttle axis is latched in slot 63 (-1 when absent)
    float throttleInput;  // +0x470 slot 63 latches axis 0x500 value (dead zone 0.2, else -1 when no device); slot 80 tests it > 0.33
    float steerInput;  // +0x474 slot 63: = -axis(0x4f8).value; slot 88 classifies it with 0x504.y into 8 stick sectors; slot 64 zeroes it when crashed
    bool throttleOn;  // +0x478  racesnd.cpp throttle-on flag (RaceView +0x478); Bike's gas gyro acts only while set
    bool driveDisengaged;                     // +0x479  slot 49: = no crash and (backingUp or slot 81); passed as the gearbox's driveCut, so DriveForce gives no engine torque; Bike's gas gyro and racesnd.cpp's engine channel test it (tier 2)
    char backingUp;                           // +0x47A  slot 49: = slot 82 (truncated to a byte) (KrustyBike: forwardSpeed < 6 and the back-up input); DriveForce pushes backwards while set; KrustyBike slot 86 ramps that push off the BackUpBottom pose phase +0x664 (tier 2)
    char pad_0x47B[0x1];
    VehicleSteerState* steerState;  // +0x47c VehicleSteerState*: node plus steer angle; slots 53/60/73/36 use it
    VehicleSpeedState* engineState;  // +0x480 VehicleSpeedState*: 6-entry per-gear tables, advanced by UpdateThrottle/AutoShift with throttle/brake flags and speed in slot 49 (provisional)
    int justLanded;  // +0x484 slot 49 clears it per frame; slot 71 sets it to 1 when the airborne flag 0x108 drops (landing)
    Vec3 takeoffPosition;  // +0x488 slot 71 copies position field_0x0c here when the vehicle leaves the ground
    Vec3 takeoffVelocity;  // +0x494 slot 71 copies velocity field_0x64 here when the vehicle leaves the ground
    int outOfRace;  // +0x4A0  set to 1 by slot 44; EventManager counts the racer done, KrustyBike's nearest-bike search skips it
    float driveForceScale;                    // +0x4A4  the constructor sets 1.0; slot 86 (and KrustyBike's) passes it as the gearbox DriveForce scale
    int wheelsInContact;  // +0x4a8 slot 54 divides the wheel normal sum by it; slot 49: 0x5a8 = (it == wheel count), 0x5a4 = (it != 0); slots 46/7 branch on 0/1/2
    float leanAngle;  // +0x4ac slot 56 stores asin(...) of the ground normal against the forward axis; slot 57 returns it; Bike rotates the body by it
    float leanCos;  // +0x4b0 slot 56 stores cos(leanAngle) there
    float wheelForwardAlignment;              // +0x4B4  slot 49: = slot 75, |dot(mean wheel rollDirection, savedForward)| (1 with no wheels); Bike slot 99 (nose-over test) and slot 92 (lean target) use it (tier 2)
    float turnAngle;  // +0x4b8 result of slot 74 (signed deflection angle of the velocity), see turnRate
    float steerRate;  // +0x4bc slot 60: -(a*b) - steerAngle divided by (c*dt+...); slot 53 feeds steerRate*dt to the steer state
    Vec3 tiltAxisLocal;  // +0x4c0 slot 49: WorldToLocalDirection of the lift axis, used as the RotateAboutPoint axis
    Vec3 sideAxis;  // +0x4cc slots 34/35: cross product of the two basis vectors field_0x88 and field_0x94
    float suspensionWeightScale;              // +0x4D8  LoadVehicle's float argument after the weight (Bike passes 0.9); slot 0 multiplies baseWeight by it (provisional)
    float suspensionMass;                     // +0x4DC  slot 0: = (suspensionWeightScale * baseWeight + arg) / 32.2 (weight to slugs); Method_005293E0 passes it to Shock::ApplyDamping as the mass factor of the critical damping (tier 2)
    float targetLeanAngle;  // +0x4e0 Bike clamps it to +-maxLeanAngle and subtracts it from the current lean (slot 57)
    float leanError;        // +0x4e4 Bike slot 92 (0x0040ba30): _finite()/fabs on it (float, tier 2)
    float field_0x4e8;
    float wheelBase;  // +0x4ec slot 73 returns wheelBase / sin(steer angle) (bicycle-model turn radius)
    int spawnProtected;  // +0x4f0 slot 50 stores arg a here, slot 51 returns it; slot 33 (respawn) sets it using slot 45 (3.0)
    float spawnProtectTimer;   // +0x4f4 tier 2: countdown; slot 49 tests > 0 and subtracts field_0x13c (frame dt), slot 50 stores it
    VehicleAxis* steerAxis;  // +0x4f8 slot 63 reads its value into steerInput and 0x504.x
    VehicleAxis* leanAxis;  // +0x4fc slot 63 reads -value into controlInput.y
    VehicleAxis* throttleAxis;  // +0x500 slot 63 reads its value into throttleInput (dead zone 0.2)
    Vec3 controlInput;  // +0x504 slot 63 writes x = steer (cubed for axis kinds 2/3), y = -lean axis; slot 64 zeroes it when crashed
    Vec3 prevControlInput;  // +0x510 slot 49 copies controlInput here before slot 64 refreshes it
    float forceFeedbackGain;                  // +0x51C  slot 93 sets 0.5; KrustyBike slot 93 sets FeedbackSlider (TrackGame+0xffc) * 0.04; KrustyBike slot 94 scales the pad force smoothedForwardAccel * it * 180
    int steerPoseState;                       // +0x520  Bike sets 1..5 from controlInput.x (|x| > 0.9 hard, < 0.032 centre, else partial, by side); Bike slot 89 runs its 5-state pose machine on it (provisional)
    float controlResponse[6];                 // +0x524  KrustyBike slot 96 maps the response sliders (pct * 0.4 + 0.8; [0] steering pct * 0.2 + 0.575): [0] Steering, [1] Pitch, [2] PressThrottle, [3] ReleaseThrottle, [4] PressBrakes, [5] ReleaseBrakes; [2..3] are the gearbox rise/fall rates
    VehicleWheel** wheelList;  // +0x53c array of VehicleWheel*, iterated up to wheelCount in slots 7/54/72/75/85/86
    int wheelCapacity;               // +0x540 wheel capacity (AddWheel)
    int wheelCount;  // +0x544 loop bound over wheelList in every wheel loop
    VehicleWheel* primaryWheel;  // +0x548 slot 7 (single contact) and slot 58 (single wheel) use it as the lead wheel; slot 46 pairs it with secondaryWheel
    VehicleWheel* secondaryWheel;  // +0x54c partner of primaryWheel in slot 46 (two-wheel case)
    int ownedWheelsInContact;  // +0x550 PlaceWheels: wheels in contact whose ownerRef is set (no reader decoded)
    VehicleTicker** lateTickers;   // +0x554 owned arrays of objects with a virtual slot 0 (RunTickers)
    int lateTickerCapacity;               // +0x558 capacity of lateTickers
    int lateTickerCount;               // +0x55c count of lateTickers
    VehicleTicker** earlyTickers;  // +0x560 array of objects whose virtual slot 0 is called first by RunTickers; AddEarlyTicker appends to it
    int earlyTickerCapacity;               // +0x564 capacity of earlyTickers
    int earlyTickerCount;               // +0x568 count of earlyTickers
    int auxWheelCount;               // +0x56c wheels added with an aux object (AddWheel)
    int contactTotal;  // +0x570 = field_0x1cc + wheelsInContact (body contacts plus wheels in contact) in slot 7; slot 49 branches on > 0
    Vec3 crashHeading;     // +0x574 horizontal forward (x, 0, z) stored on a crash (Bike slot 100, Vehicle::CheckCrash) and by the Bike and KrustyBike loaders (no reader decoded)
    float maxLeanAngle;  // +0x580 Bike clamps the target lean to +-maxLeanAngle
    float maxLeanRate;  // +0x584 Bike limits the lean error change to maxLeanRate * dt
    float smoothedVerticalAccel;  // +0x588 slot 49: = verticalAccelSmoother->value, fed with (vy - prev vy) * (1/dt)
    VehicleSmoother* verticalAccelSmoother;  // +0x58c VehicleSmoother fed in slot 49 with the per-step change of velocity.y
    float smoothedForwardAccel;  // +0x590 slot 49: = forwardAccelSmoother->value
    float prevLocalForwardVelocity;  // +0x594 slot 49: stores local velocity z after smoothing (previous-step value for the next difference)
    VehicleSmoother* forwardAccelSmoother;  // +0x598 VehicleSmoother fed with (local vel z - prevLocalForwardVelocity)/dt
    int landingLatched;  // +0x59c slot 71: set when landing while not yet latched, cleared in slot 49 while airborne
    int wheelPointInContact;  // +0x5A0 UpdateWheelsInContact (0x00529a20) sets it when any in-contact wheel's pointInContact (+0x15c) is set
    int anyWheelInContact;  // +0x5a4 slot 49: = (wheelsInContact != 0)
    int allWheelsInContact;  // +0x5a8 slot 49: = (wheelsInContact == wheelCount)
    VehicleSparkEmitter* sparkEmitter;  // +0x5AC  VehicleSparkEmitter built by LoadVehicle (Vehicle.cpp line 0x1dc); impact handlers cast it to VehicleImpactSink
    float prevThrottleFraction;               // +0x5B0  slot 21 copies engineState->throttleFraction here; KrustyBike's tests compare it with the current throttle fraction (rising/falling)
    int prevCrashState;  // +0x5b4 slot 49 stores crashState there at the end of each step; slots 1/67 clear it
    char pad_0x5B8[0x4];      // own data ends at 0x5bc; the compiler places the vtordisp there
};

VEH_CHECK_OFFSET(Vehicle, forwardSpeed, 0x434);
VEH_CHECK_OFFSET(Vehicle, turnRate, 0x43C);
VEH_CHECK_OFFSET(Vehicle, movingForward, 0x440);
VEH_CHECK_OFFSET(Vehicle, crashState, 0x444);
VEH_CHECK_OFFSET(Vehicle, speedGainLimit, 0x450);
VEH_CHECK_OFFSET(Vehicle, crashTimer, 0x454);
VEH_CHECK_OFFSET(Vehicle, crashTimerReload, 0x458);
VEH_CHECK_OFFSET(Vehicle, inputMap, 0x468);
VEH_CHECK_OFFSET(Vehicle, inputDevice, 0x46C);
VEH_CHECK_OFFSET(Vehicle, throttleInput, 0x470);
VEH_CHECK_OFFSET(Vehicle, steerInput, 0x474);
VEH_CHECK_OFFSET(Vehicle, throttleOn, 0x478);
VEH_CHECK_OFFSET(Vehicle, driveDisengaged, 0x479);
VEH_CHECK_OFFSET(Vehicle, backingUp, 0x47A);
VEH_CHECK_OFFSET(Vehicle, steerState, 0x47C);
VEH_CHECK_OFFSET(Vehicle, engineState, 0x480);
VEH_CHECK_OFFSET(Vehicle, justLanded, 0x484);
VEH_CHECK_OFFSET(Vehicle, takeoffPosition, 0x488);
VEH_CHECK_OFFSET(Vehicle, takeoffVelocity, 0x494);
VEH_CHECK_OFFSET(Vehicle, driveForceScale, 0x4A4);
VEH_CHECK_OFFSET(Vehicle, wheelsInContact, 0x4A8);
VEH_CHECK_OFFSET(Vehicle, leanAngle, 0x4AC);
VEH_CHECK_OFFSET(Vehicle, leanCos, 0x4B0);
VEH_CHECK_OFFSET(Vehicle, wheelForwardAlignment, 0x4B4);
VEH_CHECK_OFFSET(Vehicle, turnAngle, 0x4B8);
VEH_CHECK_OFFSET(Vehicle, steerRate, 0x4BC);
VEH_CHECK_OFFSET(Vehicle, tiltAxisLocal, 0x4C0);
VEH_CHECK_OFFSET(Vehicle, sideAxis, 0x4CC);
VEH_CHECK_OFFSET(Vehicle, suspensionWeightScale, 0x4D8);
VEH_CHECK_OFFSET(Vehicle, suspensionMass, 0x4DC);
VEH_CHECK_OFFSET(Vehicle, targetLeanAngle, 0x4E0);
VEH_CHECK_OFFSET(Vehicle, leanError, 0x4E4);
VEH_CHECK_OFFSET(Vehicle, field_0x4e8, 0x4E8);
VEH_CHECK_OFFSET(Vehicle, wheelBase, 0x4EC);
VEH_CHECK_OFFSET(Vehicle, spawnProtected, 0x4F0);
VEH_CHECK_OFFSET(Vehicle, spawnProtectTimer, 0x4F4);
VEH_CHECK_OFFSET(Vehicle, steerAxis, 0x4F8);
VEH_CHECK_OFFSET(Vehicle, leanAxis, 0x4FC);
VEH_CHECK_OFFSET(Vehicle, throttleAxis, 0x500);
VEH_CHECK_OFFSET(Vehicle, controlInput, 0x504);
VEH_CHECK_OFFSET(Vehicle, prevControlInput, 0x510);
VEH_CHECK_OFFSET(Vehicle, forceFeedbackGain, 0x51C);
VEH_CHECK_OFFSET(Vehicle, steerPoseState, 0x520);
VEH_CHECK_OFFSET(Vehicle, controlResponse, 0x524);
VEH_CHECK_OFFSET(Vehicle, wheelList, 0x53C);
VEH_CHECK_OFFSET(Vehicle, wheelCount, 0x544);
VEH_CHECK_OFFSET(Vehicle, primaryWheel, 0x548);
VEH_CHECK_OFFSET(Vehicle, secondaryWheel, 0x54C);
VEH_CHECK_OFFSET(Vehicle, contactTotal, 0x570);
VEH_CHECK_OFFSET(Vehicle, smoothedVerticalAccel, 0x588);
VEH_CHECK_OFFSET(Vehicle, verticalAccelSmoother, 0x58C);
VEH_CHECK_OFFSET(Vehicle, smoothedForwardAccel, 0x590);
VEH_CHECK_OFFSET(Vehicle, prevLocalForwardVelocity, 0x594);
VEH_CHECK_OFFSET(Vehicle, forwardAccelSmoother, 0x598);
VEH_CHECK_OFFSET(Vehicle, landingLatched, 0x59C);
VEH_CHECK_OFFSET(Vehicle, wheelPointInContact, 0x5A0);
VEH_CHECK_OFFSET(Vehicle, anyWheelInContact, 0x5A4);
VEH_CHECK_OFFSET(Vehicle, allWheelsInContact, 0x5A8);
VEH_CHECK_OFFSET(Vehicle, sparkEmitter, 0x5AC);
VEH_CHECK_OFFSET(Vehicle, prevThrottleFraction, 0x5B0);
VEH_CHECK_OFFSET(Vehicle, prevCrashState, 0x5B4);
typedef char veh_assert_sizeof[(sizeof(Vehicle) == 0x5ec) ? 1 : -1];
