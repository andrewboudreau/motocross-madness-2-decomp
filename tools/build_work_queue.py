#!/usr/bin/env python3
from __future__ import annotations
import json
from pathlib import Path


def load(path,default):
 p=Path(path); return json.loads(p.read_text()) if p.exists() else default


def rank(row):
    status=row.get('status','discovered')
    if row.get('vc6_exact') is True: return -1000
    if row.get('clang_exact') is True: return -500
    score=0
    if status=='calibration': score+=95
    elif row.get('compiler_stability')=='high': score+=80
    elif row.get('compiler_stability')=='medium': score+=55
    else: score+=30
    size=int(row.get('target_size') or 99)
    score += max(0,35-min(size,35))
    uses=row.get('vtable_uses',[])
    score += min(15,len(uses))
    hints=row.get('source_xref_hints',[])
    if hints:
        d=int(hints[0]['distance'])
        if d<=0x100: score+=20
        elif d<=0x400: score+=15
        elif d<=0x1000: score+=10
        elif d<=0x4000: score+=4
    return score


def main():
    funcs=load('analysis/function_manifest.json',[])
    dossiers={d['class']:d for d in load('analysis/class_dossiers.json',[])}
    rows=[]
    for f in funcs:
        r=dict(f); r['priority_score']=rank(r)
        classes=[]
        for u in r.get('vtable_uses',[]):
            if u['class'] not in classes: classes.append(u['class'])
        if r.get('provisional_class') and r['provisional_class'] not in classes: classes.insert(0,r['provisional_class'])
        r['classes']=classes
        # Pull strongest source hint from class dossiers when function-local proximity is weak.
        ch=[]
        for c in classes[:8]:
            ds=dossiers.get(c,{}).get('source_candidates',[])
            if ds: ch.append({'class':c,**ds[0]})
        r['class_source_hints']=ch
        rows.append(r)
    next_rows=[r for r in rows if r.get('clang_exact') is not True and r.get('vc6_exact') is not True]
    next_rows.sort(key=lambda r:(-r['priority_score'],int(r['target_va'],16)))
    validated=[r for r in rows if r.get('clang_exact') is True or r.get('vc6_exact') is True]
    out={'next':next_rows,'validated':validated,'all':rows}
    Path('analysis/work_queue.json').write_text(json.dumps(out,indent=2)+'\n')

    lines=['# Agent work queue','',
           'Mechanically ranked from the function manifest. Priority is a convenience heuristic, not evidence. Calibration targets are intentionally ranked highly because VC6 can resolve compiler-shape questions.','',
           '## Next targets','',
           '| Rank | VA | Size | Kind/status | Stability | Classes | Nearest source hint |',
           '|---:|---|---:|---|---|---|---|']
    for i,r in enumerate(next_rows[:40],1):
        classes=', '.join(r.get('classes',[])[:3]) or '—'
        h=(r.get('source_xref_hints') or [{}])[0]
        src=Path(h.get('path','')).name if h.get('path') else '—'
        if h.get('distance') is not None: src+=f" (0x{int(h['distance']):x})"
        ks=r.get('kind') or r.get('calibration_name') or r.get('status','')
        lines.append(f"| {i} | `{r['target_va']}` | {r.get('target_size','?')} | {ks} | {r.get('compiler_stability','—')} | {classes} | {src} |")
    lines += ['','## Validated clang/MSVC-ABI plumbing samples','']
    for r in sorted(validated,key=lambda x:int(x['target_va'],16)):
        classes=', '.join(r.get('classes',[])[:3]) or r.get('provisional_class') or '—'
        lines.append(f"- `{r['target_va']}` — {r.get('target_size','?')} B — {r.get('kind') or r.get('status')} — {classes}")
    Path('analysis/WORK_QUEUE.md').write_text('\n'.join(lines)+'\n')
    print(json.dumps({'next_targets':len(next_rows),'validated_targets':len(validated),'output':'analysis/work_queue.json'},indent=2))

if __name__=='__main__': main()
