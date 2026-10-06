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
    void UnknownFunction4aacc0(void* data, unsigned int size, int from, int to, int flags);

    int field_0x00;                                     // type
    int field_0x04;                                     // sender
    int field_0x08;                                     // receiver
    int field_0x0c;
    unsigned int field_0x10;                            // data size
    unsigned char field_0x14[NET_MAX_GENERIC_MSG_SIZE];
};

// The 0x8f4-byte message NetProcs.cpp's receive loop allocates (0x004afc10)
// and appends to NetworkInterface+0x100.
class NetIncomingMessage : public NetMessage {
public:
    NetIncomingMessage();                               // 0x004aad50
    void UnknownFunction4aad60(void* data, unsigned int size, int from, int to);
};

// A guaranteed message awaiting acknowledgement (0x904 bytes): data[2..3]
// carry the sequence number that 0x004aafe0 matches.
class NetPendingMessage : public NetMessage {
public:
    NetPendingMessage();                                // 0x004aad80
    int UnknownFunction4aadc0(int type, short sequence, void* data, unsigned int size, int from,
                              int to);

    unsigned int field_0x8f4;                           // data size
    float field_0x8f8;                                  // time since the last send
    NetPendingMessage* field_0x8fc;                     // next
    int field_0x900;
};

class NetworkInterface;

// Per-player list of NetPendingMessage (one pointer).
class NetPendingList {
public:
    NetPendingList();                                   // 0x004aae20
    ~NetPendingList();                                  // 0x004aae30
    void UnknownFunction4aae50(int type, short sequence, void* data, unsigned int size, int from,
                               int to);
    long UnknownFunction4aaf20(NetworkInterface* net, int unused);
    void UnknownFunction4aafe0(short sequence);

    NetPendingMessage* field_0x00;
};

// A remote player (0x20 bytes, singly linked from NetworkInterface+0x30).
class NetPlayer {
public:
    NetPlayer(int id, const char* name);                // 0x004ab050
    ~NetPlayer();                                       // 0x004ab0d0

    int field_0x00;                                     // DirectPlay id
    char field_0x04[16];                                // name
    NetPendingList* field_0x14;
    unsigned int field_0x18;                            // time stamp (0x004ac8d0)
    NetPlayer* field_0x1c;                              // next
};

// The 0x8dc-byte file chunk message (type 0xcf).
struct NetFileChunk {
    int field_0x00;
    char field_0x04[0x8ce];
    int field_0x8d4;                                    // byte count
    int field_0x8d8;                                    // chunk index
};

// The file announcement NetFile::UnknownFunction4ab280 reads.
struct NetFileHeader {
    int field_0x00;
    char field_0x04[0x104];                             // file name
    int field_0x108;                                    // size
};

// A file transferred in 0x8ce-byte chunks (0x12c bytes).
class NetFile {
public:
    NetFile();                                          // 0x004ab0f0
    ~NetFile();                                         // 0x004ab160
    void UnknownFunction4ab190(int unused);
    void UnknownFunction4ab1b0();
    void UnknownFunction4ab280(int id, NetFileHeader* header);
    void UnknownFunction4ab380(int id, NetFileChunk* chunk);
    void UnknownFunction4ab440();

    char* field_0x00;                                   // file data
    int* field_0x04;                                    // chunk received flags
    int field_0x08;                                     // file size
    char field_0x0c[0x104];                             // file name
    int field_0x110;
    int field_0x114;                                    // destination player
    int field_0x118;                                    // chunk count
    int field_0x11c;                                    // chunks done
    void (*field_0x120)(void* context);                 // completion callback
    void* field_0x124;                                  // callback context
    unsigned char field_0x128_bit0 : 1;                 // sending
    unsigned char field_0x128_bit1 : 1;                 // receiving
    unsigned char field_0x128_bit2 : 1;                 // sent
    unsigned char field_0x128_bit3 : 1;                 // received
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

// 0x18 bytes; NetworkInterface::UnknownFunction4ab6b0 builds seven of them.
class PlayerInfoType : public InfoType {
public:
    PlayerInfoType() {                                  // 0x004adcc0
        field_0x04 = 0;
        field_0x08[0] = 0;
    }
    virtual void UnknownVirtualSlot1() {                // 0x004add00
        field_0x04 = 0;
        field_0x08[0] = 0;
    }

    int field_0x04;                                     // DirectPlay id
    char field_0x08[16];                                // name
};

// 0x120 bytes.
class ConnectionInfoType : public InfoType {
public:
    ConnectionInfoType();                               // 0x004add10

    void UnknownFunction4add30(NetworkInterface* owner);

    char field_0x04[0x104];                             // name
    unsigned long field_0x108[4];                       // service provider GUID
    void* field_0x118;                                  // DirectPlay connection data
    NetworkInterface* field_0x11c;                      // owner
};

// 0x10c bytes; its constructor (0x00523b94) is emitted by another TU.
class SessionInfoType : public InfoType {
public:
    SessionInfoType() {
        field_0x04[0] = 0;
        field_0x108 = 0;
    }
    virtual void UnknownVirtualSlot1();                 // 0x004adc90

    char field_0x04[0x104];
    void* field_0x108;
};

// Game+0x08 (0x128 bytes). RTTI NetworkInterface; one virtual slot.
class NetworkInterface {
public:
    NetworkInterface();                                 // 0x004ab480
    ~NetworkInterface();                                // 0x004ab570
    virtual void UnknownVirtualSlot0(int value);        // 0x004ad050

    long UnknownFunction4ab6b0(int mode);
    long UnknownFunction4ab960();
    void UnknownFunction4abd40(unsigned char** data, unsigned long* size, int* guaranteed,
                               int* sequence, int* target);
    int UnknownFunction4abdf0(void* connection, int unused, int kind);
    long UnknownFunction4abe70();
    int UnknownFunction4abf10(_GUID provider, char* address, char* port, void* comPort,
                              void** result, unsigned long* resultSize);
    int UnknownFunction4ac2d0(char* address);
    int UnknownFunction4ac3c0(char* name, unsigned long flags);
    int UnknownFunction4ac480(const _GUID* instance, unsigned long flags);
    int UnknownFunction4ac510(int value);
    int UnknownFunction4ac5e0(char* name);
    int UnknownFunction4ac630();
    int UnknownFunction4ac650(char* name, unsigned long* player);
    int UnknownFunction4ac6f0(unsigned long player);
    int UnknownFunction4ac720(int player, char* name);  // the player's name; 0 if none
    NetPlayer* UnknownFunction4ac7c0(int* index);
    int UnknownFunction4ad0f0(int player, NetName** name);
    NetPlayer* UnknownFunction4ac800(int player);
    int UnknownFunction4ac830(int type, void* data, int size, int from, int to);
    int UnknownFunction4ac8d0(float a, float b);
    int UnknownFunction4ac950();
    void UnknownFunction4ac980(int player, const char* name);
    void UnknownFunction4aca80(int player);
    void UnknownFunction4acb00(void* data, unsigned int size, int from, int to, int flags);
    void UnknownFunction4acbc0(int type, void* data, int size, int from);
    void UnknownFunction4acc50(int value);
    void UnknownFunction4acca0(short sequence);
    int UnknownFunction4accd0(unsigned long player);
    int UnknownFunction4acd20();
    void UnknownFunction4acdb0(unsigned long* flags);
    int UnknownFunction4acde0(int value);
    void UnknownFunction4aced0(int value);
    static int __stdcall UnknownFunction4acd70(unsigned long id, unsigned long type,
                                               const NetName* name, unsigned long flags,
                                               void* context);
    static int __stdcall UnknownFunction4acd90(unsigned long id, unsigned long type,
                                               const NetName* name, unsigned long flags,
                                               void* context);
    int UnknownFunction4ad280(int type, void* data, int size, short sequence, int from, int to,
                              void* extra, int extraSize);
    void UnknownFunction4ad180(const _GUID* provider);
    long UnknownFunction4add50(ConnectionInfoType* connections, int* count);
    long UnknownFunction4adff0(SessionInfoType* sessions, int* count, unsigned long flags);
    long UnknownFunction4ae200(_GUID* session, PlayerInfoType* players, int* count);

    UnknownDirectPlay4A* field_0x04;
    UnknownDirectPlayLobby3A* field_0x08;
    int field_0x0c;                                     // local player id
    int isHost;                                         // 1 after Open with DPOPEN_CREATE (0x004ac3c0) or DPSYS_HOST (KrustyUI slot 24); 0 after DPOPEN_JOIN
    int field_0x14;                                     // connected (TrackGame slot 4)
    int field_0x18;                                     // connection mode 1..4
    void* field_0x1c;                                   // NetProcs thread (0x004af6a0)
    unsigned int field_0x20;                            // its thread id
    void* field_0x24;                                   // events
    void* field_0x28;
    void* field_0x2c;
    NetPlayer* field_0x30;
    NetMessage* field_0x34;                             // 0x100 received messages
    int field_0x38;
    int field_0x3c;
    int field_0x40;
    unsigned short field_0x44;                          // guaranteed-message sequence
    NetCriticalSection field_0x48;
    NetCriticalSection field_0x60;
    int field_0x78;
    float field_0x7c;
    float field_0x80;
    int field_0x84[8];                                  // ids of removed players
    NetCaps field_0xa4;
    NetCaps field_0xcc;
    int field_0xf4;
    unsigned long field_0xf8;                           // server player id
    unsigned long field_0xfc;                           // group id
    ContainerList<NetIncomingMessage*> field_0x100;
    ContainerList<NetFile*> field_0x114;
};
