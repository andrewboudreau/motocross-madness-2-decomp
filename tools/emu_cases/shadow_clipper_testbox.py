"""D3DIMShadowClipper::TestBox 0x0052f570, the shadow view's forwarder (tools/emu_diff.py case).

src/bringup/Group1_ShadowClipper.cpp forwards every argument unchanged to
VisibilityClipper::TestBox (src/bringup/Group4_Visibility.cpp), which binds to retail
0x0052f570 here, so this checks the forwarder's argument order and return value against
retail's single body. Scenes, setup and probes are visibility_testbox's. The game build
also names that forwarder ?TestBox@GridVisibilityClipper@@...; both symbols share one
address there.
"""
from tools.emu_cases.visibility_testbox import BINDINGS, PROBES, RETAIL_VA, cases, setup

SYMBOL = '?TestBox@D3DIMShadowClipper@@QAEHPAVD3DIMShadowCameraView@@PBM11PAH2H@Z'
SOURCE = 'src/bringup/Group1_ShadowClipper.cpp'

__all__ = ['BINDINGS', 'PROBES', 'RETAIL_VA', 'SOURCE', 'SYMBOL', 'cases', 'setup']
