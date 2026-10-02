#!/usr/bin/env python3
from __future__ import annotations
import argparse,json
from pathlib import Path
from mcm2tool.pe import PEImage
from mcm2tool.coff import CoffObject, relocation_mask
from mcm2tool.resolved_match import match_object, RelocationError

def compare(pe: PEImage, obj: CoffObject, symbol: str, target_va: int, target_size: int, bindings_path: Path | None = None) -> dict:
 """Masked comparison, plus the strict (relocation-resolved) one when bindings are given."""
 sym=obj.find_symbol(symbol);cand,csize,rels=obj.symbol_extent(sym)
 # Use requested target function size. A COMDAT section normally gives exact candidate size.
 target=pe.bytes_at_va(target_va,target_size)
 # Function extents come from COFF metadata, never the requested target size.
 pad=0
 mask=relocation_mask(obj,sym,len(cand),rels)
 n=min(len(target),len(cand));mism=[];comparable=0;matching=0
 for i in range(n):
  if mask[i]: continue
  comparable+=1
  if target[i]==cand[i]:matching+=1
  elif len(mism)<64:mism.append({'offset':i,'target':target[i],'candidate':cand[i]})
 exact=(len(target)==len(cand) and matching==comparable)
 result={'target_va':f'0x{target_va:08x}','target_size':len(target),'candidate_size':len(cand),'symbol':sym.name,'relocations_masked':sum(mask),'comparable_bytes':comparable,'matching_bytes':matching,'match_percent':round(100*matching/comparable,4) if comparable else 100.0,'exact_after_relocation_mask':exact,'alignment_padding_bytes':pad,'mismatches':mism}
 result['extent_source']=sym.extent_source
 if bindings_path:
  values=json.loads(Path(bindings_path).read_text())
  bindings={name:int(value,0) if isinstance(value,str) else value for name,value in values.items()}
  try: strict=match_object(obj,sym.name,target_va,target,bindings)
  except RelocationError as exc: strict={'strict_exact':False,'ignored_bytes':0,'error':str(exc)}
  result.update({'strict_exact':strict['strict_exact'],'strict_match':strict})
 return result

def main():
 ap=argparse.ArgumentParser(description='Compare a COFF function with mcm2.exe; --bindings resolves relocation fields instead of masking them.')
 ap.add_argument('--exe',required=True);ap.add_argument('--target-va',required=True,type=lambda x:int(x,0));ap.add_argument('--target-size',required=True,type=lambda x:int(x,0))
 ap.add_argument('--obj',required=True);ap.add_argument('--symbol',required=True);ap.add_argument('--json',action='store_true')
 ap.add_argument('--bindings',type=Path,help='JSON symbol-to-VA map; resolve relocations and require every byte to match')
 a=ap.parse_args()
 result=compare(PEImage(a.exe),CoffObject(a.obj),a.symbol,a.target_va,a.target_size,a.bindings)
 exact=result['strict_exact'] if a.bindings else result['exact_after_relocation_mask']
 if a.json:print(json.dumps(result,indent=2))
 else:
  name=result['symbol'];tsize=result['target_size'];csize=result['candidate_size']
  if a.bindings:
   print(f"{name}: target={tsize} candidate={csize} strict_exact={exact} ignored_bytes=0")
   if result['strict_match'].get('error'): print(result['strict_match']['error'])
  else:
   print(f"{name}: target={tsize} candidate={csize} comparable={result['comparable_bytes']} match={result['match_percent']:.2f}% exact={exact}")
  for d in result['mismatches'][:20]: print(f"  +0x{d['offset']:x}: target {d['target']:02x} candidate {d['candidate']:02x}")
 raise SystemExit(0 if exact else 1)
if __name__=='__main__':main()
