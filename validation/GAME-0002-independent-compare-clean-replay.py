"""Compare approved typed source and two root-owned replay records, no processes."""
import json,struct,hashlib,sys
from pathlib import Path
R=Path(r'C:\Users\ADMIN\CLionProjects\OpenBoxer'); V=R/'validation'
META=json.loads((R/'specs/gameplay/GAME-0002-observer-metadata.json').read_text(encoding='utf-8-sig'))
SITES={int(s['callRva'],16):s for s in META['calls']};SPANS=META['capture']['globalExtents']
MANIFEST=json.loads((V/'GAME-0002-independent-approved-clean-source-manifest.json').read_text())
ROOT=Path(sys.argv[1]); OUT=Path(sys.argv[2]); checks=0;errors=[];records=[]
LOCAL={0x1c70b:('L-POS-P','arg'),0x1c740:('L-POS-O','arg'),0x1cac6:('L-NEG','arg'),0x1cad6:('L-MUL-F','arg'),0x1cb7b:('L-MUL-R','arg'),0x1d775:('L-HIT','owner'),0x1d900:('L-HIT-POS','arg'),0x1d9e8:('L-POOL-V','arg'),0x1e04c:('L-TAIL-POS','arg'),0x1e079:('L-ZERO','owner'),0x1e08e:('L-ZERO','arg'),0x1e0a3:('L-TAIL-V','arg'),0x1e145:('L-ADD','arg'),0x1e17e:('L-MUL-S','arg'),0x1e2f3:('L-LIMIT','arg')}
def u(b,o=0):return struct.unpack_from('<I',b,o)[0]
def sha(p):return hashlib.sha256(p.read_bytes()).hexdigest()
def eq(x,y,context):
 global checks
 checks+=1
 if x!=y:errors.append({'field':context,'original':str(x)[:120],'candidate':str(y)[:120]})
def roles(d):
 b=d['before']; out={b['player_body']:'registered_player_body',b['opponent_body']:'registered_opponent_body',b['world']:'registered_world',0x5849f8:'static_AI_actor',0x584910:'static_player_actor'}
 for addr,name in [(0x5761dc,'global_player_position'),(0x576240,'global_opponent_position'),(0x5848e0,'effect_position'),(0x575df0,'audio_walk'),(0x578070,'audio_block'),(0x578050,'audio_hit'),(0x585340,'audio_AI5'),(0x585320,'audio_AIother')]:out[addr]=name
 for off in [12,28,44,76,92,108,124]:out[0x5849f8+off]=f'actor_vector_{off}'
 for e in d['events']:
  if e['call_rva'] in LOCAL:
   name,kind=LOCAL[e['call_rva']];addr=e['owner'] if kind=='owner' else e['args'][0]
   eq(out.get(addr,name),name,'role alias');out[addr]=name
  if e['call_rva']==0x1e1f4:out[e['args'][2]]='L-AUDIO'
 return out
def norm(x,m):return m.get(x,('RAW',x))
def data(hexvalue,kind,m):
 b=bytearray.fromhex(hexvalue); fields=[]
 offs={'actor232':[152],'player176':[152],'player_body2288':[0,244,2276,2280],'opponent_body2288':[0,244,2276,2280],'world_list12':[0,4]}.get(kind,[])
 if kind=='globals':
  cursor=0
  for span in SPANS:
   if int(span['rva'],16) in [0x1849a8,0x184a90,0x1853a4]:offs.append(cursor)
   cursor+=span['size']
 for off in offs:
  value=u(b,off);fields.append((off,norm(value,m)));b[off:off+4]=b'\0'*4
 return b,fields
def fp(x,y,context):
 a,b=bytes.fromhex(x),bytes.fromhex(y);eq(len(a),544,context+' extentA');eq(len(b),544,context+' extentB')
 for off,n in [(0,2),(2,2),(4,1),(24,4),(28,4),(160,128),(512,2),(516,2),(520,2)]:eq(a[off:off+n],b[off:off+n],context+f' exactOffset{off}')
 for side,z in enumerate([a,b]):
  eq(z[0:2],z[512:514],context+f'CWduplicate{side}');eq(z[2:4],z[516:518],context+f'SWduplicate{side}')
  tags=u(z,520)&65535
  for physical in range(8):eq(((tags>>(2*physical))&3)==3,not bool(z[4]&(1<<physical)),context+f'tagConsistency{side}/{physical}')
 top=(u(a,2)>>11)&7; tags=u(a,520)&65535
 for logical in range(8):
  if ((tags>>(2*((top+logical)%8)))&3)!=3:eq(a[32+16*logical:42+16*logical],b[32+16*logical:42+16*logical],context+f'liveST{logical}')
def compare(a,b,label):
 ma,mb=roles(a),roles(b)
 for key in ['module','mode','eax','nested_whole_observed']:eq(a[key],b[key],label+key)
 if 'source->original:' in label:
  eq(a['caller'],0x42d8eb,label+'nativeSourceCaller')
  eq(b['caller'],0,label+'directBodyFixtureCaptureCallerPlaceholder')
 else:eq(a['caller'],b['caller'],label+'caller')
 for when in ['before','after']:
  eq(a[when]['global_span_sizes'],b[when]['global_span_sizes'],label+when+'spans')
  for key in ['actor232','player176','player_body2288','opponent_body2288','world_list12','globals']:eq(data(a[when][key],key,ma),data(b[when][key],key,mb),label+when+key)
 for key in ['entry_fp544','exit_fp544']:fp(a[key],b[key],label+key)
 eq(len(a['events']),len(b['events']),label+'eventcount')
 for j,(x,y) in enumerate(zip(a['events'],b['events'])):
  t=label+f'event{j}:';s=SITES[x['call_rva']]
  for key in ['ordinal','call_rva','return_rva','identity','rng_before','rng_after','combat_before16','combat_after16','arg_before','arg_after','returned_bytes']:eq(x[key],y[key],t+key)
  for key in ['owner_before','owner_after']:
   kind='opponent_body2288' if s['writes']=='opponent_body' else 'actor232' if s['identity']=='sub_401D4D' else ''
   eq(data(x[key],kind,ma),data(y[key],kind,mb),t+key)
  if s['ecx']:eq(norm(x['owner'],ma),norm(y['owner'],mb),t+'owner')
  eq(len(x['args']),len(y['args']),t+'argsCount')
  for k,(xx,yy) in enumerate(zip(x['args'],y['args'])):
   pointer=k<len(s['args']) and 'pointer' in s['args'][k];eq(norm(xx,ma) if pointer else xx,norm(yy,mb) if pointer else yy,t+f'arg{k}')
  if s['ret']=='element_pointer_EAX':eq((norm(x['owner'],ma),x['eax']-x['owner']),(norm(y['owner'],mb),y['eax']-y['owner']),t+'elementEAX')
  elif s['ret']=='output_pointer_EAX':eq(norm(x['eax'],ma),norm(y['eax'],mb),t+'pointerEAX')
  else:eq(x['eax'],y['eax'],t+'rawEAX')
  if 'scalar80' in x or 'scalar80' in y:eq(x.get('scalar80'),y.get('scalar80'),t+'scalar80')
  for key in ['entry_fp544','exit_fp544']:fp(x[key],y[key],t+key)
  for z in [x,y]:
   before,after=[bytes.fromhex(z[k]) for k in ['entry_regs36','exit_regs36']]
   for off in [0,4,8,16]:eq(u(before,off),u(after,off),t+'calleeSaved')
   pop=int(s['abi'].split('ret')[-1]) if s['abi'].startswith('thiscall') else 0;eq((u(after,12)-u(before,12))&0xffffffff,4+pop,t+'ESP')
for item in MANIFEST['records']:
 sid=item['sourceId'];p=Path(item['path']);eq(sha(p),item['sha256'],sid+'exportHash');export=json.loads(p.read_text());source=export['sourceRecord']
 folder=ROOT/sid;paths=[folder/'damage-replay-original-001.json',folder/'damage-candidate-002.json']
 if not all(p.exists() for p in paths):records.append({'sourceId':sid,'status':'BLOCKED_MISSING_RECORD','expectedPaths':list(map(str,paths))});continue
 before=len(errors);o,c=[json.loads(p.read_text()) for p in paths];eq(o['candidate'],False,sid+'originalIdentity');eq(c['candidate'],True,sid+'candidateIdentity')
 compare(source,o,sid+'source->original:');compare(o,c,sid+'original->candidate:')
 records.append({'sourceId':sid,'status':'FAIL' if len(errors)>before else 'PASS_ARCHITECTURAL_STATE_EVENTS_ONLY','originalPath':str(paths[0]),'candidatePath':str(paths[1]),'originalSha256':sha(paths[0]),'candidateSha256':sha(paths[1]),'events':len(o['events'])})
result={'status':'FAIL' if errors else 'BLOCKED_MISSING_RECORD' if any(r['status'].startswith('BLOCKED') for r in records) else 'PASS_STATE_EVENTS_ARCHITECTURAL_FP_PENDING_RAW_PROVENANCE','checks':checks,'errors':errors,'records':records,'equivalence':'BLOCKED_FULL_RAW_FP_PROVENANCE_NATIVE_COMPOSED_DEFAULT_OFF_PENDING'}
OUT.write_text(json.dumps(result,indent=2));print(json.dumps({'status':result['status'],'checks':checks,'errors':errors[:10],'records':len(records)}))
