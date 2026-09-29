#!/usr/bin/env python3
from __future__ import annotations
import argparse
import json
import os
from pathlib import Path
import sys
import tempfile

ROOT = Path(__file__).resolve().parents[1]
sys.path.insert(0, str(ROOT))
from mcm2tool.private_bundle import BundleError, download, install_archive

def main():
    ap = argparse.ArgumentParser(
        description='Install the private VC6 SP3 + MCM2 bundle outside Git, with full hash verification.'
    )
    source = ap.add_mutually_exclusive_group()
    source.add_argument('--archive', type=Path, help='existing local private bundle ZIP')
    source.add_argument(
        '--url-env',
        default=None,
        help='environment variable holding private bundle URL; URL is never printed',
    )
    ap.add_argument(
        '--token-env',
        default='MCM2_PRIVATE_BUNDLE_TOKEN',
        help='optional bearer-token environment variable',
    )
    expected_path = ROOT / 'config/private_bundle_expected.json'
    default_sha = os.environ.get('MCM2_PRIVATE_BUNDLE_SHA256')
    if not default_sha and expected_path.exists():
        default_sha = json.loads(expected_path.read_text()).get('archive_sha256')
    ap.add_argument('--archive-sha256', default=default_sha)
    ap.add_argument(
        '--root',
        type=Path,
        default=Path(
            os.environ.get('MCM2_PRIVATE_ROOT', '~/.cache/mcm2-private')
        ).expanduser(),
    )
    ap.add_argument('--overwrite', action='store_true')
    ap.add_argument('--json', action='store_true')
    args = ap.parse_args()

    archive = args.archive
    temporary = None
    if archive is None:
        env_name = args.url_env or 'MCM2_PRIVATE_BUNDLE_URL'
        url = os.environ.get(env_name)
        if not url:
            raise BundleError(f'set {env_name} or pass --archive')
        temporary = tempfile.TemporaryDirectory(prefix='mcm2-private-download-')
        archive = Path(temporary.name) / 'private-inputs.zip'
        token = os.environ.get(args.token_env) if args.token_env else None
        print(f'Downloading private bundle from ${env_name} ...', file=sys.stderr)
        download(url, archive, token)

    try:
        install = install_archive(
            archive.resolve(), args.root, args.archive_sha256, args.overwrite
        )
    finally:
        if temporary:
            temporary.cleanup()

    state = {
        'private_root': str(install.root),
        'vc6_root': str(install.vc6_root),
        'exe': str(install.exe),
        'archive_sha256': install.archive_sha256,
        'payload_files': install.payload_files,
        'env': {
            'MCM2_PRIVATE_ROOT': str(install.root),
            'VC6_ROOT': str(install.vc6_root),
            'MCM2_EXE': str(install.exe),
            'WINEPREFIX': str(install.root / 'wine-vc6'),
            'WINEARCH': 'win32',
            'WINEDEBUG': '-all',
        },
    }
    if args.json:
        print(json.dumps(state, indent=2))
    else:
        print(f'Private inputs ready: {install.root}')
        print(f'VC6_ROOT={install.vc6_root}')
        print(f'MCM2_EXE={install.exe}')
        print(f'archive_sha256={install.archive_sha256}')
        print('Use: python3 tools/with_private_env.py <command> [args...]')

if __name__ == '__main__':
    try:
        main()
    except (BundleError, OSError, ValueError) as exc:
        print(f'private-bundle: {exc}', file=sys.stderr)
        raise SystemExit(2)
