// Helper called by EcoSystem at 0x0045ab4b. Recovered behavior at 0x00401040.
// It increments a generation/age-like dword; it does not clear a buffer.
// Original class, field and method names remain unknown.
struct EpochProbe { unsigned int tick; void Advance(); };
void EpochProbe::Advance(){++tick;}
