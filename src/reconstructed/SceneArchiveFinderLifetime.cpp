// Scene finder destruction is independently identified by TrackGame::Shutdown:
// it calls this body on +0x574, then separately deallocates the same pointer.
// This grouping does not establish original translation-unit ownership.
#include "SceneArchiveFinder.h"

// Complete retail body at 0x00464e90 is RET. In particular it does not release
// sceneArchive; preserve the observed destructor instead of inventing cleanup.
UnknownTrackGameObject574::~UnknownTrackGameObject574() {
}
