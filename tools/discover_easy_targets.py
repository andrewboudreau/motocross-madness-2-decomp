#!/usr/bin/env python3
from __future__ import annotations
import argparse,json
from pathlib import Path
def main():
 ap=argparse.ArgumentParser();ap.add_argument('--analysis',default='analysis/easy_targets.json');ap.add_argument('--class',dest='class_name');ap.add_argument('--kind');ap.add_argument('--all-uses',action='store_true');ap.add_argument('--json',action='store_true');a=ap.parse_args();rows=json.loads(Path(a.analysis).read_text())
 if a.class_name: rows=[r for r in rows if any(u['class']==a.class_name for u in r['uses'])]
 if a.kind: rows=[r for r in rows if r['kind']==a.kind]
 if a.json: print(json.dumps(rows,indent=2));return
 for r in rows:
  uses=r['uses'] if a.all_uses else r['uses'][:6];where=', '.join(f"{u['class']}[{u['slot']}]" for u in uses);print(f"{r['target_va']}  {r['kind']:<24} {r['target_size']:>3}B [{r['compiler_stability']}]  {where}")
if __name__=='__main__':main()
