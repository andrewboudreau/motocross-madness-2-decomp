#pragma once

// NetThread.h -- functions of D:\aardvark\VC\krusty2\NetThread.cpp
// (0x004af680..0x004aff73). Every name is provisional (tier 3); the calling
// conventions and argument order are decoded from retail call sites.

// NetworkInterface 0x004acc50 as this file calls it. Retail pushes a float
// here (0x004af809, fstp [esp]): the seconds since the previous keep-alive
// pass. Net.cpp's body uses the argument as an int (the initial lost
// player id), so its declaration keeps int and the true type is unresolved.
// The thread calls the method through this view, as ControlInterface.h's
// UnknownKeyboardBoolView does for a similar conflict. A second overload in
// Net.h is not used: it reorders operands in TrackGame slot 1.
class UnknownNetKeepAliveView {
public:
    void UnknownFunction4acc50(float elapsed);
};

// 0x004af680: NetworkInterface+0x78 read under the +0x60 lock.
int UnknownFunction4af680(NetworkInterface* net);

// 0x004af6a0: the receive thread NetworkInterface::UnknownFunction4ab6b0
// starts with _beginthreadex; it waits on the three events at +0x24/+0x2c/+0x28.
unsigned int __stdcall UnknownFunction4af6a0(void* net);

// 0x004af8e0: drains the DirectPlay receive queue; returns an HRESULT.
long UnknownFunction4af8e0(unsigned int time);

// 0x004afa40: whether a received message equals a stored NetMessage.
int UnknownFunction4afa40(void* data, unsigned int size, int from, int type, void* otherData,
                          unsigned int otherSize, int otherFrom, int otherType);

// 0x004afa90: routes or handles one received message.
void UnknownFunction4afa90(unsigned char* data, unsigned int size, int from, int to,
                           unsigned int time);
