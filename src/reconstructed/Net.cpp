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
void ReportDirectPlayError(long result, const char* file, int line);

// 0x004af6a0 (NetThread.cpp): the receive thread.
unsigned int __stdcall ReceiveThreadProc(void* net);

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
extern const GUID g_NetApplicationGuid;

int __stdcall EnumConnectionsCallback(const GUID* provider, void* connection, unsigned long size,
                                    const NetName* name, unsigned long flags, void* context);
int __stdcall EnumSessionsCallback(const NetSessionDesc* desc, unsigned long* timeout,
                                    unsigned long flags, void* context);
int __stdcall EnumPlayersCallback(unsigned long id, unsigned long type, const NetName* name,
                                    unsigned long flags, void* context);


// 0x004aac40: EnumAddress callback; copies the INet address string.
int __stdcall EnumAddressCallback(const GUID& type, unsigned long size, const void* data,
                                    void* context)
{
    if (type == DPAID_INet)
        strcpy((char*)context, (const char*)data);
    return 1;
}

// 0x004aac90
NetMessage::NetMessage()
{
    type = 0;
    from = 0;
    to = 0;
    flags = 0;
    size = 0;
    memset(data, 0, sizeof(data));
}

// 0x004aacc0
void NetMessage::Set(void* source, unsigned int length, int sender, int receiver, int messageFlags)
{
    char text[256];

    if (!sender)
        type = *(int*)source;
    else
        type = *(unsigned char*)source;
    from = sender;
    to = receiver;
    flags = messageFlags;
    size = length;
    memcpy(data, source, length);
    if (length >= NET_MAX_GENERIC_MSG_SIZE)
        sprintf(text, "dwMsgSize (%d)NET_MAX_GENERIC_MSG_SIZE not large enough\n", length);
}

// 0x004aad50
NetIncomingMessage::NetIncomingMessage()
{
}

// 0x004aad60
void NetIncomingMessage::SetReceived(void* source, unsigned int length, int sender, int receiver)
{
    Set(source, length, sender, receiver, 0);
}

// 0x004aad80
NetPendingMessage::NetPendingMessage()
{
    memset(this, 0, sizeof(NetMessage));
    sendSize = 0;
    resendTimer = 0;
    next = 0;
    field_0x900 = 0;
}

// 0x004aae20 (identical-code folded with another TU's one-pointer constructor)
NetPendingList::NetPendingList()
{
    head = 0;
}

// 0x004aae30
NetPendingList::~NetPendingList()
{
    NetPendingMessage* message = head;
    while (message) {
        NetPendingMessage* current = message;
        message = message->next;
        delete current;
    }
}

// 0x004aae50: appends a guaranteed message.
void NetPendingList::Append(int type, short sequence, void* data,
                                           unsigned int size, int from, int to)
{
    if (!head) {
        NetPendingMessage* message = new(__FILE__, 209) NetPendingMessage;
        if (message && message->SetPending(type, sequence, data, size, from, to))
            head = message;
        return;
    }
    NetPendingMessage* last;
    for (NetPendingMessage* message = head; message; message = message->next)
        last = message;
    last->next = new(__FILE__, 218) NetPendingMessage;
    if (last->next)
        last->next->SetPending(type, sequence, data, size, from, to);
}

// 0x004aaf20: resends every message unacknowledged for four seconds.
long NetPendingList::ResendExpired(NetworkInterface* net, int unused)
{
    long result = 0;
    for (NetPendingMessage* message = head; message; message = message->next) {
        message->resendTimer += g_TrackGame->frameTime;
        if (message->resendTimer > 4.0f) {
            result = net->directPlay->SendEx(message->from, message->to, 0x600,
                                             message->data, message->sendSize, 0, 0, 0, 0);
            if (result == NET_DPERR_INVALIDPLAYER)
                return NET_DPERR_INVALIDPLAYER;
            if (result != 0 && result != NET_E_PENDING)
                ReportDirectPlayError(result, __FILE__, 269);
            message->resendTimer = 0;
        }
    }
    return result;
}

// 0x004aafe0: drops the acknowledged message.
void NetPendingList::Acknowledge(short sequence)
{
    if (!head)
        return;
    NetPendingMessage* message = head;
    NetPendingMessage* previous = message;
    while (message) {
        if (*(short*)&message->data[2] == sequence)
            break;
        previous = message;
        message = message->next;
    }
    if (!message)
        return;
    if (head == message) {
        head = message->next;
        delete message;
    } else {
        previous->next = message->next;
        delete message;
    }
}

// 0x004ab050
NetPlayer::NetPlayer(int playerId, const char* playerName)
{
    id = playerId;
    int length = strlen(playerName);
    int count = length > 15 ? 15 : length;
    strncpy(name, playerName, count);
    name[count] = 0;
    pending = new(__FILE__, 314) NetPendingList;
    lastKeepAliveTime = 0;
    next = 0;
}

// 0x004ab0d0
NetPlayer::~NetPlayer()
{
    if (pending)
        delete pending;
}

// 0x004ab0f0
NetFile::NetFile()
{
    data = 0;
    chunkReceived = 0;
    size = 0;
    strcpy(name, "");
    sourceId = 0;
    destination = 0;
    chunkCount = 0;
    chunksDone = 0;
    onComplete = 0;
    onCompleteContext = 0;
    sending = 0;
    receiving = 0;
    sent = 0;
    received = 0;
}

// 0x004ab160
NetFile::~NetFile()
{
    if (data)
        DebugFree(data, __FILE__, 347);
    if (chunkReceived)
        delete chunkReceived;
}

// 0x004ab190
void NetFile::Update(int unused)
{
    if (sending)
        SendNextChunk();
}

// 0x004ab1b0: sends the next chunk.
void NetFile::SendNextChunk()
{
    NetFileChunk chunk;

    sourceId = 0;
    int offset = chunksDone * sizeof(chunk.data);
    int count = size - offset;
    if (count >= (int)sizeof(chunk.data))
        count = sizeof(chunk.data);
    chunk.byteCount = count;
    memcpy(chunk.data, data + offset, count);
    chunk.index = chunksDone;
    g_TrackGame->network->Send(0xcf, &chunk, sizeof(chunk), 0,
                                                             destination);
    chunksDone++;
    if (chunksDone >= chunkCount) {
        sending = 0;
        sent = 1;
        onComplete(onCompleteContext);
    }
}

// 0x004ab280: starts receiving the announced file.
void NetFile::BeginReceive(int id, NetFileHeader* header)
{
    sourceId = id;
    int length = strlen(header->name);
    int count = length > 0x103 ? 0x103 : length;
    strncpy(name, header->name, count);
    name[count] = 0;
    size = header->size;
    if (data)
        DebugFree(data, __FILE__, 497);
    data = (char*)DebugMalloc(size, __FILE__, 499);
    chunkCount = size / (int)sizeof(((NetFileChunk*)0)->data) + 1;
    chunksDone = 0;
    chunkReceived = new(__FILE__, 503) int[chunkCount];
    for (int i = 0; i < chunkCount; i++)
        chunkReceived[i] = 0;
    receiving = 1;
}

// 0x004ab380: stores a received chunk.
void NetFile::ReceiveChunk(int id, NetFileChunk* chunk)
{
    if (sourceId != id)
        return;
    if (chunkReceived[chunk->index])
        return;
    chunkReceived[chunk->index] = 1;
    memcpy(data + chunk->index * sizeof(chunk->data), chunk->data,
           chunk->byteCount);
    chunksDone++;
    if (chunksDone == chunkCount) {
        receiving = 0;
        Save();
        DebugFree(data, __FILE__, 534);
        data = 0;
        received = 1;
    }
}

// 0x004ab440: writes the received file.
void NetFile::Save()
{
    FILE* file = fopen(name, "wb");
    if (file) {
        fwrite(data, 1, size, file);
        fclose(file);
    }
}

// 0x004ab480
NetworkInterface::NetworkInterface()
{
    directPlay = 0;
    lobby = 0;
    localPlayer = 0;
    isHost = 0;
    lobbyConnected = 0;
    connectionMode = 0;
    receiveThread = 0;
    receiveThreadId = 0;
    receiveEvent = 0;
    quitEvent = 0;
    keepAliveEvent = 0;
    players = 0;
    messages = 0;
    field_0x38 = 0;
    field_0x3c = 0;
    nextMessage = 0;
    nextSequence = 0;
    InitializeCriticalSection(NET_LOCK(lock));
    InitializeCriticalSection(NET_LOCK(keepAliveLock));
    keepAliveRunning = 0;
    keepAliveTimeout = 0;
    keepAliveInterval = 0;
    for (int i = 0; i < 8; i++)
        removedPlayers[i] = 0;
    memset(&guaranteedCaps, 0, sizeof(guaranteedCaps));
    memset(&caps, 0, sizeof(caps));
    providerKind = 0;
    serverPlayer = 0;
    group = 0;
}

// 0x004ab570
NetworkInterface::~NetworkInterface()
{
    Shutdown();
    EnterCriticalSection(NET_LOCK(lock));
    NetPlayer* player = players;
    while (player) {
        NetPlayer* current = player;
        player = player->next;
        delete current;
    }
    if (messages) {
        delete messages;
        messages = 0;
    }
    for (int i = 0; i < files.m_count; i++) {
        NetFile* file = files.Get(i);
        if (file)
            delete file;
    }
    files.Clear();
    LeaveCriticalSection(NET_LOCK(lock));
    DeleteCriticalSection(NET_LOCK(lock));
    DeleteCriticalSection(NET_LOCK(keepAliveLock));
}

// 0x004ab6b0: starts DirectPlay in connection `mode` (1..4); 0 or an HRESULT.
long NetworkInterface::Initialize(int mode)
{
    PlayerInfoType found[7];
    int count;
    long result;

    connectionMode = mode;
    receiveEvent = CreateEventA(0, 0, 0, 0);
    quitEvent = CreateEventA(0, 0, 0, 0);
    keepAliveEvent = CreateEventA(0, 0, 0, 0);
    if (!receiveEvent || !quitEvent || !keepAliveEvent)
        goto outOfMemory;
    messages = new(__FILE__, 650) NetMessage[0x100];
    if (!messages)
        goto outOfMemory;
    receiveThread = (void*)_beginthreadex(0, 0, ReceiveThreadProc, this, 0, &receiveThreadId);
    if (!receiveThread)
        goto outOfMemory;
    result = ConnectUsingLobby();
    if (result < 0 && (mode == 4 || result != NET_DPERR_NOTLOBBIED))
        goto failed;
    if (!lobbyConnected) {
        result = CoCreateInstance(CLSID_DirectPlay, 0, CLSCTX_INPROC_SERVER, IID_IDirectPlay4A,
                                  (void**)&directPlay);
        if (result < 0)
            goto failed;
    }
    if (lobbyConnected) {
        if (isHost) {
            if (!CreateGroup(localPlayer))
                goto failed;
        } else {
            if (!FindGroup())
                goto failed;
        }
    }
    if (EnumPlayers(0, found, &count) >= 0) {
        for (int i = 0; i < count; i++)
            AddPlayer(found[i].id, found[i].name);
    }
    if (connectionMode == 3 && isHost)
        heldMessages.Init(16, 16);
    files.Init(2, 1);
    return 0;

outOfMemory:
    result = NET_E_OUTOFMEMORY;
failed:
    Shutdown();
    return result;
}

// 0x004abd40: strips the trailer DPlay appended to a received message.
void NetworkInterface::StripMessageTrailer(unsigned char** data, unsigned long* size,
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
    DebugFree(message, __FILE__, 929);
    *size -= trailerSize;
}

// 0x004abdf0: initializes the service provider connection.
int NetworkInterface::InitializeConnection(void* connection, int, int kind)
{
    if (directPlay->InitializeConnection(connection, 0))
        return 0;
    guaranteedCaps.size = sizeof(guaranteedCaps);
    if (directPlay->GetCaps(&guaranteedCaps, 1) < 0)
        memset(&guaranteedCaps, 0, sizeof(guaranteedCaps));
    caps.size = sizeof(caps);
    if (directPlay->GetCaps(&caps, 0) < 0)
        memset(&caps, 0, sizeof(caps));
    providerKind = kind;
    return 1;
}

// 0x004abe70: stops the thread and releases DirectPlay.
long NetworkInterface::Shutdown()
{
    if (keepAliveRunning)
        StopKeepAlive();
    if (receiveThread) {
        SetEvent(quitEvent);
        WaitForSingleObject(receiveThread, INFINITE);
    }
    if (receiveThread) {
        CloseHandle(receiveThread);
        receiveThread = 0;
    }
    if (receiveEvent) {
        CloseHandle(receiveEvent);
        receiveEvent = 0;
    }
    if (quitEvent) {
        CloseHandle(quitEvent);
        quitEvent = 0;
    }
    if (keepAliveEvent) {
        CloseHandle(keepAliveEvent);
        keepAliveEvent = 0;
    }
    if (directPlay) {
        if (localPlayer) {
            directPlay->DestroyPlayer(localPlayer);
            localPlayer = 0;
        }
        directPlay->Close();
        if (directPlay) {
            directPlay->Release();
            directPlay = 0;
        }
    }
    return 0;
}

// 0x004ac2d0: the local player's INet address.
int NetworkInterface::GetLocalAddress(char* address)
{
    strcpy(address, "");
    if (!(providerKind & 6))
        return 0;
    unsigned long size;
    long result = directPlay->GetPlayerAddress(localPlayer, 0, &size);
    if (result != NET_DPERR_BUFFERTOOSMALL) {
        ReportDirectPlayError(result, __FILE__, 1158);
        return 0;
    }
    void* data = DebugMalloc(size, __FILE__, 1162);
    if (directPlay->GetPlayerAddress(localPlayer, data, &size))
        goto failed;
    if (lobby->EnumAddress(EnumAddressCallback, data, size, address))
        goto failed;
    DebugFree(data, __FILE__, 1172);
    return 1;
failed:
    return 0;
}

// 0x004ac3c0: creates and opens a session named `name`.
int NetworkInterface::CreateSession(char* name, unsigned long flags)
{
    if (!directPlay)
        return 0;
    unsigned long sessionFlags;
    NetSessionDesc desc;
    GetSessionFlags(&sessionFlags);
    memset(&desc, 0, sizeof(desc));
    desc.size = sizeof(desc);
    desc.flags = sessionFlags;
    *(GUID*)desc.application = g_NetApplicationGuid;
    desc.maxPlayers = 8;
    desc.sessionName = name;
    if (directPlay->Open(&desc, flags | 2)) {
        directPlay->Close();
        return 0;
    }
    isHost = 1;
    return 1;
}

// 0x004ac480: joins the session `instance`.
int NetworkInterface::JoinSession(const GUID* instance, unsigned long flags)
{
    if (!directPlay)
        return 0;
    NetSessionDesc desc;
    memset(&desc, 0, sizeof(desc));
    desc.size = sizeof(desc);
    *(GUID*)desc.instance = *instance;
    if (directPlay->Open(&desc, flags | 1))
        goto failed;
    isHost = 0;
    if (!FindGroup())
        goto failed;
    return 1;
failed:
    directPlay->Close();
    return 0;
}

// 0x004ac510: closes (value 0) or opens the session to new players.
int NetworkInterface::SetSessionJoinable(int value)
{
    unsigned long size;

    if (!directPlay || !isHost)
        return 0;
    NetSessionDesc* desc = 0;
    long result = directPlay->GetSessionDesc(desc, &size);
    if (result == NET_DPERR_BUFFERTOOSMALL) {
        desc = (NetSessionDesc*)DebugMalloc(size, __FILE__, 1254);
        if (!desc)
            return 0;
        if (directPlay->GetSessionDesc(desc, &size))
            goto failed;
    }
    if (value)
        desc->flags &= ~1;
    else
        desc->flags |= 1;
    if (directPlay->SetSessionDesc(desc, 0))
        goto failed;
    DebugFree(desc, __FILE__, 1274);
    return 1;
failed:
    if (desc)
        DebugFree(desc, __FILE__, 1279);
    return 0;
}

// 0x004ac5e0: creates the local player and joins or creates the group.
int NetworkInterface::CreateLocalPlayer(char* name)
{
    unsigned long player;

    if (!CreatePlayer(name, &player))
        goto failed;
    localPlayer = player;
    if (isHost) {
        if (!CreateGroup(player))
            goto failed;
    } else {
        FindGroup();
    }
    return 1;
failed:
    return 0;
}

// 0x004ac630
int NetworkInterface::DestroyLocalPlayer()
{
    if (localPlayer) {
        int result = DestroyPlayer(localPlayer);
        localPlayer = 0;
        return result;
    }
    return 0;
}

// 0x004ac650
int NetworkInterface::CreatePlayer(char* name, unsigned long* player)
{
    NetName playerName;

    if (!directPlay)
        return 0;
    memset(&playerName, 0, sizeof(playerName));
    playerName.size = sizeof(playerName);
    playerName.shortName = name;
    playerName.longName = 0;
    if (isHost && connectionMode == 2) {
        if (directPlay->CreatePlayer(player, &playerName, receiveEvent, 0, 0, 0x100))
            goto failed;
    } else {
        if (directPlay->CreatePlayer(player, &playerName, receiveEvent, 0, 0, 0))
            goto failed;
    }
    return 1;
failed:
    return 0;
}

// 0x004ac6f0
int NetworkInterface::DestroyPlayer(unsigned long player)
{
    if (directPlay && player && directPlay->DestroyPlayer(player) == 0)
        return 1;
    return 0;
}

// 0x004ac720
int NetworkInterface::GetPlayerName(int player, char* name)
{
    strcpy(name, "");
    NetName* playerName = 0;
    if (!QueryPlayerName(player, &playerName))
        goto failed;
    strncpy(name, playerName->shortName, 16);
    DebugFree(playerName, __FILE__, 1365);
    return 1;
failed:
    if (playerName)
        DebugFree(playerName, __FILE__, 1370);
    return 0;
}

// 0x004ac800
NetPlayer* NetworkInterface::FindPlayer(int id)
{
    NetPlayer* player = players;
    if (!player)
        return 0;
    while (player) {
        if (player->id == id)
            break;
        player = player->next;
    }
    if (!player)
        return 0;
    return player;
}

// 0x004ac830: sends a message; types with bit 7 set are guaranteed.
int NetworkInterface::Send(int type, void* data, int size, int from, int to)
{
    if (!from)
        from = localPlayer;
    if (type & 0x80) {
        SendGuaranteed(type, data, size, from);
    } else if (connectionMode == 3 && !isHost) {
        int target = to;
        if (!SendRaw(type, data, size, 0, from, serverPlayer, &target, sizeof(target)))
            return 0;
    } else {
        if (!SendRaw(type, data, size, 0, from, to, 0, 0))
            return 0;
    }
    return 1;
}

// 0x004ac8d0
int NetworkInterface::StartKeepAlive(float a, float b)
{
    EnterCriticalSection(NET_LOCK(keepAliveLock));
    keepAliveRunning = 1;
    keepAliveTimeout = a;
    keepAliveInterval = b;
    int index = 0;
    unsigned int time = UnknownFunction4bfa80();
    NetPlayer* player;
    while ((player = NextPlayer(&index)) != 0)
        player->lastKeepAliveTime = time;
    LeaveCriticalSection(NET_LOCK(keepAliveLock));
    SetEvent(keepAliveEvent);
    return 1;
}

// 0x004ac950
int NetworkInterface::StopKeepAlive()
{
    EnterCriticalSection(NET_LOCK(keepAliveLock));
    keepAliveRunning = 0;
    LeaveCriticalSection(NET_LOCK(keepAliveLock));
    return 1;
}

// 0x004ac980: adds a player unless known or removed.
void NetworkInterface::AddPlayer(int id, const char* name)
{
    for (int i = 0; i < 8; i++) {
        if (id == removedPlayers[i])
            return;
    }
    if (!players) {
        players = new(__FILE__, 1595) NetPlayer(id, name);
        return;
    }
    NetPlayer* last;
    NetPlayer* player = players;
    while (player) {
        if (player->id == id)
            return;
        last = player;
        player = player->next;
    }
    last->next = new(__FILE__, 1603) NetPlayer(id, name);
}

// 0x004aca80: removes a player and remembers its id.
void NetworkInterface::RemovePlayer(int id)
{
    if (!players)
        return;
    NetPlayer* player = players;
    NetPlayer* previous = player;
    while (player) {
        if (player->id == id)
            break;
        previous = player;
        player = player->next;
    }
    if (!player)
        return;
    if (players == player)
        players = player->next;
    else
        previous->next = player->next;
    for (int i = 0; i < 8; i++) {
        if (!removedPlayers[i]) {
            removedPlayers[i] = player->id;
            break;
        }
    }
    delete player;
}

// 0x004acb00: stores a received message in the next free ring slot.
void NetworkInterface::QueueMessage(void* data, unsigned int size, int from, int to,
                                             int flags)
{
    int tries = 0;
    while (messages[nextMessage].type & 0x80) {
        nextMessage++;
        if (++tries >= 0x100) {
            field_0x38 = 1;
            return;
        }
        if (nextMessage >= 0x100) {
            nextMessage = 0;
            field_0x3c = 0x100;
            field_0x38 = 1;
        }
    }
    if (!from)
        messages[nextMessage].Set(data, size, 0, to, flags);
    else
        messages[nextMessage].Set(data, size, from, to, flags);
    nextMessage++;
    if (nextMessage >= 0x100) {
        nextMessage = 0;
        field_0x3c = 0x100;
        field_0x38 = 1;
    }
}

// 0x004acbc0: sends a guaranteed message to every other player.
void NetworkInterface::SendGuaranteed(int type, void* data, int size, int from)
{
    EnterCriticalSection(NET_LOCK(lock));
    for (NetPlayer* player = players; player; player = player->next) {
        if (player->id && localPlayer != player->id) {
            SendRaw(type, data, size, nextSequence, from, player->id, 0, 0);
            player->pending->Append(type, nextSequence, data, size, from,
                                                      player->id);
            nextSequence++;
        }
    }
    LeaveCriticalSection(NET_LOCK(lock));
}

// 0x004acc50: resends pending messages; drops a player DirectPlay no longer knows.
void NetworkInterface::ResendPending(int value)
{
    int lost = 0;
    NetPlayer* player = players;
    if (!player)
        return;
    int lostPlayer = value;
    for (; player; player = player->next) {
        if (player->pending->ResendExpired(this, value) == NET_DPERR_INVALIDPLAYER) {
            lostPlayer = player->id;
            lost = 1;
        }
    }
    if (lost)
        RemovePlayer(lostPlayer);
}

// 0x004acca0: an acknowledgement for `sequence`.
void NetworkInterface::AcknowledgeMessage(short sequence)
{
    for (NetPlayer* player = players; player; player = player->next) {
        if (localPlayer != player->id)
            player->pending->Acknowledge(sequence);
    }
}

// 0x004accd0: the host creates the group and joins it.
int NetworkInterface::CreateGroup(unsigned long player)
{
    if (directPlay->CreateGroup(&group, 0, 0, 0, 0))
        goto failed;
    if (directPlay->AddPlayerToGroup(group, player))
        goto failed;
    serverPlayer = player;
    return 1;
failed:
    return 0;
}

// 0x004acd20: a client finds the group and its server player.
int NetworkInterface::FindGroup()
{
    if (directPlay->EnumGroups(0, (void*)EnumGroupsCallback, this, 0))
        goto failed;
    if (group &&
        directPlay->EnumGroupPlayers(group, 0, (void*)EnumGroupPlayersCallback, this, 0))
        goto failed;
    return 1;
failed:
    return 0;
}

// 0x004acd70
int __stdcall NetworkInterface::EnumGroupsCallback(unsigned long id, unsigned long type,
                                                      const NetName* name, unsigned long flags,
                                                      void* context)
{
    ((NetworkInterface*)context)->group = id;
    return 1;
}

// 0x004acd90
int __stdcall NetworkInterface::EnumGroupPlayersCallback(unsigned long id, unsigned long type,
                                                      const NetName* name, unsigned long flags,
                                                      void* context)
{
    ((NetworkInterface*)context)->serverPlayer = id;
    return 1;
}

// 0x004acdb0: DPSESSION flags for the connection mode.
void NetworkInterface::GetSessionFlags(unsigned long* flags)
{
    *flags = 0xa040;
    switch (connectionMode) {
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
int NetworkInterface::Update(int value)
{
    EnterCriticalSection(NET_LOCK(lock));
    DispatchMessages(value);
    if (connectionMode == 3 && isHost)
        UnknownVirtualSlot0(value);
    for (int i = 0; i < files.m_count; i++) {
        NetFile* file = files.Get(i);
        file->Update(value);
        if (file->sent || file->received) {
            files.Remove(file);
            if (file)
                delete file;
        }
    }
    LeaveCriticalSection(NET_LOCK(lock));
    return 1;
}

// 0x004ad050 (slot 0): sends the held messages to the group.
void NetworkInterface::UnknownVirtualSlot0(int)
{
    for (int i = 0; i < heldMessages.m_count; i++) {
        NetIncomingMessage* message = heldMessages.Get(i);
        SendRaw(message->type, message->data, message->size, 0,
                              localPlayer, message->to, 0, 0);
        delete message;
    }
    heldMessages.Clear();
}

// 0x004ad0f0: the player's DPNAME, allocated; 0 on failure.
int NetworkInterface::QueryPlayerName(int player, NetName** name)
{
    if (!directPlay)
        return 0;
    unsigned long size;
    void* data;
    if (directPlay->GetPlayerName(player, 0, &size) != NET_DPERR_BUFFERTOOSMALL)
        goto failed;
    data = DebugMalloc(size, __FILE__, 1999);
    if (!data)
        goto failed;
    if (directPlay->GetPlayerName(player, data, &size)) {
        DebugFree(data, __FILE__, 2015);
        goto failed;
    }
    *name = (NetName*)data;
    return 1;
failed:
    return 0;
}

// 0x004ad180: reads the session caps and the service provider kind.
void NetworkInterface::ReadProviderCaps(const GUID* provider)
{
    memset(&guaranteedCaps, 0, sizeof(guaranteedCaps));
    memset(&caps, 0, sizeof(caps));
    guaranteedCaps.size = sizeof(guaranteedCaps);
    if (directPlay->GetCaps(&guaranteedCaps, 1) < 0)
        memset(&guaranteedCaps, 0, sizeof(guaranteedCaps));
    caps.size = sizeof(caps);
    if (directPlay->GetCaps(&caps, 0) < 0) {
        memset(&caps, 0, sizeof(caps));
        return;
    }
    if (*provider == DPSPGUID_IPX)
        providerKind = 1;
    else if (*provider == DPSPGUID_MODEM)
        providerKind = 0x10;
    else if (*provider == DPSPGUID_TCPIP)
        providerKind = 4;
    else if (*provider == DPSPGUID_SERIAL)
        providerKind = 8;
}

// 0x004ad280: sends `data`, appending `extra` when given.
int NetworkInterface::SendRaw(int type, void* data, int size, short sequence,
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
        result = directPlay->SendEx(from, to, 0x600, buffer, total, 0, 0, 0, 0);
        DebugFree(buffer, __FILE__, 2182);
    } else {
        result = directPlay->SendEx(from, to, 0x600, data, size, 0, 0, 0, 0);
    }
    if (result && result != NET_E_PENDING) {
        ReportDirectPlayError(result, __FILE__, 2264);
        return 0;
    }
    return 1;
}

// 0x006886ac, 0x006886b0, 0x006886b4: entries filled by the enumeration callbacks.
int g_EnumConnectionCount;
int g_EnumSessionCount;
int g_EnumPlayerCount;

// 0x004ad3b0..0x004ad592: a WinSock stream client (TrackGame's global at
// 0x0068a48c). Placement between Net.cpp functions is the ownership evidence.
DebugSocket::DebugSocket()
{
    field_0x00 = 0;
    field_0x08 = 1000;
    field_0x0c = 0;
}

DebugSocket::~DebugSocket()
{
    closesocket(field_0x00);
}

// 0x004ad3e0: connects to `address`:`port`, retrying for two seconds.
int DebugSocket::Connect(const char* address, int port)
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
int DebugSocket::SendData(const char* data, int length)
{
    field_0x04 = send(field_0x00, data, length, 0);
    if (field_0x04 == -1) {
        field_0x04 = WSAGetLastError();
        return field_0x04;
    }
    return field_0x04 == length ? 0 : -1;
}

// 0x004ad570: sends a string.
int DebugSocket::SendText(const char* text)
{
    return SendData(text, strlen(text));
}

// 0x004ad5a0: formats a DirectPlay error report; retail discards the text.
void ReportDirectPlayError(long result, const char* file, int line)
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
    name[0] = 0;
    connection = 0;
    owner = 0;
}

// 0x004add30
void ConnectionInfoType::Reset(NetworkInterface* networkInterface)
{
    name[0] = 0;
    connection = 0;
    owner = networkInterface;
}

// 0x004add50: lists up to five service provider connections.
long NetworkInterface::EnumConnections(ConnectionInfoType* connections, int* count)
{
    for (int i = 0; i < 5; i++)
        connections[i].Reset(this);
    if (!directPlay)
        return NET_DPERR_INVALIDOBJECT;
    g_EnumConnectionCount = 0;
    long result = directPlay->EnumConnections(&g_NetApplicationGuid, EnumConnectionsCallback,
                                              connections, 0);
    *count = g_EnumConnectionCount;
    return result;
}

// 0x004addc0: EnumConnections callback; records connections that initialize.
int __stdcall EnumConnectionsCallback(const GUID* provider, void* connection, unsigned long size,
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
        if (!connections->owner)
            goto failed;
        if (!connections->owner->CreateAddress(*provider, "", "", 0, &address,
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
        strncpy(connections[g_EnumConnectionCount].name, name->shortName, count);
        connections[g_EnumConnectionCount].name[count] = 0;
        connections[g_EnumConnectionCount].connection = DebugMalloc(size, __FILE__, 2553);
        if (!connections[g_EnumConnectionCount].connection)
            goto failed;
        memcpy(connections[g_EnumConnectionCount].connection, connection, size);
        entry = &connections[g_EnumConnectionCount];
        *(GUID*)entry->provider = *provider;
        if (++g_EnumConnectionCount >= 5)
            return 0;
    }
failed:
    ReportDirectPlayError(result, __FILE__, 2564);
    if (directPlay)
        directPlay->Release();
    return 1;
}

// 0x004adff0: lists up to five sessions.
long NetworkInterface::EnumSessions(SessionInfoType* sessions, int* count,
                                             unsigned long flags)
{
    if (sessions) {
        for (int i = 0; i < 5; i++)
            sessions[i].UnknownVirtualSlot1();
    }
    if (!directPlay)
        return NET_DPERR_INVALIDOBJECT;
    NetSessionDesc desc;
    memset(&desc, 0, sizeof(desc));
    desc.size = sizeof(desc);
    *(GUID*)desc.application = g_NetApplicationGuid;
    g_EnumSessionCount = 0;
    long result = directPlay->EnumSessions(&desc, 0, EnumSessionsCallback, sessions, flags);
    while (result == NET_DPERR_CONNECTING)
        result = directPlay->EnumSessions(&desc, 0, EnumSessionsCallback, sessions, flags);
    *count = g_EnumSessionCount;
    if (result < 0) {
        ReportDirectPlayError(result, __FILE__, 2665);
        return result;
    }
    return 0;
}

// 0x004ae100: EnumSessions callback.
int __stdcall EnumSessionsCallback(const NetSessionDesc* desc, unsigned long*,
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
        strncpy(sessions[g_EnumSessionCount].name, desc->sessionName, count);
        sessions[g_EnumSessionCount].name[count] = 0;
        sessions[g_EnumSessionCount].instance = DebugMalloc(sizeof(GUID), __FILE__, 2616);
        GUID* instance = (GUID*)sessions[g_EnumSessionCount].instance;
        if (instance) {
            *instance = *(const GUID*)desc->instance;
            if (++g_EnumSessionCount >= 5)
                return 0;
        }
    }
    return 1;
}

// 0x004ae200: lists up to seven players of `session`.
long NetworkInterface::EnumPlayers(GUID* session, PlayerInfoType* entries, int* count)
{
    for (int i = 0; i < 7; i++)
        entries[i].UnknownVirtualSlot1();
    if (!directPlay)
        return NET_DPERR_INVALIDOBJECT;
    g_EnumPlayerCount = 0;
    long result = directPlay->EnumPlayers(session, EnumPlayersCallback, entries, 0x10);
    *count = g_EnumPlayerCount;
    return result;
}

// 0x004ae270: EnumPlayers callback.
int __stdcall EnumPlayersCallback(unsigned long id, unsigned long, const NetName* name,
                                    unsigned long, void* context)
{
    PlayerInfoType* players = (PlayerInfoType*)context;
    players[g_EnumPlayerCount].id = id;
    int length = strlen(name->shortName);
    int count = length > 15 ? 15 : length;
    strncpy(players[g_EnumPlayerCount].name, name->shortName, count);
    players[g_EnumPlayerCount].name[count] = 0;
    return ++g_EnumPlayerCount < 7;
}

// 0x004add00
void PlayerInfoType::UnknownVirtualSlot1()
{
    id = 0;
    name[0] = 0;
}

// 0x004adc90
void SessionInfoType::UnknownVirtualSlot1()
{
    name[0] = 0;
    if (instance) {
        DebugFree(instance, __FILE__, 2472);
        instance = 0;
    }
}
