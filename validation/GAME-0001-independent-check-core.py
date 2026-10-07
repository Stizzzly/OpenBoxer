"""Independent approved-contract verifier for scripted replacement fixtures.

Opaque callbacks are controlled test inputs. Original equivalence remains a
separate required comparison. No callback behavior is inferred from the core.
"""
import importlib.util
import json
from pathlib import Path
import struct

base=Path(__file__).parent
s=importlib.util.spec_from_file_location('numeric',base/'GAME-0001-independent-numeric.py')
n=importlib.util.module_from_spec(s);s.loader.exec_module(n)
rows=[json.loads(x) for x in (base/'GAME-0001-independent-core.jsonl').read_text(encoding='utf-8-sig').splitlines()]
failures=[];checks=0
def check(row,label,actual,expected):
    global checks
    checks+=1
    if actual!=expected:failures.append(dict(id=row['id'],label=label,actual=actual,expected=expected))
for r in rows:
    key=['Z','X','C'][r['key']];v=r['variant'];a=n.Arithmetic()
    argument=a.distance_argument([r['position'],0,0,0],[0,0,0,0])
    check(r,'position-derived sqrt argument',r['sqrtArgument'],f'{argument:016x}')
    # Mock supplies double sqrt(argument). Verification derives expected result
    # independently from argument, then models the parent float32 store.
    sqrt_value=struct.unpack('<d',struct.pack('<Q',argument))[0]**0.5
    distance=struct.unpack('<I',struct.pack('<f',sqrt_value))[0]
    check(r,'distance float32',r['distance'],distance)
    expected=n.strike(distance,key,r['hand']);hit=expected['hit']
    for name in ['type','latch','attack']:check(r,name,r[name],expected[name])
    check(r,'pending',r['pending'],int(hit));check(r,'marker',r['marker'],int(hit))
    timer=a.timer(0x3dcccccd,0);fatigue=a.fatigue(0x3dcccccd,0x41200000 if v&1 else 0)
    acc=a.accumulator('00000000000000c00040',0x3e800000 if v==3 else 0x3dcccccd,0x40e00000 if v==3 else 0x40000000)
    for name,value in [('timer',timer),('fatigue',fatigue),('accumulator',acc)]:check(r,name,r[name],value)
    p=bytearray([0xa5]*176)
    def put(offset,value):struct.pack_into('<I',p,offset,value)
    put(152,0x11223344);put(164,expected['animation'])
    for i in range(3):p[168+i]=0
    p[168+r['key']]=expected['hand'];p[171]=0
    for i,value in enumerate([r['position'],0,0,0xdeadbeef]):put(12+4*i,value)
    for i in range(3):put(76+4*i,0)
    velocity=[0x40800000 if v==3 else 0x40400000 if v==2 else 0x40000000,0 if v==3 else 0x3f800000,0,0xabcdef01]
    for i,value in enumerate(velocity):put(92+4*i,value)
    helper=10 if v==0 else 11
    for i in range(4):put(124+4*i,0xabc00000+i+helper*16)
    put(140,acc);put(172,0x3fbfffff if v==0 else 0x3fc00000 if v==1 else 0x3fc00001)
    if v==3:put(32,0x40a00000)
    check(r,'full176 state including unchanged bytes',r['player'],p.hex())
    # Independent ordered opaque-callback contract. IDs from approved typed API,
    # not a trace-derived expected list.
    events=[2,2]+[1]*8+[17,0]
    if hit:events += [0,3]
    events += [0,4,0,1,5,2,0,6,7,8,9,1,helper,12,13,14]
    if v==2 or v==3:events += [16]
    events += [1]*4+[17]
    if v==3:events += [1,18,12]
    check(r,'ordered callbacks', [e[0] for e in r['events']],events)
    for i,e in enumerate(r['events']):
        check(r,f'event{i} CW',e[2],0x027f)
        check(r,f'event{i} TOP',(e[1]>>11)&7,0)
        check(r,f'event{i} empty tag',e[3],0)
        check(r,f'event{i} inherited precision sticky',e[1]&0x20,0x20)
    cursor=next(e for e in r['events'] if e[0]==4)
    check(r,'cursor parent combo CC',cursor[1]&0x4700,0x0100)
    motion=next(e for e in r['events'] if e[0]==helper)
    check(r,'motion threshold CC',motion[1]&0x4700,0x0100 if v==0 else 0x4000 if v==1 else 0)
    check(r,'final TOP',r['sw']&0x3800,0)
    check(r,'final inherited sticky',r['sw']&0x20,0x20)
    check(r,'final CC',r['sw']&0x4700,0x4000 if v==2 else 0 if v==3 else 0x0100)
    check(r,'incidental EAX',r['eax'],0x7654abcd if v==3 else 0xd00d0000|r['sw'])
report=dict(cases=len(rows),checks=checks,failures=failures,scope='independent scripted replacement contract audit; original/native/ABI still pending')
(base/'GAME-0001-independent-core-audit.json').write_text(json.dumps(report,indent=2))
print(json.dumps(dict(cases=len(rows),checks=checks,failures=len(failures))))
raise SystemExit(bool(failures))
