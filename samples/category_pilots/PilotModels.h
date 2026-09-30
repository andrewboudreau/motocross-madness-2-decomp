#ifndef MCM2_CATEGORY_PILOT_MODELS_H
#define MCM2_CATEGORY_PILOT_MODELS_H

// These are testable behavior models, NOT original declarations or byte matches.
// View/operations boundaries keep unknown types and x86 ABI out of host tests.
// Terrain: complete reviewed NORMAL cleanup body, excluding vptr/EH machinery.
// EcoSystem: scope shell only; its large iteration/math body is explicitly opaque.

namespace mcm2_pilots {

template<class View, class Operations>
void TerrainNormalCleanup(View& self, Operations& ops) {
    int previous = ops.select_terrain();
    if (self.table_cb8()) {
        for (int i = 0; i < self.count_cbc(); ++i)
            ops.release_ecx_slot2(self.table_cb8()[i]); // Retail has NO per-item null guard.
        ops.free_debug(self.table_cb8(), 0x4b3); // Source path: Terrain.cpp
    }
    if (self.member_c3c()) ops.release_ecx_slot2(self.member_c3c());
    if (self.member_030()) ops.release_ecx_slot2(self.member_030());
    for (int i = 0; i < self.count_540(); ++i) {
        void* item = self.inline_item_544(i);
        if (item) ops.release_ecx_slot2(item);
    }
    if (self.member_044()) {
        ops.prepare_044(self.member_044(), 1);
        // Retail reloads the member after preparation and checks it again.
        if (self.member_044()) ops.delete_slot0(self.member_044(), 1);
    }
    if (self.member_034()) {
        ops.release_stack_slot2(self.member_034());
        self.clear_034(); // Unlike other members, this pointer is explicitly zeroed.
    }
    if (self.table_c14()) {
        for (int i = 0; i < self.count_c10(); ++i) {
            void* item = self.table_c14()[i];
            if (item) ops.tracked_delete(item);
        }
        ops.free_debug(self.table_c14(), 0x4d0);
    }
    void* item = self.member_c84();
    if (item) { ops.destroy_401020(item); ops.tracked_delete(item); }
    item = self.member_c88();
    if (item) { ops.destroy_401020(item); ops.tracked_delete(item); }
    ops.restore(previous);
    ops.base_cleanup(); // GameObject destructor body, AFTER category restoration.
}

template<class View, class Operations>
int EcoSystemScopeShell(View& self, Operations& ops) {
    if (!ops.global_enabled()) return 1;
    int previous = ops.select_ecosystem();
    unsigned int before = ops.sample_counter();
    // Read at this point, not before the counter helper. Do not add RAII restore.
    if (!self.member_034_nonzero()) return 1;
    ops.unreconstructed_ecosystem_body();
    unsigned int after = ops.sample_counter();
    ops.store_elapsed(after - before); // 32-bit wraparound on the target.
    ops.restore(previous);
    return 1;
}

} // namespace mcm2_pilots
#endif
