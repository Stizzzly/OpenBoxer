import json,re,struct
from pathlib import Path
v=Path(r'C:\Users\ADMIN\CLionProjects\OpenBoxer\validation');d=Path(r'C:\Users\ADMIN\Boxer-lab\ms3d\damage-fixture-observations-stage4')
inventory=json.loads((v/'GAME-0002-independent-stage4-fp-inventory.json').read_text())
names=sorted({x['checkpoint'] for x in inventory['rawDifferenceInventory'] if x['field']=='envFCS'})
def u16(b,o):return struct.unpack_from('<H',b,o)[0]
def u32(b,o):return struct.unpack_from('<I',b,o)[0]
def state(s):
 b=bytes.fromhex(s);return {'CW':hex(u16(b,0)),'SW':hex(u16(b,2)),'TOP':(u16(b,2)>>11)&7,'fullTags':hex(u16(b,520)),'abridgedTags':hex(b[4]),'FIP':hex(u32(b,524)),'FOP':hex(u16(b,530)),'FDP':hex(u32(b,532)),'FCS':hex(u16(b,528)),'FDS':hex(u16(b,536)),'legacyRaw28':b[512:540].hex()}
records=[]
for name in names:
 match=re.fullmatch(r'pair(\d+):event(\d+):site(0x[0-9a-f]+):(entry_fp544|exit_fp544)',name);n,j,site,key=match.groups();n=int(n);j=int(j)
 a,b=[json.loads((d/f'{prefix}-{number:03}.json').read_text()) for prefix,number in [('damage-fixture-original',n),('damage-candidate',n+1)]]
 records.append({'checkpoint':name,'original':state(a['events'][j][key]),'candidate':state(b['events'][j][key]),'originalSameEventEntry':state(a['events'][j]['entry_fp544']),'originalPreviousEventExit':state(a['events'][j-1]['exit_fp544']) if j else None,'candidateSameEventEntry':state(b['events'][j]['entry_fp544'])})
result={'status':'TYPED_OBSERVATIONS_ONLY_UNRESOLVED_LEGACY_PROVENANCE','count':len(records),'records':records,'policy':'No exclusions or semantic explanation inferred. Source raw captures retained.'}
(v/'GAME-0002-independent-stage4-legacy-reset-checkpoints.json').write_text(json.dumps(result,indent=2));print(json.dumps({'count':len(records),'checkpoints':names}))
