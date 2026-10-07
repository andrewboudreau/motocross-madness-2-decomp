// Rectangle2D's out-of-line methods, 0x004e8ad0..0x004e8c4b, between
// recorder.cpp (last xref 0x004e7ceb) and ResourceManager.cpp (first xref
// 0x004e8de6); RenderTarget's code follows at 0x004e8c50. The class name
// comes from RTTI (vtable 0x005577fc); the TU name is unattested.
// FontTexture.cpp and TextService.cpp call them.

#include "Rectangle2D.h"

// 0x004e8ad0
Rectangle2D::Rectangle2D(float x1, float y1, float x2, float y2) : field_0x04(x1, y1), field_0x10(x2, y2)
{
}

// 0x004e8b60
Rectangle2D::Rectangle2D(const Rectangle2D& other) : field_0x04(other.field_0x04), field_0x10(other.field_0x10)
{
}

// 0x004e8bc0
Rectangle2D::~Rectangle2D()
{
}

// 0x004e8c20
Rectangle2D& Rectangle2D::operator=(const Rectangle2D& other)
{
    if (this != &other) {
        field_0x04 = other.field_0x04;
        field_0x10 = other.field_0x10;
    }
    return *this;
}
