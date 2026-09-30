#!/usr/bin/env python3
from __future__ import annotations
import argparse,json
from mcm2tool.pe import PEImage
from mcm2tool.coff import CoffObject, relocation_mask

def hx(b):return ' '.join(f'{x:02x}' for x in b)
def main():
 ap=argparse.ArgumentParser(description='Compare a compiled COFF function with bytes in mcm2.exe; candidate relocation fields are ignored.')
 ap.add_argument('--exe',required=True);ap.add_argument('--target-va',required=True,type=lambda x:int(x,0));ap.add_argument('--target-size',required=True,type=lambda x:int(x,0))
 ap.add_argument('--obj',required=True);ap.add_argument('--symbol',required=True);ap.add_argument('--json',action='store_true')
 a=ap.parse_args();pe=PEImage(a.exe);obj=CoffObject(a.obj);sym=obj.find_symbol(a.symbol);cand,csize,rels=obj.symbol_extent(sym)
 # Use requested target function size. A COMDAT section normally gives exact candidate size.
 target=pe.bytes_at_va(a.target_va,a.target_size)
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
 result={'target_va':f'0x{a.target_va:08x}','target_size':len(target),'candidate_size':len(cand),'symbol':sym.name,'relocations_masked':sum(mask),'comparable_bytes':comparable,'matching_bytes':matching,'match_percent':round(100*matching/comparable,4) if comparable else 100.0,'exact_after_relocation_mask':exact,'alignment_padding_bytes':pad,'mismatches':mism}
 if a.json:print(json.dumps(result,indent=2))
 else:
  print(f"{sym.name}: target={len(target)} candidate={len(cand)} comparable={comparable} match={result['match_percent']:.2f}% exact={exact}")
  if mism:
   for d in mism[:20]: print(f"  +0x{d['offset']:x}: target {d['target']:02x} candidate {d['candidate']:02x}")
 raise SystemExit(0 if exact else 1)
if __name__=='__main__':main()
