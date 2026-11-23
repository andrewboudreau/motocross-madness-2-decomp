// Group6_BikeRaceViews.cpp -- bring-up forwarder for the one provisional view
// method BikeRace::CreateRacers (src/reconstructed/BikeRace.cpp) still calls:
//   BikeRaceKrustyBikeView::Load   0x0048fc80 = KrustyBike::Load (ret 0x7c both)
// The racer pointer is KrustyBike's primary pointer
// (docs/BIKE_RACE_STATE_CONNECTIONS.md). The other view methods were replaced
// by direct calls to D3DIMSoultreeObject, Terrain, ProjectedShadow and
// KrustyBike in BikeRace.cpp.
// No project header is included: the declarations below repeat only the members
// needed, with the same class keys, so every mangled name equals the canonical one.
// None of these classes gets a constructor here, so no vtable is emitted.
// Vec3/Vector3 are both three packed floats.

struct Vec3 {
    float x, y, z;
};
struct Vector3 {
    float x, y, z;
};
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
class Terrain;

class KrustyBike {
public:
    GameObject* Load(int context, int engineName, const char* modelPath, const SoultreeLoadDesc* riderModel, int desc,
                     int info, Vec3 startPosition, Vec3 forward, Vec3 up, int terrainProbe,
                     VehicleInputMap* map, unsigned char kind, unsigned char remote, KbRace* race,
                     int riderName, int a16, int netId, const KbBikeSetup* setup, int gasGyroOption,
                     int brakeGyroOption, int linkedObject, int trackObject, KbRecorder* recorder,
                     void* aiSkill, int withRider);
};

// ---- the view (same key and signature as BikeRace.h)

class BikeRaceKrustyBikeView {
public:
    GameObject* Load(void* owner, const char* engineName, const char* bikeFile, const char* riderFile,
                     LightManager* lights, TextureContext* textures, Vector3 position,
                     Vector3 direction, Vector3 up, Terrain* terrain,
                     ControlInterface* control, char ai, int remote, BikeRace* race,
                     const char* name, int b, int id, BikeEngineSetup* setup, int gasGyroOption, int brakeGyroOption,
                     int particles, Scene* scene, KrustyVCR* vcr, float skill, int withRider);
};

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
                                       Terrain* terrain,
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
