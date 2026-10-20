#pragma once

// One pointer at retail 0x0056e26c, initialized to the TrackGame singleton
// at 0x006851a0. The definition lives in MSZoneInterface.cpp. This minimal
// declaration avoids importing the full reconstructed class into local views.
class TrackGame;
extern TrackGame* g_TrackGame;

// Callers explicitly cast this pointer to their provisional offset views.
// Every access reads the shared pointer; no view owns or caches another copy.
