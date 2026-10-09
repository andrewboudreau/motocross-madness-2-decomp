// Readerless kVec3 set .CRT$XCU 6-9 at 0x00404e80..0x00404fc0 (globals
// 0x00577818..0x00577854). Any translation unit that includes the shared
// math header (src/krusty2/math/Math3D.h) gets this set whether or not it
// reads the constants; nothing in .text reads these copies, so the unit is
// bounded only by link order (after AuralScape.cpp, before Bike.cpp;
// BackgroundImage.cpp and BaseObject.cpp are the known units in that span).
// The four pairs are the standard copy-initialised dynamic initializers
// (docs/INITIALIZERS.md); the self-contained MatrixUtil.h Vector3 gives the
// same bodies as Math3D.h's Vec3, and needs no include path.
#include "../../src/reconstructed/MatrixUtil.h"

static const Vector3 kVec3Zero = Vector3(0.0f, 0.0f, 0.0f);   // 0x00577828, $E 0x00404e80
static const Vector3 kVec3XAxis = Vector3(1.0f, 0.0f, 0.0f);  // 0x00577838, $E 0x00404ed0
static const Vector3 kVec3YAxis = Vector3(0.0f, 1.0f, 0.0f);  // 0x00577848, $E 0x00404f20
static const Vector3 kVec3ZAxis = Vector3(0.0f, 0.0f, 1.0f);  // 0x00577818, $E 0x00404f70
