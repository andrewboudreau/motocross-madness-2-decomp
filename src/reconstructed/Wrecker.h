#pragma once

#include "GameObject.h"
#include "MatrixUtil.h"

// Reconstruction of part of D:\aardvark\VC\krusty2\wrecker.cpp. Evidence and
// the per-function table are in docs/WRECKER.md.
//
// Confirmed (RTTI): Wrecker : GraphicsTest : GameObject : BaseObject, all
// single non-virtual inheritance; primary vtable 0x00558e24 (27 slots,
// COL 0x0055fb70). Wrecker overrides slot 0 (deleting dtor 0x005300a0),
// slot 10 (0x005306e0), slot 14 (0x004de400) and slot 23 (0x00532890).
// Allocation size 0x53c (Bike slot 97, `new(bike.cpp) Wrecker(1)`).

// Shared debug-drawing base and collision layout. GraphicsTest ends at +0x34;
// the fields following it belong to each derived object.
#include "../krusty2/collision/CollisionObject.h"

class SoultreeObject;
typedef SoultreeObject UnknownWreckerSkeleton;

// Object at Wrecker+0x38: RTTI ConstraintMethodCollisionModel (new 0x10c at
// line 0x68, constructor 0x0043b8d0 writes vtables 0x00551260 at +0 and
// 0x005511f0 at +0xc, COLs 0x0055b0f8 and 0x0055b098). Its CollisionObject
// base keeps GraphicsTest/GameObject at +0xc, so Wrecker's GameObject calls
// (slot 8, slot 11, AppendChild) go through that subobject. Wrecker reads
// manualStep (+0xe8), the probes (+0x100/+0x104, 32-byte ConstraintProbe
// records) and sets the damping and impulse limits (+0xf0..+0xf8).
#include "../krusty2/constraint/ConstraintMethodCollisionModel.h"

// Global collision quadtree (0x0068aba4, VisibilityQuadTree's constructor
// installs it). 0x004dcf20 (thiscall, ret 8) removes an object from a cell.
#include "../krusty2/broadphase/Quadtree.h"
extern QuadTree* g_collisionQuadTree;

// A 4x4 transform read as four Vector3 rows (right, up, forward,
// position). Retail copies these rows with integer moves, which is how VC6
// copies a Vector3 struct (a Matrix4 element-wise copy would use the FPU).
struct UnknownWreckerFrame {
    Vector3 right;
    float rightW;                             // +0xC  fourth column of the right row (the 4x4's _14)
    Vector3 up;
    float upW;                                // +0x1C  _24
    Vector3 forward;
    float forwardW;                           // +0x2C  _34
    Vector3 position;
    float positionW;                          // +0x3C  _44
};

// A 4x4 transform with D3DMATRIX element names (row-vector convention,
// translation in row 4). Slot 10 keeps three of these as locals; the
// element names (not m[r][c] indexing) give retail's operand order in the
// inlined inverse and products.
struct UnknownWreckerMatrix {
    UnknownWreckerMatrix() {}
    float _11, _12, _13, _14;
    float _21, _22, _23, _24;
    float _31, _32, _33, _34;
    float _41, _42, _43, _44;
};

// Vector storage shared with the scene-node helpers. The complete scene-node
// declaration is needed only by Wrecker.cpp; other consumers use pointer types.
#include "../krusty2/math/Vec3.h"

// The character's current motion (src/krusty2 Motion, Motnctrl.h).
struct UnknownWreckerMotion {
    unsigned char field_0x00[0xa8];
    int frameCount;                    // +0xa8 "FrameCount"
    unsigned char field_0xac[0xb4 - 0xac];
    int fps;                           // +0xb4 "FPS"
};

// Rider is the primary Character/D3DIM subobject; its GameObject base remains virtual.
#include "../krusty2/motion/D3DIMSoultreeCharacter.h"
typedef D3DIMSoultreeCharacter UnknownWreckerCharacter;

// Object at Wrecker+0x40: RTTI PhysicsRigidBody (new 0x2a8 at line 0x5f,
// constructor 0x004cc120 writes vtable 0x00556ec0, COL 0x0055e558). Wrecker
// calls it through slots 8, 28, 29, 33, 34 and 44 and reads mass (+0x178) for
// the box inertia 0x00532150 scales by 1/12.
#include "../krusty2/rigidbody/PhysicsBody.h"

// Object at Wrecker+0x3c: RTTI SelectiveGravityModel (new 0x3c at line
// 0x5b, constructor 0x004f9760). Wrecker adds the rigid body through slot 27
// and sets manualStep (+0x38) so slot 10 leaves the stepping to Wrecker.
#include "../krusty2/gravity/SelectiveGravityModel.h"

// One particle of the manager below (the layout src/krusty2/effects/
// ParticleManager.h documents; 0x00531740 fills position, frame, age,
// lifetime, size, growth, flags and velocity directly).
struct UnknownWreckerParticle {
    Vector3 position;          // +0x00 position
    int color;                 // +0x0c colour
    int frame;                 // +0x10 sprite frame
    float age;                 // +0x14 age
    float lifetime;            // +0x18 lifetime
    float size;                // +0x1c size
    float growth;              // +0x20 growth per tick
    unsigned char field_0x24[0x34 - 0x24];
    unsigned int flags;        // +0x34 flags (4: integrate position, 0x10: gravity)
    Vector3 velocity;          // +0x38 velocity
    void* vertex;              // +0x44
};

// Particle system at Wrecker+0x11c (Bike hands over its own; RTTI
// ParticleManager). 0x004baa50 (thiscall, ret 0x20) adds one particle;
// 0x00531740 takes the next free entry of particles itself.
class UnknownWreckerParticles : public GameObject {
public:
    int AddParticle(float age, const Vector3* position, int frame, float size,
                              float lifetime, float growth, unsigned int flags, int color);

    int activeCount;           // +0x2c live particle count (the next free index)
    int visibleCount;                         // +0x30  ParticleManager +0x30 (src/krusty2/effects/ParticleManager.h); not read here
    unsigned char field_0x34[0x40 - 0x34];
    UnknownWreckerParticle* particles[1000]; // +0x40
    unsigned char field_0xfe0[0x1f98 - 0xfe0];
    Vector3 gravity;           // +0x1f98 gravity (0, -64, 0)
};

// 0x00460b50 (cdecl): table-driven square root (FollowCamera.h declares the
// same function).
float FastSqrt(float value);
// 0x00460c00 (cdecl): the matching reciprocal square root.
float FastInvSqrt(float value);

// 0x005015b0 (cdecl): the out-of-line copy of a vector scale (see
// src/krusty2/math/Math3D.h, Vec3ScaleCall).
Vec3* Vec3ScaleCall(Vec3* out, const Vec3* v, float scale);


class Wrecker : public GraphicsTest {
public:
    explicit Wrecker(int flags);                       // 0x0052ff90
    virtual ~Wrecker();                                // 0x005300a0 / 0x005300c0
    // Slot 10, 0x005306e0 (4178 bytes): the per-frame wreck update
    // (Wrecker.cpp).
    virtual int Tick(float frameTime);
    // Slot 14, 0x004de400: `jmp GameObject slot 14`. The linker folded
    // this body with identical ones of other classes, so the copy that
    // survives lies outside wrecker.cpp's code.
    virtual int Render3D();
    virtual int OnControlPress(ControlEvent* event, InputButtonState* entry);       // 0x00532890

    // 0x00530190: slot 8 on the owner, then 0x005301c0; returns this.
    Wrecker* SetupWrecker(void* owner, UnknownWreckerCharacter* rider, const char* name);
    void CreateBodies(UnknownWreckerCharacter* rider, const char* name); // 0x005301c0: gravity, rigid body, collision
    void AddContactAnimation(int value);             // appends to contactAnimations
    void AddRandomAnimation(int value);             // appends to randomAnimations
    int AddIndexedAnimation(int value);              // appends to indexedAnimations, returns its index
    void AddIgnoredObject(CollisionObject* object); // appends to ignoreObjects
    void EndWreck();                  // 0x005327c0: back to state 0, the pose reset
    void SetPushBox(UnknownWreckerSkeleton* frame, Vector3 center, Vector3 halfExtents); // 0x005328b0: boxFrame/boxCenter/boxHalfExtents
    void PushProbesOutOfBox(float frameTime); // 0x005329e0: pushImpulse (samples/race/WreckerNearMisses.cpp)
    void AddProbe(Vector3 offset, int bone); // 0x00532900: a probe point and its position/hit slots
    void SprayContactParticles(Vector3 a, Vector3 b, Vector3 normal, float frameTime);
    void EmitProbeTrail(Vector3 from, Vector3 to, float frameTime);
    void EmitProbeEffects(float frameTime); // 0x00532020: trail and spray for each probe that hit
    void UpdateInertia();             // 0x00532150: box inertia from the skeleton's bounds
    void StartAnimatedWreck(int index); // 0x00532220: state 1, indexed animation, body disabled
    void StartTumble();               // 0x00532310: state 2, random animation, body moving with the pose steps
    void ContinueAsTumble();          // 0x00532490: state 1 -> 2 halfway through the animation (slot 10)
    void RebuildPoseSteps();

    UnknownWreckerCharacter* character; // +0x34
    ConstraintMethodCollisionModel* collisionModel; // +0x38
    SelectiveGravityModel* gravityModel; // +0x3c
    PhysicsRigidBody* rigidBody; // +0x40
    int wreckState;            // +0x44 wreck state (0 idle, 1..3)
    int* indexedAnimations;    // +0x48
    int indexedAnimationCount; // +0x4c
    int* randomAnimations;     // +0x50
    int randomAnimationCount;  // +0x54
    int* contactAnimations;    // +0x58
    int contactAnimationCount; // +0x5c
    CollisionObject** ignoreObjects; // +0x60
    int ignoreObjectCount;     // +0x64
    float inertiaFactor;       // +0x68
    UnknownWreckerFrame lastPose; // +0x6c last pose of poseNode
    int hadContact;                           // +0xac
    int contactThisFrame;                     // +0xb0
    int carryRiderPose;        // +0xb4 slot 10 carries the rider's root over by poseNode's motion
    UnknownWreckerSkeleton* poseNode; // +0xb8
    Vector3 linearStep;        // +0xbc
    Vector3 angularStep;       // +0xc8
    int pendingAnimation;      // +0xd4
    int anyProbeHit;           // +0xd8
    Vector3 hitCenter;         // +0xdc
    Vector3 boxCenter;         // +0xe8
    Vector3 boxHalfExtents;    // +0xf4
    UnknownWreckerSkeleton* boxFrame; // +0x100 space of the box boxCenter/boxHalfExtents
    Vector3* probePositions;   // +0x104 one per collision-model probe
    int* probeHits;            // +0x108 one per collision-model probe
    Vector3 pushImpulse;       // +0x10c
    float pushScale;           // +0x118
    UnknownWreckerParticles* particles; // +0x11c
    int trailFrame;            // +0x120 particle frame, cycles 0..12
    int sprayFrame;            // +0x124
    float randomTable[256];    // +0x128 random values in [0, 1)
    int randomIndex;           // +0x528
    int skipProbeEffects;      // +0x52c set by EndWreck: slot 10 skips one EmitProbeEffects
    int blendStarted;          // +0x530 a wreck blend is under way; slot 10 sets carryRiderPose once the character's blend ends
    int poseStepsPending;      // +0x534 slot 10 rebuilds the pose steps once carryRiderPose is set
    int tumbleStarted;         // +0x538 set by StartTumble; cleared when a second blend starts
};

typedef char wrecker_assert_sizeof[(sizeof(Wrecker) == 0x53c) ? 1 : -1];
