// Group5 bring-up definition (best-guess, link frontier). Not a strict match.
//
// PCBailoutDlg::HandleEvent (0x004d8fc0), extracted from
// samples/ui/ProCircuitProcsNearMisses.cpp (836 of 882 bytes match: same
// calls, strings and flow; register choices differ). The includes and the
// file-local UnknownSetText helper are those of
// src/reconstructed/ProCircuitProcs.cpp; __FILE__ is the retail path.
#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#include "ProCircuitProcs.h"

#include "DialogEventKind.h"
#include "SelectGamePicProcs.h"
#include "KrustyUI.h"
#include "MenuId.h"
#include "TrackGame.h"
#include "EventManager.h"
#include "MatrixUtil.h"
#include "DebugAlloc.h"
#include "../krusty2/math/FastMath.h"
#include "OptionProcs.h"
#include "BackgroundImage.h"
#include "Camera.h"

#define PRO_CIRCUIT_PROCS_FILE "D:\\aardvark\\VC\\krusty2\\ProCircuitProcs.cpp"

static inline void UnknownSetText(UIControl* control, int id)
{
    control->SetTextFromResource(g_TrackGame->resourceInstance, id);
}

// 0x004d8fc0
void PCBailoutDlg::HandleEvent(UnknownDialogEvent* event)
{
    UnknownTrackGameObject3444* circuit = g_TrackGame->proCircuit;
    UIControl* label;
    char text[128];
    switch (event->kind) {
    case kDialogInit: {
        int fee = circuit->entryFees[circuit->series];
        int cash = circuit->racers[0].money;
        playerCash = cash;
        cashShortfall = fee - cash;
        entryFee = fee;
        UnknownSetText(FindControl("TitleText", 12), 0x144a);
        UIControl* description = FindControl("TxtDescription", 12);
        description->wordWrap = 1;
        UnknownSetText(description, 0x1449);
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
        UnknownSetText(FindControl("TxtItem4", 12), 0x144c);
        item = FindControl("Item4", 12);
        sprintf(text, "$%d", cashShortfall);
        item->SetText(text);
        item->SetFontColor(0xff00);
        FindControl("Back", 1)->Show(0, 1);
        break;
    }
    case kDialogCommand:
        if (_stricmp("ButAccept", event->controlName) == 0) {
            circuit->racers[0].money += cashShortfall;
            circuit->racers[0].debts += cashShortfall;
            g_TrackGame->proCircuit->Save(g_TrackGame->careerFile);
            PCCentralDlg* dialog = new(PRO_CIRCUIT_PROCS_FILE, 1551) PCCentralDlg;
            guiManager->ShowDialog((UIDialog*)dialog, 0, 2, 0, 0, 0, 0, 1);
            EndDialog(0);
        } else if (_stricmp("ButDecline", event->controlName) == 0) {
            circuit->careerFlags |= 4;
            PCFailedDlg* dialog = new(PRO_CIRCUIT_PROCS_FILE, 1556) PCFailedDlg;
            guiManager->ShowDialog((UIDialog*)dialog, 0, 2, 0, 0, 0, 0, 1);
            EndDialog(0);
        }
        break;
    }
}

