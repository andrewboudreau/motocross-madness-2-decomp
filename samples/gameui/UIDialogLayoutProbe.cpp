// UIDialogLayoutProbe.cpp -- compile-time proof that UIDialog.h's member
// layout and the derived dialog classes reproduce the retail sizes.
//
// UIDialog's offsets are those its constructor 0x00469db0 initialises and
// its destructor 0x0046a070 releases (src/reconstructed/GameUi.cpp); the
// derived sizes are the `new` sites that allocate each dialog (recorded in
// the class comments of the dialog headers). A failing check is a compile
// error.
#include <stddef.h>

#include "../../src/reconstructed/DialogProc.h"
#include "../../src/reconstructed/DlgProcs.h"
#include "../../src/reconstructed/InGameProcs.h"
#include "../../src/reconstructed/NetProcs.h"
#include "../../src/reconstructed/OptionProcs.h"
#include "../../src/reconstructed/ProCircuitProcs.h"
#include "../../src/reconstructed/SelectGamePicProcs.h"
#include "../../src/reconstructed/TrackRecordDlg.h"
#include "../../src/reconstructed/UIDialog.h"

#define LAYOUT_CHECK(name, cond) typedef char layout_check_##name[(cond) ? 1 : -1]
#define DIALOG_SIZE(cls, size) LAYOUT_CHECK(size_##cls, sizeof(cls) == (size))
#define DIALOG_OFFSET(member, offset) LAYOUT_CHECK(offset_##member, offsetof(UIDialog, member) == (offset))

// UIDialog (0x7f58 bytes).
LAYOUT_CHECK(uidialog_size, sizeof(UIDialog) == 0x7f58);
DIALOG_OFFSET(parentDialog, 0x2c);
DIALOG_OFFSET(guiManager, 0x30);
DIALOG_OFFSET(guiUser, 0x34);
DIALOG_OFFSET(resourceName, 0x38);
DIALOG_OFFSET(resourceArchive, 0xb8);
DIALOG_OFFSET(isShown, 0xbc);
DIALOG_OFFSET(openingMenu, 0xc4);
DIALOG_OFFSET(animatesControls, 0xc8);
DIALOG_OFFSET(dialogFont, 0xd8);
DIALOG_OFFSET(dialogFontHeight, 0xdc);
DIALOG_OFFSET(dialogFontFace, 0xe0);
DIALOG_OFFSET(dialogBackground, 0x110);
DIALOG_OFFSET(soundGroup, 0x114);
DIALOG_OFFSET(textColors, 0x118);
DIALOG_OFFSET(dialogPalette, 0x140);
DIALOG_OFFSET(field_0x144, 0x144);
DIALOG_OFFSET(field_0x148, 0x148);
DIALOG_OFFSET(field_0x154, 0x154);
DIALOG_OFFSET(scaleToScreen, 0x158);
DIALOG_OFFSET(screenArea, 0x160);
DIALOG_OFFSET(popupAlignment, 0x170);
DIALOG_OFFSET(screenWidth, 0x174);
DIALOG_OFFSET(dialogResult, 0x17c);
DIALOG_OFFSET(imageTable, 0x18c);
DIALOG_OFFSET(cursorAnimation, 0x95c);
DIALOG_OFFSET(soundTable, 0x960);
DIALOG_OFFSET(imageCount, 0x9b0);
DIALOG_OFFSET(parentBackground, 0x9b8);
DIALOG_OFFSET(scaleX, 0x9c4);
DIALOG_OFFSET(sectionTable, 0x9cc);
DIALOG_OFFSET(sectionCount, 0x7efc);
DIALOG_OFFSET(ownsPalette, 0x7f0c);
DIALOG_OFFSET(field_0x7f18, 0x7f18);
DIALOG_OFFSET(dialogTextures, 0x7f20);
DIALOG_OFFSET(timerList, 0x7f24);
DIALOG_OFFSET(controlContainer, 0x7f3c);
DIALOG_OFFSET(screenGrab, 0x7f44);
DIALOG_OFFSET(grabRegion, 0x7f48);
DIALOG_OFFSET(isClosing, 0x7f50);
DIALOG_OFFSET(closeFrame, 0x7f54);
LAYOUT_CHECK(section_size, sizeof(UnknownGameUiSection) == 0x3c);
LAYOUT_CHECK(timer_list_size, sizeof(ContainerList<UITimer*>) == 0x14);

// Derived dialogs: their own members start at +0x7f58.
DIALOG_SIZE(ChoiceDlg, 0x7f58);
DIALOG_SIZE(SessionDlg, 0x7f58);
DIALOG_SIZE(TransDlg, 0x7f64);
DIALOG_SIZE(Intro1Dlg, 0x7f5c);
DIALOG_SIZE(Exit1Dlg, 0x7f68);
DIALOG_SIZE(MainDlg, 0x7f80);
DIALOG_SIZE(NewbieDlg, 0x7f58);
DIALOG_SIZE(SinglePlayerDlg, 0x7f64);
DIALOG_SIZE(SPEventDlg, 0x7f68);
DIALOG_SIZE(SPBikeRiderDlg, 0x7f9c);
DIALOG_SIZE(SPRaceInfoDlg, 0x7f58);
DIALOG_SIZE(Intro2Dlg, 0x7f64);
DIALOG_SIZE(Intro3Dlg, 0x7f60);
DIALOG_SIZE(EditBoxDlg, 0x7f60);
DIALOG_SIZE(DemoDlg, 0x7f58);
DIALOG_SIZE(LoadingDlg, 0x7f5c);
DIALOG_SIZE(ProfileExistsDlg, 0x7f58);
DIALOG_SIZE(GhostFilesDlg, 0x7f88);
DIALOG_SIZE(ReplayFilesDlg, 0x7f88);
DIALOG_SIZE(OptGameSettingsDlg, 0x7f58);
DIALOG_SIZE(OptGraphicsDlg, 0x7f58);
DIALOG_SIZE(OptSoundDlg, 0x7f58);
DIALOG_SIZE(OptControlsDlg, 0x7f7c);
DIALOG_SIZE(OptGarageDlg, 0x7f5c);
DIALOG_SIZE(OptMessagesDlg, 0x7f58);
DIALOG_SIZE(GlobalSettingsDlg, 0x7f58);
DIALOG_SIZE(ConfirmRestoreDlg, 0x7f58);
DIALOG_SIZE(MPEventDlg, 0x7f5c);
DIALOG_SIZE(MPBikeRiderDlg, 0x7f9c);
DIALOG_SIZE(MPRaceInfoDlg, 0x7f58);
DIALOG_SIZE(MPOptionsDlg, 0x7f58);
DIALOG_SIZE(MultiPlayerDlg, 0x8110);
DIALOG_SIZE(PCBonusTrackDlg, 0x7f58);
DIALOG_SIZE(PCBailoutDlg, 0x7f64);
DIALOG_SIZE(PCFinishedDlg, 0x7f58);
DIALOG_SIZE(PCBunnyDlg, 0x7f64);
DIALOG_SIZE(PCFailedDlg, 0x7f58);
DIALOG_SIZE(PCCompleteDlg, 0x7f58);
DIALOG_SIZE(PCNewEventDlg, 0x7f58);
DIALOG_SIZE(PCCentralNextDlg, 0x7f58);
DIALOG_SIZE(PCCentralBikeRiderDlg, 0x7f9c);
DIALOG_SIZE(PCCentralStandingsDlg, 0x7f58);
DIALOG_SIZE(PCCentralDlg, 0x7f64);
DIALOG_SIZE(PCNewDlg, 0x7f68);
DIALOG_SIZE(TrackRecordDlg, 0x7f68);
