#!/usr/bin/env python3
from __future__ import annotations
import argparse
import hashlib
import json
import os
from pathlib import Path
import struct
import sys
import tempfile

ROOT = Path(__file__).resolve().parents[1]
sys.path.insert(0, str(ROOT))
from mcm2tool.coff import CoffObject, relocation_mask
from mcm2tool.coff_archive import get_archive_member
from mcm2tool.pe import PEImage
from mcm2tool.resolved_match import match_object
from mcm2tool.toolchain import (
    find_child_ci,
    fingerprint_toolchain,
    infer_vc98_root,
)

EXPECTED_EXE_SHA = (
    '31fde4cc686a5ee89ef9095b90235325b195596867ecacefe511263e1509b874'
)

STRICT = [
    ('build\\intel\\mt_obj\\malloc.obj', '_malloc', 0x0053789D),
    ('build\\intel\\mt_obj\\malloc.obj', '__nh_malloc', 0x005378AF),
    ('build\\intel\\mt_obj\\malloc.obj', '__heap_alloc', 0x005378DB),
    ('build\\intel\\mt_obj\\free.obj', '_free', 0x00537929),
    ('build\\intel\\mt_obj\\msize.obj', '__msize', 0x005351F0),
]
SUPPORT = [
    (
        'build\\intel\\mt_obj\\sbheap.obj',
        '___sbh_find_block',
        0x0053B67F,
    ),
    (
        'build\\intel\\mt_obj\\sbheap.obj',
        '___sbh_free_block',
        0x0053B6AA,
    ),
    (
        'build\\intel\\mt_obj\\sbheap.obj',
        '___sbh_alloc_block',
        0x0053B9D5,
    ),
    ('build\\intel\\mt_obj\\handler.obj', '__callnewh', 0x00544E78),
]
BINDINGS = {
    '__newmode': 0x0068AEE8,
    '__nh_malloc': 0x005378AF,
    '__heap_alloc': 0x005378DB,
    '__callnewh': 0x00544E78,
    '___sbh_threshold': 0x005763F4,
    '__lock': 0x0053821F,
    '___sbh_alloc_block': 0x0053B9D5,
    '__unlock': 0x00538280,
    '__crtheap': 0x0068B460,
    '__imp__HeapAlloc@12': 0x0055019C,
    '___sbh_find_block': 0x0053B67F,
    '___sbh_free_block': 0x0053B6AA,
    '__imp__HeapFree@12': 0x00550198,
    '__imp__HeapSize@12': 0x00550238,
}


def sha(path: Path) -> str:
    h = hashlib.sha256()
    with path.open('rb') as f:
        for chunk in iter(lambda: f.read(1024 * 1024), b''):
            h.update(chunk)
    return h.hexdigest()


def rich_records(data: bytes) -> list[dict]:
    pe = struct.unpack_from('<I', data, 0x3C)[0]
    rich = data.rfind(b'Rich', 0, pe)
    if rich < 0 or rich + 8 > len(data):
        raise ValueError('Rich header not found')
    key = struct.unpack_from('<I', data, rich + 4)[0]
    dans = None
    for off in range(rich - 4, 0x3F, -4):
        if struct.unpack_from('<I', data, off)[0] ^ key == 0x536E6144:
            dans = off
            break
    if dans is None:
        raise ValueError('Rich DanS marker not found')
    words = [
        struct.unpack_from('<I', data, off)[0] ^ key
        for off in range(dans, rich, 4)
    ]
    if words[:4] != [0x536E6144, 0, 0, 0] or (len(words) - 4) % 2:
        raise ValueError('malformed Rich header')
    rows = []
    for i in range(4, len(words), 2):
        compid, count = words[i], words[i + 1]
        rows.append(
            {
                'product_id': compid >> 16,
                'build': compid & 0xFFFF,
                'count': count,
            }
        )
    return rows


def object_from_member(
    library: Path, member: str, temp: Path
) -> CoffObject:
    data = get_archive_member(library, member)
    path = temp / Path(member.replace('\\', '/')).name
    if not path.exists() or path.read_bytes() != data:
        path.write_bytes(data)
    return CoffObject(path)


def masked_match(
    pe: PEImage, obj: CoffObject, symbol: str, target_va: int
) -> dict:
    sym = obj.find_symbol(symbol)
    raw, size, relocs = obj.symbol_extent(sym)
    retail = pe.bytes_at_va(target_va, size)
    mask = relocation_mask(obj, sym, size, relocs)
    compared = [i for i in range(size) if not mask[i]]
    mismatches = [i for i in compared if raw[i] != retail[i]]
    return {
        'symbol': sym.name,
        'target_va': f'0x{target_va:08x}',
        'function_size': size,
        'relocations': len(relocs),
        'relocation_bytes_excluded': sum(mask),
        'compared_positions': len(compared),
        'matching_positions': len(compared) - len(mismatches),
        'non_relocated_exact': not mismatches,
        'mismatches': mismatches[:64],
    }


def single_thread_control(
    library: Path, member: str, symbol: str, temp: Path
) -> dict:
    obj = object_from_member(library, member, temp)
    sym = obj.find_symbol(symbol)
    _, size, relocs = obj.symbol_extent(sym)
    return {
        'symbol': symbol,
        'function_size': size,
        'relocations': len(relocs),
    }


def main():
    ap = argparse.ArgumentParser(
        description=(
            'Demonstrate VC6/SP3 provenance by matching shipped LIBCMT '
            'object code directly to retail MCM2.'
        )
    )
    ap.add_argument('--vc6-root', default=os.environ.get('VC6_ROOT'))
    ap.add_argument(
        '--exe',
        type=Path,
        default=Path(os.environ.get('MCM2_EXE', 'work/game/mcm2.exe')),
    )
    ap.add_argument(
        '--out',
        type=Path,
        default=Path('work/vc6-runtime-proof.json'),
    )
    args = ap.parse_args()
    if not args.vc6_root:
        raise SystemExit('set VC6_ROOT or pass --vc6-root')
    vc6 = Path(args.vc6_root).expanduser().resolve()
    exe = args.exe.expanduser().resolve()
    if sha(exe) != EXPECTED_EXE_SHA:
        raise SystemExit('unexpected MCM2 executable hash')
    pe = PEImage(exe)

    vc98 = infer_vc98_root(vc6)
    libdir = find_child_ci(vc98, 'Lib')
    if not libdir:
        raise SystemExit('VC98/Lib not found')
    libcmt = find_child_ci(libdir, 'LIBCMT.LIB')
    libc = find_child_ci(libdir, 'LIBC.LIB')
    if not libcmt or not libc:
        raise SystemExit('LIBCMT.LIB and LIBC.LIB are required')

    expected_bundle = ROOT / 'config/private_bundle_expected.json'
    pinned = (
        json.loads(expected_bundle.read_text())
        if expected_bundle.exists()
        else {}
    )
    expected_mt = (pinned.get('core_files') or {}).get(
        'VC98/LIB/LIBCMT.LIB'
    )
    mt_sha = sha(libcmt)
    pinned_mt = not expected_mt or mt_sha == expected_mt
    if expected_mt and not pinned_mt:
        raise SystemExit('LIBCMT.LIB does not match the pinned private bundle')

    rich = rich_records(pe.data)
    rich_cpp = next(
        (
            row
            for row in rich
            if row == {'product_id': 11, 'build': 8447, 'count': 197}
        ),
        None,
    )
    rich_link = next(
        (
            row
            for row in rich
            if row == {'product_id': 4, 'build': 8447, 'count': 2}
        ),
        None,
    )
    fp = fingerprint_toolchain(vc6)
    by_name = {f['name'].upper(): f for f in fp['files']}
    c2_versions = (by_name.get('C2.DLL') or {}).get(
        'version_strings', []
    )
    c1xx_versions = (by_name.get('C1XX.DLL') or {}).get(
        'version_strings', []
    )
    link_versions = (by_name.get('LINK.EXE') or {}).get(
        'version_strings', []
    )
    generation_match = (
        any(
            v.startswith(('12.0.8447', '12.00.8447'))
            for v in c2_versions
        )
        and any(
            v.startswith(('12.0.8472', '12.00.8472'))
            for v in c1xx_versions
        )
        and any(
            v.startswith(('6.0.8447', '6.00.8447'))
            for v in link_versions
        )
    )

    strict_rows, support_rows = [], []
    with tempfile.TemporaryDirectory(prefix='mcm2-vc6-runtime-') as td:
        temp = Path(td)
        cache = {}
        for member, symbol, va in STRICT:
            key = member.casefold()
            obj = cache.setdefault(
                key, object_from_member(libcmt, member, temp)
            )
            sym = obj.find_symbol(symbol)
            _, size, _ = obj.symbol_extent(sym)
            row = match_object(
                obj, symbol, va, pe.bytes_at_va(va, size), BINDINGS
            )
            row['archive_member'] = member
            strict_rows.append(row)
        for member, symbol, va in SUPPORT:
            key = member.casefold()
            obj = cache.setdefault(
                key, object_from_member(libcmt, member, temp)
            )
            row = masked_match(pe, obj, symbol, va)
            row['archive_member'] = member
            support_rows.append(row)
        controls = {
            'single_thread_free': single_thread_control(
                libc,
                'build\\intel\\st_obj\\free.obj',
                '_free',
                temp,
            ),
            'single_thread_msize': single_thread_control(
                libc,
                'build\\intel\\st_obj\\msize.obj',
                '__msize',
                temp,
            ),
        }

    strict_exact = all(
        row['strict_exact'] and row['ignored_bytes'] == 0
        for row in strict_rows
    )
    support_exact = all(
        row['non_relocated_exact'] for row in support_rows
    )
    resolved_bytes = sum(row['retail_size'] for row in strict_rows)
    resolved_relocs = sum(
        len(row['relocations_applied']) for row in strict_rows
    )
    support_compared = sum(
        row['compared_positions'] for row in support_rows
    )
    support_body = sum(row['function_size'] for row in support_rows)
    controls_distinguish_mt = (
        controls['single_thread_free']['function_size'] != 72
        and controls['single_thread_msize']['function_size'] != 69
    )
    verdict = bool(
        rich_cpp
        and rich_link
        and generation_match
        and pinned_mt
        and strict_exact
        and support_exact
        and controls_distinguish_mt
    )

    report = {
        'input_sha256': EXPECTED_EXE_SHA,
        'libcmt_sha256': mt_sha,
        'pinned_libcmt_match': pinned_mt,
        'rich_records': rich,
        'rich_cpp_8447': bool(rich_cpp),
        'rich_linker_8447': bool(rich_link),
        'toolchain': fp,
        'sp3_8447_generation_match': generation_match,
        'strict_resolved_matches': strict_rows,
        'supporting_nonrelocated_matches': support_rows,
        'negative_control_libc_single_thread': controls,
        'summary': {
            'strict_functions_exact': len(strict_rows),
            'strict_bytes_exact_after_relocation': resolved_bytes,
            'strict_relocations_applied': resolved_relocs,
            'support_functions_exact_nonrelocated': len(support_rows),
            'support_bytes_compared_exact': support_compared,
            'support_function_body_bytes': support_body,
            'total_directly_compared_exact_bytes': (
                resolved_bytes + support_compared
            ),
            'correct_vc6_sp3_8447_toolchain_family_demonstrated': verdict,
        },
        'scope': (
            'This demonstrates the VC6/SP3 build generation and identical '
            'shipped LIBCMT object code in MCM2. It does not by itself prove '
            'every original per-translation-unit compiler flag.'
        ),
    }
    args.out.parent.mkdir(parents=True, exist_ok=True)
    args.out.write_text(json.dumps(report, indent=2) + '\n')
    print(json.dumps(report['summary'], indent=2))
    if not verdict:
        raise SystemExit(1)


if __name__ == '__main__':
    main()
