#pragma once

// Shared value type: retail helpers read x/y/z at +0/+4/+8 and return its
// 12 bytes through a hidden result pointer. The name remains provisional.
// Math3D.h supplies the free operators and per-unit constants separately.
struct Vec3 {
    float x, y, z;

    Vec3() {}
    Vec3(float x_, float y_, float z_) { x = x_; y = y_; z = z_; }
    // d3dvec.inl's single-value constructor; the chained assignment stores z, y, x
    // (Vehicle slot 7's single-wheel branch).
    Vec3(float f) { x = y = z = f; }

    float& operator[](int i) { return (&x)[i]; }
    const float& operator[](int i) const { return (&x)[i]; }

    Vec3& operator+=(const Vec3& v) { x += v.x; y += v.y; z += v.z; return *this; }
    Vec3& operator-=(const Vec3& v) { x -= v.x; y -= v.y; z -= v.z; return *this; }
    Vec3& operator*=(float s) { x *= s; y *= s; z *= s; return *this; }
    Vec3& operator/=(float s) { x /= s; y /= s; z /= s; return *this; }
};
