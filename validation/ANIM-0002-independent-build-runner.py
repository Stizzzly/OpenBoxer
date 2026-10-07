"""Emit a spec-oracle-driven core runner, independent of implementation tests."""
import json
from pathlib import Path
HERE=Path(__file__).resolve().parent
cases=json.loads((HERE/'ANIM-0002-independent-vectors.json').read_text())
expected=json.loads((HERE/'ANIM-0002-independent-expected.json').read_text())
code=r'''
#include "clip.hpp"
#include <array>
#include <vector>
#include <string>
#include <cstring>
#include <cstdio>
#include <stdexcept>
using Bytes=std::vector<uint8_t>;
Bytes hex(const char*s){Bytes v;for(;*s;s+=2){unsigned x;sscanf(s,"%2x",&x);v.push_back(uint8_t(x));}return v;}
uint32_t rd(const Bytes&v,int o){uint32_t x;memcpy(&x,v.data()+o,4);return x;}
void wr(Bytes&v,int o,uint32_t x){memcpy(v.data()+o,&x,4);}
std::string name(const Bytes&v){auto p=memchr(v.data(),0,v.size());if(!p)throw std::runtime_error("unterminated");return std::string((char*)v.data(),(char*)p);}
void check(bool b){if(!b)throw std::runtime_error("oracle mismatch");}
struct FP {alignas(16) uint8_t bytes[512];};
FP fp(){FP e;__asm__ volatile("fxsave %0":"=m"(e));return e;}
bool sameFP(const FP&a,const FP&b){return memcmp(a.bytes,b.bytes,5)==0&&memcmp(a.bytes+24,b.bytes+24,4)==0;}
struct Action {int kind,index,record,result;Bytes before,after;uint32_t t_before,t_after;std::string rec_name,req_name;};
struct Write{int rec,offset;uint32_t bits;};
struct Mut{std::vector<Write>w;bool request=false;Bytes req;};
struct Fixture:clip::State{
 Bytes model,request;std::vector<Bytes>records;uint32_t time=0,owner_bits=0;size_t n=0;
 std::vector<Action>actions;std::vector<Mut>mut;
 bool fpMutation=false;FP lastFP{};
 int32_t bound()const override{return int32_t(rd(model,40));}
 Action& act(int k){check(n<actions.size());auto&a=actions[n];check(a.kind==k&&a.before==model&&a.t_before==time);return a;}
 void effects(){auto&a=actions[n];auto&m=mut[n];for(auto&w:m.w)wr(w.rec<0?model:records[w.rec],w.offset,w.bits);if(m.request)request=m.req;time=a.t_after;check(a.after==model);++n;if(fpMutation){uint16_t cw=0x037f;uint32_t mx=0x3f81;__asm__ volatile("fninit; fldcw %0; ldmxcsr %1; fld1"::"m"(cw),"m"(mx));lastFP=fp();}}
 void*lookup(int32_t i)override{auto&a=act(0);check(a.index==i);int rec=a.record;effects();return records[rec].data();}
 int32_t compare(void*p)override{auto&a=act(1);int rec=-1;for(size_t i=0;i<records.size();++i)if(records[i].data()==p)rec=int(i);check(rec==a.record);check(name(records[rec])==a.rec_name&&name(request)==a.req_name);int r=a.result;effects();return r;}
 void selector(int32_t i)override{wr(model,44,uint32_t(i));}int32_t selector()const override{return int32_t(rd(model,44));}
 uint32_t start(void*p)const override{uint32_t x;memcpy(&x,(uint8_t*)p+256,4);return x;}
 void frame(uint32_t x)override{wr(model,48,x);}void factor(uint32_t x)override{wr(model,76,x);}
 uint32_t clock()const override{return time;}void anchor(uint32_t x)override{wr(model,80,x);}uint32_t owner()const override{return owner_bits;}
};
int main(){int passes=0;
'''
for case_index,(c,e) in enumerate(zip(cases,expected)):
    code+='try{Fixture f;\n'
    code+=f'f.model=hex("{c["model_hex"]}");f.request=hex("{c["request_hex"]}");f.time={c["time_bits"]}u;f.owner_bits={c["owner_bits"]}u;\n'
    for k in sorted(c['records'],key=int):code+=f'f.records.push_back(hex("{c["records"][k]}"));\n'
    for a,o in zip(c['dependencies'],e['callbacks']):
        code+='f.actions.push_back(Action{'+','.join([str(0 if a['kind']=='lookup' else 1),str(o.get('index',0)),str(o.get('return_record_role',o.get('record_role','0'))),str(o.get('result',0)),f'hex("{o["model_hex_before"]}")',f'hex("{o["model_hex_after"]}")',str(o['time_bits_before'])+'u',str(o['time_bits_after'])+'u',json.dumps(bytes.fromhex(o.get('record_name_hex','')).decode('ascii')),json.dumps(bytes.fromhex(o.get('request_name_hex','')).decode('ascii'))])+'});\n'
        ws=[f'Write{{-1,{w["offset"]},{w["bits"]&0xffffffff}u}}' for w in a.get('model_writes',[])]+[f'Write{{{w["record"]},{w["offset"]},{w["bits"]&0xffffffff}u}}' for w in a.get('record_writes',[])]
        code+='f.mut.push_back(Mut{{'+','.join(ws)+'},'+str('request_hex' in a).lower()+',hex("'+a.get('request_hex','')+'")});\n'
    if c['id']=='match_0':code+='f.fpMutation=true;\n'
    code+=f'check(clip::select(f)=={e["eax"]}u);check(f.n==f.actions.size());check(f.model==hex("{e["model_hex"]}"));check(f.time=={e["time_bits"]}u);'
    if c['id']=='match_0':code+='check(sameFP(fp(),f.lastFP));__asm__ volatile("fninit");uint32_t mx=0x1f80;__asm__ volatile("ldmxcsr %0"::"m"(mx));'
    code+=f'puts("PASS {c["id"]}");++passes;}}catch(const std::exception&e){{printf("FAIL {c["id"]}: %s\\n",e.what());return 1;}}\n'
code+='printf("TOTAL %d\\n",passes);return 0;}\n'
(HERE/'ANIM-0002-independent-runner.cpp').write_text(code,encoding='utf-8')
print(f'emitted {len(cases)} cases')
