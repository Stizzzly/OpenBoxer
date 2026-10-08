import json,hashlib,struct
from pathlib import Path
root=Path(r'C:\Users\ADMIN\CLionProjects\OpenBoxer');v=root/'validation';out=v/'GAME-0002-approved-source-replay';out.mkdir(exist_ok=True)
meta=json.loads((root/'specs/gameplay/GAME-0002-observer-metadata.json').read_text(encoding='utf-8-sig'));sites={int(c['callRva'],16):c for c in meta['calls']}
choices={'stage1':[17,18,19,20,21,22,23,24,25,27,29,30,32],'farstage1':[1,2,3,4,5,6,7,8,9,10,11,12,13,15,17,19,20]}
dirs={'stage1':'damage-original-observer-stage1','farstage1':'damage-original-far-stage1'}
def sha(p):return hashlib.sha256(p.read_bytes()).hexdigest()
def u(b,o=0):return struct.unpack_from('<I',b,o)[0]
manifest=[];fail=[];checks=0
for group,ids in choices.items():
 ad=json.loads((v/f'GAME-0002-independent-{group}-admission.json').read_text());adm={r['id']:r for r in ad['records']};ig=json.loads((v/f'GAME-0002-independent-{group}-integrity.json').read_text());integ={r['id']:r for r in ig['records']}
 src=Path(r'C:\Users\ADMIN\Boxer-lab\ms3d')/dirs[group];prov=src/'prepatch-provenance.json';provenance=json.loads(prov.read_text())
 for n in ids:
  path=src/f'damage-original-{n:03}.json';d=json.loads(path.read_text());issues=[]
  if adm[n]['reasons'] or adm[n]['localErrors'] or integ[n]['integrityErrors']:issues.append('source not admitted')
  roles={addr:role for role,addr in adm[n]['localPointerRoles'].items()};before=d['before'];roles.update({before['player_body']:'registered_player_body',before['opponent_body']:'registered_opponent_body',before['world']:'registered_world',0x5849f8:'static_AI_actor',0x584910:'static_player_actor'})
  for addr,role in [(0x5761dc,'global_player_position'),(0x576240,'global_opponent_position'),(0x5848e0,'effect_position'),(0x575df0,'audio_walk'),(0x578070,'audio_block'),(0x578050,'audio_hit'),(0x585340,'audio_AI5'),(0x585320,'audio_AIother')]:roles[addr]=role
  for off in [12,28,44,76,92,108,124]:roles[0x5849f8+off]=f'actor_vector_{off}'
  def norm(addr):
   if addr in roles:return {'role':roles[addr]}
   return {'raw32':addr}
  events=[]
  for e in d['events']:
   s=sites[e['call_rva']];ne={'ordinal':e['ordinal'],'callRva':e['call_rva'],'identity':e['identity'],'owner':norm(e['owner']) if s['ecx'] else {'raw32':e['owner']},'args':[],'EAX':{'raw32':e['eax']}}
   for idx,arg in enumerate(e['args']):
    # Metadata args describes vector BYVALUE as one role but stack has4words.
    kind=s['args'][idx] if idx<len(s['args']) else 'BYVALUE_raw32'
    ne['args'].append(norm(arg) if 'pointer' in kind else {'raw32':arg})
   if s['ret']=='element_pointer_EAX':
    checks+=1
    if e['eax']!=e['owner']+4*e['args'][0]:issues.append('noncanonical accessor return')
    ne['EAX']={'elementOf':norm(e['owner']),'index':e['args'][0]}
   elif s['ret']=='output_pointer_EAX':
    expected=e['owner'] if s['abi'].startswith('thiscall_ret12') else e['args'][0];checks+=1
    if e['eax']!=expected:issues.append('noncanonical output return')
    ne['EAX']=norm(expected)
   if e['combat_before16']!=e['combat_after16']:issues.append('opaque callback changed governing combat globals')
   if s['writes']=='opponent_body':
    a=bytes.fromhex(e['owner_before']);b=bytes.fromhex(e['owner_after'])
    for off,size in [(0,4),(216,4),(244,4),(652,4),(2276,4),(2280,4),(2284,1)]:
     checks+=1
     if a[off:off+size]!=b[off:off+size]:issues.append('callback structural witness changed')
   if s['ret'].startswith('raw80'):
    fp=bytes.fromhex(e['exit_fp544']);cw=u(fp)&65535;sw=u(fp,2)&65535
    checks+=1
    if cw!=0x27f or ((sw>>11)&7)!=7:issues.append('scalar callback unsupported FP stack/CW')
   events.append(ne)
  rec={'status':'APPROVED_TYPED_ORIGINAL_SOURCE_REPLAY_INPUT_ONLY','sourceId':f'{group}-{n:03}','sourcePath':str(path),'sourceSha256':sha(path),'provenancePath':str(prov),'provenanceSha256':sha(prov),'rootProvenance':provenance,'admissionReview':adm[n],'pointerRoles':[{'address':a,'role':r} for a,r in sorted(roles.items())],'normalizationPolicy':'Only declared roles. Original source bytes/EAX/registers preserved. No general numeric-address rewriting; unknown raw words remain exact. Local aliases use approved distinct identities.','normalizedEventRoles':events,'sourceRecord':d}
  if issues:fail.append({'id':rec['sourceId'],'errors':issues});continue
  dest=out/(rec['sourceId']+'.json');dest.write_text(json.dumps(rec,separators=(',',':')))
  manifest.append({'sourceId':rec['sourceId'],'path':str(dest),'sha256':sha(dest),'sourcePath':str(path),'sourceSha256':sha(path),'provenancePath':str(prov),'provenanceSha256':sha(prov),'entryFP':integ[n]['entryFP'],'events':integ[n]['events'],'consumed':integ[n]['consumed'],'entryBlock':integ[n]['entryBlock'],'finalBlock':integ[n]['finalBlock']})
result={'status':'PASS_SELECTED_SOURCE_EXPORT_ONLY' if not fail else 'BLOCKED_EXPORT','checks':checks,'count':len(manifest),'errors':fail,'records':manifest,'approvedContractHashes':[{'path':str(root/'specs/gameplay'/s),'sha256':sha(root/'specs/gameplay'/s)} for s in ['GAME-0002-opponent-strike-consumption.md','GAME-0002-typed-fp-locals.md','GAME-0002-observer-metadata.json','GAME-0002-original-natural-evidence.md']],'equivalence':'BLOCKED_PENDING_SOURCE_ORIGINAL_REPLAY_AND_CANDIDATE'}
(v/'GAME-0002-independent-approved-source-manifest.json').write_text(json.dumps(result,indent=2));print(json.dumps({'status':result['status'],'count':len(manifest),'checks':checks,'errors':fail},indent=2))
