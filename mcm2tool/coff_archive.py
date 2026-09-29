from __future__ import annotations
from dataclasses import dataclass
from pathlib import Path

ARCHIVE_MAGIC = b'!<arch>\n'


class CoffArchiveError(ValueError):
    pass


@dataclass(frozen=True)
class ArchiveMember:
    name: str
    data: bytes


def _normalize(name: str) -> str:
    return name.replace('\\', '/').rstrip('/').casefold()


def _long_name(table: bytes, offset: int) -> str:
    if offset < 0 or offset >= len(table):
        raise CoffArchiveError(f'bad long-name offset: {offset}')
    ends = [
        p for p in (
            table.find(b'/\n', offset),
            table.find(b'\0', offset),
            table.find(b'\n', offset),
        ) if p >= 0
    ]
    end = min(ends) if ends else len(table)
    return table[offset:end].decode('utf-8', 'replace')


def read_archive(path: str | Path) -> list[ArchiveMember]:
    data = Path(path).read_bytes()
    if not data.startswith(ARCHIVE_MAGIC):
        raise CoffArchiveError('not a COFF archive')
    offset = len(ARCHIVE_MAGIC)
    long_names = b''
    out: list[ArchiveMember] = []
    while offset < len(data):
        if offset + 60 > len(data):
            raise CoffArchiveError('truncated archive member header')
        header = data[offset:offset + 60]
        if header[58:60] != b'\x60\n':
            raise CoffArchiveError(
                f'invalid archive member header at {offset:#x}'
            )
        raw_name = header[:16].decode('ascii', 'replace').rstrip()
        try:
            size = int(header[48:58].decode('ascii').strip())
        except ValueError as exc:
            raise CoffArchiveError('invalid archive member size') from exc
        start, end = offset + 60, offset + 60 + size
        if end > len(data):
            raise CoffArchiveError('truncated archive member body')
        body = data[start:end]
        if raw_name == '//':
            long_names = body
        elif raw_name not in ('/', '/SYM64/'):
            if raw_name.startswith('/') and raw_name[1:].isdigit():
                name = _long_name(long_names, int(raw_name[1:]))
            else:
                name = raw_name[:-1] if raw_name.endswith('/') else raw_name
            out.append(ArchiveMember(name, body))
        offset = end + (end & 1)
    return out


def get_archive_member(path: str | Path, name: str) -> bytes:
    wanted = _normalize(name)
    hits = [m for m in read_archive(path) if _normalize(m.name) == wanted]
    if len(hits) != 1:
        raise CoffArchiveError(
            f'expected one archive member {name!r}; found {len(hits)}'
        )
    return hits[0].data
