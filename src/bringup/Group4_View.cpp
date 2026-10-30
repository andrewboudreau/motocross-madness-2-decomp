// Bring-up definition (best-guess-unreviewed): UnknownKrustyBikeView is the
// view RaceView.h/InGameProcs.cpp use for the race object whose 0x0041f1d0
// is BikeRace::NextCameraTarget (BikeRaceCore.h; the body is provided by
// src/bringup/Group1_BikeRaceCamera.cpp). Both names bind to the same retail
// address and the view pointer is the race object itself, so this forwards.
//
// Minimal local declarations: only the member functions are named, so the
// mangled names equal the ones the real headers produce.
class BikeRace {
public:
    void NextCameraTarget(int forward, int racers, int objects); // 0x0041f1d0
};

struct UnknownKrustyBikeView {
    void NextCameraTarget(int forward, int racers, int objects); // 0x0041f1d0
};

// 0x0041f1d0 (ret 0xc): the next (`forward`) or previous camera target.
void UnknownKrustyBikeView::NextCameraTarget(int forward, int racers, int objects)
{
    reinterpret_cast<BikeRace*>(this)->NextCameraTarget(forward, racers, objects);
}
