// Near-miss NetThread.cpp candidates (D:\aardvark\VC\krusty2\NetThread.cpp),
// kept out of src/reconstructed until they match. The exact functions of the
// file are in src/reconstructed/NetThread.cpp.
//
// ReceiveMessages (0x004af8e0, 338 bytes): same instructions and frame
// except register colouring in the receive loop. Retail loads the buffer
// pointer at the loop head (the realloc back edge enters after that load),
// keeps &size in ebx and calls Receive through ecx; VC6 here loads the
// buffer later and uses eax/ecx, saving the ebx push. Moving the from/to
// declarations, a local buffer copy, a local interface pointer and do/while
// or continue forms all compile identically.
//
// HandleReceivedMessage (0x004afa90, 1252 bytes): frame, branch layout and
// all blocks except two match: the 0x80 acknowledgement (retail holds data in
// edx and the sequence in ax/dx, VC6 here swaps them) and the 0x4b keep-alive
// (retail caches data in esi across EnterCriticalSection and loads the game
// pointer into ecx). Caching data in a typed local fixes 0x4b but rotates the
// registers of every later block. Shapes that mattered: the duplicated
// `to = target` in both arms, the if/else `deliver`, the `routed` copy of data
// after 0x004abd40 and typed locals for the 0xcd/0xcf payloads.

#include <windows.h>
#include <stdio.h>
#include <string.h>

#include "../../src/reconstructed/TrackGame.h"
#include "../../src/reconstructed/Net.h"
#include "../../src/reconstructed/NetThread.h"
#include "../../src/reconstructed/DirectPlayMessages.h"

#define NET_LOCK(cs) ((CRITICAL_SECTION*)&(cs))
#define NET_DPERR_BUFFERTOOSMALL 0x8877001e
#define NET_E_INVALIDARG         0x80070057

static unsigned char* s_ReceiveBuffer;            // receive buffer
static unsigned int g_UnknownGlobal6886dc;              // its size
static int s_DuplicateToggle;                       // duplicate toggle
static int g_UnknownGlobal6886e4;                       // duplicate filter enabled (never written)

long ReceiveMessages(unsigned int time)
{
    char text[256];
    EnterCriticalSection(NET_LOCK(g_UnknownGlobal56e26c->field_0x08->lock));
    if (!g_UnknownGlobal56e26c->field_0x08) {
        sprintf(text, "LpGame->LpNetworkInterface invalid = 0x%x\n",
                g_UnknownGlobal56e26c->field_0x08);
        LeaveCriticalSection(NET_LOCK(g_UnknownGlobal56e26c->field_0x08->lock));
        return NET_E_INVALIDARG;
    }
    if (!g_UnknownGlobal56e26c->field_0x08->directPlay) {
        sprintf(text, "LpDirectPlayX invalid = 0x%x\n",
                g_UnknownGlobal56e26c->field_0x08->directPlay);
        LeaveCriticalSection(NET_LOCK(g_UnknownGlobal56e26c->field_0x08->lock));
        return NET_E_INVALIDARG;
    }
    unsigned long size = g_UnknownGlobal6886dc;
    for (;;) {
        unsigned long from = 0;
        unsigned long to = 0;
        long result = g_UnknownGlobal56e26c->field_0x08->directPlay->Receive(
            &from, &to, 1 /* DPRECEIVE_ALL */, s_ReceiveBuffer, &size);
        if (result == NET_DPERR_BUFFERTOOSMALL) {
            if (s_ReceiveBuffer)
                DebugFree(s_ReceiveBuffer, __FILE__, 221);
            g_UnknownGlobal6886dc = 0;
            s_ReceiveBuffer = (unsigned char*)DebugMalloc(size, __FILE__, 223);
            if (!s_ReceiveBuffer)
                break;
            g_UnknownGlobal6886dc = size;
        } else {
            if (result < 0)
                break;
            if (size >= 4)
                HandleReceivedMessage(s_ReceiveBuffer, size, from, to, time);
        }
    }
    LeaveCriticalSection(NET_LOCK(g_UnknownGlobal56e26c->field_0x08->lock));
    return 0;
}

void HandleReceivedMessage(unsigned char* data, unsigned int size, int from, int to,
                           unsigned int time)
{
    int last = -1;
    NetworkInterface* net = g_UnknownGlobal56e26c->field_0x08;
    if (net->nextMessage == 0) {
        if (net->field_0x38 != 0)
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
        NetMessage* message = &g_UnknownGlobal56e26c->field_0x08->messages[last];
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
    if (g_UnknownGlobal56e26c->field_0x08->connectionMode == 3
        && g_UnknownGlobal56e26c->field_0x08->isHost && from != 0 && (type & 0x20)) {
        int guaranteed;
        g_UnknownGlobal56e26c->field_0x08->StripMessageTrailer(&data, (unsigned long*)&size,
                                                                 &guaranteed, &sequence, &target);
        unsigned char* routed = data;
        if (guaranteed) {
            from = sequence;
            to = target;
        } else {
            to = target;
        }
        if (to == 0 || to == g_UnknownGlobal56e26c->field_0x08->localPlayer)
            deliver = 1;
        else
            deliver = 0;
        if (to != g_UnknownGlobal56e26c->field_0x08->localPlayer) {
            NetIncomingMessage* incoming = new(__FILE__, 414) NetIncomingMessage;
            incoming->SetReceived(routed, size, from, to);
            g_UnknownGlobal56e26c->field_0x08->heldMessages.Add(incoming);
        }
    }

    if (from == 0) {
        if (*(int*)data == DPSYS_HOST)
            g_UnknownGlobal56e26c->field_0x08->isHost = 1;
    } else {
        if (type & 0x80) {
            short ack[2];
            ack[1] = *(short*)(data + 2);
            g_UnknownGlobal56e26c->field_0x08->SendRaw(
                0x40, ack, 4, *(short*)(data + 2),
                g_UnknownGlobal56e26c->field_0x08->localPlayer, from, 0, 0);
        }
        if (type == 0x4b) {
            EnterCriticalSection(NET_LOCK(g_UnknownGlobal56e26c->field_0x08->keepAliveLock));
            NetPlayer* player = g_UnknownGlobal56e26c->field_0x08->FindPlayer(*(int*)(data + 4));
            if (player)
                player->lastKeepAliveTime = UnknownFunction4bfa80();
            LeaveCriticalSection(NET_LOCK(g_UnknownGlobal56e26c->field_0x08->keepAliveLock));
            return;
        } else if (type == 0xcc) {
            if (*(int*)(data + 4) == g_UnknownGlobal56e26c->field_0x08->localPlayer)
                g_UnknownGlobal56e26c->field_0x08->StopKeepAlive();
        } else if (type == 0xcd) {
            NetFileHeader* header = (NetFileHeader*)data;
            NetFile* file = new(__FILE__, 484) NetFile;
            file->BeginReceive(from, header);
        } else if (type == 0xcf) {
            NetFileChunk* chunk = (NetFileChunk*)data;
            for (int i = 0; i < g_UnknownGlobal56e26c->field_0x08->files.m_count; i++) {
                NetFile* file = g_UnknownGlobal56e26c->field_0x08->files.Get(i);
                if (file->sourceId == from)
                    file->ReceiveChunk(from, chunk);
            }
            return;
        } else if (type == 0xce) {
            for (int i = 0; i < g_UnknownGlobal56e26c->field_0x08->files.m_count; i++) {
                NetFile* file = g_UnknownGlobal56e26c->field_0x08->files.Get(i);
                if (file->sourceId == from) {
                    g_UnknownGlobal56e26c->field_0x08->files.Remove(file);
                    delete file;
                }
            }
        }
    }
    if (deliver)
        g_UnknownGlobal56e26c->field_0x08->QueueMessage(data, size, from, to, time);
}
