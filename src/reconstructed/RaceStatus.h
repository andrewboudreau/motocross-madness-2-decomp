#pragma once

#include "RaceView.h"

// RaceStatus.cpp (literal __FILE__ at 0x00572990): the per-racer status list
// (UnknownEventRacerPart nodes, 0x54 bytes, DebugCalloc'd) that a race view
// builds for its racers, the gate and lap counting behind it, and the race
// order. All cdecl; names are provisional. See docs/RACESTATUS.md.

class Track;

// 0x00689c30 and 0x00689c48: debug arrows of the next gate (BikeRace.h
// declares them for its slot 14), set by 0x004e59c0.
extern Vector3 g_UnknownGlobal689c30[2], g_UnknownGlobal689c48[2];
// 0x00689c6c: cleared when the list is built; set once the view's own racer
// has finished.
extern int g_UnknownGlobal689c6c;

// 0x0047b670 (gameui.cpp): whether moving from `from` to `to` crosses
// `gate`.
int UnknownFunction47b670(Vector3* from, Vector3* to, UnknownBikeRaceNode* gate, int direction);

// 0x004e58c0: counts a lap when `from`..`to` crosses the finish `gate`
// (or `forced` when `skipTest`): lap time, lap start times by lap % 100,
// the best lap from the second lap on, `finished` at `lapLimit` unless
// `unlimited`.
int UnknownFunction4e58c0(Vector3* from, Vector3* to, unsigned short* laps, UnknownBikeRaceNode* gate,
                          char* finished, int lapLimit, float* lapTime, float* lapTimes, float now,
                          float* lapStart, float* bestLap, int direction, int unlimited, int skipTest,
                          int forced);
// 0x004e59c0: advances `*current` through the gate list when crossed (back
// to `first` and a lap counted after the last), records the gate time by
// gate count % 600, and points the debug arrows at the next gate.
int UnknownFunction4e59c0(Vector3* from, Vector3* to, UnknownBikeRaceNode** current, UnknownBikeRaceNode* first,
                          int* count, int* gateIndex, float* times, float now, int direction,
                          unsigned short* laps, float* lapTime, float* bestLap, float* lapStart, char* finished,
                          int lapLimit, int unlimited, int skipTest, int forced);
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
// 0x004e5f40: track distances to `probe`, then orders `*list` by laps and
// distance and numbers the racers' positions.
int UnknownFunction4e5f40(UnknownEventRacerPart** list, Track* track, UnknownBikeRaceNode* probe);
// 0x004e6120: orders `*list` by gates passed and distance to the next gate
// and numbers the racers' positions.
int UnknownFunction4e6120(UnknownEventRacerPart** list);
// 0x004e6210: each racer's time behind the leader, from gate times (`mode`
// 1) or lap times.
int UnknownFunction4e6210(UnknownEventRacerPart* list, int mode);
// 0x004e62d0: ranks the view's racers by score (+0x768) and, unless
// `keepRacing`, finishes them all when the time limit is up.
int UnknownFunction4e62d0(int keepRacing);
// 0x004e63e0: per frame for a lap race: places each unfinished racer on the
// track (leading by 3.5 times its velocity), counts laps at `finish`, fills
// in the other racers' results once the view's racer has finished, and
// orders the list.
int UnknownFunction4e63e0(UnknownEventRacerPart** list, Track* track, float frameTime, UnknownBikeRaceNode* start,
                          UnknownBikeRaceNode* finish, int lapLimit, int unlimited);
// 0x004e6a50: per frame for a gate race: counts each unfinished racer's
// gates from `firstGate`, reports a changed gate of the followed racer to
// the view owner, fills in the other racers' results once the view's racer
// has finished, and orders the list.
int UnknownFunction4e6a50(UnknownEventRacerPart** list, UnknownBikeRaceNode* firstGate, float frameTime, int lapLimit,
                          int unlimited);
