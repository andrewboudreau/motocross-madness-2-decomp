// Point2D's out-of-line methods, 0x004d28b0..0x004d2936, between
// Pixtrans.cpp and the gearbox unit (GearRatios.cpp). The class name comes
// from RTTI; the TU name is unattested. Rectangle2D (0x004e8ad0..) calls
// them.

#include "Rectangle2D.h"

// 0x004d28b0
Point2D::Point2D(float x, float y) : x(x), y(y)
{
}

// 0x004d28f0
Point2D::Point2D(const Point2D& other) : x(other.x), y(other.y)
{
}

// 0x004d2910
Point2D::~Point2D()
{
}

// 0x004d2920
Point2D& Point2D::operator=(const Point2D& other)
{
    if (this != &other) {
        x = other.x;
        y = other.y;
    }
    return *this;
}
