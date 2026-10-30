// Bring-up definition (best-guess-unreviewed) for PCBunnyDlg::HandleEvent
// (retail 0x004d9860), the Pro Circuit loan ("bunny") offer dialog.
// Extracted from the near-miss candidate in
// samples/ui/ProCircuitProcsNearMisses.cpp (compiled there inside a copy of
// ProCircuitProcs.cpp); ProCircuitProcs.h declares the method but
// src/reconstructed/ProCircuitProcs.cpp does not define it yet.
#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#include "../reconstructed/ProCircuitProcs.h"

#include "../reconstructed/DialogEventKind.h"
#include "../reconstructed/SelectGamePicProcs.h"
#include "../reconstructed/KrustyUI.h"
#include "../reconstructed/MenuId.h"
#include "../reconstructed/TrackGame.h"
#include "../reconstructed/EventManager.h"
#include "../reconstructed/MatrixUtil.h"
#include "../reconstructed/DebugAlloc.h"
#include "../krusty2/math/FastMath.h"
#include "../reconstructed/OptionProcs.h"
#include "../reconstructed/BackgroundImage.h"
#include "../reconstructed/Camera.h"

// Sets `control`'s text from string resource `id` (ProCircuitProcs.cpp's helper).
static inline void UnknownSetText(UIControl* control, int id)
{
    control->SetTextFromResource(g_TrackGame->resourceInstance, id);
}

// 0x004d9860
void PCBunnyDlg::HandleEvent(UnknownDialogEvent* event)
{
    UnknownTrackGameObject3444* circuit = g_TrackGame->proCircuit;
    char text[128];
    switch (event->kind) {
    case kDialogInit: {
        int debt = circuit->racers[0].debts;
        int fee = circuit->entryFees[circuit->series];
        int cash = circuit->racers[0].money;
        playerCash = cash;
        loanAmount = fee - cash + debt;
        entryFee = fee;
        UnknownSetText(FindControl("TitleText", 12), 0x1452);
        UIControl* description = FindControl("TxtDescription", 12);
        description->wordWrap = 1;
        UnknownSetText(description, 0x1451);
        unsigned int color = circuit->racers[0].money < 0 ? 0xff : 0xff00;
        UnknownSetText(FindControl("TxtItem1", 12), 0x144b);
        UIControl* item = FindControl("Item1", 12);
        sprintf(text, "$%d", playerCash);
        item->SetText(text);
        item->SetFontColor(color);
        UnknownSetText(FindControl("TxtItem2", 12), 0x144d);
        item = FindControl("Item2", 12);
        sprintf(text, "$%d", entryFee);
        item->SetText(text);
        item->SetFontColor(0xff);
        UnknownSetText(FindControl("TxtItem3", 12), 0x1453);
        item = FindControl("Item3", 12);
        sprintf(text, "$%d", debt);
        item->SetText(text);
        item->SetFontColor(0xff);
        UnknownSetText(FindControl("TxtItem4", 12), 0x144f);
        item = FindControl("Item4", 12);
        sprintf(text, "$%d", loanAmount);
        item->SetText(text);
        item->SetFontColor(0xff00);
        UnknownSetText(FindControl("TxtItem5", 12), 0x1450);
        item = FindControl("Item5", 12);
        sprintf(text, "$%d", 1000);
        item->SetText(text);
        item->SetFontColor(0xff00);
        FindControl("Back", 1)->Show(0, 1);
        break;
    }
    case kDialogCommand:
        if (_stricmp("ButAccept", event->controlName) == 0) {
            circuit->racers[0].money += loanAmount - circuit->racers[0].debts;
            circuit->racers[0].debts = 0;
            g_TrackGame->proCircuit->Save(g_TrackGame->careerFile);
            PCCentralDlg* dialog = new(__FILE__, 1744) PCCentralDlg;
            guiManager->ShowDialog((UIDialog*)dialog, 0, 2, 0, 0, 0, 0, 1);
            EndDialog(0);
        } else if (_stricmp("ButDecline", event->controlName) == 0) {
            circuit->careerFlags |= 4;
            PCFailedDlg* dialog = new(__FILE__, 1749) PCFailedDlg;
            guiManager->ShowDialog((UIDialog*)dialog, 0, 2, 0, 0, 0, 0, 1);
            EndDialog(0);
        }
        break;
    }
}
