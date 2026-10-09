#include "damage_capture.hpp"
#include "damage_observer_metadata.hpp"
#include "runtime_options.hpp"
#include <cstdio>
#include <string>
#include <stdexcept>
namespace damage {
namespace {
struct Field {uint32_t kind=0,value=0;std::string name;};
struct Role {uint32_t original=0,current=0,size=0;std::string name;};
struct ReplayEvent {Event data;uint32_t site=0;unsigned count=0;Field owner,eax;std::array<Field,4> args;};
struct Transcript {std::string id,exportHash,sourceHash;uint32_t base=0,mode=0,eax=0;alignas(16) std::array<uint8_t,544> entryFp{},exitFp{};Snapshot before,after;std::vector<Role> roles;std::vector<ReplayEvent> events;};
struct Reader {
    FILE*f;bool ok=true;
    uint32_t word(){uint32_t x=0;if(std::fread(&x,4,1,f)!=1)ok=false;return x;}
    std::string string(){const unsigned n=word();if(n>1024){ok=false;return {};}std::string x(n,'\0');if(n && std::fread(x.data(),1,n,f)!=n)ok=false;return x;}
    std::vector<uint8_t> blob(unsigned max){unsigned n=word();if(n>max){ok=false;return {};}std::vector<uint8_t>x(n);if(n && std::fread(x.data(),1,n,f)!=n)ok=false;return x;}
    Field field(){Field x;x.kind=word();x.name=string();x.value=word();if(x.kind>2)ok=false;return x;}
    void snapshot(Snapshot&s){s.actor=blob(232);s.player=blob(176);s.playerBody=blob(2288);s.opponentBody=blob(2288);s.globals=blob(8192);s.world=blob(12);s.playerAddress=word();s.opponentAddress=word();s.worldAddress=word();unsigned n=word();if(n!=16){ok=false;return;}for(unsigned i=0;i<n;++i)s.spanSizes.push_back(word());if(s.actor.size()!=232 || s.player.size()!=176 || s.playerBody.size()!=2288 || s.opponentBody.size()!=2288 || s.globals.size()!=8128 || s.world.size()!=12)ok=false;}
    void fp(std::array<uint8_t,544>&out){auto v=blob(544);if(v.size()!=544)ok=false;else std::memcpy(out.data(),v.data(),544);}
    bool read(Transcript&t){if(word()!=0x33474d44 || word()!=1)return false;t.id=string();t.exportHash=string();t.sourceHash=string();t.base=word();t.mode=word();t.eax=word();fp(t.entryFp);fp(t.exitFp);snapshot(t.before);snapshot(t.after);unsigned n=word();if(n>64)return false;for(unsigned i=0;i<n;++i){Role r;r.original=word();r.name=string();t.roles.push_back(r);}n=word();if(n>512)return false;for(unsigned i=0;i<n;++i){ReplayEvent e;e.site=word();e.data.owner=word();e.data.eax=word();e.data.rngBefore=word();e.data.rngAfter=word();e.count=word();if(e.count>4)return false;for(unsigned j=0;j<e.count;++j)e.data.args[j]=word();e.owner=field();e.eax=field();for(unsigned j=0;j<e.count;++j)e.args[j]=field();e.data.ownerBefore=blob(2288);e.data.ownerAfter=blob(2288);e.data.returned=blob(16);fp(e.data.entryFp);fp(e.data.exitFp);for(auto&v:e.data.beforeArgs)v=blob(16);for(auto&v:e.data.afterArgs)v=blob(16);t.events.push_back(std::move(e));}return ok && std::fgetc(f)==EOF;}
};
Transcript *input=nullptr;uintptr_t module=0;unsigned next=0,errors=0;std::array<uint8_t,2288> playerBody{},opponentBody{};std::array<uint8_t,11000> world{};
Role &role(const std::string&name){for(auto&r:input->roles)if(r.name==name)return r;throw std::runtime_error("undeclared replay pointer role");}
uint32_t resolved(const Field&field){if(!field.kind)return field.value;auto&r=role(field.name);if(!r.current)throw std::runtime_error("unbound replay pointer role");return r.current+(field.kind==2?4*field.value:0);}
void bind(const Field&field,uint32_t current,unsigned size){if(!field.kind){if(field.value!=current)++errors;return;}auto&r=role(field.name);const uint32_t base=current-(field.kind==2?4*field.value:0);if(r.current && r.current!=base)++errors;else{r.current=base;if(size>r.size)r.size=size;}}
uint32_t known(uint32_t old){if(!old)return 0;for(auto&r:input->roles)if(r.original==old && r.current)return r.current;throw std::runtime_error("unknown structural pointer role");}
void bodyPointers(uint8_t*b){for(unsigned off:{0u,244u,2276u,2280u})put(b+off,known(word(b+off)));}
void initializeRoles(){for(auto&r:input->roles){r.current=0;r.size=16;if(r.name=="registered_player_body"){r.current=address(playerBody.data());r.size=2288;}else if(r.name=="registered_opponent_body"){r.current=address(opponentBody.data());r.size=2288;}else if(r.name=="registered_world"){r.current=address(world.data());r.size=11000;}else if(r.name.rfind("L-",0)!=0){r.current=uint32_t(module+r.original-input->base);if(r.name=="static_AI_actor")r.size=232;else if(r.name=="static_player_actor")r.size=176;}else if(r.name=="L-AUDIO")r.size=4;}}
Snapshot relocated(const Snapshot&source){Snapshot value=source;value.playerAddress=address(playerBody.data());value.opponentAddress=address(opponentBody.data());value.worldAddress=address(world.data());bodyPointers(value.playerBody.data());bodyPointers(value.opponentBody.data());put(value.actor.data()+152,address(opponentBody.data()));put(value.player.data()+152,address(playerBody.data()));for(unsigned off:{0u,4u})put(value.world.data()+off,known(word(value.world.data()+off)));size_t cursor=0;constexpr uint32_t spans[][2]={{0x1761c8,200},{0x1762a0,128},{0x176370,4},{0x175eec,4},{0x184778,80},{0x185594,4},{0x177f90,16},{0x1849a8,4},{0x184a90,4},{0x1853a4,4},{0x1848e0,16},{0x184ae0,16},{0x184c98,16},{0x17f710,7600},{0x171b90,4},{0x175df0,28}};for(auto&s:spans){if(s[0]==0x1849a8 || s[0]==0x184a90 || s[0]==0x1853a4)put(value.globals.data()+cursor,known(word(value.globals.data()+cursor)));cursor+=s[1];}return value;}
bool same(const Snapshot&a,const Snapshot&b){return a.actor==b.actor && a.player==b.player && a.playerBody==b.playerBody && a.opponentBody==b.opponentBody && a.globals==b.globals && a.world==b.world && a.playerAddress==b.playerAddress && a.opponentAddress==b.opponentAddress && a.worldAddress==b.worldAddress && a.spanSizes==b.spanSizes;}
void initializeInput(){initializeRoles();playerBody.fill(0);opponentBody.fill(0);world.fill(0);restoreSnapshot(module,relocated(input->before));}
void bytesBefore(const void*p,const std::vector<uint8_t>&expected){if(!expected.empty() && std::memcmp(p,expected.data(),expected.size()))++errors;}
Result scripted(const Call&call){
    if(next>=input->events.size())throw std::runtime_error("extra replay callback");auto&e=input->events[next++];if(e.site!=call.site || e.count!=call.count)throw std::runtime_error("replay callback order/ABI");if(word(pointer(uint32_t(module+0x171b90)))!=e.data.rngBefore)++errors;
    if(!call.callerCleanup){bind(e.owner,address(call.owner),unsigned(e.data.ownerBefore.size()));auto expected=e.data.ownerBefore;if(expected.size()==2288)bodyPointers(expected.data());else if(expected.size()==232)put(expected.data()+152,known(word(expected.data()+152)));bytesBefore(call.owner,expected);}
    for(unsigned i=0;i<call.count;++i){if(e.args[i].kind)bind(e.args[i],call.args[i],unsigned(e.data.beforeArgs[i].size()));else if(e.args[i].value!=call.args[i])++errors;if(!e.data.beforeArgs[i].empty())bytesBefore(pointer(call.args[i]),e.data.beforeArgs[i]);}
    if(!e.data.ownerAfter.empty()){auto value=e.data.ownerAfter;if(value.size()==2288)bodyPointers(value.data());else if(value.size()==232)put(value.data()+152,known(word(value.data()+152)));std::memcpy(call.owner,value.data(),value.size());}
    for(unsigned i=0;i<call.count;++i)if(!e.data.afterArgs[i].empty())std::memcpy(pointer(call.args[i]),e.data.afterArgs[i].data(),e.data.afterArgs[i].size());
    put(pointer(uint32_t(module+0x171b90)),e.data.rngAfter);
    Result result;result.eax=resolved(e.eax);if(!e.data.returned.empty() && call.id!=Callback::Element)std::memcpy(pointer(result.eax),e.data.returned.data(),e.data.returned.size());std::memcpy(result.fp.data(),e.data.exitFp.data(),544);if(call.scalar)std::memcpy(result.scalar80.data(),e.data.exitFp.data()+32,10);return result;
}
}
extern "C" uint32_t damage_dispatch(void*,uintptr_t,int32_t);
extern "C" uint32_t __attribute__((thiscall)) ai_damage_replay_candidate(void*a,int32_t mode){return damage_dispatch(a,module+0x2d8eb,mode);}
uint32_t aiReplays(uintptr_t base){
    PreserveFp preserve;char path[MAX_PATH]{};if(!GetEnvironmentVariableA("OPENBOXER_AI_REPLAY",path,sizeof(path)))return 40;
    FILE*f=std::fopen(path,"rb");if(!f)return 41;Transcript transcript;Reader reader{f};const bool loaded=reader.read(transcript);std::fclose(f);if(!loaded || transcript.base!=base || transcript.mode>1)return 42;
    module=base;input=&transcript;auto saved=captureSnapshot(base);if(!installObserver(base,true,true)){input=nullptr;return 31;}setScript(&scripted);forceFixtureReplacement(false);forceFixtureAiReplacement(true);setReplacementEnabled(false);unsigned failures=0;
    FILE *report=std::fopen("C:/Users/ADMIN/Boxer-lab/ms3d/ai-replay.txt","wb");
    for(unsigned candidate=0;candidate<2;++candidate){try{initializeInput();next=0;errors=0;__asm__ volatile("fxrstor %0"::"m"(*reinterpret_cast<std::array<uint8_t,512>*>(transcript.entryFp.data())):"memory","st");const char *reason=nullptr;if(!admittedAi(base,pointer(uint32_t(base+0x1849f8)),int32_t(transcript.mode),base+0x2d8eb,reason)){++failures;if(report)std::fprintf(report,"source=%s candidate=%u preflight=0 reason=%s\n",transcript.id.c_str(),candidate,reason);continue;}
    beginManualCapture(base,int32_t(transcript.mode),candidate?"ai-replay-candidate":"ai-replay-original",candidate!=0);unsigned replaced=0,fallback=0;routeCounters(&replaced,&fallback);Witness witness{};damage_invoke(candidate?uintptr_t(&ai_damage_replay_candidate):base+0x1c4e0,pointer(uint32_t(base+0x1849f8)),int32_t(transcript.mode),&witness,transcript.entryFp.data());endManualCapture(witness.eax,&witness);routeCounters(nullptr,nullptr);const bool abi=witness.beforeEsp==witness.afterEsp && witness.ebx==0x11223344 && witness.esi==0x22334455 && witness.edi==0x33445566 && witness.ebp==0x44556677;const bool complete=next==transcript.events.size(),route=candidate?(replaced==1 && !fallback):(!replaced && !fallback),sourceState=same(captureSnapshot(base),relocated(transcript.after)),sourceEax=witness.eax==transcript.eax;if(!abi || !complete || errors || !route || !sourceState || !sourceEax)++failures;if(report)std::fprintf(report,"source=%s exportHash=%s sourceHash=%s candidate=%u events=%u/%u typed_errors=%u abi=%u replacement=%u fallback=%u route=%u source_state=%u source_eax=%u eax=%08x expectedSourceEAX=%08x\n",transcript.id.c_str(),transcript.exportHash.c_str(),transcript.sourceHash.c_str(),candidate,next,unsigned(transcript.events.size()),errors,abi,replaced,fallback,route,sourceState,sourceEax,witness.eax,transcript.eax);
    }catch(const std::exception&e){++failures;if(report)std::fprintf(report,"source=%s candidate=%u error=%s\n",transcript.id.c_str(),candidate,e.what());}}
    setScript(nullptr);forceFixtureAiReplacement(false);const bool restored=restoreObserver();if(!restored)++failures;restoreSnapshot(base,saved);input=nullptr;if(report){std::fprintf(report,"cleanup observer_restored=%u failures=%u\n",restored,failures);std::fclose(report);}return failures;
}
}
