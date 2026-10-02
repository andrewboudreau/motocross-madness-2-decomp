// SteeringControl.h -- layout of the SteeringControl helper (SteeringControl.cpp).
#ifndef KRUSTY2_MOTION_STEERINGCONTROL_H
#define KRUSTY2_MOTION_STEERINGCONTROL_H

// No RTTI and no vptr; the only __FILE__ reference is the constructor (line 0x17).
class SteeringControl {
public:
    SteeringControl(float scale, float t, const Vec3* axisZ, const Vec3* axisY);
    void Release();
    int SetAxisFromDirection(const Vec3* worldDir, SoultreeObject* frame);
    int SetAxisFromPoints(SoultreeObject* from, SoultreeObject* to, SoultreeObject* frame);
    void SetAngle(float a, SoultreeObject* frame);
    void AddAngle(float delta, SoultreeObject* frame);

    SoultreeObject* node; // +0x00 scene node allocated by the ctor (0x1a4 bytes, line 0x17)
    float angle;          // +0x04 zeroed by the ctor; set/accumulated by 0x00504e20/0x00504ec0, flushed to 0 below 1e-3
    float field_0x08;     // +0x08 ctor: scale * clamp01(t); not read in this TU
    Vec3 axis;            // +0x0c ctor copies a static zero vector; the axis functions store a local-space direction
};

#endif
