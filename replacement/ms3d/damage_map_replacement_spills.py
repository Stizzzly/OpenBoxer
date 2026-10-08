"""Derive spill address formulas from the frozen replacement call frames only."""
import hashlib,json,re,struct,subprocess
from pathlib import Path
root=Path(__file__).resolve().parents[2]
dll=root/'replacement/ms3d/frozen-game0002-fixture-v4/ms3d_replacement.dll'
raw=dll.read_bytes();sha=hashlib.sha256(raw).hexdigest()
assert sha.upper()=='E34F66520A6FFB963FD7F5968DDA96DA5F0C39B6CB278264977E6FE729CA7A2E'
pe=struct.unpack_from('<I',raw,0x3c)[0];base=struct.unpack_from('<I',raw,pe+52)[0]
dump=subprocess.check_output(['C:/msys64/mingw32/bin/objdump.exe','-d','-C',str(dll)],text=True)
groups={};name=None
for line in dump.splitlines():
    label=re.match(r'^([0-9a-f]+) <(.+)>:$',line)
    if label:name=label[2];groups[name]=[];continue
    match=re.match(r'^\s*([0-9a-f]+):\s+((?:[0-9a-f]{2} )+)\s*(\S+)\s*(.*)$',line)
    if match and name:groups[name].append({'rva':int(match[1],16)-base,'bytes':bytes.fromhex(match[2]).hex(),'mnemonic':match[3],'operand':match[4]})
helpers=[]
for name,instructions in groups.items():
    if not name.startswith('damage::fp::'):continue
    # callerESP=C, CALL puts PC at C-4. Track the straight-line prologue.
    esp=-4;ebp=None;prologue=[]
    for ins in instructions:
        m,o=ins['mnemonic'],ins['operand']
        if m.startswith('f'):break
        prologue.append(ins)
        if m=='push':esp-=4
        elif m=='mov' and o=='%esp,%ebp':ebp=esp
        elif m in ('sub','and') and o.endswith(',%esp'):
            value=int(o.split(',')[0][1:],0)
            if m=='sub':esp-=value
            else:
                alignment=((~value)&0xffffffff)+1
                assert alignment in (8,16)
                esp=(esp//alignment)*alignment
    operands=[]
    for ins in instructions:
        if not ins['mnemonic'].startswith('f'):continue
        match=re.fullmatch(r'(-?0x[0-9a-f]+)?\(%(esp|ebp)\)',ins['operand'])
        if match:
            displacement=int(match[1] or '0',0);origin=esp if match[2]=='esp' else ebp
            assert origin is not None
            operands.append(dict(ins,operandDeltaFromCallerESP=origin+displacement))
    if operands:helpers.append({'symbol':name,'callerAlignmentRequirement':'C is multiple16 (direct update), or updateS-16 (lt/gt wrapper)','prologue':prologue,'helperEBPDeltaFromCallerESP':ebp,'helperESPDeltaFromCallerESP':esp,'x87MemoryOperands':operands})
update=groups['damage::update(damage::State&, int)']
calls=[i for i in update if i['mnemonic']=='call' and ('damage::fp::' in i['operand'] or '::lt(' in i['operand'] or '::gt(' in i['operand'])]
wrappers=[]
for name,ins in groups.items():
    if name.startswith('damage::(anonymous namespace)::') and ('::lt(' in name or '::gt(' in name):wrappers.append({'symbol':name,'helperCallerESPDeltaFromUpdateS':-16,'instructions':ins})
out={'status':'REPLACEMENT_ONLY_SPILL_PROVENANCE_NOT_COMPARISON_PASS','dll':str(dll),'sha256':sha,'preferredBase':base,
'updateSDefinition':'S is update fixed aligned ESP. locals[15] starts S+0x4614; L-POS-P first getter1C70B arg0=locals[0], so S=arg0-0x4614. Pool getter1D9E8 arg0=locals[7]=S+0x4684.',
'updateEntryPrologue':update[:9],'localsBaseInstruction':next(i for i in update if i['operand']=='0x4614(%esp),%eax'),
'poolProof':{'helperFstpsRvas':[0x94928,0x94936],'poolCallRvasXYZ':[0x5d3ac,0x5d4a1,0x5d596],'helperEBP':'S-8','spillFDP':'S-12','fromInitialPlayerGetterArg0':'arg0-0x4620','fromPoolGetterArg0':'arg0-0x4690','sourceRole':'poolX/Y/Z float32 result; returned as uint32 then written to corresponding pool field',
'instructionShape':'CALL to helper, PUSH EBP, MOV ESP,EBP; FSTPS -4(EBP). See exact helper prologue/instruction and direct call anchors.'},
'helperFrames':helpers,'directUpdateFPCalls':calls,'comparisonWrappers':wrappers,
'validationRule':'Independently verify same frozen DLL and runtime base; at each checkpoint choose the actually reached source operation/call chain. Check exact FDP formula and encoded FOP decoded operation; do not use a generic stack range.'}
target=root/'validation/GAME-0002-replacement-v4-spill-map.json';target.write_text(json.dumps(out,indent=2))
print(f'Replacement-only helper frames={len(helpers)} direct call anchors={len(calls)}')
