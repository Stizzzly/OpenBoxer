"""Compare only coordinator-approved typed whole-body observations."""
import json, pathlib, hashlib, struct, sys
stage=sys.argv[1] if len(sys.argv)>1 else 'stage2'
count=126 if stage=='stage2' else 216
base=pathlib.Path(__file__).parent/f'GAME-0001-{stage}-typed'
failures=[];checks=0
def ck(pair,label,a,b):
    global checks
    checks+=1
    if a!=b: failures.append({'pair':pair,'label':label,'original':a,'candidate':b})
def fp(raw):
    b=bytes.fromhex(raw)
    # FIP/FDP refer to differing implementation/callback instruction addresses;
    # dead register payload/reserved bytes are not live FP state.
    sw=int.from_bytes(b[2:4],'little');tag=b[4];top=(sw>>11)&7
    live=[b[32+16*i:42+16*i].hex() for i in range(8) if tag&(1<<((top+i)&7))]
    return {'cw':b[:2].hex(),'sw':sw,'tag':tag,'mxcsr':b[24:28].hex(),'live80':live}
def snap(pair,label,a,b):
    for k in a:
        if k=='fp512':ck(pair,label+'.fp',fp(a[k]),fp(b[k]))
        else:ck(pair,label+'.'+k,a[k],b[k])
manifest=[]
for n in range(1,count+1,2):
    pair=(n+1)//2
    paths=[base/f'strike-fixture-{i:03}.json' for i in (n,n+1)]
    a,b=[json.loads(p.read_text()) for p in paths]
    manifest += [{'file':p.name,'sha256':hashlib.sha256(p.read_bytes()).hexdigest()} for p in paths]
    for p in paths:
        sidecar=p.with_suffix('.skt')
        if sidecar.exists():manifest.append({'file':sidecar.name,'sha256':hashlib.sha256(sidecar.read_bytes()).hexdigest()})
    ck(pair,'roles',(a['candidate'],b['candidate']),(False,True))
    ck(pair,'format',a['format'],b['format']);ck(pair,'reason',a['reason'],b['reason'])
    ck(pair,'final fullEAX',a['eax'],b['eax'])
    snap(pair,'entry',a['before'],b['before']);snap(pair,'exit',a['after'],b['after'])
    ck(pair,'eventcount',len(a['events']),len(b['events']))
    player_address=next(e['owner'] for e in a['events'] if e['id']==4)
    body_addresses=[node['address'] for node in a['before']['active_list']]
    for i,(x,y) in enumerate(zip(a['events'],b['events'])):
        label=f'event{i}'
        ck(pair,label+'.id',x['id'],y['id'])
        ck(pair,label+'.pointer_arg_bytes',x['pointer_arg_bytes'],y['pointer_arg_bytes'])
        # Typed local pointers are normalized per callback lifetime. Preserve
        # equality/offset relations within that event; primitives stay exact.
        pointers=[(x['owner'],y['owner'])] if x['owner'] else []
        for j,size in enumerate(x['pointer_arg_bytes']):
            if size:pointers.append((x['args'][j],y['args'][j]))
        for j,(u,v) in enumerate(pointers):
            if player_address<=u<player_address+176 or any(t<=u<t+2288 for t in body_addresses) or 0x400000<=u<0x600000:
                ck(pair,label+f'.stablepointer{j}',u,v)
            for k,(uu,vv) in enumerate(pointers[:j]):
                ck(pair,label+f'.pointeralias{j}-{k}',u==uu,v==vv)
        for j,(u,v) in enumerate(zip(x['args'],y['args'])):
            if not x['pointer_arg_bytes'][j]:ck(pair,label+f'.primitive{j}',u,v)
        pointer_return=x['id'] in (0,1,2,7,10,11,18)
        if pointer_return:
            roles=[(j,x['eax']-u) for j,(u,v) in enumerate(pointers) if 0<=x['eax']-u<16]
            candidate_roles=[(j,y['eax']-v) for j,(u,v) in enumerate(pointers) if 0<=y['eax']-v<16]
            ck(pair,label+'.returnrole',roles,candidate_roles)
            ck(pair,label+'.knownreturn',bool(roles),True)
        else:ck(pair,label+'.fullEAX',x['eax'],y['eax'])
        for k in ('scalar80','returned_bytes','returned_pointer_size','arg_before','arg_after'):
            ck(pair,label+'.'+k,x[k],y[k])
        for k in ('owner_before','owner_after','owner_size'):
            if k in x:ck(pair,label+'.'+k,x[k],y[k])
        ck(pair,label+'.returnedFP',fp(x['returned_fp512']),fp(y['returned_fp512']))
        snap(pair,label+'.entry',x['before'],y['before']);snap(pair,label+'.exit',x['after'],y['after'])
provenance={'stage2':(17876,'84167428AE591D35A83380CBFD30CAD1568460B8183AF95A0FB6818009ECED52'),'stage3':(20352,'CA4204E18E23BB6AEDA8CA3D838D2D07C51C2C2B2596744D86FBC47EF2BD7DC8'),'stage4':(1800,'BA2338A9B584B1AC19E8E5AE4C002A6A649A5680731F44A0EFD4F102DDBDB53D'),'stage5':(8480,'7D85CBB8B189D5FE9B5869DB49F72A67E90FFED2E677BA58E3A5ADBE2006A7D8')}
provenance['stage6']=(14328,'0604B11ADC5BB31E0C012E4B78C1061DF9803236F37719DA207276B4200B3015')
pid,dll=provenance[stage]
report={'pairs':count//2,'checks':checks,'failures':failures,'primary':'FAIL' if failures else 'PASS','scope':'scripted typed whole-original/candidate observations only; natural replay and guard/ABI in separate reports','provenance':{'pid':pid,'dll':dll,'launcher':'55E3630E4DCE61AE3617C934163E617AA8F78164200FAAC05E6DB0A8F879DC11'},'files':manifest}
(base.parent/f'GAME-0001-independent-{stage}-comparison.json').write_text(json.dumps(report,indent=2))
print(json.dumps({'pairs':count//2,'checks':checks,'failures':len(failures),'first':failures[:8]}))
