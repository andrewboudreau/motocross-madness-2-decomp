// Net.cpp -- reconstruction of D:\aardvark\VC\krusty2\Net.cpp
// (0x004aab00..0x004ae2e7). See Net.h and docs/NET.md.

#include <windows.h>
#include <stdio.h>
#include <string.h>
#include <process.h>

#include "TrackGame.h"
#include "Net.h"

#define NET_LOCK(cs) ((CRITICAL_SECTION*)&(cs))

// Per-TU vector constants (tier 2): the four dynamic initialisers at
// 0x004aab00..0x004aac3b, as in Lzw.cpp; they store 0x00688680, 0x00688690,
// 0x006886a0 and 0x00688670. The names are tier 3.
struct NetConstVec3 {
    float x, y, z;
    NetConstVec3(float x_, float y_, float z_) { x = x_; y = y_; z = z_; }
};
static const NetConstVec3 kVec3Zero = NetConstVec3(0.0f, 0.0f, 0.0f);
static const NetConstVec3 kVec3XAxis = NetConstVec3(1.0f, 0.0f, 0.0f);
static const NetConstVec3 kVec3YAxis = NetConstVec3(0.0f, 1.0f, 0.0f);
static const NetConstVec3 kVec3ZAxis = NetConstVec3(0.0f, 0.0f, 1.0f);

// HRESULTs (tier 1 literals; SDK names tier 3).
#define NET_DPERR_BUFFERTOOSMALL 0x8877001e
#define NET_DPERR_INVALIDPLAYER  0x88770096
#define NET_DPERR_NOTLOBBIED     0x8877042e
#define NET_DPERR_INVALIDOBJECT  0x88770082
#define NET_DPERR_CONNECTING     0x8877015e
#define NET_E_PENDING            0x8000000a
#define NET_E_OUTOFMEMORY        0x8007000e

// cdecl 0x004ad5a0: reports a DirectPlay error with its source position.
void UnknownFunction4ad5a0(long result, const char* file, int line);

// 0x004af6a0 (NetProcs.cpp): the receive thread.
unsigned int __stdcall UnknownFunction4af6a0(void* net);

// dplay.h/dplobby.h GUIDs.
extern "C" const GUID CLSID_DirectPlay;                 // 0x005567c0
extern "C" const GUID IID_IDirectPlay4A;                // 0x005567b0
extern "C" const GUID IID_IDirectPlayLobby3A;           // 0x00556870
extern "C" const GUID DPSPGUID_IPX;                     // 0x005567d0
extern "C" const GUID DPSPGUID_TCPIP;                   // 0x005567e0
extern "C" const GUID DPSPGUID_SERIAL;                  // 0x005567f0
extern "C" const GUID DPSPGUID_MODEM;                   // 0x00556800
extern "C" const GUID DPAID_INet;                       // 0x00556940

// 0x00556dc0: the application GUID passed to Open and EnumConnections.
extern const GUID g_UnknownGuid556dc0;

int __stdcall UnknownFunction4addc0(const GUID* provider, void* connection, unsigned long size,
                                    const NetName* name, unsigned long flags, void* context);
int __stdcall UnknownFunction4ae100(const NetSessionDesc* desc, unsigned long* timeout,
                                    unsigned long flags, void* context);
int __stdcall UnknownFunction4ae270(unsigned long id, unsigned long type, const NetName* name,
                                    unsigned long flags, void* context);


// 0x004aac40: EnumAddress callback; copies the INet address string.
int __stdcall UnknownFunction4aac40(const GUID& type, unsigned long size, const void* data,
                                    void* context)
{
    if (type == DPAID_INet)
        strcpy((char*)context, (const char*)data);
    return 1;
}

// 0x004aac90
NetMessage::NetMessage()
{
    field_0x00 = 0;
    field_0x04 = 0;
    field_0x08 = 0;
    field_0x0c = 0;
    field_0x10 = 0;
    memset(field_0x14, 0, sizeof(field_0x14));
}

// 0x004aacc0
void NetMessage::UnknownFunction4aacc0(void* data, unsigned int size, int from, int to, int flags)
{
    char text[256];

    if (!from)
        field_0x00 = *(int*)data;
    else
        field_0x00 = *(unsigned char*)data;
    field_0x04 = from;
    field_0x08 = to;
    field_0x0c = flags;
    field_0x10 = size;
    memcpy(field_0x14, data, size);
    if (size >= NET_MAX_GENERIC_MSG_SIZE)
        sprintf(text, "dwMsgSize (%d)NET_MAX_GENERIC_MSG_SIZE not large enough\n", size);
}

// 0x004aad50
NetIncomingMessage::NetIncomingMessage()
{
}

// 0x004aad60
void NetIncomingMessage::UnknownFunction4aad60(void* data, unsigned int size, int from, int to)
{
    UnknownFunction4aacc0(data, size, from, to, 0);
}

// 0x004aad80
NetPendingMessage::NetPendingMessage()
{
    memset(this, 0, sizeof(NetMessage));
    field_0x8f4 = 0;
    field_0x8f8 = 0;
    field_0x8fc = 0;
    field_0x900 = 0;
}

// 0x004aae20 (identical-code folded with another TU's one-pointer constructor)
NetPendingList::NetPendingList()
{
    field_0x00 = 0;
}

// 0x004aae30
NetPendingList::~NetPendingList()
{
    NetPendingMessage* message = field_0x00;
    while (message) {
        NetPendingMessage* current = message;
        message = message->field_0x8fc;
        delete current;
    }
}

// 0x004aae50: appends a guaranteed message.
void NetPendingList::UnknownFunction4aae50(int type, short sequence, void* data,
                                           unsigned int size, int from, int to)
{
    if (!field_0x00) {
        NetPendingMessage* message = new(__FILE__, 209) NetPendingMessage;
        if (message && message->UnknownFunction4aadc0(type, sequence, data, size, from, to))
            field_0x00 = message;
        return;
    }
    NetPendingMessage* last;
    for (NetPendingMessage* message = field_0x00; message; message = message->field_0x8fc)
        last = message;
    last->field_0x8fc = new(__FILE__, 218) NetPendingMessage;
    if (last->field_0x8fc)
        last->field_0x8fc->UnknownFunction4aadc0(type, sequence, data, size, from, to);
}

// 0x004aaf20: resends every message unacknowledged for four seconds.
long NetPendingList::UnknownFunction4aaf20(NetworkInterface* net, int unused)
{
    long result = 0;
    for (NetPendingMessage* message = field_0x00; message; message = message->field_0x8fc) {
        message->field_0x8f8 += g_UnknownGlobal56e26c->field_0x2f0;
        if (message->field_0x8f8 > 4.0f) {
            result = net->field_0x04->SendEx(message->field_0x04, message->field_0x08, 0x600,
                                             message->field_0x14, message->field_0x8f4, 0, 0, 0, 0);
            if (result == NET_DPERR_INVALIDPLAYER)
                return NET_DPERR_INVALIDPLAYER;
            if (result != 0 && result != NET_E_PENDING)
                UnknownFunction4ad5a0(result, __FILE__, 269);
            message->field_0x8f8 = 0;
        }
    }
    return result;
}

// 0x004aafe0: drops the acknowledged message.
void NetPendingList::UnknownFunction4aafe0(short sequence)
{
    if (!field_0x00)
        return;
    NetPendingMessage* message = field_0x00;
    NetPendingMessage* previous = message;
    while (message) {
        if (*(short*)&message->field_0x14[2] == sequence)
            break;
        previous = message;
        message = message->field_0x8fc;
    }
    if (!message)
        return;
    if (field_0x00 == message) {
        field_0x00 = message->field_0x8fc;
        delete message;
    } else {
        previous->field_0x8fc = message->field_0x8fc;
        delete message;
    }
}

// 0x004ab050
NetPlayer::NetPlayer(int id, const char* name)
{
    field_0x00 = id;
    int length = strlen(name);
    int count = length > 15 ? 15 : length;
    strncpy(field_0x04, name, count);
    field_0x04[count] = 0;
    field_0x14 = new(__FILE__, 314) NetPendingList;
    field_0x18 = 0;
    field_0x1c = 0;
}

// 0x004ab0d0
NetPlayer::~NetPlayer()
{
    if (field_0x14)
        delete field_0x14;
}

// 0x004ab0f0
NetFile::NetFile()
{
    field_0x00 = 0;
    field_0x04 = 0;
    field_0x08 = 0;
    strcpy(field_0x0c, "");
    field_0x110 = 0;
    field_0x114 = 0;
    field_0x118 = 0;
    field_0x11c = 0;
    field_0x120 = 0;
    field_0x124 = 0;
    field_0x128_bit0 = 0;
    field_0x128_bit1 = 0;
    field_0x128_bit2 = 0;
    field_0x128_bit3 = 0;
}

// 0x004ab160
NetFile::~NetFile()
{
    if (field_0x00)
        operator delete(field_0x00, __FILE__, 347);
    if (field_0x04)
        delete field_0x04;
}

// 0x004ab190
void NetFile::UnknownFunction4ab190(int unused)
{
    if (field_0x128_bit0)
        UnknownFunction4ab1b0();
}

// 0x004ab1b0: sends the next chunk.
void NetFile::UnknownFunction4ab1b0()
{
    NetFileChunk chunk;

    field_0x110 = 0;
    int offset = field_0x11c * sizeof(chunk.field_0x04);
    int count = field_0x08 - offset;
    if (count >= (int)sizeof(chunk.field_0x04))
        count = sizeof(chunk.field_0x04);
    chunk.field_0x8d4 = count;
    memcpy(chunk.field_0x04, field_0x00 + offset, count);
    chunk.field_0x8d8 = field_0x11c;
    g_UnknownGlobal56e26c->field_0x08->UnknownFunction4ac830(0xcf, &chunk, sizeof(chunk), 0,
                                                             field_0x114);
    field_0x11c++;
    if (field_0x11c >= field_0x118) {
        field_0x128_bit0 = 0;
        field_0x128_bit2 = 1;
        field_0x120(field_0x124);
    }
}

// 0x004ab280: starts receiving the announced file.
void NetFile::UnknownFunction4ab280(int id, NetFileHeader* header)
{
    field_0x110 = id;
    int length = strlen(header->field_0x04);
    int count = length > 0x103 ? 0x103 : length;
    strncpy(field_0x0c, header->field_0x04, count);
    field_0x0c[count] = 0;
    field_0x08 = header->field_0x108;
    if (field_0x00)
        operator delete(field_0x00, __FILE__, 497);
    field_0x00 = (char*)DebugMalloc(field_0x08, __FILE__, 499);
    field_0x118 = field_0x08 / (int)sizeof(((NetFileChunk*)0)->field_0x04) + 1;
    field_0x11c = 0;
    field_0x04 = new(__FILE__, 503) int[field_0x118];
    for (int i = 0; i < field_0x118; i++)
        field_0x04[i] = 0;
    field_0x128_bit1 = 1;
}

// 0x004ab380: stores a received chunk.
void NetFile::UnknownFunction4ab380(int id, NetFileChunk* chunk)
{
    if (field_0x110 != id)
        return;
    if (field_0x04[chunk->field_0x8d8])
        return;
    field_0x04[chunk->field_0x8d8] = 1;
    memcpy(field_0x00 + chunk->field_0x8d8 * sizeof(chunk->field_0x04), chunk->field_0x04,
           chunk->field_0x8d4);
    field_0x11c++;
    if (field_0x11c == field_0x118) {
        field_0x128_bit1 = 0;
        UnknownFunction4ab440();
        operator delete(field_0x00, __FILE__, 534);
        field_0x00 = 0;
        field_0x128_bit3 = 1;
    }
}

// 0x004ab440: writes the received file.
void NetFile::UnknownFunction4ab440()
{
    FILE* file = fopen(field_0x0c, "wb");
    if (file) {
        fwrite(field_0x00, 1, field_0x08, file);
        fclose(file);
    }
}

// 0x004ab480
NetworkInterface::NetworkInterface()
{
    field_0x04 = 0;
    field_0x08 = 0;
    field_0x0c = 0;
    field_0x10 = 0;
    field_0x14 = 0;
    field_0x18 = 0;
    field_0x1c = 0;
    field_0x20 = 0;
    field_0x24 = 0;
    field_0x28 = 0;
    field_0x2c = 0;
    field_0x30 = 0;
    field_0x34 = 0;
    field_0x38 = 0;
    field_0x3c = 0;
    field_0x40 = 0;
    field_0x44 = 0;
    InitializeCriticalSection(NET_LOCK(field_0x48));
    InitializeCriticalSection(NET_LOCK(field_0x60));
    field_0x78 = 0;
    field_0x7c = 0;
    field_0x80 = 0;
    for (int i = 0; i < 8; i++)
        field_0x84[i] = 0;
    memset(&field_0xa4, 0, sizeof(field_0xa4));
    memset(&field_0xcc, 0, sizeof(field_0xcc));
    field_0xf4 = 0;
    field_0xf8 = 0;
    field_0xfc = 0;
}

// 0x004ab570
NetworkInterface::~NetworkInterface()
{
    UnknownFunction4abe70();
    EnterCriticalSection(NET_LOCK(field_0x48));
    NetPlayer* player = field_0x30;
    while (player) {
        NetPlayer* current = player;
        player = player->field_0x1c;
        delete current;
    }
    if (field_0x34) {
        delete field_0x34;
        field_0x34 = 0;
    }
    for (int i = 0; i < field_0x114.m_count; i++) {
        NetFile* file = field_0x114.Get(i);
        if (file)
            delete file;
    }
    field_0x114.Clear();
    LeaveCriticalSection(NET_LOCK(field_0x48));
    DeleteCriticalSection(NET_LOCK(field_0x48));
    DeleteCriticalSection(NET_LOCK(field_0x60));
}

// 0x004ab6b0: starts DirectPlay in connection `mode` (1..4); 0 or an HRESULT.
long NetworkInterface::UnknownFunction4ab6b0(int mode)
{
    PlayerInfoType players[7];
    int count;
    long result;

    field_0x18 = mode;
    field_0x24 = CreateEventA(0, 0, 0, 0);
    field_0x28 = CreateEventA(0, 0, 0, 0);
    field_0x2c = CreateEventA(0, 0, 0, 0);
    if (!field_0x24 || !field_0x28 || !field_0x2c)
        goto outOfMemory;
    field_0x34 = new(__FILE__, 650) NetMessage[0x100];
    if (!field_0x34)
        goto outOfMemory;
    field_0x1c = (void*)_beginthreadex(0, 0, UnknownFunction4af6a0, this, 0, &field_0x20);
    if (!field_0x1c)
        goto outOfMemory;
    result = UnknownFunction4ab960();
    if (result < 0 && (mode == 4 || result != NET_DPERR_NOTLOBBIED))
        goto failed;
    if (!field_0x14) {
        result = CoCreateInstance(CLSID_DirectPlay, 0, CLSCTX_INPROC_SERVER, IID_IDirectPlay4A,
                                  (void**)&field_0x04);
        if (result < 0)
            goto failed;
    }
    if (field_0x14) {
        if (field_0x10) {
            if (!UnknownFunction4accd0(field_0x0c))
                goto failed;
        } else {
            if (!UnknownFunction4acd20())
                goto failed;
        }
    }
    if (UnknownFunction4ae200(0, players, &count) >= 0) {
        for (int i = 0; i < count; i++)
            UnknownFunction4ac980(players[i].field_0x04, players[i].field_0x08);
    }
    if (field_0x18 == 3 && field_0x10)
        field_0x100.Init(16, 16);
    field_0x114.Init(2, 1);
    return 0;

outOfMemory:
    result = NET_E_OUTOFMEMORY;
failed:
    UnknownFunction4abe70();
    return result;
}

// 0x004abd40: strips the trailer DPlay appended to a received message.
void NetworkInterface::UnknownFunction4abd40(unsigned char** data, unsigned long* size,
                                             int* guaranteed, int* sequence, int* target)
{
    unsigned char* message = *data;
    message[0] &= ~0x20;
    *guaranteed = message[0] >> 7;
    int trailerSize = *guaranteed ? 8 : 4;
    int* trailer = (int*)(*data - trailerSize + *size);
    *target = trailer[0];
    *sequence = *guaranteed ? trailer[0] + 1 : 0;
    *data = (unsigned char*)DebugMalloc(*size - trailerSize, __FILE__, 927);
    memcpy(*data, message, *size - trailerSize);
    operator delete(message, __FILE__, 929);
    *size -= trailerSize;
}

// 0x004abdf0: initializes the service provider connection.
int NetworkInterface::UnknownFunction4abdf0(void* connection, int, int kind)
{
    if (field_0x04->InitializeConnection(connection, 0))
        return 0;
    field_0xa4.size = sizeof(field_0xa4);
    if (field_0x04->GetCaps(&field_0xa4, 1) < 0)
        memset(&field_0xa4, 0, sizeof(field_0xa4));
    field_0xcc.size = sizeof(field_0xcc);
    if (field_0x04->GetCaps(&field_0xcc, 0) < 0)
        memset(&field_0xcc, 0, sizeof(field_0xcc));
    field_0xf4 = kind;
    return 1;
}

// 0x004abe70: stops the thread and releases DirectPlay.
long NetworkInterface::UnknownFunction4abe70()
{
    if (field_0x78)
        UnknownFunction4ac950();
    if (field_0x1c) {
        SetEvent(field_0x28);
        WaitForSingleObject(field_0x1c, INFINITE);
    }
    if (field_0x1c) {
        CloseHandle(field_0x1c);
        field_0x1c = 0;
    }
    if (field_0x24) {
        CloseHandle(field_0x24);
        field_0x24 = 0;
    }
    if (field_0x28) {
        CloseHandle(field_0x28);
        field_0x28 = 0;
    }
    if (field_0x2c) {
        CloseHandle(field_0x2c);
        field_0x2c = 0;
    }
    if (field_0x04) {
        if (field_0x0c) {
            field_0x04->DestroyPlayer(field_0x0c);
            field_0x0c = 0;
        }
        field_0x04->Close();
        if (field_0x04) {
            field_0x04->Release();
            field_0x04 = 0;
        }
    }
    return 0;
}

// 0x004ac2d0: the local player's INet address.
int NetworkInterface::UnknownFunction4ac2d0(char* address)
{
    strcpy(address, "");
    if (!(field_0xf4 & 6))
        return 0;
    unsigned long size;
    long result = field_0x04->GetPlayerAddress(field_0x0c, 0, &size);
    if (result != NET_DPERR_BUFFERTOOSMALL) {
        UnknownFunction4ad5a0(result, __FILE__, 1158);
        return 0;
    }
    void* data = DebugMalloc(size, __FILE__, 1162);
    if (field_0x04->GetPlayerAddress(field_0x0c, data, &size))
        goto failed;
    if (field_0x08->EnumAddress(UnknownFunction4aac40, data, size, address))
        goto failed;
    operator delete(data, __FILE__, 1172);
    return 1;
failed:
    return 0;
}

// 0x004ac3c0: creates and opens a session named `name`.
int NetworkInterface::UnknownFunction4ac3c0(char* name, unsigned long flags)
{
    if (!field_0x04)
        return 0;
    unsigned long sessionFlags;
    NetSessionDesc desc;
    UnknownFunction4acdb0(&sessionFlags);
    memset(&desc, 0, sizeof(desc));
    desc.size = sizeof(desc);
    desc.flags = sessionFlags;
    *(GUID*)desc.application = g_UnknownGuid556dc0;
    desc.maxPlayers = 8;
    desc.sessionName = name;
    if (field_0x04->Open(&desc, flags | 2)) {
        field_0x04->Close();
        return 0;
    }
    field_0x10 = 1;
    return 1;
}

// 0x004ac480: joins the session `instance`.
int NetworkInterface::UnknownFunction4ac480(const GUID* instance, unsigned long flags)
{
    if (!field_0x04)
        return 0;
    NetSessionDesc desc;
    memset(&desc, 0, sizeof(desc));
    desc.size = sizeof(desc);
    *(GUID*)desc.instance = *instance;
    if (field_0x04->Open(&desc, flags | 1))
        goto failed;
    field_0x10 = 0;
    if (!UnknownFunction4acd20())
        goto failed;
    return 1;
failed:
    field_0x04->Close();
    return 0;
}

// 0x004ac510: closes (value 0) or opens the session to new players.
int NetworkInterface::UnknownFunction4ac510(int value)
{
    unsigned long size;

    if (!field_0x04 || !field_0x10)
        return 0;
    NetSessionDesc* desc = 0;
    long result = field_0x04->GetSessionDesc(desc, &size);
    if (result == NET_DPERR_BUFFERTOOSMALL) {
        desc = (NetSessionDesc*)DebugMalloc(size, __FILE__, 1254);
        if (!desc)
            return 0;
        if (field_0x04->GetSessionDesc(desc, &size))
            goto failed;
    }
    if (value)
        desc->flags &= ~1;
    else
        desc->flags |= 1;
    if (field_0x04->SetSessionDesc(desc, 0))
        goto failed;
    operator delete(desc, __FILE__, 1274);
    return 1;
failed:
    if (desc)
        operator delete(desc, __FILE__, 1279);
    return 0;
}

// 0x004ac5e0: creates the local player and joins or creates the group.
int NetworkInterface::UnknownFunction4ac5e0(char* name)
{
    unsigned long player;

    if (!UnknownFunction4ac650(name, &player))
        goto failed;
    field_0x0c = player;
    if (field_0x10) {
        if (!UnknownFunction4accd0(player))
            goto failed;
    } else {
        UnknownFunction4acd20();
    }
    return 1;
failed:
    return 0;
}

// 0x004ac630
int NetworkInterface::UnknownFunction4ac630()
{
    if (field_0x0c) {
        int result = UnknownFunction4ac6f0(field_0x0c);
        field_0x0c = 0;
        return result;
    }
    return 0;
}

// 0x004ac650
int NetworkInterface::UnknownFunction4ac650(char* name, unsigned long* player)
{
    NetName playerName;

    if (!field_0x04)
        return 0;
    memset(&playerName, 0, sizeof(playerName));
    playerName.size = sizeof(playerName);
    playerName.shortName = name;
    playerName.longName = 0;
    if (field_0x10 && field_0x18 == 2) {
        if (field_0x04->CreatePlayer(player, &playerName, field_0x24, 0, 0, 0x100))
            goto failed;
    } else {
        if (field_0x04->CreatePlayer(player, &playerName, field_0x24, 0, 0, 0))
            goto failed;
    }
    return 1;
failed:
    return 0;
}

// 0x004ac6f0
int NetworkInterface::UnknownFunction4ac6f0(unsigned long player)
{
    if (field_0x04 && player && field_0x04->DestroyPlayer(player) == 0)
        return 1;
    return 0;
}

// 0x004ac720
int NetworkInterface::UnknownFunction4ac720(int player, char* name)
{
    strcpy(name, "");
    NetName* playerName = 0;
    if (!UnknownFunction4ad0f0(player, &playerName))
        goto failed;
    strncpy(name, playerName->shortName, 16);
    operator delete(playerName, __FILE__, 1365);
    return 1;
failed:
    if (playerName)
        operator delete(playerName, __FILE__, 1370);
    return 0;
}

// 0x004ac800
NetPlayer* NetworkInterface::UnknownFunction4ac800(int id)
{
    NetPlayer* player = field_0x30;
    if (!player)
        return 0;
    while (player) {
        if (player->field_0x00 == id)
            break;
        player = player->field_0x1c;
    }
    if (!player)
        return 0;
    return player;
}

// 0x004ac830: sends a message; types with bit 7 set are guaranteed.
int NetworkInterface::UnknownFunction4ac830(int type, void* data, int size, int from, int to)
{
    if (!from)
        from = field_0x0c;
    if (type & 0x80) {
        UnknownFunction4acbc0(type, data, size, from);
    } else if (field_0x18 == 3 && !field_0x10) {
        int target = to;
        if (!UnknownFunction4ad280(type, data, size, 0, from, field_0xf8, &target, sizeof(target)))
            return 0;
    } else {
        if (!UnknownFunction4ad280(type, data, size, 0, from, to, 0, 0))
            return 0;
    }
    return 1;
}

// 0x004ac8d0
int NetworkInterface::UnknownFunction4ac8d0(float a, float b)
{
    EnterCriticalSection(NET_LOCK(field_0x60));
    field_0x78 = 1;
    field_0x7c = a;
    field_0x80 = b;
    int index = 0;
    unsigned int time = UnknownFunction4bfa80();
    NetPlayer* player;
    while ((player = UnknownFunction4ac7c0(&index)) != 0)
        player->field_0x18 = time;
    LeaveCriticalSection(NET_LOCK(field_0x60));
    SetEvent(field_0x2c);
    return 1;
}

// 0x004ac950
int NetworkInterface::UnknownFunction4ac950()
{
    EnterCriticalSection(NET_LOCK(field_0x60));
    field_0x78 = 0;
    LeaveCriticalSection(NET_LOCK(field_0x60));
    return 1;
}

// 0x004ac980: adds a player unless known or removed.
void NetworkInterface::UnknownFunction4ac980(int id, const char* name)
{
    for (int i = 0; i < 8; i++) {
        if (id == field_0x84[i])
            return;
    }
    if (!field_0x30) {
        field_0x30 = new(__FILE__, 1595) NetPlayer(id, name);
        return;
    }
    NetPlayer* last;
    NetPlayer* player = field_0x30;
    while (player) {
        if (player->field_0x00 == id)
            return;
        last = player;
        player = player->field_0x1c;
    }
    last->field_0x1c = new(__FILE__, 1603) NetPlayer(id, name);
}

// 0x004aca80: removes a player and remembers its id.
void NetworkInterface::UnknownFunction4aca80(int id)
{
    if (!field_0x30)
        return;
    NetPlayer* player = field_0x30;
    NetPlayer* previous = player;
    while (player) {
        if (player->field_0x00 == id)
            break;
        previous = player;
        player = player->field_0x1c;
    }
    if (!player)
        return;
    if (field_0x30 == player)
        field_0x30 = player->field_0x1c;
    else
        previous->field_0x1c = player->field_0x1c;
    for (int i = 0; i < 8; i++) {
        if (!field_0x84[i]) {
            field_0x84[i] = player->field_0x00;
            break;
        }
    }
    delete player;
}

// 0x004acb00: stores a received message in the next free ring slot.
void NetworkInterface::UnknownFunction4acb00(void* data, unsigned int size, int from, int to,
                                             int flags)
{
    int tries = 0;
    while (field_0x34[field_0x40].field_0x00 & 0x80) {
        field_0x40++;
        if (++tries >= 0x100) {
            field_0x38 = 1;
            return;
        }
        if (field_0x40 >= 0x100) {
            field_0x40 = 0;
            field_0x3c = 0x100;
            field_0x38 = 1;
        }
    }
    if (!from)
        field_0x34[field_0x40].UnknownFunction4aacc0(data, size, 0, to, flags);
    else
        field_0x34[field_0x40].UnknownFunction4aacc0(data, size, from, to, flags);
    field_0x40++;
    if (field_0x40 >= 0x100) {
        field_0x40 = 0;
        field_0x3c = 0x100;
        field_0x38 = 1;
    }
}

// 0x004acbc0: sends a guaranteed message to every other player.
void NetworkInterface::UnknownFunction4acbc0(int type, void* data, int size, int from)
{
    EnterCriticalSection(NET_LOCK(field_0x48));
    for (NetPlayer* player = field_0x30; player; player = player->field_0x1c) {
        if (player->field_0x00 && field_0x0c != player->field_0x00) {
            UnknownFunction4ad280(type, data, size, field_0x44, from, player->field_0x00, 0, 0);
            player->field_0x14->UnknownFunction4aae50(type, field_0x44, data, size, from,
                                                      player->field_0x00);
            field_0x44++;
        }
    }
    LeaveCriticalSection(NET_LOCK(field_0x48));
}

// 0x004acc50: resends pending messages; drops a player DirectPlay no longer knows.
void NetworkInterface::UnknownFunction4acc50(int value)
{
    int lost = 0;
    NetPlayer* player = field_0x30;
    if (!player)
        return;
    int lostPlayer = value;
    for (; player; player = player->field_0x1c) {
        if (player->field_0x14->UnknownFunction4aaf20(this, value) == NET_DPERR_INVALIDPLAYER) {
            lostPlayer = player->field_0x00;
            lost = 1;
        }
    }
    if (lost)
        UnknownFunction4aca80(lostPlayer);
}

// 0x004acca0: an acknowledgement for `sequence`.
void NetworkInterface::UnknownFunction4acca0(short sequence)
{
    for (NetPlayer* player = field_0x30; player; player = player->field_0x1c) {
        if (field_0x0c != player->field_0x00)
            player->field_0x14->UnknownFunction4aafe0(sequence);
    }
}

// 0x004accd0: the host creates the group and joins it.
int NetworkInterface::UnknownFunction4accd0(unsigned long player)
{
    if (field_0x04->CreateGroup(&field_0xfc, 0, 0, 0, 0))
        goto failed;
    if (field_0x04->AddPlayerToGroup(field_0xfc, player))
        goto failed;
    field_0xf8 = player;
    return 1;
failed:
    return 0;
}

// 0x004acd20: a client finds the group and its server player.
int NetworkInterface::UnknownFunction4acd20()
{
    if (field_0x04->EnumGroups(0, (void*)UnknownFunction4acd70, this, 0))
        goto failed;
    if (field_0xfc &&
        field_0x04->EnumGroupPlayers(field_0xfc, 0, (void*)UnknownFunction4acd90, this, 0))
        goto failed;
    return 1;
failed:
    return 0;
}

// 0x004acd70
int __stdcall NetworkInterface::UnknownFunction4acd70(unsigned long id, unsigned long type,
                                                      const NetName* name, unsigned long flags,
                                                      void* context)
{
    ((NetworkInterface*)context)->field_0xfc = id;
    return 1;
}

// 0x004acd90
int __stdcall NetworkInterface::UnknownFunction4acd90(unsigned long id, unsigned long type,
                                                      const NetName* name, unsigned long flags,
                                                      void* context)
{
    ((NetworkInterface*)context)->field_0xf8 = id;
    return 1;
}

// 0x004acdb0: DPSESSION flags for the connection mode.
void NetworkInterface::UnknownFunction4acdb0(unsigned long* flags)
{
    *flags = 0xa040;
    switch (field_0x18) {
    case 1:
    case 4:
        *flags = 0xa044;
        break;
    case 2:
        *flags = 0xb040;
        break;
    }
}

// 0x004acde0: the per-frame update.
int NetworkInterface::UnknownFunction4acde0(int value)
{
    EnterCriticalSection(NET_LOCK(field_0x48));
    UnknownFunction4aced0(value);
    if (field_0x18 == 3 && field_0x10)
        UnknownVirtualSlot0(value);
    for (int i = 0; i < field_0x114.m_count; i++) {
        NetFile* file = field_0x114.Get(i);
        file->UnknownFunction4ab190(value);
        if (file->field_0x128_bit2 || file->field_0x128_bit3) {
            field_0x114.Remove(file);
            if (file)
                delete file;
        }
    }
    LeaveCriticalSection(NET_LOCK(field_0x48));
    return 1;
}

// 0x004ad050 (slot 0): sends the held messages to the group.
void NetworkInterface::UnknownVirtualSlot0(int)
{
    for (int i = 0; i < field_0x100.m_count; i++) {
        NetIncomingMessage* message = field_0x100.Get(i);
        UnknownFunction4ad280(message->field_0x00, message->field_0x14, message->field_0x10, 0,
                              field_0x0c, message->field_0x08, 0, 0);
        delete message;
    }
    field_0x100.Clear();
}

// 0x004ad0f0: the player's DPNAME, allocated; 0 on failure.
int NetworkInterface::UnknownFunction4ad0f0(int player, NetName** name)
{
    if (!field_0x04)
        return 0;
    unsigned long size;
    void* data;
    if (field_0x04->GetPlayerName(player, 0, &size) != NET_DPERR_BUFFERTOOSMALL)
        goto failed;
    data = DebugMalloc(size, __FILE__, 1999);
    if (!data)
        goto failed;
    if (field_0x04->GetPlayerName(player, data, &size)) {
        operator delete(data, __FILE__, 2015);
        goto failed;
    }
    *name = (NetName*)data;
    return 1;
failed:
    return 0;
}

// 0x004ad180: reads the session caps and the service provider kind.
void NetworkInterface::UnknownFunction4ad180(const GUID* provider)
{
    memset(&field_0xa4, 0, sizeof(field_0xa4));
    memset(&field_0xcc, 0, sizeof(field_0xcc));
    field_0xa4.size = sizeof(field_0xa4);
    if (field_0x04->GetCaps(&field_0xa4, 1) < 0)
        memset(&field_0xa4, 0, sizeof(field_0xa4));
    field_0xcc.size = sizeof(field_0xcc);
    if (field_0x04->GetCaps(&field_0xcc, 0) < 0) {
        memset(&field_0xcc, 0, sizeof(field_0xcc));
        return;
    }
    if (*provider == DPSPGUID_IPX)
        field_0xf4 = 1;
    else if (*provider == DPSPGUID_MODEM)
        field_0xf4 = 0x10;
    else if (*provider == DPSPGUID_TCPIP)
        field_0xf4 = 4;
    else if (*provider == DPSPGUID_SERIAL)
        field_0xf4 = 8;
}

// 0x004ad280: sends `data`, appending `extra` when given.
int NetworkInterface::UnknownFunction4ad280(int type, void* data, int size, short sequence,
                                            int from, int to, void* extra, int extraSize)
{
    if (!from)
        return 1;
    unsigned char* message = (unsigned char*)data;
    long result;
    message[0] = (unsigned char)type;
    if (type & 0x80)
        *(short*)(message + 2) = sequence;
    if (extra) {
        message[0] = (unsigned char)(type | 0x20);
        int total = size + extraSize;
        unsigned char* buffer = (unsigned char*)DebugMalloc(total, __FILE__, 2156);
        memcpy(buffer, data, size);
        memcpy(buffer + size, extra, extraSize);
        result = field_0x04->SendEx(from, to, 0x600, buffer, total, 0, 0, 0, 0);
        operator delete(buffer, __FILE__, 2182);
    } else {
        result = field_0x04->SendEx(from, to, 0x600, data, size, 0, 0, 0, 0);
    }
    if (result && result != NET_E_PENDING) {
        UnknownFunction4ad5a0(result, __FILE__, 2264);
        return 0;
    }
    return 1;
}

// 0x006886ac, 0x006886b0, 0x006886b4: entries filled by the enumeration callbacks.
int g_UnknownGlobal6886ac;
int g_UnknownGlobal6886b0;
int g_UnknownGlobal6886b4;

// 0x004ad3b0..0x004ad592: a WinSock stream client (TrackGame's global at
// 0x0068a48c). Placement between Net.cpp functions is the ownership evidence.
UnknownTrackGameGlobal68a48c::UnknownTrackGameGlobal68a48c()
{
    field_0x00 = 0;
    field_0x08 = 1000;
    field_0x0c = 0;
}

UnknownTrackGameGlobal68a48c::~UnknownTrackGameGlobal68a48c()
{
    closesocket(field_0x00);
}

// 0x004ad3e0: connects to `address`:`port`, retrying for two seconds.
int UnknownTrackGameGlobal68a48c::UnknownFunction4ad3e0(const char* address, int port)
{
    sockaddr_in socketAddress;
    unsigned long nonBlocking;
    WSADATA data;

    field_0x10 = port;
    WSAStartup(0x101, &data);
    field_0x00 = socket(AF_INET, SOCK_STREAM, 0);
    hostent* host = gethostbyname(address);
    if (!host) {
        socketAddress.sin_family = AF_INET;
        socketAddress.sin_port = htons((unsigned short)field_0x10);
        socketAddress.sin_addr.s_addr = inet_addr(address);
    } else {
        socketAddress.sin_family = AF_INET;
        socketAddress.sin_port = htons((unsigned short)field_0x10);
        socketAddress.sin_addr.S_un.S_un_b.s_b1 = host->h_addr_list[0][0];
        socketAddress.sin_addr.S_un.S_un_b.s_b2 = host->h_addr_list[0][1];
        socketAddress.sin_addr.S_un.S_un_b.s_b3 = host->h_addr_list[0][2];
        socketAddress.sin_addr.S_un.S_un_b.s_b4 = host->h_addr_list[0][3];
    }
    field_0x0c = 0;
    field_0x04 = -1;
    int now = UnknownFunction4bfa80();
    int start = now;
    while (field_0x04 == -1) {
        field_0x04 = connect(field_0x00, (sockaddr*)&socketAddress, sizeof(socketAddress));
        if (field_0x04 == -1)
            now = UnknownFunction4bfa80();
        if (now - start > 2000)
            return 1;
    }
    field_0x0c = 1;
    field_0x04 = ioctlsocket(field_0x00, FIONBIO, 0);
    nonBlocking = 1;
    field_0x04 = ioctlsocket(field_0x00, FIONBIO, &nonBlocking);
    return field_0x04 == -1;
}

// 0x004ad530: sends `length` bytes; 0 when all were sent.
int UnknownTrackGameGlobal68a48c::UnknownFunction4ad530(const char* data, int length)
{
    field_0x04 = send(field_0x00, data, length, 0);
    if (field_0x04 == -1) {
        field_0x04 = WSAGetLastError();
        return field_0x04;
    }
    return field_0x04 == length ? 0 : -1;
}

// 0x004ad570: sends a string.
int UnknownTrackGameGlobal68a48c::UnknownFunction4ad570(const char* text)
{
    return UnknownFunction4ad530(text, strlen(text));
}

// 0x004ad5a0: formats a DirectPlay error report; retail discards the text.
void UnknownFunction4ad5a0(long result, const char* file, int line)
{
    char name[256];
    char message[1024];

    switch (result) {
    case 0:
        return;
    case 0x80004001:
        sprintf(name, "DPERR_UNSUPPORTED");
        break;
    case 0x8000000a:
        sprintf(name, "DPERR_PENDING");
        break;
    case 0x80004002:
        sprintf(name, "DPERR_NOINTERFACE");
        break;
    case 0x80004005:
        sprintf(name, "DPERR_GENERIC");
        break;
    case 0x8007000e:
        sprintf(name, "DPERR_NOMEMORY");
        break;
    case 0x8877000a:
        sprintf(name, "DPERR_ACCESSDENIED");
        break;
    case 0x88770005:
        sprintf(name, "DPERR_ALREADYINITIALIZED");
        break;
    case 0x80070057:
        sprintf(name, "DPERR_INVALIDPARAM");
        break;
    case 0x88770014:
        sprintf(name, "DPERR_ACTIVEPLAYERS");
        break;
    case 0x8877001e:
        sprintf(name, "DPERR_BUFFERTOOSMALL");
        break;
    case 0x88770028:
        sprintf(name, "DPERR_CANTADDPLAYER");
        break;
    case 0x88770032:
        sprintf(name, "DPERR_CANTCREATEGROUP");
        break;
    case 0x8877003c:
        sprintf(name, "DPERR_CANTCREATEPLAYER");
        break;
    case 0x88770046:
        sprintf(name, "DPERR_CANTCREATESESSION");
        break;
    case 0x88770050:
        sprintf(name, "DPERR_CAPSNOTAVAILABLEYET");
        break;
    case 0x8877005a:
        sprintf(name, "DPERR_EXCEPTION");
        break;
    case 0x88770078:
        sprintf(name, "DPERR_INVALIDFLAGS");
        break;
    case 0x88770082:
        sprintf(name, "DPERR_INVALIDOBJECT");
        break;
    case 0x88770096:
        sprintf(name, "DPERR_INVALIDPLAYER");
        break;
    case 0x8877009b:
        sprintf(name, "DPERR_INVALIDGROUP");
        break;
    case 0x887700a0:
        sprintf(name, "DPERR_NOCAPS");
        break;
    case 0x887700aa:
        sprintf(name, "DPERR_NOCONNECTION");
        break;
    case 0x887700be:
        sprintf(name, "DPERR_NOMESSAGES");
        break;
    case 0x887700c8:
        sprintf(name, "DPERR_NONAMESERVERFOUND");
        break;
    case 0x887700d2:
        sprintf(name, "DPERR_NOPLAYERS");
        break;
    case 0x887700dc:
        sprintf(name, "DPERR_NOSESSIONS");
        break;
    case 0x887700e6:
        sprintf(name, "DPERR_SENDTOOBIG");
        break;
    case 0x887700f0:
        sprintf(name, "DPERR_TIMEOUT");
        break;
    case 0x887700fa:
        sprintf(name, "DPERR_UNAVAILABLE");
        break;
    case 0x8877010e:
        sprintf(name, "DPERR_BUSY");
        break;
    case 0x88770118:
        sprintf(name, "DPERR_USERCANCEL");
        break;
    case 0x88770122:
        sprintf(name, "DPERR_CANNOTCREATESERVER");
        break;
    case 0x8877012c:
        sprintf(name, "DPERR_PLAYERLOST");
        break;
    case 0x88770136:
        sprintf(name, "DPERR_SESSIONLOST");
        break;
    case 0x88770140:
        sprintf(name, "DPERR_UNINITIALIZED");
        break;
    case 0x8877014a:
        sprintf(name, "DPERR_NONEWPLAYERS");
        break;
    case 0x88770154:
        sprintf(name, "DPERR_INVALIDPASSWORD");
        break;
    case 0x8877015e:
        sprintf(name, "DPERR_CONNECTING");
        break;
    case 0x88770168:
        sprintf(name, "DPERR_CONNECTIONLOST");
        break;
    case 0x88770172:
        sprintf(name, "DPERR_UNKNOWNMESSAGE");
        break;
    case 0x8877017c:
        sprintf(name, "DPERR_CANCELFAILED");
        break;
    case 0x88770186:
        sprintf(name, "DPERR_INVALIDPRIORITY");
        break;
    case 0x887701a4:
        sprintf(name, "DPERR_ABORTED");
        break;
    case 0x8877019a:
        sprintf(name, "DPERR_CANCELLED");
        break;
    case 0x88770190:
        sprintf(name, "DPERR_NOTHANDLED");
        break;
    case 0x887703e8:
        sprintf(name, "DPERR_BUFFERTOOLARGE");
        break;
    case 0x88770406:
        sprintf(name, "DPERR_INVALIDINTERFACE");
        break;
    case 0x887703fc:
        sprintf(name, "DPERR_APPNOTSTARTED");
        break;
    case 0x887703f2:
        sprintf(name, "DPERR_CANTCREATEPROCESS");
        break;
    case 0x88770410:
        sprintf(name, "DPERR_NOSERVICEPROVIDER");
        break;
    case 0x88770438:
        sprintf(name, "DPERR_SERVICEPROVIDERLOADED");
        break;
    case 0x8877042e:
        sprintf(name, "DPERR_NOTLOBBIED");
        break;
    case 0x8877041a:
        sprintf(name, "DPERR_UNKNOWNAPPLICATION");
        break;
    case 0x88770442:
        sprintf(name, "DPERR_ALREADYREGISTERED");
        break;
    case 0x887707da:
        sprintf(name, "DPERR_CANTLOADSSPI");
        break;
    case 0x887707d0:
        sprintf(name, "DPERR_AUTHENTICATIONFAILED");
        break;
    case 0x8877044c:
        sprintf(name, "DPERR_NOTREGISTERED");
        break;
    case 0x887707e4:
        sprintf(name, "DPERR_ENCRYPTIONFAILED");
        break;
    case 0x88770802:
        sprintf(name, "DPERR_ENCRYPTIONNOTSUPPORTED");
        break;
    case 0x887707f8:
        sprintf(name, "DPERR_CANTLOADSECURITYPACKAGE");
        break;
    case 0x887707ee:
        sprintf(name, "DPERR_SIGNFAILED");
        break;
    case 0x8877080c:
        sprintf(name, "DPERR_CANTLOADCAPI");
        break;
    case 0x88770820:
        sprintf(name, "DPERR_LOGONDENIED");
        break;
    case 0x88770816:
        sprintf(name, "DPERR_NOTLOGGEDIN");
        break;
    default:
        sprintf(name, "Unknown Error");
        break;
    }
    sprintf(message, "\nERROR: DirectPlay (%s) in file %s at line %d\n", name, file, line);
}

// 0x004add10 (out of line: code at 0x0044b634 and 0x0044c0f5 passes its
// address to the vector constructor)
ConnectionInfoType::ConnectionInfoType()
{
    field_0x04[0] = 0;
    field_0x118 = 0;
    field_0x11c = 0;
}

// 0x004add30
void ConnectionInfoType::UnknownFunction4add30(NetworkInterface* owner)
{
    field_0x04[0] = 0;
    field_0x118 = 0;
    field_0x11c = owner;
}

// 0x004add50: lists up to five service provider connections.
long NetworkInterface::UnknownFunction4add50(ConnectionInfoType* connections, int* count)
{
    for (int i = 0; i < 5; i++)
        connections[i].UnknownFunction4add30(this);
    if (!field_0x04)
        return NET_DPERR_INVALIDOBJECT;
    g_UnknownGlobal6886ac = 0;
    long result = field_0x04->EnumConnections(&g_UnknownGuid556dc0, UnknownFunction4addc0,
                                              connections, 0);
    *count = g_UnknownGlobal6886ac;
    return result;
}

// 0x004addc0: EnumConnections callback; records connections that initialize.
int __stdcall UnknownFunction4addc0(const GUID* provider, void* connection, unsigned long size,
                                    const NetName* name, unsigned long, void* context)
{
    ConnectionInfoType* connections = (ConnectionInfoType*)context;
    UnknownDirectPlay4A* directPlay = 0;
    long result = 0;

    if ((*provider == DPSPGUID_IPX || *provider == DPSPGUID_MODEM ||
         *provider == DPSPGUID_TCPIP || *provider == DPSPGUID_SERIAL) &&
        *provider != DPSPGUID_SERIAL) {
        void* address;
        unsigned long addressSize;
        if (!connections->field_0x11c)
            goto failed;
        if (!connections->field_0x11c->UnknownFunction4abf10(*provider, "", "", 0, &address,
                                                             &addressSize))
            goto failed;
        result = CoCreateInstance(CLSID_DirectPlay, 0, CLSCTX_INPROC_SERVER, IID_IDirectPlay4A,
                                  (void**)&directPlay);
        if (result)
            goto failed;
        result = directPlay->InitializeConnection(address, 0);
        if (result)
            goto failed;
    }
    {
        ConnectionInfoType* entry;
        int length = strlen(name->shortName);
        int count = length > 0x103 ? 0x103 : length;
        strncpy(connections[g_UnknownGlobal6886ac].field_0x04, name->shortName, count);
        connections[g_UnknownGlobal6886ac].field_0x04[count] = 0;
        connections[g_UnknownGlobal6886ac].field_0x118 = DebugMalloc(size, __FILE__, 2553);
        if (!connections[g_UnknownGlobal6886ac].field_0x118)
            goto failed;
        memcpy(connections[g_UnknownGlobal6886ac].field_0x118, connection, size);
        entry = &connections[g_UnknownGlobal6886ac];
        *(GUID*)entry->field_0x108 = *provider;
        if (++g_UnknownGlobal6886ac >= 5)
            return 0;
    }
failed:
    UnknownFunction4ad5a0(result, __FILE__, 2564);
    if (directPlay)
        directPlay->Release();
    return 1;
}

// 0x004adff0: lists up to five sessions.
long NetworkInterface::UnknownFunction4adff0(SessionInfoType* sessions, int* count,
                                             unsigned long flags)
{
    if (sessions) {
        for (int i = 0; i < 5; i++)
            sessions[i].UnknownVirtualSlot1();
    }
    if (!field_0x04)
        return NET_DPERR_INVALIDOBJECT;
    NetSessionDesc desc;
    memset(&desc, 0, sizeof(desc));
    desc.size = sizeof(desc);
    *(GUID*)desc.application = g_UnknownGuid556dc0;
    g_UnknownGlobal6886b0 = 0;
    long result = field_0x04->EnumSessions(&desc, 0, UnknownFunction4ae100, sessions, flags);
    while (result == NET_DPERR_CONNECTING)
        result = field_0x04->EnumSessions(&desc, 0, UnknownFunction4ae100, sessions, flags);
    *count = g_UnknownGlobal6886b0;
    if (result < 0) {
        UnknownFunction4ad5a0(result, __FILE__, 2665);
        return result;
    }
    return 0;
}

// 0x004ae100: EnumSessions callback.
int __stdcall UnknownFunction4ae100(const NetSessionDesc* desc, unsigned long*,
                                    unsigned long flags, void* context)
{
    if (flags & 1)
        return 0;
    SessionInfoType* sessions = (SessionInfoType*)context;
    if (sessions) {
        if (!desc->sessionName)
            return 0;
        int length = strlen(desc->sessionName);
        int count = length > 0x103 ? 0x103 : length;
        strncpy(sessions[g_UnknownGlobal6886b0].field_0x04, desc->sessionName, count);
        sessions[g_UnknownGlobal6886b0].field_0x04[count] = 0;
        sessions[g_UnknownGlobal6886b0].field_0x108 = DebugMalloc(sizeof(GUID), __FILE__, 2616);
        GUID* instance = (GUID*)sessions[g_UnknownGlobal6886b0].field_0x108;
        if (instance) {
            *instance = *(const GUID*)desc->instance;
            if (++g_UnknownGlobal6886b0 >= 5)
                return 0;
        }
    }
    return 1;
}

// 0x004ae200: lists up to seven players of `session`.
long NetworkInterface::UnknownFunction4ae200(GUID* session, PlayerInfoType* players, int* count)
{
    for (int i = 0; i < 7; i++)
        players[i].UnknownVirtualSlot1();
    if (!field_0x04)
        return NET_DPERR_INVALIDOBJECT;
    g_UnknownGlobal6886b4 = 0;
    long result = field_0x04->EnumPlayers(session, UnknownFunction4ae270, players, 0x10);
    *count = g_UnknownGlobal6886b4;
    return result;
}

// 0x004ae270: EnumPlayers callback.
int __stdcall UnknownFunction4ae270(unsigned long id, unsigned long, const NetName* name,
                                    unsigned long, void* context)
{
    PlayerInfoType* players = (PlayerInfoType*)context;
    players[g_UnknownGlobal6886b4].field_0x04 = id;
    int length = strlen(name->shortName);
    int count = length > 15 ? 15 : length;
    strncpy(players[g_UnknownGlobal6886b4].field_0x08, name->shortName, count);
    players[g_UnknownGlobal6886b4].field_0x08[count] = 0;
    return ++g_UnknownGlobal6886b4 < 7;
}

// 0x004adc90
void SessionInfoType::UnknownVirtualSlot1()
{
    field_0x04[0] = 0;
    if (field_0x108) {
        operator delete(field_0x108, __FILE__, 2472);
        field_0x108 = 0;
    }
}
