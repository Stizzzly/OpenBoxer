import json, struct, math, hashlib, sys
from pathlib import Path
ROOT=Path(r'C:\Users\ADMIN\CLionProjects\OpenBoxer')
STAGE=sys.argv[1] if len(sys.argv)>1 else 'stage1'
SRC=Path(sys.argv[1]);PID=int(sys.argv[2]);DLLBASE=int(sys.argv[3],0);OWNMAP=json.loads((ROOT/'validation/GAME-0004-independent-replacement-v1-fp-map.json').read_text());ENTRYRVA=OWNMAP['updateEntryPrologue'][0]['rva']
META=json.loads((ROOT/'specs/gameplay/GAME-0004-observer-metadata.json').read_text(encoding='utf-8-sig'))
SITES={int(x['callRva'],16):x for x in META['calls']}
SPANS=[(int(x['rva'],16),x['size']) for x in META['capture']['globalExtents']]
raw=bytes.fromhex
def u(b,o=0):return struct.unpack_from('<I',b,o)[0]
def f(b,o=0):return struct.unpack_from('<f',b,o)[0]
def finite(b,o=0,bound=10000):
 x=f(b,o);return math.isfinite(x) and abs(x)<=bound and (x==0 or abs(x)>=2**-126)
def digest(p):return hashlib.sha256(p.read_bytes()).hexdigest()
reports=[];exports=[];checks=0
TYPES={1:(0x1cdce,0x1cf9f,0x1cfb1),2:(0x1d103,0x1d2e8,0x1d2fa),3:(0x1d44c,0x1d631,0x1d643)}
NEW={x for v in TYPES.values() for x in v}
LOCAL={0x1c70b:('POS-P',0x98,'arg'),0x1c740:('POS-O',0xa8,'arg'),0x1cac6:('NEG',0xb8,'arg'),0x1cad6:('MUL-F',0xc8,'arg'),0x1cb7b:('MUL-R',0xd8,'arg'),0x1cf9f:('INIT-Z',0x1c,'owner'),0x1d2e8:('INIT-X',0x2c,'owner'),0x1d631:('INIT-C',0x3c,'owner'),0x1e04c:('TAIL-POS',0x108,'arg'),0x1e079:('ZERO',0x84,'owner'),0x1e0a3:('TAIL-V',0x118,'arg'),0x1e145:('ADD',0x128,'arg'),0x1e17e:('MUL-S',0x138,'arg'),0x1e2f3:('LIMIT',0x148,'arg')}
for path in sorted(SRC.glob('ai-candidate-*.json')):
 d=json.loads(path.read_text());
 if not d['admitted']:continue
 bad=[];reject=[];roles={};fp_inventory=[]
 def check(value,label):
  global checks
  checks+=1
  if not value:bad.append(label)
 def admit(value,label):
  global checks
  checks+=1
  if not value:reject.append(label)
 gs={}
 for label in ['before','after']:
  s=d[label]
  for key,n in [('actor232',232),('player176',176),('player_body2288',2288),('opponent_body2288',2288),('world_list12',12)]:check(len(raw(s[key]))==n,label+':'+key)
  check(s['global_span_sizes']==[n for _,n in SPANS],label+':spans')
  image=raw(s['globals']);check(len(image)==sum(n for _,n in SPANS),label+':globals')
  p=0;gs[label]={}
  for r,n in SPANS:gs[label][r]=image[p:p+n];p+=n
 def g(r,label='before'):
  for base,image in gs[label].items():
   if base<=r<base+len(image):return image[r-base:]
  raise KeyError(hex(r))
 A=raw(d['before']['actor232']);AA=raw(d['after']['actor232']);P=raw(d['before']['player_body2288']);B=raw(d['before']['opponent_body2288']);bp=d['before']['player_body'];bo=d['before']['opponent_body'];world=d['before']['world']
 check(d['format']=='GAME-0004-observer-v1' and d['observation_only'] and d['candidate'] and not d['nested_whole_observed'],'original record')
 check(d['module']==0x400000 and d['caller']==0x42d8eb and d['mode'] in [0,1] and isinstance(d['thread'],int) and d['thread']>0,'route/thread')
 check(d['eax']==u(raw(d['exit_regs36']),28),'whole EAX capture')
 def abi(entry,exit,pop,label):
  er=raw(entry);xr=raw(exit);check(len(er)==36 and len(xr)==36,label+':register extent')
  for off in [0,4,8,16]:check(u(er,off)==u(xr,off),label+':nonvolatile'+str(off))
  check((u(xr,12)-u(er,12))&0xffffffff==4+pop,label+':ESP')
 abi(d['entry_regs36'],d['exit_regs36'],4,'whole')
 admit(u(g(0x184790))==d['mode'] and not g(0x184794)[0] and g(0x18479d)[0]!=0 and u(g(0x177f90))==0,'active mode')
 pi=u(g(0x184784));ai=u(g(0x184788));admit(pi<=7 and 1<=ai<=7,'fighter indices')
 if d['mode']:admit(1<=u(g(0x1847c4))<=5,'difficulty')
 for r in [0x1761c9,0x17622c,0x17622e,0x176234,0x176256,0x176254,0x17626c,0x17628c]:admit(g(r)[0]==0,'flag'+hex(r))
 admit(u(g(0x176270))==0,'combo count')
 admit(u(g(0x17625c)) in [1,2,3] and u(A,164) in [2*u(g(0x17625c))+1,2*u(g(0x17625c))+2] and 1<=u(A,216)<=3 and u(A,208)<=1,'actor state/type/readiness')
 for off in [168,169,170,171,176,177,178,204]:admit(A[off] in [0,1],'actor bool'+str(off))
 for off in [28,32,36,44,48,52,108,112,116,140,180,184,188,192,196,228]:admit(finite(A,off),'actor scalar'+str(off))
 for r in [0x1761d4,0x176238,0x176258,0x176260,0x176370,0x176230]:admit(finite(g(r)),'global scalar'+hex(r))
 admit(f(g(0x1761d4))>=0 and f(g(0x176238))>0,'health')
 admit(-10000<=f(g(0x176258))<99 and f(g(0x176260))<=struct.unpack('<f',bytes.fromhex('cdcccc3f'))[0] and 0<=f(g(0x176370))<=10000,'fatigue combo dt')
 duration=f(g(0x1762a4+16*ai));admit(finite(g(0x1762a4+16*ai)) and duration>0 and 0<=f(g(0x176230))<=10000,'AI timer/duration')
 head,tail,count=struct.unpack('<III',raw(d['before']['world_list12']));admit(count==2 and {head,tail}=={bp,bo} and bp!=bo and world!=0,'world list')
 ranges=[(bp,2288),(bo,2288),(world+10968,12)]+[(0x400000+r,n) for r,n in SPANS]+[(0x584910,176),(0x5849f8,232)]
 for i,(a,n) in enumerate(ranges[:3]):
  for b,m in ranges[i+1:]:admit(not(a<b+m and b<a+n),'body/static alias')
 admit(u(A,152)==bo and u(raw(d['before']['player176']),152)==bp,'actor body ownership')
 for address,body in [(bp,P),(bo,B)]:
  for off,value in [(0,address),(216,0),(244,world),(652,0),(2276,tail if address==head else 0),(2280,head if address==tail else 0)]:admit(u(body,off)==value,'body witness'+str(off))
  admit(body[2284]==1,'body active')
  for off in ([752,756,760] if address==bp else [408,656,660,664,668,752,756,760,768,772,776,816,820,824,832,836,840,848,852,856]):admit(finite(body,off),'body scalar'+str(off))
 entry=raw(d['entry_fp544']);admit(u(entry)&65535==0x27f and (u(entry,2)&0x3840)==0 and entry[4]==0 and (u(entry,520)&65535)==65535,'entry FP')
 images=[('whole-entry',d['entry_fp544']),('whole-exit',d['exit_fp544'])]
 rng=u(g(0x171b90));local_spans=[]
 for i,e in enumerate(d['events']):
  site=e['call_rva'];s=SITES.get(site);check(e['ordinal']==i and s is not None,'event ordinal/site')
  if not s:continue
  check(e['identity']==s['identity'] and e['return_rva']==int(s['returnRva'],16),'typed identity/return')
  pop=int(s['abi'].split('ret')[-1]) if s['abi'].startswith('thiscall') else 0
  abi(e['entry_regs36'],e['exit_regs36'],pop,'event'+str(i));check(e['owner']==u(raw(e['entry_regs36']),24),'owner ECX')
  check(len(e['args'])==(pop//4 if s['abi'].startswith('thiscall') else int(s['abi'].split('caller')[-1])//4),'args extent')
  check(e['eax']==u(raw(e['exit_regs36']),28),'EAX capture')
  check(e['rng_before']==rng,'RNG chaining');rng=e['rng_after']
  if e['identity']=='_rand':check(e['eax']<=32767,'rand domain')
  else:check(e['rng_before']==e['rng_after'],'nonrand RNG unchanged')
  size=2288 if s['writes']=='opponent_body' else 232 if s['identity']=='sub_401D4D' else 16 if s['identity'] in ['sub_401839','sub_4019C9','sub_401B1D','sub_401B36'] else 0
  check(len(raw(e['owner_before']))==size and len(raw(e['owner_after']))==size,'owner extent')
  if s['ret'].startswith('raw80'):check(e['scalar80']==e['exit_fp544'][64:84],'scalar80')
  images.extend((('event%d-%s'%(i,key),e[key]) for key in ['entry_fp544','exit_fp544']))
  if site in LOCAL:
   role,off,where=LOCAL[site];actual=e['owner'] if where=='owner' else e['args'][0];parent=u(raw(e['entry_regs36']),8)
   idx={'POS-P':0,'POS-O':1,'NEG':2,'MUL-F':3,'MUL-R':4,'INIT-Z':15,'INIT-X':16,'INIT-C':17,'TAIL-POS':9,'ZERO':10,'TAIL-V':11,'ADD':12,'MUL-S':13,'LIMIT':14}[role];check(actual==roles.get('POS-P',actual)+16*idx,'own array local'+role);check(role not in roles or roles[role]==actual,'local lifetime'+role);roles[role]=actual
  if site==0x1e1f4:
   check('AUDIO' not in roles or roles['AUDIO']==e['args'][2],'own scalar local AUDIO');roles['AUDIO']=e['args'][2]
  if s['writes']=='opponent_body':
   check(e['owner']==bo,'mutating opponent identity')
   body=raw(e['owner_after'])
   for off in [768,772,776]:check(finite(body,off,40000),'generated velocity bound')
 for label,image in images:
  b=raw(image);check(len(b)==544,label+':FP extent');check(b[:2]==b[512:514] and b[2:4]==b[516:518],label+':duplicate CW/SW')
  tags=u(b,520)&65535;top=(u(b,2)>>11)&7
  for slot in range(8):check((((tags>>(2*slot))&3)==3)==(not bool(b[4]&(1<<slot))),label+':tags')
  fp_inventory.append({'checkpoint':label,'sha256':hashlib.sha256(b).hexdigest(),'cw':u(b)&65535,'sw':u(b,2)&65535,'top':top,'fullTags':tags,'mxcsr':u(b,24),'rawBytesPreserved':True,'XMMBytes':128})
 check(len(set(roles.values()))==len(roles),'distinct local roles')
 for role,address in roles.items():
  size=4 if role=='AUDIO' else 16
  for base,n in ranges:check(not(address<base+n and base<address+size),'local/static/body alias'+role)
 for e,body,owner in [(d['events'][0],P,bp),(d['events'][1],B,bo)]:
  check(e['call_rva'] in [0x1c70b,0x1c740] and e['owner']==owner and raw(e['returned_bytes'])==body[752:768],'initial pure getter source')
 check(rng==u(g(0x171b90,'after')),'final RNG');check(g(0x17f710)==g(0x17f710,'after'),'pool unchanged');check(g(0x1762a0)==g(0x1762a0,'after'),'fighter table unchanged');check(g(0x1761d4)[:4]==g(0x1761d4,'after')[:4],'player health unchanged')
 import importlib.util
 spec=importlib.util.spec_from_file_location('policy',ROOT/'validation/GAME-0004-independent-record-policy.py');policy=importlib.util.module_from_spec(spec);spec.loader.exec_module(policy);transition=policy.transition(d)
 for issue in transition['errors']:check(False,issue)
 type=u(g(0x17625c));check(d['continuation_type']==type and d['continuation_class']==(2 if transition['classification']=='completion' else 1),'independent continuation classifier')
 check(g(0x17622d)[0]==1 and g(0x1761c8)[0]<=1,'active/pending domain');check(not any(e['call_rva'] in NEW for e in d['events']),'no initiation effects')
 if transition['classification']=='completion':admit(finite(g(0x1762bc)),'fixed completion factor')
 check(d['requested_side']=='candidate' and d['actual_route']=='candidate' and d['candidate_route'] and d['candidate_invocations']==1 and d['original_invocations']==0 and d['actual_entrypoint']==DLLBASE+ENTRYRVA and d['effects_before_fallback']==0 and d['process']==PID,'actual original dispatch witness')
 report={'id':path.stem,'sha256':digest(path),'events':len(d['events']),'continuationType':type,'continuationClass':transition['classification'],'mode':d['mode'],'playerIndex':pi,'AIindex':ai,'errors':bad,'admissionReject':reject,'localPointerRoles':roles,'fpInventory':fp_inventory}
 reports.append(report)
 if not bad and not reject:exports.append((path,d,report))
result={'status':'PASS_CANDIDATE_TYPED_NATIVE_RECORDS_ONLY' if not any(x['errors'] or x['admissionReject'] for x in reports) else 'FAIL_CAPTURE_OR_ADMISSION','checks':checks,'records':reports};(ROOT/('validation/GAME-0004-independent-native-candidate-'+SRC.name+'.json')).write_text(json.dumps(result,indent=2));print({k:v for k,v in result.items() if k!='records'});print([(x['id'],x['continuationType'],x['continuationClass'],x['errors'],x['admissionReject']) for x in reports])
