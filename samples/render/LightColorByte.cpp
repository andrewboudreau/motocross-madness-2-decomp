// LightColorByte.cpp -- helper (no retail body of its own) for the vertex lighting in
// LightManagerLighting.cpp: one channel of a light's colour scaled to 0..255.
//
// Retail's lighting workers (0x0049e6b0, 0x0049f1c0, 0x0049f8d0 in LightEmitter.cpp)
// convert the light colours with `fld; fmul 255.0f; call __ftol` (truncation;
// 0x0049e7b1, 0x0049e7c1, 0x0049e7d1 and the other ftol sites) next to their
// inline-asm fistp roundings. The game build compiles LightManagerLighting.cpp with
// /QIfist so that its casts become those FISTP roundings; that flag would also make
// this cast round. The helper stays in its own file (config/game_build.json
// candidate_sources, no extra flags), so it keeps retail's truncation; retail inlines
// it at each site instead.
int LightingColorByte(float channel) {
    return (int)(channel * 255.0f);
}
