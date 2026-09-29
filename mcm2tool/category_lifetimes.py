"""Normal-CFG select/restore witnesses. Not a proof of global category state.

Call bodies, exceptions and indirect jumps are not modeled. Every result says
whether a local restore call is seen, not whether a category is actually active.
"""
from __future__ import annotations
from collections import deque
from .allocation import Insn, direct_call_target, hx
from .category_contexts import successors


def normal_exit_paths(body: list[Insn], select: int, restore: int, limit=32768) -> dict:
    if not body or select == restore or limit < 1:
        raise ValueError('nonempty body, distinct boundaries and positive limit required')
    instructions = {i.va: i for i in body}
    if len(instructions) != len(body):
        raise ValueError('duplicate instruction address')
    ordered = sorted(body, key=lambda i: i.va)
    for a, b in zip(ordered, ordered[1:]):
        if a.end != b.va:
            raise ValueError('body must be a contiguous reviewed instruction range')
    # A finite abstract state records local boundary history, NOT runtime globals.
    pending = deque([(ordered[0].va, 'not_selected', [])])
    seen, exits, stops = set(), [], []
    while pending:
        va, state, witness = pending.popleft()
        if (va, state) in seen:
            continue
        if len(seen) >= limit:
            stops.append({'va': hx(va), 'reason': 'state_limit'})
            break
        seen.add((va, state))
        ins = instructions.get(va)
        if ins is None:
            stops.append({'va': hx(va), 'reason': 'outside_reviewed_range', 'state': state})
            continue
        witness = witness + [va]
        target = direct_call_target(ins)
        if target == select:
            state = 'selected_without_local_restore'
        elif target == restore:
            state = ('local_restore_after_select' if state in
                     ('selected_without_local_restore', 'local_restore_after_select')
                     else 'local_restore_without_select')
        if ins.mnemonic.startswith('ret') or (ins.mnemonic in ('rep','repz') and ins.operands.startswith('ret')):
            exits.append({'return_va': hx(va), 'local_history': state,
                          'witness_instructions': [hx(v) for v in witness],
                          'witness_calls': [{'site_va': hx(v), 'target_va': hx(direct_call_target(instructions[v]))}
                                            for v in witness if direct_call_target(instructions[v]) is not None]})
            continue
        next_vas, reason = successors(ins)
        if reason:
            stops.append({'va': hx(va), 'reason': reason, 'state': state})
        pending.extend((n, state, witness) for n in next_vas)
    return {'normal_returns': sorted(exits, key=lambda r:(r['return_va'],r['local_history'])),
            'unresolved_or_abnormal_stops': stops,
            'visited_abstract_states': len(seen),
            'exception_edges_modeled': False, 'callee_effects_modeled': False,
            'restore_argument_equivalence_proven_by_this_analysis': False,
            'branch_predicates_solved': False}
