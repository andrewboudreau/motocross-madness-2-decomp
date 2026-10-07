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
void ReportDirectPlayError(long result, const char* file, int line);

// File statics (tier 2: 0x006886d0..0x006886e4 are referenced only from
// 0x004af680..0x004aff73); 0x006886dc and 0x006886e4 are used only by the
// near misses in samples/net/NetThreadNearMisses.cpp.
static unsigned int s_LastKeepAliveTime;              // time of the previous keep-alive pass
static unsigned int s_KeepAliveTime;              // time of the current pass
static unsigned char* s_ReceiveBuffer;            // receive buffer
static int s_DuplicateToggle;                       // duplicate toggle

int IsKeepAliveRunning(NetworkInterface* net)
{
    EnterCriticalSection(NET_LOCK(net->keepAliveLock));
    int value = net->keepAliveRunning;
    LeaveCriticalSection(NET_LOCK(net->keepAliveLock));
    return value;
}

unsigned int __stdcall ReceiveThreadProc(void* arg)
{
    NetworkInterface* net = (NetworkInterface*)arg;
    HANDLE events[3];
    int index = 0;
    events[0] = net->receiveEvent;
    events[1] = net->keepAliveEvent;
    events[2] = net->quitEvent;
    s_ReceiveBuffer = 0;
    DWORD signalled;
    while ((signalled = WaitForMultipleObjects(3, events, FALSE, INFINITE)) != 2) {
        switch (signalled) {
        case 0: {
            long result = ReceiveMessages(UnknownFunction4bfa80());
            if (result != 0) {
                ReportDirectPlayError(result, __FILE__, 81);
                goto done;
            }
            break;
        }
        case 1:
            index = 0;
            s_LastKeepAliveTime = s_KeepAliveTime = UnknownFunction4bfa80();
            while (IsKeepAliveRunning(net)) {
                ReceiveMessages(UnknownFunction4bfa80());
                if (net->isHost) {
                    EnterCriticalSection(NET_LOCK(net->keepAliveLock));
                    s_KeepAliveTime = UnknownFunction4bfa80();
                    index = 0;
                    NetPlayer* player;
                    while ((player = net->NextPlayer(&index)) != 0) {
                        if (net->localPlayer != player->id
                            && (float)(s_KeepAliveTime - player->lastKeepAliveTime) * 0.001f
                                   > net->keepAliveTimeout) {
                            int message[2];
                            message[1] = player->id;
                            net->Send(0xcc, message, 8, net->localPlayer, 0);
                            net->RemovePlayer(player->id);
                        }
                    }
                    LeaveCriticalSection(NET_LOCK(net->keepAliveLock));
                }
                EnterCriticalSection(NET_LOCK(net->keepAliveLock));
                int message[2];
                message[1] = net->localPlayer;
                net->Send(0x4b, message, 8, net->localPlayer, 0);
                ((NetKeepAliveView*)net)->ResendPending(
                    (float)(s_KeepAliveTime - s_LastKeepAliveTime) / 1000.0f);
                s_LastKeepAliveTime = s_KeepAliveTime;
                float interval = net->keepAliveInterval;
                LeaveCriticalSection(NET_LOCK(net->keepAliveLock));
                Sleep((DWORD)(interval * 1000.0f));
            }
            break;
        }
    }
done:
    EnterCriticalSection(NET_LOCK(net->lock));
    if (s_ReceiveBuffer) {
        DebugFree(s_ReceiveBuffer, __FILE__, 162);
        s_ReceiveBuffer = 0;
    }
    LeaveCriticalSection(NET_LOCK(net->lock));
    _endthreadex(0);
    return 0;
}

int IsSameMessage(void* data, unsigned int size, int from, int type, void* otherData,
                          unsigned int otherSize, int otherFrom, int otherType)
{
    if (size == otherSize && from == otherFrom && type == otherType
        && memcmp(data, otherData, size) == 0)
        return 1;
    s_DuplicateToggle = 0;
    return 0;
}

