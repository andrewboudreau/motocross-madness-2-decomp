// Group5 bring-up definition (best-guess, link frontier). Not a strict match.
//
// 0x004e57d0 is RaceSound::PlayScoreSounds (defined in racesnd.cpp; same
// body: compares +0x2c with the racer, plays bonusSound for scores >= 5000 in
// event types 0/4, applause in event type 3). KrustyBike.cpp reaches it as
// race->handler->ReportTrickScore(this, score), so the handler object is the
// RaceSound and the bike pointer is the racer it compares.
#include "../krusty2/vehicle/BikeRaceHandlerView.h"
#include "RaceSound.h"

void KbRaceHandler::ReportTrickScore(KrustyBike* who, float score)
{
    reinterpret_cast<RaceSound*>(this)->PlayScoreSounds(reinterpret_cast<EventKrustyBikeView*>(who), score);
}
