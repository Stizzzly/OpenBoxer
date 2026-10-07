#include "clip_trace.hpp"
#include "world_runtime.hpp"
#include "clip_observer.hpp"
#include "action_roles.hpp"
#include <windows.h>
namespace clip {
namespace {
struct Fixture {
    std::array<uint8_t,756> owner{};
    std::vector<uint8_t> records=std::vector<uint8_t>(816),alternate=std::vector<uint8_t>(816);
    std::array<char,256> request{};
    uint32_t clock=0x447a0000; unsigned mutation=0,lookups=0;
    Events events;
    uint8_t *model() { return owner.data()+644; }
};
Fixture *active=nullptr; uintptr_t originalModule=0; Compare originalCompare=nullptr;
void bind(Fixture &f) { put(f.model()+88,uint32_t(reinterpret_cast<uintptr_t>(f.records.data()))); put(f.model()+92,uint32_t(reinterpret_cast<uintptr_t>(f.records.data()+f.records.size()))); put(f.model()+96,uint32_t(reinterpret_cast<uintptr_t>(f.records.data()+f.records.size()))); }
int32_t ascii(const char *a,const char *b) { while(true) { unsigned x=uint8_t(*a++),y=uint8_t(*b++); if(x>='A' && x<='Z')x+=32; if(y>='A' && y<='Z')y+=32; if(x!=y || !x || !y)return int32_t(x)-int32_t(y); } }
void syncClock() { if(originalModule) put(reinterpret_cast<void*>(originalModule+0x17d904),active->clock); }
void *__attribute__((thiscall)) lookup(void *collection,int32_t index) {
    Fixture &f=*active; ++f.lookups;
    if(f.mutation==1 && f.lookups==1) std::strcpy(f.request.data(),"beta");
    if(f.mutation==2 && f.lookups==1) put(f.model()+40,1);
    if(f.mutation==3 && f.lookups==1) put(f.model()+40,3);
    if(f.mutation==4 && f.lookups==2) { put(f.model()+44,91); f.clock=0x80000000; }
    if(f.mutation==5 && f.lookups==2) { put(f.records.data()+uint32_t(index)*272+256,555); f.clock=0x7fc12345; }
    uint8_t *begin=reinterpret_cast<uint8_t*>(uintptr_t(word(static_cast<uint8_t*>(collection)+4)));
    if(f.mutation==6 && f.lookups==2) begin=f.alternate.data();
    void *record=begin+uint32_t(index)*272;
    const uint32_t role=begin==f.alternate.data()?uint32_t(index)+3:uint32_t(index);
    f.events.lookup(index,role); syncClock(); return record;
}
int32_t __attribute__((cdecl)) compare(const char *a,const char *b) {
    Fixture &f=*active;
    int32_t result=originalCompare?originalCompare(a,b):ascii(a,b);
    if(f.mutation==7) result=0x80001234;
    f.events.compare(a,b,result,uint32_t((reinterpret_cast<uintptr_t>(a)-reinterpret_cast<uintptr_t>(f.records.data()))/272));
    if(f.mutation==8) put(f.model()+40,1);
    if(f.mutation==9) put(f.model()+40,3);
    if(f.mutation==10 && result==0) { put(f.model()+44,77); put(f.model()+88,uint32_t(reinterpret_cast<uintptr_t>(f.alternate.data()))); f.clock=0xdeadbeef; }
    if(f.mutation==11 && result==0) { put(f.records.data()+uint32_t(f.lookups-1)*272+256,666); std::strcpy(f.request.data(),"changed"); f.clock=0; }
    syncClock(); return result;
}
uint32_t __attribute__((thiscall)) candidate(void *owner,const char *request) { BinaryState state(owner,request,{lookup,compare,&active->clock}); return select(state); }
struct Witness { uint32_t beforeEsp,afterEsp,ebx,esi,edi,ebp,eax,cw,sw,mx,afterCW,afterSW,afterMX; };
extern "C" void clip_invoke(Entry,void *,const char *,Witness *);
bool abi(const Witness &w) { return w.beforeEsp==w.afterEsp && w.ebx==0x11223344 && w.esi==0x22334455 && w.edi==0x33445566 && w.ebp==0x44556677; }
void initialFp() { uint16_t cw=0x027f; uint32_t mx=0x1f80; __asm__ volatile("fninit; fldcw %0; ldmxcsr %1"::"m"(cw),"m"(mx)); }
Fixture make(unsigned id) {
    Fixture f;
    for(unsigned i=0;i<189;++i) put(f.owner.data()+4*i,0x12340000+i);
    put(f.model()+40,3); put(f.model()+44,77);
    const char *names[]={"alpha","beta","gamma"};
    for(unsigned i=0;i<3;++i) { std::strcpy(reinterpret_cast<char*>(f.records.data()+272*i),names[i]); put(f.records.data()+272*i+256,100+i); }
    f.alternate=f.records; for(unsigned i=0;i<3;++i)put(f.alternate.data()+272*i+256,700+i);
    std::strcpy(f.request.data(),"beta");
    if(id==0)put(f.model()+40,0);
    if(id==1)put(f.model()+40,0xffffffff);
    if(id==2)std::strcpy(f.request.data(),"missing");
    if(id==3)std::strcpy(f.request.data(),"alpha");
    if(id==4)std::strcpy(f.request.data(),"gamma");
    if(id==5)std::strcpy(f.request.data(),"BeTa");
    if(id==6) { std::strcpy(reinterpret_cast<char*>(f.records.data()),"BETA"); }
    if(id==7) { f.request[0]=0; f.records[0]=0; }
    if(id>=8 && id<=18) f.mutation=id-7;
    if(id==10 || id==16) { put(f.model()+40,1); std::strcpy(f.request.data(),"gamma"); }
    if(id==12)std::strcpy(f.request.data(),"alpha");
    if(id==18)std::strcpy(f.request.data(),"alpha");
    if(id>=19 && id<=22) { const uint32_t bits[]={0,0x80000000,0x7fc12345,0xff800000}; f.clock=bits[id-19]; }
    if(id==23) { std::strcpy(f.request.data(),"beta"); put(f.model()+44,1); put(f.model()+48,101); put(f.model()+76,0x3dcccccd); put(f.model()+80,f.clock); }
    return f;
}
Fixture makeAction(unsigned index) {
    Fixture f=make(3); f.records.assign(8*272,0); f.alternate=f.records;
    const uint32_t starts[]={99,0,23,10,33,46,65,224};
    const uint32_t ends[]={174,10,33,23,46,65,84,249};
    for(unsigned i=0;i<8;++i) {
        std::strcpy(reinterpret_cast<char*>(f.records.data()+272*i),action::roles[i].name);
        put(f.records.data()+272*i+256,starts[i]); put(f.records.data()+272*i+260,ends[i]); put(f.records.data()+272*i+268,i?40:30);
    }
    f.alternate=f.records; put(f.model()+40,8); std::strcpy(f.request.data(),action::roles[index].name); return f;
}
uint32_t normalizedEax(uint32_t eax,Fixture &f) { return eax==uint32_t(reinterpret_cast<uintptr_t>(f.owner.data()))?0xffffffff:eax; }
unsigned run(FILE *report,unsigned id,Fixture input,uintptr_t base,const std::array<uint8_t,112> *captured=nullptr,uint32_t capturedEax=0,uint32_t capturedClock=0) {
    character::FpPreserver preserve;
    Fixture live=input; bind(live); active=&live;
    const auto before=live.owner; Witness old{},fresh{}; Fp oldInitial{},oldFinal{},newInitial{},newFinal{};
    Fixture originalOutput;
    const uint32_t savedClock=base?word(reinterpret_cast<void*>(base+0x17d904)):0;
    if(base) { put(reinterpret_cast<void*>(base+0x17d904),live.clock); initialFp(); oldInitial=environment(); clip_invoke(reinterpret_cast<Entry>(base+0x6e40),live.owner.data(),live.request.data(),&old); oldFinal=environment(); originalOutput=live; }
    live=input; bind(live); active=&live; initialFp(); newInitial=environment(); clip_invoke(candidate,live.owner.data(),live.request.data(),&fresh); newFinal=environment();
    if(base) put(reinterpret_cast<void*>(base+0x17d904),savedClock);
    const bool stateEqual=!base || (originalOutput.owner==live.owner && originalOutput.clock==live.clock && originalOutput.records==live.records && originalOutput.alternate==live.alternate && originalOutput.request==live.request);
    const bool callbacksEqual=!base || originalOutput.events.entries==live.events.entries;
    const bool returnEqual=!base || old.eax==fresh.eax;
    const bool fpEqual=newInitial==newFinal && (!base || (oldInitial==newInitial && oldFinal==newFinal));
    const bool abiOk=abi(fresh) && (!base || abi(old));
    std::array<uint8_t,112> output{},initial{},oldState{}; std::memcpy(output.data(),live.model(),112); std::memcpy(initial.data(),before.data()+644,112); std::memcpy(oldState.data(),originalOutput.model(),112); normalize(output.data()); normalize(initial.data()); normalize(oldState.data());
    const uint32_t eax=normalizedEax(fresh.eax,live);
    const bool capturedEqual=!captured || (output==*captured && eax==capturedEax && live.clock==capturedClock);
    bool expected=true;
    if(!base && !captured) { if(id==0 || id==1)expected=fresh.eax==0xcccccccc && live.owner==before; if(id==2)expected=fresh.eax==3 && live.owner==before; if(id>=3 && id<=7)expected=eax==0xffffffff && word(live.model()+76)==0x3dcccccd && word(live.model()+80)==live.clock; }
    if(!captured && id>=24 && id<32)expected=eax==0xffffffff && word(live.model()+44)==id-24 && word(live.model()+48)==word(input.records.data()+272*(id-24)+256) && word(live.model()+76)==0x3dcccccd && word(live.model()+80)==live.clock;
    const bool equal=stateEqual && callbacksEqual && returnEqual && fpEqual && abiOk && capturedEqual && expected;
    std::fprintf(report,"{\"case\":%u,\"differential\":%s,\"mutation\":%u,\"equal\":%s,\"state_equal\":%s,\"callback_equal\":%s,\"return_equal\":%s,\"fp_equal\":%s,\"abi_ok\":%s,\"captured_state_reproduced\":%s,\"initial_clock_bits\":%u,\"candidate_clock_bits\":%u,\"original_clock_bits\":%u,\"candidate_eax_role_or_bits\":%u,\"original_eax_role_or_bits\":%u,\"initial_model_words\":",id,base?"true":"false",input.mutation,equal?"true":"false",stateEqual?"true":"false",callbacksEqual?"true":"false",returnEqual?"true":"false",fpEqual?"true":"false",abiOk?"true":"false",capturedEqual?"true":"false",input.clock,live.clock,originalOutput.clock,eax,base?normalizedEax(old.eax,live):0); words(report,initial.data()); std::fputs(",\"candidate_model_words\":",report); words(report,output.data()); std::fputs(",\"original_model_words\":",report); words(report,oldState.data()); std::fputs(",\"initial_records_hex\":",report); hex(report,input.records.data(),input.records.size()); std::fputs(",\"initial_request_hex\":",report); hex(report,input.request.data(),std::strlen(input.request.data())); std::fputs(",\"candidate_events\":",report); live.events.write(report); std::fputs(",\"original_events\":",report); originalOutput.events.write(report); std::fputs(",\"candidate_initial_fp\":",report); fpJson(report,newInitial); std::fputs(",\"candidate_final_fp\":",report); fpJson(report,newFinal); std::fputs(",\"original_initial_fp\":",report); fpJson(report,oldInitial); std::fputs(",\"original_final_fp\":",report); fpJson(report,oldFinal); std::fputs("}\n",report);
    return !equal;
}
// Comparator observer CALL metadata must be approved before this isolation
// can be completed; no copied instruction or stolen-byte trampoline is used.
struct Isolation {
    uintptr_t base; uint32_t savedClock;
    explicit Isolation(uintptr_t b):base(b),savedClock(word(reinterpret_cast<void*>(b+0x17d904))) { originalModule=b; originalCompare=reinterpret_cast<Compare>(b+0x989c0); world::redirect(b+0x10b4,reinterpret_cast<uintptr_t>(&lookup)); comparatorCall(b,reinterpret_cast<uintptr_t>(&compare)); }
    ~Isolation() { comparatorCall(base,base+0x989c0); world::redirect(base+0x10b4,base+0x97a0); put(reinterpret_cast<void*>(base+0x17d904),savedClock); originalModule=0; originalCompare=nullptr; }
};
}
uint32_t offlineFixtures(const char *path) { FILE *f=std::fopen(path,"wb"); if(!f)return 100; unsigned failures=0; for(unsigned i=0;i<24;++i)failures+=run(f,i,make(i),0); std::fclose(f); return failures; }
uint32_t fixtures(uintptr_t base) { character::FpPreserver fp; FILE *f=std::fopen("C:/Users/ADMIN/Boxer-lab/ms3d/clip-fixtures.jsonl","wb"); if(!f)return 100; Isolation isolation(base); unsigned failures=0; for(unsigned i=0;i<24;++i)failures+=run(f,i,make(i),base); std::fclose(f); return failures; }
uint32_t replayList(uintptr_t base,const char *variable,const char *reportPath) {
    character::FpPreserver fp; char list[4096]{}; GetEnvironmentVariableA(variable,list,sizeof(list)); if(!list[0])return 101;
    FILE *report=std::fopen(reportPath,"wb"); if(!report)return 100;
    Isolation isolation(base); unsigned failures=0,id=0;
    for(char *path=std::strtok(list,";");path && id<16;path=std::strtok(nullptr,";"),++id) {
        CaptureHeader h; Fixture input; std::array<uint8_t,112> output{}; FILE *f=std::fopen(path,"rb");
        bool ok=f && std::fread(&h,sizeof(h),1,f)==1 && h.magic==0x32504c43 && h.version==1 && h.count>0 && h.count<=256 && h.requestSize>0 && h.requestSize<=256 && std::fread(input.model(),1,112,f)==112 && (input.records.resize(h.count*272),std::fread(input.records.data(),1,input.records.size(),f)==input.records.size()) && std::fread(input.request.data(),1,h.requestSize,f)==h.requestSize && input.request[h.requestSize-1]==0 && std::fread(output.data(),1,112,f)==112;
        if(f)std::fclose(f); if(!ok) { ++failures; continue; } input.clock=h.clock; input.alternate=input.records; failures+=run(report,id,input,base,&output,h.eax,h.afterClock);
    }
    std::fclose(report); return failures;
}
uint32_t replays(uintptr_t base) { return replayList(base,"OPENBOXER_CLIP_REPLAY_LIST","C:/Users/ADMIN/Boxer-lab/ms3d/clip-replays.jsonl"); }
uint32_t actionReplays(uintptr_t base) { return replayList(base,"OPENBOXER_ACTION_REPLAY_LIST","C:/Users/ADMIN/Boxer-lab/ms3d/action-replays.jsonl"); }
uint32_t actionCases(FILE *report,uintptr_t base) {
    unsigned failures=0;
    for(unsigned i=0;i<24;++i)failures+=run(report,i,make(i),base);
    for(unsigned i=0;i<8;++i)failures+=run(report,24+i,makeAction(i),base);
    return failures;
}
uint32_t offlineActionFixtures(const char *path) { FILE *f=std::fopen(path,"wb"); if(!f)return 100; unsigned result=actionCases(f,0); std::fclose(f); return result; }
uint32_t actionFixtures(uintptr_t base) { character::FpPreserver fp; FILE *f=std::fopen("C:/Users/ADMIN/Boxer-lab/ms3d/action-fixtures.jsonl","wb"); if(!f)return 100; Isolation isolation(base); unsigned result=actionCases(f,base); std::fclose(f); return result; }

}

