#include "clip_trace.hpp"
#include "character_native.hpp"
#include "runtime_options.hpp"
#include "world_runtime.hpp"
#include "clip_observer.hpp"
#include "action_roles.hpp"
#include "action_capture.hpp"
namespace clip {
namespace {
uintptr_t module=0; bool replace=false;
bool actionReplace=false,movementEnabled=true,actionEnabled=true;
unsigned actionCalls=0,actionReplacements=0,actionHits[8]{};
thread_local Events *events=nullptr;
thread_local uint32_t collectionBegin=0;
unsigned calls=0,replacements=0;
unsigned captureHits[3]{};
uint32_t role(void *record) { return (uint32_t(reinterpret_cast<uintptr_t>(record))-collectionBegin)/272; }
void *__attribute__((thiscall)) lookup(void *collection,int32_t index) {
    void *record=reinterpret_cast<Lookup>(module+0x97a0)(collection,index);
    observeLookup(index,record);
    return record;
}
int32_t __attribute__((cdecl)) compare(const char *name,const char *request) {
    const int32_t result=reinterpret_cast<Compare>(module+0x989c0)(name,request);
    if(events) events->compare(name,request,result,role(const_cast<char*>(name)));
    return result;
}
bool string(const char *p,unsigned &length) { for(length=0;length<256;++length) { if(!character::native::readable(p+length,1)) return false; if(!p[length]) return true; } return false; }
bool supported(void *owner,const char *request) {
    character::FpPreserver fp;
    if(reinterpret_cast<uintptr_t>(owner)!=module+0x17acd0) return false;
    auto *model=static_cast<uint8_t*>(owner)+644;
    if(!character::native::animationWitness(model)) return false;
    const int32_t bound=int32_t(word(model+40));
    const uint32_t begin=word(model+88),end=word(model+92),capacity=word(model+96);
    if(bound<=0 || bound>256 || !begin || end<begin || capacity<end || (end-begin)%272 || (capacity-begin)%272 || (end-begin)/272<uint32_t(bound) || (end-begin)/272>256 || !character::native::readable(reinterpret_cast<void*>(uintptr_t(begin)),end-begin)) return false;
    unsigned length=0; if(!string(request,length)) return false;
    for(int32_t i=0;i<bound;++i) { auto *record=reinterpret_cast<uint8_t*>(uintptr_t(begin)+272u*uint32_t(i)); if(!string(reinterpret_cast<char*>(record),length) || int32_t(word(record+256))<0 || int32_t(word(record+256))>748) return false; }
    return (word(reinterpret_cast<void*>(module+0x17d904))&0x7f800000)!=0x7f800000;
}
uint32_t __attribute__((thiscall,noinline)) entry(void *owner,const char *request) {
    if(runtime_options::capture(runtime_options::Unit::Action))action::pending={};
    const uintptr_t caller=reinterpret_cast<uintptr_t>(__builtin_return_address(0));
    const bool route=caller==module+0x371be || caller==module+0x371df || caller==module+0x37200;
    const int actionRole=action::role(module,caller,reinterpret_cast<uintptr_t>(request));
    unsigned requestLength=0;
    const bool actionRoute=actionEnabled && actionRole>=0 && string(request,requestLength) && action::admit(actionRole,module,reinterpret_cast<uintptr_t>(owner),true,request);
    if((!actionRoute && (!route || !movementEnabled)) || !supported(owner,request)) return reinterpret_cast<Entry>(module+0x6e40)(owner,request);
    const bool actionCapture=actionRoute && runtime_options::capture(runtime_options::Unit::Action) && actionHits[actionRole]<2;
    const bool actionCall=actionRoute && (actionRole!=0 || actionCapture);
    const bool selectedReplace=actionCall?actionReplace:replace;
    const unsigned sequence=actionCall?++actionCalls:++calls;
    // Two first hits per verified call role: each actual state switch and its
    // repeat remain observable even after a long standing period. Maximum six.
    const unsigned routeIndex=caller==module+0x371be?0:caller==module+0x371df?1:2;
    const bool capture=actionCapture || (route && runtime_options::capture(runtime_options::Unit::Clip) && captureHits[routeIndex]<2);
    if(actionCapture)++actionHits[actionRole]; else if(capture) ++captureHits[routeIndex];
    auto *model=static_cast<uint8_t*>(owner)+644;
    std::array<uint8_t,112> before{},after{};
    std::vector<uint8_t> records; std::string requested;
    Fp initial{},final{}; CaptureHeader header; Events trace;
    if(capture) {
        initial=environment(); character::FpPreserver fp;
        std::memcpy(before.data(),model,112); collectionBegin=word(model+88);
        header.count=(word(model+92)-collectionBegin)/272; records.resize(header.count*272);
        std::memcpy(records.data(),reinterpret_cast<void*>(uintptr_t(collectionBegin)),records.size()); requested=request;
        header.requestSize=uint32_t(requested.size()+1); header.clock=word(reinterpret_cast<void*>(module+0x17d904)); header.cw=initial.cw; header.sw=initial.sw; events=&trace;
    }
    uint32_t result;
    if(selectedReplace) { BinaryState state(owner,request,{reinterpret_cast<Lookup>(module+0x10b4),compare,reinterpret_cast<void*>(module+0x17d904)}); result=select(state); if(actionCall)++actionReplacements; else ++replacements; }
    else result=reinterpret_cast<Entry>(module+0x6e40)(owner,request);
    events=nullptr;
    if(capture) {
        final=environment(); character::FpPreserver fp;
        std::memcpy(after.data(),model,112); header.afterClock=word(reinterpret_cast<void*>(module+0x17d904)); header.eax=result==uint32_t(reinterpret_cast<uintptr_t>(owner))?0xffffffff:result;
        bool pointerStable=true; for(unsigned o:{12u,16u,20u,28u,32u,36u,88u,92u,96u}) pointerStable=pointerStable && word(before.data()+o)==word(after.data()+o);
        normalize(before.data()); normalize(after.data());
        char path[256]; std::snprintf(path,sizeof(path),"C:/Users/ADMIN/Boxer-lab/ms3d/%s-native-%s-%04u.bin",actionCapture?"action":"clip",selectedReplace?"replacement":"original",sequence);
        FILE *f=std::fopen(path,"wb"); if(f) { std::fwrite(&header,sizeof(header),1,f); std::fwrite(before.data(),1,112,f); std::fwrite(records.data(),1,records.size(),f); std::fwrite(requested.c_str(),1,header.requestSize,f); std::fwrite(after.data(),1,112,f); std::fclose(f); }
        f=std::fopen(actionCapture?"C:/Users/ADMIN/Boxer-lab/ms3d/action-native.jsonl":"C:/Users/ADMIN/Boxer-lab/ms3d/clip-native.jsonl","ab"); if(f) {
            std::fprintf(f,"{\"unit\":\"ANIM-0002\",\"route\":\"%s\",\"call\":%u,\"replacement_calls\":%u,\"caller_rva\":%u,\"owner_role\":\"registered-global-owner\",\"model_role\":\"owner+644\",\"collection_role\":\"model+84\",\"request_role\":\"original-request\",\"count\":%u,\"clock_bits\":%u,\"after_clock_bits\":%u,\"eax_role_or_bits\":%u,\"pointer_words_unchanged\":%s,\"before_model_words\":",selectedReplace?"replacement":"original",sequence,actionCall?actionReplacements:replacements,uint32_t(caller-module),header.count,header.clock,header.afterClock,header.eax,pointerStable?"true":"false"); words(f,before.data()); std::fputs(",\"after_model_words\":",f); words(f,after.data()); std::fputs(",\"records_hex\":",f); hex(f,records.data(),records.size()); std::fputs(",\"request_hex\":",f); hex(f,requested.data(),requested.size()); std::fputs(",\"initial_fp\":",f); fpJson(f,initial); std::fputs(",\"final_fp\":",f); fpJson(f,final); std::fputs(",\"events\":",f); trace.write(f); if(actionCapture) std::fprintf(f,",\"action_state\":%u,\"literal_rva\":%u,\"lifetime_witness\":true",action::roles[actionRole].state,action::roles[actionRole].literal); std::fputs("}\n",f); std::fclose(f);
        }
    }
    if(actionCapture && result==uint32_t(reinterpret_cast<uintptr_t>(owner))) action::selected(model,action::roles[actionRole].state,sequence,selectedReplace);
    return result;
}
}
void observeLookup(int32_t index,void *record) { if(events)events->lookup(index,role(record)); }
bool installAction(uintptr_t base,const char *mode,bool enableMovement) {
    module=base; actionReplace=std::strcmp(mode,"replace")==0; movementEnabled=enableMovement;
    actionEnabled=!runtime_options::original(runtime_options::Unit::Action);
    if(runtime_options::capture(runtime_options::Unit::Action) && (!world::redirect(base+0x10b4,reinterpret_cast<uintptr_t>(&lookup)) || !comparatorCall(base,reinterpret_cast<uintptr_t>(&compare))))return false;
    return world::redirect(base+0x1c26,reinterpret_cast<uintptr_t>(&entry));
}
bool install(uintptr_t base,const char *mode) {
    module=base; replace=std::strcmp(mode,"replace")==0;
    if(runtime_options::capture(runtime_options::Unit::Clip) && (!world::redirect(base+0x10b4,reinterpret_cast<uintptr_t>(&lookup)) || !comparatorCall(base,reinterpret_cast<uintptr_t>(&compare)))) return false;
    return world::redirect(base+0x1c26,reinterpret_cast<uintptr_t>(&entry));
}
}

