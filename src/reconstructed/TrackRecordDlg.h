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

// The "LstTrack" control. Of the gameui.cpp classes only UIListBox
// (vtable 0x00553100) and UIDDLListBox have a slot 66, so this is probably
// UIListBox (strong inference). Only slot 66 is called: UIListBox::ReportClick
// (GameUi.h, 0x00477d30), which sends the list-select event.
class TrackRecordUIListBoxView {
public:
    virtual void UnknownVirtualSlot0();
    virtual void AddRef();
    virtual void Release();
    virtual void GetRefCount();
    virtual void Disable();
    virtual void Enable();
    virtual void OnDisable();
    virtual void OnEnable();
    virtual void SetRenderContext();
    virtual void PreTick();
    virtual void Tick();
    virtual void Step();
    virtual void PrepareGeometry();
    virtual void RenderPre3D();
    virtual void Render3D();
    virtual void RenderPost3D();
    virtual void SetPaused();
    virtual void OnDeactivate();
    virtual void OnActivate();
    virtual void OnInputEvent();
    virtual void OnChar();
    virtual void OnKeyDown();
    virtual void OnControlRelease();
    virtual void OnControlPress();
    virtual void OnNetworkMessage();
    virtual void AttachRenderContext();
    virtual void MarkPendingRelease();
    virtual void AdvanceImages();
    virtual void ContainsPoint();
    virtual void SetState();
    virtual void IsInteractive();
    virtual void EnterHover();
    virtual void LeaveHover();
    virtual void KillFocus();
    virtual void GetControlText();
    virtual void GetTextLength();
    virtual void GetTextLines();
    virtual void GetTextExtent();
    virtual void UpdateNeedsRedraw();
    virtual void ScaleArea();
    virtual void Draw();
    virtual void GrowStep();
    virtual void SlideLeftStep();
    virtual void SlideRightStep();
    virtual void SlideTopStep();
    virtual void SlideBottomStep();
    virtual void AnimInStep();
    virtual void AnimOutStep();
    virtual void GetStateTexture();
    virtual void SetEnabled();
    virtual void RequestRedraw();
    virtual void HitTest();
    virtual void SetAttachId();
    virtual void CreateControlIterator();
    virtual void BindValue();
    virtual void ButtonDown();
    virtual void ButtonUp();
    virtual void ButtonHeld();
    virtual void PlayControlSound();
    virtual void SyncBoundValue();
    virtual void HandleTimer();
    virtual void GetWidth();
    virtual void GetHeight();
    virtual void GetImageRect();
    virtual void GetTextRect();
    virtual void SetSelectedRow();
    virtual void ReportClick(int* handled);
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
