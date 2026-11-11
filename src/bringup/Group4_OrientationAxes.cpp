// Group4_OrientationAxes.cpp -- link bring-up: the provider of OrientationAxesFromAngles
// (0x004b5d00, cdecl), which the Scene readers of samples/track/SceneManagerNearMisses.cpp
// call out of line as retail does (a link_aliases entry binds their
// `void (Vector3*, Vector3*, float, float, int)` declaration to this definition; the int 0
// they pass is the same zero dword as a 0.0f roll, and they ignore the result).
//
// The body is the exact (274/274) sample samples/physics/helpers/OrientationAxes.cpp,
// included unchanged. Its owning unit is not settled (between OptionProcs.cpp and
// overlay.cpp in retail; ownership tier 3), so it stays in samples/ and the game takes it here.
#include "../../samples/physics/helpers/OrientationAxes.cpp"
