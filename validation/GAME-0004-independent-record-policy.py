"""GAME-0004 approved transition checks only; no native execution/source export."""
import json,math,struct
from pathlib import Path
R=Path(r'C:/Users/ADMIN/CLionProjects/OpenBoxer')
META=json.loads((R/'specs/gameplay/GAME-0004-observer-metadata.json').read_text())
SPANS=[(int(x['rva'],16),x['size']) for x in META['capture']['globalExtents']]
def u(raw,offset=0):return struct.unpack_from('<I',raw,offset)[0]
def f(raw,offset=0):return struct.unpack_from('<f',raw,offset)[0]
def fbits(bits):return f(struct.pack('<I',bits))
def round32(x):return struct.unpack('<I',struct.pack('<f',x))[0]
def image(snapshot):
 raw=bytes.fromhex(snapshot['globals']);assert len(raw)==sum(n for _,n in SPANS)
 values={};cursor=0
 for base,size in SPANS:values[base]=raw[cursor:cursor+size];cursor+=size
 def get(rva,size=4):
  for base,data in values.items():
   if base<=rva and rva+size<=base+len(data):return data[rva-base:rva-base+size]
  raise KeyError(hex(rva))
 return get

def transition(record):
 """Input must separately pass full structural/ABI/FP/source admission audit.
 Two float32 threshold operands have <=48 product bits, hence binary64 is
 exact under the approved CW027F 53-bit precision. Arithmetic expectations
 use binary64 rounding per operation followed by one float32 store; this
 predicts values only and never substitutes for real full-FP comparison.
 """
 before,after=record['before'],record['after'];g,h=image(before),image(after)
 a,b=bytes.fromhex(before['actor232']),bytes.fromhex(after['actor232']);errors=[]
 def ck(ok,label):
  if not ok:errors.append(label)
 kind=u(g(0x17625c));index=u(g(0x184788));timer=f(g(0x176230));duration=f(g(0x1762a4+16*index));dt=f(g(0x176370));fatigue=f(g(0x176258))
 if kind not in [1,2,3] or not all(math.isfinite(x) for x in [timer,duration,dt,fatigue]):return {'classification':'UNKNOWN','errors':['transition operands outside prerequisite domain']}
 multiplier={1:1.0,2:fbits(0x3fa66666),3:fbits(0x3ff33333)}[kind];threshold=duration*multiplier;completed=timer>threshold;expectedFatigue=fatigue
 ck(g(0x17622d,1)==b'\x01','incoming active attack');ck(u(a,164) in [2*kind+1,2*kind+2],'incoming matching state');ck(g(0x176256,1)==b'\x00','incoming lock0')
 if completed:
  factor=f(g(0x1762bc))
  if not math.isfinite(factor):return {'classification':'completion','errors':['completion fixed factor nonfinite'],'factorSource':'table128 offset28; global5762BC'}
  expectedFatigue=fbits(round32(factor*fbits(0x3fc00000)+fatigue));ck(h(0x17622d,1)==b'\x00' and u(h(0x176230))==0 and u(b,164)==0,'completion flag/timer/state tuple')
 else:
  # Do not interpret or constrain unused completion factor on increment.
  ck(h(0x17622d,1)==b'\x01' and u(b,164)==u(a,164),'increment flag/state');ck(u(h(0x176230))==round32(fbits(0x3c23d70a)*dt+timer),'single final timer store')
 if expectedFatigue>0:expectedFatigue=fbits(round32(expectedFatigue-fbits(0x3e19999a)*dt))
 lock=expectedFatigue>=fbits(0x42c60000)
 if lock:expectedFatigue=fbits(0x42c80000)
 ck(u(h(0x176258))==round32(expectedFatigue),'fatigue addition/decay/clamp raw32');ck(h(0x176256,1)==bytes([int(lock)]),'new fatigue lock byte');ck(h(0x17625c)==g(0x17625c),'committed type unchanged');ck(h(0x1761c8,1)==g(0x1761c8,1),'player pending untouched');ck(a[168:171]==b[168:171],'all alternators untouched');ck(h(0x1762a0,128)==g(0x1762a0,128),'fighter table/fixed factor alias preserved');ck(h(0x17f710,7600)==g(0x17f710,7600),'effect pool unchanged')
 return {'status':'TRANSITION_CHECK_ONLY_NOT_SOURCE_ADMISSION','classification':'completion' if completed else 'increment','type':kind,'beforeState':u(a,164),'afterState':u(b,164),'thresholdBinary64Raw':struct.pack('<d',threshold).hex(),'timerRaw32':g(0x176230).hex(),'durationRaw32':g(0x1762a4+16*index).hex(),'factorRaw32':g(0x1762bc).hex(),'generatedLock':lock,'expectedFatigueRaw32':struct.pack('<I',round32(expectedFatigue)).hex(),'errors':errors}
