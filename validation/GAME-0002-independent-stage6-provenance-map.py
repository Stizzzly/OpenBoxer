"""Explicit corresponding x87 instruction/data role proof for typed fixtures."""
import json,re,struct,hashlib,pefile
from pathlib import Path
R=Path(r'C:\Users\ADMIN\CLionProjects\OpenBoxer');V=R/'validation';D=Path(r'C:\Users\ADMIN\Boxer-lab\ms3d\damage-fixture-observations-stage6')
original=json.loads((R/'specs/gameplay/GAME-0002-original-fp-provenance.json').read_text());O={int(x['originalFipVa'],16):x for x in original['entries']}
X=json.loads((V/'GAME-0002-replacement-v6-x87-map.json').read_text());SP=json.loads((V/'GAME-0002-replacement-v6-spill-map.json').read_text());XP={x['rva']:x for x in X['rows']}
module=json.loads((D/'damage-fixture-prepatch-observation.json').read_text());base=int(module['replacement_module_base'],16)
pe=pefile.PE(X['dll']);binary=Path(X['dll']).read_bytes();assert hashlib.sha256(binary).hexdigest()==X['sha256']==SP['sha256'];assert module['replacement_module_sha256'].lower()==X['sha256']
checks=0;errors=[];blocked=[];proofs=[]
def ck(ok,msg):
 global checks
 checks+=1
 if not ok:errors.append(msg)
for collection in [X['rows'],SP['updateEntryPrologue'],SP['directUpdateFPCalls']]:
 for row in collection:
  bs=bytes.fromhex(row.get('replacementBytes',row.get('bytes','')));ck(pe.get_data(row['rva'],len(bs))==bs,'replacement frozen bytes '+hex(row['rva']))
for frame in SP['helperFrames']:
 for row in frame['prologue']+frame['x87MemoryOperands']:
  bs=bytes.fromhex(row['bytes']);ck(pe.get_data(row['rva'],len(bs))==bs,'replacement helper bytes '+hex(row['rva']))
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
  site={'pool_i_X':0x1da04,'pool_i_Y':0x1da3b,'pool_i_Z':0x1da6c}[entry['outputRole']]
  upto=index+(key=='exit_fp544') if index is not None else len(record['events'])
  count=sum(e['call_rva']==site for e in record['events'][:upto]);i=count-1
  if not 0<=i<100:return None,role+' invalid loop index'
  first=int(loc.split('+')[0],16);return first+76*i,role+f' index{i}'
 return None,role+' unsupported locator'
inventory=json.loads((V/'GAME-0002-independent-stage6-fp-inventory.json').read_text());groups={}
for diff in inventory['rawDifferenceInventory']:
 if diff['classification']=='BLOCKED_PROVENANCE':groups.setdefault(diff['checkpoint'],[]).append(diff)
cache={}
resets=set() # No stage5 reset permission inherited; fresh scope requires approval.
ruling=json.loads((R/'specs/gameplay/GAME-0002-stage6-legacy-reset-ruling.json').read_text());allowed={z['checkpoint']:z for z in ruling['records']}
for context,diffs in groups.items():
 m=re.fullmatch(r'pair(\d+):(?:(?:event(\d+):site0x[0-9a-f]+):)?(entry_fp544|exit_fp544)',context);n,j,key=m.groups();n=int(n);j=int(j) if j is not None else None
 if n not in cache:
  paths=[D/f'damage-fixture-original-{n:03}.json',D/f'damage-candidate-{n+1:03}.json'] if n!=121 else [D/'damage-guard-reference-121.json',D/'damage-guard-candidate-122.json'];cache[n]=[json.loads(p.read_text()) for p in paths]
 a,b=cache[n];aa=bytes.fromhex(a[key] if j is None else a['events'][j][key]);bb=bytes.fromhex(b[key] if j is None else b['events'][j][key]);ip,cp=u(aa,524),u(bb,524);entry=O.get(ip)
 if context in allowed:
  rule=allowed[context];ck(aa[512:540].hex()==rule['originalLegacyRaw28'] and bb[512:540].hex()==rule['candidateLegacyRaw28'],context+' approved exact raw28')
  ck(h(aa,0)==h(bb,0)==0x027f and h(aa,2)==h(bb,2) and not(h(aa,2)&0x3840) and h(aa,520)==h(bb,520)==0xffff and aa[4]==bb[4]==0 and aa[160:288]==bb[160:288] and aa[24:32]==bb[24:32],context+' reset exact architecture')
  origreset=ip==0;reset=aa if origreset else bb;retained=bb if origreset else aa
  ck(all(u(reset,o)==0 for o in [524,532]) and all(h(reset,o)==0 for o in [528,530,536]),context+' all-five reset')
  originalRoleIp=int(re.search(r'41[0-9A-Fa-f]{4}',rule['expectedParentRole'])[0],16);roleEntry=O[originalRoleIp];helper=HELPER[roleEntry['outputRole']];want=(roleEntry['operation'],roleEntry['operandWidthBits'])
  points=[(k,z) for k in range(j+1) for z in ['entry_fp544','exit_fp544'] if k<j or z=='entry_fp544']
  lineage=None
  for k,z in reversed(points):
   av=bytes.fromhex(a['events'][k][z]);bv=bytes.fromhex(b['events'][k][z]);row=XP.get(u(bv,524)-base)
   if u(av,524)!=originalRoleIp or row is None or f'damage::fp::{helper}(' not in row['symbol']:continue
   prior=bv if origreset else av
   if any(prior[o:o+sz]!=retained[o:o+sz] for o,sz in [(524,4),(528,2),(530,2),(532,4),(536,2)]):continue
   exp,loc=original_fdp(roleEntry,a,k,z);frame=next(f for f in SP['helperFrames'] if f['symbol']==row['symbol']);operand=next(q for q in frame['x87MemoryOperands'] if q['rva']==row['rva']);S=next(e for e in b['events'] if e['call_rva']==0x1c70b)['args'][0]-0x4964;C=S-16 if roleEntry['outputRole']=='combo_timer_comparison' else S
   if u(av,532)!=exp or u(bv,532)!=C+operand['operandDeltaFromCallerESP'] or decode(h(av,530))[0]!=want or decode(h(bv,530))[0]!=want or h(bv,530)!=row['encodedFOP11']:continue
   lineage={'event':k,'boundary':z,'originalFip':hex(originalRoleIp),'role':roleEntry['outputRole'],'originalFdp':hex(exp),'candidateFip':hex(u(bv,524)),'candidateFdp':hex(u(bv,532)),'candidateFrameExpectedFdp':hex(C+operand['operandDeltaFromCallerESP'])};break
  if lineage is None and not origreset and roleEntry['outputRole'].startswith('pool_i_'):
   component=roleEntry['outputRole'][-1];producerSite={'X':0x1da04,'Y':0x1da3b,'Z':0x1da6c}[component];producers=[(k,e) for k,e in enumerate(b['events'][:j]) if e['call_rva']==producerSite];producerIndex,event=producers[-1];i=len(producers)-1
   expectedOriginal,locator=original_fdp(roleEntry,a,j,key);ck(u(aa,524)==originalRoleIp and u(aa,532)==expectedOriginal and decode(h(aa,530))[0]==want,context+' retained exact original pool role')
   def poolWord(rr,va):
    raw=bytes.fromhex(rr['after']['globals']);offset=0
    metadata=json.loads((R/'specs/gameplay/GAME-0002-observer-metadata.json').read_text())
    for span in metadata['capture']['globalExtents']:
     start=0x400000+int(span['rva'],16)
     if start<=va<start+span['size']:return u(raw,offset+va-start)
     offset+=span['size']
    raise KeyError(va)
   value=event['eax']%100;scale=struct.unpack('<f',bytes.fromhex('0ad7233c'))[0] if component=='Y' else 1;offset=60 if component=='X' else 0;expectedValue=struct.unpack('<I',struct.pack('<f',(value-50)*scale-offset))[0];va={'X':0x57f738,'Y':0x57f73c,'Z':0x57f740}[component]+76*i
   ck(a['events'][producerIndex]['eax']==event['eax'] and poolWord(a,va)==poolWord(b,va)==expectedValue,context+' exact source/candidate Rand operand and stored pool result')
   anchors=[q for q in SP['directUpdateFPCalls'] if 'damage::fp::pool(' in q['operand']];anchor=anchors[{'X':0,'Y':1,'Z':2}[component]];ck(pe.get_data(anchor['rva'],len(bytes.fromhex(anchor['bytes'])))==bytes.fromhex(anchor['bytes']),context+' frozen component arithmetic call anchor')
   S=next(e for e in b['events'] if e['call_rva']==0x1c70b)['args'][0]-0x4964;lineage={'method':'Frozen component-specific pool call anchor + exact typed Rand producer + exact original/candidate stored result; reset candidate metadata is not decoded as arithmetic','component':component,'iteration':i,'producerEvent':producerIndex,'producerSite':hex(producerSite),'rawRandEAX':event['eax'],'storeVA':hex(va),'storedRaw32':hex(expectedValue),'callAnchorRVA':hex(anchor['rva']),'candidateOperandExpected':hex(S-12),'originalLocator':locator}
  ck(lineage is not None,context+' exact original/candidate retained mapped lineage')
  if lineage is None:
   blocked.append({'checkpoint':context,'reason':'Approved reset lacks corresponding frame/role lineage','differences':diffs});continue
  proofs.append({'checkpoint':context,'classification':'EXPLICIT_FRESH_STAGE6_SCRIPTED_RESET_NONLIVE_LEGACY','direction':rule['resetDirection'],'lineage':lineage,'differences':diffs,'policy':'GAME-0002-stage6-legacy-reset-ruling.json'});continue
 if ip in [base+0x975b7,base+0x975e3] and cp==ip:
  # Fixed own compiled call chain, verified below. Saved PUSHAD ESP is target entry minus eight.
  anchors={0x657b0:'5589e583e4f081ec90020000',0x69720:'5589e583ec0c',0x69741:'e84ada0200',0x97190:'5589e583e4f883ec78',0x975b3:'dd442438d9fa',0x975e3:'d9442434'}
  for rv,raw in anchors.items():ck(pe.get_data(rv,len(bytes.fromhex(raw)))==bytes.fromhex(raw),context+' scalar own frame bytes '+hex(rv))
  upto=j+(key=='exit_fp544') if j is not None else len(a['events'])
  origin=next((k for k in range(upto-1,-1,-1) if a['events'][k]['scalar80'] and u(bytes.fromhex(a['events'][k]['exit_fp544']),524)==ip),None)
  if origin is None:
   blocked.append({'checkpoint':context,'reason':'Script scalar lineage missing','differences':diffs});continue
  vals=[]
  for rr in [a,b]:
   event=rr['events'][origin];saved=u(bytes.fromhex(event['entry_regs36']),12);expected=((saved-12)&~15)-(0x2f0 if ip==base+0x975b7 else 0x2f4);vals.append({'savedESP':hex(saved),'expectedFDP':hex(expected),'originEvent':origin,'identity':event['identity'],'scalar80':event['scalar80']})
  ck(u(aa,532)==int(vals[0]['expectedFDP'],16) and u(bb,532)==int(vals[1]['expectedFDP'],16),context+' exact scalar stack FDP')
  ck(aa[524:532]==bb[524:532] and aa[536:538]==bb[536:538],context+' scalar exact FIP/FOP/selectors')
  ck(a['events'][origin]['scalar80']==b['events'][origin]['scalar80'] and a['events'][origin]['args']==b['events'][origin]['args'],context+' same scripted scalar operands/result')
  proofs.append({'checkpoint':context,'classification':'SAME_FROZEN_SCRIPT_SCALAR_RELOCATED_OPERAND','fip':hex(ip),'proof':vals,'formula':'alignDown(savedPUSHADesp-12,16)-0x2F0 for FLDL/FSQRT; minus0x2F4 for FLDS','frameChain':'targetE=savedESP+8; dispatcherT=alignDown(E-20,16)-0x290; ProfileU=alignDown(T-28,8)-0x78=T-0x98; dataU+0x38 orU+0x34','differences':diffs});continue
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
  if context in resets and cp==0:
   ck(all(u(bb,o)==0 for o in [524,532]) and all(h(bb,o)==0 for o in [528,530,536]),context+' approved all-five reset')
   ck(h(aa,0)==h(bb,0)==0x027f and h(aa,2)==h(bb,2) and (h(aa,2)&0x40)==0 and (h(aa,2)&0x3800)==0 and h(aa,520)==h(bb,520)==0xffff and aa[4]==bb[4]==0 and aa[160:288]==bb[160:288] and aa[24:28]==bb[24:28],context+' approved reset exact architecture')
   groupstart=98 if n==107 else 11 if n==115 else 430
   ae=bytes.fromhex(a['events'][groupstart]['entry_fp544']);be=bytes.fromhex(b['events'][groupstart]['entry_fp544']);ri=XP.get(u(be,524)-base);helper=HELPER[entry['outputRole']]
   ck(u(ae,524)==ip and ri is not None and f'damage::fp::{helper}(' in ri['symbol'],context+' prior candidate same-event entry helper lineage')
   fst=next(e for e in b['events'] if e['call_rva']==0x1c70b);S=fst['args'][0]-0x4964
   fr=next(f for f in SP['helperFrames'] if f['symbol']==ri['symbol']);op=next(z for z in fr['x87MemoryOperands'] if z['rva']==ri['rva']);C=S-16 if entry['outputRole'] in ['combo_timer_comparison','forward_distance_comparison','retreat_distance_comparison','retreat_distance_upper_comparison','movementchoice2_distance_comparison'] else S
   ck(u(be,532)==C+op['operandDeltaFromCallerESP'] and h(be,530)==ri['encodedFOP11'],context+' reset candidate retained operand lineage')
   proofs.append({'checkpoint':context,'classification':'EXPLICIT_STAGE5_SCRIPTED_RESET_NONLIVE_LEGACY','originalOperandRole':entry['outputRole'],'originalDataProof':role,'lineageCandidateRva':ri['rva'],'differences':diffs,'policy':'GAME-0002-typed-fp-locals.md Stage5 ten named checkpoint ruling'});continue
  row=XP.get(cp-base);helper=HELPER[entry['outputRole']]
  if row is None or f'damage::fp::{helper}(' not in row['symbol']:
   blocked.append({'checkpoint':context,'reason':'Candidate FIP/helper unresolved (including reset)','candidateFip':hex(cp),'differences':diffs});continue
  fst=next(e for e in b['events'] if e['call_rva']==0x1c70b);S=fst['args'][0]-0x4964
  frame=next(f for f in SP['helperFrames'] if f['symbol']==row['symbol']);operand=next((z for z in frame['x87MemoryOperands'] if z['rva']==row['rva']),None)
  if operand is None:
   blocked.append({'checkpoint':context,'reason':'Candidate operand frame unresolved','differences':diffs});continue
  delta=operand['operandDeltaFromCallerESP'];C=S-16 if entry['outputRole'] in ['combo_timer_comparison','forward_distance_comparison','retreat_distance_comparison','retreat_distance_upper_comparison','movementchoice2_distance_comparison'] else S;other=C+delta
  good=u(bb,532)==other and cd==want and cm!=3 and row['encodedFOP11']==h(bb,530)
  cproof={'method':'Frozen helper machine operand + typed L-POS-P local witness + approved arithmetic role','helper':row['symbol'],'candidateRva':hex(row['rva']),'candidateMemoryOperand':row['operand'],'updateS':hex(S),'helperCallerESP':hex(C),'operandDelta':delta,'expectedFdp':hex(other),'actualFdp':hex(u(bb,532)),'codeHash':X['sha256']}
 if not good:
  blocked.append({'checkpoint':context,'reason':'Candidate FDP/decoded operation mismatch','candidateProof':cproof,'differences':diffs});continue
 ck(h(aa,0)==h(bb,0)==0x027f and h(aa,2)==h(bb,2) and h(aa,520)==h(bb,520) and aa[4]==bb[4] and aa[160:288]==bb[160:288] and aa[24:28]==bb[24:28],context+' architectural gates')
 proof={'checkpoint':context,'originalFip':hex(ip),'candidateFip':hex(cp),'originalFdp':hex(u(aa,532)),'candidateFdp':hex(u(bb,532)),'originalFop':hex(h(aa,530)),'candidateFop':hex(h(bb,530)),'operation':want,'operandRole':entry['outputRole'],'originalDataProof':role,'candidateDataProof':cproof,'classification':'MAPPED_CORRESPONDING_INSTRUCTION_AND_DATA','differences':diffs};proofs.append(proof)
result={'status':'FAIL' if errors else 'BLOCKED_UNRESOLVED_PROVENANCE' if blocked else 'PASS_EXPLICIT_PROVENANCE_MAPPING','checks':checks,'errors':errors,'mappedCheckpoints':len(proofs),'mappedRawDifferences':sum(len(p['differences']) for p in proofs),'unresolvedCheckpoints':len(blocked),'proofs':proofs,'blocked':blocked,'moduleProof':module,'sourceMapHashes':{str(p):hashlib.sha256(p.read_bytes()).hexdigest() for p in [V/'GAME-0002-replacement-v6-x87-map.json',V/'GAME-0002-replacement-v6-spill-map.json',R/'specs/gameplay/GAME-0002-original-fp-provenance.json']}}
(V/'GAME-0002-independent-stage6-provenance-mapping.json').write_text(json.dumps(result,indent=2));print(json.dumps({k:result[k] for k in ['status','checks','errors','mappedCheckpoints','mappedRawDifferences','unresolvedCheckpoints']}));print(json.dumps(blocked[:4],indent=2)[:2500])
