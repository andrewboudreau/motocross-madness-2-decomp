// Bring-up helper (best-guess-unreviewed) for the terrain's vertex lighting in
// Group4_GridLighting.cpp: a light's colour scaled to 0..255.
//
// Retail's lighting workers (0x0049e6b0, 0x0049f1c0, 0x0049f8d0 in LightEmitter.cpp)
// convert the light colours with `call __ftol` (truncation; 0x0049e7b1, 0x0049e7c1,
// 0x0049e7d1 and the other ftol sites) next to their inline-asm fistp roundings. The
// game build compiles Group4_GridLighting.cpp with /QIfist so that its casts become
// those FISTP roundings; that flag would also make these casts round. This helper
// stays in its own unit, compiled without /QIfist, so it keeps retail's truncation.
// It was already called out of line (a static function) before the split, so the
// workers' shape does not change; retail inlines the three __ftol calls instead.
#include "LightEmitter.h"

// retail: _ftol of colour * 255, inlined in each worker.
void Group4Vis_LightColor(const LightEmitter* light, int* r, int* g, int* b) {
    *r = (int)(light->ambientRGBA[0] * 255.0f);
    *g = (int)(light->ambientRGBA[1] * 255.0f);
    *b = (int)(light->ambientRGBA[2] * 255.0f);
}
