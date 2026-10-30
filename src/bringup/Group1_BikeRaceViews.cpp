// Group1_BikeRaceViews.cpp -- bring-up forwarders for BikeRace.h's provisional views
// whose methods BikeRace::AimCamera (Group1_BikeRaceCamera.cpp) calls.  Each view
// method is the retail function already reconstructed under its canonical class, so
// it forwards with the same `this` (retail calls the canonical address directly):
//   UnknownBikeRaceShadow::AddCaster    0x004dab00 = ProjectedShadow::AddCaster
//   UnknownBikeRaceShadow::ClearCasters 0x004dab90 = ProjectedShadow::ClearCasters
//   UnknownBikeRaceModel::SetLod        0x00444d00 = D3DIMSoultreeObject::SetLod
//   UnknownBikeRaceModel::SetLowestLod  0x00444d40 = D3DIMSoultreeObject::SetLowestLod
//   UnknownBikeRaceModel::ClearFlag14c  0x004fdb50 = SoultreeObject::ClearFlag14c
// No project header is included: the declarations below repeat only the members
// needed, with the same class keys, so every mangled name equals the canonical one.

class ShadowCaster;

class ProjectedShadow {
public:
    void AddCaster(ShadowCaster* caster);
    void ClearCasters();
};

class SoultreeObject {
public:
    void ClearFlag14c();
};

class D3DIMSoultreeObject {
public:
    void SetLod(int value);
    void SetLowestLod(int value);
};

class UnknownBikeRaceModel {
public:
    void SetLod(int value);
    void SetLowestLod(int value);
    void ClearFlag14c();
};

class UnknownBikeRaceShadow {
public:
    void AddCaster(UnknownBikeRaceModel* model);
    void ClearCasters();
};

void UnknownBikeRaceShadow::AddCaster(UnknownBikeRaceModel* model)
{
    reinterpret_cast<ProjectedShadow*>(this)->AddCaster(reinterpret_cast<ShadowCaster*>(model));
}

void UnknownBikeRaceShadow::ClearCasters()
{
    reinterpret_cast<ProjectedShadow*>(this)->ClearCasters();
}

void UnknownBikeRaceModel::SetLod(int value)
{
    reinterpret_cast<D3DIMSoultreeObject*>(this)->SetLod(value);
}

void UnknownBikeRaceModel::SetLowestLod(int value)
{
    reinterpret_cast<D3DIMSoultreeObject*>(this)->SetLowestLod(value);
}

void UnknownBikeRaceModel::ClearFlag14c()
{
    reinterpret_cast<SoultreeObject*>(this)->ClearFlag14c();
}
