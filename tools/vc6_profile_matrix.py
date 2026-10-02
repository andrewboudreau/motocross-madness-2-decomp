#!/usr/bin/env python3
from __future__ import annotations
import argparse
import json
import os
from pathlib import Path
import subprocess
import sys

ROOT = Path(__file__).resolve().parents[1]


def run_json(command: list[str]) -> dict:
    result = subprocess.run(
        command,
        cwd=ROOT,
        text=True,
        stdout=subprocess.PIPE,
        stderr=subprocess.STDOUT,
    )
    try:
        payload = json.loads(result.stdout)
    except json.JSONDecodeError:
        return {
            'returncode': result.returncode,
            'parsed': False,
            'error': result.stdout,
        }
    return {
        'returncode': result.returncode,
        'parsed': True,
        'data': payload,
    }


def summarize_rows(rows) -> dict:
    if not isinstance(rows, list):
        return {'total': 0, 'exact': 0, 'mean_match_percent': None}
    percents = [
        float(row['match_percent'])
        for row in rows
        if isinstance(row, dict) and row.get('match_percent') is not None
    ]
    return {
        'total': len(rows),
        'exact': sum(
            bool(row.get('strict_exact', row.get('exact_after_relocation_mask')))
            for row in rows
            if isinstance(row, dict)
        ),
        'mean_match_percent': (
            round(sum(percents) / len(percents), 4) if percents else None
        ),
    }


def summarize_calibration(rows) -> dict:
    if not isinstance(rows, list):
        return {
            'total': 0,
            'compiled': 0,
            'compile_failures': 0,
            'exact': 0,
            'mean_match_percent': None,
        }
    results = [
        row.get('result', {})
        for row in rows
        if isinstance(row, dict) and not row.get('compile_error')
    ]
    percents = [
        float(result['match_percent'])
        for result in results
        if result.get('match_percent') is not None
    ]
    return {
        'total': len(rows),
        'compiled': len(results),
        'compile_failures': sum(
            bool(row.get('compile_error'))
            for row in rows
            if isinstance(row, dict)
        ),
        'exact': sum(
            bool(result.get('strict_exact', result.get('exact_after_relocation_mask')))
            for result in results
        ),
        'mean_match_percent': (
            round(sum(percents) / len(percents), 4) if percents else None
        ),
    }


def render(report: dict) -> str:
    lines = [
        '# VC6 compile-profile matrix',
        '',
        'Every row below is produced by the authentic VC6 compiler path. No clang fallback is allowed.',
        '',
        '| Profile | Easy exact | Manual exact | Calibration exact | Calibration mean | Execution |',
        '|---|---:|---:|---:|---:|---|',
    ]
    for row in report['profiles']:
        easy = row['easy']['summary']
        manual = row['manual']['summary']
        calibration = row['calibration']['summary']
        mean = calibration['mean_match_percent']
        mean_text = '—' if mean is None else f'{mean:.4f}%'
        state = 'ok' if row['execution_ok'] else 'error'
        lines.append(
            f"| `{row['profile']}` | {easy['exact']}/{easy['total']} | "
            f"{manual['exact']}/{manual['total']} | "
            f"{calibration['exact']}/{calibration['total']} | {mean_text} | {state} |"
        )
    lines += [
        '',
        'Profiles are compiler hypotheses, not original-project settings merely because they score well. Exact code-generation matches are evidence; runtime-library linkage is independently demonstrated by the CRT proof.',
        '',
    ]
    if report['leaders']:
        lines.append(
            'Highest exact-count tuple in this run: ' +
            ', '.join(f"`{name}`" for name in report['leaders']) + '.'
        )
        lines.append('')
    return '\n'.join(lines)


def main():
    parser = argparse.ArgumentParser(
        description='Run all VC6 profile hypotheses across easy, manual and calibration probes.'
    )
    parser.add_argument('--exe', default=os.environ.get('MCM2_EXE', 'work/game/mcm2.exe'))
    parser.add_argument('--vc6-root', default=os.environ.get('VC6_ROOT'))
    parser.add_argument('--profiles', type=Path, default=ROOT / 'config/compile_profiles.json')
    parser.add_argument('--profile', action='append', help='profile to run; repeat to restrict the matrix')
    parser.add_argument('--out', type=Path, default=ROOT / 'work/vc6-profile-matrix')
    args = parser.parse_args()
    if not args.vc6_root:
        raise SystemExit('set VC6_ROOT or pass --vc6-root')

    config = json.loads(args.profiles.read_text())['profiles']
    profiles = args.profile or [name for name in config if name.startswith('vc6_')]
    missing = [name for name in profiles if name not in config]
    if missing:
        raise SystemExit('unknown profile(s): ' + ', '.join(missing))
    if not profiles:
        raise SystemExit('no VC6 profiles selected')

    rows = []
    for profile in profiles:
        base = [
            '--exe', str(args.exe),
            '--compiler', 'vc6',
            '--vc6-root', str(args.vc6_root),
            '--profile', profile,
        ]
        easy = run_json([sys.executable, 'tools/run_easy_probes.py', *base])
        manual = run_json([sys.executable, 'tools/run_samples.py', *base])
        calibration = run_json([sys.executable, 'tools/run_calibration.py', *base])
        easy_data = easy.get('data') if easy['parsed'] else []
        manual_data = manual.get('data') if manual['parsed'] else []
        calibration_data = calibration.get('data') if calibration['parsed'] else []
        rows.append({
            'profile': profile,
            'flags': config[profile],
            'execution_ok': easy['parsed'] and manual['parsed'] and calibration['parsed'],
            'easy': {'run': easy, 'summary': summarize_rows(easy_data)},
            'manual': {'run': manual, 'summary': summarize_rows(manual_data)},
            'calibration': {
                'run': calibration,
                'summary': summarize_calibration(calibration_data),
            },
        })

    scores = {
        row['profile']: (
            row['easy']['summary']['exact'],
            row['manual']['summary']['exact'],
            row['calibration']['summary']['exact'],
        )
        for row in rows
        if row['execution_ok']
    }
    best = max(scores.values()) if scores else None
    leaders = sorted(name for name, score in scores.items() if score == best) if best else []
    report = {
        'schema_version': 1,
        'compiler': 'vc6',
        'clang_fallback_allowed': False,
        'profiles': rows,
        'leaders': leaders,
        'leader_metric': ['easy_exact', 'manual_exact', 'calibration_exact'],
        'claims': {
            'winning_profile_is_original_settings': False,
            'per_translation_unit_flags_recovered': False,
        },
    }
    args.out.mkdir(parents=True, exist_ok=True)
    (args.out / 'matrix.json').write_text(json.dumps(report, indent=2) + '\n')
    (args.out / 'REPORT.md').write_text(render(report))
    print(render(report))
    if not scores:
        raise SystemExit(2)


if __name__ == '__main__':
    main()
