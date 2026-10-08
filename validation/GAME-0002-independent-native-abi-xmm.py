"""Typed natural records only: native callback ABI and exact opaque XMM chain."""
import json, struct, sys, hashlib
from pathlib import Path
R=Path(__file__).resolve().parent.parent
D=Path(sys.argv[1]); OUT=Path(sys.argv[2])
meta=json.loads((R/'specs/gameplay/GAME-0002-observer-metadata.json').read_text())
sites={int(s['callRva'],16):s for s in meta['calls']}
checks=0;errors=[];rows=[]
def ck(a,label):
 global checks
 checks+=1
 if not a:errors.append(label)
def u(b,o):return struct.unpack_from('<I',b,o)[0]
for p in sorted(D.glob('damage-candidate-*.json')):
 d=json.loads(p.read_text());ctx=p.name;last=bytes.fromhex(d['entry_fp544'])[160:288]
 for j,e in enumerate(d['events']):
  a,b=[bytes.fromhex(e[k]) for k in ['entry_regs36','exit_regs36']];site=sites[e['call_rva']]
  for o in [0,4,8,16]:ck(u(a,o)==u(b,o),ctx+f'event{j} nonvolatile{o}')
  pop=int(site['abi'].split('ret')[-1]) if site['abi'].startswith('thiscall') else 0
  ck((u(b,12)-u(a,12))&0xffffffff==4+pop,ctx+f'event{j} stack ABI')
  entry=bytes.fromhex(e['entry_fp544'])[160:288]
  ck(entry==last,ctx+f'event{j} exact incoming/last opaque XMM')
  last=bytes.fromhex(e['exit_fp544'])[160:288]
 ck(bytes.fromhex(d['exit_fp544'])[160:288]==last,ctx+' whole exit exact last opaque XMM')
 rows.append({'path':str(p),'sha256':hashlib.sha256(p.read_bytes()).hexdigest(),'events':len(d['events'])})
out={'status':'PASS_NATIVE_CANDIDATE_ABI_OPAQUE_XMM_CHAIN' if not errors else 'FAIL','checks':checks,'errors':errors,'records':rows}
OUT.write_text(json.dumps(out,indent=2));print({k:out[k] for k in ['status','checks','errors']})
