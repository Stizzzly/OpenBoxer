import json,struct,hashlib,re,sys
from pathlib import Path
R=Path(r'C:\Users\ADMIN\CLionProjects\OpenBoxer');V=R/'validation';D=Path(sys.argv[1]) if len(sys.argv)>1 else Path(r'C:\Users\ADMIN\Boxer-lab\ms3d\ai-legacy-stage3')
stage='legacy-damage-stage1'
expectedPairs=60;expectedGuards=22
meta=json.loads((R/'specs/gameplay/GAME-0004-observer-metadata.json').read_text(encoding='utf-8-sig'));sites={int(c['callRva'],16):c for c in meta['calls']};checks=0;errors=[];pairs=[]
def raw(s):return bytes.fromhex(s)
def u(b,o=0):return struct.unpack_from('<I',b,o)[0]
def sha(p):return hashlib.sha256(p.read_bytes()).hexdigest()
localsites={0x1c70b:('POS-P','arg'),0x1c740:('POS-O','arg'),0x1cac6:('NEG','arg'),0x1cad6:('MUL-F','arg'),0x1cb7b:('MUL-R','arg'),0x1d775:('HIT','owner'),0x1d900:('HIT-POS','arg'),0x1d9e8:('POOL-V','arg'),0x1e04c:('TAIL-POS','arg'),0x1e079:('ZERO','owner'),0x1e08e:('ZERO','arg'),0x1e0a3:('TAIL-V','arg'),0x1e145:('ADD','arg'),0x1e17e:('MUL-S','arg'),0x1e2f3:('LIMIT','arg'),0x1cf9f:('INIT-Z','owner'),0x1d2e8:('INIT-X','owner'),0x1d631:('INIT-C','owner')}
def roles(d):
 s=d['before'];r={s['player_body']:'PLAYERBODY',s['opponent_body']:'OPPONENTBODY',s['world']:'WORLD',0x5849f8:'ACTOR',0x584910:'PLAYER'}
 for e in d['events']:
  if e['call_rva'] in localsites:
   name,where=localsites[e['call_rva']];a=e['owner'] if where=='owner' else e['args'][0]
   if a in r and r[a]!=name:raise ValueError('local role alias')
   r[a]=name
  if e['call_rva']==0x1e1f4:r[e['args'][2]]='AUDIO'
 return r
def eq(a,b,label,context):
 global checks
 checks+=1
 if a!=b:errors.append({'checkpoint':context,'field':label,'original':str(a)[:160],'candidate':str(b)[:160]})
fpcode=(V/'GAME-0002-independent-fp-inventory.py').read_text();fpns={'sys':sys};saved=sys.argv;sys.argv=[str(V/'GAME-0003-independent-compare-fixtures.py')];exec(fpcode[:fpcode.index('for number in range')],fpns);sys.argv=saved
paths=sorted(D.glob('damage-fixture-original-*.json'))+sorted(D.glob('damage-guard-reference-*.json'))
for p in paths:
 number=int(p.stem.rsplit('-',1)[1]);q=D/f'damage-candidate-{number+1:03}.json'
 if not q.exists():q=D/f'damage-guard-candidate-{number+1:03}.json'
 o,c=[json.loads(x.read_text(encoding='utf-8-sig')) for x in [p,q]];maps=[roles(o),roles(c)];context='pair'+str(number)
 def norm(x,side):return maps[side].get(x,('RAW',x))
 eq(o['candidate'],False,'original flag',context);eq(c['candidate'],True,'candidate flag',context)
 for d in [o,c]:eq(d['nested_whole_observed'],False,'not nested',context)
 for k in ['mode','eax','thread']:eq(o[k],c[k],k,context)
 for label in ['before','after']:
  for k in ['actor232','player176','player_body2288','opponent_body2288','globals','world_list12','global_span_sizes']:eq(o[label][k],c[label][k],label+k,context)
 # Manual whole captures intentionally have no PUSHAD registers. Actual whole
 # invocation ABI is supplied by the separate native Witness worker report.
 for d in [o,c]:
  eq(d['entry_regs36'],'00'*36,'manual entry register sink absent',context)
  eq(d['exit_regs36'],'00'*36,'manual exit register sink absent',context)
 for key in ['entry_fp544','exit_fp544']:fpns['inspect'](o[key],c[key],context+':'+key)
 eq(len(o['events']),len(c['events']),'events count',context)
 for i,(a,b) in enumerate(zip(o['events'],c['events'])):
  cp=context+':event'+str(i)+':site'+hex(a['call_rva']);s=sites[a['call_rva']]
  for k in ['ordinal','call_rva','return_rva','identity','rng_before','rng_after','owner_before','owner_after','combat_before16','combat_after16','arg_before','arg_after','returned_bytes']:eq(a[k],b[k],k,cp)
  eq(a['ordinal'],i,'ordinal',cp);eq(a['identity'],s['identity'],'declared identity',cp);eq(a['return_rva'],int(s['returnRva'],16),'declared return',cp)
  if s['ecx']:eq(norm(a['owner'],0),norm(b['owner'],1),'owner role',cp)
  eq(len(a['args']),len(b['args']),'arg count',cp)
  for j,(x,y) in enumerate(zip(a['args'],b['args'])):
   definition=s['args'][j] if j<len(s['args']) else 'BYVALUEWORD';eq(norm(x,0) if 'pointer' in definition else x,norm(y,1) if 'pointer' in definition else y,'arg'+str(j),cp)
  if s['ret']=='element_pointer_EAX':eq((norm(a['owner'],0),a['eax']-a['owner']),(norm(b['owner'],1),b['eax']-b['owner']),'elementEAX',cp)
  elif s['ret']=='output_pointer_EAX':eq(norm(a['eax'],0),norm(b['eax'],1),'outputEAX',cp)
  else:eq(a['eax'],b['eax'],'rawEAX',cp)
  eq(a.get('scalar80'),b.get('scalar80'),'scalar80',cp)
  for key in ['entry_fp544','exit_fp544']:fpns['inspect'](a[key],b[key],cp+':'+key)
  for e in [a,b]:
   er=raw(e['entry_regs36']);xr=raw(e['exit_regs36'])
   for off in [0,4,8,16]:eq(u(er,off),u(xr,off),'callback nonvolatile'+str(off),cp)
   pop=int(s['abi'].split('ret')[-1]) if s['abi'].startswith('thiscall') else 0;eq((u(xr,12)-u(er,12))&0xffffffff,4+pop,'callback stack',cp)
 pairs.append({'id':number,'originalPath':str(p),'candidatePath':str(q),'originalSha256':sha(p),'candidateSha256':sha(q),'events':len(o['events'])})
report=(D/'damage-fixtures.txt').read_text();abi=re.findall(r'candidate=([01]).*?replacement=(\d+) fallback=(\d+) route=(\d+) abi=(\d+) esp=([0-9a-f]+)/([0-9a-f]+)',report)
eq(len(abi),2*expectedPairs,'297pair ABI report rows','REPORT')
for row in abi:
 eq(row[3:5],('1','1'),'route ABI','REPORT');eq(row[5],row[6],'ESP witness','REPORT')
 if row[0]=='1':eq(row[1:3],('1','0'),'actual replacement','REPORT')
eq('cleanup observer_restored=1 failures=0' in report,True,'restored cleanup','REPORT')
guards=re.findall(r'^guard=\d+ .+ pass=(\d+)',report,re.M);eq(len(guards),expectedGuards,'22guards','REPORT');eq(set(guards),{'1'},'guards pass','REPORT')
counts={}
for x in fpns['differences']:counts[x['classification']]=counts.get(x['classification'],0)+1
fpresult={'status':'FAIL' if fpns['errors'] else 'BLOCKED_PROVENANCE','checks':fpns['checks'],'pairs':len(pairs),'errors':fpns['errors'],'differenceCounts':counts,'rawDifferenceInventory':fpns['differences'],'sources':pairs}
(V/f'GAME-0004-independent-fixture-{stage}-fp-inventory.json').write_text(json.dumps(fpresult,indent=2))
result={'status':'FAIL' if errors or fpns['errors'] else 'PASS_STATE_CALLBACK_ABI_ARCHITECTURAL_FP_PENDING_PROVENANCE','checks':checks,'pairs':len(pairs),'errors':errors,'FPerrorCount':len(fpns['errors']),'sources':pairs,'differenceCounts':counts,'wholeRegisterSource':'Separate worker native Witness report; manual JSON register sink absent, not a register-equality waiver'}
(V/f'GAME-0004-independent-fixture-{stage}-comparison.json').write_text(json.dumps(result,indent=2));print(json.dumps({k:v for k,v in result.items() if k!='sources' and k!='errors'}));print(json.dumps(errors[:12]));print(json.dumps(fpns['errors'][:12]))
