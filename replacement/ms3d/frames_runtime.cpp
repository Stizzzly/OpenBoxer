#include "frames_layout.hpp"
#include "frames_continuation.hpp"
#include "frames_observer.hpp"
#include "clip_trace.hpp"
#include "character_native.hpp"
#include "world_runtime.hpp"
#include "runtime_options.hpp"
#include "action_capture.hpp"
namespace frames {
namespace {
uintptr_t module=0; bool replace=false;
unsigned calls=0,replacements=0; bool capturedStart=false,capturedWrap=false,capturedRepeat=false;
thread_local character::Trace *trace=nullptr;
thread_local std::array<uint8_t,112> *prepared=nullptr;
extern "C" uint32_t __attribute__((stdcall)) frames_entry(void *);
extern "C" uint32_t __attribute__((stdcall)) frames_observe(void *);
void boundary(void *model,uint32_t opaque) {
    if(!trace)return;
    character::FpPreserver fp; std::memcpy(prepared->data(),model,112);
    uint32_t args[29]; args[0]=opaque; for(unsigned i=0;i<28;++i)args[i+1]=word(prepared->data()+4*i);
    for(unsigned o:{12u,16u,20u,28u,32u,36u})args[1+o/4]=0;
    args[1+88/4]=1; args[1+92/4]=2; args[1+96/4]=3;
    trace->append("advancement-enter",args,29,"model",0,"opaque-entry-ecx");
}
void *__attribute__((thiscall)) lookup(void *collection,int32_t index) { void *record=reinterpret_cast<Lookup>(module+0x97a0)(collection,index); clip::observeLookup(index,record); if(trace) { const uint32_t arg=uint32_t(index); trace->append("lookup",&arg,1,"model+84",0,"selected-record"); } return record; }
int32_t __attribute__((thiscall)) count(void *collection) { const int32_t result=reinterpret_cast<animation::Count>(module+0x9730)(collection); if(trace)trace->append("count",nullptr,0,"model+84",uint32_t(result)); return result; }
uint32_t __attribute__((stdcall)) candidateAdvance(void *model);
// Assembly ABI forwarder retains the parent's incoming opaque ECX.
extern "C" uint32_t frames_candidate_advance_dispatch(void *,uint32_t);
extern "C" uint32_t __attribute__((stdcall)) frames_candidate_advance(void *);
bool supported(void *model,std::array<uint8_t,112> &prospective) {
    character::FpPreserver fp;
    if(!character::native::animationWitness(model))return false;
    std::memcpy(prospective.data(),model,112); auto *m=prospective.data();
    const uint32_t begin=word(m+88),end=word(m+92),capacity=word(m+96);
    const int32_t gate=int32_t(word(m+40)),index=int32_t(word(m+44)),frame=int32_t(word(m+48));
    if(!begin || end<begin || capacity<end || (end-begin)%272 || (capacity-begin)%272 || gate<=0 || uint32_t(gate)>(end-begin)/272 || index<0 || uint32_t(index)>=(end-begin)/272 || frame<0 || frame>748 || !character::native::readable(reinterpret_cast<void*>(uintptr_t(begin)),end-begin))return false;
    const auto *record=reinterpret_cast<const uint8_t*>(uintptr_t(begin)+272u*uint32_t(index));
    const int32_t start=int32_t(word(record+256)),divisor=int32_t(word(record+260));
    if(start<0 || start>=divisor || divisor>749)return false;
    uint32_t value=next(uint32_t(frame),divisor,start); put(m+52,value); put(m+60,value);
    for(unsigned offset:{64u,68u,72u}) { value=next(value,divisor,start); put(m+offset,value); }
    return animation::framesSupported(model,m);
}
void modelWords(FILE *f,const std::array<uint8_t,112> &m) { clip::words(f,m.data()); }
}
extern "C" uint32_t frames_candidate_advance_dispatch(void *model,uint32_t opaque) {
    boundary(model,opaque);
    animation::continuation::Scope admission(model);
    return frames_call(reinterpret_cast<Entry>(module+0x15aa),model,opaque);
}
extern "C" uint32_t frames_observe_dispatch(void *model,uint32_t opaque,uint32_t caller) {
    boundary(model,opaque);
    if(trace)trace->append("reference-advance",&caller,1,"original-body6810");
    return frames_call(reinterpret_cast<Entry>(module+0x6810),model,opaque);
}
extern "C" uint32_t frames_dispatch(void *model,uint32_t opaque,uint32_t caller) {
    std::array<uint8_t,112> prospective{};
    if(caller!=module+0x5f0d || !supported(model,prospective)) { action::pending={}; return frames_call(reinterpret_cast<Entry>(module+0x5d40),model,opaque); }
    const bool actionCapture=action::waiting(model);
    if(action::pending.model && !actionCapture)action::pending={};
    action::CompositionScope actionScope(actionCapture);
    const unsigned framesBefore=replacements;
    const unsigned sequence=++calls;
    auto *m=static_cast<uint8_t*>(model); const auto *record=reinterpret_cast<uint8_t*>(uintptr_t(word(m+88))+272u*word(m+44));
    const uint32_t start=word(record+256),divisor=word(record+260),frame=word(m+48);
    const bool wrap=frame+4>=divisor;
    const bool capture=actionCapture || (runtime_options::capture(runtime_options::Unit::Frames) && ((!capturedStart && frame==start) || (!capturedWrap && wrap) || (!capturedRepeat && sequence>=120)));
    character::Trace events; std::array<uint8_t,112> before{},after{},boundaryState{}; std::array<uint8_t,272> selected{};
    uint32_t now=0,afterNow=0; clip::Fp initial{},final{};
    const unsigned advancedBefore=animation::nativeReplacementCount(),continuationBefore=animation::nativeContinuationCount();
    if(capture) { initial=clip::environment(); character::FpPreserver fp; std::memcpy(before.data(),m,112); std::memcpy(selected.data(),record,272); now=word(reinterpret_cast<void*>(module+0x17d904)); trace=&events; prepared=&boundaryState; }
    uint32_t result;
    if(replace) { BinaryState state(model,{reinterpret_cast<Lookup>(module+0x10b4),frames_candidate_advance}); result=prepare(state,opaque); ++replacements; }
    else result=frames_call(reinterpret_cast<Entry>(module+0x5d40),model,opaque);
    trace=nullptr; prepared=nullptr;
    if(capture) {
        final=clip::environment(); character::FpPreserver fp; std::memcpy(after.data(),m,112); afterNow=word(reinterpret_cast<void*>(module+0x17d904));
        if(!actionCapture && frame==start)capturedStart=true; if(wrap)capturedWrap=true; if(sequence>=120)capturedRepeat=true;
        if(actionCapture)action::completed(model,before.data(),boundaryState.data(),after.data(),result,framesBefore,replacements,advancedBefore,animation::nativeReplacementCount(),animation::nativeContinuationCount()-continuationBefore,sequence,replace);
        clip::normalize(before.data()); clip::normalize(after.data()); clip::normalize(boundaryState.data());
        char path[256]; std::snprintf(path,sizeof(path),"C:/Users/ADMIN/Boxer-lab/ms3d/%s-%s-%04u.bin",actionCapture?"action-pose":"frames-native",replace?"replacement":"original",sequence);
        FILE *f=std::fopen(path,"wb"); if(f) { const uint32_t header[]={0x334d5246,1,now,afterNow,result,opaque,uint32_t(initial.cw),uint32_t(initial.sw),word(before.data()+44),uint32_t(advancedBefore),uint32_t(animation::nativeReplacementCount()),uint32_t(animation::nativeContinuationCount()-continuationBefore)}; std::fwrite(header,4,12,f); std::fwrite(before.data(),1,112,f); std::fwrite(selected.data(),1,272,f); std::fwrite(boundaryState.data(),1,112,f); std::fwrite(after.data(),1,112,f); std::fclose(f); }
        f=std::fopen("C:/Users/ADMIN/Boxer-lab/ms3d/frames-native.jsonl","ab"); if(f) {
            std::fprintf(f,"{\"unit\":\"ANIM-0003\",\"route\":\"%s\",\"call\":%u,\"replacement_calls\":%u,\"caller_rva\":%u,\"opaque_entry_ecx\":%u,\"clock_bits\":%u,\"after_clock_bits\":%u,\"eax\":%u,\"start_checkpoint\":%s,\"wrap_checkpoint\":%s,\"repeat_checkpoint\":%s,\"animation_replacements_before\":%u,\"animation_replacements_after\":%u,\"trusted_continuations_delta\":%u,\"before_model_words\":",replace?"replacement":"original",sequence,replacements,caller-uint32_t(module),opaque,now,afterNow,result,frame==start?"true":"false",wrap?"true":"false",sequence>=120?"true":"false",advancedBefore,animation::nativeReplacementCount(),animation::nativeContinuationCount()-continuationBefore); modelWords(f,before); std::fputs(",\"prepared_model_words\":",f); modelWords(f,boundaryState); std::fputs(",\"after_model_words\":",f); modelWords(f,after); std::fputs(",\"record_hex\":",f); clip::hex(f,selected.data(),272); std::fputs(",\"initial_fp\":",f); clip::fpJson(f,initial); std::fputs(",\"final_fp\":",f); clip::fpJson(f,final); std::fputs(",\"events\":",f); events.write(f); std::fputs("}\n",f); std::fclose(f);
        }
    }
    return result;
}
bool install(uintptr_t base,const char *mode) {
    module=base; replace=std::strcmp(mode,"replace")==0;
    if((runtime_options::capture(runtime_options::Unit::Frames) || runtime_options::capture(runtime_options::Unit::Action)) && (!world::redirect(base+0x10b4,reinterpret_cast<uintptr_t>(&lookup)) || !world::redirect(base+0x14d8,reinterpret_cast<uintptr_t>(&count)) || !advancementCall(base,reinterpret_cast<uintptr_t>(&frames_observe))))return false;
    return world::redirect(base+0x1d66,reinterpret_cast<uintptr_t>(&frames_entry));
}
}
