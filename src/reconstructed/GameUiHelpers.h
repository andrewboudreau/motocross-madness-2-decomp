#pragma once

// GameUiHelpers.cpp (provisional name): the six cdecl helpers at
// 0x0047b670..0x0047bb20, after gameui.cpp's vcall thunks (0x0047b66f) and
// before GhostMod.cpp (0x0047bb20). No literal __FILE__, RTTI or source
// reference reaches the range; the ownership tier is 3. The same functions
// are also declared where their callers live (RaceStatus.h, EcoSystem.h,
// DeviceSetup.h).

#include "MatrixUtil.h"

struct UnknownBikeRaceNode;

// 0x0047b670: whether moving from `from` to `to` crosses `gate` (a box of
// BikeRace.h's UnknownBikeRaceNode kind): the x/z segment must cut the
// gate's across line within both, the height at the crossing must be within
// the gate's height, and with `direction` the move must go along the gate's
// facing. A gate at the origin never matches.
int UnknownFunction47b670(Vector3* from, Vector3* to, UnknownBikeRaceNode* gate, int direction);

// 0x0047b800: intersects the segment p0 + t * d0 with p1 + u * d1 (2-D),
// writing the parameters. 0 when the lines are parallel or a pointer is null.
int UnknownFunction47b800(float p0x, float p0y, float d0x, float d0y,
                          float p1x, float p1y, float d1x, float d1y,
                          float* t, float* u);

// 0x0047b8a0: a profile value as a float, `defaultValue` when the key is
// missing ("NONE").
float UnknownFunction47b8a0(const char* section, const char* key, double defaultValue,
                            const char* path);

// 0x0047b930, 0x0047b9e0 and 0x0047ba80: the profile API on `file` joined
// to the current directory ("%s\\%s") unless it already holds a backslash.
void UnknownFunction47b930(const char* section, const char* key, const char* defaultValue,
                           char* buffer, int size, const char* file);
int UnknownFunction47b9e0(const char* section, const char* key, int defaultValue,
                          const char* file);
void UnknownFunction47ba80(const char* section, const char* key, const char* value,
                           const char* file);
