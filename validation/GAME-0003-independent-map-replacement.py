"""Analyze only frozen replacement machine code for exact FP address formulas."""
import hashlib,json,re,struct,subprocess,sys
from pathlib import Path
R=Path(r'C:\Users\ADMIN\CLionProjects\OpenBoxer');version=sys.argv[1] if len(sys.argv)>1 else 'v2'
F=R/('replacement/ms3d/frozen-game0003-candidate-'+version);dll=F/'ms3d_replacement.dll';data=dll.read_bytes();sha=hashlib.sha256(data).hexdigest()
manifest=json.loads((F/'manifest.json').read_text());assert sha==manifest.get('dllSha256',manifest.get('artifacts',{}).get('ms3d_replacement.dll')).lower()
pe=struct.unpack_from('<I',data,0x3c)[0];base=struct.unpack_from('<I',data,pe+52)[0]
dump=subprocess.check_output([r'C:\msys64\mingw32\bin\objdump.exe','-d','-C',str(dll)],text=True)
groups={};name=None
for line in dump.splitlines():
 m=re.match(r'^([0-9a-f]+) <(.+)>:$',line)
 if m:name=m[2];groups[name]=[];continue
 m=re.match(r'^\s*([0-9a-f]+):\s+((?:[0-9a-f]{2} )+)\s*(\S+)\s*(.*)$',line)
 if m and name:groups[name].append({'rva':int(m[1],16)-base,'bytes':bytes.fromhex(m[2]).hex(),'mnemonic':m[3],'operand':m[4]})
update=groups['damage::update(damage::State&, int, bool)'];rows=[];frames=[]
for name,ins in groups.items():
 if 'damage' not in name:continue
 for row in ins:
  if not row['mnemonic'].startswith('f'):continue
  code=bytes.fromhex(row['bytes']);op=next((i for i,b in enumerate(code) if 0xd8<=b<=0xdf),None)
  fop=None if op is None or op+1>=len(code) else ((code[op]&7)<<8)|code[op+1]
  rows.append(dict(row,symbol=name,encodedFOP11=fop))
 if not name.startswith('damage::fp::'):continue
 esp=-4;ebp=None;prologue=[]
 for row in ins:
  m,o=row['mnemonic'],row['operand']
  if m.startswith('f'):break
  prologue.append(row)
  if m=='push':esp-=4
  elif m=='mov' and o=='%esp,%ebp':ebp=esp
  elif m in ['sub','and'] and o.endswith(',%esp'):
   value=int(o.split(',')[0][1:],0)
   if m=='sub':esp-=value
   else:alignment=((~value)&0xffffffff)+1;assert alignment in [8,16];esp=(esp//alignment)*alignment
 operands=[]
 for row in ins:
  if not row['mnemonic'].startswith('f'):continue
  m=re.fullmatch(r'(-?0x[0-9a-f]+)?\(%(esp|ebp)\)',row['operand'])
  if m:
   origin=esp if m[2]=='esp' else ebp;assert origin is not None
   operands.append(dict(row,operandDeltaFromCallerESP=origin+int(m[1] or '0',0)))
 if operands:frames.append({'symbol':name,'prologue':prologue,'helperEBPDeltaFromCallerESP':ebp,'helperESPDeltaFromCallerESP':esp,'x87MemoryOperands':operands})
# First construction of std::array<Vector,18> and its begin call identify the
# persistent local array. Keep every instruction used as evidence.
first=next(i for i,row in enumerate(update) if '18u>::begin()' in row['operand'])
window=update[max(0,first-6):first+1];lea=next(row for row in window if row['mnemonic']=='lea' and re.fullmatch(r'0x[0-9a-f]+\(%esp\),%eax',row['operand']))
locals_offset=int(lea['operand'].split('(')[0],16)
wrappers=[{'symbol':name,'instructions':ins} for name,ins in groups.items() if name.startswith('damage::(anonymous namespace)::') and ('::lt(' in name or '::gt(' in name)]
result={'status':'REPLACEMENT_ONLY_PROVENANCE_NOT_EQUIVALENCE','dll':str(dll),'sha256':sha,'preferredBase':base,'rows':rows,'helperFrames':frames,'updateEntryPrologue':update[:12],'localsOffsetFromUpdateFixedESP':locals_offset,'localsEvidence':window,'directUpdateFPCalls':[r for r in update if r['mnemonic']=='call' and ('damage::fp::' in r['operand'] or '::lt(' in r['operand'] or '::gt(' in r['operand'])],'comparisonWrappers':wrappers,'scriptedScalarFrameChain':[{'symbol':name,'instructions':ins[:14]} for name,ins in groups.items() if name=='damage_script_dispatch' or name=='damage::fixture::Profile::call(damage::Call const&)' or '::scripted(damage::Call const&)' in name or name in ['damage_script2','damage_script3','damage_script14','damage_script15']], 'Sdefinition':'Candidate first original-position getter output pointer minus localsOffsetFromUpdateFixedESP; separately verify callback persistent array identity. Helpers called directly use C=S; gt/lt wrapper uses C=S-16 only after own wrapper frame audit.'}
out=R/('validation/GAME-0003-independent-replacement-'+version+'-fp-map.json');out.write_text(json.dumps(result,indent=2));print(json.dumps({'rows':len(rows),'frames':len(frames),'localOffset':hex(locals_offset),'hash':sha}))
