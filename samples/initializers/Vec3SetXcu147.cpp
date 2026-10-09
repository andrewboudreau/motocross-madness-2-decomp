// Readerless kVec3 set .CRT$XCU 147-150 at 0x00489640..0x00489780 (globals
// 0x0067c2b8..0x0067c2f4), after InGameProcs.cpp and before the InputDevice
// run; nothing in .text reads these copies. See Vec3SetXcu6.cpp.
#include "../../src/reconstructed/MatrixUtil.h"

static const Vector3 kVec3Zero = Vector3(0.0f, 0.0f, 0.0f);   // 0x0067c2c8, $E 0x00489640
static const Vector3 kVec3XAxis = Vector3(1.0f, 0.0f, 0.0f);  // 0x0067c2d8, $E 0x00489690
static const Vector3 kVec3YAxis = Vector3(0.0f, 1.0f, 0.0f);  // 0x0067c2e8, $E 0x004896e0
static const Vector3 kVec3ZAxis = Vector3(0.0f, 0.0f, 1.0f);  // 0x0067c2b8, $E 0x00489730
