#pragma once

#include "RaceView.h"

// RaceStatus.cpp (literal __FILE__ at 0x00572990): the per-racer status list
// (UnknownEventRacerPart nodes, 0x54 bytes, DebugCalloc'd) that a race view
// builds for its racers. All cdecl; names are provisional.

// 0x00689c6c: cleared when the list is built.
extern int g_UnknownGlobal689c6c;

// 0x004e5c70: whether `list` has a node for `racer`.
int UnknownFunction4e5c70(UnknownEventRacerPart* list, UnknownEventRacer* racer);
// 0x004e5ca0: frees the nodes of racers whose GameObject +0x25 bit 0 is
// clear, detaching them from their racers.
int UnknownFunction4e5ca0(UnknownEventRacerPart** list);
// 0x004e5d00: builds `*list` for `view`: its own racer, then (in some race
// modes) the AI racers, then the other racers.
int UnknownFunction4e5d00(UnknownEventRacerPart** list, UnknownKrustyBikeView* view);
// 0x004e5f10: frees every node of `list`.
int UnknownFunction4e5f10(UnknownEventRacerPart* list);
