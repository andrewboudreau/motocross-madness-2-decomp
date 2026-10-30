// Group0_SoultreeBridge.h -- calls into the defined Vec3 forms of two
// SoultreeObject helpers for translation units whose headers only declare
// Vector3 overloads (which nobody defines). The node and frame pointers are
// the SoultreeObject (primary, +0) addresses.
#pragma once

// 0x004fc970: *out = the node's local translation.
void Group0_SoultreeGetPosition(void* node, float* out);
// 0x004fc690: SoultreeObject::TranslateIn(frame, delta).
void Group0_SoultreeTranslateIn(void* node, void* frame, const float* delta);
// 0x004fb8c0: SoultreeObject::SetMatrixIn(frame, m), m a 4x4 float matrix.
void Group0_SoultreeSetMatrixIn(void* node, void* frame, void* matrix);
// 0x004fc050: SoultreeObject::SetAxesIn(frame, axisZ, axisY, orthogonalize, keepZ).
void Group0_SoultreeSetAxesIn(void* node, void* frame, const float* axisZ, const float* axisY,
                              int orthogonalize, int keepZ);
// 0x004fc740: SoultreeObject::SetPositionIn(frame, p).
void Group0_SoultreeSetPositionIn(void* node, void* frame, const float* p);
// 0x004fdab0: SoultreeObject::MarkSubtreeDirty().
void Group0_SoultreeMarkSubtreeDirty(void* node);
