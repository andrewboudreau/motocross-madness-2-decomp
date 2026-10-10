#pragma once

#include "Game.h"
#include "PCRenderTarget.h"

// Minimal Win32 shapes PCGame keeps by value.
struct UnknownRect {
    long left;
    long top;
    long right;
    long bottom;
};

struct UnknownOSVersionInfo {               // OSVERSIONINFOA (0x94 bytes)
    unsigned long size;
    unsigned long majorVersion;
    unsigned long minorVersion;
    unsigned long buildNumber;
    unsigned long platformId;
    char servicePack[128];
};

// RTTI: PCGame : Game (vtable 0x00555e50, 38 slots). Its functions pass the
// literal __FILE__ "D:\\aardvark\\VC\\krusty2\\PCGame.cpp". The Windows layer
// of the game: window and instance handles, cursor and IME handling, and
// the Rainbow Studios registry names. Names are provisional.
class PCGame : public Game {
public:
    PCGame();                                 // 0x004bfaa0
    virtual ~PCGame();                        // 0x004bfc20 (deleting wrapper 0x004bfc00)
    virtual int UnknownVirtualSlot2();        // 0x004bfc40: Game's slot 2
    virtual int UnknownVirtualSlot5();        // 0x004c0250: restores lost surfaces, then the root's slot 18
    virtual int UnknownVirtualSlot6();        // 0x004c0310
    virtual int UnknownVirtualSlot7();        // 0x004c07d0: sets the render states
    virtual int UnknownVirtualSlot13(UnknownControlEvent* event, UnknownInputEntry* entry); // 0x004c03a0
    virtual int UnknownVirtualSlot14(UnknownControlEvent* event, UnknownInputEntry* entry); // 0x004c0400
    virtual int UnknownVirtualSlot15();       // 0x004c0230: shutdown, then shows the cursor
    virtual int UnknownVirtualSlot19(int mode); // 0x004c04a0: changes the display mode
    // Settings under HKEY_LOCAL_MACHINE\<+0x4b8>; "Sub\\Value" names a subkey.
    virtual int GetRegistryInt(const char* name, int defaultValue);       // 0x004c1c20: DWORD
    virtual float GetRegistryFloat(const char* name, float defaultValue); // 0x004c1d50: float
    virtual int GetRegistryFlag(const char* name, int defaultValue);      // 0x004c1e80: flag
    virtual int GetRegistryString(const char* name, const char* defaultValue, char* buffer,
                                  unsigned long* size);                     // 0x004c1fc0: string
    virtual int GetRegistryBinary(const char* name, void* data, unsigned long* size); // 0x004c2110: binary
    virtual int SetRegistryInt(const char* name, int value);              // 0x004c2240: write DWORD
    virtual int UnknownVirtualSlot26(const char* name, int value);            // 0x004c2240 (folded)
    virtual int SetRegistryFlag(const char* name, int value);             // 0x004c2240 (folded)
    virtual int SetRegistryString(const char* name, const char* value);   // 0x004c2370: write string
    virtual int SetRegistryBinary(const char* name, const void* data, unsigned long size); // 0x004c24b0
    virtual RenderTarget* UnknownVirtualSlot31(); // 0x004c0a90: creates the render target
    virtual int UnknownVirtualSlot32();       // 0x004c0c60: sets the display mode
    virtual int UnknownVirtualSlot34(UnknownDisplay* display); // 0x004c05a0: filters display modes
    virtual int UnknownVirtualSlot35(int value); // 0x004c0340: root slot 20 unless input is held
    virtual int UnknownVirtualSlot36(int value); // 0x004c0370: root slot 21 unless input is held
    virtual int UnknownVirtualSlot37();       // 0x004c0c10: the "lobby" command-line switch

    // Game+0x10 is always the PCRenderTarget that slot 31 creates.
    PCRenderTarget* PCTarget() { return (PCRenderTarget*)renderTarget; }

    void SetWindowRect(const UnknownRect* rect);          // 0x004c0470
    // 0x004c0760: in full screen, marks modes larger than width x height
    // unusable (unless the display keeps them).
    int LimitDisplayModes(UnknownDisplay* display, int width, int height);
    // 0x004bfc50: PCGame's start-up before Game's initialiser; fills
    // `message` on failure.
    int StartUp(char* message);
    int ProfileDisplays();                               // 0x004c0d10: profiles the displays, returns 1 (near miss, samples/game)
    int SaveDisplayProfile(UnknownDisplay* display);     // 0x004c1610: saves its profile
    int ProfileEveryDisplay();                           // 0x004c16b0: profiles every display
    int IsAnyProfileStale();                             // 0x004c1410: 1 if any profile is stale
    int DeleteDisplayProfiles();                         // 0x004c1a00: deletes the profiles
    int LoadDisplayProfile(UnknownDisplay* display);     // 0x004c16f0: loads its profile

    UnknownGuid deviceGuid;                   // Direct3D device GUID ("Renderer" setting)
    UnknownRect windowRect;                    // +0x308: window rectangle (slot 32)
    void* instanceHandle;                      // +0x318: instance handle
    void* windowHandle;                        // +0x31c: window handle
    char companyName[0x80];                   // "Rainbow Studios"
    char applicationName[0x80];                // +0x3a0: "Rainbow Demo"
    void* resourceInstance;                    // +0x420: string resource instance (defaults to +0x318)
    UnknownOSVersionInfo osVersion;            // +0x424
    char registryKey[0x80];                    // +0x4b8: "SOFTWARE\\Rainbow Studios\\Demo"
    void* imeLibrary;                          // +0x538: IMM32.DLL
    void* inputContext;                        // +0x53c: created input context
    void* previousInputContext;                // +0x540: previous input context (ImmAssociateContext result)
    int field_0x544;                          // disables the 0x004ccd60 first argument
    unsigned char displayProfilesStale : 1;    // +0x548 bit 0
    int magFilter;                            // texture stage 0 D3DTSS_MAGFILTER (slot 7)
    int minFilter;                            // D3DTSS_MINFILTER
    int mipFilter;                            // D3DTSS_MIPFILTER
};
