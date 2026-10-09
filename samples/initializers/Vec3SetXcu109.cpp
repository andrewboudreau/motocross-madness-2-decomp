// Readerless kVec3 set .CRT$XCU 109-112 at 0x004624e0..0x00462620 (globals
// 0x0065b3f8..0x0065b434). It either opens the Fog unit or closes the
// file-stream helpers (docs/UNATTRIBUTED.md); nothing in .text reads these
// copies. See Vec3SetXcu6.cpp.
#include "../../src/reconstructed/MatrixUtil.h"

static const Vector3 kVec3Zero = Vector3(0.0f, 0.0f, 0.0f);   // 0x0065b408, $E 0x004624e0
static const Vector3 kVec3XAxis = Vector3(1.0f, 0.0f, 0.0f);  // 0x0065b418, $E 0x00462530
static const Vector3 kVec3YAxis = Vector3(0.0f, 1.0f, 0.0f);  // 0x0065b428, $E 0x00462580
static const Vector3 kVec3ZAxis = Vector3(0.0f, 0.0f, 1.0f);  // 0x0065b3f8, $E 0x004625d0
