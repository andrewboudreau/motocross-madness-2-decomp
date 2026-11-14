// NetThread.cpp -- reconstruction of D:\aardvark\VC\krusty2\NetThread.cpp
// (0x004af680..0x004aff73). See NetThread.h, Net.h and docs/NET.md.
// The receive loop 0x004af8e0 is a near miss in
// samples/net/NetThreadNearMisses.cpp.

#include <windows.h>
#include <string.h>
#include <process.h>

#include "TrackGame.h"
#include "Net.h"
#include "NetThread.h"
#include "DirectPlayMessages.h"

#define NET_LOCK(cs) ((CRITICAL_SECTION*)&(cs))

// cdecl 0x004ad5a0 (Net.cpp): reports a DirectPlay error with its source position.
void ReportDirectPlayError(long result, const char* file, int line);

// File statics (tier 2: 0x006886d0..0x006886e4 are referenced only from
// 0x004af680..0x004aff73); 0x006886dc (the buffer's size) is used only by the
// near miss in samples/net/NetThreadNearMisses.cpp.
static unsigned int s_LastKeepAliveTime;              // time of the previous keep-alive pass
static unsigned int s_KeepAliveTime;              // time of the current pass
static unsigned char* s_ReceiveBuffer;            // receive buffer
static int s_DuplicateToggle;                       // duplicate toggle
static int g_UnknownGlobal6886e4;                       // duplicate filter enabled (never written)

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
            long result = ReceiveMessages(ReadClock());
            if (result != 0) {
                ReportDirectPlayError(result, __FILE__, 81);
                goto done;
            }
            break;
        }
        case 1:
            index = 0;
            s_LastKeepAliveTime = s_KeepAliveTime = ReadClock();
            while (IsKeepAliveRunning(net)) {
                ReceiveMessages(ReadClock());
                if (net->isHost) {
                    EnterCriticalSection(NET_LOCK(net->keepAliveLock));
                    s_KeepAliveTime = ReadClock();
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


// 0x004afa90: one received message. Drops every other copy of a repeated
// message while the duplicate filter is on, lets the host of a routed session
// hold messages addressed to other players, acknowledges guaranteed messages
// (0x80), and handles keep-alives (0x4b), removals (0xcc) and file transfers
// (0xcd..0xcf) before queueing the message for the game.
// Shapes that matter: the duplicated `to = target` in both arms, the if/else
// `deliver`, the `routed` copy of data after 0x004abd40 and typed locals for
// the 0xcd/0xcf payloads. Two locals set VC6's eax/ecx/edx rotation of
// expression temporaries: the acknowledgement reads the local player into
// `sender` before the SendRaw call (written in the argument list it takes a
// turn, and data lands in eax with the sequence in dx/ax instead of edx and
// ax/dx), and the keep-alive holds data in `message` across
// EnterCriticalSection (esi). With only the second, every later block was one
// register off.
void HandleReceivedMessage(unsigned char* data, unsigned int size, int from, int to,
                           unsigned int time)
{
    int last = -1;
    NetworkInterface* net = g_TrackGame->network;
    if (net->nextMessage == 0) {
        if (net->ringFull != 0)
            last = 0xff;
        s_DuplicateToggle = 0;
    } else {
        last = net->nextMessage - 1;
    }
    int type;
    if (from == 0)
        type = *(int*)data;
    else
        type = data[0];
    if (last >= 0) {
        NetMessage* message = &g_TrackGame->network->messages[last];
        if (IsSameMessage(data, size, from, type, message->data,
                                  message->size, message->from, message->type)
            && g_UnknownGlobal6886e4) {
            s_DuplicateToggle = 1 - s_DuplicateToggle;
            if (s_DuplicateToggle)
                return;
        }
    }

    int sequence = 0;
    int target = 0;
    int deliver = 1;
    if (g_TrackGame->network->connectionMode == 3
        && g_TrackGame->network->isHost && from != 0 && (type & 0x20)) {
        int guaranteed;
        g_TrackGame->network->StripMessageTrailer(&data, (unsigned long*)&size,
                                                                 &guaranteed, &sequence, &target);
        unsigned char* routed = data;
        if (guaranteed) {
            from = sequence;
            to = target;
        } else {
            to = target;
        }
        if (to == 0 || to == g_TrackGame->network->localPlayer)
            deliver = 1;
        else
            deliver = 0;
        if (to != g_TrackGame->network->localPlayer) {
            NetIncomingMessage* incoming = new(__FILE__, 414) NetIncomingMessage;
            incoming->SetReceived(routed, size, from, to);
            g_TrackGame->network->heldMessages.Add(incoming);
        }
    }

    if (from == 0) {
        if (*(int*)data == DPSYS_HOST)
            g_TrackGame->network->isHost = 1;
    } else {
        if (type & 0x80) {
            short ack[2];
            ack[1] = *(short*)(data + 2);
            int sender = g_TrackGame->network->localPlayer;
            g_TrackGame->network->SendRaw(
                0x40, ack, 4, *(short*)(data + 2), sender, from, 0, 0);
        }
        if (type == 0x4b) {
            unsigned char* message = data;
            EnterCriticalSection(NET_LOCK(g_TrackGame->network->keepAliveLock));
            NetPlayer* player = g_TrackGame->network->FindPlayer(*(int*)(message + 4));
            if (player)
                player->lastKeepAliveTime = ReadClock();
            LeaveCriticalSection(NET_LOCK(g_TrackGame->network->keepAliveLock));
            return;
        } else if (type == 0xcc) {
            if (*(int*)(data + 4) == g_TrackGame->network->localPlayer)
                g_TrackGame->network->StopKeepAlive();
        } else if (type == 0xcd) {
            NetFileHeader* header = (NetFileHeader*)data;
            NetFile* file = new(__FILE__, 484) NetFile;
            file->BeginReceive(from, header);
        } else if (type == 0xcf) {
            NetFileChunk* chunk = (NetFileChunk*)data;
            for (int i = 0; i < g_TrackGame->network->files.m_count; i++) {
                NetFile* file = g_TrackGame->network->files.Get(i);
                if (file->sourceId == from)
                    file->ReceiveChunk(from, chunk);
            }
            return;
        } else if (type == 0xce) {
            for (int i = 0; i < g_TrackGame->network->files.m_count; i++) {
                NetFile* file = g_TrackGame->network->files.Get(i);
                if (file->sourceId == from) {
                    g_TrackGame->network->files.Remove(file);
                    delete file;
                }
            }
        }
    }
    if (deliver)
        g_TrackGame->network->QueueMessage(data, size, from, to, time);
}
