"""LightManager::LightVertices 0x0049e4a0, the LightEmitter.h forwarder (tools/emu_diff.py case).

src/bringup/Group5_LightManagerLightVertices.cpp forwards to GridVertexSink::LightVertices
(src/bringup/Group4_GridLighting.cpp) with the colour array pointer passed as the int
last argument. The callee binds to retail 0x0049e4a0 here, so this checks the forwarder's
arguments against retail's body. Scenes, setup (allocator stubs included) and probes are
grid_light_vertices'.
"""
from tools.emu_cases.grid_light_vertices import BINDINGS, PROBES, RETAIL_VA, cases, setup

SYMBOL = '?LightVertices@LightManager@@QAEXPAXH00H0@Z'
SOURCE = 'src/bringup/Group5_LightManagerLightVertices.cpp'

__all__ = ['BINDINGS', 'PROBES', 'RETAIL_VA', 'SOURCE', 'SYMBOL', 'cases', 'setup']
