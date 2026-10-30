// Group2_KrustyBikeCameraTick.cpp -- link bring-up: KrustyBikeCamera::Tick (0x00497e20,
// vtable slot 10). Body from samples/camera/KrustyBikeCameraNearMisses.cpp (register-rotation
// near miss), checked against the retail disassembly: jump table 0..5 -> +0x55c, +0x560,
// +0x564, +0x558, +0x568, +0x560 (quarry, baja, national, supercross, tag, enduro).
//
// The view's position writer is retail 0x004210f0, bound in src as
// BikeRace::WriteRelativePositions (BikeRace.bindings.json); RaceView.h's view
// declares the same function on UnknownKrustyBikeView (a view of the same object,
// BikeRace.h). It is called through a BikeRace view here so no second name is introduced.
#include "../reconstructed/KrustyBikeCamera.h"
#include "../reconstructed/TrackGame.h"

class BikeRace {
public:
    // 0x004210f0: writes two positions relative to `reference`.
    void WriteRelativePositions(Vector3* a, Vector3* b, void* reference, int flags);
};

namespace {

// 0x0067c3f4 (.bss byte): set while the network start view is active.
bool s_networkStartViewActive;

// 0x0067c3e8 is KrustyBikeCamera.cpp's file-static kVec3YAxis; this unit keeps its own copy.
Vector3 s_upAxis(0.0f, 1.0f, 0.0f);

} // namespace

// Picks the view by the global event type on first use. While the view has its network
// start pending it drives FollowCamera slots 46 (once) and 47 from it; otherwise it leaves
// that mode, restoring the state, and runs FollowCamera slot 10.
int KrustyBikeCamera::Tick(float frameTime) {
    if (!raceView) {
        switch (g_TrackGame->mode.raceSettings.eventType) {
            case 2:
                raceView = g_TrackGame->nationalEvent->raceView;
                break;
            case 3:
                raceView = g_TrackGame->supercrossEvent->raceView;
                break;
            case 0:
                raceView = g_TrackGame->quarryEvent->raceView;
                break;
            case 1:
            case 5:
                raceView = g_TrackGame->bajaEnduroEvent->raceView;
                break;
            case 4:
                raceView = g_TrackGame->tagEvent->raceView;
                break;
        }
    }
    if (raceView->networkStartPending) {
        Vector3 first;
        Vector3 second;
        BikeRace* race = (BikeRace*)raceView;
        if (!s_networkStartViewActive) {
            s_networkStartViewActive = true;
            race->WriteRelativePositions(&second, &first, g_TrackGame->bajaEnduroEvent, 0);
            FollowCamera::UnknownVirtualSlot46(&first, &s_upAxis);
        }
        race->WriteRelativePositions(&first, &second, g_TrackGame->bajaEnduroEvent, 0);
        first.y = raceView->localRacer->position.y + 2.0f;
        FollowCamera::UnknownVirtualSlot47(frameTime, &first);
        return 1;
    }
    if (s_networkStartViewActive) {
        s_networkStartViewActive = false;
        UnknownVirtualSlot62();
        UnknownVirtualSlot71(cameraState);
    }
    FollowCamera::Tick(frameTime);
    return 1;
}
