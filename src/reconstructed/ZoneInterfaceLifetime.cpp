// TrackGame::CreateGameObjects calls the folded constructor at 0x004676a0
// on its newly allocated networkGameObject (+0x3410). The address also serves
// unrelated constructors; it does not establish a shared class or original TU.
#include "TrackGame.h"

UnknownTrackGameObject3410Base::UnknownTrackGameObject3410Base() {
    directPlay = 0;
    lobby = 0;
}
