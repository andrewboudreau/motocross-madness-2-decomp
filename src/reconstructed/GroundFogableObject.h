#ifndef MCM2_GROUND_FOGABLE_OBJECT_H
#define MCM2_GROUND_FOGABLE_OBJECT_H

// Terrain and EcoSystem's RTTI identify this nonpolymorphic second base at
// +0x2c. Constructor 0x004aae20 stores one zero dword; the field's role is
// still provisional.
class GroundFogableObject {
public:
    GroundFogableObject();
    int field_0x00;
};

#endif
