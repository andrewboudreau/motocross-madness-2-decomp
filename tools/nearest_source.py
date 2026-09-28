#!/usr/bin/env python3
import argparse,json
from pathlib import Path
a=argparse.ArgumentParser();a.add_argument('va');a.add_argument('--analysis',default='analysis/source_xrefs.json');a.add_argument('-n','--count',type=int,default=8);x=a.parse_args();va=int(x.va,0);rows=[]
for r in json.loads(Path(x.analysis).read_text()):
 for q in r.get('text_xrefs',[]):
  qv=int(q,16) if isinstance(q,str) else int(q);rows.append((abs(qv-va),qv-va,qv,r['path']))
for d,delta,qv,p in sorted(rows)[:x.count]:print(f"{d:#08x} delta={delta:+#x} 0x{qv:08x} {p}")
