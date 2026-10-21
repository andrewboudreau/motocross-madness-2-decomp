#pragma once

class KrustyBike;

struct KbRaceHandler {
    void Fn_004DE580(void* who, float a, int b);
    void ReportTrickScore(KrustyBike* who, float score);   // 0x004e57d0: reports a trick score
};
