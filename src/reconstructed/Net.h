#pragma once

// Net.h -- types of D:\aardvark\VC\krusty2\Net.cpp (0x004aab00..0x004ae2e7).
// See docs/NET.md. NetworkInterface, InfoType, PlayerInfoType,
// ConnectionInfoType and SessionInfoType are RTTI names (tier 1); every other
// type and member name is provisional. This header does not include
// <windows.h> because Game.h includes it.

#include "ContainerList.h"

struct _GUID;

// DirectPlay COM interfaces, declared here because VC6's DPLAY.H predates
// IDirectPlay4. The IIDs passed to CoCreateInstance/QueryInterface are the
// SDK's IID_IDirectPlay4A and IID_IDirectPlayLobby3A (GUID bytes at
// 0x005567b0 and 0x00556870); the slot offsets are decoded from the calls.
struct UnknownDirectPlay4A {
    virtual long __stdcall QueryInterface(const _GUID& iid, void** result);
    virtual unsigned long __stdcall AddRef();
    virtual unsigned long __stdcall Release();
    virtual long __stdcall AddPlayerToGroup(unsigned long group, unsigned long player);
    virtual long __stdcall Close();
    virtual long __stdcall CreateGroup(unsigned long* group, struct NetName* name, void* data,
                                       unsigned long dataSize, unsigned long flags);
    virtual long __stdcall CreatePlayer(unsigned long* player, struct NetName* name, void* event,
                                        void* data, unsigned long dataSize, unsigned long flags);
    virtual long __stdcall DeletePlayerFromGroup(unsigned long group, unsigned long player);
    virtual long __stdcall DestroyGroup(unsigned long group);
    virtual long __stdcall DestroyPlayer(unsigned long player);
    virtual long __stdcall EnumGroupPlayers(unsigned long group, _GUID* instance, void* callback,
                                            void* context, unsigned long flags);
    virtual long __stdcall EnumGroups(_GUID* instance, void* callback, void* context,
                                      unsigned long flags);
    virtual long __stdcall EnumPlayers(_GUID* instance, void* callback, void* context,
                                       unsigned long flags);
    virtual long __stdcall EnumSessions(void* desc, unsigned long timeout, void* callback,
                                        void* context, unsigned long flags);
    virtual long __stdcall GetCaps(struct NetCaps* caps, unsigned long flags);
    virtual long __stdcall GetGroupData(unsigned long group, void* data, unsigned long* size,
                                        unsigned long flags);
    virtual long __stdcall GetGroupName(unsigned long group, void* data, unsigned long* size);
    virtual long __stdcall GetMessageCount(unsigned long player, unsigned long* count);
    virtual long __stdcall GetPlayerAddress(unsigned long player, void* data, unsigned long* size);
    virtual long __stdcall GetPlayerCaps(unsigned long player, struct NetCaps* caps,
                                         unsigned long flags);
    virtual long __stdcall GetPlayerData(unsigned long player, void* data, unsigned long* size,
                                         unsigned long flags);
    virtual long __stdcall GetPlayerName(unsigned long player, void* data, unsigned long* size);
    virtual long __stdcall GetSessionDesc(void* data, unsigned long* size);
    virtual long __stdcall Initialize(_GUID* guid);
    virtual long __stdcall Open(struct NetSessionDesc* desc, unsigned long flags);
    virtual long __stdcall Receive(unsigned long* from, unsigned long* to, unsigned long flags,
                                   void* data, unsigned long* size);
    virtual long __stdcall Send(unsigned long from, unsigned long to, unsigned long flags,
                                void* data, unsigned long size);
    virtual long __stdcall SetGroupData(unsigned long group, void* data, unsigned long size,
                                        unsigned long flags);
    virtual long __stdcall SetGroupName(unsigned long group, struct NetName* name,
                                        unsigned long flags);
    virtual long __stdcall SetPlayerData(unsigned long player, void* data, unsigned long size,
                                         unsigned long flags);
    virtual long __stdcall SetPlayerName(unsigned long player, struct NetName* name,
                                         unsigned long flags);
    virtual long __stdcall SetSessionDesc(void* desc, unsigned long flags);
    virtual long __stdcall AddGroupToGroup(unsigned long parent, unsigned long group);
    virtual long __stdcall CreateGroupInGroup(unsigned long parent, unsigned long* group,
                                              struct NetName* name, void* data,
                                              unsigned long dataSize, unsigned long flags);
    virtual long __stdcall DeleteGroupFromGroup(unsigned long parent, unsigned long group);
    virtual long __stdcall EnumConnections(const _GUID* application, void* callback,
                                           void* context, unsigned long flags);
    virtual long __stdcall EnumGroupsInGroup(unsigned long group, _GUID* instance,
                                             void* callback, void* context, unsigned long flags);
    virtual long __stdcall GetGroupConnectionSettings(unsigned long flags, unsigned long group,
                                                      void* data, unsigned long* size);
    virtual long __stdcall InitializeConnection(void* connection, unsigned long flags);
    virtual long __stdcall SecureOpen(struct NetSessionDesc* desc, unsigned long flags,
                                      void* security, void* credentials);
    virtual long __stdcall SendChatMessage(unsigned long from, unsigned long to,
                                           unsigned long flags, void* chat);
    virtual long __stdcall SetGroupConnectionSettings(unsigned long flags, unsigned long group,
                                                      void* connection);
    virtual long __stdcall StartSession(unsigned long flags, unsigned long group);
    virtual long __stdcall GetGroupFlags(unsigned long group, unsigned long* flags);
    virtual long __stdcall GetGroupParent(unsigned long group, unsigned long* parent);
    virtual long __stdcall GetPlayerAccount(unsigned long player, unsigned long flags,
                                            void* data, unsigned long* size);
    virtual long __stdcall GetPlayerFlags(unsigned long player, unsigned long* flags);
    virtual long __stdcall GetGroupOwner(unsigned long group, unsigned long* owner);
    virtual long __stdcall SetGroupOwner(unsigned long group, unsigned long owner);
    virtual long __stdcall SendEx(unsigned long from, unsigned long to, unsigned long flags,
                                  void* data, unsigned long size, unsigned long priority,
                                  unsigned long timeout, void* context, unsigned long* messageId);
};

struct UnknownDirectPlayLobby3A {
    virtual long __stdcall QueryInterface(const _GUID& iid, void** result);
    virtual unsigned long __stdcall AddRef();
    virtual unsigned long __stdcall Release();
    virtual long __stdcall Connect(unsigned long flags, UnknownDirectPlay4A** result, void* outer);
    virtual long __stdcall CreateAddress(const _GUID& provider, const _GUID& dataType,
                                         const void* data, unsigned long dataSize, void* address,
                                         unsigned long* addressSize);
    virtual long __stdcall EnumAddress(void* callback, const void* address,
                                       unsigned long addressSize, void* context);
    virtual long __stdcall EnumAddressTypes(void* callback, const _GUID& provider, void* context,
                                            unsigned long flags);
    virtual long __stdcall EnumLocalApplications(void* callback, void* context,
                                                 unsigned long flags);
    virtual long __stdcall GetConnectionSettings(unsigned long application, void* data,
                                                 unsigned long* size);
    virtual long __stdcall ReceiveLobbyMessage(unsigned long flags, unsigned long application,
                                               unsigned long* messageFlags, void* data,
                                               unsigned long* size);
    virtual long __stdcall RunApplication(unsigned long flags, unsigned long* application,
                                          void* connection, void* event);
    virtual long __stdcall SendLobbyMessage(unsigned long flags, unsigned long application,
                                            void* data, unsigned long size);
    virtual long __stdcall SetConnectionSettings(unsigned long flags, unsigned long application,
                                                 void* connection);
    virtual long __stdcall SetLobbyMessageEvent(unsigned long flags, unsigned long application,
                                                void* event);
    virtual long __stdcall CreateCompoundAddress(const void* elements, unsigned long count,
                                                 void* address, unsigned long* addressSize);
};

// DPNAME (0x10 bytes).
struct NetName {
    unsigned long size;
    unsigned long flags;
    char* shortName;
    char* longName;
};

// DPCAPS (0x28 bytes).
struct NetCaps {
    unsigned long size;
    unsigned long flags;
    unsigned long maxBufferSize;
    unsigned long maxQueueSize;
    unsigned long maxPlayers;
    unsigned long hundredBaud;
    unsigned long latency;
    unsigned long maxLocalPlayers;
    unsigned long headerLength;
    unsigned long timeout;
};

// DPSESSIONDESC2 (0x50 bytes).
struct NetSessionDesc {
    unsigned long size;
    unsigned long flags;
    unsigned char instance[16];
    unsigned char application[16];
    unsigned long maxPlayers;
    unsigned long currentPlayers;
    char* sessionName;
    char* password;
    unsigned long reserved1;
    unsigned long reserved2;
    unsigned long user1;
    unsigned long user2;
    unsigned long user3;
    unsigned long user4;
};

// DPLCONNECTION (0x28 bytes).
struct NetConnection {
    unsigned long size;
    unsigned long flags;                                // 2: create the session
    NetSessionDesc* session;
    NetName* playerName;
    unsigned char provider[16];                         // service provider GUID
    void* address;
    unsigned long addressSize;
};

// Win32 CRITICAL_SECTION layout (0x18 bytes); Net.cpp casts it.
struct NetCriticalSection {
    void* debugInfo;
    long lockCount;
    long recursionCount;
    void* owningThread;
    void* lockSemaphore;
    unsigned long spinCount;
};

#define NET_MAX_GENERIC_MSG_SIZE 0x8de

// A received or queued message (0x8f4 bytes). Field use is decoded from
// 0x004aacc0; for a DirectPlay system message (from == 0) the type is the
// first dword of the data, otherwise its first byte.
class NetMessage {
public:
    NetMessage();                                       // 0x004aac90
    void Set(void* source, unsigned int length, int sender, int receiver, int messageFlags);  // 0x004aacc0

    int type;                                     // type
    int from;                                     // sender
    int to;                                     // receiver
    int flags;
    unsigned int size;                            // data size
    unsigned char data[NET_MAX_GENERIC_MSG_SIZE];
};

// The 0x8f4-byte message NetThread.cpp's receive loop allocates (0x004afc10)
// and appends to NetworkInterface+0x100.
class NetIncomingMessage : public NetMessage {
public:
    NetIncomingMessage();                               // 0x004aad50
    void SetReceived(void* source, unsigned int length, int sender, int receiver);  // 0x004aad60
};

// A guaranteed message awaiting acknowledgement (0x904 bytes): data[2..3]
// carry the sequence number that 0x004aafe0 matches.
class NetPendingMessage : public NetMessage {
public:
    NetPendingMessage();                                // 0x004aad80
    int SetPending(int messageType, short sequence, void* source, unsigned int length, int sender,  // 0x004aadc0
                              int receiver);

    unsigned int sendSize;                           // data size
    float resendTimer;                                  // time since the last send
    NetPendingMessage* next;                     // next
    int field_0x900;
};

class NetworkInterface;

// Per-player list of NetPendingMessage (one pointer).
class NetPendingList {
public:
    NetPendingList();                                   // 0x004aae20
    ~NetPendingList();                                  // 0x004aae30
    void Append(int type, short sequence, void* data, unsigned int size, int from,  // 0x004aae50
                               int to);
    long ResendExpired(NetworkInterface* net, int unused);  // 0x004aaf20
    void Acknowledge(short sequence);  // 0x004aafe0

    NetPendingMessage* head;
};

// A remote player (0x20 bytes, singly linked from NetworkInterface+0x30).
class NetPlayer {
public:
    NetPlayer(int playerId, const char* playerName);                // 0x004ab050
    ~NetPlayer();                                       // 0x004ab0d0

    int id;                                     // DirectPlay id
    char name[16];                                // name
    NetPendingList* pending;
    unsigned int lastKeepAliveTime;                            // time stamp (0x004ac8d0)
    NetPlayer* next;                              // next
};

// The 0x8dc-byte file chunk message (type 0xcf).
struct NetFileChunk {
    int field_0x00;
    char data[0x8ce];
    int byteCount;                                    // byte count
    int index;                                    // chunk index
};

// The file announcement NetFile::BeginReceive reads.
struct NetFileHeader {
    int field_0x00;
    char name[0x104];                             // file name
    int size;                                    // size
};

// A file transferred in 0x8ce-byte chunks (0x12c bytes).
class NetFile {
public:
    NetFile();                                          // 0x004ab0f0
    ~NetFile();                                         // 0x004ab160
    void Update(int unused);  // 0x004ab190
    void SendNextChunk();  // 0x004ab1b0
    void BeginReceive(int id, NetFileHeader* header);  // 0x004ab280
    void ReceiveChunk(int id, NetFileChunk* chunk);  // 0x004ab380
    void Save();  // 0x004ab440

    char* data;                                   // file data
    int* chunkReceived;                                    // chunk received flags
    int size;                                     // file size
    char name[0x104];                             // file name
    int sourceId;
    int destination;                                    // destination player
    int chunkCount;                                    // chunk count
    int chunksDone;                                    // chunks done
    void (*onComplete)(void* context);                 // completion callback
    void* onCompleteContext;                                  // callback context
    unsigned char sending : 1;                 // sending
    unsigned char receiving : 1;                 // receiving
    unsigned char sent : 1;                 // sent
    unsigned char received : 1;                 // received
};

// RTTI InfoType family. InfoType's own functions (0x00523c80 destructor,
// 0x0044d720 deleting destructor, 0x0044d710 empty slot 1) come from another
// TU; Net.cpp emits the derived constructors, PlayerInfoType's slot 1 and
// SessionInfoType's slot 1.
class InfoType {
public:
    virtual ~InfoType() {}
    virtual void UnknownVirtualSlot1() {}
};

// 0x18 bytes; NetworkInterface::Initialize builds seven of them.
class PlayerInfoType : public InfoType {
public:
    PlayerInfoType() {                                  // 0x004adcc0
        id = 0;
        name[0] = 0;
    }
    // 0x004add00 (Net.cpp). Out of line: SelectGamePicProcs.cpp 0x004f53c1
    // calls it on each 0x00689d08 record instead of inlining it.
    virtual void UnknownVirtualSlot1();

    int id;                                     // DirectPlay id
    char name[16];                                // name
};

// 0x120 bytes.
class ConnectionInfoType : public InfoType {
public:
    ConnectionInfoType();                               // 0x004add10

    void Reset(NetworkInterface* networkInterface);  // 0x004add30

    char name[0x104];                             // name
    unsigned long provider[4];                       // service provider GUID
    void* connection;                                  // DirectPlay connection data
    NetworkInterface* owner;                      // owner
};

// 0x10c bytes; its constructor (0x00523b94) is emitted by another TU.
class SessionInfoType : public InfoType {
public:
    SessionInfoType() {
        name[0] = 0;
        instance = 0;
    }
    virtual void UnknownVirtualSlot1();                 // 0x004adc90

    char name[0x104];
    void* instance;
};

// Game+0x08 (0x128 bytes). RTTI NetworkInterface; one virtual slot.
class NetworkInterface {
public:
    NetworkInterface();                                 // 0x004ab480
    ~NetworkInterface();                                // 0x004ab570
    virtual void UnknownVirtualSlot0(int value);        // 0x004ad050

    long Initialize(int mode);  // 0x004ab6b0
    long ConnectUsingLobby();  // 0x004ab960
    void StripMessageTrailer(unsigned char** data, unsigned long* size, int* guaranteed,  // 0x004abd40
                               int* sequence, int* target);
    int InitializeConnection(void* connection, int unused, int kind);  // 0x004abdf0
    long Shutdown();  // 0x004abe70
    int CreateAddress(_GUID provider, char* address, char* port, void* comPort,  // 0x004abf10
                              void** result, unsigned long* resultSize);
    int GetLocalAddress(char* address);  // 0x004ac2d0
    int CreateSession(char* name, unsigned long flags);  // 0x004ac3c0
    int JoinSession(const _GUID* instance, unsigned long flags);  // 0x004ac480
    int SetSessionJoinable(int value);  // 0x004ac510
    int CreateLocalPlayer(char* name);  // 0x004ac5e0
    int DestroyLocalPlayer();  // 0x004ac630
    int CreatePlayer(char* name, unsigned long* player);  // 0x004ac650
    int DestroyPlayer(unsigned long player);  // 0x004ac6f0
    int GetPlayerName(int player, char* name);  // the player's name; 0 if none (0x004ac720)
    NetPlayer* NextPlayer(int* index);  // 0x004ac7c0
    int QueryPlayerName(int player, NetName** name);  // 0x004ad0f0
    NetPlayer* FindPlayer(int player);  // 0x004ac800
    int Send(int type, void* data, int size, int from, int to);  // 0x004ac830
    int StartKeepAlive(float a, float b);  // 0x004ac8d0
    int StopKeepAlive();  // 0x004ac950
    void AddPlayer(int player, const char* name);  // 0x004ac980
    void RemovePlayer(int player);  // 0x004aca80
    void QueueMessage(void* data, unsigned int size, int from, int to, int flags);  // 0x004acb00
    void SendGuaranteed(int type, void* data, int size, int from);  // 0x004acbc0
    void ResendPending(int value);  // 0x004acc50
    void AcknowledgeMessage(short sequence);  // 0x004acca0
    int CreateGroup(unsigned long player);  // 0x004accd0
    int FindGroup();  // 0x004acd20
    void GetSessionFlags(unsigned long* flags);  // 0x004acdb0
    int Update(int value);  // 0x004acde0
    void DispatchMessages(int value);  // 0x004aced0
    static int __stdcall EnumGroupsCallback(unsigned long id, unsigned long type,  // 0x004acd70
                                               const NetName* name, unsigned long flags,
                                               void* context);
    static int __stdcall EnumGroupPlayersCallback(unsigned long id, unsigned long type,  // 0x004acd90
                                               const NetName* name, unsigned long flags,
                                               void* context);
    int SendRaw(int type, void* data, int size, short sequence, int from, int to,  // 0x004ad280
                              void* extra, int extraSize);
    void ReadProviderCaps(const _GUID* provider);  // 0x004ad180
    long EnumConnections(ConnectionInfoType* connections, int* count);  // 0x004add50
    long EnumSessions(SessionInfoType* sessions, int* count, unsigned long flags);  // 0x004adff0
    long EnumPlayers(_GUID* session, PlayerInfoType* entries, int* count);  // 0x004ae200

    UnknownDirectPlay4A* directPlay;
    UnknownDirectPlayLobby3A* lobby;
    int localPlayer;                                     // local player id
    int isHost;                                         // 1 after Open with DPOPEN_CREATE (0x004ac3c0) or DPSYS_HOST (KrustyUI slot 24); 0 after DPOPEN_JOIN
    int lobbyConnected;                                     // connected (TrackGame slot 4)
    int connectionMode;                                     // connection mode 1..4
    void* receiveThread;                                   // NetThread.cpp receive thread (0x004af6a0)
    unsigned int receiveThreadId;                            // its thread id
    void* receiveEvent;                                   // events
    void* quitEvent;
    void* keepAliveEvent;
    NetPlayer* players;
    NetMessage* messages;                             // 0x100 received messages
    int field_0x38;
    int field_0x3c;
    int nextMessage;
    unsigned short nextSequence;                          // guaranteed-message sequence
    NetCriticalSection lock;
    NetCriticalSection keepAliveLock;
    int keepAliveRunning;
    float keepAliveTimeout;
    float keepAliveInterval;
    int removedPlayers[8];                                  // ids of removed players
    NetCaps guaranteedCaps;
    NetCaps caps;
    int providerKind;
    unsigned long serverPlayer;                           // server player id
    unsigned long group;                           // group id
    ContainerList<NetIncomingMessage*> heldMessages;
    ContainerList<NetFile*> files;
};
