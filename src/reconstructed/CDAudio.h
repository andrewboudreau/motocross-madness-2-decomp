#pragma once

// The CD-audio player TrackGame keeps at +0x3340 (TrackGame.cpp line 343
// allocates 0x10 bytes, calls Initialize, and its destructor deletes it;
// 0x00521a40 calls Stop). No RTTI and no __FILE__: the code at
// 0x00431050..0x004312a8 opens the MCI "cdaudio" device (literal 0x00568460)
// and drives the CD mixer line through the aux* API. The class and member
// names are provisional; the file name CDAudio.cpp is ours.
class CDAudio {
public:
    CDAudio();                                // 0x00431050: finds the CD aux device
    ~CDAudio();                               // 0x004310b0: closes an open device
    int Initialize();                         // 0x004310c0: opens the device
    void Stop();                              // 0x004310e0: MCI_STOP while playing
    int IsPlaying();                          // 0x00431100: MCI_STATUS_MODE == MCI_MODE_PLAY
    int SetVolume(float percent);             // 0x00431140 (ret 4)
    int GetVolume(float* percent);            // 0x00431180 (ret 4)
    int Open();                               // 0x004311e0: MCI_OPEN "cdaudio", TMSF time format
    void Close();                             // 0x00431270: restores the volume, MCI_CLOSE

    unsigned int device;                      // +0x00: MCI device id
    unsigned int auxDevice;                   // +0x04: aux device with AUXCAPS_CDAUDIO
    int opened;                               // +0x08
    float savedVolume;                        // +0x0c: volume found by Open, restored by Close
};
