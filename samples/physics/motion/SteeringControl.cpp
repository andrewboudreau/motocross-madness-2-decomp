// Near miss for src/krusty2/motion/SteeringControl.cpp (TU SteeringControl.cpp).
// SteeringControl::SetAxisFromPoints (0x00504d30, 145/234 strict): retail spills y to
// a stack slot and keeps d.x on the x87 stack (`fld st(0); fmul st(3)`), popping it
// at the end; the flow and calls match. Tried without gain: d * inv, d * (1 / len),
// inv * d.x per component (all 64 operand-order combinations), d *= inv, d / len, a
// component-wise d, other lenSq term orders, SquareMagnitude/DotProduct helpers and
// separate dir stores, a scale helper (with or without component copies, products
// first, 1 / len argument), copies of d or of d.x/d.z for the length or the scale
// (all 145; component stores 109), and an explicit `float dx` held across the scale
// (copied before or after the length, read in any operand order; all 145), and dir, d
// or len declared at function scope or len computed through an if/else (all 145), and a
// 512-variant sweep of single-product brackets and operand orders (all 145).
// Scaling a copy in place (`Vec3 dir = d; dir.x *= inv; ...`, the SoultreePhysics slot 38
// lever), in any component order, copied before the length or scaling d itself, gives
// 55-60: retail consumes d.x and d.z in place but keeps the product temporaries.
// The promoted file supplies the rest of the unit.
#include "motion/SteeringControl.cpp"

#include "SteeringAxisFromPoints.cpp"
