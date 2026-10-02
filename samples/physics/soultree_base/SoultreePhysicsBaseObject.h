// SoultreePhysicsBaseObject -- reconstructed class layout (SoulTreePhysics.cpp).
//
// Evidence (tier 1 unless noted):
//  * RTTI: direct base GameObject, which is a *virtual* base (BCD attributes 0x10,
//    pdisp=4/vdisp=4 in analysis/rtti_classes.json).  The class therefore owns a
//    vfptr at +0 and a vbptr at +4; the GameObject subobject lives at +0x220 (544).
//  * COL 0x0055f0d0 / vtable 0x00557d90 at object offset 0 (40 slots, all of them
//    declared by this class), COL 0x0055f098 / vtable 0x00557d20 at offset 544 is
//    the GameObject virtual-base vtable (slot 0 = vtordisp deleting-destructor
//    thunk 0x00504280, slot 10 = thunk 0x005042d0 -> 0x005036f0).
//  * ctor 0x00500aa0 writes vptr 0x00557d90 to +0, vbtable 0x00557e30 to +4 and
//    the 0x00557d20 vptr into the virtual base; sizeof the non-virtual part is
//    0x21c (derived classes place their next base at 540).
//  NOTE: analysis/vtable_overrides.json labels many of the 40 primary slots as
//  "overrides" of GameObject slots.  That is an artifact of comparing slot
//  numbers across unrelated vtables; slot 0 here is a float-taking method, not a
//  destructor.  We treat all 40 slots as introduced by this class.
//
// Member names are field_0xNN unless the arithmetic clearly shows the meaning;
// semantic names are tier 3 (provisional).
#ifndef SOULTREE_PHYSICS_BASE_OBJECT_H
#define SOULTREE_PHYSICS_BASE_OBJECT_H

#include "SoultreePhysicsTypes.h"

class SoultreeContact;    // elements of the field_0x12c array
class CollisionObject;    // pointed to by field_0x128 (collision/CollisionObject.h (src/krusty2))
struct SoultreeLight; // defined in SoulTreePhysics.cpp (slot 21)
// Object probed for ray hits (field_0x1f4); only the members the base uses are modelled.
struct SoultreeProbe {
    char pad_0x00[0x40];
    float worldScale;              // +0x40 copied to field_0x1f8 by slot 2
    // thiscall, callee pops 6 args
    int Fn_506e90(const Vec3* from, const Vec3* to, Vec3* out, int a, int b,
                  int c);
};
class SoultreeSlot1f0;
class SoultreeAttachTarget;
class SoultreeAttachment; // 40-byte records in the field_0x1d4 array

#include "core/GameObject.h"   // BaseObject, GameObject

class SoultreePhysicsBaseObject : public virtual GameObject {
public:
    // 0x00500aa0 (thiscall, ret 8 with the hidden vbase flag): forwards its argument to the
    // GameObject(int) virtual-base ctor 0x00468ca0 (tier 1 decoded).  Body not reconstructed.
    explicit SoultreePhysicsBaseObject(int flags);
    virtual ~SoultreePhysicsBaseObject();       // deleting dtor 0x00504290 via vbase vtable slot 0
    virtual int GameObjectVirtualSlot10(float dt); // override, thunk 0x005042d0 -> 0x005036f0
    // --- vtable 0x00557d90 (offset 0), slots 0..39, all introduced here ---
    virtual void UnknownVirtualSlot0(float value);
    virtual void UnknownVirtualSlot1(float value);
    // Slot 2 (0x00500c50): `ret 0x68` = 26 argument dwords (tier 1).  Grouping and types
    // are tier 2, from the body's stores and the struct-copy shape at both callers
    // (SoultreePhysicsCharacter slot 40 0x00503de0, SoultreePhysicsObject slot 40 0x00503970):
    //   a1 -> collisionObject->Fn_004320f0(a1, 0, 1, 1) after the CollisionObject is created
    //   a2 != 0: new node (0x1a4 bytes, ctor 0x004fb2b0(1)) stored in centerNode
    //   a3 -> respawnPosition and the node position (sceneNode->Fn_4fc630(a3.x, a3.y, a3.z))
    //   a4 -> respawnHeading and bodyForward;  a5 -> bodyUp
    //   a6 -> terrain (pointer: if non-null, its float at +0x40 goes to terrainScale)
    //   a7 -> field_0x124;  a8 -> baseWeight
    //   a9 -> collisionPointCapacity (count; collisionPoints = zeroed array of a9 dwords)
    //   a10 -> attachmentCapacity (count; attachments = a10 records of 40 bytes)
    //   a11 -> track;  a12 -> fixedStepTime;  a13 -> maxStepsPerFrame
    //   a14 -> dragCoefficient;  a15 -> restitution
    //   a16 -> collisionRadius (squared as the radius when a17 == 1: 1/(0.4*m*r*r))
    //   a17 -> collisionShape (1 = sphere inertia, else box from the node extents)
    //   a18 -> restContactThreshold;  a19 -> byte groundProbeMask
    //   a20 != 0: new CollisionObject (0xb8 bytes, ctor 0x00431e70(1)) stored in collisionObject
    // Returns the GameObject virtual base (`this ? vbase : 0` at 0x005011f0).
    virtual GameObject* UnknownVirtualSlot2(int a1, int a2, Vec3 a3, Vec3 a4,
                                            Vec3 a5, void* a6, void* a7, float a8,
                                            int a9, int a10, SoultreeSlot1f0* a11, float a12,
                                            int a13, float a14, float a15, float a16, int a17,
                                            int a18, unsigned char a19, int a20);
    // Slot 3 (ret 0x1c).  Pointer types are tier 2: Vehicle callers (slots 38, 49) pass
    // Vec3 addresses for a1..a4; a5 is the event code KrustyBike 0x0048dbf0 compares with
    // 1000 and 0x67.  a7 is the address of the same local whose value Vehicle slot 38
    // passes as slot 4's a5 (the other body's invMass float), hence float* (tier 2).
    virtual void UnknownVirtualSlot3(const Vec3* a1, const Vec3* a2,
                                     const Vec3* a3, const Vec3* a4,
                                     int a5, int a6, float* a7);
    // Slot 4 (ret 0x3c).  a2..a4/a8..a10 are dereferenced as Vec3 by this body (tier 1);
    // a1, a11, a12 receive Vec3 addresses at the Vehicle slot 38 call site (tier 2).  That
    // call (0x00526c59..0x00526cb1) pushes the event kind (its own a2) as a5 and the local
    // holding the other body's invMass (a float, 1/bodyMass) as a6, with that local's
    // address as a14 (tier 1 push order); KrustyBike 0x0048dc60 reads *a14 (tier 2).
    virtual void UnknownVirtualSlot4(const Vec3* a1, Vec3* a2, const Vec3* a3,
                                     const Vec3* a4, int a5, float a6, int a7,
                                     const Vec3* a8, const Vec3* a9,
                                     const Vec3* a10, Vec3* a11, Vec3* a12,
                                     int a13, float* a14, float a15);
    virtual int UnknownVirtualSlot5(int value);
    virtual void UnknownVirtualSlot6(Vec3* a, float* b);
    virtual void UnknownVirtualSlot7(const Vec3* a);  // only read (0x005019e0, tier 2)
    virtual void UnknownVirtualSlot8();
    virtual void UnknownVirtualSlot9(float dt, int* steps);
    virtual int UnknownVirtualSlot10();
    // Slot 11 (ret 0x14): returns the 0x004b0df0 result (tier 1).  a2..a4 receive Vec3
    // addresses and a5 an int address at the Vehicle slot 49 call site (tier 2).
    virtual int UnknownVirtualSlot11(int a1, Vec3* a2, Vec3* a3, Vec3* a4,
                                     int* a5);
    virtual int UnknownVirtualSlot12(int value);
    virtual void UnknownVirtualSlot13(Vec3* a, Vec3* b, float c);
    // Slot 14 (ret 0xc): retail 0x00502080 passes a2 to slot 16, a3 to slot 15 and later
    // dereferences a1 (tier 1 decoded stack offsets); all three are Vec3 pointers.
    // a1 is not const: the KrustyBike override 0x004965e0 zeroes a1->x and a1->z (tier 1).
    virtual void UnknownVirtualSlot14(Vec3* a1, const Vec3* a2,
                                      const Vec3* a3);
    virtual void UnknownVirtualSlot15(const Vec3* a, Vec3* b);
    virtual Vec3 UnknownVirtualSlot16(const Vec3* a);
    virtual Vec3 UnknownVirtualSlot17();
    virtual void UnknownVirtualSlot18(SoultreeAttachment* a);
    virtual void UnknownVirtualSlot19(SoultreeAttachment* a);
    virtual void UnknownVirtualSlot20(SoultreeAttachment* a);
    virtual void UnknownVirtualSlot21();
    virtual int UnknownVirtualSlot22();
    virtual int UnknownVirtualSlot23();
    virtual int UnknownVirtualSlot24();
    virtual int UnknownVirtualSlot25();
    virtual void UnknownVirtualSlot26();
    virtual void UnknownVirtualSlot27();
    virtual int UnknownVirtualSlot28(int a);
    virtual void UnknownVirtualSlot29(int a);
    virtual void UnknownVirtualSlot30();
    virtual void UnknownVirtualSlot31();
    virtual float UnknownVirtualSlot32();
    virtual int UnknownVirtualSlot33(const Vec3* a1, const Vec3* a2,
                                     const Vec3* a3, const Vec3* a4, int a5,
                                     float a6);  // a4 unused here; Vehicle passes a Vec3 address (tier 2)
    // Slots 34/35 are declared void: 0x004aa1c0/0x004aa1e0 end in `call; ret` with no use
    // of eax, identical for either return type; the Vehicle overrides 0x0040c4c0 and
    // 0x0040c540 return nothing (tier 2).
    virtual void UnknownVirtualSlot34();
    virtual void UnknownVirtualSlot35(int a, int b);
    virtual void UnknownVirtualSlot36();
    virtual SoultreeAttachment* UnknownVirtualSlot37(int type, void* a2, SoultreeAttachTarget* a3, const Vec3* v);
    // Slot 38 (ret 0xc): 0x00501600 switches on a2 (0x66/0x68/0x69/0x6a/0x2711) and reads
    // a3->+0x60 as the other body (tier 1); a3's type is provisional.
    virtual void UnknownVirtualSlot38(int a1, int a2, void* a3);
    // Slot 39 (ret 4): argument unused here; Vehicle slot 49 0x0052a940 passes it the same
    // dword it passes to slot 9 (float dt), so float (tier 2).
    virtual int UnknownVirtualSlot39(float dt);

    // Non-virtual helper 0x00501230 (thiscall, no args): installs the two TU-local contact
    // callbacks 0x00500c00/0x00500c30 into collisionObject (+0x88/+0x8c).  Tier 2; called by slot 2.
    void Fn_501230();
    // Non-virtual helpers reached from the GameObject slot 10 override 0x005036f0 (tier 1 call
    // shapes): 0x00502f60 (thiscall, ret 0xc) is the per-frame step, 0x00502c40 (thiscall, no
    // arguments) the rest/settle check it calls.
    void Fn_502f60(int steps, int held, int refreshed);
    void Fn_502c40();

    // --- data members (offsets confirmed by decoded accesses; names provisional) ---
    // vfptr at +0, vbptr at +4 (compiler generated)
    SoultreeObject* sceneNode;           // +0x08 callee of 0x4fc630/0x4fc970/... (node/transform owner)
    Vec3 position;  // +0x0c node-local translation: slots 2/33 SetPosition then GetPosition(&position); slot 28 subtracts the collision push-out (CollisionObject +0x5c) and writes it back; slot 39 restores it from respawnPosition; Vehicle slot 49 writes it to the node each step
    Vec3 centerOfMass;            // +0x18 position (tier 3): subtracted from contact points
    float invMass;                   // +0x24 1 / bodyMass (slot 0)
    float field_0x28;                   // ctor: 1.0f
    float bodyRoll;                   // 0x2c..0x44: filled by 0x4b5a60(...), previous
    float bodyPitch;                   // +0x30 copy at 0x48..0x60 (slots 1, 33, 36, 7 tail)
    float bodyYaw;  // +0x34 'yaw' out-parameter of OrientationAnglesFromVectors (slot 2, Vehicle slot 49); copied to savedYaw by slot 29
    float bodySinRoll;  // +0x38 'sinRoll' out-parameter of OrientationAnglesFromVectors; reset with bodyCosRoll = 1 in slots 1/36 (sin/cos of 0)
    float bodyCosRoll;  // +0x3c 'cosRoll' out-parameter of OrientationAnglesFromVectors; reset to 1.0f in slots 1/36
    float bodySinPitch;  // +0x40 'sinPitch' out-parameter of OrientationAnglesFromVectors; reset to 0 in slots 1/36
    float bodyCosPitch;  // +0x44 'cosPitch' out-parameter of OrientationAnglesFromVectors; reset to 1.0f in slots 1/36
    float savedRoll;  // +0x48 slot 29 (end of step), slots 1/36: savedRoll = bodyRoll; Bike fills it from OrientationAnglesFromVectors(savedForward, savedUp, ...) while the rider node is tracked
    float savedPitch;  // +0x4c slot 29: savedPitch = bodyPitch (saved copy of bodyPitch)
    float savedYaw;  // +0x50 slot 29: savedYaw = bodyYaw (saved copy of bodyYaw)
    float savedSinRoll;  // +0x54 slot 29: savedSinRoll = bodySinRoll
    float savedCosRoll;  // +0x58 slot 29: savedCosRoll = bodyCosRoll
    float savedSinPitch;  // +0x5c slot 29: savedSinPitch = bodySinPitch
    float savedCosPitch;  // +0x60 slot 29: savedCosPitch = bodyCosPitch
    Vec3 velocity;  // +0x64 slot 14: velocity += acceleration * stepTime (integrated each sub-step); linearSpeed = |velocity|; passed as the body velocity to ContactSolveImpulse and 0x0043a640; slot 6 drag opposes it
    Vec3 acceleration;  // +0x70 slot 14: acceleration = force accumulator * invMass, then velocity += acceleration * stepTime
    Vec3 prevVelocity;  // +0x7c Vehicle slot 49 stores prevVelocity = velocity at the end of each sub-step and differences the next velocity against it
    Vec3 bodyForward;  // +0x88 node world Z axis: slot 34 GetAxesIn(0, &bodyForward, &bodyUp); slot 35 SetAxesIn with the same pair; slot 14 re-orthogonalises it against bodyUp; input to OrientationAnglesFromVectors
    Vec3 bodyUp;  // +0x94 node world Y axis (second out-parameter of GetAxesIn in slot 34, SetAxesIn in slot 35); slot 36 resets it to the global up vector
    Vec3 savedForward;  // +0xa0 copy of bodyForward taken by slot 29 at the end of each step and by slots 1/2/36; Vehicle slot 43 derives respawnHeading from it
    Vec3 savedUp;  // +0xac copy of bodyUp taken by slot 29 and by slots 1/2/36
    float prevSpeed;  // +0xb8 Vehicle slot 49 sets prevSpeed = linearSpeed at the start of each sub-step; slot 6 applies drag only when prevSpeed > 0.001; the rest test zeroes the velocity when prevSpeed and linearSpeed are both tiny
    float linearSpeed;                   // +0xbc |velocity| (slots 3, 4, 14)
    Vec3 angularAcceleration;  // +0xc0 slot 14: angularVelocity += angularAcceleration * stepTime; produced from torque by slot 16 (per-axis invInertia * torque) and adjusted by slot 15
    Vec3 worldAngularVelocity;  // +0xcc worldAngularVelocity = sceneNode->LocalToWorldDirection(angularVelocity) after each solve (slots 3, 4, 38, Vehicle slot 49); passed with the lever arm to 0x0043a640 (slot 31) for the point velocity; read as the other body's angular velocity in slot 38
    Vec3 angularVelocity;  // +0xd8 body-frame angular velocity: slot 14 angularVelocity += angularAcceleration * stepTime then damped (* 0.999); passed as the angular velocity to ContactSolveImpulse (slot 4) and Fn_500220 (slot 3)
    Vec3 invInertia;  // +0xe4 body-frame inverse inertia diagonal in use: slot 16 multiplies torque by it per axis; passed as the inverse inertia to ContactSolveImpulse / Fn_500220; slot 2 initialises it from shapeInvInertia
    Vec3 shapeInvInertia;  // +0xf0 computed by slot 2 from the shape: 1/(0.4 m r^2) for a sphere (collisionShape == 1), box formula m/12 (h^2 + d^2) per axis otherwise; slots 15/16 use it instead of invInertia while slot 10 reports the body unloaded
    char field_0xfc[12];
    char airborne;  // +0x108 Vehicle slot 49: airborne = !wheel contact && !pointsTouching (no body point touching); slot 23 (moving on ground) requires !airborne; cleared by slot 1. Vehicle's comment calls it 'settled', which the assignment contradicts
    char respawnPending;  // +0x109 slot 39: when set, position = respawnPosition and the node is moved there; slot 11 uses respawnHeading as the placement heading while set; slot 2 clears it
    char asleep;  // +0x10a rest check Fn_502c40 (0x00502c40) zeroes the velocities and sets it once the body has rested 0.5 s on its contacts; GameObject slot 10 (0x005036f0) skips the whole update while it is set; slot 38 clears the other body's flag on a body-to-body hit; the ctor clears it
    Vec3 respawnPosition;  // +0x10c slot 2 stores the start position (a3) here; Vehicle slot 43 snapshots position into it; slot 39 restores position from it
    Vec3 respawnHeading;  // +0x118 slot 2 stores the start forward vector (a4); Vehicle slot 43 stores the horizontal, normalised savedForward; the placement search uses it as the reference direction while respawning
    GameObject* field_0x124;            // tier 3: only its byte +0x25 bit 0 is read (slot 21, Vehicle slot 38);
                                        // that offset is GameObject::field_0x25
    CollisionObject* collisionObject;  // +0x128 CollisionObject (0xb8 bytes, ctor 0x00431e70) built by slot 2; slot 28 runs its collision test (0x00435fb0) and pushes the body out by its +0x5c vector; slot 38 reads its contact point/normal (+0xa0/+0xac); 0x00501230 installs the collision callbacks at its +0x88/+0x8c
    SoultreeContact** collisionPoints;      // +0x12c array of collisionPointCount pointers
    int collisionPointCount;  // +0x130 loop bound over collisionPoints in slots 7, 13, 18, 30, 31
    int lastCollisionType;  // +0x134 collision callbacks 0x00500c00/0x00500c30 store the other collision object's type tag (+0x64; 0x68 = another physics body) here before calling slot 38
    char field_0x138;
    float stepTime;  // +0x13c slot 9: length of the current sub-step (fixedStepTime, or the whole frame when shorter); slot 14 integrates with it (v += a * stepTime, w += alpha * stepTime); invStepTime = 1/stepTime
    float invStepTime;  // +0x140 slot 9: invStepTime = 1.0f / stepTime
    float frameTime;  // +0x144 slot 9: frameTime = dt + stepRemainder (time to simulate this frame including the carried remainder); clamped to one fixed step when shorter
    float dragCoefficient;  // +0x148 slot 6: drag force = velocity * -(dragCoefficient * linearSpeed) (quadratic drag); stored from slot 2's a14
    float restitution;  // +0x14c first argument of ContactSolveImpulse (contact/ContactImpulse.cpp uses (restitution + 1)) in slot 4 and of Fn_500220 in slot 3; stored from slot 2's a15
    float baseWeight;  // +0x150 slot 2's a8; slot 0: totalWeight = load + baseWeight; Vehicle derives mass from it with the same 1/32.2 factor
    float totalWeight;  // +0x154 slot 0: totalWeight = load + baseWeight; weightForce.y = -totalWeight (gravity); mass = totalWeight * 0.0310559 (1/32.2 ft/s^2)
    float bodyMass;  // +0x158 totalWeight / 32.2 in slot 0 (renamed from 'mass', which collides with a local in slot 2)
    float loadWeight;  // +0x15c slot 32 returns it (0 while slot 10 reports unloaded); slot 33 passes slot 32's result through slot 1 to slot 0, which adds it to baseWeight
    float collisionRadius;  // +0x160 slot 2's a16: sphere radius squared in the inertia when collisionShape == 1; passed with collisionShape to the Vehicle contact query
    float field_0x164[9];
    Vec3 weightForce;  // +0x188 slot 0: weightForce.y = -totalWeight; Vehicle slot 49 starts the per-step force accumulator from it before drag and contact forces are added
    Vec3 rotationPivot;  // +0x194 node-local point the body is rotated about by Vehicle/Bike (RotateAboutPoint(rotationPivot, axis, angle)); set from the local centre of mass; cleared by slot 1
    Vec3 localCenterOfMass;  // +0x1a0 slot 8: localCenterOfMass = node->WorldToLocalPoint(centerOfMass) (centre of mass in node space)
    Vec3 scratchVector;  // +0x1ac member temporary reused for unrelated vectors: drag force (slot 6), cross products (slot 38), the type-4 attachment world point (slot 21), many Vehicle/Bike temporaries
    Vec3 scratchVector2;  // +0x1b8 second member temporary: the applied drag force in slot 6, temporaries in Vehicle/Bike
    int collisionShape;  // +0x1c4 slot 2's a17: 1 = sphere (inertia from collisionRadius), otherwise box inertia from the node extents; passed with collisionRadius to the Vehicle contact query
    int collisionPointCapacity;  // +0x1c8 slot 2: collisionPointCapacity = a9, the allocation size of collisionPoints (entries zeroed up to it)
    int touchingPointCount;  // +0x1cc active-contact count written by the contact refresh 0x0043ad80 in GameObject slot 10 (0x005036f0) and by the Vehicle contact query; pointsTouching = touchingPointCount > 0; Fn_502c40 compares it with restContactThreshold; slot 13 returns early when 0; cleared by slot 1
    char pointsTouching;  // +0x1d0 GameObject slot 10 (0x005036f0) and Vehicle slot 49: pointsTouching = any contact active (touchingPointCount > 0); slot 7 distributes the force over touching contacts only when set; airborne = !wheels && !pointsTouching
    SoultreeAttachment* attachments;    // +0x1d4 array of 40-byte records
    int attachmentCapacity;                    // +0x1d8 capacity
    int attachmentCount;                    // +0x1dc count
    float stepRemainder;  // +0x1e0 slot 9: stepRemainder = frameTime - n * fixedStepTime (time carried into the next frame); slot 2 clears it
    float fixedStepTime;  // +0x1e4 slot 2's a12; slot 9 divides the frame time into steps of fixedStepTime and uses it as stepTime
    float lastStepTime;  // +0x1e8 Vehicle slot 49: lastStepTime = stepTime after each sub-step
    int maxStepsPerFrame;  // +0x1ec slot 2's a13; slot 9 clamps the number of sub-steps to it
    SoultreeSlot1f0* track;  // +0x1f0 slot 2's a11; slot 18 indexes its +0xa4 block's +0x400 byte table by the contact material id; KrustyBike views the same pointer as its track object
    SoultreeProbe* terrain;         // +0x1f4 tier 2: slot 2 stores a6 here and reads float +0x40 from it;
                                        // slot 21 calls thiscall 0x00506e90 on it; passed as 'obj' to 0x004b0df0
    float terrainScale;                  // +0x1f8 fld/fmul in KrustyBike slot 12 (0x0048de20), tier 1
    char inShadow;  // +0x1fc slot 21: last result of the line-of-sight probe from the attachments' centre (+1.5 up) toward shadowLight; when the result flips, type-1/4 attachments are retinted 0x40/0x80 (blocked) or 0xff and flag 0x800 toggled
    SoultreeLight* shadowLight;    // +0x200 tier 3: light (kind 2 or 4) picked from the node list by slot 21; shadow probe target
    int staggerPhase;  // +0x204 slot 2: staggerPhase = g_SoultreeInstanceCounter (0x00689f14, cycles 0..5 per instance); slot 22 returns staggerCounter == staggerPhase, so the shadow probe runs once every 6 frames per body, staggered
    int staggerCounter;  // +0x208 slot 21: ++staggerCounter, wraps to 0 after 5; slot 2 clears it; compared with staggerPhase by slot 22
    char attachmentResetPending;  // +0x20c set on (re)initialisation; slot 21 clears it and resets every attachment target (+0x60 = 0, type 4 -> 0x004ba340)
    char justReset;  // +0x20d set on (re)initialisation, cleared by the Vehicle step; slot 21 ORs it into each attachment's trail-reset flag (+0x24) and skips moving type-4 emitters while set
    char field_0x20e;
    unsigned char groundProbeMask;  // +0x20f slot 2's a19 (byte); slot 11 passes it as the probe-result AND mask of the placement search 0x004b0df0 (contact/ObjectPlacement.cpp parameter j)
    int restContactThreshold;  // +0x210 slot 2's a18 (ctor default 1); Fn_502c40 runs the rest timer only with at least restContactThreshold - 1 active contacts (touchingPointCount) and sleeps the body when touchingPointCount reaches it
    float restTimer;                  // +0x214 float timer: Fn_502c40 adds stepTime and compares with 0.5/3.0 (tier 1)
    SoultreeObject* centerNode;  // +0x218 child SoultreeObject created by slot 2 when a2 != 0 (AddChild, SetPosition at the bounds centre); its world position becomes centerOfMass
};

#endif
