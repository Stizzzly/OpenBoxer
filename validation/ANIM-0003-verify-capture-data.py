"""Integrity check for root-authorized typed future-slot snapshot data only."""
import argparse
import hashlib
import json
import struct
from pathlib import Path
if __name__=='__main__':
    p=argparse.ArgumentParser();p.add_argument('jsonl');p.add_argument('directory');p.add_argument('output');a=p.parse_args()
    rows=[json.loads(x) for x in Path(a.jsonl).read_text(encoding='utf-8-sig').splitlines() if x.strip()];checks=[]
    for row in rows:
        file=Path(a.directory)/f'frames-native-{row["route"]}-{row["call"]:04d}.bin'
        raw=file.read_bytes();h=struct.unpack_from('<12I',raw)
        expected=(0x334d5246,1,row['clock_bits'],row['after_clock_bits'],row['eax'],row['opaque_entry_ecx'],row['initial_fp']['cw'],row['initial_fp']['sw'],row['before_model_words'][11],row['animation_replacements_before'],row['animation_replacements_after'],row['trusted_continuations_delta'])
        before=raw[48:160];record=raw[160:432];prepared=raw[432:544];after=raw[544:656]
        ok=(len(raw)==656 and h==expected and before==struct.pack('<28I',*row['before_model_words']) and record.hex()==row['record_hex'] and prepared==struct.pack('<28I',*row['prepared_model_words']) and after==struct.pack('<28I',*row['after_model_words']))
        checks.append(dict(file=str(file),result='PASS' if ok else 'FAIL_STATE',sha256=hashlib.sha256(raw).hexdigest().upper()))
    report=dict(result='PASS' if checks and all(c['result']=='PASS' for c in checks) else 'FAIL',checks=checks)
    Path(a.output).write_text(json.dumps(report,indent=2)+'\n',encoding='utf-8');print(json.dumps(report))
