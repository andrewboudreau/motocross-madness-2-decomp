#pragma once

// RTTI: SoundInterface (root; its only slot is the destructor) and
// PCSoundInterface : SoundInterface (0x478 bytes, the size Game's
// initialiser allocates). Only what Game uses is declared.
class SoundInterface {
public:
    virtual ~SoundInterface();
};

class PCSoundInterface : public SoundInterface {
public:
    PCSoundInterface();                       // 0x004be370
    void UnknownFunction4be9b0(int value);    // 0x004be9b0 (TrackGame 0x00521a30)
    // 0x004be5a0: starts the audio (TrackGame slot 4 passes 22050, 1, 8 or
    // 16, 4000000 and a mode value); DirectSound-style result.
    long UnknownFunction4be5a0(int rate, int a, int bits, int b, int c);

    unsigned char field_0x04[0x478 - 4];
};
