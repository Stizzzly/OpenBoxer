"""Run preserved replacement-only map generators against the explicit v6 hash."""
import json,re,subprocess
from pathlib import Path
root=Path(__file__).resolve().parent.parent
scripts=root/'replacement/ms3d'
old='E34F66520A6FFB963FD7F5968DDA96DA5F0C39B6CB278264977E6FE729CA7A2E'
new='B10C2E1D5CF4E422D8A83D2F7470D0E4463200CE4D8BDD09CD7BE4AA1B32052F'
p=scripts/'damage_map_replacement_fp.py'
source=p.read_text().replace(old,new).replace('fixture-v4','fixture-v6').replace('replacement-v4','replacement-v6')
exec(compile(source,str(p),'exec'),{'__file__':str(p),'__name__':'__main__'})
mapped=json.loads((root/'validation/GAME-0002-replacement-v6-x87-map.json').read_text())
fstps=[r['rva'] for r in mapped['rows'] if 'damage::fp::pool(' in r['symbol'] and r['mnemonic']=='fstps']
dll=root/'replacement/ms3d/frozen-game0002-fixture-v6/ms3d_replacement.dll'
dump=subprocess.check_output(['C:/msys64/mingw32/bin/objdump.exe','-d','-C','--disassemble=damage::update(damage::State&, int)',str(dll)],text=True)
calls=[]
for line in dump.splitlines():
    if '<damage::fp::pool(' in line:
        address=int(line.split(':')[0].strip(),16)
        calls.append(address-mapped['preferredBase'])
assert len(fstps)==2 and len(calls)==3
p=scripts/'damage_map_replacement_spills.py'
source=p.read_text().replace(old,new).replace('fixture-v4','fixture-v6').replace('replacement-v4','replacement-v6')
source=source.replace('0x4614','0x4964').replace('0x4684','0x49d4').replace('0x4620','0x4970').replace('0x4690','0x49e0')
source=source.replace('[0x94928,0x94936]',repr(fstps)).replace('[0x5d3ac,0x5d4a1,0x5d596]',repr(calls))
exec(compile(source,str(p),'exec'),{'__file__':str(p),'__name__':'__main__'})
