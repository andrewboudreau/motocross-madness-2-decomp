// Retail 0x006850b0: sixteen loader-zero bytes in writable .data's zero tail.
// The existing symbol spelling is provisional; it does not imply {1,0,0,0}.
// This POD agrees with the existing independent QuatUtil/Math3D declarations.
// Original translation-unit ownership is unknown. No dynamic initializer.
struct Quat {
    float w;
    float x;
    float y;
    float z;
};

Quat g_QuatIdentity;
