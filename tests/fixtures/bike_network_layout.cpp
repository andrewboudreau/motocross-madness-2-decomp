// Compile-only access-boundary layout checks in the caller's actual include view.
#include "vehicle/KrustyBike.h"
#include "../../src/reconstructed/Net.h"
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include "../../src/reconstructed/Recorder.h"
#include "../../src/reconstructed/EventManager.h"
#include <stddef.h>
#define CHECK(n,x) typedef char n[(x)?1:-1]
CHECK(game_network,offsetof(KbGame,network)==8);
CHECK(game_session,offsetof(KbGame,eventManager)==0x570);
CHECK(recorder_pointer,offsetof(KrustyBike,netRecorder)==0x13fc);
CHECK(owner_racer,offsetof(KbPlayer,followedRacer)==0xa8);
CHECK(owner_ghost,offsetof(KbPlayer,ball)==0xdc);
CHECK(network_host,offsetof(KbGameCfg,isHost)==offsetof(NetworkInterface,isHost));
extern "C" int BikeRecorderPrimary() {
    return (int)static_cast<GameObject*>(reinterpret_cast<VCRInterface*>(0x1000))-0x1000;
}
