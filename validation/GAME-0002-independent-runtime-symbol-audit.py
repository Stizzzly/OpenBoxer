"""Read approved typed own-module observations; never accesses processes."""
import json,hashlib,subprocess,re,struct,sys,pefile
from pathlib import Path
dll=Path(sys.argv[1]);source=Path(sys.argv[2]);out=Path(sys.argv[3]);mode=sys.argv[4]
pref=pefile.PE(str(dll)).OPTIONAL_HEADER.ImageBase;binaryHash=hashlib.sha256(dll.read_bytes()).hexdigest()
symbols={}
for line in subprocess.check_output([r'C:\msys64\mingw32\bin\nm.exe','-C','--defined-only',str(dll)],text=True).splitlines():
 m=re.match(r'([0-9a-fA-F]+)\s+\w\s+(.+)',line)
 if m:symbols.setdefault(m[2],[]).append(int(m[1],16))
checks=0;errors=[]
def ck(a,n):
 global checks
 checks+=1
 if not a:errors.append(n)
p=json.loads(source.read_text());base=int(p['moduleBase'],16)
ck(p['dllHash'].lower()==binaryHash,'frozen loaded hash')
values={}
for row in p['counters']:
 va=int(row['preferred'],16);ck(va in symbols.get(row['symbol'],[]),row['symbol']+' own symbol')
 ck(int(row['address'],16)==base+va-pref,row['symbol']+' mapped address')
 ck(struct.unpack('<I',bytes.fromhex(row['raw4']))[0]==row['u32'],row['symbol']+' raw bytes')
 values[row['symbol']]=row
for field in ['captureFlags14','originalFlags14','diagnosticFlags14']:
 expected='00'*13+'01' if field=='captureFlags14' and mode=='capture' else '00'*14
 ck(p[field]==expected,field)
target=int(p['thunks']['damage']['target'],16)
if mode=='original':
 ck(target==0x41c4e0,'original damage thunk')
 for name,row in values.items():
  if name.startswith('damage::'):ck(row['raw4'][:2]=='00',name+' original-only damage off')
elif mode=='replacement':
 ck(target==base+symbols['damage_candidate_entry'][0]-pref,'pure candidate entry thunk')
 ck(values['damage::(anonymous namespace)::replacement']['raw4'][:2]=='01','candidate enabled bool')
 # replacementCounter is an optional unsigned* test sink, not a persistent count.
 # Native execution evidence is separately supplied by captures and human/runtime observations.
 ck(values['damage::(anonymous namespace)::observersInstalled']['raw4'][:2]=='00','capture observers absent')
elif mode=='capture':
 ck(target==base+symbols['damage_entry'][0]-pref,'capture entry thunk')
 ck(values['damage::(anonymous namespace)::replacementEnabled']['raw4'][:2]=='01','candidate capture enabled')
 ck(values['damage::(anonymous namespace)::observersInstalled']['raw4'][:2]=='01','capture observers installed')
else:raise ValueError(mode)
result={'status':'PASS_TYPED_OWN_RUNTIME_SYMBOL_ROUTE_FLAGS' if not errors else 'FAIL','checks':checks,'errors':errors,'source':str(source),'sourceHash':hashlib.sha256(source.read_bytes()).hexdigest(),'dllHash':binaryHash,'preferredBase':hex(pref),'observedBase':hex(base),'mode':mode}
out.write_text(json.dumps(result,indent=2));print(result)
