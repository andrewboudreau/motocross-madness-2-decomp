#!/usr/bin/env python3
from __future__ import annotations
import argparse,struct,subprocess,shutil,os
from pathlib import Path

def find_cabs(data:bytes):
 out=[]; start=0
 while True:
  off=data.find(b'MSCF',start)
  if off<0:break
  start=off+1
  if off+36>len(data):continue
  try:
   size=struct.unpack_from('<I',data,off+8)[0]; coff=struct.unpack_from('<I',data,off+16)[0]
   minor,major=data[off+24],data[off+25]; folders,files=struct.unpack_from('<HH',data,off+26)
  except struct.error:continue
  if major==1 and 0<minor<=3 and 36<=coff<size and off+size<=len(data) and folders>0 and files>0:
   out.append({'offset':off,'size':size,'folders':folders,'files':files,'version':f'{major}.{minor}'})
 return out

def ensure_helper(repo:Path)->Path|None:
 exe=repo/'work'/'cab_extract'
 if exe.exists():return exe
 cc=shutil.which('cc') or shutil.which('gcc')
 src=repo/'tools'/'cab_extract.c'
 if cc and Path('/usr/include/archive.h').exists():
  exe.parent.mkdir(parents=True,exist_ok=True)
  r=subprocess.run([cc,'-O2',str(src),'-larchive','-o',str(exe)])
  if r.returncode==0:return exe
 return None

def extract(cab:Path,out:Path,repo:Path):
 out.mkdir(parents=True,exist_ok=True)
 if shutil.which('cabextract'):
  subprocess.run(['cabextract','-q','-d',str(out),str(cab)],check=True);return
 if shutil.which('7z'):
  subprocess.run(['7z','x','-y',f'-o{out}',str(cab)],check=True,stdout=subprocess.DEVNULL);return
 helper=ensure_helper(repo)
 if helper:
  subprocess.run([str(helper),str(cab.resolve()),str(out.resolve())],check=True);return
 raise SystemExit('No CAB extractor available. Install cabextract, 7z, or libarchive development headers + a C compiler.')

def main():
 ap=argparse.ArgumentParser(description='Extract the embedded MCM2 CAB from MCM2PCG.exe')
 ap.add_argument('installer');ap.add_argument('--out',default='work/game');ap.add_argument('--cab',default='work/MCM2.CAB');a=ap.parse_args()
 repo=Path(__file__).resolve().parents[1]; installer=Path(a.installer); data=installer.read_bytes(); cabs=find_cabs(data)
 if not cabs:raise SystemExit('No valid embedded Microsoft CAB found')
 chosen=max(cabs,key=lambda x:x['size']); cab=Path(a.cab); cab.parent.mkdir(parents=True,exist_ok=True); cab.write_bytes(data[chosen['offset']:chosen['offset']+chosen['size']])
 print(f"embedded CAB offset=0x{chosen['offset']:x} size={chosen['size']} files={chosen['files']} -> {cab}")
 extract(cab,Path(a.out),repo)
 exe=Path(a.out)/'mcm2.exe'
 if not exe.exists():raise SystemExit('CAB extracted, but mcm2.exe was not found')
 print(f'mcm2.exe: {exe}')
if __name__=='__main__':main()
