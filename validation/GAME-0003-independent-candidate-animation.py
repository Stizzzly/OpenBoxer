import json,struct,hashlib,sys
from pathlib import Path
R=Path(r'C:\Users\ADMIN\CLionProjects\OpenBoxer');D=Path(sys.argv[1]);PID=int(sys.argv[2]);M=json.loads((R/'specs/gameplay/GAME-0003-animation-observer-metadata.json').read_text());roles=M['roles'];rows=[];errors=[];checks=0
raw=bytes.fromhex
def u(b,o=0):return struct.unpack_from('<I',b,o)[0]
def sha(p):return hashlib.sha256(p.read_bytes()).hexdigest()
for p in sorted(D.glob('ai-selector-original-*.json')):
 d=json.loads(p.read_text());bad=[]
 def ck(ok,msg):
  global checks
  checks+=1
  if not ok:bad.append(msg)
 role=next((r for r in roles if int(r['returnRva'],16)+0x400000==d['caller'] and int(r['ownerRva'],16)+0x400000==d['owner'] and int(r['literalRva'],16)+0x400000==d['requested_pointer']),None)
 ck(role is not None,'exact declared route')
 if role:
  for key,field in [('variant','variant'),('desired_state','state'),('remembered_state','state'),('requested_name','requestedName'),('type','type')]:ck(d[key]==role[field],key)
 ck(d['module']==0x400000 and d['process']==PID and d['thread']>0 and d['parent_j']==1 and d['observation_only'] and not d['candidate'],'ownership/original/j')
 er=raw(d['entry_regs36']);xr=raw(d['exit_regs36']);ck(u(er,8)==d['parent_ebp'],'incoming parent EBP')
 for off in [0,4,8,16]:ck(u(er,off)==u(xr,off),'callee register'+str(off))
 ck((u(xr,12)-u(er,12))&0xffffffff==8,'ret4');ck(u(er,24)==d['owner'] and u(xr,28)==d['eax'],'ECX/EAX')
 for key in ['entry_fp544','exit_fp544']:
  b=raw(d[key]);ck(len(b)==544,key+':extent');ck(b[:2]==b[512:514] and b[2:4]==b[516:518],key+':duplicate CW/SW');tag=u(b,520)&65535
  for slot in range(8):ck((((tag>>(2*slot))&3)==3)==(not bool(b[4]&(1<<slot))),key+':tag consistency')
 ck(d['post_shape_safe'],'post model safe')
 for label in ['entry','exit']:
  b=raw(d[label+'_model112']);records=raw(d[label+'_records']);ck(len(b)==112 and len(records)%272==0 and 0<len(records)<=272*256,'boundedmodel '+label)
  ck(len(records)==u(b,92)-u(b,88),'collection extent '+label)
 ck(d['entry_clock_raw32']==d['exit_clock_raw32'],'selector clock unmodified')
 wholepath=D/f'ai-candidate-{d["whole_exit_sequence"]:03}.json';whole=json.loads(wholepath.read_text());ck(whole.get('candidate') and whole.get('actual_route')=='candidate' and whole.get('admitted') and whole.get('candidate_invocations')==1 and whole.get('original_invocations')==0 and whole.get('process')==PID,'whole candidate route/owned PID');a=raw(whole['after']['actor232']);entry=raw(d['entry_actor232']);actual=whole['actual_start_type']
 expected=bytearray(a);struct.pack_into('<I',expected,160,d['desired_state'])
 # Approved original renderer writes remembered+160 before selector. Require
 # every other actor byte exact; this is one explicit side effect, not waiver.
 exact=entry==bytes(expected);correlated=actual==d['type'] and u(a,164)==d['desired_state'] and whole['thread']==d['thread'] and exact
 rows.append({'path':str(p),'sha256':sha(p),'sequence':d['sequence'],'wholeExitSequence':d['whole_exit_sequence'],'desiredState':d['desired_state'],'type':d['type'],'declaredRole':role,'wholeSourceHash':sha(wholepath),'exactWholeAfterActorWithApprovedRememberedWrite':exact,'rememberedWrite':{'offset':160,'wholeBeforeRaw32':u(a,160),'expectedRaw32':d['desired_state'],'provenance':'Approved GAME-0003 renderer remembered store37186 before selector'},'correlation':'MATCHED_ACTOR_EXCEPT_EXACT_APPROVED_REMEMBERED_STORE_STATE_TYPE_THREAD' if correlated else 'EXCLUDED_STALE_LATEST_SEQUENCE','wholeEntryAdmission':'Candidate whole admission independently checked under approved GAME0003 combo1.6 supplement','errors':bad})
 errors.extend((p.name,x) for x in bad)
result={'status':'PASS_TYPED_SELECTOR_INTEGRITY_CORRELATED_OBSERVATION_ONLY' if not errors else 'FAIL_CAPTURE_INTEGRITY','checks':checks,'errors':errors,'records':rows,'correlatedStates':sorted(set(x['desiredState'] for x in rows if x['correlation'].startswith('MATCHED'))),'scope':'Original opaque selector called after candidate whole: identity/ABI/capture and whole-exit correlation, not replacement/native admission or pre-dispatch inequality proof'}
(R/f'validation/GAME-0003-independent-candidate-animation-{D.name.rsplit(chr(45),1)[-1]}.json').write_text(json.dumps(result,indent=2));print(json.dumps({k:v for k,v in result.items() if k!='records'}));print([(x['sequence'],x['wholeExitSequence'],x['desiredState'],x['correlation']) for x in rows])
