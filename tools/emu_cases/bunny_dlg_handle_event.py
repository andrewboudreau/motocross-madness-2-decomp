"""PCBunnyDlg::HandleEvent 0x004d9860, the career loan offer (tools/emu_diff.py case,
call-record/replay mode).

The linked body is src/bringup/Group4_DlgPCBunny.cpp. The layout, events, replayed calls and
stand-ins are those of bailout_dlg_handle_event: the init fills five labels (cash, entry fee,
debts, the loan of fee - cash + debts at +0x7f60, and a fixed $1000); ButAccept adds the loan
less the debts to the money and clears the debts; ButDecline sets career flag 4.
"""
from tools.emu_cases.bailout_dlg_handle_event import REPLAY, cases, setup  # noqa: F401

RETAIL_VA = 0x004d9860
SYMBOL = '?HandleEvent@PCBunnyDlg@@UAEXPAUUnknownDialogEvent@@@Z'
SOURCE = 'src/bringup/Group4_DlgPCBunny.cpp'
PROBES = {0x004d98a6: 'init', 0x004d9b77: 'accept', 0x004d9c2a: 'decline'}
