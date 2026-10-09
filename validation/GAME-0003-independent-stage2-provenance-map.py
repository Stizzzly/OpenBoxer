import json,re,struct,hashlib,sys,pefile
from pathlib import Path
R=Path(r'C:/Users/ADMIN/CLionProjects/OpenBoxer');V=R/'validation';D=Path(r'C:/Users/ADMIN/Boxer-lab/ms3d/ai-fixture-stage2')
O={int(e['originalFipVa'],16):e for e in json.loads((R/'specs/gameplay/GAME-0003-original-fp-provenance.json').read_text())['entries']}
X=json.loads((V/'GAME-0003-independent-replacement-v2-fp-map.json').read_text());SP=X;XP={e['rva']:e for e in X['rows']};base=0x70f80000
checks=0;errors=[];blocked=[];proofs=[]
def ck(ok,msg):
 global checks
 checks+=1
 if not ok:errors.append(msg)

pe=pefile.PE(X['dll']);dllraw=Path(X['dll']).read_bytes();ck(hashlib.sha256(dllraw).hexdigest()==X['sha256'],'own frozen DLL hash')
module=json.loads((D/'damage-fixture-prepatch-observation.json').read_text());ck(module['replacement_module_sha256'].lower()==X['sha256'],'actual loaded hash');ck(int(module['replacement_module_base'],16)==base,'actual loaded base');ck(module['dependency_call_targets_verified']==72 and module['natural_caller_verified'] and module['audio_import_resolved'],'root dependency guards')
for collection in [X['rows'],X['updateEntryPrologue'],X['directUpdateFPCalls'],X['localsEvidence']]+[f['prologue']+f['x87MemoryOperands'] for f in X['helperFrames']]+[w['instructions'] for w in X['comparisonWrappers']]+[w['instructions'] for w in X['scriptedScalarFrameChain']]:
 for row in collection:
  bs=bytes.fromhex(row['bytes']);ck(pe.get_data(row['rva'],len(bs))==bs,'own frozen instruction bytes '+hex(row['rva']))

def u(b,o):return struct.unpack_from('<I',b,o)[0]
def h(b,o):return struct.unpack_from('<H',b,o)[0]
def decode(fop):
 op=0xd8+((fop>>8)&7);mod=(fop>>6)&3;reg=(fop>>3)&7
 table={(0xd9,3):('FSTP',32),(0xdd,3):('FSTP',64),(0xd8,3):('FCOMP',32),(0xdc,3):('FCOMP',64)}
 return table.get((op,reg)),mod,op,reg
HELPER={'A+196':'decrement','Xdifference':'difference','Xproduct':'differenceProduct','Zdifference':'difference','initial_sqrt_argument':'differenceSum','cached_distance':'scalar32','pending_consumption_comparison':'scaledCompare','opponent_health':'damage','effectY_temporary':'add','effectX_temporary':'difference','effectZ_temporary':'difference','pool_i_X':'pool','pool_i_Y':'pool','pool_i_Z':'pool','combo_timer_comparison':'compare','absolute_argument':'scalar32','A+172':'scalar32','speedbranch_comparison':'scaledCompare','A+140':'progress','final_Xproduct':'product','final_sqrt_argument':'productSum','final_speed_comparison':'finalCompare','updated A+192 compared to1':'compare','retreat_distance_comparison':'compare','forward_distance_comparison':'compare','A+212 cooldown_duration':'cooldown','prelude_A196_comparison':'compare','retreat_distance_upper_comparison':'compare','retreat_X_comparison':'compare','movementchoice2_distance_comparison':'compare','movementchoice2_X_comparison':'compare'}
def original_fdp(entry,record,index,key):
 loc=entry['fdpLocator'];role=entry['expectedFdpRole'];event=record['events'][index] if index is not None else None
 if role.startswith('actor_plus'):return 0x5849f8+int(role[len('actor_plus'):]),role
 if role.startswith('global') or role.startswith('constant'):return int(loc,16),role
 if role.startswith('local_EBP_'):
  if not event:return None,role+' no frame'
  return u(bytes.fromhex(event['entry_regs36']),8)+int(loc,16),role
 if role=='outgoing_sqrt_float64_argument' or role=='outgoing_absolute_float32_argument':
  if not event:return None,role+' no stack'
  # Both cited sites capture the callee's argument after its return address;
  # pushal's saved ESP includes the observer ID/EFLAGS pair below that address.
  return u(bytes.fromhex(event['entry_regs36']),12)+12,role
 if role.startswith('pool'):
  site={'pool_i_X':0x1da04,'pool_i_Y':0x1da3b,'pool_i_Z':0x1da6c}[entry.get('outputRole',entry.get('resultRole',entry['checkpoint']))]
  upto=index+(key=='exit_fp544') if index is not None else len(record['events'])
  count=sum(e['call_rva']==site for e in record['events'][:upto]);i=count-1
  if not 0<=i<100:return None,role+' invalid loop index'
  first=int(loc.split('+')[0],16);return first+76*i,role+f' index{i}'
 return None,role+' unsupported locator'

HELPER.update({'actor_plus180':'decrement','actor_plus184':'decrement','actor_plus188':'decrement','actor_plus192':'cooldown','global576230':'multiplyAdd','global576258':'subtractProduct'})
for ip,e in O.items():
 role=e.get('outputRole',e.get('resultRole',e['checkpoint']))
 if role not in HELPER:
  HELPER[role]='scaledCompare' if ip in [0x41dc9b,0x41dcee] else 'compare'
I=json.loads((V/'GAME-0003-independent-fixture-stage2-fp-inventory.json').read_text());groups={}
for d in I['rawDifferenceInventory']:
 if d['classification']=='BLOCKED_PROVENANCE':groups.setdefault(d['checkpoint'],[]).append(d)
cache={}
for context,diffs in groups.items():
 m=re.fullmatch(r'pair(\d+):(?:event(\d+):site0x[0-9a-f]+:)?(entry_fp544|exit_fp544)',context);n,j,key=m.groups();n=int(n);j=int(j) if j is not None else None
 if n not in cache:
  a=D/f'damage-ai-fixture-original-{n:03}.json';b=D/f'damage-candidate-{n+1:03}.json'
  if not a.exists():a=D/f'damage-ai-guard-reference-{n:03}.json'
  if not b.exists():b=D/f'damage-original-{n+1:03}.json'
  cache[n]=[json.loads(p.read_text()) for p in (a,b)]
 a,b=cache[n];aa=bytes.fromhex(a[key] if j is None else a['events'][j][key]);bb=bytes.fromhex(b[key] if j is None else b['events'][j][key]);ip,cp=u(aa,524),u(bb,524);entry=O.get(ip)
 if ip in [base+0xa2337,base+0xa2363] and cp==ip:
  upto=j+(key=='exit_fp544') if j is not None else len(a['events'])
  origin=next((k for k in range(upto-1,-1,-1) if a['events'][k]['scalar80'] and u(bytes.fromhex(a['events'][k]['exit_fp544']),524)==ip),None)
  if origin is None:
   blocked.append({'checkpoint':context,'reason':'Own scalar source lineage absent','differences':diffs});continue
  vals=[]
  for rr in [a,b]:
   ev=rr['events'][origin];saved=u(bytes.fromhex(ev['entry_regs36']),12);expected=((saved-12)&~15)-(0x2f0 if ip==base+0xa2337 else 0x2f4);vals.append(expected)
  good=u(aa,532)==vals[0] and u(bb,532)==vals[1] and aa[524:532]==bb[524:532] and aa[536:538]==bb[536:538] and a['events'][origin]['args']==b['events'][origin]['args'] and a['events'][origin]['scalar80']==b['events'][origin]['scalar80']
  ck(good,context+' own scalar exact operands/result/frame')
  if not good:blocked.append({'checkpoint':context,'reason':'Own scalar frame mismatch','differences':diffs});continue
  proofs.append({'checkpoint':context,'classification':'SAME_FROZEN_SCALAR_TYPED_FRAME','originEvent':origin,'fip':hex(ip),'expectedFDP':vals,'frameFormula':'alignDown(savedPUSHADesp-12,16)-2F0/2F4; dispatcher align16/sub290, callback wrapper, Profile align8/sub78 + operand38/34 independently inspected current frozen v2','differences':diffs});continue
 if not entry:
  blocked.append({'checkpoint':context,'reason':'Original FIP not mapped (including reset/stale provenance)','originalFip':hex(ip),'candidateFip':hex(cp),'differences':diffs});continue
 expected,role=original_fdp(entry,a,j,key)
 if expected is None or u(aa,532)!=expected:
  blocked.append({'checkpoint':context,'reason':'Original FDP unresolved or wrong typed locator','expected':expected,'actual':u(aa,532),'role':role,'differences':diffs});continue
 od,om,oo,oreg=decode(h(aa,530));cd,cm,co,creg=decode(h(bb,530));want=(entry['operation'],entry['operandWidthBits'])
 if od!=want or om==3:
  blocked.append({'checkpoint':context,'reason':'Original operation does not match approved memory operation','differences':diffs});continue
 if cp==ip:
  other,other_role=original_fdp(entry,b,j,key)
  good=other is not None and u(bb,532)==other and cd==want and cm!=3
  cproof={'method':'Same original-body site in actual fallback','expectedFdp':other,'role':other_role}
 else:
  row=XP.get(cp-base);helper='attackCooldown' if ip in [0x41cf8d,0x41d2d6,0x41d61f] else HELPER[entry.get('outputRole',entry.get('resultRole',entry['checkpoint']))]
  if row is None or f'damage::fp::{helper}(' not in row['symbol']:
   blocked.append({'checkpoint':context,'reason':'Candidate FIP/helper unresolved (including reset)','candidateFip':hex(cp),'differences':diffs});continue
  fst=next(e for e in b['events'] if e['call_rva']==0x1c70b);S=fst['args'][0]-SP['localsOffsetFromUpdateFixedESP']
  frame=next(f for f in SP['helperFrames'] if f['symbol']==row['symbol']);operand=next((z for z in frame['x87MemoryOperands'] if z['rva']==row['rva']),None)
  if operand is None:
   blocked.append({'checkpoint':context,'reason':'Candidate operand frame unresolved','differences':diffs});continue
  delta=operand['operandDeltaFromCallerESP'];C=S-16 if entry.get('outputRole',entry.get('resultRole',entry['checkpoint'])) in ['combo_timer_comparison','forward_distance_comparison','retreat_distance_comparison','retreat_distance_upper_comparison','movementchoice2_distance_comparison'] or ip in [0x41cd1c,0x41cd33,0x41d051,0x41d068,0x41d39a,0x41d3b1,0x41dd91] else S;other=C+delta
  good=u(bb,532)==other and cd==want and cm!=3 and row['encodedFOP11']==h(bb,530)
  cproof={'method':'Frozen helper machine operand + typed L-POS-P local witness + approved arithmetic role','helper':row['symbol'],'candidateRva':hex(row['rva']),'candidateMemoryOperand':row['operand'],'updateS':hex(S),'helperCallerESP':hex(C),'operandDelta':delta,'expectedFdp':hex(other),'actualFdp':hex(u(bb,532)),'codeHash':X['sha256']}
 if not good:
  blocked.append({'checkpoint':context,'reason':'Candidate FDP/decoded operation mismatch','candidateProof':cproof,'differences':diffs});continue
 ck(h(aa,0)==h(bb,0)==0x027f and h(aa,2)==h(bb,2) and h(aa,520)==h(bb,520) and aa[4]==bb[4] and aa[160:288]==bb[160:288] and aa[24:28]==bb[24:28],context+' architectural gates')
 proof={'checkpoint':context,'originalFip':hex(ip),'candidateFip':hex(cp),'originalFdp':hex(u(aa,532)),'candidateFdp':hex(u(bb,532)),'originalFop':hex(h(aa,530)),'candidateFop':hex(h(bb,530)),'operation':want,'operandRole':entry.get('outputRole',entry.get('resultRole',entry['checkpoint'])),'originalDataProof':role,'candidateDataProof':cproof,'classification':'MAPPED_CORRESPONDING_INSTRUCTION_AND_DATA','differences':diffs};proofs.append(proof)

result={'status':'FAIL' if errors else 'BLOCKED_UNRESOLVED_PROVENANCE' if blocked else 'PASS_EXPLICIT_PROVENANCE_MAPPING','checks':checks,'errors':errors,'mappedCheckpoints':len(proofs),'unresolvedCheckpoints':len(blocked),'proofs':proofs,'blocked':blocked,'moduleProof':module,'codeHash':X['sha256'],'originalMapHash':hashlib.sha256((R/'specs/gameplay/GAME-0003-original-fp-provenance.json').read_bytes()).hexdigest()}
(V/'GAME-0003-independent-fixture-stage2-provenance-mapping.json').write_text(json.dumps(result,indent=2))
print(json.dumps({k:result[k] for k in ['status','checks','errors','mappedCheckpoints','unresolvedCheckpoints']}));print(json.dumps(blocked[:3],indent=2)[:2000])
