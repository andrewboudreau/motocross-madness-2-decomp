#include "MatrixUtil.h"

// Writable debug geometry consumed by BikeRace's slot 14 (0x00423410).
// Retail AI stores six float words per line; these are two-element arrays,
// although VC6 decorates their external symbols with pointer-like spelling.
// Original translation-unit ownership is not established.
Vector3 g_ObstacleDebugLeftLine[2]; // 0x005779d8
int g_ObstacleDebugUpperHit; // 0x00577a00
Vector3 g_ObstacleDebugRightLine[2]; // 0x00577a18
Vector3 g_ObstacleDebugLine[2]; // 0x00577a30
Vector3 g_ObstacleDebugLowerLine[2]; // 0x00577a48
int g_ObstacleDebugHit; // 0x00578da8
int g_ObstacleDebugLowerHit; // 0x00578dac
Vector3 g_PathTargetDebugLine[2]; // 0x00578dc0
int g_ObstacleDebugRightHit; // 0x00578dd8
Vector3 g_ObstacleDebugUpperLine[2]; // 0x00578de0
int g_ObstacleDebugLeftHit; // 0x00578df8
int g_LodDebugSelectedRow = 2; // 0x00567a88

// Vector3's ordinary empty default constructor leaves zero-initialized static
// storage intact. VC6 emits no-op startup initializers for these arrays; their
// compiler-local identities are not attributed to original retail functions.
