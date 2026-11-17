"""Pull function definitions out of an original-unit source file.

Original units (``src/reconstructed/racesnd.cpp`` ...) hold every function of a
retail object, exact or not. Native behaviour tests link one function against
retail, so they compile a private validation unit made of the unit's
``#include`` lines and the definitions they test. This module builds that
text; it never edits the unit.
"""
from __future__ import annotations

import re
from pathlib import Path

_INCLUDE = re.compile(r'^\s*#\s*include\s+([<"])([^>"]+)[>"]', re.M)


def _skip_literal(text: str, i: int) -> int:
    """Return the index after the literal or comment starting at ``i`` (or ``i``)."""
    if text.startswith('//', i):
        j = text.find('\n', i)
        return len(text) if j < 0 else j
    if text.startswith('/*', i):
        j = text.find('*/', i + 2)
        return len(text) if j < 0 else j + 2
    if text[i] in '"\'':
        quote, j = text[i], i + 1
        while j < len(text) and text[j] != quote:
            j += 2 if text[j] == '\\' else 1
        return j + 1
    return i


def function_definition(text: str, qualified: str) -> str:
    """The definition of ``Class::Name`` or ``Name``, from its line to the closing brace.

    The declarator must start a line (``void RaceSound::RefillStream() {``) and
    be the only definition with that qualified name; overloads are rejected.
    """
    name = re.escape(qualified)
    pattern = re.compile(r'^[^\s#/][^;{}()\n]*?\b' + name + r'\s*\([^;{}]*\)[^;{}]*\{', re.M)
    matches = list(pattern.finditer(text))
    if len(matches) != 1:
        raise ValueError(f'{qualified}: expected one definition, found {len(matches)}')
    match = matches[0]
    i, depth = match.end(), 1
    while depth:
        if i >= len(text):
            raise ValueError(f'{qualified}: unbalanced braces')
        j = _skip_literal(text, i)
        if j != i:
            i = j
            continue
        depth += (text[i] == '{') - (text[i] == '}')
        i += 1
    return text[match.start():i]


def include_lines(path: Path) -> list[str]:
    """The unit's ``#include`` lines, quoted paths made absolute (POSIX)."""
    text = path.read_text(encoding='latin1')
    lines = []
    for match in _INCLUDE.finditer(text):
        kind, name = match.groups()
        if kind == '"' and (path.parent / name).is_file():
            lines.append('#include "%s"' % (path.parent / name).resolve().as_posix())
        elif kind == '"':
            lines.append('#include "%s"' % name)
        else:
            lines.append('#include <%s>' % name)
    return lines


def standalone_source(path: Path, qualified_names, prelude: str = '') -> str:
    """A validation unit: the unit's includes, ``prelude``, then the named definitions."""
    text = path.read_text(encoding='latin1')
    parts = include_lines(path)
    if prelude:
        parts.append(prelude.rstrip('\n'))
    parts += [function_definition(text, name) for name in qualified_names]
    return '\n'.join(parts) + '\n'
