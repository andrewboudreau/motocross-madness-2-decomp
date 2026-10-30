// Group6_BikeRaceViews.cpp -- bring-up forwarders for the provisional views that
// BikeRace::Create (Group6_BikeRaceCreate.cpp) calls.  Each view method is a retail
// function already reconstructed under its canonical class (addresses from the
// *.bindings.json files), so it forwards with the same `this`:
//   UnknownBikeRaceMesh::SetUseLighting    0x004444c0 = D3DIMSoultreeObject::SetUseLighting
//   UnknownBikeRaceMesh::SetAxesPtr        0x004fbd70 = SoultreeObject::SetAxesPtr
//   UnknownBikeRaceMesh::SetPositionVec3   0x004fc630 = D3DIMSoultreeObject::SetPositionVec3
//   UnknownBikeRaceModel::GetPosition      0x004fc970 = SoultreeObject::GetPosition
//   UnknownBikeRaceProjector::QueryGround  0x00507c10 = Terrain::QueryGround
//   UnknownBikeRaceRacer::UnknownFunction496e20 0x00496e20 = KrustyBike::Fn_00496E20
//     (the racer pointer is KrustyBike's primary pointer; docs/BIKE_RACE_STATE_CONNECTIONS.md)
//   UnknownBikeRaceDemoDlg::Tick           0x00455b20 = DemoDlg::Tick
//   UnknownBikeRaceDemoDlg::OnControlPress 0x00455ae0 = DemoDlg::OnControlPress
//     (Group6_BikeRaceCreate.cpp's local stand-in for DlgProcs.h's DemoDlg, which
//     adds no data members to UIDialog; the canonical bodies are called with a
//     qualified, non-virtual call so the stand-in's vtable is not re-entered)
// No project header is included: the declarations below repeat only the members
// needed, with the same class keys, so every mangled name equals the canonical one.
// None of these classes gets a constructor here, so no vtable is emitted.
// Vec3/TerrainVec3/Vector3 are all three packed floats.

struct Vec3;
struct Vector3 {
    float x, y, z;
};
struct TerrainVec3;
struct KbRecorder;
class KrustyVCR;
struct UnknownControlEvent;
struct UnknownInputEntry;

class SoultreeObject {
public:
    Vec3* GetPosition(Vec3* out);
    void SetAxesPtr(const Vec3* look, const Vec3* up, int orthogonalize, int keepZ);
};

class D3DIMSoultreeObject {
public:
    void SetUseLighting(int value);
    void SetPositionVec3(Vector3 position);
};

class Terrain {
public:
    void QueryGround(TerrainVec3* pos, TerrainVec3* normal, int flatShaded, unsigned char* surface);
};

class KrustyBike {
public:
    void Fn_00496E20(KbRecorder* recorder);
};

class DemoDlg {
public:
    virtual int Tick(float frameTime);
    virtual int OnControlPress(UnknownControlEvent* event, UnknownInputEntry* entry);
};

// ---- the views (same keys and signatures as BikeRace.h / Group6_BikeRaceCreate.cpp)

struct UnknownBikeRaceMesh {
    void SetUseLighting(int value);
    void SetPositionVec3(Vector3 position);
    void SetAxesPtr(const Vector3* look, const Vector3* up, int orthogonalize, int keepZ);
};

class UnknownBikeRaceModel {
public:
    void GetPosition(Vector3* position);
};

class UnknownBikeRaceProjector {
public:
    void QueryGround(Vector3* point, Vector3* normal, int flatShaded, unsigned char* surface);
};

class UnknownBikeRaceRacer {
public:
    void UnknownFunction496e20(KrustyVCR* vcr);
};

class UnknownBikeRaceDemoDlg {
public:
    virtual int Tick(float frameTime);
    virtual int OnControlPress(UnknownControlEvent* event, UnknownInputEntry* entry);
};

void UnknownBikeRaceMesh::SetUseLighting(int value)
{
    reinterpret_cast<D3DIMSoultreeObject*>(this)->SetUseLighting(value);
}

void UnknownBikeRaceMesh::SetPositionVec3(Vector3 position)
{
    reinterpret_cast<D3DIMSoultreeObject*>(this)->SetPositionVec3(position);
}

void UnknownBikeRaceMesh::SetAxesPtr(const Vector3* look, const Vector3* up, int orthogonalize,
                                     int keepZ)
{
    reinterpret_cast<SoultreeObject*>(this)->SetAxesPtr(reinterpret_cast<const Vec3*>(look),
                                                        reinterpret_cast<const Vec3*>(up),
                                                        orthogonalize, keepZ);
}

void UnknownBikeRaceModel::GetPosition(Vector3* position)
{
    reinterpret_cast<SoultreeObject*>(this)->GetPosition(reinterpret_cast<Vec3*>(position));
}

void UnknownBikeRaceProjector::QueryGround(Vector3* point, Vector3* normal, int flatShaded,
                                           unsigned char* surface)
{
    reinterpret_cast<Terrain*>(this)->QueryGround(reinterpret_cast<TerrainVec3*>(point),
                                                  reinterpret_cast<TerrainVec3*>(normal),
                                                  flatShaded, surface);
}

// best guess: the KrustyVCR passed here is the recorder KrustyBike keeps as
// netRecorder (KbRecorder is KrustyBike.h's view of the same object).
void UnknownBikeRaceRacer::UnknownFunction496e20(KrustyVCR* vcr)
{
    reinterpret_cast<KrustyBike*>(this)->Fn_00496E20(reinterpret_cast<KbRecorder*>(vcr));
}

int UnknownBikeRaceDemoDlg::Tick(float frameTime)
{
    return reinterpret_cast<DemoDlg*>(this)->DemoDlg::Tick(frameTime);
}

int UnknownBikeRaceDemoDlg::OnControlPress(UnknownControlEvent* event, UnknownInputEntry* entry)
{
    return reinterpret_cast<DemoDlg*>(this)->DemoDlg::OnControlPress(event, entry);
}
