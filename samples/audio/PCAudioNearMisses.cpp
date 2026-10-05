// Near-miss PCAudio.cpp candidate, kept out of src/reconstructed until it
// matches. See docs/PCAUDIO.md.
//
// UnknownFunction4be910 (0x004be910, 158 bytes): sets the primary buffer's
// PCM format. The 0x14-byte frame (a WAVEFORMATEX-sized local with only the
// first 16 bytes cleared), the arithmetic and the call match (108 of 158
// bytes); only the scheduling of the zeroing stores against the argument
// loads differs. Retail clears all 16 bytes first and keeps `rate` in esi;
// every order of the four field assignments, a channel temporary, `?:` and
// an if/else body leave VC6 loading `stereo` into esi first.

#include <string.h>

#include "../../src/reconstructed/SoundInterface.h"

// 0x004be910: sets the primary buffer's PCM format.
int PCSoundInterface::UnknownFunction4be910(int rate, int stereo, int bits) {
    if (!field_0x2c_bit0)
        return 0;
    UnknownWaveFormat format;
    memset(&format, 0, 16); // the PCM part only; extraSize is left as is
    format.formatTag = 1;
    format.bitsPerSample = bits;
    format.channels = (stereo != 0) + 1;
    format.samplesPerSec = rate;
    format.avgBytesPerSec = format.channels * format.bitsPerSample * format.samplesPerSec / 8;
    format.blockAlign = format.channels * format.bitsPerSample / 8;
    return field_0x468->SetFormat(&format) >= 0;
}

