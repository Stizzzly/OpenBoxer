import json
from pathlib import Path
p=Path(__file__).resolve().parent
vectors=json.loads((p.parent.parent/'validation/GAME-0002-independent-vector-intents.json').read_text())['vectors']
mapping={
'AIindex_int32':(0,0x184788),'attackTimer_raw32':(0,0x1761cc),'block_byte':(1,0x176234),'counter_raw32':(0,0x17620c),'decision204_byte':(3,204),'difficulty_int32':(0,0x1847c4),'marker_byte':(1,0x17628c),'pending_byte':(1,0x17622c),'previousType_u32':(0,0x176224),'readiness_int32':(2,208),'sound_byte':(1,0x177f9e),'timer180_raw32':(2,180),'timer184_raw32':(2,184),'timer188_raw32':(2,188),'actorState_u32':(2,164),'blockTimer_raw32':(2,228),'cooldown192_raw32':(2,192),'mode_int32':(5,0),'opponentX_raw32':(4,752),'absoluteCallback_promoteRaw32':(6,0),'finalSqrtCallback_promoteRaw32':(7,0)}
out=['#pragma once','struct DamagePatch {unsigned kind,offset;uint32_t value;};']
for i,v in enumerate(vectors):
    out+=['inline constexpr DamagePatch damagePatch%d[]={'%i]
    for k,value in v['patch'].items():
        kind,offset=mapping[k];value=int(value,16) if isinstance(value,str) else value
        out+=['{%d,0x%x,0x%x},'%(kind,offset,value)]
    out+=['};']
out+=['struct DamageCase {const char *name;const DamagePatch *patches;unsigned count;};','inline constexpr DamageCase damageCases[]={']
for i,v in enumerate(vectors):out+=['{"%s",damagePatch%d,sizeof(damagePatch%d)/sizeof(DamagePatch)},'%(v['id'],i,i)]
out+=['};']
(p/'damage_fixture_metadata.hpp').write_text('\n'.join(out)+'\n')
