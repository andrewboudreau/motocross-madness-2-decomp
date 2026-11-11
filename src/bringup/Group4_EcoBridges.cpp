// Bring-up data (best-guess-unreviewed) for src/bringup/Group4_EcoSystem.cpp.
//
// The EcoSystem near-miss bodies weigh slope and altitude through
// g_SharedNormalDistribution (src/reconstructed/EcoSystem.h). The forwarders this
// unit used to carry for the CollisionObject and terrain calls were replaced by
// link_aliases (docs/GAME_BUILD.md#near-miss-bodies-replacing-bring-up-copies).

// 0x0056ece0: the shared bell-curve table pointer EcoSystem::GenerateObjects
// weighs slope and altitude with. In retail it points at NormalDistribution.cpp's
// static object (0x006886e8, resolution 100). That object is file-static in
// src/krusty2/effects/NormalDistribution.cpp, so
// best guess: point at an identical private instance built by the same
// out-of-line constructor (the table is a pure function of the resolution).
class NormalDistribution {
public:
    explicit NormalDistribution(int resolution);  // NormalDistribution.cpp
    ~NormalDistribution();
    float Lookup(float mean, float value, float sigma); // 0x004b0070
    float* table;
    int count;
};
static NormalDistribution s_group4NormalDistribution(100);
NormalDistribution* g_SharedNormalDistribution = &s_group4NormalDistribution;
