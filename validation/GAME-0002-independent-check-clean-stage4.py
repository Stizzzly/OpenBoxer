import json,struct,hashlib,math
from pathlib import Path
root=Path(r'C:\Users\ADMIN\CLionProjects\OpenBoxer')
src=Path(r'C:\Users\ADMIN\Boxer-lab\ms3d\damage-original-clean-stage4')
meta=json.loads((root/'specs/gameplay/GAME-0002-observer-metadata.json').read_text(encoding='utf-8-sig'))
sites={int(c['callRva'],16):c for c in meta['calls']}; spans=meta['capture']['globalExtents']
def raw(s):return bytes.fromhex(s)
def u(b,o=0):return struct.unpack_from('<I',b,o)[0]
def f(b,o=0):return struct.unpack_from('<f',b,o)[0]
def cwfp(s):
 b=raw(s);return {'cw':u(b)&65535,'sw':u(b,2)&65535,'top':(u(b,2)>>11)&7,'tags':u(b,520)&65535,'mxcsr':u(b,24)}
reports=[];checks=0;errors=[];extractions=[]
for n in range(1,41):
 path=src/f'damage-original-{n:03}.json';d=json.loads(path.read_text());bad=[];unsupported=[]
 def check(v,msg):
  global checks
  checks+=1
  if not v:bad.append(msg)
 check(d['module']==0x400000 and d['caller']==0x42d8eb and d['mode'] in [0,1],'route')
 check(not d['nested_whole_observed'],'nested');check(d.get('thread') is not None,'thread')
 gs={}
 for label in ['before','after']:
  s=d[label]
  for k,z in [('actor232',232),('player176',176),('player_body2288',2288),('opponent_body2288',2288),('world_list12',12)]:check(len(raw(s[k]))==z,label+':'+k)
  check(s['global_span_sizes']==[v['size'] for v in spans],label+':spans')
  b=raw(s['globals']);check(len(b)==sum(v['size'] for v in spans),label+':globals');p=0;g={}
  for sp in spans:g[int(sp['rva'],16)]=b[p:p+sp['size']];p+=sp['size']
  gs[label]=g
 def get(rva,label='before'):
  for base,b in gs[label].items():
   if base<=rva<base+len(b):return b[rva-base:]
  raise KeyError(hex(rva))
 A=raw(d['before']['actor232']);B=raw(d['before']['opponent_body2288']);P=raw(d['before']['player_body2288'])
 fp=cwfp(d['entry_fp544']);check(fp['cw']==0x27f and fp['top']==0 and fp['tags']==65535,'entryFP')
 for k in ['entry_regs36','exit_regs36']:check(len(raw(d[k]))==36,k)
 er=raw(d['entry_regs36']);xr=raw(d['exit_regs36'])
 for off in [0,4,8,16]:check(u(er,off)==u(xr,off),'whole nonvolatile'+str(off))
 check((u(xr,12)-u(er,12))&0xffffffff==8,'whole ESPret4')
 for r,val in [(0x17622d,0),(0x17622e,0),(0x176256,0),(0x176254,0),(0x17626c,0),(0x184794,0)]:
  if get(r)[0]!=val:unsupported.append(hex(r)+' excluded byte='+str(get(r)[0]))
 if u(get(0x176270)):unsupported.append('comboCount')
 if u(A,216) not in [1,2,3]:unsupported.append('actor216')
 if u(A,208) not in [0,1]:unsupported.append('readiness')
 if u(A,164)>11 or u(A,164)==10:unsupported.append('state')
 for off in [171,176,177,178,204]:
  if A[off] not in [0,1]:unsupported.append('actorbyte'+str(off))
 for name,body,address in [('player',P,d['before']['player_body']),('opponent',B,d['before']['opponent_body'])]:
  for off,value in [(0,address),(216,0),(244,d['before']['world']),(652,0)]:
   if u(body,off)!=value:unsupported.append(name+' witness'+str(off))
  if body[2284]!=1:unsupported.append(name+' active')
 for off in [28,32,36,44,48,52,108,112,116,140,180,184,188,192,196,228]:
  val=f(A,off)
  if not math.isfinite(val) or abs(val)>10000 or 0<abs(val)<2**-126:unsupported.append('actor scalar'+str(off))
 pool_before=get(0x17f710);pool_after=get(0x17f710,'after');rnglast=u(get(0x171b90));rngs=[];getters=[]
 for i,e in enumerate(d['events']):
  check(e['ordinal']==i,'ordinal');s=sites.get(e['call_rva']);check(s is not None,'site')
  if not s:continue
  check(e['identity']==s['identity'] and e['return_rva']==int(s['returnRva'],16),'site identity')
  for k in ['entry_fp544','exit_fp544']:check(len(raw(e[k]))==544,'fp extent')
  check(e['rng_before']==rnglast,'RNG chain');rnglast=e['rng_after']
  if e['identity']=='_rand':rngs.append((i,e));check(0<=e['eax']<=32767,'rand domain')
  else:check(e['rng_before']==e['rng_after'],'nonrand RNG mutation')
  check(len(raw(e['owner_before']))==(2288 if s['writes']=='opponent_body' else 232 if s['identity']=='sub_401D4D' else 16 if s['identity'] in ['sub_401839','sub_4019C9','sub_401B1D','sub_401B36'] else 0),'owner extent')
  if e['call_rva']==0x1d9e8:getters.append(i)
  if s['ret'].startswith('raw80'):check(e['scalar80']==e['exit_fp544'][64:84],'scalar80')
  ee=raw(e['entry_regs36']);xx=raw(e['exit_regs36'])
  for off in [0,4,8,16]:check(u(ee,off)==u(xx,off),'callback nonvolatile')
  pop=int(s['abi'].split('ret')[-1]) if s['abi'].startswith('thiscall') else 0
  check((u(xx,12)-u(ee,12))&0xffffffff==4+pop,'callback ESP')
 check(rnglast==u(get(0x171b90,'after')),'final RNG')
 consumed=get(0x17622c)[0]!=0 and get(0x17622c,'after')[0]==0
 block=get(0x176234)[0];poolrng=[e for _,e in rngs if e['call_rva'] in [0x1da04,0x1da3b,0x1da6c]]
 check(len(getters)==(100 if consumed and not block else 0),'pool getter count');check(len(poolrng)==(300 if consumed and not block else 0),'pool RNG count')
 if consumed and not block:
  for j in range(100):
   check(pool_after[j*76]==1,'pool active')
   for off in range(76):
    if off!=0 and not 40<=off<52:check(pool_before[j*76+off]==pool_after[j*76+off],'pool sentinel')
   for k in range(3):
    r=poolrng[3*j+k]['eax']%100
    val=(r-50)-60 if k==0 else (r-50)*struct.unpack('<f',bytes.fromhex('0ad7233c'))[0] if k==1 else r-50
    check(pool_after[j*76+40+4*k:j*76+44+4*k]==struct.pack('<f',val),'pool numeric')
   inds=[i for i,e in enumerate(d['events']) if e['call_rva'] in [0x1d9e8,0x1da04,0x1da3b,0x1da6c]]
  check([d['events'][i]['call_rva'] for i in inds]==[0x1d9e8,0x1da04,0x1da3b,0x1da6c]*100,'pool order')
 else:check(pool_before==pool_after,'untouched pool')
 reports.append({'id':n,'sha256':hashlib.sha256(path.read_bytes()).hexdigest(),'events':len(d['events']),'entryFP':fp,'consumed':consumed,'entryBlock':block,'finalBlock':get(0x176234,'after')[0],'initialHealth':f(get(0x176238)),'finalHealth':f(get(0x176238,'after')),'preliminaryUnsupported':unsupported,'integrityErrors':bad,'decisionRngSites':[e['call_rva'] for _,e in rngs if e['call_rva'] not in [0x1da04,0x1da3b,0x1da6c]]})
 if not bad:extractions.append({'source':str(path),'sha256':reports[-1]['sha256'],'status':'SOURCE_TYPED_NOT_FULL_ADMISSION','pointerRoles':{'registered_player_body':d['before']['player_body'],'registered_opponent_body':d['before']['opponent_body'],'registered_world':d['before']['world'],'static_AI_actor':0x5849f8,'static_player_actor':0x584910},'sourceRecord':d})
 errors.extend((n,e) for e in bad)
out={'status':'SOURCE_INTEGRITY_ONLY_NOT_EQUIVALENCE','checks':checks,'errorCount':len(errors),'records':reports}
(root/'validation/GAME-0002-independent-clean-stage4-integrity.json').write_text(json.dumps(out,indent=2))
(root/'validation/GAME-0002-independent-clean-stage4-extraction.json').write_text(json.dumps({'status':'QUARANTINED_PENDING_COMPLETE_ADMISSION_LOCAL_ALIAS_VALIDATION','records':extractions}))
print(json.dumps({'checks':checks,'errors':len(errors),'records':[{k:r[k] for k in ['id','events','consumed','entryBlock','preliminaryUnsupported','integrityErrors']} for r in reports]},indent=2))
# Additional admission checks and declared local identity checks.
locals_by_site={0x1c70b:('L-POS-P',0x98,'arg'),0x1c740:('L-POS-O',0xa8,'arg'),0x1cac6:('L-NEG',0xb8,'arg'),0x1cad6:('L-MUL-F',0xc8,'arg'),0x1cb7b:('L-MUL-R',0xd8,'arg'),0x1d775:('L-HIT',0x4c,'owner'),0x1d900:('L-HIT-POS',0xe8,'arg'),0x1d9e8:('L-POOL-V',0xf8,'arg'),0x1e04c:('L-TAIL-POS',0x108,'arg'),0x1e079:('L-ZERO',0x84,'owner'),0x1e08e:('L-ZERO',0x84,'arg'),0x1e0a3:('L-TAIL-V',0x118,'arg'),0x1e145:('L-ADD',0x128,'arg'),0x1e17e:('L-MUL-S',0x138,'arg'),0x1e2f3:('L-LIMIT',0x148,'arg')}
extra=[]
for r in reports:
 d=json.loads((src/f"damage-original-{r['id']:03}.json").read_text());A=raw(d['before']['actor232']);p=0;g={}
 for sp in spans:g[int(sp['rva'],16)]=raw(d['before']['globals'])[p:p+sp['size']];p+=sp['size']
 def gg(addr):
  for base,b in g.items():
   if base<=addr<base+len(b):return b[addr-base:]
  raise KeyError(hex(addr))
 reasons=list(r['preliminaryUnsupported']);bad=[];roles={}
 def finiteword(val):return math.isfinite(val) and abs(val)<=10000 and (val==0 or abs(val)>=2**-126)
 for off in [0x1761cc,0x1761d4,0x176238,0x176258,0x176260,0x176370]:
  if not finiteword(f(gg(off))):reasons.append('global scalar'+hex(off))
 dt=f(gg(0x176370));fatigue=f(gg(0x176258));combo=f(gg(0x176260))
 if not 0<=dt<=10000:reasons.append('dt bound')
 if not -10000<=fatigue<99:reasons.append('fatigue bound')
 if combo>struct.unpack('<f',bytes.fromhex('9a99193f'))[0]:reasons.append('combo timer>0.6')
 if f(gg(0x1761d4))<0:reasons.append('playerhealth')
 pi=u(gg(0x184784));ai=u(gg(0x184788));difficulty=u(gg(0x1847c4))
 if not 0<=pi<=7 or not 1<=ai<=7:reasons.append('indices')
 else:
  pf=f(gg(0x1762a0+16*pi));duration=f(gg(0x1762a4+16*pi));af=f(gg(0x1762a8+16*ai))
  if not all(finiteword(x) and x>0 for x in [pf,duration,af]):reasons.append('table inputs')
  predicted=struct.unpack('<f',struct.pack('<f',f(gg(0x176238))-((2.0*pf)*2.5)*af))[0]
  if not math.isfinite(predicted) or predicted<=0:reasons.append('health guard')
 if d['mode']==1 and not 1<=difficulty<=5:reasons.append('difficulty')
 for addr,val in [(0x1761c9,1),(0x17622c,None),(0x176234,None),(0x17628c,None)]:
  v=gg(addr)[0]
  if (val is not None and v!=val) or (val is None and v not in [0,1]):reasons.append('global flag'+hex(addr))
 if u(gg(0x1761f8))!=1 or u(gg(0x177f90))!=0 or not gg(0x18479d)[0] or u(gg(0x184790))!=d['mode']:reasons.append('scene/mode')
 bp=d['before']['player_body'];bo=d['before']['opponent_body'];w=d['before']['world'];pb=raw(d['before']['player_body2288']);ob=raw(d['before']['opponent_body2288']);world=raw(d['before']['world_list12'])
 if bp==bo or u(A,152)!=bo or u(raw(d['before']['player176']),152)!=bp:reasons.append('actorbody identities')
 head,tail,count=struct.unpack('<III',world)
 if count!=2 or {head,tail}!={bp,bo}:reasons.append('world head/tail/count')
 for addr,b in [(bp,pb),(bo,ob)]:
  if u(b,2276)!=(tail if addr==head else 0) or u(b,2280)!=(head if addr==tail else 0):reasons.append('world links')
  for off in ([752,756,760] if addr==bp else [752,756,760,408,768,772,776,656,660,664,668,816,820,824,832,836,840,848,852,856]):
   if not finiteword(f(b,off)):reasons.append('body scalar'+str(off))
 for e in d['events']:
  site=e['call_rva'];frame=raw(e['entry_regs36']);ebp=u(frame,8)
  if site in locals_by_site:
   role,off,where=locals_by_site[site];actual=e['owner'] if where=='owner' else e['args'][0]
   if actual!=ebp-off:bad.append(role+' frame mismatch')
   if role in roles and roles[role]!=actual:bad.append(role+' lifetime mismatch')
   roles[role]=actual
   if site!=0x1d9e8 or role not in roles:
    pass
  if site==0x1e1f4:
   if e['args'][2]!=ebp-0x88:bad.append('L-AUDIO frame mismatch')
   roles['L-AUDIO']=e['args'][2]
 # Every supplied distinct declared local must have distinct address.
 if len(set(roles.values()))!=len(roles):bad.append('local role alias')
 # Pending-zero admission must independently establish the far source route.
 if gg(0x17622c)[0]==0:
  distance=None
  cursor=0
  for span in spans:
   if int(span['rva'],16)==0x175eec:distance=f(raw(d['after']['globals']),cursor)
   cursor+=span['size']
  if distance is None or not math.isfinite(distance) or distance<6:reasons.append('near pending-zero original computed distance')
 for e in d['events'][:2]:
  body=pb if e['owner']==bp else ob if e['owner']==bo else None
  if e['call_rva'] not in [0x1c70b,0x1c740] or body is None or raw(e['returned_bytes'])!=body[752:768]:bad.append('initial native getter source mismatch')
 for label,heximage in [('whole-entry',d['entry_fp544']),('whole-exit',d['exit_fp544'])]+[(f'event{j}-{key}',ev[key]) for j,ev in enumerate(d['events']) for key in ['entry_fp544','exit_fp544']]:
  image=raw(heximage);tag=u(image,520)&65535
  if image[0:2]!=image[512:514] or image[2:4]!=image[516:518]:bad.append(label+' duplicate CW/SW inconsistent')
  for physical in range(8):
   if (((tag>>(2*physical))&3)==3)!=((image[4]&(1<<physical))==0):bad.append(label+' full/abridged tag inconsistent')
 extra.append({'id':r['id'],'admission':'REJECT' if reasons else 'SUPPORTED_ENTRY','reasons':reasons,'localErrors':bad,'localPointerRoles':roles,'consumed':r['consumed'],'entryBlock':r['entryBlock']})
(root/'validation/GAME-0002-independent-clean-stage4-admission.json').write_text(json.dumps({'status':'ENTRY_DOMAIN_ONLY_CALLBACK_MUTATION_CHECK_PENDING','records':extra},indent=2))
print(json.dumps({'supported':[r['id'] for r in extra if not r['reasons']],'localErrors':[(r['id'],r['localErrors']) for r in extra if r['localErrors']],'excluded':[(r['id'],r['reasons']) for r in extra if r['reasons']]},indent=2))

