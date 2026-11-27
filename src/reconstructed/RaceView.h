#pragma once

#include "GameObject.h"
#include "MatrixUtil.h"
#include "OverlayRect.h"
#include "Track.h"
#include "VCRfile.h"

// A queued text line (0x8c bytes, TrackOverlay.cpp). Callers build one on
// the stack (constructor 0x0051b200, no destructor) and hand it to
// TextQueueOverlay, which keeps heap copies (copy constructor 0x0051b250).
class TextQueueMessage {
public:
    TextQueueMessage(const char* text, float duration); // 0x0051b200
    TextQueueMessage(const TextQueueMessage& other);       // 0x0051b250

    char line[0x80];                          // +0x00 text
    float showSeconds;                        // +0x80 seconds to show it
    float shownSeconds;                       // +0x84 seconds shown so far
    TextQueueMessage* next;                     // +0x88 next queued line
};

// RTTI: TextQueueOverlay : GameObject (vtable 0x00558760), 0x58 bytes
// (TrackOverlay.cpp): draws the head of a queue of TextQueueMessage lines with
// GDI, then drops it after its duration. EventManager 0x0045d270 calls its
// slot 5. Member names are provisional.
class TextQueueOverlay : public GameObject {
public:
    explicit TextQueueOverlay(int flags);     // 0x0051b2a0
    virtual ~TextQueueOverlay();              // 0x0051b300 (deleting wrapper 0x0051b2e0)
    virtual int Tick(float frameTime);                 // 0x0051b3f0
    virtual int RenderPost3D();               // 0x0051b450

    // 0x0051b320: binds the render target and the text rectangle and creates
    // the font from the GUI's face name (Arial when it has none).
    TextQueueOverlay* Setup(void* target, OverlayRect rect);
    void ReplaceHeadMessage(TextQueueMessage* message); // 0x0051b540: replaces the head
    void AppendMessage(TextQueueMessage* message); // 0x0051b5e0: appends
    void DropHeadMessage();             // 0x0051b670: drops the head

    void* textFont;                           // +0x2c font (DeleteObject)
    OverlayRect textRect;              // +0x30 text rectangle (+0x40 less one)
    OverlayRect shadowRect;            // +0x40 shadow rectangle
    TextQueueMessage* queueHead;                // +0x50 queue head
    unsigned int textColor;                   // +0x54 text colour, 0xff00
};

class KrustyBike;
class BikeRace;
struct RaceGate;
class ChatOverlay;

class D3DIMSoultreeCharacter;
class Wrecker;
class SoultreeObject;
// Keep the local race-view name while using the actual scene-node type.
typedef SoultreeObject UnknownVehiclePart;

// 0x54-byte race status node (RaceStatus.cpp, DebugCalloc'd): one per
// racer, chained through +0x50; the racer points back at it from +0x744.
// +0x1c/+0x28 are the racer's previous and current (leading) positions and
// +0x34 the next gate (a view's +0xc8 list, BikeRace.h) they are tested
// against; +0x38/+0x44 are track positions.
struct RaceStatusNode {
    int creationOrder;                        // +0x00 1-based order of creation
    KrustyBike* racer;                 // +0x04 racer
    float goalDistance;                       // +0x08 progressPos to the finish probe (track races) or to the next gate's centre (gate races), 0x004e6a50
    float gateDistance;                       // +0x0c distance to the next gate (sort key)
    float progressStep;                       // +0x10 progressPos..placedPos distance this frame (0x004e6a50)
    float progressTotal;                      // +0x14 sum of accepted progress steps (gate races: of gate approach)
    int lapOffset;                            // +0x18 added to the lap count when ordering (0x004e5f40): -1 when the placedPos..progressPos distance passes the probe gate
    Vector3 previousPosition;                 // +0x1c previous position
    Vector3 currentPosition;                  // +0x28 current position
    RaceGate* nextGate;                       // +0x34 next gate
    TrackPos placedPos;                       // +0x38 Track::PlaceNear's (or ClosestPointOnTrack's) placement of previousPosition
    TrackPos progressPos;                     // +0x44 PlaceNear's search start; advances only forward to placedPos (0x004e6a50)
    RaceStatusNode* next;              // +0x50 next
};
