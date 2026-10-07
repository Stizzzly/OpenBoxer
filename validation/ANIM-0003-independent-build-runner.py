"""Emit integer/ordered-write future-slot runner from independent oracle data."""
import json
from pathlib import Path
HERE=Path(__file__).resolve().parent
cases=json.loads((HERE/'ANIM-0003-independent-vectors.json').read_text())
expected=json.loads((HERE/'ANIM-0003-independent-expected.json').read_text())
code=r'''
#include "frames.hpp"
#include <vector>
#include <cstring>
#include <cstdio>
#include <stdexcept>
using B=std::vector<uint8_t>;
B hex(const char*s){B v;for(;*s;s+=2){unsigned x;sscanf(s,"%2x",&x);v.push_back(uint8_t(x));}return v;}
uint32_t rd(const B&v,int o){uint32_t x;memcpy(&x,v.data()+o,4);return x;}
void wr(B&v,int o,uint32_t x){memcpy(v.data()+o,&x,4);}
void check(bool b){if(!b)throw std::runtime_error("oracle mismatch");}
struct W{unsigned offset;uint32_t bits;bool operator==(const W&b)const{return offset==b.offset&&bits==b.bits;}};
struct Fixture:frames::State {
 B model,record,prepared;std::vector<W>lm,lr,am,actual;uint32_t opaque,eax;int expectedIndex;unsigned calls=0;
 int32_t selector()const override{return int32_t(rd(model,44));}
 void*lookup(int32_t i)override{check(calls++==0&&i==expectedIndex);for(auto&w:lm)wr(model,w.offset,w.bits);for(auto&w:lr)wr(record,w.offset,w.bits);return record.empty()?nullptr:record.data();}
 int32_t gate()const override{return int32_t(rd(model,40));}
 int32_t recordStart(void*r)const override{check(r!=nullptr&&r==record.data());return int32_t(rd(record,256));}
 int32_t divisor(void*r)const override{check(r!=nullptr&&r==record.data());return int32_t(rd(record,260));}
 uint32_t frame()const override{return rd(model,48);}
 uint32_t slot(unsigned o)const override{return rd(model,o);}
 void slot(unsigned o,uint32_t v)override{actual.push_back({o,v});wr(model,o,v);}
 uint32_t advance(uint32_t ecx)override{check(calls++==1&&ecx==opaque&&model==prepared);for(auto&w:am)wr(model,w.offset,w.bits);return eax;}
};
int main(){int passes=0;
'''
for c,e in zip(cases,expected):
    code+='try{Fixture f;\n'
    code+=f'f.model=hex("{c["model_hex"]}");f.record=hex("{c.get("record_hex") or ""}");f.prepared=hex("{e["prepared_model_hex"]}");f.opaque={c["entry_ecx_bits"]}u;f.eax={c["advance_eax"]}u;f.expectedIndex={int.from_bytes(bytes.fromhex(c["model_hex"])[44:48],"little",signed=True)};\n'
    for src,key in (('lm','lookup_model_writes'),('lr','lookup_record_writes'),('am','advance_model_writes')):
        for w in c.get(key,[]):code+=f'f.{src}.push_back(W{{{w["offset"]},{w["bits"]&0xffffffff}u}});\n'
    ws=','.join(f'W{{{w["offset"]},{w["bits"]}u}}' for w in e['own_writes'])
    code+=f'check(frames::prepare(f,f.opaque)=={e["eax"]}u);check(f.calls==2);check(f.model==hex("{e["model_hex"]}"));check(f.actual==std::vector<W>{{{ws}}});\n'
    code+=f'puts("PASS {c["id"]}");++passes;}}catch(const std::exception&e){{printf("FAIL {c["id"]}: %s\\n",e.what());return 1;}}\n'
code+='printf("TOTAL %d\\n",passes);return 0;}\n'
(HERE/'ANIM-0003-independent-runner.cpp').write_text(code,encoding='utf-8')
print(f'emitted {len(cases)} cases')
