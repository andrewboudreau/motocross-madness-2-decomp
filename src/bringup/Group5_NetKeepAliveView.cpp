// Group5 bring-up definition (best-guess, link frontier). Not a strict match.
//
// 0x004acc50 is one retail function seen through two declarations:
// Net.cpp's NetworkInterface::ResendPending(int) (defined in src) and
// NetThread.cpp's NetKeepAliveView::ResendPending(float). Retail passes the
// argument dword straight through to NetPendingList::ResendExpired (as its
// second argument) and only uses it as the initial "lost player" register,
// which is never read unless a player is lost. So the float view forwards
// its raw bits to the existing body.
#include <windows.h>
#include "Net.h"
#include "NetThread.h"

void NetKeepAliveView::ResendPending(float elapsed)
{
    // best guess: same dword reaches NetworkInterface::ResendPending's body.
    int bits = *reinterpret_cast<int*>(&elapsed);
    reinterpret_cast<NetworkInterface*>(this)->ResendPending(bits);
}
