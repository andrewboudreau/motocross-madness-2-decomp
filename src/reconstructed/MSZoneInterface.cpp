// MSZoneInterface.cpp (literal __FILE__ at 0x0056e1b4, xrefs 0x004aa36c..
// 0x004aa64d; code 0x004aa010..0x004aa7e9): the MSN Gaming Zone lobby
// queries and score report of the network-game object at TrackGame+0x3410,
// and the global TrackGame instance. Names are provisional.

#include <string.h>
#include <windows.h>

#include "TrackGame.h"

#include "DebugAlloc.h"

// The per-file vector constants (see src/krusty2/math/Math3D.h):
// 0x00685170, 0x00685180, 0x00685190 and 0x00685160, initialised by
// 0x004aa010..0x004aa14b. The soultree physics inlines that follow them
// (0x004aa150..0x004aa340, kept in samples/physics) read this file's Y axis.
static const Vector3 kVec3Zero = Vector3(0.0f, 0.0f, 0.0f);
static const Vector3 kVec3XAxis = Vector3(1.0f, 0.0f, 0.0f);
static const Vector3 kVec3YAxis = Vector3(0.0f, 1.0f, 0.0f);
static const Vector3 kVec3ZAxis = Vector3(0.0f, 0.0f, 1.0f);

// The GUIDs at 0x00556d80..0x00556def (not defined here): the score
// object's interface and class, the player GUID sent with both queries, the
// application GUID (also used by Net.cpp) and the two property tags.
extern const GUID g_UnknownGuid556d80;
extern const GUID g_UnknownGuid556da0;
extern const GUID g_UnknownGuid556db0;
extern const GUID g_NetApplicationGuid;
extern const GUID g_UnknownGuid556dd0;
extern const GUID g_UnknownGuid556de0;

// DirectX 6 lobby system messages (dplobby.h layout; VC6's headers predate
// them): DPLMSG_GETPROPERTY (type 7) and DPLMSG_GETPROPERTYRESPONSE (type 8).
struct UnknownLobbyGetProperty {
    unsigned long type;
    unsigned long requestId;
    GUID player;
    GUID property;
};

struct UnknownLobbyGetPropertyResponse {
    unsigned long type;
    unsigned long requestId;
    GUID player;
    GUID property;
    long result;
    unsigned long dataSize;
    char data[1];
};

// The COM object 0x004aa670 creates; only the slots it calls are named.
struct UnknownZoneScore {
    virtual long __stdcall QueryInterface(const GUID& iid, void** result);
    virtual unsigned long __stdcall AddRef();
    virtual unsigned long __stdcall Release();
    virtual long __stdcall UnknownMethod3(const GUID* application, unsigned long count, int a, int b,
                                          int c);
    virtual long __stdcall UnknownMethod4(unsigned long index, const char* name, double score,
                                          int a, int flags);
    virtual long __stdcall UnknownMethod5();
    virtual long __stdcall UnknownMethod6(void* a, void* b);
    virtual long __stdcall UnknownMethod7();
    virtual long __stdcall UnknownMethod8();
    virtual long __stdcall UnknownMethod9();
    virtual long __stdcall UnknownMethod10();
    virtual long __stdcall UnknownMethod11();
    virtual long __stdcall UnknownMethod12();
    virtual long __stdcall UnknownMethod13();
    virtual long __stdcall UnknownMethod14();
    virtual long __stdcall UnknownMethod15(UnknownDirectPlayLobby3A* lobby);
};

// 0x004aa350
void UnknownTrackGameObject3410::UnknownFunction4aa350(UnknownDirectPlay4A* a, UnknownDirectPlayLobby3A* b) {
    field_0x00 = a;
    field_0x04 = b;
}

// 0x004aa360 and 0x004aa4e0 (the preset and rank queries) are near misses:
// see samples/net/MSZoneInterfaceNearMisses.cpp.

// 0x004aa670: creates the score object, adds `count` players and sends the
// result through the lobby.
int UnknownTrackGameObject3410::UnknownFunction4aa670(unsigned int count, void* a, void* b) {
    UnknownZoneScore* score = 0;
    unsigned int i;
    int flags;

    if (CoCreateInstance(g_UnknownGuid556da0, 0, CLSCTX_INPROC_SERVER, g_UnknownGuid556d80, (void**)&score) < 0)
        return 0;
    if (score->UnknownMethod3(&g_NetApplicationGuid, count, 0, 0, 0x1000) < 0)
        goto failed;
    UnknownFunction520820("\nSending The RaceStatus to the Zone\n");
    for (i = 0; i < count; i++) {
        if (field_0x08[i].field_0x40 == 1)
            flags = 2;
        else
            flags = field_0x08[i].field_0x40 ? 4 : 16;
        if (score->UnknownMethod4(i, field_0x08[i].field_0x00, field_0x08[i].field_0x44, 0, flags) < 0)
            goto failed;
        UnknownFunction520820("\nPlayer %s:%d,%d\n", field_0x08[i].field_0x00, field_0x08[i].field_0x40,
                              field_0x08[i].field_0x44);
    }
    if (score->UnknownMethod6(a, b) < 0)
        goto failed;
    if (score->UnknownMethod15(field_0x04) < 0)
        goto failed;
    UnknownFunction520820("Sent The Final Score to the Zone successfully\n");
    score->Release();
    return 1;

failed:
    score->Release();
    return 0;
}

// 0x006851a0 and the pointer to it at 0x0056e26c (in .data right after
// this file's strings); the dynamic initializer 0x004aa7b0..0x004aa7e9
// follows the functions above.
TrackGame g_UnknownTrackGame6851a0;
TrackGame* g_UnknownGlobal56e26c = &g_UnknownTrackGame6851a0;
