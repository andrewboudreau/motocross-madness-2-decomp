
// Out-of-line Vec3 helpers inside the Bike.cpp link-order bracket (0x0040ae00/0x0040ae30).
// They are not Bike methods and reference no __FILE__, so their translation unit is
// unproven: they are probably out-of-line copies of shared inline vector operators.

// Out-of-line vector helpers (retail 0x40ae00 / 0x40ae30). The scale helper is
// a thiscall member of a three-float vector; the dot product is a cdecl free
// function taking two pointers.
struct BikeVec3Ops
{
    float x, y, z;
    BikeVec3Ops& ScaleBy(float factor);
};

BikeVec3Ops& BikeVec3Ops::ScaleBy(float factor)
{
    x *= factor;
    y *= factor;
    z *= factor;
    return *this;
}

float BikeDotProduct(const BikeVec3Ops* lhs, const BikeVec3Ops* rhs)
{
    float sum = lhs->y * rhs->y + lhs->x * rhs->x;
    sum += lhs->z * rhs->z;
    return sum;
}
