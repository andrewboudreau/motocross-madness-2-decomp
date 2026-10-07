#pragma once

// TrackOverlay.cpp (literal __FILE__ "D:\aardvark\VC\krusty2\TrackOverlay.cpp",
// 0x00575290): the in-race overlays. Class names come from RTTI; every
// member and function name is provisional (docs/TRACKOVERLAY.md).

#include "Overlay.h"
#include "MatrixUtil.h"
#include "RaceView.h"

class TextureMapManager;

// 0x0050a590 (Texmap.cpp, cdecl): loads the texture file `name`; 0 on failure.
TextureMap* UnknownFunction50a590(TextureMapManager* manager, const char* name, int format,
                                  int a4, int a5, int a6, int a7, int a8, int a9,
                                  unsigned int key, int a11, int a12);

// A rectangle built from its two horizontal and two vertical edges
// (0x00518d50, 0x00518d60). It has the same layout as UnknownOverlayRect.
struct UnknownTrackOverlayRect : public UnknownOverlayRect {
    UnknownTrackOverlayRect();                                    // 0x00518d50
    UnknownTrackOverlayRect(int left, int right, int top, int bottom); // 0x00518d60
};

// The object InstrumentOverlay+0x184 points to (its +0x3b4 is read the same
// way as KrustyBikeCamera+0x3b4); only that pointer is known.
struct UnknownInstrumentState {
    unsigned char field_0x000[0xbc];
    float gaugeValue;                         // the gauge value before scaling
    unsigned char field_0x0c0[0x108 - 0xc0];
    bool field_0x108;
    unsigned char field_0x109[0x736 - 0x109];
    unsigned char field_0x736;
    unsigned char field_0x737[0x74c - 0x737];
    float lastLapTime;                        // last lap time (StatsOverlay 0x0051a560)
    float field_0x750;
    float field_0x754;                        // a time (StatsOverlay 0x0051a480)
    unsigned char field_0x758[0x770 - 0x758];
    float field_0x770;
    unsigned char field_0x774[0x784 - 0x774];
    int position;                             // position
    unsigned char field_0x788[0x7a0 - 0x788];
    unsigned short lapsDone;                  // laps done
    unsigned char field_0x7a2[0x7a4 - 0x7a2];
    char field_0x7a4;
    unsigned char field_0x7a5[0x7b8 - 0x7a5];
    int lapGate;                              // gate of the lap
};

struct UnknownInstrumentSource {
    unsigned char field_0x000[0x3b4];
    UnknownInstrumentState* state;
};

// A dial needle (InstrumentOverlay+0x11c), 0x28 bytes.
struct UnknownGauge {
    int field_0x00;
    int field_0x04;
    int centerX;                              // centre x
    int centerY;                              // centre y
    float length;                             // needle length
    float value;                              // value
    float maximum;                            // maximum value
    float firstSweepValue;                    // value at the end of the first sweep
    float startAngle;                         // start angle, degrees
    float degreesPerUnit;                     // degrees per field_0x1c beyond the first sweep
};

// RTTI: InstrumentOverlay : Overlay (vtable 0x00558590), 0x188 bytes.
class InstrumentOverlay : public Overlay {
public:
    explicit InstrumentOverlay(int flags);    // 0x00518720
    virtual ~InstrumentOverlay();             // 0x00518c00 (deleting wrapper 0x00518750)
    virtual int UnknownVirtualSlot14();       // 0x00518940

    InstrumentOverlay* UnknownFunction518770(RenderTarget* target, TextureMapManager* manager, UnknownInstrumentSource* a3); // 0x00518770
    void PlaceNeedle(UnknownGauge* gauge, UnknownOverlayVertex* vertices);           // 0x00518c60
    void PlaceNeedleTip(UnknownGauge* gauge, UnknownOverlayVertex* vertex);          // 0x00518cc0

    UnknownGauge dial;
    UnknownOverlayVertex dialNeedle[2];       // the needle, a line from tip to centre
    UnknownInstrumentSource* instrumentSource;
};

// 0x00506e90 (thiscall, ret 0x18): casts the segment from->to against the
// terrain and returns nonzero on a hit (point in *out). Its class (krusty2
// Terrain) is not reconstructed under src/reconstructed.
class UnknownTerrain {
public:
    int UnknownFunction506e90(const Vector3* from, const Vector3* to, Vector3* out, int a4, int a5, int a6);
    // 0x00507960 (ret 4): selects the detail level (QuarryStuntEvent.cpp;
    // Terrain::SelectQuality in src/krusty2/broadphase/Terrain.h).
    void UnknownFunction507960(int level);
};

class Camera;

// The object at 0x00575a98. 0x0052f340 (thiscall) projects `point` with the
// camera and its matrix (+0xec); nonzero when it is on screen.
class UnknownProjector {
public:
    int UnknownFunction52f340(Camera* camera, const void* matrix, const Vector3* point,
                              Vector3* screen, float* depth);
};
extern UnknownProjector* g_UnknownGlobal575a98;

// NameOverlay+0x12c: only +0x4c (the terrain) is read.
struct UnknownNameOverlayWorld {
    unsigned char field_0x00[0x4c];
    UnknownTerrain* terrain;
};

// RTTI: NameOverlay : Overlay (vtable 0x0055860c), 0x174 bytes: a rider's
// name tag that follows a point on screen and fades when it is hidden.
class NameOverlay : public Overlay {
public:
    explicit NameOverlay(int flags);          // 0x00518d80
    virtual ~NameOverlay();                   // 0x00518e20 (deleting wrapper 0x00518e00)
    virtual int UnknownVirtualSlot14();       // 0x005190e0

    NameOverlay* UnknownFunction518e30(RenderTarget* target, TextureMap* texture,
                                       const UnknownOverlayRect* rect, int a4,
                                       const UnknownOverlayRect* source, UnknownEventRacer* a6,
                                       UnknownNameOverlayWorld* a7, int a8); // 0x00518e30
    int IsPointVisible(const Vector3* point);                           // 0x00519000
    void UnknownFunction519080(Vector3 point);                // 0x00519080
    void UnknownFunction5190a0(Vector3 point, int a4, int a5); // 0x005190a0

    UnknownEventRacer* trackedRacer;
    int sourceWidth;                          // source width
    int field_0x124;
    int sourceHeight;                         // source height
    UnknownNameOverlayWorld* world;
    int testPhase;                            // phase of the every-fourth-frame test
    int frameCounter;                         // frame counter, 0..3
    int lastVisible;                          // last visibility result
    int field_0x13c;
    int field_0x140;
    float fadeTime;                           // fade time, 0.25
    float field_0x148;
    int field_0x14c;
    Vector3 field_0x150;
    Vector3 field_0x15c;
    int field_0x168;
    int field_0x16c;
    int field_0x170;
};

// A GDI SIZE.
struct UnknownTextExtent {
    int cx;
    int cy;
};

// StatsOverlay+0x124 (set by 0x00519880): a view whose racers 0x0051a560
// counts (0x004204e0, as UnknownKrustyBikeView in RaceView.h); +0x1b8 is a
// time (0x0051a480).
struct UnknownStatsSource {
    UnknownEventRacer* UnknownFunction4204e0(int* iterator); // 0x004204e0

    unsigned char field_0x000[0x38];
    UnknownEventRacer* ownRacer;              // its own racer
    unsigned char field_0x03c[0x1b8 - 0x3c];
    float field_0x1b8;
};

// RTTI: StatsOverlay : Overlay (vtable 0x00558680), 0x9b8 bytes: the race
// statistics panel, redrawn about once a second for the view mode
// (TrackGame +0x2d74).
class StatsOverlay : public Overlay {
public:
    explicit StatsOverlay(int flags);         // 0x00519370
    virtual ~StatsOverlay();                  // 0x00519420 (deleting wrapper 0x00519400)
    virtual int UnknownVirtualSlot10(float frameTime); // 0x00519900
    virtual int UnknownVirtualSlot13();       // 0x00519940
    virtual int UnknownVirtualSlot14();       // 0x00519980

    // 0x005194b0 (QuarryStuntEvent.cpp 0x004e0a9c): loads the panel for the
    // screen rectangle `screen`; returns this, or 0.
    StatsOverlay* UnknownFunction5194b0(RenderTarget* target, TextureMapManager* manager, void* camera,
                                        UnknownOverlayRect screen);
    // 0x00464e80: the shared empty `ret 4` body (QuarryStuntEvent.cpp
    // 0x004de4a3 passes the visual cue index).
    void UnknownFunction464e80(int index);
    int UnknownFunction519880(int value);     // 0x00519880
    int Redraw();                             // 0x005198a0
    int UnknownFunction519a20();              // 0x00519a20
    void DrawTimeLeft();                      // 0x00519e10
    int UnknownFunction519ef0();              // 0x00519ef0
    void DrawLapTime();                       // 0x0051a480
    int DrawRacePanel();                      // 0x0051a560
    int DrawTagStandings();                   // 0x0051aa40

    void* field_0x11c;                        // GDI object (DeleteObject)
    void* field_0x120;                        // GDI object (DeleteObject)
    UnknownStatsSource* statsSource;          // set by 0x00519880; read when TrackGame+0x3428 is set
    UnknownInstrumentSource* instrumentSource;
    float timeSinceRedraw;                    // seconds since the last redraw
    UnknownTrackOverlayRect rowRects[8];
    char field_0x1b0[8][0x80];
    char rowText[8][0x80];                    // the text drawn into field_0x130[i]
    UnknownTextExtent rowExtent;              // extent of the first row (GetTextExtentPoint32A)
};

// 0x005199f0 (cdecl): qsort comparator for UnknownEventScore records
// (EventManager.h), larger value first.
int UnknownFunction5199f0(const void* a, const void* b);

// RTTI: DropTextOverlay : GameObject (vtable 0x005586f0), 0xc8 bytes: one
// line of text drawn with GDI and a one-pixel black drop shadow.
class DropTextOverlay : public GameObject {
public:
    explicit DropTextOverlay(int flags);      // 0x0051ae80
    virtual ~DropTextOverlay();               // 0x0051aee0 (deleting wrapper 0x0051aec0)
    virtual int UnknownVirtualSlot10(float frameTime); // 0x0051b070 (not reconstructed)
    virtual int UnknownVirtualSlot15();       // 0x0051b0f0

    void UnknownFunction51b1f0();             // 0x0051b1f0: shows the text again
    // 0x0051af00 (ret 0x18): slot 8 on `value`, then sets up the text (size
    // characters) with the game's texture manager; returns this or 0
    // (NationalRace.cpp is the caller).
    DropTextOverlay* UnknownFunction51af00(void* value, TextureMapManager* textures, float scale, int a, int size,
                                           const char* text);

    void* textFont;                           // font (DeleteObject)
    float field_0x30;                         // 1.0
    int visible;                              // shown
    int field_0x38;
    int textX;                                // text x
    int field_0x40;
    int textY;                                // text y
    char caption[0x80];                       // text
};

// RTTI: RadarOverlay : Overlay (vtable 0x005587d0): the radar map in a
// screen corner (radar128.tga, or radar256.tga above 800 pixels) with the
// racers on it, and an optional frame-rate readout.
// The camera RadarOverlay's loader is given (KrustyBikeCamera; see
// RaceSound.h's view of it): only the followed racer at +0x3b0 is read.
struct UnknownRadarCamera {
    unsigned char field_0x000[0x3b0];
    UnknownEventRacer* followedRacer;
};

class RadarOverlay : public Overlay {
public:
    explicit RadarOverlay(int flags);         // 0x0051b690
    virtual ~RadarOverlay();                  // 0x0051b7d0 (deleting wrapper 0x0051b7b0)
    virtual int UnknownVirtualSlot10(float frameTime); // 0x0051bcd0
    virtual int UnknownVirtualSlot14();       // 0x0051bd20
    virtual int UnknownVirtualSlot23(UnknownControlEvent* event, UnknownInputEntry* entry); // 0x0051bb60

    // 0x0051b840: loads the map for the screen rectangle `screen` (only its
    // top and right edges are read).
    RadarOverlay* UnknownFunction51b840(RenderTarget* target, TextureMapManager* manager, int a3,
                                        UnknownOverlayRect screen);
    int UnknownFunction51baf0(UnknownKrustyBikeView* view, int a2); // 0x0051baf0: collects the racers
    int DrawForViewMode();                    // 0x0051bc60: draws for the view mode
    void DrawFrameRate();                     // 0x0051bdc0: frame-rate readout
    // 0x0051bed0: places every racer's name tag and draws its map dot, then
    // the track outline (modes 2, 3) or the gates (modes 1, 5).
    void DrawRacers(int mode);
    // 0x0051cb20: draws the current and the next gate (the race's +0xb0 and
    // +0xb4) as a line between their posts, the current one in yellow.
    void DrawGates();
    // 0x0051c720: draws the track outline (mode 1 or 2 picks the segment
    // edge), clipped to the map circle (near miss).
    void DrawTrackOutline(int mode);
    // 0x0051c360: maps the world point `point` to the map pixel (*x, *y);
    // when it lies beyond the map radius, returns 1 with the rim point in
    // (*rimX, *rimY).
    int WorldToMap(Vector3 point, int* x, int* y, float* rimX, float* rimY);
    // 0x0051c460: the line through a and b (x, y only) as a unit normal
    // (*nx, *ny) and offset *d; -1 when a and b coincide.
    int LineThrough(const float* a, const float* b, float* nx, float* ny, float* d);
    // 0x0051c4f0: intersects the line through a and b with the circle
    // (circle[0], circle[1]) of radius circle[2]. Returns -1 for no
    // intersection (or a degenerate line), 1 for a tangent point and 2 for
    // the intersection nearer to `point`, written to `point`; 3 when the
    // direction vanishes. `a4` is not read.
    int IntersectLineCircle(const float* a, const float* b, const float* circle, float* a4, float* point);

    float frameTimeSum;                       // frame time summed
    float frameCount;                         // frames summed
    float averageFrameTime;                   // average frame time
    float frameRateSum;                       // frame rate summed
    UnknownKrustyBikeView* radarView;
    UnknownRadarCamera* radarCamera;
    UnknownEventRacer* racers[11];
    int racerCount;                           // racer count
    int showFrameRate;                        // shows the frame rate
    int readoutCount;                         // readouts so far
    int field_0x16c;
    int field_0x170;
    int field_0x174;
    int largeMap;                             // uses the 256-pixel map
    Vector3 field_0x17c;
    float field_0x188;
    float field_0x18c;
    float field_0x190;
    float field_0x194;
    float mapScale;                           // map scale
    float mapRadius;                          // map radius, pixels
    int mapCenterX;                           // map centre
    int mapCenterY;
    UnknownOverlayRect frameRateRect;         // frame-rate text rectangle
    UnknownOverlayVertex field_0x1b8;
    UnknownOverlayVertex field_0x1d8;
    UnknownOverlayRect mapRect;               // map rectangle
};

// The race object at TrackGame+0x560 as RadarOverlay's gate drawing reads
// it (bikerace.cpp sees the same records as UnknownBikeRaceNodeOwner in
// BikeRace.h): +0xac gates, +0xb0/+0xb4 the current and next gate, +0xd8 the
// gate records (position, direction) and +0x420 one entry per gate, whose
// +0x30 enables drawing and whose first +0x50 is the half width.
struct UnknownRadarGate {
    Vector3 position;
    Vector3 direction;
};

struct UnknownRadarGateEntry {
    unsigned char field_0x00[0x30];
    int drawEnabled;
    unsigned char field_0x34[0x50 - 0x34];
    float halfWidth;
};

struct UnknownRadarGateOwner {
    unsigned char field_0x000[0xac];
    int gateCount;
    int currentGate;
    int nextGate;
    unsigned char field_0x0b8[0xd8 - 0xb8];
    UnknownRadarGate gates[0x23];
    UnknownRadarGateEntry* gateEntries[1];
};

// What ChatOverlay+0x16c lists, one per name tag; only these fields are read.
struct UnknownChatRacerState {
    unsigned char field_0x000[0x14c];
    int hideNameTag;                          // hides the name tag
};

struct UnknownChatRacer {
    unsigned char field_0x000[0x3bc];
    UnknownChatRacerState* racerState;
    unsigned char field_0x3c0[0x4a4 - 0x3c0];
    float field_0x4a4;                        // shown when TrackOverlay's 0x0068a444 is set
    unsigned char field_0x4a8[0x5e0 - 0x4a8];
    char name[0x10];                          // name
    unsigned char field_0x5f0[0x784 - 0x5f0];
    int field_0x784;                          // ChatOverlay+0x19c keeps the last value
};

// ChatOverlay+0x128 and +0x12c: only these fields are read.
struct UnknownChatCamera {
    unsigned char field_0x000[0x3b4];
    UnknownChatRacer* followedRacer;
};

struct UnknownChatView {
    // 0x004204e0: the next racer after `*iterator` (advancing it), or 0
    // (UnknownKrustyBikeView::UnknownFunction4204e0 in RaceView.h).
    UnknownChatRacer* UnknownFunction4204e0(int* iterator);

    unsigned char field_0x00[0x38];
    UnknownChatRacer* ownRacer;
};

// One chat history line (0x5c bytes).
struct UnknownChatEntry {
    int value;
    char text[0x58];
};

// Network message 0x85 (0x4e bytes sent): a chat line. Its first field is
// never set by the sender (0x0051da30).
struct UnknownChatMessage {
    int field_0x00;
    char text[0x4a];
};

// ChatOverlay+0x13c (0x1c4 bytes, no vtable): the line being typed (up to
// 0x4a characters) and four history entries.
class UnknownChatInput {
public:
    UnknownChatInput();                       // 0x0051ea50
    void AppendChar(char c);                  // 0x0051ea80: appends c
    void Backspace();                         // 0x0051eab0: removes the last character
    void ClearLine();                         // 0x0051ead0: clears the line
    char* GetLine();                          // 0x0051eae0: the line (a shared `mov eax, ecx` body)
    char* GetLineTail();                      // 0x0051eaf0: the last 0x31 characters
    char* GetHistory(int index, int* value);            // 0x0051eb10: history entry
    // 0x0051eb40: adds "name: text" with `value` to the history (three
    // lines; the oldest is dropped), and the rest of a long text as an
    // indented second line.
    void AddHistory(const char* name, const char* text, int value);

    char typedLine[0x4c];                     // the line being typed
    UnknownChatEntry history[4];
    int typedLength;                          // length of field_0x000
    int field_0x1c0;
};

// RTTI: ChatOverlay : Overlay (vtable 0x00558854), 0x3e0 bytes (the
// allocation at 0x004e08e4). Its +0x2d8 holds the racers' name tags.
class ChatOverlay : public Overlay {
public:
    explicit ChatOverlay(int flags);          // 0x0051cda0
    virtual ~ChatOverlay();                   // 0x0051cee0 (deleting wrapper 0x0051cec0)
    virtual int UnknownVirtualSlot10(float frameTime); // 0x0051e200
    virtual int UnknownVirtualSlot13();       // 0x0051e240
    virtual int UnknownVirtualSlot14();       // 0x0051e390

    // 0x0051cf80 (QuarryStuntEvent.cpp 0x004e0965): sets the overlay up for
    // the screen and the cue rectangle; returns this, or 0.
    ChatOverlay* UnknownFunction51cf80(RenderTarget* target, TextureMapManager* manager, void* camera,
                                       UnknownOverlayRect screen, UnknownOverlayRect cue);
    // 0x0051d730 (QuarryStuntEvent.cpp 0x004e02c7): collects the racers of
    // `view` and gives each a name tag; in view mode 0 one more.
    int UnknownFunction51d730(UnknownChatView* view);
    void UnknownFunction51d980(int show);     // 0x0051d980
    // 0x0051da30 (bikerace.cpp 0x0041f5b1): a typed key; Enter sends the
    // line as network message 0x85, Escape sets *result.
    int UnknownFunction51da30(int key, int* result);
    void UnknownFunction51d9c0(float value, const char* name); // 0x0051d9c0
    // 0x0051dce0: called before GameObject slot 23 with the same event.
    int UnknownFunction51dce0(UnknownControlEvent* event, UnknownInputEntry* entry, int* result);
    void UnknownFunction51dd10();             // 0x0051dd10: shows the input line
    void UnknownFunction51dd40();             // 0x0051dd40: hides it
    void UnknownFunction51dd70(const char* name, int a2, int a3); // 0x0051dd70: a2 is the text (0x0051eb40)
    // 0x0051de10: redraws the input line and the history; 0 when the
    // surface's device context is not available.
    int RedrawChat();
    void DrawNameTag(void* dc, int index);           // 0x0051e3f0: draws name tag `index`
    void UnknownFunction51e7c0();             // 0x0051e7c0
    void RedrawNameLine();                    // 0x0051e800: redraws the name line
    void UnknownFunction51e910(int index);    // 0x0051e910: redraws one name tag, or all (-1)

    void* field_0x11c;                        // GDI object (DeleteObject)
    void* field_0x120;                        // GDI object (DeleteObject)
    void* field_0x124;                        // GDI object (DeleteObject)
    UnknownChatCamera* chatCamera;
    UnknownChatView* chatView;
    float timeSum;                            // time summed
    int largeLayout;                          // the large layout (set by 0x0051cf80)
    int inputShown;                           // input line shown
    UnknownChatInput* chatInput;
    UnknownOverlayRect chatRect;              // chat rectangle (set by 0x0051cf80)
    UnknownOverlayRect cueRect;               // cue rectangle; left/top place the extra name tag (0x0051d730)
    int field_0x160;
    int field_0x164;
    int nameChanged;                          // name changed
    UnknownChatRacer* racers[11];
    int field_0x198;                          // name tag index
    int lastRacerValues[11];                  // field_0x16c[i]->field_0x784 last seen
    Overlay* field_0x1c8;                     // side panels on wide screens (0x0051cf80)
    Overlay* field_0x1cc;
    float field_0x1d0;
    char playerName[0x104];                   // name
    NameOverlay* nameTags[13];
    UnknownTrackOverlayRect nameTagRects[13];
    int field_0x3dc;
};
