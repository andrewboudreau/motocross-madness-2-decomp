// Near-miss NetThread.cpp candidates (D:\aardvark\VC\krusty2\NetThread.cpp),
// kept out of src/reconstructed until they match. The exact functions of the
// file are in src/reconstructed/NetThread.cpp.
//
// UnknownFunction4af8e0 (0x004af8e0, 338 bytes): same instructions and frame
// except register colouring in the receive loop. Retail loads the buffer
// pointer at the loop head (the realloc back edge enters after that load),
// keeps &size in ebx and calls Receive through ecx; VC6 here loads the
// buffer later and uses eax/ecx, saving the ebx push. Moving the from/to
// declarations, a local buffer copy, a local interface pointer and do/while
// or continue forms all compile identically.
//
// UnknownFunction4afa90 (0x004afa90, 1252 bytes): frame, branch layout and
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

static unsigned char* g_UnknownGlobal6886d8;            // receive buffer
static unsigned int g_UnknownGlobal6886dc;              // its size
static int g_UnknownGlobal6886e0;                       // duplicate toggle
static int g_UnknownGlobal6886e4;                       // duplicate filter enabled (never written)

long UnknownFunction4af8e0(unsigned int time)
{
    char text[256];
    EnterCriticalSection(NET_LOCK(g_UnknownGlobal56e26c->field_0x08->field_0x48));
    if (!g_UnknownGlobal56e26c->field_0x08) {
        sprintf(text, "LpGame->LpNetworkInterface invalid = 0x%x\n",
                g_UnknownGlobal56e26c->field_0x08);
        LeaveCriticalSection(NET_LOCK(g_UnknownGlobal56e26c->field_0x08->field_0x48));
        return NET_E_INVALIDARG;
    }
    if (!g_UnknownGlobal56e26c->field_0x08->field_0x04) {
        sprintf(text, "LpDirectPlayX invalid = 0x%x\n",
                g_UnknownGlobal56e26c->field_0x08->field_0x04);
        LeaveCriticalSection(NET_LOCK(g_UnknownGlobal56e26c->field_0x08->field_0x48));
        return NET_E_INVALIDARG;
    }
    unsigned long size = g_UnknownGlobal6886dc;
    for (;;) {
        unsigned long from = 0;
        unsigned long to = 0;
        long result = g_UnknownGlobal56e26c->field_0x08->field_0x04->Receive(
            &from, &to, 1 /* DPRECEIVE_ALL */, g_UnknownGlobal6886d8, &size);
        if (result == NET_DPERR_BUFFERTOOSMALL) {
            if (g_UnknownGlobal6886d8)
                DebugFree(g_UnknownGlobal6886d8, __FILE__, 221);
            g_UnknownGlobal6886dc = 0;
            g_UnknownGlobal6886d8 = (unsigned char*)DebugMalloc(size, __FILE__, 223);
            if (!g_UnknownGlobal6886d8)
                break;
            g_UnknownGlobal6886dc = size;
        } else {
            if (result < 0)
                break;
            if (size >= 4)
                UnknownFunction4afa90(g_UnknownGlobal6886d8, size, from, to, time);
        }
    }
    LeaveCriticalSection(NET_LOCK(g_UnknownGlobal56e26c->field_0x08->field_0x48));
    return 0;
}

void UnknownFunction4afa90(unsigned char* data, unsigned int size, int from, int to,
                           unsigned int time)
{
    int last = -1;
    NetworkInterface* net = g_UnknownGlobal56e26c->field_0x08;
    if (net->field_0x40 == 0) {
        if (net->field_0x38 != 0)
            last = 0xff;
        g_UnknownGlobal6886e0 = 0;
    } else {
        last = net->field_0x40 - 1;
    }
    int type;
    if (from == 0)
        type = *(int*)data;
    else
        type = data[0];
    if (last >= 0) {
        NetMessage* message = &g_UnknownGlobal56e26c->field_0x08->field_0x34[last];
        if (UnknownFunction4afa40(data, size, from, type, message->field_0x14,
                                  message->field_0x10, message->field_0x04, message->field_0x00)
            && g_UnknownGlobal6886e4) {
            g_UnknownGlobal6886e0 = 1 - g_UnknownGlobal6886e0;
            if (g_UnknownGlobal6886e0)
                return;
        }
    }

    int sequence = 0;
    int target = 0;
    int deliver = 1;
    if (g_UnknownGlobal56e26c->field_0x08->field_0x18 == 3
        && g_UnknownGlobal56e26c->field_0x08->isHost && from != 0 && (type & 0x20)) {
        int guaranteed;
        g_UnknownGlobal56e26c->field_0x08->UnknownFunction4abd40(&data, (unsigned long*)&size,
                                                                 &guaranteed, &sequence, &target);
        unsigned char* routed = data;
        if (guaranteed) {
            from = sequence;
            to = target;
        } else {
            to = target;
        }
        if (to == 0 || to == g_UnknownGlobal56e26c->field_0x08->field_0x0c)
            deliver = 1;
        else
            deliver = 0;
        if (to != g_UnknownGlobal56e26c->field_0x08->field_0x0c) {
            NetIncomingMessage* incoming = new(__FILE__, 414) NetIncomingMessage;
            incoming->UnknownFunction4aad60(routed, size, from, to);
            g_UnknownGlobal56e26c->field_0x08->field_0x100.Add(incoming);
        }
    }

    if (from == 0) {
        if (*(int*)data == DPSYS_HOST)
            g_UnknownGlobal56e26c->field_0x08->isHost = 1;
    } else {
        if (type & 0x80) {
            short ack[2];
            ack[1] = *(short*)(data + 2);
            g_UnknownGlobal56e26c->field_0x08->UnknownFunction4ad280(
                0x40, ack, 4, *(short*)(data + 2),
                g_UnknownGlobal56e26c->field_0x08->field_0x0c, from, 0, 0);
        }
        if (type == 0x4b) {
            EnterCriticalSection(NET_LOCK(g_UnknownGlobal56e26c->field_0x08->field_0x60));
            NetPlayer* player = g_UnknownGlobal56e26c->field_0x08->UnknownFunction4ac800(*(int*)(data + 4));
            if (player)
                player->field_0x18 = UnknownFunction4bfa80();
            LeaveCriticalSection(NET_LOCK(g_UnknownGlobal56e26c->field_0x08->field_0x60));
            return;
        } else if (type == 0xcc) {
            if (*(int*)(data + 4) == g_UnknownGlobal56e26c->field_0x08->field_0x0c)
                g_UnknownGlobal56e26c->field_0x08->UnknownFunction4ac950();
        } else if (type == 0xcd) {
            NetFileHeader* header = (NetFileHeader*)data;
            NetFile* file = new(__FILE__, 484) NetFile;
            file->UnknownFunction4ab280(from, header);
        } else if (type == 0xcf) {
            NetFileChunk* chunk = (NetFileChunk*)data;
            for (int i = 0; i < g_UnknownGlobal56e26c->field_0x08->field_0x114.m_count; i++) {
                NetFile* file = g_UnknownGlobal56e26c->field_0x08->field_0x114.Get(i);
                if (file->field_0x110 == from)
                    file->UnknownFunction4ab380(from, chunk);
            }
            return;
        } else if (type == 0xce) {
            for (int i = 0; i < g_UnknownGlobal56e26c->field_0x08->field_0x114.m_count; i++) {
                NetFile* file = g_UnknownGlobal56e26c->field_0x08->field_0x114.Get(i);
                if (file->field_0x110 == from) {
                    g_UnknownGlobal56e26c->field_0x08->field_0x114.Remove(file);
                    delete file;
                }
            }
        }
    }
    if (deliver)
        g_UnknownGlobal56e26c->field_0x08->UnknownFunction4acb00(data, size, from, to, time);
}
