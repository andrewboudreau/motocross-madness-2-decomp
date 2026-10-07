// CD-audio player, 0x00431050..0x004312a8 (provisional file name; no
// __FILE__ or RTTI). It sits between CarProcedural.cpp (last xref
// 0x0042fb52) and the rectangle clipper at 0x004312b0 (ClipRectangle.cpp),
// which precedes CollisionCharacter.cpp (0x00431a12). The MCI and aux
// calls bind to WINMM's IAT (0x005503a0..0x005503b4).

#include <windows.h>
#include <mmsystem.h>
#include <string.h>

#include "CDAudio.h"

// 0x00431050
CDAudio::CDAudio()
{
    opened = 0;
    device = 0;
    auxDevice = 0;
    savedVolume = 100.0f;
    UINT count = auxGetNumDevs();
    for (UINT i = 0; i < count; i++) {
        AUXCAPS caps;
        auxGetDevCaps(i, &caps, sizeof(caps));
        if (caps.wTechnology == AUXCAPS_CDAUDIO)
            auxDevice = i;
    }
}

// 0x004310b0
CDAudio::~CDAudio()
{
    if (opened)
        Close();
}

// 0x004310c0
int CDAudio::Initialize()
{
    if (!Open())
        return 0;
    opened = 1;
    return 1;
}

// 0x004310e0
void CDAudio::Stop()
{
    if (IsPlaying())
        mciSendCommand(device, MCI_STOP, 0, 0);
}

// 0x00431100
int CDAudio::IsPlaying()
{
    MCI_STATUS_PARMS status;
    status.dwItem = MCI_STATUS_MODE;
    if (mciSendCommand(device, MCI_STATUS, MCI_STATUS_ITEM, (DWORD)&status) == 0
        && status.dwReturn == MCI_MODE_PLAY)
        return 1;
    return 0;
}

// 0x00431140: both channels at percent of full scale.
int CDAudio::SetVolume(float percent)
{
    float fraction = percent / 100.0f;
    WORD level = (WORD)(fraction * 65535.0f);
    return auxSetVolume(auxDevice, MAKELONG(level, level)) == 0;
}

// 0x00431180
int CDAudio::GetVolume(float* percent)
{
    DWORD volume;
    *percent = 0;
    if (auxGetVolume(auxDevice, &volume) != 0)
        return 0;
    float fraction = volume / 65535.0f;
    *percent = fraction * 100.0f;
    return 1;
}

// 0x004311e0
int CDAudio::Open()
{
    MCI_OPEN_PARMS open;
    memset(&open, 0, sizeof(open));
    open.lpstrDeviceType = "cdaudio";
    if (mciSendCommand(0, MCI_OPEN, MCI_OPEN_TYPE, (DWORD)&open) != 0)
        goto failed;
    device = open.wDeviceID;
    GetVolume(&savedVolume);
    MCI_SET_PARMS set;
    set.dwTimeFormat = MCI_FORMAT_TMSF;
    if (mciSendCommand(device, MCI_SET, MCI_SET_TIME_FORMAT, (DWORD)&set) != 0)
        goto failed;
    return 1;
failed:
    return 0;
}

// 0x00431270
void CDAudio::Close()
{
    if (IsPlaying())
        Stop();
    if (opened)
        SetVolume(savedVolume);
    mciSendCommand(device, MCI_CLOSE, 0, 0);
}
