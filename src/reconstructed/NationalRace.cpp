#include <stdio.h>

#include "QuarryEvent.h"

#include "DebugAlloc.h"
#include "RaceView.h"
#include "RenderTarget.h"
#include "TrackGame.h"
#include "TrackOverlay.h"

// Reconstruction of D:\aardvark\VC\krusty2\NationalRace.cpp (literal
// __FILE__ at 0x0056e338, used by slots 27 and 29): NationalRace,
// 0x004aa7f0..0x004aaaf2, between MSZoneInterface.cpp (last xref
// 0x004aa64d) and Net.cpp (first xref 0x004aae60). RTTI confirms
// NationalRace : BaseQuarryEvent (vtable 0x00555354) and the overridden
// slots 0, 10, 27, 29 and 30. Member and function names are provisional.

// Krusty3DObjects.h's RunwayLights, as this file uses it. That header and
// QuarryEvent.h cannot be included together (both declare VisualCue), so
// the three members are redeclared here with the same signatures.
struct UnknownRunwayTerrain;
struct UnknownRunwayRacer;

class RunwayLights : public GameObject {
public:
    explicit RunwayLights(int flags);          // 0x0048a5b0
    RunwayLights* UnknownFunction48a600(void* value, int a, int b, UnknownRunwayTerrain* terrain); // 0x0048a600
    void UnknownFunction48ad50(UnknownRunwayRacer* racer); // 0x0048ad50

    float field_0x2c;                          // blink timer
    UnknownRunwayRacer* field_0x30;
    UnknownRunwayTerrain* field_0x34;
    ArcadeObject* field_0x38[5];
};

// 0x004aa7f0
NationalRace::NationalRace(int flags) : BaseQuarryEvent(flags) {
    field_0xa4 = 0;
    field_0xac = 0;
    field_0xa8 = 0;
}

// 0x004aa850
NationalRace* NationalRace::UnknownFunction4aa850(RenderTarget* target, UnknownProgressCallback progress) {
    if (!BaseQuarryEvent::UnknownFunction4de3b0(target, progress))
        return 0;
    if (field_0xa4)
        field_0xa4->UnknownFunction48ad50((UnknownRunwayRacer*)field_0x34->field_0x38);
    return this;
}

// 0x004aa890
int NationalRace::UnknownVirtualSlot27(int value) {
    char message[0x104];

    if (!BaseQuarryEvent::UnknownVirtualSlot27(value))
        return 0;
    field_0xa4 = (new (__FILE__, 53) RunwayLights(g_UnknownGlobal56e26c->mode.field_0x6a8))
                     ->UnknownFunction48a600(field_0x18, (int)field_0x7c, value, (UnknownRunwayTerrain*)field_0x40);
    if (!UnknownFunction469190(field_0xa4, -1)) {
        sprintf(message, "Runway Lights not loaded.\n");
        return 0;
    }
    return 1;
}

// 0x004aa970
void NationalRace::UnknownVirtualSlot29() {
    char name[128];
    char message[128];

    BaseQuarryEvent::UnknownVirtualSlot29();
    g_UnknownGlobal56e26c->UnknownFunction521970(0x913, name, sizeof(name));
    field_0xa8 = new (__FILE__, 71) DropTextOverlay(1);
    field_0xa8 = field_0xa8->UnknownFunction51af00(field_0x18, g_UnknownGlobal56e26c->field_0x3c, 3.0f, 0,
                                                   sizeof(name), name);
    if (!field_0xa8)
        sprintf(message, "Drop Text Overlay not created.\n");
}

// 0x004aaa50
void NationalRace::UnknownVirtualSlot30() {
    BaseQuarryEvent::UnknownVirtualSlot30();
    UnknownFunction469190(field_0xa8, -1);
}

// 0x004aaa70
int NationalRace::UnknownVirtualSlot10(float frameTime) {
    BaseQuarryEvent::UnknownVirtualSlot10(frameTime);
    if (g_UnknownGlobal56e26c->mode.field_0x27f8.field_0x00 && g_UnknownGlobal56e26c->mode.field_0x27f8.field_0x00 != 4 &&
        !g_UnknownGlobal56e26c->uiInteractionBlocked && !field_0xac) {
        if (field_0x34->field_0x38->field_0x7a0 + 1 == g_UnknownGlobal56e26c->mode.field_0x27f8.field_0x20 &&
            field_0x34->field_0x18a) {
            if (field_0xa8)
                field_0xa8->UnknownFunction51b1f0();
            field_0xac = 1;
        }
    }
    return 1;
}
