"""Resolve generated global-load probes from freshly decoded retail operands."""
from .easy import classify_easy_bytes
from .pe import PEFormatError
from .resolved_match import RelocationError, match_object


def global_bindings(retail: bytes, target: dict) -> dict[str, int]:
    if target['kind'] != 'return_float_global':
        return {}
    pattern = classify_easy_bytes(retail)
    if (pattern is None or pattern.kind != target['kind']
            or pattern.size != len(retail) or pattern.details != target['details']):
        raise RelocationError('global-load evidence differs from the current executable')
    # generate_easy_probes.py declares exactly this external float. Bind only
    # that symbol; arbitrary/unexpected references must remain unresolved.
    va = int(target['target_va'], 16)
    return {f'?g_{va:08X}@@3MA': pattern.details['address']}


def match_easy_strict(pe, obj, target: dict) -> dict:
    va = int(target['target_va'], 16)
    try:
        retail = pe.bytes_at_va(va, int(target['target_size']))
        bindings = global_bindings(retail, target)
        for address in bindings.values():
            pe.bytes_at_va(address, 4)  # The observed float load needs file backing.
        result = match_object(obj, target['symbol_contains'], va, retail, bindings)
        result['binding_evidence'] = {
            name: {'target_va': f'0x{address:08x}',
                   'basis': 'absolute operand of the decoded retail FLD instruction'}
            for name, address in bindings.items()
        }
        return result
    except (RelocationError, PEFormatError) as exc:
        return {'strict_exact': False, 'ignored_bytes': 0, 'error': str(exc)}
