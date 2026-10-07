#include "strike_capture.hpp"
#include "strike_observer_metadata.hpp"
#include "runtime_options.hpp"
#include "world_runtime.hpp"
#include "character_native.hpp"
#include <cstdio>
namespace strike {
namespace {
uintptr_t module=0;bool observers=false;
struct Capture {void *player;const char *reason;bool candidate;Snapshot before,after;std::vector<Event> events;};
thread_local Capture *active=nullptr;
unsigned sequence=0;ScriptCallback scripted=nullptr;const char *label="native";
constexpr unsigned counts[]={3,1,1,1,0,0,1,1,4,1,3,3,1,0,3,0,0,2,3};
constexpr bool cleanup[]={false,false,false,false,false,false,false,false,true,true,true,true,false,false,true,false,false,true,true};
bool scalar(Callback id){return id==Callback::ValueLength || id==Callback::Absolute || id==Callback::PointerLength || id==Callback::Sqrt;}
bool patchCall(uintptr_t at,uintptr_t target){uint8_t bytes[5]={0xe8};uint32_t delta=uint32_t(target-at-5);std::memcpy(bytes+1,&delta,4);DWORD old;if(!VirtualProtect(reinterpret_cast<void*>(at),5,PAGE_EXECUTE_READWRITE,&old))return false;std::memcpy(reinterpret_cast<void*>(at),bytes,5);FlushInstructionCache(GetCurrentProcess(),reinterpret_cast<void*>(at),5);DWORD ignored;return VirtualProtect(reinterpret_cast<void*>(at),5,old,&ignored)!=0;}
void hex(FILE *f,const void *p,size_t n){std::fputc('"',f);const auto *b=static_cast<const uint8_t*>(p);for(size_t i=0;i<n;++i)std::fprintf(f,"%02x",b[i]);std::fputc('"',f);}
void writeSnapshot(FILE *f,const Snapshot &s){
    std::fputs("{\"player176\":",f);hex(f,s.player.data(),176);std::fputs(",\"player_body2288\":",f);hex(f,s.playerBody.data(),2288);std::fputs(",\"opponent_body2288\":",f);hex(f,s.opponentBody.data(),2288);
    std::fputs(",\"globals\":",f);hex(f,s.globals.data(),s.globals.size());std::fputs(",\"fp512\":",f);hex(f,s.fp.data(),512);
    std::fprintf(f,",\"world\":%u,\"head\":%u,\"tail\":%u,\"count\":%u,\"active_list\":[",s.world,s.head,s.tail,s.count);
    for(size_t i=0;i<s.activeBodies.size();++i){if(i)std::fputc(',',f);std::fprintf(f,"{\"address\":%u,\"data\":",s.activeAddresses[i]);hex(f,s.activeBodies[i].data(),2288);std::fputc('}',f);}std::fputs("]}",f);
}
void pointerArgs(Event &e){
    switch(e.call.id){case Callback::Position:case Callback::Velocity:case Callback::Impulse:case Callback::AngularZero:case Callback::BodyVector:e.pointerArgument[0]=16;break;
    case Callback::Add:e.pointerArgument={16,16,16,0};break;case Callback::Multiply:e.pointerArgument={16,16,0,0};break;case Callback::ScalarFirstMultiply:e.pointerArgument={16,0,16,0};break;case Callback::AudioQuery:e.pointerArgument[2]=4;break;default:break;}
}
void vectorArgs(Event &e,bool after){auto &vectors=after?e.afterVectors:e.beforeVectors;for(unsigned i=0;i<e.call.count;++i)if(e.pointerArgument[i] && character::native::readable(pointer(e.call.args[i]),e.pointerArgument[i]))std::memcpy(vectors[i].data(),pointer(e.call.args[i]),e.pointerArgument[i]);}
}
void snapshot(uintptr_t base,void *p,Snapshot &s){
    __asm__ volatile("fxsave %0":"=m"(s.fp));runtime_options::PreserveFp preserve;s.globals.clear();s.activeBodies.clear();s.activeAddresses.clear();s.world=s.head=s.tail=s.count=0;
    if(character::native::readable(p,176))std::memcpy(s.player.data(),p,176);
    for(const Span &span:capturedGlobals){size_t off=s.globals.size();s.globals.resize(off+span.size);std::memcpy(s.globals.data()+off,reinterpret_cast<void*>(base+span.rva),span.size);}
    const uint32_t player=word(reinterpret_cast<void*>(base+0x1849a8)),opponent=word(reinterpret_cast<void*>(base+0x184a90));
    if(character::native::readable(pointer(player),2288))std::memcpy(s.playerBody.data(),pointer(player),2288);
    if(character::native::readable(pointer(opponent),2288))std::memcpy(s.opponentBody.data(),pointer(opponent),2288);
    s.world=word(reinterpret_cast<void*>(base+0x1853a4));
    if(!character::native::readable(pointer(s.world+10968),12))return;
    s.head=word(pointer(s.world+10968));s.tail=word(pointer(s.world+10972));s.count=word(pointer(s.world+10976));
    uint32_t node=s.head;for(unsigned i=0;i<s.count && i<64 && node;++i){if(!character::native::readable(pointer(node),2288))break;bool duplicate=false;for(auto old:s.activeAddresses)if(old==node)duplicate=true;if(duplicate)break;
        s.activeAddresses.push_back(node);s.activeBodies.emplace_back();std::memcpy(s.activeBodies.back().data(),pointer(node),2288);node=word(pointer(node+2276));}
}
bool capturing(){return active!=nullptr;}
void scriptCallbacks(ScriptCallback callback){scripted=callback;}
void captureLabel(const char *value){label=value;}
void beginCapture(uintptr_t base,void *p,const char *reason,bool candidate){runtime_options::PreserveFp preserve;module=base;active=new Capture{p,reason,candidate};snapshot(base,p,active->before);}
void endCapture(uint32_t eax){
    runtime_options::PreserveFp preserve;if(!active)return;snapshot(module,active->player,active->after);char path[256];std::snprintf(path,sizeof(path),"C:/Users/ADMIN/Boxer-lab/ms3d/strike-%s-%03u.json",label,++sequence);FILE *f=std::fopen(path,"wb");
    if(f){std::fprintf(f,"{\"format\":\"GAME-0001-typed-v1\",\"candidate\":%s,\"reason\":\"%s\",\"eax\":%u,\"before\":",active->candidate?"true":"false",active->reason,eax);writeSnapshot(f,active->before);std::fputs(",\"after\":",f);writeSnapshot(f,active->after);std::fputs(",\"events\":[",f);
        for(size_t n=0;n<active->events.size();++n){const auto &e=active->events[n];if(n)std::fputc(',',f);std::fprintf(f,"{\"id\":%u,\"caller\":%u,\"owner\":%u,\"args\":[",unsigned(e.call.id),e.caller,address(e.call.owner));for(unsigned i=0;i<e.call.count;++i){if(i)std::fputc(',',f);std::fprintf(f,"%u",e.call.args[i]);}std::fprintf(f,"],\"eax\":%u,\"scalar80\":",e.result.eax);hex(f,e.result.scalar80.data(),10);std::fputs(",\"returned_fp512\":",f);hex(f,e.result.fp.data(),512);
            std::fprintf(f,",\"owner_size\":%u,\"owner_before\":",e.ownerSize);hex(f,e.ownerBefore.data(),e.ownerSize);std::fputs(",\"owner_after\":",f);hex(f,e.ownerAfter.data(),e.ownerSize);
            std::fputs(",\"returned_bytes\":",f);hex(f,e.returnedBytes.data(),e.returnedSize);std::fprintf(f,",\"returned_pointer_size\":%u",e.returnedSize);
            std::fputs(",\"pointer_arg_bytes\":[",f);for(unsigned i=0;i<4;++i){if(i)std::fputc(',',f);std::fprintf(f,"%u",e.pointerArgument[i]);}std::fputs("],\"arg_before\":",f);hex(f,e.beforeVectors.data(),64);std::fputs(",\"arg_after\":",f);hex(f,e.afterVectors.data(),64);std::fputs(",\"before\":",f);writeSnapshot(f,e.before);std::fputs(",\"after\":",f);writeSnapshot(f,e.after);std::fputc('}',f);
        }std::fputs("]}",f);std::fclose(f);}
    std::snprintf(path,sizeof(path),"C:/Users/ADMIN/Boxer-lab/ms3d/strike-%s-%03u.skt",label,sequence);
    Transcript t;t.base=uint32_t(module);t.playerAddress=address(active->player);t.eax=eax;t.candidate=active->candidate;t.supported=std::strcmp(active->reason,"admitted")==0 || std::strcmp(label,"fixture")==0;t.before=active->before;t.after=active->after;t.events=active->events;saveTranscript(path,t);
    delete active;active=nullptr;
}
Result observedCall(BinaryState &state,const Call &call,uint32_t caller){
    if(!active)return state.BinaryState::call(call);
    Event e;e.call=call;e.caller=caller;
    {runtime_options::PreserveFp preserve;pointerArgs(e);snapshot(module,active->player,e.before);vectorArgs(e,false);if(call.id==Callback::Construct)e.ownerSize=12;else if(call.id==Callback::Element || call.id==Callback::Normalize || call.id==Callback::PointerLength)e.ownerSize=16;if(e.ownerSize && character::native::readable(call.owner,e.ownerSize))std::memcpy(e.ownerBefore.data(),call.owner,e.ownerSize);}
    e.result=scripted?scripted(call):state.BinaryState::call(call);
    {runtime_options::PreserveFp preserve;snapshot(module,active->player,e.after);vectorArgs(e,true);if(e.ownerSize && character::native::readable(call.owner,e.ownerSize))std::memcpy(e.ownerAfter.data(),call.owner,e.ownerSize);
        switch(call.id){case Callback::Element:e.returnedSize=4;break;case Callback::Construct:e.returnedSize=12;break;case Callback::Position:case Callback::Velocity:case Callback::Add:case Callback::Multiply:case Callback::ScalarFirstMultiply:e.returnedSize=16;break;default:break;}
        if(e.returnedSize && character::native::readable(pointer(e.result.eax),e.returnedSize))std::memcpy(e.returnedBytes.data(),pointer(e.result.eax),e.returnedSize);active->events.push_back(std::move(e));}
    return active->events.back().result;
}
extern "C" uint32_t strike_observer_dispatch(unsigned id,void *owner,uint32_t *args,uint32_t caller){
    Call c{Callback(id),cleanup[id]?nullptr:owner,{},counts[id],cleanup[id],scalar(Callback(id))};for(unsigned i=0;i<c.count;++i)c.args[i]=args[i];
    BinaryState state(module,active?active->player:nullptr);Result r; if(id==14 && (!active || caller!=module+0x20599)) r=state.BinaryState::call(c); else r=observedCall(state,c,caller-module);return r.eax;
}
extern "C" {void strike_observer0();void strike_observer1();void strike_observer2();void strike_observer3();void strike_observer4();void strike_observer5();void strike_observer6();void strike_observer7();void strike_observer8();void strike_observer9();void strike_observer10();void strike_observer11();void strike_observer12();void strike_observer13();void strike_observer14();void strike_observer15();void strike_observer16();void strike_observer17();void strike_observer18();}
void *observer[]={reinterpret_cast<void*>(&strike_observer0),reinterpret_cast<void*>(&strike_observer1),reinterpret_cast<void*>(&strike_observer2),reinterpret_cast<void*>(&strike_observer3),reinterpret_cast<void*>(&strike_observer4),reinterpret_cast<void*>(&strike_observer5),reinterpret_cast<void*>(&strike_observer6),reinterpret_cast<void*>(&strike_observer7),reinterpret_cast<void*>(&strike_observer8),reinterpret_cast<void*>(&strike_observer9),reinterpret_cast<void*>(&strike_observer10),reinterpret_cast<void*>(&strike_observer11),reinterpret_cast<void*>(&strike_observer12),reinterpret_cast<void*>(&strike_observer13),reinterpret_cast<void*>(&strike_observer14),reinterpret_cast<void*>(&strike_observer15),reinterpret_cast<void*>(&strike_observer16),reinterpret_cast<void*>(&strike_observer17),reinterpret_cast<void*>(&strike_observer18)};
bool installObservers(uintptr_t base){
    if(observers)return module==base;module=base;for(const auto &site:observerSites){const auto *p=reinterpret_cast<const uint8_t*>(base+site.call);if(p[0]!=0xe8 || uintptr_t(base+site.call+5+int32_t(word(p+1)))!=base+site.target)return false;}
    const auto *audioCall=reinterpret_cast<const uint8_t*>(base+0x20593);if(audioCall[0]!=0xff || audioCall[1]!=0x15 || word(audioCall+2)!=base+0x18cc00)return false;
    auto *iat=reinterpret_cast<uint32_t*>(base+0x18cc00);if(!character::native::executable(*iat))return false;originalAudio=*iat;
    observers=true;for(const auto &site:observerSites)if(!patchCall(base+site.call,address(observer[site.id]))){restoreObservers(base);return false;}
    DWORD old;if(!VirtualProtect(iat,4,PAGE_READWRITE,&old)){restoreObservers(base);return false;}*iat=address(observer[14]);DWORD ignored;if(!VirtualProtect(iat,4,old,&ignored)){restoreObservers(base);return false;}return true;
}
void restoreObservers(uintptr_t base){if(!observers)return;for(const auto &site:observerSites)patchCall(base+site.call,base+site.target);auto *iat=reinterpret_cast<uint32_t*>(base+0x18cc00);DWORD old;VirtualProtect(iat,4,PAGE_READWRITE,&old);*iat=uint32_t(originalAudio);DWORD ignored;VirtualProtect(iat,4,old,&ignored);originalAudio=0;observers=false;}
}
