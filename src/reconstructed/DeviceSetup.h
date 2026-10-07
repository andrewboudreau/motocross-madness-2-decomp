#pragma once

// DeviceSetup.cpp (provisional name): the control-layout unit
// 0x00448960..0x00449e60 (UnknownTrackGameObject33fc's methods, declared in
// TrackGame.h, and the key-name helpers after them). See docs/DEVICESETUP.md.

// 0x00448560: the DirectX SDK GetDXVersion sample with a "Blade.dll" probe
// (near miss: samples/game/DeviceSetupNearMisses.cpp). Called from the
// startup code 0x004a08f7.
void GetDXVersion(unsigned long* dxVersion, unsigned long* dxPlatform, unsigned long* blade);

// Control-file helpers in the unattributed range 0x0047b670..0x0047bb20:
// each joins the current directory and `file` ("%s\\%s") and calls
// GetPrivateProfileString / GetPrivateProfileInt / WritePrivateProfileString.
void UnknownFunction47b930(const char* section, const char* key, const char* defaultValue,
                           char* buffer, int size, const char* file);
int UnknownFunction47b9e0(const char* section, const char* key, int defaultValue,
                          const char* file);
void UnknownFunction47ba80(const char* section, const char* key, const char* value,
                           const char* file);

// 0x00449270: "<resource 0x1406> <GetKeyNameText>" for DirectInput scan code
// `code`; "Unknown" and 0 when Windows has no name for it.
int UnknownFunction449270(int code, char* text);
// 0x00449d40: `code` translated through the NEC PC-98 table when that
// keyboard is in use on Windows 9x.
int UnknownFunction449d40(int code);
// 0x00449d90: detects the keyboard layout and the platform once.
void UnknownFunction449d90();
// 0x00449db0: whether Windows is NT.
int UnknownFunction449db0();
// 0x00449de0: Japanese keyboard subtype (0x62, 0x65, 0x6a; 0 otherwise).
int UnknownFunction449de0();
// 0x00449e30: `value`, or 0 with the NEC PC-98 keyboard on Windows 9x.
int UnknownFunction449e30(int code, int value);
