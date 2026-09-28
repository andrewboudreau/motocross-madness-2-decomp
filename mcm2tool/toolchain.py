from __future__ import annotations
import hashlib, os, re
from pathlib import Path
from typing import Iterable

_VERSION_RE = re.compile(rb'(?<!\d)(\d{1,2}\.\d{1,2}\.\d{3,5}(?:\.\d{1,5})?)(?!\d)')

def sha256_file(path: Path) -> str:
    h = hashlib.sha256()
    with path.open('rb') as f:
        for chunk in iter(lambda: f.read(1024 * 1024), b''):
            h.update(chunk)
    return h.hexdigest()

def find_child_ci(parent: Path, name: str) -> Path | None:
    if not parent.is_dir(): return None
    wanted = name.casefold()
    for p in parent.iterdir():
        if p.name.casefold() == wanted: return p
    return None

def find_vc6_bin(root: Path) -> Path:
    root = root.resolve(); candidates=[]
    vc98=find_child_ci(root,'VC98')
    if vc98:
        b=find_child_ci(vc98,'Bin')
        if b: candidates.append(b)
    b=find_child_ci(root,'Bin')
    if b: candidates.append(b)
    if root.name.casefold()=='bin': candidates.append(root)
    candidates.append(root)
    for p in candidates:
        if find_child_ci(p,'cl.exe'): return p
    for cl in root.rglob('*'):
        if cl.is_file() and cl.name.casefold()=='cl.exe' and cl.parent.name.casefold()=='bin' and cl.parent.parent.name.casefold()=='vc98': return cl.parent
    raise FileNotFoundError(f'VC6 cl.exe not found below {root}')

def infer_vc98_root(root: Path) -> Path:
    b=find_vc6_bin(root)
    if b.parent.name.casefold()=='vc98': return b.parent
    if root.name.casefold()=='vc98': return root.resolve()
    return root.resolve()

def version_strings(path: Path) -> list[str]:
    data=path.read_bytes(); vals=set(m.group(1).decode('ascii','ignore') for m in _VERSION_RE.finditer(data))
    try:
        text16=data.decode('utf-16le','ignore').encode('ascii','ignore'); vals.update(m.group(1).decode('ascii','ignore') for m in _VERSION_RE.finditer(text16))
    except Exception: pass
    return sorted(vals)

def important_tool_files(root: Path) -> list[Path]:
    b=find_vc6_bin(root); names=['CL.EXE','C1.DLL','C1XX.DLL','C2.DLL','LINK.EXE','LIB.EXE','CVTRES.EXE','NMAKE.EXE','MSPDB60.DLL']; out=[]
    for n in names:
        p=find_child_ci(b,n)
        if p: out.append(p)
    return out

def fingerprint_toolchain(root: Path) -> dict:
    b=find_vc6_bin(root); vc98=infer_vc98_root(root); files=[]
    for p in important_tool_files(root):
        st=p.stat(); files.append({'name':p.name,'relative_path':str(p.relative_to(root.resolve())) if p.is_relative_to(root.resolve()) else str(p),'size':st.st_size,'sha256':sha256_file(p),'version_strings':version_strings(p)})
    include=find_child_ci(vc98,'Include'); lib=find_child_ci(vc98,'Lib')
    return {'root':str(root.resolve()),'vc98_root':str(vc98),'bin':str(b),'has_include':bool(include and include.is_dir()),'has_lib':bool(lib and lib.is_dir()),'files':files}
