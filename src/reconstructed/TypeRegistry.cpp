#include <string.h>

#include "TypeRegistry.h"

#include "DebugAlloc.h"
#include "MatrixUtil.h"

// The four per-TU vector constants (see src/krusty2/math/Math3D.h):
// 0x0068a460, 0x0068a470, 0x0068a480 and 0x0068a450, initialised by
// 0x00521d00..0x00521e3b.
static const Vector3 s_UnknownVector68a460 = Vector3(0.0f, 0.0f, 0.0f);
static const Vector3 s_UnknownVector68a470 = Vector3(1.0f, 0.0f, 0.0f);
static const Vector3 s_UnknownVector68a480 = Vector3(0.0f, 1.0f, 0.0f);
static const Vector3 s_UnknownVector68a450 = Vector3(0.0f, 0.0f, 1.0f);

// Constructed by 0x00521e40..0x00521e75 (atexit destructor).
TypeRegistry g_TypeRegistryInstance;
TypeRegistry* g_TypeRegistry = &g_TypeRegistryInstance;

TypeRegistry::TypeRegistry() {
    count = 0;
    names = 0;
}

// 0x00521e80
TypeRegistry::~TypeRegistry() {
    if (names)
        operator delete(names, __FILE__, 19);
}

// 0x00521ea0
char TypeRegistry::FindTypeId(const char* name) {
    int i;

    for (i = 0; i < count; i++) {
        if (!strncmp(name, names[i], sizeof(names[i])))
            return (char)i;
    }
    count++;
    names = (char(*)[0x80])DebugRealloc(names, count * sizeof(names[0]), __FILE__, 42);
    memset(names[i], 0, sizeof(names[i]));
    strncpy(names[i], name, sizeof(names[i]));
    return (char)(count - 1);
}
