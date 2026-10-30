// Group1_KbBikeLoader.cpp -- bring-up body for KrustyBikeTypes.h's KbBikeLoader view.
//
// KbBikeLoader::Fn_004079C0 is retail 0x004079c0 (thiscall, `ret 0xa0` = 40 argument
// dwords), Bike's own loader, reconstructed as Bike::LoadBike in
// src/krusty2/vehicle/Bike.cpp (BikeCore.h).  KrustyBike's loader (0x0048fc80) reaches
// it through this view with its own `this` (Bike is KrustyBike's primary base at
// offset 0, so no adjustment).  The view's 40 dwords line up one for one with
// LoadBike's parameters (KrustyBikeCore.h notes the shifted names: the view's desc sits in
// LoadBike's riderModel slot and a5 in its desc slot), so every argument is forwarded
// unchanged in position; only the provisional types differ.
//
// KrustyBikeTypes.h is not included (it redeclares types BikeCore.h also brings in); the
// view is redeclared with the same key and signature, so the mangled name stays
// ?Fn_004079C0@KbBikeLoader@@QAEPAVGameObject@@HHPBDPBUSoultreeLoadDesc@@HHUVec3@@22HPAUVehicleInputMap@@HHHPAXHHHMMMMMMHHHH444HMH@Z.

#include "vehicle/Bike.h"

struct KbBikeLoader {
    GameObject* Fn_004079C0(int context, int a2, const char* modelPath, const SoultreeLoadDesc* desc, int a5, int a6,
                            Vec3 startPosition, Vec3 forward, Vec3 up, int terrainProbe, VehicleInputMap* map, int a12, int a13,
                            int noSetup, void* setupName, int s50, int s54, int s58, float s0c, float s10,
                            float s14, float s18, float s1c, float s20, int gasGyro, int brakeGyro, int linkedObject, int device,
                            void* steer, void* lean, void* throttle, int trackObject, float weight, int createCollision);
};

// 0x004079c0
GameObject* KbBikeLoader::Fn_004079C0(int context, int a2, const char* modelPath, const SoultreeLoadDesc* desc,
                                      int a5, int a6, Vec3 startPosition, Vec3 forward, Vec3 up,
                                      int terrainProbe, VehicleInputMap* map, int a12, int a13, int noSetup,
                                      void* setupName, int s50, int s54, int s58, float s0c, float s10,
                                      float s14, float s18, float s1c, float s20, int gasGyro, int brakeGyro,
                                      int linkedObject, int device, void* steer, void* lean, void* throttle,
                                      int trackObject, float weight, int createCollision)
{
    return reinterpret_cast<Bike*>(this)->LoadBike(
        context, (const char*)a2, modelPath, (const char*)desc, (const SoultreeLoadDesc*)a5,
        (const BikeA5C8*)a6, startPosition, forward, up, (void*)terrainProbe, map,
        (const char*)a12, a13, noSetup, (int*)setupName, s50, s54, s58, s0c, s10, s14, s18, s1c,
        s20, gasGyro, brakeGyro, (void*)linkedObject, (void*)device, (VehicleAxis*)steer,
        (VehicleAxis*)lean, (VehicleAxis*)throttle, (SoultreeSlot1f0*)trackObject, weight,
        createCollision);
}
