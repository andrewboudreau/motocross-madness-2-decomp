// The inherited member retains UIDialog ownership and the original layouts.
#include "../../src/reconstructed/OptionProcs.h"
#include <stddef.h>

typedef char CheckOptionsSize[sizeof(OptionsDlg) == 0x7f70 ? 1 : -1];
typedef char CheckOptionsPages[offsetof(OptionsDlg, gameSettingsPage) == 0x7f58 ? 1 : -1];
typedef char CheckGlobalSize[sizeof(GlobalSettingsDlg) == 0x7f58 ? 1 : -1];
typedef char CheckConfirmSize[sizeof(ConfirmRestoreDlg) == 0x7f58 ? 1 : -1];
typedef char CheckAdvancedSize[sizeof(OptAdvancedGraphicsDlg) == 0x7f58 ? 1 : -1];
typedef char CheckMessagesSize[sizeof(OptMessagesDlg) == 0x7f58 ? 1 : -1];

void (UIDialog::* const inheritedSync[])(int) = {
    &OptionsDlg::SyncValues,
    &GlobalSettingsDlg::SyncValues,
    &ConfirmRestoreDlg::SyncValues,
    &OptAdvancedGraphicsDlg::SyncValues,
    &OptMessagesDlg::SyncValues
};
