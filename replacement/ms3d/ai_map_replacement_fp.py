"""Record x87 provenance from the frozen REPLACEMENT only, never the original."""
import hashlib,json,re,struct,subprocess
from pathlib import Path
root=Path(__file__).resolve().parents[2]
dll=root/'replacement/ms3d/frozen-game0003-candidate-v3/ms3d_replacement.dll'
data=dll.read_bytes()
assert hashlib.sha256(data).hexdigest().upper()=='295C9122B7A24AD7746ADCBE528604082F0B7F37593F31B1C0917552F29B5C13'
pe=struct.unpack_from('<I',data,0x3c)[0]
base=struct.unpack_from('<I',data,pe+24+28)[0]
dump=subprocess.check_output(['C:/msys64/mingw32/bin/objdump.exe','-d','-C',str(dll)],text=True)
symbol='';rows=[]
for line in dump.splitlines():
    label=re.match(r'^([0-9a-f]+) <(.+)>:$',line)
    if label:symbol=label[2];continue
    match=re.match(r'^\s*([0-9a-f]+):\s+((?:[0-9a-f]{2} )+)\s*(\S+)\s*(.*)$',line)
    if not match or 'damage' not in symbol:continue
    raw=bytes.fromhex(match[2]);mnemonic=match[3]
    if not mnemonic.startswith('f'):continue
    opcode=next((i for i,b in enumerate(raw) if 0xd8<=b<=0xdf),None)
    fop=None if opcode is None or opcode+1>=len(raw) else ((raw[opcode]&7)<<8)|raw[opcode+1]
    rows.append({'rva':int(match[1],16)-base,'symbol':symbol,'replacementBytes':raw.hex(),'mnemonic':mnemonic,'operand':match[4],'encodedFOP11':fop})
out={'status':'REPLACEMENT_ONLY_PROVENANCE_MAP_NOT_EQUIVALENCE','dll':str(dll),'sha256':hashlib.sha256(data).hexdigest(),'preferredBase':base,'runtimeInstructionRule':'runtime DLL base + rva; prove loaded module independently','FOPPolicy':'Decode operation and operand width. ModRM addressing bits identify relocated data and must not be mistaken for a different x87 operation. No blanket exclusion.','rows':rows}
target=root/'validation/GAME-0003-replacement-v3-x87-map.json'
target.write_text(json.dumps(out,indent=2))
print(f'Replacement-only x87 mapped instructions: {len(rows)}')
