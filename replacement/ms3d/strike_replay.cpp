#include "strike_capture.hpp"
#include "runtime_options.hpp"
#include <cstdio>
#include <stdexcept>
#include <windows.h>
namespace strike {
namespace {
struct Stream {FILE *f;bool writing;bool ok=true;
 void bytes(void *p,size_t n){if(!ok)return;ok=(writing?std::fwrite(p,1,n,f):std::fread(p,1,n,f))==n;}
 template<class T>void scalar(T &v){bytes(&v,sizeof(v));}
 template<class T>void sequence(std::vector<T> &v,unsigned maximum){uint32_t n=uint32_t(v.size());scalar(n);if(n>maximum){ok=false;return;}if(!writing)v.resize(n);for(auto &a:v)bytes(&a,sizeof(a));}
 void snap(Snapshot &s){bytes(s.player.data(),176);bytes(s.playerBody.data(),2288);bytes(s.opponentBody.data(),2288);sequence(s.globals,1024);sequence(s.activeBodies,64);sequence(s.activeAddresses,64);scalar(s.world);scalar(s.head);scalar(s.tail);scalar(s.count);bytes(s.fp.data(),512);}
 void event(Event &e){uint32_t id=unsigned(e.call.id),owner=address(e.call.owner),count=e.call.count,flags=unsigned(e.call.callerCleanup)|(unsigned(e.call.scalar)<<1);scalar(id);scalar(owner);scalar(count);scalar(flags);if(id>18 || count>4){ok=false;return;}if(!writing){e.call.id=Callback(id);e.call.owner=pointer(owner);e.call.count=count;e.call.callerCleanup=flags&1;e.call.scalar=flags&2;}bytes(e.call.args.data(),16);scalar(e.caller);bytes(e.beforeVectors.data(),64);bytes(e.afterVectors.data(),64);bytes(e.pointerArgument.data(),sizeof(e.pointerArgument));scalar(e.result.eax);bytes(e.result.scalar80.data(),10);bytes(e.result.fp.data(),512);bytes(e.returnedBytes.data(),16);scalar(e.returnedSize);scalar(e.ownerSize);bytes(e.ownerBefore.data(),16);bytes(e.ownerAfter.data(),16);if(e.returnedSize>16 || e.ownerSize>16){ok=false;return;}snap(e.before);snap(e.after);}
 void transcript(Transcript &t){uint32_t magic=0x31544b53,version=3;scalar(magic);scalar(version);if(magic!=0x31544b53 || version!=3){ok=false;return;}scalar(t.base);scalar(t.playerAddress);scalar(t.eax);scalar(t.candidate);scalar(t.supported);snap(t.before);snap(t.after);uint32_t n=uint32_t(t.events.size());scalar(n);if(n>256){ok=false;return;}if(!writing)t.events.resize(n);for(auto &e:t.events)event(e);}
};
size_t offset(uint32_t rva){size_t result=0;for(auto span:capturedGlobals){if(span.rva==rva)return result;result+=span.size;}throw std::runtime_error("missing global role");}
Transcript *recorded=nullptr;uintptr_t module;uint8_t *p;unsigned nextEvent=0;bool mismatch=false;
std::array<uint8_t,2288> body{},opponent{};std::array<uint8_t,11000> world{};struct LocalRole{uint32_t original,current,size;};std::vector<LocalRole> locals;
uint32_t originalBody(bool player){return word(recorded->before.globals.data()+offset(player?0x1849a8:0x184a90));}
uint32_t remap(uint32_t old){const auto &t=*recorded;
 if(old>=t.playerAddress && old<t.playerAddress+176)return address(p)+old-t.playerAddress;
 for(auto span:capturedGlobals)if(old>=t.base+span.rva && old<t.base+span.rva+span.size)return uint32_t(module+span.rva)+old-t.base-span.rva;
 for(bool player:{true,false}){const uint32_t b=originalBody(player);if(old>=b && old<b+2288)return address(player?body.data():opponent.data())+old-b;}
 if(old>=t.before.world && old<t.before.world+11000)return address(world.data())+old-t.before.world;
 for(const auto &local:locals)if(old>=local.original && old<local.original+local.size)return local.current+old-local.original;
 return old;
}
bool fixedRole(uint32_t old){const auto &t=*recorded;if(old>=t.playerAddress && old<t.playerAddress+176)return true;for(auto span:capturedGlobals)if(old>=t.base+span.rva && old<t.base+span.rva+span.size)return true;for(bool player:{true,false})if(old>=originalBody(player) && old<originalBody(player)+2288)return true;return old>=t.before.world && old<t.before.world+11000;}
bool bind(uint32_t original,uint32_t current,unsigned size){if(fixedRole(original))return current==remap(original);for(auto &local:locals)if(original>=local.original && original<local.original+local.size)return current==local.current+original-local.original;locals.push_back({original,current,size});return true;}
void rebaseBody(std::array<uint8_t,2288> &b){for(unsigned off:{0u,244u,2276u,2280u})put(b.data()+off,remap(word(b.data()+off)));}
void copyInput(const Snapshot &s){std::memcpy(p,s.player.data(),176);put(p+152,address(body.data()));body=s.playerBody;opponent=s.opponentBody;rebaseBody(body);rebaseBody(opponent);world.fill(0);put(world.data()+10968,remap(s.head));put(world.data()+10972,remap(s.tail));put(world.data()+10976,s.count);size_t off=0;
 for(auto span:capturedGlobals){std::memcpy(reinterpret_cast<void*>(module+span.rva),s.globals.data()+off,span.size);if(span.rva==0x1849a8 || span.rva==0x184a90 || span.rva==0x1853a4)put(reinterpret_cast<void*>(module+span.rva),remap(word(s.globals.data()+off)));off+=span.size;}
}
void delta(uint8_t *dst,const uint8_t *before,const uint8_t *after,size_t n){for(size_t i=0;i<n;++i)if(before[i]!=after[i])dst[i]=after[i];}
void changes(const Snapshot &a,const Snapshot &b){delta(p,a.player.data(),b.player.data(),176);put(p+152,address(body.data()));delta(body.data(),a.playerBody.data(),b.playerBody.data(),2288);delta(opponent.data(),a.opponentBody.data(),b.opponentBody.data(),2288);rebaseBody(body);rebaseBody(opponent);size_t off=0;for(auto span:capturedGlobals){if(span.rva!=0x1849a8 && span.rva!=0x184a90 && span.rva!=0x1853a4)delta(reinterpret_cast<uint8_t*>(module+span.rva),a.globals.data()+off,b.globals.data()+off,span.size);off+=span.size;}}
Result replayCall(const Call &c){if(nextEvent>=recorded->events.size()){mismatch=true;throw std::runtime_error("extra replay callback");}const Event &e=recorded->events[nextEvent++];if(c.id!=e.call.id || c.count!=e.call.count){mismatch=true;throw std::runtime_error("replay callback order");}
 const uint32_t own=address(c.owner);if(e.call.owner && (e.ownerSize?!bind(address(e.call.owner),own,16):own!=remap(address(e.call.owner))))mismatch=true;
 for(unsigned i=0;i<c.count;++i){if(e.pointerArgument[i]){if(!bind(e.call.args[i],c.args[i],e.pointerArgument[i]))mismatch=true;if(std::memcmp(pointer(c.args[i]),e.beforeVectors[i].data(),e.pointerArgument[i]))mismatch=true;}else if(c.args[i]!=e.call.args[i])mismatch=true;}
 if(e.ownerSize && std::memcmp(c.owner,e.ownerBefore.data(),e.ownerSize))mismatch=true;
 changes(e.before,e.after);if(e.ownerSize)std::memcpy(c.owner,e.ownerAfter.data(),e.ownerSize);for(unsigned i=0;i<c.count;++i)if(e.pointerArgument[i])std::memcpy(pointer(c.args[i]),e.afterVectors[i].data(),e.pointerArgument[i]);
 // Constructor and normalize have mutable ECX ranges rather than stack pointer arguments.
 if(c.id==Callback::Construct){for(unsigned i=0;i<3;++i)put(static_cast<uint8_t*>(c.owner)+4*i,c.args[i]);}
 if(c.id==Callback::Normalize){/* local is unused by the parent; return/state remains opaque */}
 Result r=e.result;
 if(e.returnedSize){uint32_t result=remap(e.result.eax);bool found=result!=e.result.eax;
   if(e.result.eax==address(e.call.owner)){result=own;found=true;}
   if(c.id==Callback::Element && e.result.eax==address(e.call.owner)+4*c.args[0]){result=own+4*c.args[0];found=true;}
   for(unsigned i=0;i<c.count;++i)if(e.pointerArgument[i] && e.result.eax>=e.call.args[i] && e.result.eax<e.call.args[i]+e.pointerArgument[i]){result=c.args[i]+e.result.eax-e.call.args[i];found=true;}
   if(!found){mismatch=true;throw std::runtime_error("unapproved returned pointer role");}
   r.eax=result;if(c.id!=Callback::Element)std::memcpy(pointer(result),e.returnedBytes.data(),e.returnedSize);
 }
 __asm__ volatile("fxrstor %0"::"m"(r.fp):"memory","st","xmm0","xmm1","xmm2","xmm3","xmm4","xmm5","xmm6","xmm7");return r;
}
bool approvedReturns(){std::vector<LocalRole> declared;for(const auto &e:recorded->events){if(e.ownerSize)declared.push_back({address(e.call.owner),0,16});for(unsigned i=0;i<e.call.count;++i)if(e.pointerArgument[i])declared.push_back({e.call.args[i],0,e.pointerArgument[i]});if(e.returnedSize){bool recognized=fixedRole(e.result.eax);for(const auto &range:declared)if(e.result.eax>=range.original && uint64_t(e.result.eax)+e.returnedSize<=uint64_t(range.original)+range.size)recognized=true;if(!recognized)return false;}}return true;}
class ReplayState:public BinaryState{public:using BinaryState::BinaryState;Result call(const Call &c) override{return observedCall(*this,c,0);}};
}
bool saveTranscript(const char *path,const Transcript &source){FILE *f=std::fopen(path,"wb");if(!f)return false;Transcript t=source;Stream s{f,true};s.transcript(t);bool ok=s.ok;return std::fclose(f)==0 && ok;}
bool loadTranscript(const char *path,Transcript &t){FILE *f=std::fopen(path,"rb");if(!f)return false;Stream s{f,false};s.transcript(t);bool ok=s.ok && std::fgetc(f)==EOF;std::fclose(f);return ok;}
uint32_t replays(uintptr_t base){runtime_options::PreserveFp preserve;char path[MAX_PATH]{};if(!GetEnvironmentVariableA("OPENBOXER_STRIKE_REPLAY",path,sizeof(path)))return 40;Transcript t;if(!loadTranscript(path,t) || t.candidate || !t.supported || t.events.empty())return 41;if(t.before.count!=2 || t.before.activeAddresses.size()!=2 || t.before.activeBodies.size()!=2 || t.after.count!=2)return 42;module=base;p=reinterpret_cast<uint8_t*>(base+0x184910);recorded=&t;if(!approvedReturns()){recorded=nullptr;return 43;}Snapshot saved;snapshot(base,p,saved);if(!installObservers(base))return 31;scriptCallbacks(&replayCall);captureLabel("replay");unsigned failures=0;
 for(unsigned candidate=0;candidate<2;++candidate){locals.clear();copyInput(t.before);nextEvent=0;mismatch=false;__asm__ volatile("fxrstor %0"::"m"(t.before.fp):"memory","st","xmm0","xmm1","xmm2","xmm3","xmm4","xmm5","xmm6","xmm7");beginCapture(base,p,"transcript-isolated-replay",candidate!=0);uint32_t eax=0;
 try {if(candidate){ReplayState state(base,p);eax=update(state);}else {Result r;strike_bridge(base+0x1ef90,address(p),nullptr,0,0,&r,0);eax=r.eax;}}catch(...){mismatch=true;}
 if(nextEvent!=t.events.size() || mismatch)++failures;endCapture(eax);
 }
 scriptCallbacks(nullptr);restoreObservers(base);std::memcpy(p,saved.player.data(),176);size_t off=0;for(auto span:capturedGlobals){std::memcpy(reinterpret_cast<void*>(base+span.rva),saved.globals.data()+off,span.size);off+=span.size;}captureLabel("native");recorded=nullptr;return failures;
}
}
