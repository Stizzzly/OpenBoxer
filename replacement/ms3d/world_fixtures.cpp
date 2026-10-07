#include "world_fixtures.hpp"
#include "world_runtime.hpp"
#include "world_abi.hpp"
#include <windows.h>
#include <vector>
#include <string>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <sstream>
#include <iomanip>
namespace world {
namespace {
struct Field { uint32_t va; unsigned width; };
constexpr Field fields[]={
    {0x577f9f,1},{0x577fb8,1},{0x584770,4},{0x584788,4},
    {0x585594,4},{0x575d94,4},{0x584798,4},{0x585560,4},{0x585564,4},
    {0x56fe70,4},{0x58477c,4},{0x584780,4},{0x584778,4},
    {0x56ff18,4},{0x585570,4},{0x56ff1c,4},{0x56ff20,4},{0x585574,4},{0x56ff24,4},
    {0x5853a2,1},{0x584794,1},{0x58479c,1},{0x58479d,1}
};
struct Scenario { int selector,sound,mode,decision,signedInput,random,script; };
struct Recorder {
    State state;
    Scenario scenario;
    std::vector<void*> objects;
    std::vector<std::string> events;
    void* holder=nullptr;
    unsigned holderSerial=0;
    bool valid=true;
    ~Recorder() { for(auto* value:objects) std::free(value); }
    std::string object(void* value) const {
        if(!value) return "null";
        for(std::size_t i=0;i<objects.size();i++) if(objects[i]==value) return "model"+std::to_string(i+1);
        if(value==state.address(0x575e20)) return "module+175e20";
        if(value==state.address(0x57b2b0)) return "module+17b2b0";
        if(value==holder) return "holder"+std::to_string(holderSerial);
        return "UNKNOWN_POINTER";
    }
    std::string snapshot() const {
        std::ostringstream out;
        for(const auto f:fields) {
            out<<std::hex<<f.va<<'=';
            if(f.va==0x585560 || f.va==0x585564) out<<object(state.pointer(f.va));
            else if(f.width==1) out<<unsigned(state.byte(f.va));
            else out<<static_cast<uint32_t>(state.integer(f.va));
            out<<';';
        }
        // Width witnesses immediately adjacent to byte-sized direct stores.
        for(uint32_t va:{0x5853a1u,0x5853a3u,0x584793u,0x584795u,0x58479bu,0x58479eu}) out<<std::hex<<va<<'='<<unsigned(state.byte(va))<<';';
        return out.str();
    }
    void record(const std::string& name,const std::string& args="") { events.push_back(name+"("+args+") | "+snapshot()); }
};
thread_local Recorder* current=nullptr;
static void* __cdecl allocate(uint32_t size) {
    auto& r=*current; r.valid=r.valid && size==0x8d9c4 && r.objects.size()<2;
    void* value=std::calloc(1,size); if(!value) std::abort(); r.objects.push_back(value);
    r.record("allocate",std::to_string(size)+"->"+r.object(value)); return value;
}
static bool __attribute__((thiscall)) loadModel(void* object,const char* path) { auto& r=*current; r.record("loadModel",r.object(object)+","+path); return false; }
static LoadModel fakeTable[2]={nullptr,loadModel};
static void* __attribute__((thiscall)) construct(void* object) { auto& r=*current; r.record("construct",r.object(object)); *reinterpret_cast<LoadModel**>(object)=fakeTable; return object; }
static void __attribute__((thiscall)) reload(void* object) { auto& r=*current; r.record("reload",r.object(object)); }
static void __attribute__((thiscall)) lightA(void* object) { auto& r=*current; r.record("lightA",r.object(object)); }
static void __attribute__((thiscall)) lightB(void* object) { auto& r=*current; r.record("lightB",r.object(object)); }
static void __cdecl stageA() { current->record("stageA"); }
static void __cdecl stageB() { current->record("stageB"); }
static void __cdecl stageC() {
    auto& r=*current; r.record("stageC"); r.state.store(0x585594,static_cast<uint32_t>(r.scenario.decision));
    if(r.scenario.script==1) { r.state.storeByte(0x577f9f,!r.scenario.sound); r.state.storeByte(0x577fb8,!r.scenario.mode); r.state.store(0x584770,0x13579bdf); }
}
static void __cdecl stageD() { auto& r=*current; r.record("stageD"); if(r.scenario.script==2) r.state.store(0x585594,2); }
static void __cdecl optional(int32_t value) { auto& r=*current; r.record("optional",std::to_string(value)); if(r.scenario.script==3) r.state.store(0x585594,1); }
static void __cdecl callbackA(int32_t value) { auto& r=*current; r.record("callback2000A",std::to_string(value)); if(r.scenario.script==4) r.state.store(0x585594,2); }
static void __cdecl callbackB(int32_t value) { current->record("callback2000B",std::to_string(value)); }
static int32_t __cdecl random() { current->record("rand",std::to_string(current->scenario.random)); return current->scenario.random; }
static int32_t __cdecl final() { current->record("final","sentinel=-19088743"); return -19088743; }
static void* __attribute__((thiscall)) holder(void* object,const char* filename,void* scratch) {
    auto& r=*current; r.holder=object; ++r.holderSerial;
    bool bytes=true; for(unsigned i=0;i<16;i++) bytes=bytes && static_cast<uint8_t*>(object)[i]==0xcc;
    for(unsigned i=0;i<4;i++) bytes=bytes && static_cast<uint8_t*>(scratch)[i]==0xcc;
    r.valid=r.valid && bytes && (reinterpret_cast<uintptr_t>(object)%4==0) && (reinterpret_cast<uintptr_t>(scratch)%4==0);
    r.record("holder",r.object(object)+","+filename+",scratch=cccccccc,holder_cc="+std::to_string(bytes));
    std::memset(object,0x19,16); return object;
}
static void __attribute__((thiscall)) audioLoad(void* audio,void* object,int32_t first,int32_t second) {
    auto& r=*current; r.valid=r.valid && object==r.holder && first==1 && second==1;
    r.record("audioLoad",r.object(audio)+","+r.object(object)+","+std::to_string(first)+","+std::to_string(second));
    if(r.scenario.script==5 && r.holderSerial==1) r.state.store(0x575d94,2);
}
static void __attribute__((thiscall)) destroyHolder(void* object) { auto& r=*current; r.valid=r.valid && object==r.holder; r.record("holderDestroy",r.object(object)); }
static void __attribute__((thiscall)) resetAudio(void* object) { auto& r=*current; r.record("audioReset",r.object(object)); }
static void __attribute__((thiscall)) playAudio(void* object) { auto& r=*current; r.record("audioPlay",r.object(object)); }
static int32_t __stdcall cursor(int32_t x,int32_t y) { current->valid=current->valid && x==320 && y==240; current->record("SetCursorPos",std::to_string(x)+","+std::to_string(y)); return 77; }
static Callbacks recorderCallbacks() { return {allocate,construct,reload,lightA,lightB,stageA,stageB,stageC,stageD,optional,callbackA,callbackB,random,final,holder,audioLoad,destroyHolder,resetAudio,playAudio,cursor}; }
static void reset(State s,const Scenario& p) {
    for(const auto f:fields) { if(f.width==1) s.storeByte(f.va,0xa5); else s.store(f.va,0xa5a5a5a5); }
    for(uint32_t va:{0x5853a1u,0x5853a3u,0x584793u,0x584795u,0x58479bu,0x58479eu}) s.storeByte(va,0x6d);
    s.storeByte(0x577f9f,static_cast<uint8_t>(p.sound)); s.storeByte(0x577fb8,static_cast<uint8_t>(p.mode));
    s.store(0x584770,static_cast<uint32_t>(p.selector)); s.store(0x584788,static_cast<uint32_t>(p.signedInput));
    s.storePointer(0x585560,nullptr); s.storePointer(0x585564,nullptr);
}
static std::vector<Scenario> scenarios() {
    std::vector<Scenario> result;
    for(int selector:{1,2,3,4,5,6,7,8,0,9,-1}) for(int sound:{0,1}) for(int mode:{0,1}) for(int decision:{0,1,2,-1}) for(int input:{-1,0,1}) for(int rng:{0,1,2,3,7}) result.push_back({selector,sound,mode,decision,input,rng,0});
    for(int sound:{0,1}) for(int mode:{0,1}) result.push_back({2,sound,mode,0,1,3,1});
    result.push_back({1,0,0,0,0,0,2}); result.push_back({1,0,1,2,0,1,3});
    result.push_back({1,0,0,1,0,2,4}); result.push_back({1,0,0,0,0,0,5});
    return result;
}
static std::vector<std::string> eventNames(const Recorder& recorder) {
    std::vector<std::string> result; for(const auto& e:recorder.events) result.push_back(e.substr(0,e.find('('))); return result;
}
static bool oracle(const Recorder& r,int32_t result) {
    const auto& p=r.scenario;
    const int sound=p.script==1?!p.sound:p.sound,mode=p.script==1?!p.mode:p.mode;
    int decision=p.decision; if(p.script==3 && mode) decision=1;
    std::vector<std::string> expected;
    if(p.sound) expected.push_back("audioReset");
    for(unsigned model=0;model<2;model++) { expected.push_back("allocate"); expected.push_back("construct"); if(p.selector>=1 && p.selector<=8) expected.push_back("loadModel"); expected.push_back("reload"); }
    expected.insert(expected.end(),{"lightA","lightB","stageA","stageB","stageC"});
    if(mode) expected.push_back("optional");
    if(decision==0 || decision==1) { expected.push_back("stageD"); if(p.script==2) decision=2; }
    expected.insert(expected.end(),{"rand","holder","audioLoad","holderDestroy"});
    if(p.script==5) expected.insert(expected.end(),{"holder","audioLoad","holderDestroy"});
    if(sound) expected.push_back("audioPlay"); expected.push_back("SetCursorPos");
    if(decision==1) { expected.push_back("callback2000A"); if(p.script==4) decision=2; }
    if(decision==2) expected.push_back("callback2000B"); expected.push_back("final");
    const uint32_t offset=mode?(p.signedInput==0?0xc1800000u:p.signedInput>0?0xc1000000u:0u):0u;
    const auto s=r.state;
    return r.valid && result==-19088743 && r.objects.size()==2 && eventNames(r)==expected &&
        static_cast<uint32_t>(s.integer(0x584778))==offset && s.integer(0x584798)==1 &&
        static_cast<uint32_t>(s.integer(0x56fe70))==0xbfc00000 && s.byte(0x5853a2)==1 &&
        static_cast<uint32_t>(s.integer(0x56ff18))==0x3f800000 && static_cast<uint32_t>(s.integer(0x56ff1c))==0x40a00000 &&
        static_cast<uint32_t>(s.integer(0x585574))==0xc0000000 && s.byte(0x584794)==0 && s.byte(0x58479c)==0 && s.byte(0x58479d)==0 &&
        s.byte(0x5853a1)==0x6d && s.byte(0x5853a3)==0x6d && s.byte(0x584793)==0x6d && s.byte(0x584795)==0x6d && s.byte(0x58479b)==0 && s.byte(0x58479e)==0x6d;
}
static bool patchRecorders(uintptr_t base) {
    struct Redirect { uint32_t rva; uintptr_t target; };
    const Redirect routes[]={
        {0x95420,reinterpret_cast<uintptr_t>(&allocate)},{0x1956,reinterpret_cast<uintptr_t>(&construct)},
        {0x1a64,reinterpret_cast<uintptr_t>(&reload)},{0x1690,reinterpret_cast<uintptr_t>(&lightA)},{0x1974,reinterpret_cast<uintptr_t>(&lightB)},
        {0x10e1,reinterpret_cast<uintptr_t>(&stageA)},{0x127b,reinterpret_cast<uintptr_t>(&stageB)},{0x1131,reinterpret_cast<uintptr_t>(&stageC)},
        {0x13d4,reinterpret_cast<uintptr_t>(&optional)},{0x1e2e,reinterpret_cast<uintptr_t>(&stageD)},
        {0x9a750,reinterpret_cast<uintptr_t>(&random)},{0x1924,reinterpret_cast<uintptr_t>(&holder)},
        {0x1fc8,reinterpret_cast<uintptr_t>(&audioLoad)},{0x1555,reinterpret_cast<uintptr_t>(&destroyHolder)},
        {0x169f,reinterpret_cast<uintptr_t>(&resetAudio)},{0x1bf9,reinterpret_cast<uintptr_t>(&playAudio)},
        {0x14ec,reinterpret_cast<uintptr_t>(&callbackA)},{0x12fd,reinterpret_cast<uintptr_t>(&callbackB)},{0x1122,reinterpret_cast<uintptr_t>(&final)}
    };
    for(const auto route:routes) if(!redirect(base+route.rva,route.target)) return false;
    auto* slot=reinterpret_cast<Cursor*>(base+0x18cca0); DWORD old;
    if(!VirtualProtect(slot,4,PAGE_READWRITE,&old)) return false;
    *slot=cursor; DWORD ignored; return VirtualProtect(slot,4,old,&ignored)!=0;
}
static void emit(FILE* file,unsigned id,const Recorder& r) {
    for(std::size_t sequence=0;sequence<r.events.size();sequence++) std::fprintf(file,"scenario=%u sequence=%u %s\n",id,static_cast<unsigned>(sequence),r.events[sequence].c_str());
    std::fprintf(file,"scenario=%u FINAL %s\n",id,r.snapshot().c_str());
}
static int32_t __cdecl offlineEntry(int32_t selector,int32_t,int32_t) { return run(current->state,recorderCallbacks(),selector); }
}
uint32_t offlineFixtures() {
    std::vector<uint8_t> image(0x19f000);
    State state{image.data()}; unsigned failures=0;
    for(const auto scenario:scenarios()) {
        reset(state,scenario); Recorder recorder{state,scenario}; current=&recorder;
        WorldAbiReport report{};
        const int32_t result=world_abi_probe(offlineEntry,scenario.selector,1,1,&report);
        if(!oracle(recorder,result) || !worldAbiValid(report)) ++failures;
        current=nullptr;
    }
    return failures;
}
uint32_t fixtures(uintptr_t base) {
    const auto cases=scenarios(); // Allocate private infrastructure before entry redirects.
    FILE* summary=std::fopen("C:/Users/ADMIN/Boxer-lab/ms3d/world-fixtures-summary.txt","wb");
    FILE* originals=std::fopen("C:/Users/ADMIN/Boxer-lab/ms3d/world-original-trace.txt","wb");
    FILE* candidates=std::fopen("C:/Users/ADMIN/Boxer-lab/ms3d/world-candidate-trace.txt","wb");
    if(!summary || !originals || !candidates) return 90;
    if(!patchRecorders(base) || !install(base,"replace")) return 91;
    setFixtureMode(true);
    unsigned failures=0,id=0;
    State state{reinterpret_cast<uint8_t*>(base)};
    const auto original=reinterpret_cast<MapLoad>(base+0x2e970),candidate=reinterpret_cast<MapLoad>(base+0x10be);
    for(const auto scenario:cases) {
        ++id; std::vector<std::string> events; std::string finalState; int32_t originalResult=0,candidateResult=0;
        bool originalOracle=false,candidateOracle=false; uintptr_t originalBefore,originalAfter,candidateBefore,candidateAfter;
        WorldAbiReport originalAbi{},candidateAbi{};
        unsigned short originalCwBefore,originalCwAfter,candidateCwBefore,candidateCwAfter;
        unsigned originalMxBefore,originalMxAfter,candidateMxBefore,candidateMxAfter;
        {
            reset(state,scenario); Recorder recorder{state,scenario}; current=&recorder;
            __asm__ volatile("fnstcw %0":"=m"(originalCwBefore)); __asm__ volatile("stmxcsr %0":"=m"(originalMxBefore));
            __asm__ volatile("movl %%esp,%0":"=m"(originalBefore)); originalResult=world_abi_probe(original,scenario.selector,1,1,&originalAbi); __asm__ volatile("movl %%esp,%0":"=m"(originalAfter));
            __asm__ volatile("fnstcw %0":"=m"(originalCwAfter)); __asm__ volatile("stmxcsr %0":"=m"(originalMxAfter));
            originalOracle=oracle(recorder,originalResult); events=recorder.events; finalState=recorder.snapshot(); emit(originals,id,recorder); current=nullptr;
        }
        reset(state,scenario); Recorder recorder{state,scenario}; current=&recorder; const auto count=replacementCount();
        __asm__ volatile("fnstcw %0":"=m"(candidateCwBefore)); __asm__ volatile("stmxcsr %0":"=m"(candidateMxBefore));
        __asm__ volatile("movl %%esp,%0":"=m"(candidateBefore)); candidateResult=world_abi_probe(candidate,scenario.selector,1,1,&candidateAbi); __asm__ volatile("movl %%esp,%0":"=m"(candidateAfter));
        __asm__ volatile("fnstcw %0":"=m"(candidateCwAfter)); __asm__ volatile("stmxcsr %0":"=m"(candidateMxAfter));
        candidateOracle=oracle(recorder,candidateResult); emit(candidates,id,recorder);
        const bool trace=events==recorder.events,global=finalState==recorder.snapshot(),route=replacementCount()==count+1;
        const bool stack=originalBefore==originalAfter && candidateBefore==candidateAfter && worldAbiValid(originalAbi) && worldAbiValid(candidateAbi);
        const bool fp=originalCwBefore==originalCwAfter && candidateCwBefore==candidateCwAfter && (originalMxBefore&0xffc0)==(originalMxAfter&0xffc0) && (candidateMxBefore&0xffc0)==(candidateMxAfter&0xffc0);
        const bool equal=trace && global && route && stack && fp && originalOracle && candidateOracle && originalResult==candidateResult;
        if(!equal) ++failures;
        std::fprintf(summary,"scenario=%u selector=%d sound=%d mode=%d decision=%d signedInput=%d rand=%d script=%d original=%d candidate=%d trace=%d globals=%d route=%d stack=%d oracle=%d/%d result=%s\n",id,scenario.selector,scenario.sound,scenario.mode,scenario.decision,scenario.signedInput,scenario.random,scenario.script,originalResult,candidateResult,trace,global,route,stack,originalOracle,candidateOracle,equal?"PASS":"FAIL");
        std::fprintf(summary," abi original_esp=%08lx/%08lx candidate_esp=%08lx/%08lx original_x87=%04x/%04x candidate_x87=%04x/%04x original_mxcsr=%08x/%08x candidate_mxcsr=%08x/%08x fp=%d\n",static_cast<unsigned long>(originalBefore),static_cast<unsigned long>(originalAfter),static_cast<unsigned long>(candidateBefore),static_cast<unsigned long>(candidateAfter),originalCwBefore,originalCwAfter,candidateCwBefore,candidateCwAfter,originalMxBefore,originalMxAfter,candidateMxBefore,candidateMxAfter,fp);
        std::fprintf(summary," nonvolatile original=%08x/%08x/%08x/%08x candidate=%08x/%08x/%08x/%08x\n",originalAbi.ebp,originalAbi.ebx,originalAbi.esi,originalAbi.edi,candidateAbi.ebp,candidateAbi.ebx,candidateAbi.esi,candidateAbi.edi);
        if(!equal && !events.empty() && !recorder.events.empty()) {
            const auto common=(std::min)(events.size(),recorder.events.size());
            for(std::size_t i=0;i<common;i++) if(events[i]!=recorder.events[i]) { std::fprintf(summary," first_difference sequence=%u original=%s candidate=%s\n",static_cast<unsigned>(i),events[i].c_str(),recorder.events[i].c_str()); break; }
        }
        if(id%100==0) std::fflush(summary); current=nullptr;
    }
    std::fprintf(summary,"TOTAL scenarios=%u failures=%u\n",id,failures);
    std::fclose(summary); std::fclose(originals); std::fclose(candidates); return failures;
}
}
