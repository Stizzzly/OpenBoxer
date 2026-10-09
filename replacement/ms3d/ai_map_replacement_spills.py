"""Derive spill address formulas from the frozen replacement call frames only."""
import hashlib,json,re,struct,subprocess
from pathlib import Path
root=Path(__file__).resolve().parents[2]
dll=root/'replacement/ms3d/frozen-game0003-candidate-v3/ms3d_replacement.dll'
raw=dll.read_bytes();sha=hashlib.sha256(raw).hexdigest()
assert sha.upper()=='295C9122B7A24AD7746ADCBE528604082F0B7F37593F31B1C0917552F29B5C13'
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

update=groups['damage::update(damage::State&, int, bool)']
calls=[i for i in update if i['mnemonic']=='call' and ('damage::fp::' in i['operand'] or '::lt(' in i['operand'] or '::gt(' in i['operand'])]
wrappers=[{'symbol':name,'instructions':ins} for name,ins in groups.items() if name.startswith('damage::(anonymous namespace)::') and ('::lt(' in name or '::gt(' in name)]
relevant=[{'symbol':name,'instructions':ins} for name,ins in groups.items() if name.startswith('damage::') and ('::fp::' in name or name.startswith('damage::update(') or '(anonymous namespace)::' in name)]
out={'status':'REPLACEMENT_ONLY_SPILL_AND_SYMBOL_PROVENANCE_NOT_EQUIVALENCE','dll':str(dll),'sha256':sha,'preferredBase':base,'helperFrames':helpers,'directUpdateFPCalls':calls,'comparisonWrappers':wrappers,'replacementSymbols':relevant,'updateEntryPrologue':update[:15],'rule':'Only own replacement image is read. CallerESP-relative operands are proven by emitted helper prologue. Relate each runtime callback checkpoint to actually reached specific source call and operand; no blanket FDP/address or reset waiver.'}
target=root/'validation/GAME-0003-replacement-v3-spill-symbol-map.json';target.write_text(json.dumps(out,indent=2));print('Own helper frames',len(helpers),'update FPcalls',len(calls),'symbols',len(relevant))
