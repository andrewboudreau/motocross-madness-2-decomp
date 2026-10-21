#include "../../src/reconstructed/KrustyUI.h"
#include "../../src/reconstructed/DlgProcs.h"
#include <stddef.h>

typedef char ProgressOffset[offsetof(KrustyUI, progressDialog) == 0x490 ? 1 : -1];
typedef char LoadingSize[sizeof(LoadingDlg) == 0x7f5c ? 1 : -1];
typedef char LoadingTail[offsetof(LoadingDlg, framesShown) == 0x7f58 ? 1 : -1];
UIDialog* KrustyUI::* progressDialogMember = &KrustyUI::progressDialog;

// This implicit conversion must retain the primary pointer published by retail.
UIDialog* LoadingDialogBase(LoadingDlg* dialog) { return dialog; }
