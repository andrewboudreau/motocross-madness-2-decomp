// Near-miss PCAudio.cpp candidates, kept out of src/reconstructed until they
// match. See docs/PCAUDIO.md. Each differs only in branch layout or register
// choice:
//
// SetPrimaryFormat (0x004be910, 158 bytes): sets the primary buffer's
// PCM format. The 0x14-byte frame (a WAVEFORMATEX-sized local with only the
// first 16 bytes cleared), the arithmetic and the call match (108 of 158
// bytes); only the scheduling of the zeroing stores against the argument
// loads differs. Retail clears all 16 bytes first and keeps `rate` in esi;
// every order of the four field assignments, a channel temporary, `?:` and
// an if/else body leave VC6 loading `stereo` into esi first; so do all 24
// assignment orders x five channel-count spellings, memset sizes 16 and
// sizeof - 4, a derived struct clearing in its constructor, unsigned/short
// parameter types (bool or unsigned char `stereo`: 38) and blockAlign
// first; avgBytesPerSec from `rate` or as a product scores lower.
// Session 10, no gain: `1 + (stereo != 0)`, `!!stereo + 1` and
// `(stereo ? 1 : 0) + 1` give the same 108; `channels = 1; if (stereo)
// channels = 2;` (34), an inline SetPcmFormat(format*, channels, rate, bits)
// helper (51) and rate first with (unsigned short) casts (68) are worse.
//
// UnknownPCAudioObject 0x004bdc00 (the loader thread; retail extent is 483
// bytes, 0x4bdc00..0x4bdde3): spelling the inner queue loop `while (1)`
// instead of `for (;;)` keeps it top-tested like retail (instruction count
// 155 = retail, candidate 478 bytes; `for (;;)` is rotated, 162
// instructions and 491 bytes, though it scores 95 strict bytes against 91
// here). Left: register allocation. Retail keeps ebp = audio,
// ebx = &audio->field_0x0c then `loaded`, edi = &sound->field_0x48, and
// spills `ready`/`bytes` to one frame slot (frame 0xc); VC6 here caches the
// EnterCriticalSection pointer in ebx, spills `loaded` (frame 8). Tried
// without gain: context macro instead of the `audio` local, `loaded = 0`
// before the lock, function-scope ready/bytes/loaded, one shared variable
// for ready and bytes, a CRITICAL_SECTION* local, inline Lock/Unlock
// helpers, `loaded = 1` after Play (same strict score), swapped bitfield
// stores, `if (!loaded) continue;`, if-wrapping instead of `continue`, the
// ready test inline in the if.

#include <process.h>
#include <string.h>

#include "../../src/reconstructed/PCAudio.h"
#include "../../src/reconstructed/DebugAlloc.h"
#include "../../src/reconstructed/MemTag.h"
#include "../../src/reconstructed/TextureMap.h"
#include "../../src/reconstructed/TrackGame.h"
#include "../../src/reconstructed/UnknownResourceManager.h"

#define SoundSystem() ((PCSoundInterface*)g_TrackGame->soundInterface)

#include "PrimarySoundFormat.cpp"

#include "AudioLoaderThread.cpp"
