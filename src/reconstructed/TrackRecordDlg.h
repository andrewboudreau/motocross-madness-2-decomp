#pragma once

#include "DialogProc.h"
#include "DirectoryList.h"
#include "GameUi.h"
#include "UIDialog.h"

// An 8-byte list row (0x0051ffe0, TrackRecord.cpp line 535): a strdup'd
// name and a value.
struct TrackRecordRow {
    char* fileName;                           // +0x00 strdup of the high-score file's name; LstTrack's selection passes it to ShowTrackScores
    int field_0x04;
};

// RTTI: TrackRecordDlg : UIDialog (vtable 0x0055177c; 0x7f68 bytes, the size
// allocated at 0x0044c27b and 0x0049a2e6, with an inline constructor).
// Names are provisional (docs/TRACKRECORD.md).
class TrackRecordDlg : public UIDialog {
public:
    TrackRecordDlg() : UIDialog(1, "HiScores.dtm") {} // inline (KrustyUI 0x00499b20)
    // 0x0051f600: the dialog's procedure; its +0x08 is the type (1..6) and
    // +0x0c the dialog whose controls the list helpers fill.
    virtual void HandleEvent(DialogEvent* event);
    void LabelTabs(int series);                                 // 0x0051fc40: labels the tabs
    void ShowTab(DialogEvent* event, int series); // 0x0051fe80: shows a tab
    void FreeRows();                                            // 0x0051ff40: frees the rows
    void ListHighScoreTracks(UIControl* list, DirectoryList* directory, int series); // 0x0051ffe0
    void FillLists(DialogEvent* event, int series); // 0x00520390: fills the lists
    void AddStatsRow(DialogEvent* event, const char* name, float value,
                               const char* text);                           // 0x00520480
    // 0x005204e0: the high scores of `name` (points for series 0, else lap times).
    void ShowTrackScores(DialogEvent* event, int times, const char* name);

    int selectedSeries;                       // +0x7f58 selected series (0..5)
    int selectedSubTab;                       // +0x7f5c TabLeft/TabMiddle/TabRight (0..2); mirrors highScores->extensionIndex, LabelTabs selects it
    TrackRecordRow** rows;             // +0x7f60 the listed tracks (FreeRows)
    int rowCount;                             // +0x7f64 row count
};
