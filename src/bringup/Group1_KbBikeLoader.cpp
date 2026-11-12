// Group1_KbBikeLoader.cpp -- bring-up body for KrustyBikeTypes.h's KbBikeLoader view.
//
// KbBikeLoader::LoadBike is retail 0x004079c0 (thiscall, `ret 0xa0` = 40 argument
// dwords), Bike's own loader, reconstructed as Bike::LoadBike in
// src/krusty2/vehicle/Bike.cpp (BikeCore.h).  KrustyBike's loader (0x0048fc80) reaches
// it through this view with its own `this` (Bike is KrustyBike's primary base at
// offset 0, so no adjustment).  The view's 40 dwords line up one for one with
// LoadBike's parameters and carry the same names (wave 2, docs/NAMING.md; a13 is LoadBike's
// unused a19), so every argument is forwarded
// unchanged in position; only the provisional types differ.
//
// KrustyBikeTypes.h is not included (it redeclares types BikeCore.h also brings in); the
// view is redeclared with the same key and signature, so the mangled name stays
// ?LoadBike@KbBikeLoader@@QAEPAVGameObject@@HHPBDPBUSoultreeLoadDesc@@HHUVec3@@22HPAUVehicleInputMap@@HHHPAXHHHMMMMMMHHHH444HMH@Z.

#include "vehicle/Bike.h"

struct KbBikeLoader {
    GameObject* LoadBike(int context, int engineName, const char* modelPath, const SoultreeLoadDesc* riderModel, int desc, int info,
                            Vec3 startPosition, Vec3 forward, Vec3 up, int terrainProbe, VehicleInputMap* map, int name, int a13,
                            int defaultEngine, void* torqueTable, int rpmLow, int rpmHigh, int rpmStep, float frontSpring, float frontDamper,
                            float rearSpring, float rearDamper, float frontRatio, float rearRatio, int gasGyroOption, int brakeGyroOption, int linkedObject, int device,
                            void* steer, void* lean, void* throttle, int trackObject, float weight, int createCollision);
};

// 0x004079c0
GameObject* KbBikeLoader::LoadBike(int context, int engineName, const char* modelPath, const SoultreeLoadDesc* riderModel,
                                      int desc, int info, Vec3 startPosition, Vec3 forward, Vec3 up,
                                      int terrainProbe, VehicleInputMap* map, int name, int a13, int defaultEngine,
                                      void* torqueTable, int rpmLow, int rpmHigh, int rpmStep, float frontSpring, float frontDamper,
                                      float rearSpring, float rearDamper, float frontRatio, float rearRatio, int gasGyroOption, int brakeGyroOption,
                                      int linkedObject, int device, void* steer, void* lean, void* throttle,
                                      int trackObject, float weight, int createCollision)
{
    return reinterpret_cast<Bike*>(this)->LoadBike(
        context, (const char*)engineName, modelPath, (const char*)riderModel, (const SoultreeLoadDesc*)desc,
        (const BikeA5C8*)info, startPosition, forward, up, (void*)terrainProbe, map,
        (const char*)name, a13, defaultEngine, (int*)torqueTable, rpmLow, rpmHigh, rpmStep, frontSpring, frontDamper, rearSpring, rearDamper, frontRatio,
        rearRatio, gasGyroOption, brakeGyroOption, (void*)linkedObject, (void*)device, (VehicleAxis*)steer,
        (VehicleAxis*)lean, (VehicleAxis*)throttle, (SoultreeSlot1f0*)trackObject, weight,
        createCollision);
}
