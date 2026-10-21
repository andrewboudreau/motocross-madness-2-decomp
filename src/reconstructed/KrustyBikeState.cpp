// Writable bike controls confirmed from retail storage and decoded callers.
// This data-only grouping does not establish original TU ownership.
// Scalar declarations are shared with KrustyBikeTypes.h; this storage-only
// source does not need its class and per-TU vector-initializer dependencies.

// File-backed initial values; race setup subsequently reloads these controls.
int g_kbLatencyHiding = 1;       // 0x0056cb3c
int g_kbRateLimiting = 1;        // 0x0056cb40
int g_kbAllowWarping = 1;        // 0x0056cb44
int g_kbUseExtrapLimit = 1;      // 0x0056cb48
float g_kbExtrapLimit = 0.5f;    // 0x0056cb4c; setup's 100 ms default is a later write
float g_kbLatencyThreshold = 0.5f; // 0x0056cb50
float g_kbWarpThreshold = 0.2f;  // 0x0056cb54
int g_kbUseTimeReceived = 1;     // 0x0056cb58
int g_kbAllowNegative = 1;       // 0x0056cb5c
int g_kbInterpolate = 1;         // 0x0056cb60

// Loader-zeroed retail storage, not file-backed initialized words.
int g_kbUseLatencyThreshold;    // 0x0067c3a0
int g_kbPadActive;              // 0x0067c3ac
