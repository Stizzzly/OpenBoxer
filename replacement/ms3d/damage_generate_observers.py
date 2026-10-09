"""Generate observer wrappers solely from approved typed metadata."""
import json
from pathlib import Path
p=Path(__file__).resolve().parent
old=json.loads((p.parent.parent/'specs/gameplay/GAME-0002-observer-metadata.json').read_text())
m=json.loads((p.parent.parent/'specs/gameplay/GAME-0003-observer-metadata.json').read_text())
assert m['calls'][:len(old['calls'])]==old['calls']
assert len(m['calls'])==len(old['calls'])+9
assert m['status']=='APPROVED_OBSERVER_ONLY'
rows=[]
assembly=['.text']
identities=['sub_45AC10','sub_4019C9','_sqrt','sub_401ABE','sub_401F64','sub_401839','sub_401B1D','_rand','sub_45B780','sub_40179E','sub_45B210','sub_401D4D','sub_45B2E0','sub_401014','sub_4011E0','sub_401CD0','sub_45B690','sub_401B36','ds:__imp_alGetSourcei','sub_401EB0','sub_401F5F']
for i,c in enumerate(m['calls']):
    name=c['identity']
    target=int(name[4:],16)-0x400000 if name.startswith('sub_') else {'_rand':0x9a750,'_sqrt':0x95b74,'ds:__imp_alGetSourcei':0}[name]
    abi=c['abi']; n=int(abi.split('ret')[-1])//4 if abi.startswith('thiscall') else int(abi.split('caller')[-1])//4
    kinds={'output16_pointer':16,'input16_pointer':16,'outputInt32_pointer':4}
    # Named scalar pointer role is checked explicitly by metadata spellings.
    ptr=[kinds.get(a,4 if 'pointer' in a and 'int32' in a.lower() else 0) for a in c['args']]
    while len(ptr)<4:ptr.append(0)
    rows.append('{%s,%s,0x%x,"%s",%d,%d,{%s},%s,%s},'%(c['callRva'],c['returnRva'],target,name,identities.index(name),n,','.join(map(str,ptr)),str('raw80' in c['ret']).lower(),str(c['writes']=='opponent_body').lower()))
    assembly+=['.globl _damage_observer%d'%i,'_damage_observer%d:'%i,' pushl $%d'%i,' jmp _damage_observer_common']
    assembly+=['.globl _damage_script%d'%i,'_damage_script%d:'%i,' leal 4(%esp),%eax',' pushl %eax',' pushl %ecx',' pushl $%d'%i,' call _damage_script_dispatch',' addl $12,%esp',' ret'+(' $%d'%(n*4) if abi.startswith('thiscall') and n else '')]
header='#pragma once\nstruct DamageSite {uint32_t call,returned,target;const char *name;unsigned id,count;unsigned pointers[4];bool scalar,body;};\ninline constexpr DamageSite damageSites[]={\n'+'\n'.join(rows)+'\n};\n'
header+='inline constexpr unsigned damageLegacySiteCount=%d;\n'%len(old['calls'])
header+='extern "C" {\n'+''.join('void damage_observer%d();\n'%i for i in range(len(rows)))+'}\ninline void *damageWrappers[]={'+','.join('reinterpret_cast<void*>(&damage_observer%d)'%i for i in range(len(rows)))+'};\n'
header+='extern "C" {\n'+''.join('void damage_script%d();\n'%i for i in range(len(rows)))+'}\ninline void *damageScripts[]={'+','.join('reinterpret_cast<void*>(&damage_script%d)'%i for i in range(len(rows)))+'};\n'
(p/'damage_observer_metadata.hpp').write_text(header)
assembly += ['.globl _damage_entry','_damage_entry:',' pushl $-1',' jmp _damage_observer_common',
'_damage_observer_common:',' pushfl',' pushal',' movl %esp,%ebp',' subl $576,%esp',' andl $-16,%esp',' fxsave (%esp)',' fnstenv 512(%esp)',' pushl %esp',' pushl %ebp',' call _damage_observer_before',' addl $8,%esp',' movl %eax,36(%ebp)',' movl $_damage_observer_after,40(%ebp)',' fxrstor (%esp)',' fldenv 512(%esp)',' movl %ebp,%esp',' popal',' popfl',' ret',
'_damage_observer_after:',' pushl $0',' pushfl',' pushal',' movl %esp,%ebp',' subl $576,%esp',' andl $-16,%esp',' fxsave (%esp)',' fnstenv 512(%esp)',' pushl %esp',' pushl %ebp',' call _damage_observer_return',' addl $8,%esp',' movl %eax,36(%ebp)',' fxrstor (%esp)',' fldenv 512(%esp)',' movl %ebp,%esp',' popal',' popfl',' ret']
assembly=[line.replace(' fnstenv 512(%esp)',' fnstenv 512(%esp)') for line in assembly]
# Instrumentation uses its own masked, empty FP environment. The original
# environment, including pending scalar returns, is restored before forwarding.
lines=[]
for line in assembly:
    lines.append(line)
    if line==' fnstenv 512(%esp)':
        lines.extend([' fninit',' movl $0x1f80,544(%esp)',' ldmxcsr 544(%esp)'])
(p/'damage_observers.S').write_text('\n'.join(lines)+'\n')

classifier=m['capture']['actualStart']
assert classifier['status']=='APPROVED_CONFIRMED'
rows=[]
for t in classifier['types']:
    rows.append('{%du,{%s},{%s}}'%(t['type'],','.join(t['orderedNewCalls']),','.join(str(x) for x in t['states'])))
(p/'ai_observer_metadata.hpp').write_text('#pragma once\nstruct AiStartType {unsigned type;uint32_t calls[3],states[2];};\ninline constexpr AiStartType aiStartTypes[]={'+','.join(rows)+'};\ninline constexpr uint32_t aiCommittedTypeRva='+classifier['committedTypeRva']+';\ninline constexpr uint32_t aiAttackRva='+classifier['attackRva']+';\ninline constexpr uint32_t aiPlayerPendingRva='+classifier['playerPendingRva']+';\n')
