#include "damage_capture.hpp"
#include "damage_observer_metadata.hpp"
#include "runtime_options.hpp"
#include "world_runtime.hpp"
#include <cstring>
namespace damage {
namespace {uintptr_t module=0;bool replacement=false;unsigned *replacementCounter=nullptr,*fallbackCounter=nullptr;}
Result BinaryState::call(const Call &call){
    const DamageSite *site=nullptr;for(auto&s:damageSites)if(s.call==call.site){site=&s;break;}
    if(!site || site->id!=unsigned(call.id) || site->count!=call.count)return {};
    uint32_t target=site->target?uint32_t(base_+site->target):audioTarget();
    target=callbackTarget(call.site,target);Result result;damage_bridge(target,address(call.owner),call.args.data(),call.count,call.callerCleanup,&result,call.scalar,liveXmm.data());return result;
}
extern "C" uint32_t damage_dispatch(void *actor,uintptr_t caller,int32_t mode){
    alignas(16) std::array<uint8_t,128> incomingXmm;saveXmm(incomingXmm.data());
    caller=capturedCaller(uint32_t(caller));const char *reason=nullptr;const bool supported=admitted(module,actor,mode,caller,reason);
    if(supported && replacement){if(replacementCounter)++*replacementCounter;candidateWitness(true,reason);BinaryState state(module,actor);restoreXmm(incomingXmm.data());return update(state,mode);}
    if(fallbackCounter)++*fallbackCounter;candidateWitness(false,reason);Result result;const uint32_t argument=uint32_t(mode);damage_bridge(module+0x1c4e0,address(actor),&argument,1,0,&result,0,incomingXmm.data());return result.eax;
}
extern "C" void damage_candidate_entry();
bool install(uintptr_t base,const char *mode){
    module=base;replacement=std::strcmp(mode,"replace")==0 && !runtime_options::original(runtime_options::Unit::Damage);
    setReplacementEnabled(replacement);
    if(runtime_options::capture(runtime_options::Unit::Damage))return installObserver(base,false);
    if(!replacement)return true;
    const auto *entry=reinterpret_cast<const uint8_t*>(base+0x13a7),*caller=reinterpret_cast<const uint8_t*>(base+0x2d8e6);
    if(base!=0x400000 || entry[0]!=0xe9 || uint32_t(base+0x13a7+5+int32_t(word(entry+1)))!=base+0x1c4e0 || caller[0]!=0xe8 || uint32_t(base+0x2d8e6+5+int32_t(word(caller+1)))!=base+0x13a7)return false;
    return world::redirect(base+0x13a7,uintptr_t(&damage_candidate_entry));
}
void routeCounters(unsigned *candidate,unsigned *original){replacementCounter=candidate;fallbackCounter=original;}
void forceFixtureReplacement(bool value){replacement=value;}
}
