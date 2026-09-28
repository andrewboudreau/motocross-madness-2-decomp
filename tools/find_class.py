#!/usr/bin/env python3
import argparse,json
from pathlib import Path
a=argparse.ArgumentParser();a.add_argument('name');a.add_argument('--analysis',default='analysis');x=a.parse_args();rows=json.loads((Path(x.analysis)/'rtti_classes.json').read_text());q=x.name.lower();print(json.dumps([r for r in rows if q in r['name'].lower() or q in r['decorated'].lower()],indent=2))
