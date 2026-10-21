// The texture group's embedded timer constructor at 0x004cb670.
// This preserves its existing view; it does not establish original class identity.
#include "TextureMapManager.h"

unsigned int ReadClock();

UnknownTimer::UnknownTimer(int interval) {
    field_0x04 = 0;
    timerStart = (int)ReadClock();
    timerInterval = interval;
}
