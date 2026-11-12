#pragma once

class KrustyBike;

struct KbRaceHandler {
    void NotifyContactImpulse(void* bike, float impulse, int eventKind);
    void ReportTrickScore(KrustyBike* who, float score);   // 0x004e57d0: reports a trick score
};
