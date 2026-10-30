// Group0_PCLastRaceDlg.cpp -- bring-up body for PCLastRaceDlg::PayOutRace,
// extracted from samples/ui/ProCircuitProcsNearMisses.cpp (that sample
// reports the same flow, calls and x87 code as retail with a register/load
// order residue). Only this function is taken from the sample; the sample
// itself includes the whole ProCircuitProcs.cpp, so the includes here are
// that file's own.
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

// 0x005719e0, the retail __FILE__ of ProCircuitProcs.cpp.
static const char kProCircuitProcsFile[] = "D:\\aardvark\\VC\\krusty2\\ProCircuitProcs.cpp";

// 0x004d8c40: pays out the race: purse shares by place, capped medical,
// repair and stunt amounts per result, then race and season ranks; the
// unpaid remainder of the purse goes to the race winner. As in retail the
// share buffer (line 1420) is never freed.
void PCLastRaceDlg::PayOutRace()
{
    UnknownTrackGameObject3444* circuit = g_TrackGame->proCircuit;
    int fee = circuit->entryFees[circuit->series];
    int purse = (int)(fee * circuit->purseMultiplier);
    float* shares = (float*)DebugMalloc(circuit->racerCount * 4, kProCircuitProcsFile, 1420);
    float total = 0.0f;
    int i;
    for (i = 0; i < circuit->racerCount; i++)
        total += circuit->placePct[i];
    for (i = 0; i < circuit->racerCount; i++)
        shares[i] = circuit->placePct[i] / total;
    circuit->feesPaid += fee;

    int paid = 0;
    for (int j = 0; j < g_TrackGame->eventManager->resultCount; j++) {
        UnknownProCircuitResult* result = &((UnknownProCircuitResult*)g_TrackGame->eventManager->results)[j];
        UnknownProCircuitRacer* racer = &circuit->racers[result->racer];
        racer->raceStats.medicalCost = (int)__min(purse * circuit->medicalCapPct, result->medical * 250.0f);
        racer->raceStats.repairCost = (int)__min(purse * circuit->repairCapPct, result->repairs * 500.0f);
        racer->raceStats.stuntWinnings = (int)__min(purse * circuit->stuntCapPct, result->stunts * 400.0f);
        racer->raceStats.purseWinnings = (int)ceil(purse * shares[result->position - 1]);
        racer->raceStats.points = result->points;
        paid += racer->raceStats.purseWinnings;
        racer->money += racer->raceStats.stuntWinnings - racer->raceStats.medicalCost - racer->raceStats.repairCost - fee +
                             racer->raceStats.purseWinnings;
        racer->totalStats.medicalCost += racer->raceStats.medicalCost;
        racer->totalStats.repairCost += racer->raceStats.repairCost;
        racer->totalStats.stuntWinnings += racer->raceStats.stuntWinnings;
        racer->totalStats.purseWinnings += racer->raceStats.purseWinnings;
        racer->totalStats.points += racer->raceStats.points;
    }

    UnknownProCircuitRank order[11];
    for (i = 0; i < circuit->racerCount; i++) {
        order[i].points = circuit->racers[i].raceStats.points;
        order[i].racer = i;
    }
    qsort(order, circuit->racerCount, sizeof(UnknownProCircuitRank), CompareRankPoints);
    for (i = 0; i < circuit->racerCount; i++)
        circuit->racers[order[i].racer].raceStats.rank = i + 1;
    int rest = purse - paid;
    circuit->racers[order[0].racer].raceStats.purseWinnings += rest;
    circuit->racers[order[0].racer].totalStats.purseWinnings += rest;
    circuit->racers[order[0].racer].money += rest;
    for (i = 0; i < circuit->racerCount; i++) {
        order[i].points = circuit->racers[i].totalStats.points;
        order[i].racer = i;
    }
    qsort(order, circuit->racerCount, sizeof(UnknownProCircuitRank), CompareRankPoints);
    for (i = 0; i < circuit->racerCount; i++)
        circuit->racers[order[i].racer].totalStats.rank = i + 1;
}
