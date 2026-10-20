// Near-miss Net.cpp candidates, kept out of src/reconstructed until they
// match. See docs/NET.md.
//
// NetworkInterface::ConnectUsingLobby (0x004ab960, 989 bytes): about
// 15%; the candidate is 1072 bytes. Retail keeps the constant zero in ebx
// and tail-merges the error paths; VC6 here keeps `connection` in ebx and
// emits each NET_ERROR_TEXT expansion separately.
//
// NetworkInterface::DispatchMessages (0x004aced0, 370 bytes): 361/370.
// Addressing the slot through a byte offset (MessageAt below) fixes the
// SIB base/index order everywhere (was 355/370 with messages[index]). The
// rest is in the system-message block: retail loads [edx+edi+4] (from) and
// then forms the pointer with `lea ebx,[edx+edi]`; VC6 here forms it first
// (`mov ecx,[esi+0x34]; mov ebx,edi; add ebx,ecx`) and pushes system[2]
// from edx instead of eax. Taking the pointer only inside the `if (!from)`
// (through offset or index) gives retail's load+lea but swaps ebx and ebp
// for the index and the pointer (197/370); local declaration order, name
// buffer order, `last != index`, a split increment, removing the `system`
// local or the `message` pointer, per-case `system` locals and a loop-scope
// pointer declared outside the `if` do not change that swap. Using offset in
// the pointer itself (`&MessageAt(..)` or a cast) outside the `if` gives a
// 368-byte form; a reference for the whole loop body is far worse. Sweeps of
// the in-`if` pointer form (144 combinations: pointer and test forms, the
// `system` local, the switch operand, increment forms and the `more` operand
// order) keep the ebx/ebp swap (best 197) and leave the original at 361.
// Also unchanged or worse: `value` reused as `last`, an inverted head test
// (326), a `NetMessage* base` local (83), `++index >= 0x100`, a split
// increment with `== 0x100` (360) and an `== 0xff` reset (318/372).
// Testing `MessageAt(messages, offset).from` and taking the pointer inside
// the `if` (or using no pointer) is identical to retail once registers are
// masked (0 hunks, 139 aligned): only the ebx/ebp swap is left, so VC6 here
// weights the index above the message web, where retail weights it below.
// Pointer scope, `register`, const, a reference, per-use MessageAt and a
// pointer shared with the 0x40 block do not change the weights.
#include <windows.h>
#include <stdio.h>
#include <string.h>

#include "../../src/reconstructed/TrackGame.h"
#include "../../src/reconstructed/Net.h"
#include "../../src/reconstructed/DirectPlayMessages.h"

#define NET_LOCK(cs) ((CRITICAL_SECTION*)&(cs))
#define NET_DPERR_BUFFERTOOSMALL 0x8877001e
#define NET_E_OUTOFMEMORY        0x8007000e

void ReportDirectPlayError(long result, const char* file, int line);

extern "C" const GUID IID_IDirectPlay4A;                // 0x005567b0
extern "C" const GUID IID_IDirectPlayLobby3A;           // 0x00556870

extern "C" long __stdcall DirectPlayLobbyCreateA(GUID* provider,
                                                 UnknownDirectPlayLobby3A** lobby,
                                                 void* outer, void* data,
                                                 unsigned long dataSize);

// Copies a literal error text into `error`, truncated to 255 characters.
#define NET_ERROR_TEXT(error, text)                                      \
    {                                                                    \
        int length = strlen(text);                                       \
        int count = length > 255 ? 255 : length;                         \
        strncpy(error, text, count);                                     \
        error[count] = 0;                                                \
    }

// 0x004ab960: connects with the settings of a lobby that launched the game.
long NetworkInterface::ConnectUsingLobby()
{
    UnknownDirectPlayLobby3A* baseLobby = 0;
    UnknownDirectPlay4A* baseDirectPlay = 0;
    NetConnection* connection = 0;
    UnknownDirectPlay4A* directPlay4 = 0;
    unsigned long size;
    unsigned long flags;
    char error[256];
    long result;

    strcpy(error, "");
    result = DirectPlayLobbyCreateA(0, &baseLobby, 0, 0, 0);
    if (result < 0) {
        NET_ERROR_TEXT(error, "Error: ConnectUsingLobby::DirectPlayLobbyCreate");
        goto failed;
    }
    result = baseLobby->QueryInterface(IID_IDirectPlayLobby3A, (void**)&lobby);
    if (result < 0) {
        NET_ERROR_TEXT(error, "Error: ConnectUsingLobby::QueryInterface");
        goto failed;
    }
    result = lobby->GetConnectionSettings(0, 0, &size);
    if (result >= 0 || result == NET_DPERR_BUFFERTOOSMALL) {
        connection = (NetConnection*)DebugMalloc(size, __FILE__, 778);
        if (!connection) {
            result = NET_E_OUTOFMEMORY;
            NET_ERROR_TEXT(error, "DPERR_OUTOFMEMORY");
            goto failed;
        }
        result = lobby->GetConnectionSettings(0, connection, &size);
    }
    if (result < 0) {
        NET_ERROR_TEXT(error, "Error: ConnectUsingLobby::GetConnectionSettings");
        goto failed;
    }
    GetSessionFlags(&flags);
    connection->session->flags = flags;
    connection->session->maxPlayers = 8;
    result = lobby->SetConnectionSettings(0, 0, connection);
    if (result < 0) {
        NET_ERROR_TEXT(error, "Error: ConnectUsingLobby::SetConnectionSettings");
        goto failed;
    }
    result = lobby->Connect(0, &baseDirectPlay, 0);
    if (result < 0) {
        NET_ERROR_TEXT(error, "Error: ConnectUsingLobby::Connect");
        goto failed;
    }
    result = baseDirectPlay->QueryInterface(IID_IDirectPlay4A, (void**)&directPlay4);
    if (result < 0) {
        NET_ERROR_TEXT(error, "Error: ConnectUsingLobby::QueryInterface");
        goto failed;
    }
    EnterCriticalSection(NET_LOCK(lock));
    directPlay = directPlay4;
    isHost = (connection->flags >> 1) & 1;              // DPLCONNECTION_CREATESESSION
    ReadProviderCaps((const GUID*)connection->provider);
    if (connectionMode == 2 && isHost)
        result = directPlay->CreatePlayer((unsigned long*)&localPlayer, connection->playerName,
                                          receiveEvent, 0, 0, 0x100);
    else
        result = directPlay->CreatePlayer((unsigned long*)&localPlayer, connection->playerName,
                                          receiveEvent, 0, 0, 0);
    if (result < 0) {
        LeaveCriticalSection(NET_LOCK(lock));
        NET_ERROR_TEXT(error, "Error: ConnectUsingLobby::CreatePlayer");
        goto failed;
    }
    AddPlayer(localPlayer, connection->playerName->shortName);
    lobbyConnected = 1;
    LeaveCriticalSection(NET_LOCK(lock));
    if (baseDirectPlay) {
        baseDirectPlay->Release();
        baseDirectPlay = 0;
    }
    if (baseLobby) {
        baseLobby->Release();
        baseLobby = 0;
    }
    DebugFree(connection, __FILE__, 867);
    return 0;

failed:
    ReportDirectPlayError(result, __FILE__, 887);
    if (baseDirectPlay) {
        baseDirectPlay->Release();
        baseDirectPlay = 0;
    }
    if (directPlay4) {
        directPlay4->Release();
        directPlay4 = 0;
    }
    if (baseLobby) {
        baseLobby->Release();
        baseLobby = 0;
    }
    if (connection)
        DebugFree(connection, __FILE__, 892);
    return result;
}

// Ring slot at a byte offset into the received-message array. Addressing the
// slot as messages + offset (offset = index * sizeof) gives retail's
// [eax+edi+4] SIB order; messages[index] gives [edi+eax+4].
static inline NetMessage& MessageAt(NetMessage* messages, int offset)
{
    return *(NetMessage*)((char*)messages + offset);
}

// 0x004aced0: dispatches the received messages in ring order.
void NetworkInterface::DispatchMessages(int value)
{
    char name[16];
    char systemName[16];
    int index;
    int last;
    int more;

    ResendPending(value);
    if (nextMessage || wrappedMessages) {
        if (!wrappedMessages) {
            index = 0;
            last = nextMessage - 1;
        } else {
            index = nextMessage;
            last = nextMessage - 1;
            if (last < 0)
                last = 0xff;
        }
        do {
            more = index != last;
            int offset = index * sizeof(NetMessage);
            if (MessageAt(messages, offset).from && !FindPlayer(MessageAt(messages, offset).from) &&
                GetPlayerName(MessageAt(messages, offset).from, name))
                AddPlayer(MessageAt(messages, offset).from, name);
            if (MessageAt(messages, offset).type == 0x40) {
                AcknowledgeMessage(*(short*)(MessageAt(messages, offset).data + 2));
            } else {
                g_TrackGame->OnNetworkMessage(
                    MessageAt(messages, offset).type, MessageAt(messages, offset).data,
                    MessageAt(messages, offset).from, MessageAt(messages, offset).to,
                    MessageAt(messages, offset).flags);
                NetMessage* message = &messages[index];
                if (!message->from) {
                    // DirectPlay system message: type, player type, player id.
                    int* system = (int*)message->data;
                    switch (message->type) {
                    case DPSYS_CREATEPLAYERORGROUP:
                        if (system[2] && system[1] == 1) {
                            GetPlayerName(system[2], systemName);
                            AddPlayer(system[2], systemName);
                        }
                        break;
                    case DPSYS_DESTROYPLAYERORGROUP:
                        RemovePlayer(system[2]);
                        break;
                    }
                }
            }
            MessageAt(messages, offset).type = 0;
            if (++index >= 0x100)
                index = 0;
        } while (more);
        nextMessage = 0;
        wrappedMessages = 0;
    }
    ringFull = 0;
}
