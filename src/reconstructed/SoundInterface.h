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

    unsigned char field_0x04[0x478 - 4];
};
