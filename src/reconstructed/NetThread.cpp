// NetThread.cpp -- reconstruction of D:\aardvark\VC\krusty2\NetThread.cpp
// (0x004af680..0x004aff73). See NetThread.h, Net.h and docs/NET.md.
// The receive loop 0x004af8e0 and the dispatcher 0x004afa90 are near misses
// in samples/net/NetThreadNearMisses.cpp.

#include <windows.h>
#include <string.h>
#include <process.h>

#include "TrackGame.h"
#include "Net.h"
#include "NetThread.h"

#define NET_LOCK(cs) ((CRITICAL_SECTION*)&(cs))

// cdecl 0x004ad5a0 (Net.cpp): reports a DirectPlay error with its source position.
void UnknownFunction4ad5a0(long result, const char* file, int line);

// File statics (tier 2: 0x006886d0..0x006886e4 are referenced only from
// 0x004af680..0x004aff73); 0x006886dc and 0x006886e4 are used only by the
// near misses in samples/net/NetThreadNearMisses.cpp.
static unsigned int g_UnknownGlobal6886d0;              // time of the previous keep-alive pass
static unsigned int g_UnknownGlobal6886d4;              // time of the current pass
static unsigned char* g_UnknownGlobal6886d8;            // receive buffer
static int g_UnknownGlobal6886e0;                       // duplicate toggle

int UnknownFunction4af680(NetworkInterface* net)
{
    EnterCriticalSection(NET_LOCK(net->field_0x60));
    int value = net->field_0x78;
    LeaveCriticalSection(NET_LOCK(net->field_0x60));
    return value;
}

unsigned int __stdcall UnknownFunction4af6a0(void* arg)
{
    NetworkInterface* net = (NetworkInterface*)arg;
    HANDLE events[3];
    int index = 0;
    events[0] = net->field_0x24;
    events[1] = net->field_0x2c;
    events[2] = net->field_0x28;
    g_UnknownGlobal6886d8 = 0;
    DWORD signalled;
    while ((signalled = WaitForMultipleObjects(3, events, FALSE, INFINITE)) != 2) {
        switch (signalled) {
        case 0: {
            long result = UnknownFunction4af8e0(UnknownFunction4bfa80());
            if (result != 0) {
                UnknownFunction4ad5a0(result, __FILE__, 81);
                goto done;
            }
            break;
        }
        case 1:
            index = 0;
            g_UnknownGlobal6886d0 = g_UnknownGlobal6886d4 = UnknownFunction4bfa80();
            while (UnknownFunction4af680(net)) {
                UnknownFunction4af8e0(UnknownFunction4bfa80());
                if (net->isHost) {
                    EnterCriticalSection(NET_LOCK(net->field_0x60));
                    g_UnknownGlobal6886d4 = UnknownFunction4bfa80();
                    index = 0;
                    NetPlayer* player;
                    while ((player = net->UnknownFunction4ac7c0(&index)) != 0) {
                        if (net->field_0x0c != player->field_0x00
                            && (float)(g_UnknownGlobal6886d4 - player->field_0x18) * 0.001f
                                   > net->field_0x7c) {
                            int message[2];
                            message[1] = player->field_0x00;
                            net->UnknownFunction4ac830(0xcc, message, 8, net->field_0x0c, 0);
                            net->UnknownFunction4aca80(player->field_0x00);
                        }
                    }
                    LeaveCriticalSection(NET_LOCK(net->field_0x60));
                }
                EnterCriticalSection(NET_LOCK(net->field_0x60));
                int message[2];
                message[1] = net->field_0x0c;
                net->UnknownFunction4ac830(0x4b, message, 8, net->field_0x0c, 0);
                ((UnknownNetKeepAliveView*)net)->UnknownFunction4acc50(
                    (float)(g_UnknownGlobal6886d4 - g_UnknownGlobal6886d0) / 1000.0f);
                g_UnknownGlobal6886d0 = g_UnknownGlobal6886d4;
                float interval = net->field_0x80;
                LeaveCriticalSection(NET_LOCK(net->field_0x60));
                Sleep((DWORD)(interval * 1000.0f));
            }
            break;
        }
    }
done:
    EnterCriticalSection(NET_LOCK(net->field_0x48));
    if (g_UnknownGlobal6886d8) {
        operator delete(g_UnknownGlobal6886d8, __FILE__, 162);
        g_UnknownGlobal6886d8 = 0;
    }
    LeaveCriticalSection(NET_LOCK(net->field_0x48));
    _endthreadex(0);
    return 0;
}

int UnknownFunction4afa40(void* data, unsigned int size, int from, int type, void* otherData,
                          unsigned int otherSize, int otherFrom, int otherType)
{
    if (size == otherSize && from == otherFrom && type == otherType
        && memcmp(data, otherData, size) == 0)
        return 1;
    g_UnknownGlobal6886e0 = 0;
    return 0;
}

