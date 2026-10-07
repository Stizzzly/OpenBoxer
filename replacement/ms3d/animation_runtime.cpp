#include "animation_layout.hpp"
#include "animation_fp.hpp"
#include "character_native.hpp"
#include "character_trace.hpp"
#include "runtime_options.hpp"
#include "world_runtime.hpp"
#include "frames_continuation.hpp"
#include <array>
#include <string>
namespace animation {
namespace {
uintptr_t module;
bool replace;
unsigned calls=0,replaced=0;
unsigned continued=0;
bool capturedAdvance=false,capturedSubinterval=false;
thread_local character::Trace *trace=nullptr;
int32_t __attribute__((thiscall)) count(void *collection) {
    const int32_t result=reinterpret_cast<Count>(module+0x9730)(collection);
    if(trace) trace->append("count",nullptr,0,"model.animationCollection",uint32_t(result));
    return result;
}
void *__attribute__((thiscall)) lookup(void *collection,int32_t index) {
    void *result=reinterpret_cast<Lookup>(module+0x97a0)(collection,index);
    if(trace) { const uint32_t arg=uint32_t(index); trace->append("lookup",&arg,1,"model.animationCollection",0,"selectedRecord"); }
    return result;
}
bool supported(void *m,const void *preparedState=nullptr) {
    character::FpPreserver preserve;
    uint16_t cw; __asm__ volatile("fnstcw %0":"=m"(cw));
    alignas(16) uint8_t environment[512]; __asm__ volatile("fxsave %0":"=m"(environment));
    // Arbitrary occupied x87 stacks are outside this first native arithmetic
    // adapter's validated ABI boundary. Reject before prediction or callbacks.
    if(environment[4]!=0) return false;
    if(cw!=0x027f || !character::native::animationWitness(m)) return false;
    const auto *p=static_cast<const uint8_t*>(preparedState?preparedState:m);
    const uint32_t begin=word(p+88),end=word(p+92),capacity=word(p+96);
    const int32_t index=int32_t(word(p+44));
    if(!begin || end<begin || capacity<end || (end-begin)%272 ||
       (capacity-begin)%272 || index<0 || uint32_t(index)>=(end-begin)/272 ||
       !character::native::readable(reinterpret_cast<void*>(uintptr_t(begin)),end-begin)) return false;
    const int32_t rate=int32_t(word(reinterpret_cast<void*>(uintptr_t(begin)+272u*uint32_t(index)+268)));
    const uint32_t now=word(reinterpret_cast<void*>(module+0x17d904)),anchor=word(p+80);
    if(rate<=0 || !fp::finite(now) || !fp::finite(anchor) ||
       word(reinterpret_cast<void*>(module+0x162114))!=0x447a0000 ||
       word(reinterpret_cast<void*>(module+0x162024))!=0x3f800000 ||
       word(reinterpret_cast<void*>(module+0x162040))!=0) return false;
    const uint32_t elapsed=fp::elapsed(now,anchor);
    if(!fp::finite(elapsed)) return false;
    const auto progress=predict(elapsed,rate);
    if(progress.steps>4 || !fp::finite(progress.initialFactor)) return false;
    if(progress.steps) {
        const int32_t frame=int32_t(word(p+56+4*progress.steps));
        if(frame<0 || frame>748 || !fp::finite(fp::moveAnchor(anchor,fp::interval(rate),int32_t(progress.steps)))) return false;
    }
    return true;
}
void stateWords(FILE *f,const std::array<uint8_t,112> &state) {
    std::fputc('[',f);
    for(unsigned i=0;i<28;++i) std::fprintf(f,"%s%u",i?",":"",word(state.data()+4*i));
    std::fputc(']',f);
}
void normalize(std::array<uint8_t,112> &state) {
    // Pointer fields become role IDs. Remaining words are raw typed data bits.
    for(unsigned offset:{12u,16u,20u,28u,32u,36u}) put(state.data()+offset,0);
    put(state.data()+88,1); put(state.data()+92,2); put(state.data()+96,3);
}
uint32_t __attribute__((stdcall,noinline)) entry(void *model) {
    const uintptr_t caller=reinterpret_cast<uintptr_t>(__builtin_return_address(0));
    const bool trusted=continuation::consume(model);
    const bool allowed=(caller==module+0x5e4d || trusted) && supported(model);
    if(!allowed) return reinterpret_cast<Entry>(module+0x6810)(model);
    const unsigned sequence=++calls;
    unsigned predictedSteps=0;
    if(runtime_options::capture(runtime_options::Unit::Animation)) {
        character::FpPreserver preserve;
        const auto *p=static_cast<uint8_t*>(model);
        const int32_t index=int32_t(word(p+44));
        const int32_t rate=int32_t(word(reinterpret_cast<void*>(uintptr_t(word(p+88))+272u*uint32_t(index)+268)));
        predictedSteps=predict(fp::elapsed(word(reinterpret_cast<void*>(module+0x17d904)),word(p+80)),rate).steps;
    }
    const bool capture=runtime_options::capture(runtime_options::Unit::Animation) &&
        ((!predictedSteps && !capturedSubinterval) || (predictedSteps && !capturedAdvance) || sequence==120 || sequence==240);
    character::Trace events;
    std::array<uint8_t,112> before{},after{};
    std::array<uint8_t,272> record{};
    uint32_t now=0,timeAfter=0,collectionCount=0,index=0,result;
    uint16_t cw=0,sw=0,afterSW=0;
    uint32_t mx=0,afterMX=0;
    uint8_t tag=0,afterTag=0;
    if(capture) {
        character::FpPreserver preserve;
        std::memcpy(before.data(),model,112);
        const uint32_t begin=word(before.data()+88);
        index=word(before.data()+44); collectionCount=(word(before.data()+92)-begin)/272;
        std::memcpy(record.data(),reinterpret_cast<void*>(uintptr_t(begin)+272*index),272);
        now=word(reinterpret_cast<void*>(module+0x17d904));
        __asm__ volatile("fnstcw %0; fnstsw %1":"=m"(cw),"=m"(sw));
        alignas(16) uint8_t environment[512]; __asm__ volatile("fxsave %0":"=m"(environment));
        tag=environment[4]; mx=word(environment+24);
        trace=&events;
    }
    if(replace) { BinaryState state(model,{reinterpret_cast<Count>(module+0x14d8),reinterpret_cast<Lookup>(module+0x10b4),reinterpret_cast<void*>(module+0x17d904)}); result=advance(state); ++replaced; if(trusted)++continued; }
    else result=reinterpret_cast<Entry>(module+0x6810)(model);
    trace=nullptr;
    if(capture) {
        __asm__ volatile("fnstsw %0":"=m"(afterSW));
        alignas(16) uint8_t environment[512]; __asm__ volatile("fxsave %0":"=m"(environment));
        afterTag=environment[4]; afterMX=word(environment+24);
        character::FpPreserver preserve;
        std::memcpy(after.data(),model,112); timeAfter=word(reinterpret_cast<void*>(module+0x17d904));
        bool pointerWordsUnchanged=true;
        for(unsigned offset:{12u,16u,20u,28u,32u,36u,88u,92u,96u})
            pointerWordsUnchanged=pointerWordsUnchanged && word(before.data()+offset)==word(after.data()+offset);
        if(predictedSteps) capturedAdvance=true; else capturedSubinterval=true;
        normalize(before); normalize(after);
        char path[256]; std::snprintf(path,sizeof(path),"C:/Users/ADMIN/Boxer-lab/ms3d/animation-native-%s-%04u.bin",replace?"replacement":"original",sequence);
        FILE *f=std::fopen(path,"wb");
        if(f) {
            const uint32_t header[]={0x314d4e41,1,now,collectionCount,index,uint32_t(cw),uint32_t(sw),result,uint32_t(afterSW),timeAfter};
            std::fwrite(header,4,10,f); std::fwrite(before.data(),1,112,f); std::fwrite(record.data(),1,272,f); std::fwrite(after.data(),1,112,f); std::fclose(f);
        }
        f=std::fopen("C:/Users/ADMIN/Boxer-lab/ms3d/animation-native.jsonl","ab");
        if(f) {
            std::fprintf(f,"{\"unit\":\"ANIM-0001\",\"route\":\"%s\",\"call\":%u,\"replacement_calls\":%u,\"clock_bits\":%u,\"count\":%u,\"index\":%u,\"rate\":%d,\"cw\":%u,\"sw\":%u,\"after_sw\":%u,\"eax\":%u,\"after_clock_bits\":%u,\"before_model_words\":",replace?"replacement":"original",sequence,replaced,now,collectionCount,index,int32_t(word(record.data()+268)),cw,sw,afterSW,result,timeAfter);
            stateWords(f,before); std::fputs(",\"after_model_words\":",f); stateWords(f,after);
            std::fprintf(f,",\"pointer_words_unchanged\":%s,\"predicted_steps\":%u,\"mxcsr\":%u,\"after_mxcsr\":%u,\"abridged_tag\":%u,\"after_abridged_tag\":%u,\"top\":%u,\"after_top\":%u,\"events\":",pointerWordsUnchanged?"true":"false",predictedSteps,mx,afterMX,unsigned(tag),unsigned(afterTag),(sw>>11)&7,(afterSW>>11)&7);
            events.write(f); std::fputs("}\n",f); std::fclose(f);
        }
    }
    return result;
}
}
bool framesSupported(void *model,const void *preparedState) { return supported(model,preparedState); }
unsigned nativeReplacementCount() { return replaced; }
unsigned nativeContinuationCount() { return continued; }
bool install(uintptr_t base,const char *mode) {
    module=base; replace=std::strcmp(mode,"replace")==0;
    if(runtime_options::capture(runtime_options::Unit::Animation) &&
       (!world::redirect(base+0x14d8,reinterpret_cast<uintptr_t>(&count)) ||
        !world::redirect(base+0x10b4,reinterpret_cast<uintptr_t>(&lookup)))) return false;
    return world::redirect(base+0x15aa,reinterpret_cast<uintptr_t>(&entry));
}
}
