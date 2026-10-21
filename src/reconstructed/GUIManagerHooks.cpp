// Palette-related direct calls in GUIManager::Tick and UIDialog destruction.
// The original method name and translation-unit ownership remain unproven.
#include "GUIManagerCore.h"

// Complete retail body at 0x00464e90 is RET. Caller evidence binds this role;
// sharing that tiny address with other classes does not establish identity.
// Keep out of line so the original callers retain their external call.
void GUIManager::UnknownFunction464e90() {
}
