"""ShadowTransformPointsOrtho 0x004a1b00, the shadow bridge's forwarder (tools/emu_diff.py case).

src/bringup/Group5_ShadowBridges.cpp forwards (destination, source, matrix, count,
destination stride, source stride) to TransformPoints (src/bringup/Group0_Math.cpp),
which binds to retail 0x004a1b00 here, so this checks the forwarder's argument order
against retail's body. Cases and setup are transform_points'.
"""
from tools.emu_cases.transform_points import RETAIL_VA, cases, setup

SYMBOL = '?ShadowTransformPointsOrtho@@YAXPAUShadowVec3@@PBXPBUShadowMatrix@@HHH@Z'
SOURCE = 'src/bringup/Group5_ShadowBridges.cpp'

__all__ = ['RETAIL_VA', 'SOURCE', 'SYMBOL', 'cases', 'setup']
