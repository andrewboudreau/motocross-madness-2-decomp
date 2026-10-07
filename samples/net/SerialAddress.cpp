// SerialAddress.cpp -- the serial-connection helpers 0x004ae2f0..0x004ae454
// between Net.cpp's last function (the EnumPlayers callback 0x004ae270) and
// NetProcs.cpp's first literal user (0x004ae460). They have no __FILE__ or
// RTTI, so either file may own them (docs/NET.md, docs/DIALOGPROCS.md);
// they stay here until something decides it. Names are ours.
//
// UnknownFunction4ae410 converts SerialPopupDlg's selection indices
// (UnknownSerialSettings, NetProcs.h) into the DirectPlay DPCOMPORTADDRESS
// values (UnknownComPortAddress, DlgProcs.h): CBR_* baud rates, ONESTOPBIT..
// TWOSTOPBITS, NOPARITY/EVENPARITY/ODDPARITY/MARKPARITY (the dialog lists
// even before odd) and DPCPA_NOFLOW..DPCPA_RTSDTRFLOW.

#include "../../src/reconstructed/NetProcs.h"
#include "../../src/reconstructed/DlgProcs.h"

// 0x004ae2f0: "ButBaud" index to baud rate.
static int SerialBaudRate(int index) {
    switch (index) {
    case 0: return 4800;
    case 1: return 9600;
    case 2: return 14400;
    case 3: return 19200;
    case 4: return 38400;
    case 5: return 56000;
    case 6: return 57600;
    case 7: return 115200;
    case 8: return 128000;
    case 9: return 256000;
    }
    return 0;
}

// 0x004ae370: "ButStop" index to stop bits (ONESTOPBIT, ONE5STOPBITS, TWOSTOPBITS).
static int SerialStopBits(int index) {
    switch (index) {
    case 0: return 0;
    case 1: return 1;
    case 2: return 2;
    }
    return 0;
}

// 0x004ae390: "ButParity" index (none, even, odd, mark) to NOPARITY,
// EVENPARITY, ODDPARITY, MARKPARITY.
static int SerialParity(int index) {
    switch (index) {
    case 0: return 0;
    case 1: return 2;
    case 2: return 1;
    case 3: return 3;
    }
    return 0;
}

// 0x004ae3d0: "ButFlow" index to DPCPA_NOFLOW .. DPCPA_RTSDTRFLOW.
static int SerialFlowControl(int index) {
    switch (index) {
    case 0: return 0;
    case 1: return 1;
    case 2: return 2;
    case 3: return 3;
    case 4: return 4;
    }
    return 0;
}

// 0x004ae410
void UnknownFunction4ae410(UnknownSerialSettings* settings, UnknownComPortAddress* address) {
    address->port = settings->port + 1;              // "ButPort" index to COM number
    address->baudRate = SerialBaudRate(settings->baudRate);
    address->stopBits = SerialStopBits(settings->stopBits);
    address->parity = SerialParity(settings->parity);
    address->flowControl = SerialFlowControl(settings->flowControl);
}
