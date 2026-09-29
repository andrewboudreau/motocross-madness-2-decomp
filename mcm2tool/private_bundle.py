from __future__ import annotations

from dataclasses import dataclass
import hashlib
import json
from pathlib import Path, PurePosixPath
import shutil
import stat
import tempfile
import urllib.request
import zipfile

MANIFEST_PATH = "work/private-inputs/SHA256SUMS.json"
PROVENANCE_PATH = "work/private-inputs/PROVENANCE.json"
EXPECTED_GAME_SHA256 = "31fde4cc686a5ee89ef9095b90235325b195596867ecacefe511263e1509b874"
REQUIRED_FILES = (
    "toolchains/vc6sp3/VC98/BIN/CL.EXE",
    "toolchains/vc6sp3/VC98/BIN/C1.DLL",
    "toolchains/vc6sp3/VC98/BIN/C1XX.DLL",
    "toolchains/vc6sp3/VC98/BIN/C2.DLL",
    "toolchains/vc6sp3/VC98/BIN/LINK.EXE",
    "toolchains/vc6sp3/VC98/BIN/MSPDB60.DLL",
    "toolchains/vc6sp3/VC98/INCLUDE/STDIO.H",
    "toolchains/vc6sp3/VC98/LIB/LIBC.LIB",
    "toolchains/vc6sp3/VC98/LIB/LIBCMT.LIB",
    "work/game/mcm2.exe",
    PROVENANCE_PATH,
    MANIFEST_PATH,
)
MAX_FILES = 10000
MAX_UNCOMPRESSED_BYTES = 512 * 1024 * 1024


class BundleError(ValueError):
    pass


@dataclass(frozen=True)
class BundleInstall:
    root: Path
    vc6_root: Path
    exe: Path
    provenance: Path
    archive_sha256: str
    payload_files: int


def sha256_file(path: Path) -> str:
    h = hashlib.sha256()
    with path.open("rb") as f:
        for chunk in iter(lambda: f.read(1024 * 1024), b""):
            h.update(chunk)
    return h.hexdigest()


def _normalized_member(name: str) -> PurePosixPath:
    # ZIP uses forward slash regardless of host. Reject Windows drive/UNC forms too.
    if not name or "\\" in name or name.startswith(("/", "\\")):
        raise BundleError(f"unsafe archive member: {name!r}")
    path = PurePosixPath(name)
    if path.is_absolute() or any(part in ("", ".", "..") for part in path.parts):
        raise BundleError(f"unsafe archive member: {name!r}")
    if path.parts and ":" in path.parts[0]:
        raise BundleError(f"unsafe archive member: {name!r}")
    return path


def inspect_zip(path: Path) -> tuple[zipfile.ZipFile, dict[str, zipfile.ZipInfo]]:
    zf = zipfile.ZipFile(path)
    infos: dict[str, zipfile.ZipInfo] = {}
    total = 0
    for info in zf.infolist():
        member = _normalized_member(info.filename)
        canonical = member.as_posix()
        if canonical in infos:
            zf.close()
            raise BundleError(f"duplicate archive member: {canonical}")
        mode = (info.external_attr >> 16) & 0xFFFF
        if stat.S_ISLNK(mode):
            zf.close()
            raise BundleError(f"symlink archive member refused: {canonical}")
        if not info.is_dir():
            total += info.file_size
            if total > MAX_UNCOMPRESSED_BYTES:
                zf.close()
                raise BundleError("archive exceeds uncompressed size limit")
        infos[canonical] = info
        if len(infos) > MAX_FILES:
            zf.close()
            raise BundleError("archive contains too many files")
    return zf, infos


def verify_archive(path: Path, expected_archive_sha256: str | None = None) -> dict:
    archive_sha = sha256_file(path)
    if expected_archive_sha256 and archive_sha.casefold() != expected_archive_sha256.casefold():
        raise BundleError(f"archive SHA-256 mismatch: got {archive_sha}")
    zf, infos = inspect_zip(path)
    try:
        missing = [name for name in REQUIRED_FILES if name not in infos]
        if missing:
            raise BundleError("required files missing: " + ", ".join(missing))
        manifest = json.loads(zf.read(MANIFEST_PATH))
        if not isinstance(manifest, dict) or not manifest:
            raise BundleError("invalid SHA256SUMS.json")
        payload = {name for name, info in infos.items() if not info.is_dir() and name != MANIFEST_PATH}
        if set(manifest) != payload:
            missing_manifest = sorted(payload - set(manifest))
            phantom = sorted(set(manifest) - payload)
            raise BundleError(f"manifest/archive membership differs; unlisted={missing_manifest[:5]} missing={phantom[:5]}")
        for name, expected in sorted(manifest.items()):
            if not isinstance(expected, str) or not len(expected) == 64:
                raise BundleError(f"invalid manifest hash for {name}")
            got = hashlib.sha256(zf.read(name)).hexdigest()
            if got.casefold() != expected.casefold():
                raise BundleError(f"payload SHA-256 mismatch: {name}")
        game_sha = hashlib.sha256(zf.read("work/game/mcm2.exe")).hexdigest()
        if game_sha != EXPECTED_GAME_SHA256:
            raise BundleError(f"unexpected mcm2.exe SHA-256: {game_sha}")
        provenance = json.loads(zf.read(PROVENANCE_PATH))
        if provenance.get("target_sha256") != game_sha:
            raise BundleError("PROVENANCE.json target hash does not match game payload")
        return {
            "archive_sha256": archive_sha,
            "payload_files": len(manifest),
            "target_sha256": game_sha,
            "provenance": provenance,
        }
    finally:
        zf.close()


def download(url: str, destination: Path, token: str | None = None) -> None:
    destination.parent.mkdir(parents=True, exist_ok=True)
    headers = {"User-Agent": "mcm2-decomp-private-bootstrap/1"}
    if token:
        headers["Authorization"] = f"Bearer {token}"
    request = urllib.request.Request(url, headers=headers)
    with urllib.request.urlopen(request, timeout=120) as response, destination.open("wb") as out:
        shutil.copyfileobj(response, out, length=1024 * 1024)


def install_archive(archive: Path, root: Path, expected_archive_sha256: str | None = None,
                    overwrite: bool = False) -> BundleInstall:
    result = verify_archive(archive, expected_archive_sha256)
    root = root.expanduser().resolve()
    marker = root / "private-inputs-state.json"
    if root.exists() and any(root.iterdir()):
        if marker.exists() and not overwrite:
            current = json.loads(marker.read_text())
            if current.get("archive_sha256") == result["archive_sha256"]:
                return BundleInstall(root, root / "toolchains/vc6sp3", root / "work/game/mcm2.exe",
                                     root / PROVENANCE_PATH, result["archive_sha256"], result["payload_files"])
        if not overwrite:
            raise BundleError(f"destination is not empty: {root}; pass overwrite=True to replace it")
    root.parent.mkdir(parents=True, exist_ok=True)
    with tempfile.TemporaryDirectory(prefix="mcm2-private-install-", dir=root.parent) as td:
        stage = Path(td) / "payload"
        stage.mkdir()
        zf, infos = inspect_zip(archive)
        try:
            for name, info in infos.items():
                if info.is_dir():
                    continue
                dest = stage.joinpath(*PurePosixPath(name).parts)
                dest.parent.mkdir(parents=True, exist_ok=True)
                with zf.open(info) as src, dest.open("wb") as out:
                    shutil.copyfileobj(src, out, length=1024 * 1024)
        finally:
            zf.close()
        # Recheck extracted bytes; never trust extraction alone.
        manifest = json.loads((stage / MANIFEST_PATH).read_text())
        for name, expected in manifest.items():
            if sha256_file(stage / name) != expected:
                raise BundleError(f"extracted payload hash mismatch: {name}")
        state = {
            "schema_version": 1,
            "archive_sha256": result["archive_sha256"],
            "payload_files": result["payload_files"],
            "target_sha256": result["target_sha256"],
            "vc6_root": str((root / "toolchains/vc6sp3").resolve()),
            "exe": str((root / "work/game/mcm2.exe").resolve()),
            "provenance": result["provenance"],
        }
        (stage / "private-inputs-state.json").write_text(json.dumps(state, indent=2) + "\n")
        if root.exists():
            shutil.rmtree(root)
        stage.replace(root)
    return BundleInstall(root, root / "toolchains/vc6sp3", root / "work/game/mcm2.exe",
                         root / PROVENANCE_PATH, result["archive_sha256"], result["payload_files"])
