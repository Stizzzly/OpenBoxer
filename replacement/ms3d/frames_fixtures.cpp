#include "frames_layout.hpp"
#include "frames_observer.hpp"
#include "clip_trace.hpp"
#include "animation_fp.hpp"
#include "world_runtime.hpp"
#include <windows.h>
namespace frames {
namespace {
struct Fixture {
    std::array<uint8_t,512> storage{};
    std::array<uint8_t,272> record{};
    uint32_t clock=0,opaque=0xabc12345,returned=0xfedcba98;
    unsigned mutation=0,lookups=0; bool composed=false;
    character::Trace events;
    std::array<uint8_t,112> boundary{};
    uint32_t advanceECX=0;
    uint8_t *model() { return storage.data()+256; }
};
Fixture *active=nullptr; uintptr_t originalModule=0; bool originalRun=false;
void normalized(std::array<uint8_t,112> &m) { clip::normalize(m.data()); }
void syncClock() { if(originalModule)put(reinterpret_cast<void*>(originalModule+0x17d904),active->clock); }
void *__attribute__((thiscall)) lookup(void *,int32_t index) {
    auto &f=*active; ++f.lookups;
    const uint32_t arg=uint32_t(index); f.events.append("lookup",&arg,1,"model+84",0,"selected-record");
    if(f.lookups==1) {
        if(f.mutation==1) { put(f.model()+40,0); syncClock(); return nullptr; }
        if(f.mutation==2)put(f.model()+40,1);
        if(f.mutation==3)put(f.model()+48,9);
        if(f.mutation==4) { put(f.record.data()+256,4); put(f.record.data()+260,7); }
        if(f.mutation==5)put(f.model()+44,91);
        if(f.mutation==6) { syncClock(); return f.model()-208; }
    }
    syncClock(); return f.record.data();
}
int32_t __attribute__((thiscall)) count(void *) { active->events.append("count",nullptr,0,"model+84",1); return 1; }
extern "C" uint32_t __attribute__((stdcall)) frames_fixture_entry(void *);
extern "C" uint32_t __attribute__((stdcall)) frames_fixture_advance(void *);
struct Witness { uint32_t beforeEsp,afterEsp,ebx,esi,edi,ebp,eax,cw,sw,mx,afterCW,afterSW,afterMX; };
extern "C" void frames_invoke(Entry,void *,uint32_t,Witness *);
bool abi(const Witness &w) { return w.beforeEsp==w.afterEsp && w.ebx==0x11223344 && w.esi==0x22334455 && w.edi==0x33445566 && w.ebp==0x44556677; }
void initialFp() { uint16_t cw=0x027f; uint32_t mx=0x1f80; __asm__ volatile("fninit; fldcw %0; ldmxcsr %1"::"m"(cw),"m"(mx)); }
Fixture make(unsigned id) {
    Fixture f; for(unsigned i=0;i<28;++i)put(f.model()+4*i,0x12340000+i);
    put(f.model()+40,1); put(f.model()+44,0); put(f.model()+48,3); put(f.model()+52,7); put(f.model()+80,0); put(f.record.data()+256,2); put(f.record.data()+260,10); put(f.record.data()+268,10);
    if(id==0)put(f.model()+40,0);
    if(id==1)put(f.model()+40,0xfffffffe);
    if(id>=2 && id<=6)put(f.model()+48,id+3);
    if(id==7)put(f.record.data()+260,1);
    if(id==8)put(f.record.data()+256,0);
    if(id==9)put(f.record.data()+256,uint32_t(-3));
    if(id==10)put(f.model()+48,uint32_t(-8));
    if(id==11)put(f.record.data()+260,uint32_t(-7));
    if(id==12) { put(f.model()+48,uint32_t(-8)); put(f.record.data()+260,uint32_t(-7)); }
    if(id==13)put(f.model()+48,0x7fffffff);
    if(id==14) { put(f.model()+48,0x7fffffff); put(f.record.data()+260,uint32_t(-7)); }
    if(id>=15 && id<=20)f.mutation=id-14;
    if(id==16)put(f.model()+40,0);
    if(id==21)f.returned=0;
    if(id==22)f.returned=0xcccccccc;
    if(id==23)f.mutation=7;
    if(id==24)f.mutation=8;
    if(id==25)f.mutation=9;
    if(id>=26 && id<=33) { f.composed=true; const uint32_t times[]={0,50,100,200,300,400,99,450}; f.clock=animation::fp::bits(float(times[id-26])); put(f.model()+76,0x3e800000); }
    return f;
}
unsigned run(FILE *report,unsigned id,Fixture input,uintptr_t base,const std::array<uint8_t,112> *captured=nullptr,uint32_t capturedEax=0,uint32_t capturedClock=0,const std::array<uint8_t,112> *capturedBoundary=nullptr) {
    character::FpPreserver preserve; Fixture live=input,originalOutput; active=&live;
    Witness old{},fresh{}; clip::Fp oldInitial{},oldFinal{},newInitial{},newFinal{};
    const uint32_t savedClock=base?word(reinterpret_cast<void*>(base+0x17d904)):0;
    if(base) { originalRun=true; put(reinterpret_cast<void*>(base+0x17d904),live.clock); initialFp(); oldInitial=clip::environment(); frames_invoke(reinterpret_cast<Entry>(base+0x5d40),live.model(),live.opaque,&old); oldFinal=clip::environment(); originalOutput=live; }
    originalRun=false; live=input; active=&live; initialFp(); newInitial=clip::environment(); frames_invoke(frames_fixture_entry,live.model(),live.opaque,&fresh); newFinal=clip::environment();
    if(base)put(reinterpret_cast<void*>(base+0x17d904),savedClock);
    const bool stateEqual=!base || (live.storage==originalOutput.storage && live.record==originalOutput.record && live.clock==originalOutput.clock && live.boundary==originalOutput.boundary);
    const bool callbackEqual=!base || live.events.events==originalOutput.events.events;
    const bool returnEqual=!base || fresh.eax==old.eax;
    const bool ecxEqual=live.advanceECX==input.opaque && (!base || originalOutput.advanceECX==input.opaque);
    const bool fpEqual=!base || (oldInitial==newInitial && oldFinal==newFinal);
    const bool abiOk=abi(fresh) && (!base || abi(old));
    std::array<uint8_t,112> output{},initial{},oldState{}; std::memcpy(output.data(),live.model(),112); std::memcpy(initial.data(),input.model(),112); std::memcpy(oldState.data(),originalOutput.model(),112); normalized(output); normalized(initial); normalized(oldState);
    const bool captureEqual=!captured || (output==*captured && fresh.eax==capturedEax && live.clock==capturedClock && (!capturedBoundary || live.boundary==*capturedBoundary));
    bool expected=true; if(!base && !captured && id==0)for(unsigned o:{52u,60u,64u,68u,72u})expected=expected && word(output.data()+o)==0;
    if(!base && !captured && !input.composed)expected=expected && fresh.eax==input.returned;
    const bool equal=stateEqual && callbackEqual && returnEqual && ecxEqual && fpEqual && abiOk && captureEqual && expected;
    std::fprintf(report,"{\"case\":%u,\"differential\":%s,\"composed\":%s,\"mutation\":%u,\"equal\":%s,\"state_equal\":%s,\"callback_equal\":%s,\"return_equal\":%s,\"ecx_equal\":%s,\"fp_equal\":%s,\"abi_ok\":%s,\"captured_state_reproduced\":%s,\"opaque_entry_ecx\":%u,\"candidate_advance_ecx\":%u,\"original_advance_ecx\":%u,\"initial_clock_bits\":%u,\"candidate_clock_bits\":%u,\"original_clock_bits\":%u,\"candidate_eax\":%u,\"original_eax\":%u,\"initial_model_words\":",id,base?"true":"false",input.composed?"true":"false",input.mutation,equal?"true":"false",stateEqual?"true":"false",callbackEqual?"true":"false",returnEqual?"true":"false",ecxEqual?"true":"false",fpEqual?"true":"false",abiOk?"true":"false",captureEqual?"true":"false",input.opaque,live.advanceECX,originalOutput.advanceECX,input.clock,live.clock,originalOutput.clock,fresh.eax,old.eax); clip::words(report,initial.data()); std::fputs(",\"candidate_prepared_model_words\":",report); clip::words(report,live.boundary.data()); std::fputs(",\"original_prepared_model_words\":",report); clip::words(report,originalOutput.boundary.data()); std::fputs(",\"candidate_model_words\":",report); clip::words(report,output.data()); std::fputs(",\"original_model_words\":",report); clip::words(report,oldState.data()); std::fputs(",\"initial_record_hex\":",report); clip::hex(report,input.record.data(),272); std::fputs(",\"candidate_events\":",report); live.events.write(report); std::fputs(",\"original_events\":",report); originalOutput.events.write(report); std::fputs(",\"candidate_initial_fp\":",report); clip::fpJson(report,newInitial); std::fputs(",\"candidate_final_fp\":",report); clip::fpJson(report,newFinal); std::fputs(",\"original_initial_fp\":",report); clip::fpJson(report,oldInitial); std::fputs(",\"original_final_fp\":",report); clip::fpJson(report,oldFinal); std::fputs("}\n",report); return !equal;
}
struct Isolation {
    uintptr_t base; uint32_t clock;
    explicit Isolation(uintptr_t b):base(b),clock(word(reinterpret_cast<void*>(b+0x17d904))) { originalModule=b; world::redirect(b+0x10b4,reinterpret_cast<uintptr_t>(&lookup)); world::redirect(b+0x14d8,reinterpret_cast<uintptr_t>(&count)); advancementCall(b,reinterpret_cast<uintptr_t>(&frames_fixture_advance)); }
    ~Isolation() { advancementCall(base,base+0x15aa); world::redirect(base+0x10b4,base+0x97a0); world::redirect(base+0x14d8,base+0x9730); put(reinterpret_cast<void*>(base+0x17d904),clock); originalModule=0; }
};
}
extern "C" uint32_t frames_fixture_dispatch(void *model,uint32_t opaque) { BinaryState state(model,{lookup,frames_fixture_advance}); return prepare(state,opaque); }
extern "C" uint32_t frames_fixture_advance_dispatch(void *model,uint32_t opaque) {
    auto &f=*active; f.advanceECX=opaque; std::memcpy(f.boundary.data(),model,112); normalized(f.boundary);
    uint32_t args[29]; args[0]=opaque; for(unsigned i=0;i<28;++i)args[i+1]=word(f.boundary.data()+4*i); f.events.append("advancement-enter",args,29,"model");
    uint32_t result=f.returned;
    if(f.composed) { if(originalRun)result=frames_call(reinterpret_cast<Entry>(originalModule+0x6810),model,opaque); else { animation::BinaryState state(model,{count,lookup,&f.clock}); result=animation::advance(state); } }
    else {
        if(f.mutation==7) { put(static_cast<uint8_t*>(model)+64,0xaabbccdd); put(static_cast<uint8_t*>(model)+48,444); f.clock=0x7fc12345; }
        if(f.mutation==8) { const uint16_t cw=0x037f; const uint32_t mx=0x3f80; __asm__ volatile("fldcw %0; ldmxcsr %1"::"m"(cw),"m"(mx)); }
        if(f.mutation==9)__asm__ volatile("fld1");
    }
    syncClock(); f.events.append("advancement-return",nullptr,0,"model",result); return result;
}
uint32_t offlineFixtures(const char *path) { FILE *f=std::fopen(path,"wb"); if(!f)return 100; unsigned failures=0; for(unsigned i=0;i<34;++i)failures+=run(f,i,make(i),0); std::fclose(f); return failures; }
uint32_t fixtures(uintptr_t base) { character::FpPreserver fp; FILE *f=std::fopen("C:/Users/ADMIN/Boxer-lab/ms3d/frames-fixtures.jsonl","wb"); if(!f)return 100; Isolation isolation(base); unsigned failures=0; for(unsigned i=0;i<34;++i)failures+=run(f,i,make(i),base); std::fclose(f); return failures; }
uint32_t replays(uintptr_t base) {
    character::FpPreserver fp; char list[4096]{}; GetEnvironmentVariableA("OPENBOXER_FRAMES_REPLAY_LIST",list,sizeof(list)); if(!list[0])return 101; FILE *report=std::fopen("C:/Users/ADMIN/Boxer-lab/ms3d/frames-replays.jsonl","wb"); if(!report)return 100;
    Isolation isolation(base); unsigned failures=0,id=0;
    for(char *path=std::strtok(list,";");path && id<16;path=std::strtok(nullptr,";"),++id) {
        uint32_t h[12]{}; Fixture input; std::array<uint8_t,112> boundary{},output{}; FILE *f=std::fopen(path,"rb");
        bool ok=f && std::fread(h,4,12,f)==12 && h[0]==0x334d5246 && h[1]==1 && std::fread(input.model(),1,112,f)==112 && std::fread(input.record.data(),1,272,f)==272 && std::fread(boundary.data(),1,112,f)==112 && std::fread(output.data(),1,112,f)==112;
        if(f)std::fclose(f); if(!ok || int32_t(word(input.record.data()+260))<=0 || int32_t(word(input.record.data()+268))<=0 || !animation::fp::finite(h[2]) || !animation::fp::finite(word(input.model()+80))) { ++failures; continue; }
        input.clock=h[2]; input.opaque=h[5]; input.composed=true; failures+=run(report,id,input,base,&output,h[4],h[3],&boundary);
    }
    std::fclose(report); return failures;
}
}
