#ifndef MCM2_DIRECTPLAYMESSAGES_H
#define MCM2_DIRECTPLAYMESSAGES_H

// DirectPlay system message types (dplay.h). For a system message
// NetworkInterface stores `from` 0 and the NetMessage type is the first dword
// of the data (0x004aacc0); its dispatcher 0x004aced0 itself handles 3 and 5
// after passing them to Game slot 17.
#define DPSYS_CREATEPLAYERORGROUP  0x0003
#define DPSYS_DESTROYPLAYERORGROUP 0x0005
#define DPSYS_HOST                 0x0101

// The data of system message 5 (a player left): the leading dwords of dplay.h's
// DPMSG_DESTROYPLAYERORGROUP.
struct DPMsgDestroyPlayerOrGroup {
    int type;                                 // +0x00 5: a DirectPlay system message (no Send passes 5; NetMessage::Set 0x004aacc0 reads the whole dword when from == 0)
    int playerType;
    int playerId;                             // +0x08 player id, matched against racers' networkPlayerId
};

#endif
