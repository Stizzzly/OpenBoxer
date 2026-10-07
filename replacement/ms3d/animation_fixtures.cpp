#include "animation_layout.hpp"
#include "animation_fp.hpp"
#include "character_trace.hpp"
#include "world_runtime.hpp"
#include <array>
#include <vector>
#include <windows.h>
namespace animation {
namespace {
struct Fixture {
    std::array<uint8_t,112> model{};
    std::array<uint8_t,272> record{};
    uint32_t clock=0;
    int32_t count=1;
    unsigned mutation=0;
    uint32_t captureCW=0,captureSW=0;
    bool hasCapturedOutput=false;
    std::array<uint8_t,112> capturedOutput{};
    uint32_t capturedEax=0,capturedClock=0;
    character::Trace events;
};
Fixture *active;
void countMutation(Fixture &f) {
    if(f.mutation==1) { f.clock=fp::bits(211.0f); put(f.model.data()+80,fp::bits(11.0f)); put(f.model.data()+44,2); }
    if(f.mutation==2) { f.clock=fp::bits(900.0f); put(f.model.data()+48,0xaaaa5555); }
}
int32_t __attribute__((thiscall)) countRecorder(void *) {
    countMutation(*active);
    active->events.append("count",nullptr,0,"model.animationCollection",uint32_t(active->count));
    return active->count;
}
void *__attribute__((thiscall)) lookupRecorder(void *,int32_t index) {
    if(active->mutation==3) {
        put(active->model.data()+80,fp::bits(300.0f));
        active->clock=fp::bits(2000.0f);
        put(active->model.data()+60,555); put(active->record.data()+268,30);
    }
    const uint32_t arg=uint32_t(index);
    active->events.append("lookup",&arg,1,"model.animationCollection",0,"selectedRecord");
    return active->record.data();
}
// Original reads the module clock; recorders update that data only when an
// explicitly isolated original CPU replay is active.
uintptr_t originalModule=0;
int32_t __attribute__((thiscall)) originalCount(void *p) {
    const int32_t r=countRecorder(p); put(reinterpret_cast<void*>(originalModule+0x17d904),active->clock); return r;
}
void *__attribute__((thiscall)) originalLookup(void *p,int32_t i) {
    void *r=lookupRecorder(p,i); put(reinterpret_cast<void*>(originalModule+0x17d904),active->clock); return r;
}
struct Witness { uint32_t beforeEsp,afterEsp,ebx,esi,edi,ebp,eax,cw,sw,mx,afterCW,afterSW,afterMX; };
struct FpWitness {
    uint16_t cw=0,sw=0;
    uint8_t tag=0;
    uint32_t mx=0;
};
FpWitness environment() {
    alignas(16) uint8_t data[512]; __asm__ volatile("fxsave %0":"=m"(data));
    FpWitness e; std::memcpy(&e.cw,data,2); std::memcpy(&e.sw,data+2,2); e.tag=data[4]; e.mx=word(data+24); return e;
}
void environmentJson(FILE *f,const FpWitness &e) {
    std::fprintf(f,"{\"cw\":%u,\"sw\":%u,\"top\":%u,\"abridged_tag\":%u,\"mxcsr\":%u}",e.cw,e.sw,(e.sw>>11)&7,unsigned(e.tag),e.mx);
}
extern "C" void animation_invoke(Entry,void *,Witness *);
uint32_t __attribute__((stdcall)) candidate(void *m) {
    BinaryState state(m,{countRecorder,lookupRecorder,&active->clock}); return advance(state);
}
void initialFp() {
    const uint16_t cw=0x027f; const uint32_t mx=0x1f80;
    __asm__ volatile("fninit; fldcw %0; ldmxcsr %1"::"m"(cw),"m"(mx));
}
bool abi(const Witness &w) { return w.beforeEsp==w.afterEsp && w.ebx==0x11223344 && w.esi==0x22334455 && w.edi==0x33445566 && w.ebp==0x44556677 && w.cw==w.afterCW; }
void words(FILE *f,const std::array<uint8_t,112> &m) { std::fputc('[',f); for(unsigned i=0;i<28;++i) std::fprintf(f,"%s%u",i?",":"",word(m.data()+4*i)); std::fputc(']',f); }
Fixture make(unsigned id) {
    Fixture f;
    for(unsigned i=0;i<28;++i) put(f.model.data()+4*i,0x12340000+i);
    put(f.model.data()+44,0); put(f.model.data()+48,17); put(f.model.data()+76,fp::bits(.375f)); put(f.model.data()+80,fp::bits(0.0f));
    int32_t rate=10;
    float elapsed=0;
    if(id==0) { f.count=0; f.mutation=2; elapsed=77; }
    else if(id==1) { f.count=-3; elapsed=25; }
    else if(id==2) elapsed=-25;
    else if(id==3) elapsed=50;
    else if(id>=4 && id<=14) elapsed=float(id-3)*100;
    else if(id==15) elapsed=2500;
    else if(id==16) elapsed=100.00001f;
    else if(id==17) elapsed=99.99999f;
    else if(id>=18 && id<=29) { rate=id<24?30:60; elapsed=float((id-18)%6)*1000.0f/float(rate); }
    else if(id==30) { elapsed=150; f.mutation=1; }
    else if(id==31) { elapsed=150; f.mutation=3; }
    else if(id==32) { elapsed=-0.0f; }
    else if(id==33) { elapsed=0; put(f.model.data()+80,fp::bits(-0.0f)); }
    else if(id==34) { elapsed=200; put(f.model.data()+80,fp::bits(100.0f)); }
    else if(id==35) { rate=2147483647; elapsed=.000005f; }
    else if(id==36) { rate=1; elapsed=11500; }
    else if(id==37) { f.count=0; elapsed=0; }
    else if(id==38) { elapsed=100.0f; put(f.model.data()+76,0x80000000); }
    else { rate=30; elapsed=366.66666f; }
    f.clock=fp::bits(elapsed); put(f.record.data()+268,uint32_t(rate));
    return f;
}
unsigned run(FILE *report,unsigned id,Fixture input,uintptr_t base) {
    Fixture a=input,b=input; Witness old{},fresh{};
    FpWitness oldInitial,oldFinal,newInitial,newFinal;
    character::FpPreserver callerFp;
    uint32_t savedClock=0;
    if(base) {
        savedClock=word(reinterpret_cast<void*>(base+0x17d904));
        active=&a; put(reinterpret_cast<void*>(base+0x17d904),a.clock);
        initialFp(); oldInitial=environment(); animation_invoke(reinterpret_cast<Entry>(base+0x6810),a.model.data(),&old); oldFinal=environment();
    }
    active=&b; initialFp(); newInitial=environment(); animation_invoke(candidate,b.model.data(),&fresh); newFinal=environment();
    if(base) put(reinterpret_cast<void*>(base+0x17d904),savedClock);
    bool equal=!base || (a.model==b.model && a.clock==b.clock && a.record==b.record && old.eax==fresh.eax && a.events.events==b.events.events);
    const bool fpEqual=!base || (oldInitial.cw==newInitial.cw && oldInitial.sw==newInitial.sw &&
        oldInitial.tag==newInitial.tag && oldInitial.mx==newInitial.mx && oldFinal.cw==newFinal.cw &&
        oldFinal.sw==newFinal.sw && oldFinal.tag==newFinal.tag && oldFinal.mx==newFinal.mx);
    const bool stateEqual=equal;
    equal=equal && fpEqual;
    const bool abiOk=abi(fresh) && (!base || abi(old));
    const bool observedEqual=!input.hasCapturedOutput || (b.model==input.capturedOutput && fresh.eax==input.capturedEax && b.clock==input.capturedClock);
    // The approved comparison supplement requires exact SW in equal initial
    // environments. Keep the diagnostic fields alongside the primary result.
    std::fprintf(report,"{\"case\":%u,\"differential\":%s,\"equal\":%s,\"abi_ok\":%s,\"original_eax\":%u,\"candidate_eax\":%u,\"original_sw\":%u,\"candidate_sw\":%u,\"status_equal\":%s,\"initial_clock_bits\":%u,\"initial_rate\":%d,\"count\":%d,\"mutation\":%u,\"initial_model_words\":",id,base?"true":"false",equal?"true":"false",abiOk?"true":"false",old.eax,fresh.eax,old.afterSW,fresh.afterSW,old.afterSW==fresh.afterSW?"true":"false",input.clock,int32_t(word(input.record.data()+268)),input.count,input.mutation);
    words(report,input.model); std::fprintf(report,",\"state_equal\":%s,\"fp_equal\":%s,\"candidate_model_words\":",stateEqual?"true":"false",fpEqual?"true":"false"); words(report,b.model);
    std::fputs(",\"original_model_words\":",report); words(report,a.model);
    std::fprintf(report,",\"candidate_clock_bits\":%u,\"original_clock_bits\":%u,\"capture_source_cw\":%u,\"capture_source_sw\":%u,\"captured_state_reproduced\":%s,\"has_captured_output\":%s,\"replay_environment_policy\":\"clean-identical-fninit-CW027F-MXCSR1F80\",\"original_initial_fp\":",b.clock,a.clock,input.captureCW,input.captureSW,observedEqual?"true":"false",input.hasCapturedOutput?"true":"false");
    environmentJson(report,oldInitial); std::fputs(",\"original_final_fp\":",report); environmentJson(report,oldFinal);
    std::fputs(",\"candidate_initial_fp\":",report); environmentJson(report,newInitial); std::fputs(",\"candidate_final_fp\":",report); environmentJson(report,newFinal);
    std::fputs(",\"candidate_events\":",report); b.events.write(report); std::fputs(",\"original_events\":",report); a.events.write(report); std::fputs("}\n",report);
    return !equal || !abiOk || !observedEqual || !fpEqual;
}
struct DependencyIsolation {
    uintptr_t base; uint32_t savedClock;
    explicit DependencyIsolation(uintptr_t b):base(b),savedClock(word(reinterpret_cast<void*>(b+0x17d904))) {
        originalModule=b;
        world::redirect(b+0x14d8,reinterpret_cast<uintptr_t>(&originalCount));
        world::redirect(b+0x10b4,reinterpret_cast<uintptr_t>(&originalLookup));
    }
    ~DependencyIsolation() {
        world::redirect(base+0x14d8,base+0x9730); world::redirect(base+0x10b4,base+0x97a0);
        put(reinterpret_cast<void*>(base+0x17d904),savedClock); originalModule=0;
    }
};
}
uint32_t offlineFixtures(const char *path) {
    FILE *f=std::fopen(path,"wb"); if(!f) return 100;
    unsigned failures=0;
    for(unsigned i=0;i<40;++i) failures+=run(f,i,make(i),0);
    std::fclose(f); return failures;
}
uint32_t fixtures(uintptr_t base) {
    character::FpPreserver fp;
    FILE *f=std::fopen("C:/Users/ADMIN/Boxer-lab/ms3d/animation-fixtures.jsonl","wb"); if(!f) return 100;
    DependencyIsolation isolate(base);
    unsigned failures=0; for(unsigned i=0;i<40;++i) failures+=run(f,i,make(i),base);
    std::fclose(f); return failures;
}
uint32_t replays(uintptr_t base) {
    character::FpPreserver fp;
    char list[4096]{}; GetEnvironmentVariableA("OPENBOXER_ANIMATION_REPLAY_LIST",list,sizeof(list));
    if(!list[0]) return 101;
    FILE *report=std::fopen("C:/Users/ADMIN/Boxer-lab/ms3d/animation-replays.jsonl","wb"); if(!report) return 100;
    DependencyIsolation isolate(base);
    unsigned failures=0,id=0;
    for(char *path=std::strtok(list,";");path && id<16;path=std::strtok(nullptr,";"),++id) {
        FILE *f=std::fopen(path,"rb"); Fixture input; uint32_t h[10]{};
        bool ok=f && std::fread(h,4,10,f)==10 && h[0]==0x314d4e41 && h[1]==1 && h[5]==0x027f &&
            std::fread(input.model.data(),1,112,f)==112 && std::fread(input.record.data(),1,272,f)==272 &&
            std::fread(input.capturedOutput.data(),1,112,f)==112;
        if(f) std::fclose(f);
        if(!ok || !h[3] || h[3]>0x7fffffffu || h[4]>=h[3] || int32_t(word(input.record.data()+268))<=0 ||
           !fp::finite(h[2]) || !fp::finite(word(input.model.data()+80)) ||
           !fp::finite(fp::elapsed(h[2],word(input.model.data()+80)))) { ++failures; continue; }
        input.clock=h[2]; input.count=int32_t(h[3]); input.captureCW=h[5]; input.captureSW=h[6];
        input.hasCapturedOutput=true; input.capturedEax=h[7]; input.capturedClock=h[9]; failures+=run(report,id,input,base);
    }
    std::fclose(report); return failures;
}
}
