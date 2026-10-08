import json,struct,hashlib,re,sys
from pathlib import Path
R=Path(r'C:\Users\ADMIN\CLionProjects\OpenBoxer');V=R/'validation';D=Path(r'C:\Users\ADMIN\Boxer-lab\ms3d\damage-fixture-observations-stage2')
if len(sys.argv)>1:D=Path(sys.argv[1])
meta=json.loads((R/'specs/gameplay/GAME-0002-observer-metadata.json').read_text(encoding='utf-8-sig'));sites={int(c['callRva'],16):c for c in meta['calls']};spans=meta['capture']['globalExtents'];intent=json.loads((V/'GAME-0002-independent-vector-intents.json').read_text(encoding='utf-8-sig'))['vectors'];checks=0;errors=[];dead=[];records=[]
def bs(s):return bytes.fromhex(s)
def u(b,o=0):return struct.unpack_from('<I',b,o)[0]
def sha(p):return hashlib.sha256(p.read_bytes()).hexdigest()
localsites={0x1c70b:('L-POS-P','arg'),0x1c740:('L-POS-O','arg'),0x1cac6:('L-NEG','arg'),0x1cad6:('L-MUL-F','arg'),0x1cb7b:('L-MUL-R','arg'),0x1d775:('L-HIT','owner'),0x1d900:('L-HIT-POS','arg'),0x1d9e8:('L-POOL-V','arg'),0x1e04c:('L-TAIL-POS','arg'),0x1e079:('L-ZERO','owner'),0x1e08e:('L-ZERO','arg'),0x1e0a3:('L-TAIL-V','arg'),0x1e145:('L-ADD','arg'),0x1e17e:('L-MUL-S','arg'),0x1e2f3:('L-LIMIT','arg')}
mapping={'AIindex_int32':('g',0x184788),'attackTimer_raw32':('g',0x1761cc),'block_byte':('gb',0x176234),'counter_raw32':('g',0x17620c),'decision204_byte':('ab',204),'difficulty_int32':('g',0x1847c4),'marker_byte':('gb',0x17628c),'pending_byte':('gb',0x17622c),'previousType_u32':('g',0x176224),'readiness_int32':('a',208),'sound_byte':('gb',0x177f9e),'timer180_raw32':('a',180),'timer184_raw32':('a',184),'timer188_raw32':('a',188),'actorState_u32':('a',164),'blockTimer_raw32':('a',228),'cooldown192_raw32':('a',192),'mode_int32':('mode',0),'opponentX_raw32':('body',752),'absoluteCallback_promoteRaw32':('absolute',0),'finalSqrtCallback_promoteRaw32':('sqrt',0)}
def globals_of(s):
 data=bs(s['globals']);g={};p=0
 for span in spans:g[int(span['rva'],16)]=data[p:p+span['size']];p+=span['size']
 return g
def val(g,addr):
 for base,b in g.items():
  if base<=addr<base+len(b):return b[addr-base:]
 raise KeyError(hex(addr))
def roles(d):
 s=d['before'];r={s['player_body']:'PLAYERBODY',s['opponent_body']:'OPPONENTBODY',s['world']:'WORLD',0x5849f8:'ACTOR',0x584910:'PLAYER'}
 for a,name in [(0x5761dc,'POSPLAYER'),(0x576240,'POSOPPONENT'),(0x5848e0,'EFFECT'),(0x575df0,'AUDIOWALK'),(0x578070,'AUDIOBLOCK'),(0x578050,'AUDIOHIT'),(0x585340,'AUDIO5'),(0x585320,'AUDIOOTHER')]:r[a]=name
 for off in [12,28,44,76,92,108,124]:r[0x5849f8+off]='ACTOR+'+str(off)
 for e in d['events']:
  if e['call_rva'] in localsites:
   name,where=localsites[e['call_rva']];a=e['owner'] if where=='owner' else e['args'][0]
   if a in r and r[a]!=name:raise ValueError('role alias')
   r[a]=name
  if e['call_rva']==0x1e1f4:r[e['args'][2]]='L-AUDIO'
 return r
for index,it in enumerate(intent):
 name=it['id'];ep=[]
 def eq(a,b,msg):
  global checks
  checks+=1
  if a!=b:ep.append({'field':msg,'original':str(a)[:160],'candidate':str(b)[:160]})
 def fp(x,y,msg):
  a=bs(x);b=bs(y);eq(len(a),544,msg+' original extent');eq(len(b),544,msg+' candidate extent')
  for off,n,role in [(0,2,'CW'),(2,2,'SW'),(4,1,'abridgedtag'),(24,4,'MXCSR'),(512,2,'envCW'),(516,2,'envSW'),(520,2,'fulltag')]:eq(a[off:off+n],b[off:off+n],msg+role)
  eq(a[0:2],a[512:514],msg+' original duplicateCW');eq(b[0:2],b[512:514],msg+' candidate duplicateCW');eq(a[2:4],a[516:518],msg+' original duplicateSW');eq(b[2:4],b[516:518],msg+' candidate duplicateSW')
  top=(u(a,2)>>11)&7;tags=u(a,520)&65535
  for logical in range(8):
   aa=a[32+16*logical:42+16*logical];bb=b[32+16*logical:42+16*logical];physical=(top+logical)%8
   if ((tags>>(physical*2))&3)!=3:eq(aa,bb,msg+f'liveST{logical}')
   elif aa!=bb:dead.append({'case':name,'field':msg,'slot':logical,'original':aa.hex(),'candidate':bb.hex()})
 paths=[D/f'damage-fixture-original-{2*index+1:03}.json',D/f'damage-candidate-{2*index+2:03}.json'];o,c=[json.loads(p.read_text()) for p in paths];maps=[roles(o),roles(c)]
 def norm(addr,side):return maps[side].get(addr,('RAW',addr))
 for d,side in [(o,0),(c,1)]:
  eq(d['candidate'],bool(side),'candidate identity');eq(d['nested_whole_observed'],False,'nested');eq(d['module'],0x400000,'module')
  for label in ['before','after']:
   s=d[label];eq(s['global_span_sizes'],[p['size'] for p in spans],'span lengths')
   for k,n in [('actor232',232),('player176',176),('player_body2288',2288),('opponent_body2288',2288),('world_list12',12),('globals',sum(p['size'] for p in spans))]:eq(len(bs(s[k])),n,label+k+' extent')
  g=globals_of(d['before']);a=bs(d['before']['actor232'])
  for key,v in it['patch'].items():
   v=int(v,16) if isinstance(v,str) else v;kind,off=mapping[key]
   if kind in ['g','gb']:got=val(g,off)[0] if kind=='gb' else u(val(g,off))
   elif kind in ['a','ab']:got=a[off] if kind=='ab' else u(a,off)
   elif kind=='body':got=u(bs(d['before']['opponent_body2288']),off)
   elif kind=='mode':got=d['mode']
   else:continue
   eq(got,v,'intent '+key)
 for k in ['mode','eax','thread']:eq(o[k],c[k],k)
 for label in ['before','after']:
  for k in ['actor232','player176','player_body2288','opponent_body2288','globals','world_list12']:eq(o[label][k],c[label][k],label+k)
 fp(o['entry_fp544'],c['entry_fp544'],'whole entry');fp(o['exit_fp544'],c['exit_fp544'],'whole exit')
 eq(len(o['events']),len(c['events']),'event count')
 for j,(a,b) in enumerate(zip(o['events'],c['events'])):
  prefix='event'+str(j)+':'
  for k in ['ordinal','call_rva','return_rva','identity','rng_before','rng_after','owner_before','owner_after','combat_before16','combat_after16','arg_before','arg_after','returned_bytes']:eq(a[k],b[k],prefix+k)
  s=sites.get(a['call_rva']);eq(s is not None,True,prefix+'metadata')
  if not s:continue
  eq(a['ordinal'],j,prefix+'ordinal');eq(a['identity'],s['identity'],prefix+'expected identity')
  if s['ecx']:eq(norm(a['owner'],0),norm(b['owner'],1),prefix+'owner')
  eq(len(a['args']),len(b['args']),prefix+'arg count')
  for k,(x,y) in enumerate(zip(a['args'],b['args'])):
   typ=s['args'][k] if k<len(s['args']) else 'BYVALUEWORD'
   eq(norm(x,0) if 'pointer' in typ else x,norm(y,1) if 'pointer' in typ else y,prefix+f'arg{k}')
  if s['ret']=='element_pointer_EAX':eq((norm(a['owner'],0),a['eax']-a['owner']),(norm(b['owner'],1),b['eax']-b['owner']),prefix+'elementEAX')
  elif s['ret']=='output_pointer_EAX':eq(norm(a['eax'],0),norm(b['eax'],1),prefix+'outputEAX')
  else:eq(a['eax'],b['eax'],prefix+'rawEAX')
  if 'scalar80' in a or 'scalar80' in b:eq(a.get('scalar80'),b.get('scalar80'),prefix+'scalar80')
  fp(a['entry_fp544'],b['entry_fp544'],prefix+'entry');fp(a['exit_fp544'],b['exit_fp544'],prefix+'exit')
  for d in [a,b]:
   er=bs(d['entry_regs36']);xr=bs(d['exit_regs36'])
   for off in [0,4,8,16]:eq(u(er,off),u(xr,off),prefix+'callee saved')
   pop=int(s['abi'].split('ret')[-1]) if s['abi'].startswith('thiscall') else 0;eq((u(xr,12)-u(er,12))&0xffffffff,4+pop,prefix+'ESP')
 # Contract assertions independent of both sides: hit outcome and callback counts.
 for d in [o,c]:
  g=globals_of(d['after']);ga=globals_of(d['before']);asserts=it['assertions'];ev=d['events'];A=bs(d['after']['actor232'])
  tests={'consume':val(ga,0x17622c)[0]==1 and val(g,0x17622c)[0]==0,'poolGetters':sum(e['call_rva']==0x1d9e8 for e in ev),'poolRng':sum(e['call_rva'] in [0x1da04,0x1da3b,0x1da6c] for e in ev),'decisionRngCount':sum(e['call_rva'] in [0x1cc19,0x1cc3e,0x1cc67] for e in ev),'health_raw32':val(g,0x176238)[:4][::-1].hex().upper(),'finalBlock':val(g,0x176234)[0],'finalState':u(A,164),'comboTimer_raw32':val(g,0x176260)[:4][::-1].hex().upper(),'comboCount_u32':u(val(g,0x176270)),'aux288_int32':u(val(g,0x176288)),'actor220_int32':u(A,220),'movement171_byte':A[171],'consumptionPlayCount':sum(e['call_rva'] in [0x1d7e1,0x1d7ed,0x1d80b,0x1d817] for e in ev),'tailQueryCount':sum(e['call_rva']==0x1e1f4 for e in ev),'tailBranch':'add' if any(e['call_rva']==0x1e145 for e in ev) else 'multiply','limit':any(e['call_rva']==0x1e305 for e in ev)}
  for key,want in asserts.items():
   if key in tests:eq(tests[key],want,'contract '+key)
  if 'modulo' in asserts and tests['decisionRngCount']:
   rd=next(e for e in ev if e['call_rva'] in [0x1cc19,0x1cc3e,0x1cc67]);eq(u(A,224),rd['eax']%asserts['modulo']+1,'contract modulo')
  if 'counterModulo32' in asserts:beforecount=u(val(ga,0x17620c));eq(u(val(g,0x17620c)),(beforecount+int(asserts['increment']))&0xffffffff,'contract wrap')
 records.append({'case':name,'index':index,'errors':ep,'originalPath':str(paths[0]),'candidatePath':str(paths[1]),'originalSha256':sha(paths[0]),'candidateSha256':sha(paths[1]),'events':len(o['events'])});errors.extend({'case':name,**e} for e in ep)
report=(D/'damage-fixtures.txt').read_text();abi=re.findall(r'candidate=([01]).*?replacement=(\d+) fallback=(\d+) route=(\d+) abi=(\d+) esp=([0-9a-f]+)/([0-9a-f]+)',report)
checks+=1
if len(abi)!=120:errors.append({'field':'ABI report row count','count':len(abi)})
for row in abi:
 checks+=1
 if row[3:5]!=('1','1') or row[5]!=row[6] or (row[0]=='1' and row[1:3]!=('1','0')):errors.append({'field':'ABI witness','row':row})
checks+=1
if 'cleanup observer_restored=1 failures=0' not in report:errors.append({'field':'cleanup'})
result={'status':'FAIL' if errors else 'PASS_SCRIPTED_PAIRS_PENDING_FP_POLICY','checks':checks,'pairs':len(records),'differences':len(errors),'errors':errors[:200],'records':records,'deadRaw80Differences':dead[:100],'deadRaw80DifferenceCount':len(dead),'comparisonPolicy':'EXACT all snapshots/state/data/callback values; only approved pointer roles normalize. FP exact CW/SW/tags/MXCSR/live80; FP reserved/dead slots and FIP/FDP listed separately pending approved policy. Full source retained. ABI report independently audited.','equivalence':'BLOCKED_NATURAL_REPLAY_AND_ACTIVATION'}
(Path(sys.argv[2]) if len(sys.argv)>2 else V/'GAME-0002-independent-stage2-comparison.json').write_text(json.dumps(result,indent=2));print(json.dumps({'status':result['status'],'checks':checks,'pairs':len(records),'differences':len(errors),'firstErrors':errors[:20],'deadRaw80Differences':len(dead)},indent=2))
