// TrackGame::CreateGameObjects calls the folded constructor at 0x004676a0
// on its newly allocated networkGameObject (+0x3410). The address also serves
// unrelated constructors; it does not establish a shared class or original TU.
#include "TrackGame.h"

MSZoneInterfaceBase::MSZoneInterfaceBase() {
    directPlay = 0;
    lobby = 0;
}
