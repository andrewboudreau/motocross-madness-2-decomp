// Near-miss Net.cpp candidates, kept out of src/reconstructed until they
// match. See docs/NET.md.
//
// NetPendingMessage::UnknownFunction4aadc0 (0x004aadc0, 84 bytes): 87.5%.
// Retail stores the type, the sequence and then field_0x8f4 (the size);
// VC6 here hoists the field_0x8f4 store ahead of the other two.
//
// NetworkInterface::UnknownFunction4ab960 (0x004ab960, 989 bytes): about
// 15%; the candidate is 1072 bytes. Retail keeps the constant zero in ebx
// and tail-merges the error paths; VC6 here keeps `connection` in ebx and
// emits each NET_ERROR_TEXT expansion separately.
//
// NetworkInterface::UnknownFunction4abf10 (0x004abf10, 948 bytes): 92.4%.
// The MODEM case keeps `port` in edx where retail uses ebx.
//
// NetworkInterface::UnknownFunction4ac7c0 (0x004ac7c0, 53 bytes): 86.8%.
// The loop counter and *index swap ecx and edx. Caching *index in a local,
// declaring the counter first and `*index = i + 1` (90.6%) do not fix it.
//
// NetworkInterface::UnknownFunction4aced0 (0x004aced0, 370 bytes): 95.5%.
// The ring-slot address uses [edi+eax+4] where retail has [eax+edi+4], and
// after the slot-17 call retail reloads field_0x34 into edx and forms the
// slot with lea; direct indexing throughout (53.3%) and a pointer for the
// whole body (47.9%) are worse.
#include <windows.h>
#include <stdio.h>
#include <string.h>

#include "../../src/reconstructed/TrackGame.h"
#include "../../src/reconstructed/Net.h"
#include "../../src/reconstructed/DirectPlayMessages.h"

#define NET_LOCK(cs) ((CRITICAL_SECTION*)&(cs))
#define NET_DPERR_BUFFERTOOSMALL 0x8877001e
#define NET_E_OUTOFMEMORY        0x8007000e

void UnknownFunction4ad5a0(long result, const char* file, int line);

extern "C" const GUID IID_IDirectPlay4A;                // 0x005567b0
extern "C" const GUID IID_IDirectPlayLobby3A;           // 0x00556870
extern "C" const GUID DPSPGUID_IPX;                     // 0x005567d0
extern "C" const GUID DPSPGUID_TCPIP;                   // 0x005567e0
extern "C" const GUID DPSPGUID_SERIAL;                  // 0x005567f0
extern "C" const GUID DPSPGUID_MODEM;                   // 0x00556800
extern "C" const GUID DPAID_INet;                       // 0x00556940
extern "C" const GUID DPAID_ServiceProvider;            // 0x005568e0
extern "C" const GUID DPAID_Modem;                      // 0x00556900
extern "C" const GUID DPAID_Phone;                      // 0x00556920
extern "C" const GUID DPAID_INetPort;                   // 0x00556960
extern "C" const GUID DPAID_ComPort;                    // 0x00556970

// DPCOMPOUNDADDRESSELEMENT (0x18 bytes).
struct NetAddressElement {
    GUID type;
    unsigned long size;
    void* data;
};

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

// 0x004aadc0
int NetPendingMessage::UnknownFunction4aadc0(int type, short sequence, void* data,
                                              unsigned int size, int from, int to)
{
    UnknownFunction4aacc0(data, size, from, to, 0);
    field_0x00 = type;
    *(short*)&field_0x14[2] = sequence;
    field_0x8f4 = size;
    field_0x8f8 = 0;
    field_0x8fc = 0;
    return 1;
}

// 0x004ab960: connects with the settings of a lobby that launched the game.
long NetworkInterface::UnknownFunction4ab960()
{
    UnknownDirectPlayLobby3A* lobby = 0;
    UnknownDirectPlay4A* directPlay = 0;
    NetConnection* connection = 0;
    UnknownDirectPlay4A* directPlay4 = 0;
    unsigned long size;
    unsigned long flags;
    char error[256];
    long result;

    strcpy(error, "");
    result = DirectPlayLobbyCreateA(0, &lobby, 0, 0, 0);
    if (result < 0) {
        NET_ERROR_TEXT(error, "Error: ConnectUsingLobby::DirectPlayLobbyCreate");
        goto failed;
    }
    result = lobby->QueryInterface(IID_IDirectPlayLobby3A, (void**)&field_0x08);
    if (result < 0) {
        NET_ERROR_TEXT(error, "Error: ConnectUsingLobby::QueryInterface");
        goto failed;
    }
    result = field_0x08->GetConnectionSettings(0, 0, &size);
    if (result >= 0 || result == NET_DPERR_BUFFERTOOSMALL) {
        connection = (NetConnection*)DebugMalloc(size, __FILE__, 778);
        if (!connection) {
            result = NET_E_OUTOFMEMORY;
            NET_ERROR_TEXT(error, "DPERR_OUTOFMEMORY");
            goto failed;
        }
        result = field_0x08->GetConnectionSettings(0, connection, &size);
    }
    if (result < 0) {
        NET_ERROR_TEXT(error, "Error: ConnectUsingLobby::GetConnectionSettings");
        goto failed;
    }
    UnknownFunction4acdb0(&flags);
    connection->session->flags = flags;
    connection->session->maxPlayers = 8;
    result = field_0x08->SetConnectionSettings(0, 0, connection);
    if (result < 0) {
        NET_ERROR_TEXT(error, "Error: ConnectUsingLobby::SetConnectionSettings");
        goto failed;
    }
    result = field_0x08->Connect(0, &directPlay, 0);
    if (result < 0) {
        NET_ERROR_TEXT(error, "Error: ConnectUsingLobby::Connect");
        goto failed;
    }
    result = directPlay->QueryInterface(IID_IDirectPlay4A, (void**)&directPlay4);
    if (result < 0) {
        NET_ERROR_TEXT(error, "Error: ConnectUsingLobby::QueryInterface");
        goto failed;
    }
    EnterCriticalSection(NET_LOCK(field_0x48));
    field_0x04 = directPlay4;
    isHost = (connection->flags >> 1) & 1;              // DPLCONNECTION_CREATESESSION
    UnknownFunction4ad180((const GUID*)connection->provider);
    if (field_0x18 == 2 && isHost)
        result = field_0x04->CreatePlayer((unsigned long*)&field_0x0c, connection->playerName,
                                          field_0x24, 0, 0, 0x100);
    else
        result = field_0x04->CreatePlayer((unsigned long*)&field_0x0c, connection->playerName,
                                          field_0x24, 0, 0, 0);
    if (result < 0) {
        LeaveCriticalSection(NET_LOCK(field_0x48));
        NET_ERROR_TEXT(error, "Error: ConnectUsingLobby::CreatePlayer");
        goto failed;
    }
    UnknownFunction4ac980(field_0x0c, connection->playerName->shortName);
    field_0x14 = 1;
    LeaveCriticalSection(NET_LOCK(field_0x48));
    if (directPlay) {
        directPlay->Release();
        directPlay = 0;
    }
    if (lobby) {
        lobby->Release();
        lobby = 0;
    }
    DebugFree(connection, __FILE__, 867);
    return 0;

failed:
    UnknownFunction4ad5a0(result, __FILE__, 887);
    if (directPlay) {
        directPlay->Release();
        directPlay = 0;
    }
    if (directPlay4) {
        directPlay4->Release();
        directPlay4 = 0;
    }
    if (lobby) {
        lobby->Release();
        lobby = 0;
    }
    if (connection)
        DebugFree(connection, __FILE__, 892);
    return result;
}

// 0x004abf10: builds a DirectPlay address for `provider`.
int NetworkInterface::UnknownFunction4abf10(GUID provider, char* address, char* port,
                                            void* comPort, void** result,
                                            unsigned long* resultSize)
{
    NetAddressElement elements[3];
    unsigned long size = 0;
    int count;
    void* buffer;

    if (provider == DPSPGUID_IPX) {
        elements[0].type = DPAID_ServiceProvider;
        elements[0].size = sizeof(GUID);
        elements[0].data = (void*)&DPSPGUID_IPX;
        count = 1;
    } else if (provider == DPSPGUID_TCPIP) {
        elements[0].type = DPAID_ServiceProvider;
        elements[0].size = sizeof(GUID);
        elements[0].data = (void*)&DPSPGUID_TCPIP;
        elements[1].type = DPAID_INet;
        elements[1].size = lstrlenA(address) + 1;
        elements[1].data = address;
        count = 2;
        if (strcmp(port, "")) {
            elements[2].type = DPAID_INetPort;
            elements[2].size = 2;
            atoi(port);
            elements[2].data = &port;
            count = 3;
        }
    } else if (provider == DPSPGUID_MODEM) {
        elements[0].type = DPAID_ServiceProvider;
        elements[0].size = sizeof(GUID);
        elements[0].data = (void*)&DPSPGUID_MODEM;
        count = 1;
        if (strcmp(port, "")) {
            elements[1].type = DPAID_Phone;
            elements[1].data = port;
            elements[1].size = lstrlenA(port) + 1;
            count = 2;
        }
        elements[count].type = DPAID_Modem;
        elements[count].size = lstrlenA(address) + 1;
        elements[count].data = address;
        count++;
    } else if (provider == DPSPGUID_SERIAL && comPort) {
        elements[0].type = DPAID_ServiceProvider;
        elements[0].size = sizeof(GUID);
        elements[0].data = (void*)&DPSPGUID_SERIAL;
        elements[1].type = DPAID_ComPort;
        elements[1].size = 0x14;
        elements[1].data = comPort;
        count = 2;
    } else {
        return 0;
    }
    if (field_0x08->CreateCompoundAddress(elements, count, 0, &size) != NET_DPERR_BUFFERTOOSMALL)
        goto failed;
    buffer = DebugMalloc(size, __FILE__, 1115);
    if (!buffer)
        goto failed;
    if (field_0x08->CreateCompoundAddress(elements, count, buffer, &size) < 0) {
        DebugFree(buffer, __FILE__, 1135);
        goto failed;
    }
    *result = buffer;
    *resultSize = size;
    return 1;
failed:
    return 0;
}

// 0x004ac7c0: the player at *index, advancing *index.
NetPlayer* NetworkInterface::UnknownFunction4ac7c0(int* index)
{
    NetPlayer* player = field_0x30;
    if (!player)
        return 0;
    for (int i = 0; i != *index; i++) {
        if (!player)
            return 0;
        player = player->field_0x1c;
    }
    if (!player)
        return 0;
    (*index)++;
    return player;
}

// 0x004aced0: dispatches the received messages in ring order.
void NetworkInterface::UnknownFunction4aced0(int value)
{
    char name[16];
    char systemName[16];
    int index;
    int last;
    int more;

    UnknownFunction4acc50(value);
    if (field_0x40 || field_0x3c) {
        if (!field_0x3c) {
            index = 0;
            last = field_0x40 - 1;
        } else {
            index = field_0x40;
            last = field_0x40 - 1;
            if (last < 0)
                last = 0xff;
        }
        do {
            more = index != last;
            if (field_0x34[index].field_0x04 && !UnknownFunction4ac800(field_0x34[index].field_0x04) &&
                UnknownFunction4ac720(field_0x34[index].field_0x04, name))
                UnknownFunction4ac980(field_0x34[index].field_0x04, name);
            if (field_0x34[index].field_0x00 == 0x40) {
                UnknownFunction4acca0(*(short*)(field_0x34[index].field_0x14 + 2));
            } else {
                g_UnknownGlobal56e26c->UnknownVirtualSlot17(
                    field_0x34[index].field_0x00, field_0x34[index].field_0x14,
                    field_0x34[index].field_0x04, field_0x34[index].field_0x08,
                    field_0x34[index].field_0x0c);
                NetMessage* message = &field_0x34[index];
                if (!message->field_0x04) {
                    // DirectPlay system message: type, player type, player id.
                    int* system = (int*)message->field_0x14;
                    switch (message->field_0x00) {
                    case DPSYS_CREATEPLAYERORGROUP:
                        if (system[2] && system[1] == 1) {
                            UnknownFunction4ac720(system[2], systemName);
                            UnknownFunction4ac980(system[2], systemName);
                        }
                        break;
                    case DPSYS_DESTROYPLAYERORGROUP:
                        UnknownFunction4aca80(system[2]);
                        break;
                    }
                }
            }
            field_0x34[index].field_0x00 = 0;
            if (++index >= 0x100)
                index = 0;
        } while (more);
        field_0x40 = 0;
        field_0x3c = 0;
    }
    field_0x38 = 0;
}
