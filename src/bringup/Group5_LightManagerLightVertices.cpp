// Group5 bring-up definition (best-guess, link frontier). Not a strict match.
//
// LightManager::LightVertices (0x0049e4a0, ret 0x18; LightEmitter.h) is the
// function Griddraw.h calls as GridVertexSink::LightVertices. The terrain's
// view already has a body (src/bringup/Group4_GridLighting.cpp, written from
// the retail disassembly of 0x0049e4a0 and its three workers), so the
// LightManager name forwards to it. The two views differ only in the type of
// the last argument (LightEmitter.h: colour array pointer, Griddraw.h: int).
// Both classes are declared only as far as the call needs; the qualified call
// binds directly and no vtable is referenced.
class GridVertexSink {
public:
    void LightVertices(void* matrix, int count, void* vertices, void* buffer,
                       int stride, int flags);  // 0x0049e4a0
};

class LightManager {
public:
    void LightVertices(void* matrix, int count, void* vertices, void* buffer,
                       int stride, void* colors);
};

void LightManager::LightVertices(void* matrix, int count, void* vertices, void* buffer, int stride,
                                 void* colors)
{
    reinterpret_cast<GridVertexSink*>(this)->LightVertices(matrix, count, vertices, buffer, stride,
                                                           (int)colors);
}
