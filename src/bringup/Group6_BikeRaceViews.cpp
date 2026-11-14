// Group6_BikeRaceViews.cpp -- bring-up forwarders for the provisional views that
// BikeRace::Create (Group6_BikeRaceCreate.cpp) calls.  Each view method is a retail
// function already reconstructed under its canonical class (addresses from the
// *.bindings.json files), so it forwards with the same `this`:
//   StartD3DIMSoultreeObjectView::SetUseLighting    0x004444c0 = D3DIMSoultreeObject::SetUseLighting
//   StartD3DIMSoultreeObjectView::SetAxesPtr        0x004fbd70 = SoultreeObject::SetAxesPtr
//   StartD3DIMSoultreeObjectView::SetPositionVec3   0x004fc630 = D3DIMSoultreeObject::SetPositionVec3
//   RacerD3DIMSoultreeObjectView::GetPosition      0x004fc970 = SoultreeObject::GetPosition
//   BikeRaceTerrainView::QueryGround  0x00507c10 = Terrain::QueryGround
//   BikeRaceKrustyBikeView::SetNetRecorder 0x00496e20 = KrustyBike::SetNetRecorder
//     (the racer pointer is KrustyBike's primary pointer; docs/BIKE_RACE_STATE_CONNECTIONS.md)
//   RacerD3DIMSoultreeObjectView::ReplaceMaterialTexture 0x00444c70 = D3DIMSoultreeObject::ReplaceMaterialTexture
//   BikeRaceKrustyBikeView::Load             0x0048fc80 = KrustyBike::Load (ret 0x7c both)
// No project header is included: the declarations below repeat only the members
// needed, with the same class keys, so every mangled name equals the canonical one.
// None of these classes gets a constructor here, so no vtable is emitted.
// Vec3/TerrainVec3/Vector3 are all three packed floats.

struct Vec3 {
    float x, y, z;
};
struct Vector3 {
    float x, y, z;
};
struct TerrainVec3;
struct KbRecorder;
struct SoultreeLoadDesc;
struct VehicleInputMap;
struct KbRace;
struct KbBikeSetup;
class KrustyVCR;
class GameObject;
class LightManager;
struct TextureContext;
class ControlInterface;
class BikeRace;
struct BikeEngineSetup;
class Scene;

class SoultreeObject {
public:
    Vec3* GetPosition(Vec3* out);
    void SetAxesPtr(const Vec3* look, const Vec3* up, int orthogonalize, int keepZ);
};

class D3DIMSoultreeObject {
public:
    void SetUseLighting(int value);
    int ReplaceMaterialTexture(int index, const char* name, int textures);
    void SetPositionVec3(Vector3 position);
};

class Terrain {
public:
    void QueryGround(TerrainVec3* pos, TerrainVec3* normal, int flatShaded, unsigned char* surface);
};

class KrustyBike {
public:
    void SetNetRecorder(KbRecorder* recorder);
    GameObject* Load(int context, int engineName, const char* modelPath, const SoultreeLoadDesc* riderModel, int desc,
                     int info, Vec3 startPosition, Vec3 forward, Vec3 up, int terrainProbe,
                     VehicleInputMap* map, unsigned char kind, unsigned char remote, KbRace* race,
                     int riderName, int a16, int netId, const KbBikeSetup* setup, int gasGyroOption,
                     int brakeGyroOption, int linkedObject, int trackObject, KbRecorder* recorder,
                     void* aiSkill, int withRider);
};

// ---- the views (same keys and signatures as BikeRace.h / Group6_BikeRaceCreate.cpp)

struct StartD3DIMSoultreeObjectView {
    void SetUseLighting(int value);
    void SetPositionVec3(Vector3 position);
    void SetAxesPtr(const Vector3* look, const Vector3* up, int orthogonalize, int keepZ);
};

class RacerD3DIMSoultreeObjectView {
public:
    void GetPosition(Vector3* position);
    int ReplaceMaterialTexture(int index, const char* name, int textures);
};

class BikeRaceTerrainView {
public:
    void QueryGround(Vector3* point, Vector3* normal, int flatShaded, unsigned char* surface);
};

class BikeRaceKrustyBikeView {
public:
    void SetNetRecorder(KrustyVCR* vcr);
    GameObject* Load(void* owner, const char* engineName, const char* bikeFile, const char* riderFile,
                     LightManager* lights, TextureContext* textures, Vector3 position,
                     Vector3 direction, Vector3 up, BikeRaceTerrainView* terrain,
                     ControlInterface* control, char ai, int remote, BikeRace* race,
                     const char* name, int b, int id, BikeEngineSetup* setup, int gasGyroOption, int brakeGyroOption,
                     int particles, Scene* scene, KrustyVCR* vcr, float skill, int withRider);
};

void StartD3DIMSoultreeObjectView::SetUseLighting(int value)
{
    reinterpret_cast<D3DIMSoultreeObject*>(this)->SetUseLighting(value);
}

void StartD3DIMSoultreeObjectView::SetPositionVec3(Vector3 position)
{
    reinterpret_cast<D3DIMSoultreeObject*>(this)->SetPositionVec3(position);
}

void StartD3DIMSoultreeObjectView::SetAxesPtr(const Vector3* look, const Vector3* up, int orthogonalize,
                                     int keepZ)
{
    reinterpret_cast<SoultreeObject*>(this)->SetAxesPtr(reinterpret_cast<const Vec3*>(look),
                                                        reinterpret_cast<const Vec3*>(up),
                                                        orthogonalize, keepZ);
}

void RacerD3DIMSoultreeObjectView::GetPosition(Vector3* position)
{
    reinterpret_cast<SoultreeObject*>(this)->GetPosition(reinterpret_cast<Vec3*>(position));
}

void BikeRaceTerrainView::QueryGround(Vector3* point, Vector3* normal, int flatShaded,
                                           unsigned char* surface)
{
    reinterpret_cast<Terrain*>(this)->QueryGround(reinterpret_cast<TerrainVec3*>(point),
                                                  reinterpret_cast<TerrainVec3*>(normal),
                                                  flatShaded, surface);
}

// best guess: the KrustyVCR passed here is the recorder KrustyBike keeps as
// netRecorder (KbRecorder is KrustyBike.h's view of the same object).
void BikeRaceKrustyBikeView::SetNetRecorder(KrustyVCR* vcr)
{
    reinterpret_cast<KrustyBike*>(this)->SetNetRecorder(reinterpret_cast<KbRecorder*>(vcr));
}

int RacerD3DIMSoultreeObjectView::ReplaceMaterialTexture(int index, const char* name, int textures)
{
    return reinterpret_cast<D3DIMSoultreeObject*>(this)->ReplaceMaterialTexture(index, name, textures);
}

static inline Vec3 Vector3AsVec3(const Vector3& v)
{
    Vec3 result;
    result.x = v.x;
    result.y = v.y;
    result.z = v.z;
    return result;
}

// best guess: the two views pass the same 25 stack words to 0x0048fc80; each view
// argument goes to the canonical parameter in the same position.  The skill float is
// the word KrustyBike::Load keeps as `aiSkill` (+0x15d8), so its bits are passed unchanged.
GameObject* BikeRaceKrustyBikeView::Load(void* owner, const char* engineName, const char* bikeFile,
                                       const char* riderFile, LightManager* lights,
                                       TextureContext* textures, Vector3 position,
                                       Vector3 direction, Vector3 up,
                                       BikeRaceTerrainView* terrain,
                                       ControlInterface* control, char ai, int remote,
                                       BikeRace* race, const char* name, int b, int id,
                                       BikeEngineSetup* setup, int gasGyroOption, int brakeGyroOption,
                                       int particles, Scene* scene, KrustyVCR* vcr, float skill,
                                       int withRider)
{
    void* skillWord = *reinterpret_cast<void**>(&skill);
    return reinterpret_cast<KrustyBike*>(this)->Load(
        (int)owner, (int)engineName, bikeFile, reinterpret_cast<const SoultreeLoadDesc*>(riderFile),
        (int)lights, (int)textures, Vector3AsVec3(position), Vector3AsVec3(direction),
        Vector3AsVec3(up), (int)terrain, reinterpret_cast<VehicleInputMap*>(control),
        (unsigned char)ai, (unsigned char)remote, reinterpret_cast<KbRace*>(race), (int)name, b,
        id, reinterpret_cast<const KbBikeSetup*>(setup), gasGyroOption, brakeGyroOption, particles, (int)scene,
        reinterpret_cast<KbRecorder*>(vcr), skillWord, withRider);
}
