// Near-miss NetThread.cpp candidates (D:\aardvark\VC\krusty2\NetThread.cpp),
// kept out of src/reconstructed until they match. The exact functions of the
// file are in src/reconstructed/NetThread.cpp.
//
// ReceiveMessages (0x004af8e0, 338 bytes): 274 of 339 bytes, every instruction
// retail's but one. The buffer pointer is a local that the reallocation
// assigns, which gives retail's registers (buffer in eax, &size in ebx,
// Receive through ecx; 234 before, with the buffer read from the global in
// the argument list). Retail reloads the local once at the loop head and the
// reallocation jumps past that load; VC6 here keeps one load before the loop
// and one after the dispatch.
// Tried for the last instruction: the reload at the loop head with a `goto`
// or a nested retry loop past it (VC6 clones the Receive call for the inner
// back edge, 385 bytes), `for (init;;)`, `while (1)`, a `continue` after the
// reallocation and the reload after the if/else (all 274). Tried before the
// local: a buffer or &size local that is only read (propagated away, 234),
// the buffer passed to the dispatch (106), inline Receive wrappers taking the
// buffer, the size or the interface (234, 194), a local interface pointer, a
// function-pointer local and inline accessors for the buffer and the size.
// Swept after that (1800 renderings, none above 274): the local declared
// without or with a null initialiser, `do`/`while (1)`, the reload as an
// assignment inside the Receive argument list or as the first statement of
// the body, the allocation chained into the global and the local in either
// order or tested inside the `if`, the local in the DebugFree test, and the
// dispatch written as an early `continue` for size < 4 (271 of 349). The loop
// written with labels only (reload/retry gotos, no loop statement) loses the
// local altogether (130 of 385). The two loads VC6 emits are byte-identical
// to retail's one and both sit directly before the join, so this is a loop
// head retail has and the candidate does not, not a missed tail merge.
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
    unsigned char* buffer = s_ReceiveBuffer;
    for (;;) {
        unsigned long from = 0;
        unsigned long to = 0;
        long result = g_TrackGame->network->directPlay->Receive(
            &from, &to, 1 /* DPRECEIVE_ALL */, buffer, &size);
        if (result == NET_DPERR_BUFFERTOOSMALL) {
            if (s_ReceiveBuffer)
                DebugFree(s_ReceiveBuffer, __FILE__, 221);
            g_UnknownGlobal6886dc = 0;
            buffer = (unsigned char*)DebugMalloc(size, __FILE__, 223);
            s_ReceiveBuffer = buffer;
            if (!buffer)
                break;
            g_UnknownGlobal6886dc = size;
        } else {
            if (result < 0)
                break;
            if (size >= 4)
                HandleReceivedMessage(s_ReceiveBuffer, size, from, to, time);
            buffer = s_ReceiveBuffer;
        }
    }
    LeaveCriticalSection(NET_LOCK(g_TrackGame->network->lock));
    return 0;
}
