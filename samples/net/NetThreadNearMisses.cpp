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
// or continue forms all compile identically, as do a function-pointer local
// for Receive and inline accessors for the buffer and the size.
//
// HandleReceivedMessage (0x004afa90) is exact in src/reconstructed/NetThread.cpp:
// the acknowledgement reads the local player into a local before SendRaw and
// the keep-alive holds data in a local across EnterCriticalSection. Tried
// first: a data-pointer or short-pointer local in the acknowledgement (no
// change), inline acknowledgement helpers taking data, the interface or the
// sequence (668 to 697), a sequence local (767) and an interface local (no
// change).

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

long ReceiveMessages(unsigned int time)
{
    char text[256];
    EnterCriticalSection(NET_LOCK(g_TrackGame->network->lock));
    if (!g_TrackGame->network) {
        sprintf(text, "LpGame->LpNetworkInterface invalid = 0x%x \n",
                g_TrackGame->network);
        LeaveCriticalSection(NET_LOCK(g_TrackGame->network->lock));
        return NET_E_INVALIDARG;
    }
    if (!g_TrackGame->network->directPlay) {
        sprintf(text, "LpDirectPlayX invalid = 0x%x\n",
                g_TrackGame->network->directPlay);
        LeaveCriticalSection(NET_LOCK(g_TrackGame->network->lock));
        return NET_E_INVALIDARG;
    }
    unsigned long size = g_UnknownGlobal6886dc;
    for (;;) {
        unsigned long from = 0;
        unsigned long to = 0;
        long result = g_TrackGame->network->directPlay->Receive(
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
    LeaveCriticalSection(NET_LOCK(g_TrackGame->network->lock));
    return 0;
}
