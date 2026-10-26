#pragma once

#include "GameObject.h"
#include "MatrixUtil.h"
#include "D3DIMSoulTree.h"  // D3DIMSoultreeObject (the "sphere.slt" model)

// LightEmitter.cpp (literal __FILE__ at 0x0056ddf0, xrefs 0x0049e276..
// 0x0049e4fc; code 0x0049dd30..0x004a01c4). RTTI: LightEmitter : GameObject
// (vtable 0x005550cc) and LightManager : GameObject (vtable 0x00555144).
// Field and method names are provisional.

// LightEmitter::linkedTarget (+0x9c); only these two helpers (both in this
// file) are known.
class UnknownLightEmitterTarget {
public:
    void SetRgbColor(unsigned int color);           // 0x0049de70
    void SetPosition(const Vector3* position);      // 0x0049de90

    unsigned char field_0x00[0xb8];
    Vector3 targetPosition;                                    // +0xb8 the emitter's position (SetPosition)
    unsigned char field_0xc4[0xd0 - 0xc4];
    unsigned int rgbColor;                                     // colour without alpha
};

class LightManager;

class LightEmitter : public GameObject {
public:
    explicit LightEmitter(int flags);               // 0x0049deb0
    virtual ~LightEmitter();                        // 0x0049dfc0 (deleting wrapper 0x0049dfa0)
    // Slot 12 is the shared `return 1` body 0x00467ae0.
    virtual int PrepareGeometry();

    // 0x0049e230 (ret 0x2c): slot 8 of the base, then the type, colour,
    // range, optional position/direction and sphere model; arguments 8-10
    // are unused.
    LightEmitter* Create(void* value, int type, unsigned int color,
                                        const Vector3* position, const Vector3* direction,
                                        float range, int sphere, int a8, int a9, int a10,
                                        int index);
    unsigned int PackedColor();                  // 0x0049dfd0: packed ARGB colour
    void SetColor(unsigned int color);        // 0x0049e020: sets the colours
    void SetRange(float range);                            // 0x0049e0f0
    void SetPosition(const Vector3* position);             // 0x0049e150
    void SetDirection(const Vector3* direction);           // 0x0049e1e0

    int lightType;                          // type (LightManager 0x004a0190 searches it)
    // +0x30..+0x97 has the D3DLIGHT7 layout (strong inference: 0x68 bytes, a
    // type 1..4 picked from the emitter type, default 3 = directional; SetColor
    // copies the diffuse colour into the next two; the constructor's 1, 1, 0, 0
    // and pi / 4, pi / 2 are D3D's falloff, attenuation and spot-cone values).
    int d3dLightType;                       // +0x30 dltType, default 3
    float colorRGBA[4];                     // colour (r, g, b, a)
    float specularRGBA[4];                  // +0x44 dcvSpecular
    float ambientRGBA[4];                   // +0x54 dcvAmbient; SetColor copies the diffuse colour here too
    Vector3 lightPosition;                  // position
    Vector3 lightDirection;                 // direction
    float lightRange;                       // range, sqrt(FLT_MAX) when unbounded
    float falloff;                          // +0x80 dvFalloff
    float attenuation0;                     // +0x84 dvAttenuation0
    float attenuation1;                     // +0x88
    float attenuation2;                     // +0x8c
    float spotInnerAngle;                   // +0x90 dvTheta, pi / 4
    float spotOuterAngle;                   // +0x94 dvPhi, pi / 2
    D3DIMSoultreeObject* debugSphere;       // "sphere.slt" model
    UnknownLightEmitterTarget* linkedTarget; // +0x9c ctor 0; SetColor and SetPosition forward the colour and position to it
    int lightIndex;                         // +0xa0 Create's last argument, -1 by default
    LightManager* manager;                  // owner (set by 0x0049e470)
    unsigned int field_0xa8;                // compared, never written here
};

class LightManager : public GameObject {
public:
    explicit LightManager(int flags);               // 0x0049e390
    virtual ~LightManager();                        // 0x0049e400 (deleting wrapper 0x0049e3e0)
    // Slot 8 is the shared base-forwarding body 0x004452e0.
    virtual GameObject* SetRenderContext(void* value);

    void AddLight(LightEmitter* light);      // 0x0049e470: adds a light
    void MarkChanged();                         // 0x0049e490: counts a change
    LightEmitter* FindLightOfType(int type);        // 0x004a0190: first light of `type`
    // 0x0049e4a0 lights `count` vertices: it grows the per-vertex
    // red/green/blue arrays, picks a worker by light count and type, then
    // scales the optional colour array by the results (Griddraw.h's
    // GridVertexSink is this function seen from the terrain). Not
    // reconstructed: it and its
    // three workers 0x0049e6b0, 0x0049f1c0 and 0x0049f8d0 (ebp frames) round
    // with an inlined `fld [tmp]; mov eax, [p]; fistp [eax]` helper that VC6
    // only emits for inline assembly.
    void LightVertices(void* matrix, int count, void* vertices, void* buffer,
                       int stride, void* colors);

    int lightCount;                         // light count
    LightEmitter* lights[50];               // +0x30
    int* vertexRed;                         // per-vertex red (one allocation of 3 * capacity)
    int* vertexGreen;                       // green
    int* vertexBlue;                        // blue
    int field_0x104;
    int field_0x108;
    int field_0x10c;
    int vertexCapacity;                     // capacity in vertices
    int field_0x114;
    int changeCount;                        // change counter
};

// 0x0049f180 (cdecl): out-of-line negation, also called from the terrain.
Vector3 operator-(const Vector3& v);
