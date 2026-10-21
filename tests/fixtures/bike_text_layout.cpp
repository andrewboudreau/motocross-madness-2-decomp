#include <new>
#include <stddef.h>
#include "vehicle/KrustyBike.h"
#include "../../src/reconstructed/EventManager.h"
#include "../../src/reconstructed/TrackGame.h"
#define CHECK(n,x) typedef char n[(x)?1:-1]
CHECK(message_storage,sizeof(KbMessage)==sizeof(UnknownMessage));
// KbMessage was opaque byte storage. These callers allocate it separately;
// no embedded-struct alignment equivalence is asserted.
CHECK(event_manager,offsetof(KbGame,eventManager)==offsetof(TrackGame,eventManager));
extern "C" int BikeTextLayouts() {return (int)static_cast<GameObject*>(reinterpret_cast<TextQueueOverlay*>(0x1000))-0x1000;}
UnknownMessage* ConstructMessage(void* storage,const char* text,float duration) {
    return new(storage) UnknownMessage(text,duration);
}
