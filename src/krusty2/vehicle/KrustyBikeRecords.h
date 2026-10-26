#pragma once
#include "../math/Vec3.h"

// Value records embedded in the canonical KrustyBike class.
struct UnknownBikeAIPath {
    int count;          // +0x000
    int index;          // +0x004 current segment
    float t;            // +0x008 parameter along the current segment
    Vec3 points[200];   // +0x00c
    float targetSpeed;  // +0x96c 0x00414370 writes 40 or 1000, 0x0040d200 the
                        //        takeoff speed (clamped to >= 40.0f)
    float accelLimitScale;  // +0x970 0x0040eca0 caps the rear wheel's acceleration at it x speedTable[speed] (1.5f when there is no path)
    float lookAhead;    // +0x974
};

struct KbBikeMessage {
    char field_0x00;
    unsigned char finishPosition;  // +0x1  KrustyBike+0x784 finishPosition (0x0049276c)
    short field_0x02;                    // ms since the state's last send
    short lapCount;  // +0x4  KrustyBike+0x7a0 lapCount (0x004926f5)
    short gatesPassed;  // +0x6  KrustyBike+0x790 gatesPassed (0x00492700)
    Vec3 position;                       // +0x08
    float roll, pitch, yaw;              // +0x14
    float lastLapTime;  // +0x20  KrustyBike+0x74c lastLapTime (0x00492746)
    int field_0x24;
    float fastestLap;  // +0x28  KrustyBike+0x750 fastestLap (0x00492719)
    float timeBehindLeader;  // +0x2C  KrustyBike+0x770 timeBehindLeader (0x00492737)
    Vec3 angularVelocity;                // +0x30 (y plus the turn rate)
    Vec3 velocity;                       // +0x3c
    unsigned char poseIndex : 4;         // +0x48
    unsigned char poseState : 4;
    unsigned char poseParam;             // +0x49 x 100
    unsigned char poseBlend;             // +0x4a x 100
    unsigned char poseLeanBlend;         // +0x4b x 100
    unsigned int time;                   // +0x4c
    unsigned char motion : 6;            // +0x50 crash motion (17 without one)
    unsigned char pad_0x50 : 2;
    unsigned char crashDirection : 3;    // +0x51
    unsigned char crashed : 1;
    unsigned char flag4 : 1;             // vehicle slot 51
    unsigned char flag5 : 1;             // +0x7a4
    unsigned char flag6 : 1;             // +0x78c
    unsigned char flag7 : 1;             // +0x478
    unsigned char flag8 : 1;             // +0x52 +0x479
    unsigned char flag9 : 1;             //       +0x153c
    unsigned char lapGate;  // +0x53  KrustyBike+0x7b8 lapGate (0x00492892; +0x7a0 in modes 2 and 3)
    unsigned char loaderKind;  // +0x54  KrustyBike+0x11c0 loaderKind (0x00492723)
};

struct KbBikeNetState : KbBikeMessage {
    unsigned int timeReceived;           // +0x58 local clock when it arrived
    int field_0x5c;                      // +0x5c set when the lap/race fields are valid
};

struct KbBikeState {
    Vec3 position;                       // +0x00
    float roll, pitch, yaw;              // +0x0c
    Vec3 angularVelocity;                // +0x18
    Vec3 velocity;                       // +0x24
    int field_0x30;
    unsigned int time;                   // +0x34
    // What message 13's byte deltas could not carry (0x00492ad0), per value.
    Vec3 velocityError;                  // +0x38
    Vec3 positionError;                  // +0x44
    Vec3 angularVelocityError;           // +0x50
    float rollError;                     // +0x5c
    float pitchError;                    // +0x60
    float yawError;                      // +0x64
    float timer;                         // +0x68 seconds since the last full message
};
