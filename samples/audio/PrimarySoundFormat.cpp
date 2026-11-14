// Complete nonexact primary-buffer format setup. See docs/PCAUDIO.md.
#include <string.h>
#include "../../src/reconstructed/SoundInterface.h"

// 0x004be910: sets the primary buffer's PCM format (108/158). Retail keeps
// `rate` in esi from the entry and loads `stereo` late into edx; VC6 here
// does the reverse. Unchanged or worse: all 24 orders of the four computed
// stores, 0..7 dead initialised locals, `samples`/`channels` locals and the
// byte rate from `(unsigned long)rate` (1536 forms).
int PCSoundInterface::SetPrimaryFormat(int rate, int stereo, int bits) {
    if (!audioStarted)
        return 0;
    WaveFormatEx format;
    memset(&format, 0, 16); // the PCM part only; extraSize is left as is
    format.formatTag = 1;
    format.bitsPerSample = bits;
    format.channels = (stereo != 0) + 1;
    format.samplesPerSec = rate;
    format.avgBytesPerSec = format.channels * format.bitsPerSample * format.samplesPerSec / 8;
    format.blockAlign = format.channels * format.bitsPerSample / 8;
    return primaryBuffer->SetFormat(&format) >= 0;
}
