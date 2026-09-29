#ifndef MCM2_ECOSYSTEM_PASS_H
#define MCM2_ECOSYSTEM_PASS_H

// A recovered prefix, not an original class name or complete object size.
// No vtable is invented: bytes 0..7 remain opaque.
namespace mcm2_eco {
typedef unsigned int Word;
struct RecordPrefix {
    unsigned char unknown_000[8];
    unsigned char group_008;
    unsigned char unknown_009[3];
    unsigned short coordinate_00c, coordinate_00e, coordinate_010;
    unsigned char definition_012, state_013, parameter_014, parameter_015;
    unsigned char flags_016, unknown_017;
    void Evaluate(int* mode, int* state); // target 0x00456890
    void Apply(int mode, int state);     // target 0x00456a10
};
struct DefinitionPrefix {
    unsigned char unknown_000[0x1d8];
    int limit_1d8;
    float Metric15(unsigned char);       // target 0x00455ff0
    float Metric14(unsigned char);       // target 0x00455f90
};
struct FramePrefix {
    unsigned char unknown_000[0x1c0];
    float scale_1c0;
};
struct Position { float x, y, z; };

// This is the whole TOP-LEVEL normal body at 0x0045aad0, not the callees.
// Ops retains each external call as a distinct boundary; no opaque loop remains.
// Real models accumulator intent only; x87 precision/control-word equivalence
// remains unproven. Explicit float conversions mark observed memory stores.
#ifndef MCM2_ECO_INLINE
#define MCM2_ECO_INLINE inline
#endif
template<class View, class Ops>
MCM2_ECO_INLINE int Run(View& self, Ops& ops) {
    typedef typename Ops::Real Real;
    // E00: gates, category, timer. Preserve the selected early return.
    if (!ops.enabled()) return 1;
    int previous = ops.select();
    Word before = ops.sample();
    if (!self.ready()) return 1;

    // E01: optional age/accounting object. Re-read it after callbacks.
    if (self.scratch()) {
        int reclaimable;
        Word total = ops.totals(self.scratch(), &reclaimable);
        if (reclaimable > 0x40000)
            ops.trim(self.scratch(), total - static_cast<Word>(reclaimable));
        ops.advance(self.scratch());
    }
    // E02: publish context, scale conversion, reset both work-list counts.
    ops.publish(self.frame());
    self.inverse_scale() = static_cast<float>(Real(ops.constant_65535()) / self.frame()->scale_1c0);
    Real scaled = Real(self.frame()->scale_1c0) * ops.constant_inverse();
    self.count_a() = 0;
    self.count_b() = 0;
    self.scaled_extent() = static_cast<float>(scaled);
    ops.reset_iterator();

    // E03: iterator is a callee-owned traversal, not a guessed linked-list stride.
    for (RecordPrefix* item = ops.next(); item; item = ops.next()) {
        if (item->group_008 != self.group()) continue;
        int mode = item->flags_016 & 1;
        int state = item->state_013;
        ops.evaluate(item, &mode, &state);
        DefinitionPrefix* definition = self.definition(item->definition_012);
        // E04/E05: mode zero goes through the geometry predicate.
        if (!mode) {
            float bound = static_cast<float>(ops.metric15(definition, item->parameter_015));
            Real lift = Real(ops.metric14(definition, item->parameter_014)) * ops.constant_half();
            // FCOMP/test AH,0x41 selects lift for <= OR unordered, not just <.
            if (!(Real(bound) > lift)) bound = static_cast<float>(lift);
            float scale = ops.coordinate_scale();
            Position p;
            p.x = static_cast<float>(Real(item->coordinate_00c) * scale);
            p.y = static_cast<float>(Real(item->coordinate_00e) * scale + lift);
            p.z = static_cast<float>(Real(item->coordinate_010) * scale);
            if (!ops.test_geometry(self.frame(), &p, bound, 0)) continue;
        }
        // E06: helper can mutate the record. The following tests re-read bytes.
        ops.apply(item, mode, state);
        // E07: grow by 100 pointer slots only on count == capacity.
        if (item->state_013) {
            Word capacity = self.capacity_a();
            if (self.count_a() == capacity) {
                RecordPrefix** old = self.list_a();
                RecordPrefix** next = ops.reallocate(old, capacity * 4u + 0x190u, 0x8dd);
                self.list_a() = next;
                // Preserve the retail pointer-comparison condition (NOT success).
                if (next != old) self.capacity_a() += 100;
            }
            self.list_a()[self.count_a()] = item; // no invented failure/null guard
            ++self.count_a();
        }
        // E08: signed comparison of the updated low flag bit against definition.
        if (static_cast<int>(item->flags_016 & 1) < definition->limit_1d8) {
            Word capacity = self.capacity_b();
            if (self.count_b() == capacity) {
                RecordPrefix** old = self.list_b();
                RecordPrefix** next = ops.reallocate(old, capacity * 4u + 0x50u, 0x8ea);
                self.list_b() = next;
                if (next != old) self.capacity_b() += 20;
            }
            self.list_b()[self.count_b()] = item;
            ++self.count_b();
        }
    }
    // E09: elapsed dword subtraction and normal restore.
    Word after = ops.sample();
    ops.elapsed(after - before);
    ops.restore(previous);
    return 1;
}
} // namespace mcm2_eco
#endif
