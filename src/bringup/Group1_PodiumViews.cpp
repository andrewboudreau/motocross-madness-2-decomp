// Group1_PodiumViews.cpp -- bring-up forwarders for the provisional views that
// EventManager::CreatePodiumScene (Group1_PodiumScene.cpp) calls.  Every view method
// is a retail function already reconstructed under its canonical class (addresses
// from the canonical *.bindings.json), so each forwards with the same `this`:
//   UnknownKrustyBikeView::NextBike          0x004204e0 = BikeRace::NextBike
//   UnknownPodiumTrackObject::QueryGround    0x00507c10 = Terrain::QueryGround
//   UnknownPodiumNode::GetPosition           0x004fc970 = SoultreeObject::GetPosition
//   UnknownPodiumNode::GetAxes               0x004fc4f0 = SoultreeObject::GetAxes
//   UnknownPodiumArcade::SetPosition         0x004014f0 = ArcadeObject::SetPosition
//   UnknownPodiumArcade::SetAxes             0x00401520 = ArcadeObject::SetAxes
//   UnknownPodiumSoultree::SetUseLighting    0x004444c0 = D3DIMSoultreeObject::SetUseLighting
//   UnknownPodiumSoultree::SetPosition       0x004fc660 = SoultreeObject::SetPosition
//   UnknownPodiumSoultree::SetAxesPtr        0x004fbd70 = SoultreeObject::SetAxesPtr
//   UnknownPodiumSoultree::ReplaceMaterialTexture 0x00444c70 = D3DIMSoultreeObject::ReplaceMaterialTexture
//   UnknownPodiumSound::UnknownPodiumSound   0x004bba10 = Sound::Sound
//   UnknownPodiumSound::Load                 0x004bc320 = Sound::Load
//   UnknownPodiumSound::PlayWithOptions      0x004bc6b0 = Sound::PlayWithOptions
// No project header is included: the declarations below repeat only the members
// needed, with the same class keys, so every mangled name equals the canonical one.
// Vec3/TerrainVec3/Vector3 are all three packed floats.

#include <new.h>

struct Vec3;
struct Vector3;
struct TerrainVec3;
struct UnknownEventRacer;
class UnknownBikeRaceRacer;
class UnknownTextureStream;
class GameObject;
class SoundGroup;

class BikeRace {
public:
    UnknownBikeRaceRacer* NextBike(int* iterator);
};

class Terrain {
public:
    void QueryGround(TerrainVec3* pos, TerrainVec3* normal, int flatShaded, unsigned char* surface);
};

class SoultreeObject {
public:
    Vec3* GetPosition(Vec3* out);
    void GetAxes(Vec3* direction, Vec3* up);
    void SetPosition(const Vec3& position);
    void SetAxesPtr(const Vec3* look, const Vec3* up, int orthogonalize, int keepZ);
};

class D3DIMSoultreeObject {
public:
    void SetUseLighting(int value);
    int ReplaceMaterialTexture(int index, const char* name, int context);
};

class ArcadeObject {
public:
    void SetPosition(const Vector3* position);
    void SetAxes(const Vector3* a, const Vector3* b, int c, int d);
};

class Sound {
public:
    Sound(SoundGroup* group, int type);
    int Load(const char* name, UnknownTextureStream* stream, int flags, int a, int b, int c);
    int PlayWithOptions(int restart, unsigned long playFlags, int preferHardware);
};

// ---- the views (same keys and signatures as RaceView.h / EventManager.h / the sample)

struct UnknownKrustyBikeView {
    UnknownEventRacer* NextBike(int* iterator);
};

struct UnknownPodiumTrackObject {
    void QueryGround(Vector3* point, int a, int b, int c);
};

struct UnknownPodiumNode {
    void GetPosition(Vector3* position);
    void GetAxes(Vector3* direction, Vector3* up);
};

class UnknownPodiumArcade {
public:
    void SetPosition(const Vector3* position);
    void SetAxes(const Vector3* a, const Vector3* b, int c, int d);
};

struct UnknownPodiumSoultree {
    void SetUseLighting(int value);
    void SetPosition(const Vector3* position);
    void SetAxesPtr(const Vector3* look, const Vector3* up, int orthogonalize, int keepZ);
    void ReplaceMaterialTexture(int index, const char* name, void* context);
};

class UnknownPodiumSound {
public:
    UnknownPodiumSound(GameObject* group, int type);
    int Load(const char* name, UnknownTextureStream* stream, int flags, int a, int b, int c);
    int PlayWithOptions(int restart, unsigned long playFlags, int preferHardware);
};

UnknownEventRacer* UnknownKrustyBikeView::NextBike(int* iterator)
{
    return reinterpret_cast<UnknownEventRacer*>(reinterpret_cast<BikeRace*>(this)->NextBike(iterator));
}

// The view's three ints are Terrain::QueryGround's normal out, flat-shaded flag and
// surface out (the podium passes 0 for all of them).
void UnknownPodiumTrackObject::QueryGround(Vector3* point, int a, int b, int c)
{
    reinterpret_cast<Terrain*>(this)->QueryGround(reinterpret_cast<TerrainVec3*>(point),
                                                  reinterpret_cast<TerrainVec3*>(a), b,
                                                  reinterpret_cast<unsigned char*>(c));
}

void UnknownPodiumNode::GetPosition(Vector3* position)
{
    reinterpret_cast<SoultreeObject*>(this)->GetPosition(reinterpret_cast<Vec3*>(position));
}

void UnknownPodiumNode::GetAxes(Vector3* direction, Vector3* up)
{
    reinterpret_cast<SoultreeObject*>(this)->GetAxes(reinterpret_cast<Vec3*>(direction),
                                                     reinterpret_cast<Vec3*>(up));
}

void UnknownPodiumArcade::SetPosition(const Vector3* position)
{
    reinterpret_cast<ArcadeObject*>(this)->SetPosition(position);
}

void UnknownPodiumArcade::SetAxes(const Vector3* a, const Vector3* b, int c, int d)
{
    reinterpret_cast<ArcadeObject*>(this)->SetAxes(a, b, c, d);
}

void UnknownPodiumSoultree::SetUseLighting(int value)
{
    reinterpret_cast<D3DIMSoultreeObject*>(this)->SetUseLighting(value);
}

void UnknownPodiumSoultree::SetPosition(const Vector3* position)
{
    reinterpret_cast<SoultreeObject*>(this)->SetPosition(*reinterpret_cast<const Vec3*>(position));
}

void UnknownPodiumSoultree::SetAxesPtr(const Vector3* look, const Vector3* up, int orthogonalize,
                                       int keepZ)
{
    reinterpret_cast<SoultreeObject*>(this)->SetAxesPtr(reinterpret_cast<const Vec3*>(look),
                                                        reinterpret_cast<const Vec3*>(up),
                                                        orthogonalize, keepZ);
}

void UnknownPodiumSoultree::ReplaceMaterialTexture(int index, const char* name, void* context)
{
    reinterpret_cast<D3DIMSoultreeObject*>(this)->ReplaceMaterialTexture(index, name, (int)context);
}

// 0x004bba10: the podium allocates 0x1f8 bytes and constructs a PCAudio Sound in place.
UnknownPodiumSound::UnknownPodiumSound(GameObject* group, int type)
{
    new (this) Sound(reinterpret_cast<SoundGroup*>(group), type);
}

int UnknownPodiumSound::Load(const char* name, UnknownTextureStream* stream, int flags, int a,
                             int b, int c)
{
    return reinterpret_cast<Sound*>(this)->Load(name, stream, flags, a, b, c);
}

int UnknownPodiumSound::PlayWithOptions(int restart, unsigned long playFlags, int preferHardware)
{
    return reinterpret_cast<Sound*>(this)->PlayWithOptions(restart, playFlags, preferHardware);
}
