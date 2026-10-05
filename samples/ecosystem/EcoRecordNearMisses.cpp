// Near-miss EcoSystem record candidate, kept out of src/reconstructed until
// it matches. See docs/ECOSYSTEM.md. Types and names are provisional: only
// the offsets below are observed.
//
// UnknownEcoRecord::UnknownFunction456890 (0x00456890, 369 bytes): the
// distance fade that EcoSystem slot 12 calls before 0x00456a10. 368 of 369
// bytes match: retail loads the frame pointer ([+0x18]+8) before the first
// fmul, VC6 here after it. Frame, view, EcoSystem and scale locals,
// declaration order, operand order, casts, a coordinate array and a
// position struct do not move it. Nesting the +0x1fc test and indexing the
// band table without a pointer local are what line the rest up.

// cdecl 0x00460b50 (FastSqrt in src/krusty2/math/FastMath.h).
float FastSqrt(float value);
struct UnknownEcoFrame {
    unsigned char field_0x000[0x170];
    float field_0x170;
    float field_0x174;
    float field_0x178;
};
struct UnknownEcoView {
    unsigned char field_0x00[8];
    UnknownEcoFrame* field_0x08;
};
struct UnknownEcoDefinition {
    unsigned char field_0x000[0x1fc];
    int field_0x1fc;
};
// The EcoSystem instance at 0x0059aebc.
class EcoSystem {
public:
    unsigned char field_0x000[0x18];
    UnknownEcoView* field_0x18;
    unsigned char field_0x01c[0x58 - 0x1c];
    UnknownEcoDefinition* field_0x58[256];
    unsigned char field_0x458[0x5a8 - 0x458];
    float field_0x5a8;
    unsigned char field_0x5ac[0x5c0 - 0x5ac];
    int field_0x5c0;
};
// 0x20-byte distance bands at 0x0059af14, indexed by EcoSystem+0x5c0.
struct UnknownEcoBand {
    int field_0x00;
    int field_0x04;
    unsigned char field_0x08[0x18];
};
extern EcoSystem* g_UnknownGlobal59aebc;
extern UnknownEcoBand* g_UnknownGlobal59af14;
class UnknownEcoRecord {
public:
    void UnknownFunction456890(int* culled, int* level);
    unsigned char field_0x00[8];
    unsigned char field_0x08;
    unsigned char field_0x09[3];
    unsigned short field_0x0c;
    unsigned short field_0x0e;
    unsigned short field_0x10;
    unsigned char field_0x12;
    unsigned char field_0x13;
};

// 0x00456890: sets *culled and *level = 0xff when the record lies beyond the
// current band's outer distance (per axis, then radially); otherwise *level
// is 0 inside the inner distance or without definition +0x1fc, else the
// position between inner and outer scaled to 0..255.
void UnknownEcoRecord::UnknownFunction456890(int* culled, int* level) {
    UnknownEcoDefinition* definition = g_UnknownGlobal59aebc->field_0x58[field_0x12];
    UnknownEcoFrame* frame = g_UnknownGlobal59aebc->field_0x18->field_0x08;
    float dx = frame->field_0x170 - field_0x0c * g_UnknownGlobal59aebc->field_0x5a8;
    float dz = frame->field_0x178 - field_0x10 * g_UnknownGlobal59aebc->field_0x5a8;
    *culled = 0;
    float outer = (float)g_UnknownGlobal59af14[g_UnknownGlobal59aebc->field_0x5c0].field_0x00;
    float inner = (float)g_UnknownGlobal59af14[g_UnknownGlobal59aebc->field_0x5c0].field_0x04;
    if (dx > outer || dz > outer) {
        *culled = 1;
        *level = 0xff;
        return;
    }
    float distance = FastSqrt(dz * dz + dx * dx);
    if (distance > outer) {
        *culled = 1;
        *level = 0xff;
        return;
    }
    if (definition->field_0x1fc) {
        if (distance < inner)
            *level = 0;
        else
            *level = (int)((distance - inner) * 255.0f / (outer - inner));
    } else {
        *level = 0;
    }
}
