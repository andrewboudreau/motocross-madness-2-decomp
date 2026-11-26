// Group5 bring-up definition (best-guess, link frontier). Not a strict match.
//
// LightManager::LightVertices (0x0049e4a0, ret 0x18; LightEmitter.h) is the
// function Griddraw.h calls as GridVertexSink::LightVertices. The terrain's
// view already has a body (src/bringup/Group4_GridLighting.cpp, written from
// the retail disassembly of 0x0049e4a0 and its three workers), so the
// LightManager name forwards to it. The two views differ only in the type of
// the last argument (LightEmitter.h: colour array pointer, Griddraw.h: int).
// The qualified call binds directly and no vtable is referenced.
#include "Griddraw.h"       // GridVertexSink
#include "LightEmitter.h"   // LightManager

void LightManager::LightVertices(void* matrix, int count, void* vertices, void* buffer, int stride,
                                 void* colors)
{
    reinterpret_cast<GridVertexSink*>(this)->LightVertices(matrix, count, vertices, buffer, stride,
                                                           (int)colors);
}
