// Native tests exercise the reconstructed candidates, not the original game.
#include <cassert>
#include <cstdio>
#include "AllocationAccountingProbe.cpp"
AllocationLedgerProbe* probe_ledger = 0;
Word probe_fallback_counter = 0;
static Word counters_a[3], counters_b[3];
static AllocationLedgerProbe a, b;
static char token;
static Word size_result = 32, size_calls, free_calls;
static bool fail_malloc, change_ledger;
static Word counter_seen_by_allocator;
extern "C" Word probe_size_005351F0(void* p) {
    assert(p == &token); ++size_calls;
    if (change_ledger) probe_ledger = &b;
    return size_result;
}
extern "C" void probe_free_00537929(void*) { ++free_calls; }
extern "C" void* probe_malloc_0053789D(Word) {
    counter_seen_by_allocator = probe_fallback_counter;
    return fail_malloc ? 0 : &token;
}
extern "C" void* probe_calloc_00537AA9(Word, Word) { return fail_malloc ? 0 : &token; }
static void reset() {
    a = AllocationLedgerProbe(); b = AllocationLedgerProbe();
    a.byte_counters = counters_a; a.current_category = 1;
    b.byte_counters = counters_b; b.current_category = 2;
    for (int i = 0; i < 3; ++i) counters_a[i] = counters_b[i] = 100;
    probe_ledger = &a; probe_fallback_counter = 100;
    size_calls = free_calls = 0; fail_malloc = change_ledger = false;
    size_result = 32;
}
int main() {
    reset(); a.RestoreCategory(2); assert(a.current_category == 2);
    reset(); ProbeTrackedDeallocate(0); assert(size_calls == 0 && free_calls == 1 && counters_a[1] == 100);
    reset(); ProbeTrackedDeallocate(&token); assert(size_calls == 1 && free_calls == 1 && counters_a[1] == 68 && probe_fallback_counter == 100);
    reset(); probe_ledger = 0; ProbeTrackedDeallocate(&token); assert(probe_fallback_counter == 68);
    reset(); a.byte_counters = 0; ProbeTrackedDeallocate(&token); assert(probe_fallback_counter == 68);
    reset(); change_ledger = true; ProbeTrackedDeallocate(&token); assert(counters_a[1] == 100 && counters_b[2] == 68);
    reset(); size_result = 101; ProbeTrackedDeallocate(&token); assert(counters_a[1] == ~Word(0));
    reset(); assert(ProbeAllocateAfterSuccess(7) == &token); assert(counters_a[1] == 107);
    reset(); fail_malloc = true; assert(ProbeAllocateAfterSuccess(7) == 0); assert(counters_a[1] == 100);
    reset(); probe_ledger = 0; assert(ProbeAllocateAfterSuccess(7) == &token); assert(probe_fallback_counter == 107 && counter_seen_by_allocator == 100);
    reset(); probe_ledger = 0; fail_malloc = true; assert(ProbeAllocateBeforeAttempt(7) == 0); assert(probe_fallback_counter == 107 && counter_seen_by_allocator == 107);
    reset(); assert(ProbeCallocAfterSuccess(3, 7) == &token); assert(counters_a[1] == 121);
    reset(); fail_malloc = true; assert(ProbeCallocAfterSuccess(3, 7) == 0); assert(counters_a[1] == 100);
    reset(); assert(ProbeCallocAfterSuccess(0x80000000u, 2) == &token); assert(counters_a[1] == 100);
    std::puts("14 accounting candidate scenarios passed (native model; not original-game execution)");
}
