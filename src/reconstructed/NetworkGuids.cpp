#include <windows.h>

// Readonly identifiers used by Net.cpp and MSZoneInterface.cpp. Values and
// storage are confirmed from retail; names describe the observed uses. This
// provider does not establish the original defining translation unit.
typedef char NetworkGuidSizeMustBe16[(sizeof(GUID) == 16) ? 1 : -1];

// 0x00556d80: interface requested when creating the Zone score object.
extern const GUID g_ZoneScoreInterfaceGuid = {
    0x3faf0afd, 0xb48b, 0x11d2, {0x8a, 0x51, 0x00, 0xc0, 0x4f, 0x8e, 0xf4, 0xe9}
};
// 0x00556da0: class requested by the same CoCreateInstance call.
extern const GUID g_ZoneScoreClassGuid = {
    0x3faf0afe, 0xb48b, 0x11d2, {0x8a, 0x51, 0x00, 0xc0, 0x4f, 0x8e, 0xf4, 0xe9}
};
// 0x00556db0: player tag sent with both lobby property queries.
extern const GUID g_LobbyQueryPlayerGuid = {
    0x184dbbc1, 0xff39, 0x11cf, {0xb1, 0x12, 0x00, 0xa0, 0xc9, 0x0a, 0xea, 0xb6}
};
// 0x00556dc0: application tag used by DirectPlay and Zone score reporting.
extern const GUID g_NetApplicationGuid = {
    0x184dbbc2, 0xff39, 0x11cf, {0xb1, 0x12, 0x00, 0xa0, 0xc9, 0x0a, 0xea, 0xb6}
};
// 0x00556dd0: property tag requested by GetLobbyPreset.
extern const GUID g_LobbyPresetPropertyGuid = {
    0xb31465e4, 0xaa74, 0x4c4e, {0xae, 0xba, 0x42, 0x1b, 0xc8, 0x38, 0xfb, 0x06}
};
// 0x00556de0: property tag requested by GetLobbyRank.
extern const GUID g_LobbyRankPropertyGuid = {
    0xdf629fe8, 0x5a83, 0x455b, {0x81, 0x2d, 0x0d, 0xbb, 0xfa, 0xf1, 0xad, 0x11}
};
