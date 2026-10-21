// Compile-only call-boundary probes. No unconstructed object is executed.
#include "../../src/reconstructed/GUIManagerCore.h"
#include "../../src/reconstructed/SceneArchiveFinder.h"
#include <stddef.h>

typedef char PaletteChangedOffset[offsetof(GUIManager, paletteChanged) == 0x1f0 ? 1 : -1];
typedef char FinderArchiveOffset[offsetof(UnknownTrackGameObject574, sceneArchive) == 0 ? 1 : -1];
typedef char FinderSize[sizeof(UnknownTrackGameObject574) == 0x394 ? 1 : -1];

void InvokeGuiEmptyHook(GUIManager* manager) {
    manager->UnknownFunction464e90();
}
void DestroySceneFinder(UnknownTrackGameObject574* finder) {
    finder->~UnknownTrackGameObject574();
}
