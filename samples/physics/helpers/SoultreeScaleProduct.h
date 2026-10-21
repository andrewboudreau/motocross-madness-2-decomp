// Shared readable diagonal-matrix product used by the two reconstructed scale views.
// Preserve the complete thirteen-store operation; translation remains unchanged.
#pragma once
#include "../../../src/reconstructed/Matrix4.h"

static inline void ScaleProduct(Matrix4* out, Matrix4 m, Matrix4 s)
{
    out->_11 = s._11 * m._11 + s._13 * m._31 + s._12 * m._21 + s._14 * m._41;
    out->_12 = s._11 * m._12 + s._13 * m._32 + s._12 * m._22 + s._14 * m._42;
    out->_13 = s._11 * m._13 + s._13 * m._33 + s._12 * m._23 + s._14 * m._43;
    out->_14 = s._13 * m._34 + s._11 * m._14 + s._12 * m._24 + s._14 * m._44;
    out->_21 = s._21 * m._11 + s._22 * m._21 + s._23 * m._31 + s._24 * m._41;
    out->_22 = s._21 * m._12 + s._22 * m._22 + s._23 * m._32 + s._24 * m._42;
    out->_23 = s._21 * m._13 + s._22 * m._23 + s._23 * m._33 + s._24 * m._43;
    out->_24 = s._21 * m._14 + s._22 * m._24 + s._23 * m._34 + s._24 * m._44;
    out->_31 = s._31 * m._11 + s._32 * m._21 + s._33 * m._31 + s._34 * m._41;
    out->_32 = s._31 * m._12 + s._32 * m._22 + s._33 * m._32 + s._34 * m._42;
    out->_33 = s._31 * m._13 + s._32 * m._23 + s._33 * m._33 + s._34 * m._43;
    out->_34 = s._31 * m._14 + s._32 * m._24 + s._33 * m._34 + s._34 * m._44;
    out->_44 = s._41 * m._14 + s._42 * m._24 + s._43 * m._34 + s._44 * m._44;
}
