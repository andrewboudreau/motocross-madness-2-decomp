// Group1_VisualCueViews.cpp -- bring-up forwarders around VisualCue.
//
// * QuarryEvent.h declares VisualCue::LoadVisualCue (0x0048adf0, ret 0x34) with
//   QuarryStuntEvent's argument types; Krusty3DObjects.cpp defines the same retail
//   function with Krusty3DObjects.h's types.  The QuarryEvent overload forwards
//   every argument unchanged (the light manager travels as the int `lights`).
// * VisualCue::Tick (Group1_VisualCueTick.cpp) calls two provisional view methods
//   whose retail targets are already reconstructed under canonical classes:
//     UnknownRunwayTerrain::QueryGround 0x00507c10 = Terrain::QueryGround
//     UnknownBonusRacerPart::GetPosition 0x004fc970 = SoultreeObject::GetPosition
//   (the caller passes 0 for the surface pointer; GetPosition's returned pointer is
//   the argument and is ignored by the view).
// The two VisualCue declarations (QuarryEvent.h derives from GameObject,
// Krusty3DObjects.h from ArcadeObject) cannot share a translation unit, so the
// classes are declared here with only the members needed, same class keys and
// therefore the canonical mangled names.

#include "MatrixUtil.h"   // Vector3 (passed by value)

class LightManager;
class UnknownTerrain;
struct UnknownKrustyBikeView;
struct UnknownQuarryCamera;
struct UnknownRunwayTerrain;
struct UnknownVisualCueView;
struct UnknownArcadeView;
struct TerrainVec3;
struct Vec3;

class VisualCue {
public:
    // Krusty3DObjects.h (defined in Krusty3DObjects.cpp)
    VisualCue* LoadVisualCue(void* context, int lights, int options, UnknownRunwayTerrain* terrain,
                             Vector3 position, UnknownVisualCueView* view, int pixels,
                             UnknownArcadeView* camera, float size, float screenXFraction,
                             float screenYFraction);
    // QuarryEvent.h
    VisualCue* LoadVisualCue(void* target, LightManager* lights, int value, UnknownTerrain* terrain,
                             Vector3 offset, UnknownKrustyBikeView* view, int count,
                             UnknownQuarryCamera* camera, float a9, float a10, float a11);
};

// 0x0048adf0, QuarryStuntEvent's view.
VisualCue* VisualCue::LoadVisualCue(void* target, LightManager* lights, int value,
                                    UnknownTerrain* terrain, Vector3 offset,
                                    UnknownKrustyBikeView* view, int count,
                                    UnknownQuarryCamera* camera, float a9, float a10, float a11)
{
    return LoadVisualCue(target, (int)lights, value, (UnknownRunwayTerrain*)terrain, offset,
                         (UnknownVisualCueView*)view, count, (UnknownArcadeView*)camera,
                         a9, a10, a11);
}

class Terrain {
public:
    void QueryGround(TerrainVec3* pos, TerrainVec3* normal, int flatShaded, unsigned char* surface);
};

struct UnknownRunwayTerrain {
    void QueryGround(Vector3* point, Vector3* normal, int flatShaded, int surface);
};

// 0x00507c10
void UnknownRunwayTerrain::QueryGround(Vector3* point, Vector3* normal, int flatShaded, int surface)
{
    reinterpret_cast<Terrain*>(this)->QueryGround((TerrainVec3*)point, (TerrainVec3*)normal,
                                                  flatShaded, (unsigned char*)surface);
}

class SoultreeObject {
public:
    Vec3* GetPosition(Vec3* out);
};

struct UnknownBonusRacerPart {
    void GetPosition(Vector3* translation);
};

// 0x004fc970
void UnknownBonusRacerPart::GetPosition(Vector3* translation)
{
    reinterpret_cast<SoultreeObject*>(this)->GetPosition((Vec3*)translation);
}
