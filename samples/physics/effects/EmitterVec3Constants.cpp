// EmitterVec3Constants.cpp -- the four TU-private constant vectors at 0x004b9e00..0x004b9f3f.
//
// Ownership is uncertain: they are the "vector constants" initialisers that open about 73 retail
// translation units, so they mark a TU start between SparkParticleEmitter (ends 0x004b9dc0) and
// SteamParticleEmitter (starts 0x004b9f40), but that TU has no __FILE__ string and is not
// necessarily Particles.cpp.  Globals at 0x00689128 (0,0,0), 0x00689138 (1,0,0), 0x00689148 (0,1,0)
// and 0x00689118 (0,0,1).  Copy-initialisation gives retail's stack temporary plus copy.
struct EmitterConstVec3 {
    float x, y, z;
    EmitterConstVec3(float x_, float y_, float z_) { x = x_; y = y_; z = z_; }
};
static const EmitterConstVec3 kVec3Zero = EmitterConstVec3(0.0f, 0.0f, 0.0f);
static const EmitterConstVec3 kVec3XAxis = EmitterConstVec3(1.0f, 0.0f, 0.0f);
static const EmitterConstVec3 kVec3YAxis = EmitterConstVec3(0.0f, 1.0f, 0.0f);
static const EmitterConstVec3 kVec3ZAxis = EmitterConstVec3(0.0f, 0.0f, 1.0f);
