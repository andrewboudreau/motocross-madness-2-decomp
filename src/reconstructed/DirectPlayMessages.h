#ifndef MCM2_DIRECTPLAYMESSAGES_H
#define MCM2_DIRECTPLAYMESSAGES_H

// DirectPlay system message types (dplay.h). For a system message
// NetworkInterface stores `from` 0 and the NetMessage type is the first dword
// of the data (0x004aacc0); its dispatcher 0x004aced0 itself handles 3 and 5
// after passing them to Game slot 17.
#define DPSYS_CREATEPLAYERORGROUP  0x0003
#define DPSYS_DESTROYPLAYERORGROUP 0x0005
#define DPSYS_HOST                 0x0101

#endif
