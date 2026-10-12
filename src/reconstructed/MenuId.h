#pragma once

// Menu ids taken by KrustyUI::OpenMenu (0x00499b20) and kept in KrustyUI+0x3c.
// Each name is the dialog class (RTTI) that OpenMenu constructs for the id
// (tier 2); OpenMenu passes the id on to ShowDialog, and DlgProcs.cpp's button
// handlers 0x004526b0, 0x00452930 and 0x00453090 switch on it. Ids that share
// a dialog keep their number as a suffix: what tells them apart is not
// established. Ids OpenMenu has no dialog for (0xc9-0xcd, 0xd0, 0x104, 0x105,
// 0x10e, 0x12c) are left numeric.
//
// SelectGamePicProcs.cpp does not include it: declaring the enum there (at
// any include position, even unused) changes VC6's code for FillFileList
// (0x004f17a0).
enum KrustyMenuId {
    kMenuMain = 100,              // MainDlg
    kMenuSinglePlayer = 101,      // SinglePlayerDlg
    kMenuOptions = 102,           // OptionsDlg
    kMenuLoading = 104,           // LoadingDlg(1)
    kMenuSerialPopup = 0xd8,      // SerialPopupDlg
    kMenuTCPAddress = 0xd9,       // TCPAddressDlg
    kMenuWaitOrCall = 0xfc,       // WaitOrCallDlg
    kMenuRemoveProfile = 0x12f,   // RemoveProfileDlg
    kMenuContinue = 0x190,        // ContinueDlg (TrackGame slot 14 toggles it)
    kMenuExit = 0x191,            // ExitDlg (TrackGame slot 14 opens it)
    kMenuConnectError = 0x1f9,    // ConnectErrorDlg
    kMenuPlayerRemoved = 0x1fa,   // PlayerRemovedDlg
    kMenuHostJoin85c = 0x85c,     // HostJoinDlg
    kMenuHostJoin85d = 0x85d,     // HostJoinDlg
    kMenuMultiPlayer866 = 0x866,  // MultiPlayerDlg
    kMenuMultiPlayer867 = 0x867,  // MultiPlayerDlg
    kMenuMultiPlayer868 = 0x868,  // MultiPlayerDlg
    kMenu88e = 0x88e,             // PCLastRaceDlg or PCCentralDlg with a pro
                                  // circuit, else SinglePlayerDlg or MultiPlayerDlg
    kMenuPCCentral = 0x88f,       // PCCentralDlg (only with a pro circuit)
    kMenuTrackRecord910 = 0x910,  // TrackRecordDlg; closing it opens kMenuSinglePlayer
    kMenuTrackRecord911 = 0x911,  // TrackRecordDlg; closing it opens KrustyUI+0x38
    kMenuUserNameBb9 = 0xbb9,     // UserNameDlg
    kMenuUserNameBba = 0xbba,     // UserNameDlg
    kMenuUserNameBbb = 0xbbb      // UserNameDlg
};
