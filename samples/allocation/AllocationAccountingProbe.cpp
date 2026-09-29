// Reconstructed behaviors from the supplied MCM2 build. Names below are ours,
// NOT original Rainbow symbols. No claim of an original class/TU declaration.
// The backend names encode reviewed addresses, not unproved CRT identities.
typedef unsigned int Word;
struct AllocationLedgerProbe {
    Word category_count;        // +0x00, observed count of 128-byte labels
    char* category_labels;      // +0x04
    Word* byte_counters;        // +0x08
    Word* secondary_counters;   // +0x0c, separate counter family
    Word current_category;     // +0x10
    Word unknown_14;
    Word unknown_18;
    void RestoreCategory(Word value);
};
#if defined(_M_IX86) || defined(__i386__)
typedef char ledger_size_check[sizeof(AllocationLedgerProbe) == 0x1c ? 1 : -1];
#endif
extern AllocationLedgerProbe* probe_ledger;
extern Word probe_fallback_counter;
extern "C" Word probe_size_005351F0(void*);
extern "C" void probe_free_00537929(void*);
extern "C" void* probe_malloc_0053789D(Word);
extern "C" void* probe_calloc_00537AA9(Word, Word);

// 0x004a2d90: no virtual dispatch or RTTI exists for this record.
void AllocationLedgerProbe::RestoreCategory(Word value) {
    current_category = value;
}

// 0x004a2e60 / 0x004a3060 / 0x004a30c0: same body after verifying all
// rel32 call destinations. Other signature arguments, if any, remain unknown.
// Do not cache the selected bucket across the size callback: retail reloads it.
extern "C" void ProbeTrackedDeallocate(void* block) {
    if (block) {
        if (probe_ledger && probe_ledger->byte_counters) {
            Word size = probe_size_005351F0(block);
            probe_ledger->byte_counters[probe_ledger->current_category] -= size;
        } else {
            Word size = probe_size_005351F0(block);
            probe_fallback_counter -= size;
        }
    }
    // Retail calls the backend even for null. Preserve that behavior.
    probe_free_00537929(block);
}

// 0x004a2e20: account requested bytes only after successful allocation.
extern "C" void* ProbeAllocateAfterSuccess(Word size) {
    void* block = probe_malloc_0053789D(size);
    if (block) {
        if (probe_ledger && probe_ledger->byte_counters)
            probe_ledger->byte_counters[probe_ledger->current_category] += size;
        else
            probe_fallback_counter += size;
    }
    return block;
}

// 0x004a3010: importantly different from the previous entry. Accounting is
// performed BEFORE the call, even when the backend returns null.
extern "C" void* ProbeAllocateBeforeAttempt(Word size) {
    if (probe_ledger && probe_ledger->byte_counters)
        probe_ledger->byte_counters[probe_ledger->current_category] += size;
    else
        probe_fallback_counter += size;
    return probe_malloc_0053789D(size);
}

// 0x004a2fc0: requested count*size, not a backend-reported size, is charged.
extern "C" void* ProbeCallocAfterSuccess(Word count, Word size) {
    Word total = count * size;
    void* block = probe_calloc_00537AA9(count, size);
    if (block) {
        if (probe_ledger && probe_ledger->byte_counters)
            probe_ledger->byte_counters[probe_ledger->current_category] += total;
        else
            probe_fallback_counter += total;
    }
    return block;
}
