#pragma once

#include "VehicleCamera.h"

struct BikeCameraRiderView {
    unsigned char field_0x000[0x1a0];
    SoultreeObject* modelNode;            // +0x1a0 the rider model; has a "Head" part
};

// Object at BikeCameraWreckerView+0x40, Wrecker's PhysicsRigidBody
// (src/krusty2/rigidbody/PhysicsBody.h); KrustyBikeCamera slot 34 leads the
// camera by its velocity.
struct BikeCameraPhysicsRigidBodyView {
    unsigned char field_0x000[0x154];
    Vector3 velocity;                    // +0x154 PhysicsBody::velocity
};

// The racer's +0x604 object: the Wrecker (Wrecker.h) lines up at +0x40
// rigidBody (velocity +0x154), +0x44 wreckState (0 idle, 1..3) and +0xb0
// contactThisFrame.
struct BikeCameraWreckerView {
    unsigned char field_0x00[0x40];
    BikeCameraPhysicsRigidBodyView* rigidBody;    // +0x40 KrustyBikeCamera slot 34
    int wreckState;                      // +0x44 BikeCamera slot 75 tests 3
    unsigned char field_0x48[0xb0 - 0x48];
    int contactThisFrame;                // +0xb0 racesnd.cpp 0x004e39b0 plays a fall sound
};

// Object at BikeCamera+0x3b0. It shares VehicleCameraVehicleView's offsets
// (+0x48, +0x3bc, +0x444, +0x460) and adds the fields below; its class is not
// established.
struct BikeCameraBikeView {
    unsigned char field_0x000[0x48];
    float savedRoll;                          // +0x48 krusty2 SoultreePhysicsBaseObject
    unsigned char field_0x04c[0x58 - 0x4c];
    float savedCosRoll;                       // +0x58 slot 51 multiplies it with savedRoll
    unsigned char field_0x05c[0x3bc - 0x5c];
    SoultreeObject* modelNode;            // +0x3bc
    unsigned char field_0x3c0[0x444 - 0x3c0];
    int crashState;                           // +0x444
    unsigned char field_0x448[0x460 - 0x448];
    int crashReason;                          // +0x460
    unsigned char field_0x464[0x5c4 - 0x464];
    BikeCameraRiderView* riderCharacter;   // +0x5c4
    unsigned char field_0x5c8[0x604 - 0x5c8];
    BikeCameraWreckerView* wrecker;          // +0x604 the racer's Wrecker
};

// RTTI: BikeCamera : VehicleCamera. Its code sits near BikeAI.cpp references;
// the translation unit is not established.
class BikeCamera : public VehicleCamera {
public:
    explicit BikeCamera(int flags); // 0x00416e20
    virtual ~BikeCamera();          // 0x00416e70 (deleting wrapper 0x00416e50)

    // 0x00416e80: VehicleCamera's setup (0x0052b9e0); this or 0.
    BikeCamera* SetupBikeCamera(void* context, float fieldOfViewEaseRate, float distanceEaseRate, float initialLookAhead,
                                      float initialFollowHeight, float initialYawOffset, int capacity, int count,
                                      const int* list);

    virtual Vector3 GetStateFiveFollowPoint(float dt);
    virtual Vector3 GetTargetPoint();
    virtual void SteerToKeepTargetOnScreen(float dt);
    virtual Vector3 GetOnboardEyePosition();
    virtual float GetRoll();
    virtual void HideSubject();
    virtual void ShowSubject();
    virtual bool ShouldFollowSubject();

protected:
    BikeCameraBikeView* bike; // +0x3b0
};
