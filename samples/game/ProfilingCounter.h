#pragma once

// Functional profiling backend. Retail reads RDTSC at these boundaries; VC6
// cannot express that instruction in ordinary C++. This QPC implementation is
// deliberately nonexact and must be used together with its calibration.
__int64 ReadProfilingCounter();
