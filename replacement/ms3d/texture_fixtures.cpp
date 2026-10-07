#include "texture.hpp"
#include "texture_abi.hpp"
#include "world_runtime.hpp"
#include <vector>
#include <string>
#include <cstring>
#include <cstdio>
#include <sstream>
#include <iomanip>
namespace texture {
namespace {
enum class Mutation { none,shrink1,shrink0,grow3,uploadTable,lengthTable,lengthEmptyTable,lengthName,lengthCount };
struct Case { const char* name; int32_t count; std::vector<std::string> names; std::vector<uint32_t> ids; Mutation mutation=Mutation::none; unsigned passes=1; bool shared=false; uint32_t result=0; };
struct Recorder {
    Case test;
    std::vector<uint8_t> model,first,second;
    std::vector<std::string> names;
    std::vector<std::string> events;
    unsigned lengths=0,uploads=0;
    bool valid=true;
    explicit Recorder(const Case& value):test(value),model(0x8d9c4,0x5b),first(value.names.size()*80,0xa7),second(value.names.size()*80,0x6c) {
        names.reserve(test.names.size()*2+1);
        for(const auto& text:test.names) names.push_back(text);
        for(unsigned i=0;i<test.names.size();i++) names.push_back("relocated"+std::to_string(i)+".bmp");
        names.push_back("");
        setCount(test.count); setTable(first.data());
        for(unsigned i=0;i<test.names.size();i++) {
            setId(first,i,0x11110000+i); setId(second,i,0x22220000+i);
            setName(first,i,names[(test.shared?0:i)].c_str()); setName(second,i,names[test.names.size()+i].c_str());
        }
    }
    void setCount(int32_t value) { std::memcpy(model.data()+countOffset,&value,4); }
    void setTable(uint8_t* value) { std::memcpy(model.data()+tableOffset,&value,4); }
    static void setId(std::vector<uint8_t>& data,unsigned i,uint32_t id) { std::memcpy(data.data()+i*80+72,&id,4); }
    static uint32_t id(const std::vector<uint8_t>& data,unsigned i) { uint32_t value; std::memcpy(&value,data.data()+i*80+72,4); return value; }
    static void setName(std::vector<uint8_t>& data,unsigned i,const char* name) { std::memcpy(data.data()+i*80+76,&name,4); }
    std::string identity(const char* pointer) const { for(unsigned i=0;i<names.size();i++) if(names[i].c_str()==pointer) return "string"+std::to_string(i); return "UNKNOWN_STRING"; }
    std::string snapshot() const {
        std::ostringstream out;
        out<<"count="<<texture::count(const_cast<uint8_t*>(model.data()))<<" table="<<(texture::table(const_cast<uint8_t*>(model.data()))==first.data()?"A":"B");
        for(unsigned which=0;which<2;which++) {
            const auto& data=which?second:first; out<<" "<<(which?"B":"A")<<"=[";
            for(unsigned i=0;i<test.names.size();i++) {
                const char* pointer; std::memcpy(&pointer,data.data()+i*80+76,4);
                out<<std::hex<<id(data,i)<<':'<<identity(pointer)<<';';
            }
            out<<']';
        }
        return out.str();
    }
    bool untouched() const {
        for(std::size_t i=0;i<model.size();i++) if((i<countOffset || i>=tableOffset+4) && model[i]!=0x5b) return false;
        for(unsigned i=0;i<test.names.size();i++) for(unsigned j=0;j<72;j++) if(first[i*80+j]!=0xa7 || second[i*80+j]!=0x6c) return false;
        return true;
    }
    void record(const std::string& callback,const char* pointer,uint32_t result) { events.push_back(callback+"("+identity(pointer)+",path="+pointer+",return="+std::to_string(result)+") | "+snapshot()); }
};
thread_local Recorder* current=nullptr;
uint32_t __cdecl length(const char* name) {
    auto& r=*current; const uint32_t result=static_cast<uint32_t>(std::strlen(name)); r.record("strlen",name,result);
    ++r.lengths;
    if(r.lengths==1) {
        if(r.test.mutation==Mutation::lengthTable || r.test.mutation==Mutation::lengthEmptyTable) r.setTable(r.second.data());
        if(r.test.mutation==Mutation::lengthName) Recorder::setName(r.first,0,r.names.back().c_str());
        if(r.test.mutation==Mutation::lengthCount) r.setCount(0);
    }
    return result;
}
uint32_t __cdecl upload(const char* name) {
    auto& r=*current;
    const uint32_t result=r.uploads<r.test.ids.size()?r.test.ids[r.uploads]:0xdeadbeef;
    r.valid=r.valid && r.uploads<r.test.ids.size(); r.record("upload",name,result); ++r.uploads;
    if(r.uploads==1) {
        if(r.test.mutation==Mutation::shrink1) r.setCount(1);
        if(r.test.mutation==Mutation::shrink0) r.setCount(0);
        if(r.test.mutation==Mutation::grow3) r.setCount(3);
        if(r.test.mutation==Mutation::uploadTable) r.setTable(r.second.data());
    }
    return result;
}
uint32_t __attribute__((thiscall)) offline(void* object) { return run(object,{length,upload}); }
std::vector<Case> cases() {
    return {
        {"zero",0,{"a.bmp"},{},Mutation::none,1,false,0xcccccccc},
        {"negative",-1,{"a.bmp"},{},Mutation::none,1,false,0xcccccccc},
        {"minimum_signed",INT32_MIN,{"a.bmp"},{},Mutation::none,1,false,0xcccccccc},
        {"empty",1,{""},{},Mutation::none,1,false,1},
        {"large_id",1,{"a.bmp"},{0xffffffff},Mutation::none,1,false,1},
        {"zero_id",1,{"missing.bmp"},{0},Mutation::none,1,false,1},
        {"failure_then_later",5,{"a.bmp","","a.bmp","missing.bmp","later.bmp"},{11,22,0,0xffffffff},Mutation::none,1,false,5},
        {"all_empty",3,{"","",""},{},Mutation::none,1,false,3},
        {"same_pointer_twice",2,{"same.bmp","same.bmp"},{11,22},Mutation::none,1,true,2},
        {"second_reload",3,{"a.bmp","","b.bmp"},{10,20,30,40},Mutation::none,2,false,3},
        {"count_shrink1",3,{"a.bmp","b.bmp","c.bmp"},{101},Mutation::shrink1,1,false,1},
        {"count_shrink0",3,{"a.bmp","b.bmp","c.bmp"},{102},Mutation::shrink0,1,false,1},
        {"count_grow3",1,{"a.bmp","b.bmp","c.bmp"},{101,102,103},Mutation::grow3,1,false,3},
        {"upload_table_move",3,{"a.bmp","b.bmp","c.bmp"},{11,22,33},Mutation::uploadTable,1,false,3},
        {"strlen_table_move",2,{"a.bmp","b.bmp"},{11,22},Mutation::lengthTable,1,false,2},
        {"strlen_empty_table_move",2,{"","b.bmp"},{22},Mutation::lengthEmptyTable,1,false,2},
        {"strlen_filename_move",1,{"a.bmp"},{0},Mutation::lengthName,1,false,1},
        {"strlen_count_shrink",3,{"a.bmp","b.bmp","c.bmp"},{11},Mutation::lengthCount,1,false,1}
    };
}
bool oracle(const Recorder& r,uint32_t result) {
    if(!r.valid || !r.untouched() || result!=r.test.result || r.uploads!=r.test.ids.size()) return false;
    if(r.test.count<=0) return r.events.empty() && Recorder::id(r.first,0)==0x11110000;
    if(r.test.mutation==Mutation::none) {
        unsigned at=(r.test.passes-1)*static_cast<unsigned>(r.test.ids.size()/r.test.passes);
        for(unsigned i=0;i<r.test.names.size();i++) {
            const uint32_t expected=r.test.names[i].empty()?0:r.test.ids[at++];
            if(Recorder::id(r.first,i)!=expected) return false;
        }
    }
    if(r.test.mutation==Mutation::uploadTable || r.test.mutation==Mutation::lengthTable || r.test.mutation==Mutation::lengthEmptyTable) {
        for(unsigned i=0;i<r.test.names.size();i++) if(Recorder::id(r.first,i)!=0x11110000+i) return false;
        const uint32_t first=r.test.mutation==Mutation::lengthEmptyTable?0:r.test.ids[0];
        if(Recorder::id(r.second,0)!=first || table(const_cast<uint8_t*>(r.model.data()))!=r.second.data()) return false;
    }
    if(r.test.mutation==Mutation::lengthName && Recorder::id(r.first,0)!=0) return false;
    if((r.test.mutation==Mutation::shrink0 || r.test.mutation==Mutation::shrink1 || r.test.mutation==Mutation::lengthCount) && Recorder::id(r.first,1)!=0x11110001) return false;
    return true;
}
void emit(FILE* file,unsigned scenario,const Recorder& r) {
    for(unsigned sequence=0;sequence<r.events.size();sequence++) std::fprintf(file,"scenario=%u sequence=%u %s\n",scenario,sequence,r.events[sequence].c_str());
    std::fprintf(file,"scenario=%u FINAL %s untouched=%d\n",scenario,r.snapshot().c_str(),r.untouched());
}
struct Result { uint32_t value=0; bool abi=true,fp=true; WorldAbiReport registers{}; uint16_t cwBefore=0,cwAfter=0; uint32_t mxBefore=0,mxAfter=0; };
Result invoke(Reload callable,Recorder& r) {
    Result result; current=&r;
    for(unsigned pass=0;pass<r.test.passes;pass++) {
        __asm__ volatile("fnstcw %0":"=m"(result.cwBefore)); __asm__ volatile("stmxcsr %0":"=m"(result.mxBefore));
        result.value=texture_abi_probe(callable,r.model.data(),&result.registers);
        __asm__ volatile("fnstcw %0":"=m"(result.cwAfter)); __asm__ volatile("stmxcsr %0":"=m"(result.mxAfter));
        result.abi=result.abi && worldAbiValid(result.registers);
        result.fp=result.fp && result.cwBefore==result.cwAfter && (result.mxBefore&0xffc0)==(result.mxAfter&0xffc0);
    }
    current=nullptr; return result;
}
}
uint32_t offlineFixtures() {
    unsigned failures=0;
    for(const auto& test:cases()) { Recorder recorder(test); const auto result=invoke(offline,recorder); if(!oracle(recorder,result.value) || !result.abi || !result.fp) ++failures; }
    return failures;
}
uint32_t fixtures(uintptr_t module) {
    const auto inputs=cases();
    FILE* summary=std::fopen("C:/Users/ADMIN/Boxer-lab/ms3d/texture-fixtures-summary.txt","wb");
    FILE* originals=std::fopen("C:/Users/ADMIN/Boxer-lab/ms3d/texture-original-trace.txt","wb");
    FILE* candidates=std::fopen("C:/Users/ADMIN/Boxer-lab/ms3d/texture-candidate-trace.txt","wb");
    if(!summary || !originals || !candidates) return 90;
    if(!world::redirect(module+0x1799,reinterpret_cast<uintptr_t>(&upload)) || !world::redirect(module+0x99880,reinterpret_cast<uintptr_t>(&length)) || !install(module,"replace")) return 91;
    fixtureMode(true);
    unsigned failures=0,id=0;
    const auto original=reinterpret_cast<Reload>(module+0x16680),candidate=reinterpret_cast<Reload>(module+0x1a64);
    for(const auto& test:inputs) {
        ++id; Recorder a(test),b(test);
        const auto ar=invoke(original,a); const auto before=replacementCount(); const auto br=invoke(candidate,b);
        const bool route=replacementCount()==before+test.passes,trace=a.events==b.events,state=a.snapshot()==b.snapshot();
        const bool exact=route && trace && state && ar.value==br.value && oracle(a,ar.value) && oracle(b,br.value) && ar.abi && br.abi && ar.fp && br.fp;
        if(!exact) ++failures;
        emit(originals,id,a); emit(candidates,id,b);
        std::fprintf(summary,"scenario=%u name=%s initialCount=%d passes=%u originalEAX=%08x candidateEAX=%08x route=%d trace=%d state=%d oracle=%d/%d untouched=%d/%d ABI=%d/%d FP=%d/%d result=%s\n",id,test.name,test.count,test.passes,ar.value,br.value,route,trace,state,oracle(a,ar.value),oracle(b,br.value),a.untouched(),b.untouched(),ar.abi,br.abi,ar.fp,br.fp,exact?"PASS":"FAIL");
        std::fprintf(summary," abi originalESP=%08x/%08x candidateESP=%08x/%08x originalNV=%08x/%08x/%08x/%08x candidateNV=%08x/%08x/%08x/%08x x87=%04x/%04x,%04x/%04x MXCSR=%08x/%08x,%08x/%08x\n",ar.registers.before,ar.registers.after,br.registers.before,br.registers.after,ar.registers.ebp,ar.registers.ebx,ar.registers.esi,ar.registers.edi,br.registers.ebp,br.registers.ebx,br.registers.esi,br.registers.edi,ar.cwBefore,ar.cwAfter,br.cwBefore,br.cwAfter,ar.mxBefore,ar.mxAfter,br.mxBefore,br.mxAfter);
        if(!trace) for(unsigned i=0;i<(std::min)(a.events.size(),b.events.size());i++) if(a.events[i]!=b.events[i]) { std::fprintf(summary," firstDifference=%u original=%s candidate=%s\n",i,a.events[i].c_str(),b.events[i].c_str()); break; }
        std::fflush(summary);
    }
    std::fprintf(summary,"TOTAL scenarios=%u failures=%u\n",id,failures); std::fclose(summary); std::fclose(originals); std::fclose(candidates); return failures;
}
}
