import json,struct,hashlib,re,sys
from pathlib import Path
R=Path(r'C:\Users\ADMIN\CLionProjects\OpenBoxer');V=R/'validation';D=Path(sys.argv[1]) if len(sys.argv)>1 else Path(r'C:/Users/ADMIN/Boxer-lab/ms3d/continuation-replay-observed-stage1')
stage='stage3' if D.name.endswith('stage3') else 'stage2'
expectedPairs=337 if stage=='stage3' else 297;expectedGuards=28 if stage=='stage3' else 22
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

def normalized(value,key,mp):
 if not isinstance(value,str):return value
 offsets=[]
 if key in ['actor232','player176'] or key in ['owner_before','owner_after'] and len(value) in [464,352]:offsets=[152]
 elif key in ['player_body2288','opponent_body2288'] or key in ['owner_before','owner_after'] and len(value)==4576:offsets=[0,244,2276,2280]
 elif key=='world_list12':offsets=[0,4]
 elif key=='globals':
  cur=0
  for span in meta['capture']['globalExtents']:
   if int(span['rva'],16) in [0x1849a8,0x184a90,0x1853a4]:offsets.append(cur)
   cur+=span['size']
 if not offsets:return value
 bs=bytearray.fromhex(value);ptrs=[]
 for off in offsets:
  ptrs.append((off,mp.get(u(bs,off),('RAW',u(bs,off)))));bs[off:off+4]=b'\x00'*4
 return (bs.hex(),ptrs)
manifest=json.loads((V/'GAME-0004-approved-native-original-manifest.json').read_text());missing=[];records=[]
for item in manifest['records']:
 sid=item['id'];folder=D/sid;p=folder/'damage-ai-continuation-replay-original-001.json';q=folder/'damage-ai-continuation-replay-candidate-002.json'
 if not p.exists() or not q.exists() or not (folder/'ai-continuation-replay.txt').exists():missing.append(sid);continue
 export=Path(item['path']);eq(sha(export),item['sha256'],'export hash',sid);source=json.loads(export.read_text());original=json.loads(p.read_text());candidate=json.loads(q.read_text());report=(folder/'ai-continuation-replay.txt').read_text()
 for needle in ['typed_errors=0 abi=1','source_state=1 source_eax=1','candidate=1','replacement=1 fallback=0 route=1','cleanup observer_restored=1 failures=0',item['sha256'],item['sourceSha256']]:eq(needle in report,True,'worker '+needle,sid)
 for o,c,label in [(source,original,'source->original'),(original,candidate,'original->candidate')]:
  context=sid+':'+label
  maps=[roles(o),roles(c)]
  def norm(x,side):return maps[side].get(x,('RAW',x))
  for k in ['mode','eax']:eq(o[k],c[k],k,context)
  for label in ['before','after']:
   for k in ['actor232','player176','player_body2288','opponent_body2288','globals','world_list12','global_span_sizes']:eq(normalized(o[label][k],k,maps[0]),normalized(c[label][k],k,maps[1]),label+k,context)
  eq(len(o['events']),len(c['events']),'events count',context)
  for i,(a,b) in enumerate(zip(o['events'],c['events'])):
   cp=context+':event'+str(i)+':site'+hex(a['call_rva']);s=sites[a['call_rva']]
   for k in ['ordinal','call_rva','return_rva','identity','rng_before','rng_after','owner_before','owner_after','combat_before16','combat_after16','arg_before','arg_after','returned_bytes']:eq(normalized(a[k],k,maps[0]),normalized(b[k],k,maps[1]),k,cp)
   eq(a['ordinal'],i,'ordinal',cp);eq(a['identity'],s['identity'],'declared identity',cp);eq(a['return_rva'],int(s['returnRva'],16),'declared return',cp)
   if s['ecx']:eq(norm(a['owner'],0),norm(b['owner'],1),'owner role',cp)
   eq(len(a['args']),len(b['args']),'arg count',cp)
   for j,(x,y) in enumerate(zip(a['args'],b['args'])):
    definition=s['args'][j] if j<len(s['args']) else 'BYVALUEWORD';eq(norm(x,0) if 'pointer' in definition else x,norm(y,1) if 'pointer' in definition else y,'arg'+str(j),cp)
   if s['ret']=='element_pointer_EAX':eq((norm(a['owner'],0),a['eax']-a['owner']),(norm(b['owner'],1),b['eax']-b['owner']),'elementEAX',cp)
   elif s['ret']=='output_pointer_EAX':eq(norm(a['eax'],0),norm(b['eax'],1),'outputEAX',cp)
   else:eq(a['eax'],b['eax'],'rawEAX',cp)
   eq(a.get('scalar80'),b.get('scalar80'),'scalar80',cp)
   pass
   for e in [a,b]:
    er=raw(e['entry_regs36']);xr=raw(e['exit_regs36'])
    for off in [0,4,8,16]:eq(u(er,off),u(xr,off),'callback nonvolatile'+str(off),cp)
    pop=int(s['abi'].split('ret')[-1]) if s['abi'].startswith('thiscall') else 0;eq((u(xr,12)-u(er,12))&0xffffffff,4+pop,'callback stack',cp)
 records.append({'id':sid,'originalSha256':sha(p),'candidateSha256':sha(q)})
result={'status':'FAIL' if errors else 'BLOCKED_MISSING_RECORD' if missing else 'PASS_EXACT_TYPED_STATE_CALLBACK_ABI','checks':checks,'errors':errors,'missing':missing,'records':records,'FP':'Separate full architectural/provenance inventory required'}
(V/'GAME-0004-independent-replay-stage1-state-callback.json').write_text(json.dumps(result,indent=2));print(json.dumps({k:v for k,v in result.items() if k!='records'}))
