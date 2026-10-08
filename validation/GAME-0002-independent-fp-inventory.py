"""Read typed observation JSON only; retain all raw differences and tag evidence."""
import json, struct, hashlib, sys
from pathlib import Path
ROOT=Path(r'C:\Users\ADMIN\CLionProjects\OpenBoxer')
DATA=Path(sys.argv[1]) if len(sys.argv)>1 else Path(r'C:\Users\ADMIN\Boxer-lab\ms3d\damage-fixture-observations-stage2')
OUT=Path(sys.argv[2]) if len(sys.argv)>2 else ROOT/'validation/GAME-0002-independent-stage2-fp-inventory.json'
def u16(b,o): return struct.unpack_from('<H',b,o)[0]
def u32(b,o): return struct.unpack_from('<I',b,o)[0]
checks=0; errors=[]; differences=[]; sources=[]
def check(ok,context,what):
    global checks
    checks+=1
    if not ok: errors.append({'checkpoint':context,'field':what})
def inspect(sa,sb,context):
    a,b=bytes.fromhex(sa),bytes.fromhex(sb)
    check(len(a)==544 and len(b)==544,context,'extent')
    if len(a)!=544 or len(b)!=544:return
    tops=[(u16(x,2)>>11)&7 for x in [a,b]]
    tags=[u16(x,520) for x in [a,b]]
    for side,x in enumerate([a,b]):
        check(x[0:2]==x[512:514],context,f'duplicateCW{side}')
        check(x[2:4]==x[516:518],context,f'duplicateSW{side}')
        for physical in range(8):
            empty=((tags[side]>>(2*physical))&3)==3
            check(empty==(not bool(x[4]&(1<<physical))),context,f'tagConsistency{side}:{physical}')
    for o,n,name in [(0,2,'CW'),(2,2,'SW'),(4,1,'abridgedTags'),(24,4,'MXCSR'),(28,4,'MXCSR_MASK'),(512,2,'envCW'),(516,2,'envSW'),(520,2,'fullTags')]:
        check(a[o:o+n]==b[o:o+n],context,name)
    covered=set()
    def field(o,n,name,classification,reason,extra=None):
        covered.update(range(o,o+n))
        if a[o:o+n]!=b[o:o+n]:
            item={'checkpoint':context,'field':name,'offset':o,'bytes':n,'original':a[o:o+n].hex(),'candidate':b[o:o+n].hex(),'classification':classification,'reason':reason}
            if extra:item.update(extra)
            differences.append(item)
    for logical in range(8):
        physical=[(top+logical)%8 for top in tops]
        t=[(tags[s]>>(2*physical[s]))&3 for s in range(2)]
        abr=[(x[4]>>physical[s])&1 for s,x in enumerate([a,b])]
        dead=t==[3,3] and abr==[0,0]
        field(32+16*logical,10,f'ST{logical}','EXCLUDED_EMPTY' if dead else 'EXACT_LIVE','Both corresponding architectural tags EMPTY' if dead else 'Live in at least one capture',{'logicalSlot':logical,'physicalSlots':physical,'fullTags':tags,'slotTags':t,'abridgedBits':abr,'TOP':tops})
        if not dead:check(a[32+16*logical:42+16*logical]==b[32+16*logical:42+16*logical],context,f'liveST{logical}')
        field(42+16*logical,6,f'ST{logical}padding','EXCLUDED_RESERVED','FXSAVE 16-byte x87 save slot has 10-byte payload plus 6 reserved bytes')
    for o,n,name in [(6,2,'FOP'),(8,4,'FIP'),(12,2,'FCS'),(16,4,'FDP'),(20,2,'FDS'),(524,4,'envFIP'),(528,2,'envFCS'),(530,2,'envFOP'),(532,4,'envFDP'),(536,2,'envFDS')]:
        field(o,n,name,'BLOCKED_PROVENANCE','Requires corresponding instruction/data mapping or checkpoint-specific stale provenance')
    for o,n,name in [(5,1,'FXSAVE_reserved5'),(14,2,'FXSAVE_reserved14'),(22,2,'FXSAVE_reserved22'),(288,176,'FXSAVE_reserved288'),(464,48,'FXSAVE_software_available'),(514,2,'FNSTENV_reserved514'),(518,2,'FNSTENV_reserved518'),(522,2,'FNSTENV_reserved522'),(538,2,'FNSTENV_reserved538'),(540,4,'capture_array_padding')]:
        field(o,n,name,'EXCLUDED_RESERVED','32-bit FXSAVE/FNSTENV documented reserved or software/padding region; FNSTENV writes 28 bytes into final 32-byte array area')
    field(160,128,'XMM0..7','FAIL_ARCHITECTURAL','Approved exact XMM0..7 architectural state')
    check(a[160:288]==b[160:288],context,'XMM0..7')
    for o,n,name in [(0,2,'CW'),(2,2,'SW'),(4,1,'abridgedTags'),(24,4,'MXCSR'),(28,4,'MXCSR_MASK'),(512,2,'envCW'),(516,2,'envSW'),(520,2,'fullTags')]:field(o,n,name,'FAIL_ARCHITECTURAL','Exact architectural field')
    check(len(covered)==544,context,'all544BytesClassified')
for number in range(1,123,2):
    p=DATA/f'damage-fixture-original-{number:03}.json';q=DATA/f'damage-candidate-{number+1:03}.json'
    if number==121:
        p=DATA/'damage-guard-reference-121.json';q=DATA/'damage-guard-candidate-122.json'
    if not p.exists() or not q.exists():continue
    a,b=[json.loads(x.read_text(encoding='utf-8-sig')) for x in [p,q]]
    sources.append({'original':str(p),'candidate':str(q),'originalSha256':hashlib.sha256(p.read_bytes()).hexdigest(),'candidateSha256':hashlib.sha256(q.read_bytes()).hexdigest()})
    for key in ['entry_fp544','exit_fp544']:inspect(a[key],b[key],f'pair{number}:{key}')
    check(len(a['events'])==len(b['events']),f'pair{number}','eventCount')
    for index,(x,y) in enumerate(zip(a['events'],b['events'])):
        for key in ['entry_fp544','exit_fp544']:inspect(x[key],y[key],f'pair{number}:event{index}:site{hex(x["call_rva"])}:{key}')
counts={}
for d in differences:counts[d['classification']]=counts.get(d['classification'],0)+1
result={'status':'FAIL' if errors else ('BLOCKED_PROVENANCE' if any(k.startswith('BLOCKED') for k in counts) else 'PASS_FP_SCOPE'),'checks':checks,'pairs':len(sources),'errors':errors,'differenceCounts':counts,'rawDifferenceInventory':differences,'sources':sources,'policySha256':hashlib.sha256((ROOT/'specs/gameplay/GAME-0002-typed-fp-locals.md').read_bytes()).hexdigest()}
OUT.write_text(json.dumps(result,indent=2),encoding='utf-8')
print(json.dumps({k:result[k] for k in ['status','checks','pairs','differenceCounts']}))
