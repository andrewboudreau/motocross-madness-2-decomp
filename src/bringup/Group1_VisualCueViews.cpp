// Group1_VisualCueViews.cpp -- bring-up forwarders around VisualCue.
//
// * QuarryEvent.h declares VisualCue::LoadVisualCue (0x0048adf0, ret 0x34) with
//   QuarryStuntEvent's argument types; Krusty3DObjects.cpp defines the same retail
//   function with Krusty3DObjects.h's types.  The QuarryEvent overload forwards
//   every argument unchanged (the light manager travels as the int `lights`).
// * VisualCue::Tick and BonusObjectManager (src/reconstructed/Krusty3DObjects.cpp)
//   call a provisional view method whose retail target is already reconstructed
//   under its canonical class:
//     BonusSoultreeObjectView::GetPosition 0x004fc970 = SoultreeObject::GetPosition
//   (GetPosition's returned pointer is the argument and is ignored by the view).
// The two VisualCue declarations (QuarryEvent.h derives from GameObject,
// Krusty3DObjects.h from ArcadeObject) cannot share a translation unit, so the
// classes are declared here with only the members needed, same class keys and
// therefore the canonical mangled names.

#include "MatrixUtil.h"   // Vector3 (passed by value)

class LightManager;
class OverlayTerrainView;
struct EventBikeRaceView;
struct QuarryEventKrustyBikeCameraView;
struct RunwayTerrainView;
struct VisualCueBikeRaceView;
struct ArcadeCameraView;
struct TerrainVec3;
struct Vec3;

class VisualCue {
public:
    // Krusty3DObjects.h (defined in Krusty3DObjects.cpp)
    VisualCue* LoadVisualCue(void* context, int lights, int options, RunwayTerrainView* terrain,
                             Vector3 position, VisualCueBikeRaceView* view, int pixels,
                             ArcadeCameraView* camera, float size, float screenXFraction,
                             float screenYFraction);
    // QuarryEvent.h
    VisualCue* LoadVisualCue(void* target, LightManager* lights, int options, OverlayTerrainView* terrain,
                             Vector3 offset, EventBikeRaceView* view, int count,
                             QuarryEventKrustyBikeCameraView* camera, float size, float screenXFraction, float screenYFraction);
};

// 0x0048adf0, QuarryStuntEvent's view.
VisualCue* VisualCue::LoadVisualCue(void* target, LightManager* lights, int options,
                                    OverlayTerrainView* terrain, Vector3 offset,
                                    EventBikeRaceView* view, int count,
                                    QuarryEventKrustyBikeCameraView* camera, float size, float screenXFraction, float screenYFraction)
{
    return LoadVisualCue(target, (int)lights, options, (RunwayTerrainView*)terrain, offset,
                         (VisualCueBikeRaceView*)view, count, (ArcadeCameraView*)camera,
                         size, screenXFraction, screenYFraction);
}

class SoultreeObject {
public:
    Vec3* GetPosition(Vec3* out);
};

struct BonusSoultreeObjectView {
    void GetPosition(Vector3* translation);
};

// 0x004fc970
void BonusSoultreeObjectView::GetPosition(Vector3* translation)
{
    reinterpret_cast<SoultreeObject*>(this)->GetPosition((Vec3*)translation);
}
