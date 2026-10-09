#include "damage_capture.hpp"
#include "damage_fp.hpp"
#include "ai_observer_metadata.hpp"
#include "ai_capture_policy.hpp"
#include "damage_observer_metadata.hpp"
#include "character_native.hpp"
#include "runtime_options.hpp"
#include <windows.h>
#include <vector>
#include <cstring>
#include <cstdio>
#include <cstdlib>
namespace damage {
extern "C" void damage_candidate_entry();
namespace {
uintptr_t module=0;uint32_t audio=0;unsigned limit=256,sequence=0;unsigned categories[6]{};bool aiObservation=false;unsigned aiStarts[3]{},aiNoStart=0;unsigned continuationCounts[3][2]{},continuationUnsupported=0;unsigned lastCompletionSequence=0;DWORD lastCompletionThread=0;uint64_t aiDeadline=0;volatile LONG ownerThread=0;
struct Saved {uintptr_t at;std::vector<uint8_t> original;};
std::vector<Saved> saved;
struct Span {uint32_t rva,size;};
constexpr Span spans[]={{0x1761c8,200},{0x1762a0,128},{0x176370,4},{0x175eec,4},{0x184778,80},{0x185594,4},{0x177f90,16},{0x1849a8,4},{0x184a90,4},{0x1853a4,4},{0x1848e0,16},{0x184ae0,16},{0x184c98,16},{0x17f710,7600},{0x171b90,4},{0x175df0,28}};
void *ptr(uint32_t x){return reinterpret_cast<void*>(uintptr_t(x));}
std::vector<uint8_t> bytes(const void *p,unsigned n){std::vector<uint8_t> v;if(character::native::readable(p,n)){v.resize(n);std::memcpy(v.data(),p,n);}return v;}
Snapshot snapshot(){Snapshot s;s.actor=bytes(ptr(uint32_t(module+0x1849f8)),232);s.player=bytes(ptr(uint32_t(module+0x184910)),176);s.playerAddress=word(ptr(uint32_t(module+0x1849a8)));s.opponentAddress=word(ptr(uint32_t(module+0x184a90)));s.worldAddress=word(ptr(uint32_t(module+0x1853a4)));s.playerBody=bytes(ptr(s.playerAddress),2288);s.opponentBody=bytes(ptr(s.opponentAddress),2288);s.world=bytes(ptr(s.worldAddress+10968),12);for(auto span:spans){auto v=bytes(ptr(uint32_t(module+span.rva)),span.size);s.spanSizes.push_back(unsigned(v.size()));s.globals.insert(s.globals.end(),v.begin(),v.end());}return s;}
struct Context {uint32_t returned=0;int site=-1;bool whole=false,record=false,suppressed=false;Event event;};
thread_local std::vector<Context> stack;
thread_local Capture *active=nullptr;thread_local unsigned suppression=0;thread_local int semanticSite=-1;ScriptCallback scripted=nullptr;bool replacementEnabled=false,observersInstalled=false;Capture completed;
void hex(FILE *f,const void *p,size_t n){std::fputc('"',f);auto b=static_cast<const uint8_t*>(p);for(size_t i=0;i<n;++i)std::fprintf(f,"%02x",b[i]);std::fputc('"',f);}
void hex(FILE *f,const std::vector<uint8_t>&v){hex(f,v.data(),v.size());}
void snapshotJson(FILE*f,const Snapshot&s){std::fputs("{\"actor232\":",f);hex(f,s.actor);std::fputs(",\"player176\":",f);hex(f,s.player);std::fputs(",\"player_body2288\":",f);hex(f,s.playerBody);std::fputs(",\"opponent_body2288\":",f);hex(f,s.opponentBody);std::fputs(",\"globals\":",f);hex(f,s.globals);std::fputs(",\"global_span_sizes\":[",f);for(size_t i=0;i<s.spanSizes.size();++i){if(i)std::fputc(',',f);std::fprintf(f,"%u",s.spanSizes[i]);}std::fputs("]",f);std::fputs(",\"world_list12\":",f);hex(f,s.world);std::fprintf(f,",\"player_body\":%u,\"opponent_body\":%u,\"world\":%u}",s.playerAddress,s.opponentAddress,s.worldAddress);}
unsigned actualAiStart(const Capture&);
unsigned actualContinuation(const Capture&);
void save(){char path[512],directory[384]{};GetEnvironmentVariableA(aiObservation?"OPENBOXER_AI_CAPTURE_DIR":"OPENBOXER_DAMAGE_CAPTURE_DIR",directory,sizeof(directory));if(!directory[0])std::strcpy(directory,"C:/Users/ADMIN/Boxer-lab/ms3d");if(aiObservation)std::snprintf(path,sizeof(path),"%s/ai-%s-%03u.json",directory,aiCaptureCandidateRequested()?"candidate":"original",sequence);else std::snprintf(path,sizeof(path),"%s/damage-%s-%03u.json",directory,active->label,sequence);FILE*f=std::fopen(path,"wb");if(!f)return;auto &c=*active;
std::fprintf(f,"{\"format\":\"%s\",\"candidate\":%s,\"module\":%u,\"caller\":%u,\"mode\":%u,\"eax\":%u,\"entry_fp544\":",(aiObservation || std::strncmp(c.label,"ai-",3)==0)?(continuationCaptureRequested() || std::strncmp(c.label,"ai-continuation-",16)==0)?"GAME-0004-observer-v1":"GAME-0003-observer-v1":"GAME-0002-observer-v1",c.candidate?"true":"false",unsigned(module),c.caller,c.mode,c.eax);hex(f,c.entryFp.data(),544);if(aiObservation)std::fprintf(f,",\"actual_start_type\":%u,\"observation_only\":true,\"process\":%lu,\"tick_ms\":%llu,\"requested_side\":\"%s\",\"actual_route\":\"%s\",\"candidate_route\":%s,\"candidate_invocations\":%u,\"original_invocations\":%u,\"actual_entrypoint\":%u,\"admitted\":%s,\"admission_reason\":\"%s\",\"effects_before_fallback\":0",actualAiStart(c),GetCurrentProcessId(),static_cast<unsigned long long>(GetTickCount64()),aiCaptureCandidateRequested()?"candidate":"original",c.candidate?"candidate":"original",c.candidate?"true":"false",c.candidateInvocations,c.originalInvocations,c.actualEntrypoint,c.admittedRoute?"true":"false",c.admissionReason);
if(aiObservation && continuationCaptureRequested())std::fprintf(f,",\"continuation_class\":%u,\"continuation_type\":%u",actualContinuation(c),c.before.globals.size()>=200?word(c.before.globals.data()+0x17625c-0x1761c8):0);
std::fprintf(f,",\"thread\":%lu",GetCurrentThreadId());std::fputs(",\"exit_fp544\":",f);hex(f,c.exitFp.data(),544);std::fputs(",\"entry_regs36\":",f);hex(f,c.entryRegs.data(),36);std::fputs(",\"exit_regs36\":",f);hex(f,c.exitRegs.data(),36);std::fputs(",\"before\":",f);snapshotJson(f,c.before);std::fputs(",\"after\":",f);snapshotJson(f,c.after);std::fprintf(f,",\"nested_whole_observed\":%s",c.nested?"true":"false");std::fputs(",\"events\":[",f);
for(size_t i=0;i<c.events.size();++i){auto&e=c.events[i];auto&s=damageSites[e.site];if(i)std::fputc(',',f);std::fprintf(f,"{\"ordinal\":%u,\"call_rva\":%u,\"return_rva\":%u,\"identity\":\"%s\",\"owner\":%u,\"args\":[",unsigned(i),s.call,s.returned,s.name,e.owner);for(unsigned j=0;j<s.count;++j){if(j)std::fputc(',',f);std::fprintf(f,"%u",e.args[j]);}std::fprintf(f,"],\"eax\":%u,\"rng_before\":%u,\"rng_after\":%u,\"owner_before\":",e.eax,e.rngBefore,e.rngAfter);hex(f,e.ownerBefore);std::fputs(",\"owner_after\":",f);hex(f,e.ownerAfter);std::fputs(",\"combat_before16\":",f);hex(f,e.combatBefore.data(),16);std::fputs(",\"combat_after16\":",f);hex(f,e.combatAfter.data(),16);std::fputs(",\"arg_before\":[",f);for(unsigned j=0;j<4;++j){if(j)std::fputc(',',f);hex(f,e.beforeArgs[j]);}std::fputs("],\"arg_after\":[",f);for(unsigned j=0;j<4;++j){if(j)std::fputc(',',f);hex(f,e.afterArgs[j]);}std::fputs("],\"returned_bytes\":",f);hex(f,e.returned);std::fputs(",\"entry_fp544\":",f);hex(f,e.entryFp.data(),544);std::fputs(",\"exit_fp544\":",f);hex(f,e.exitFp.data(),544);std::fputs(",\"entry_regs36\":",f);hex(f,e.entryRegs.data(),36);std::fputs(",\"exit_regs36\":",f);hex(f,e.exitRegs.data(),36);if(s.scalar){std::fputs(",\"scalar80\":",f);hex(f,e.exitFp.data()+32,10);}std::fputc('}',f);}std::fputs("]}\n",f);std::fclose(f);}
void combat(std::array<uint32_t,4> &v){v[0]=*static_cast<uint8_t*>(ptr(uint32_t(module+0x176234)));v[1]=*static_cast<uint8_t*>(ptr(uint32_t(module+0x17622c)));v[2]=word(ptr(uint32_t(module+0x176238)));v[3]=word(ptr(uint32_t(module+0x1849f8+164)));}
unsigned ownerSize(const DamageSite&s){if(s.body)return 2288;if(std::strcmp(s.name,"sub_401D4D")==0)return 232;if(std::strcmp(s.name,"sub_401839")==0 || std::strcmp(s.name,"sub_4019C9")==0 || std::strcmp(s.name,"sub_401B1D")==0 || std::strcmp(s.name,"sub_401B36")==0)return 16;return 0;}
bool aiEligible(){
    if(continuationCaptureRequested()){
        auto w=[](uint32_t r){return word(ptr(uint32_t(module+r)));};const uint32_t type=w(0x17625c),state=w(0x184a9c);
        if(*static_cast<uint8_t*>(ptr(uint32_t(module+0x17622d)))!=1 || type<1 || type>3 || (state!=2*type+1 && state!=2*type+2))return false;
        const uint32_t player=w(0x1849a8),opponent=w(0x184a90),world=w(0x1853a4);return player!=opponent && character::native::readable(ptr(player),2288) && character::native::readable(ptr(opponent),2288) && world && character::native::readable(ptr(world+10968),12);
    }
    PreserveFp preserve;const uint16_t cw=0x027f;__asm__ volatile("fninit; fldcw %0"::"m"(cw):"st");
    auto w=[](uint32_t r){return word(ptr(uint32_t(module+r)));};auto b=[](uint32_t r){return *static_cast<uint8_t*>(ptr(uint32_t(module+r)));};
    auto finite=[](uint32_t x){x&=0x7fffffff;return !x || (x>=0x00800000 && x<=0x461c4000);};
    if(b(0x184794) || !b(0x18479d) || w(0x177f90))return false;
    for(auto r:{0x1761c9u,0x1761c8u,0x17622cu,0x17622du,0x17622eu,0x176234u,0x176256u,0x176254u,0x17626cu})if(b(r))return false;
    if(w(0x176270))return false;
    auto*a=static_cast<uint8_t*>(ptr(uint32_t(module+0x1849f8)));if(word(a+164)>2 || word(a+216)<1 || word(a+216)>3)return false;
    const uint32_t dt=w(0x176370),index=w(0x184788);if(index<1 || index>7 || !finite(dt) || (dt&0x80000000 && dt&0x7fffffff))return false;
    const uint32_t duration=w(0x1762a4+16*index),timer=w(0x176230);if(!finite(duration) || duration&0x80000000 || !(duration&0x7fffffff) || !finite(timer) || fp::less(fp::compare(timer,0)) || fp::greater(fp::compare(timer,duration)))return false;
    const uint32_t player=w(0x1849a8),opponent=w(0x184a90),world=w(0x1853a4);
    if(!world || player==opponent || !character::native::readable(ptr(player),2288) || !character::native::readable(ptr(opponent),2288) || !character::native::readable(ptr(world+10968),12) || word(a+152)!=opponent || w(0x184910+152)!=player)return false;
    const uint32_t head=word(ptr(world+10968)),tail=word(ptr(world+10972));if(word(ptr(world+10976))!=2 || !((head==player && tail==opponent)||(head==opponent && tail==player)))return false;
    for(auto body:{player,opponent}){auto*r=static_cast<uint8_t*>(ptr(body));if(word(r)!=body || word(r+244)!=world || r[2284]!=1 || word(r+652) || word(r+216) || word(r+2280)!=(body==head?0:head) || word(r+2276)!=(body==tail?0:tail))return false;for(unsigned off:{752u,756u,760u})if(!finite(word(r+off)))return false;}
    auto*p=static_cast<uint8_t*>(ptr(player));auto*o=static_cast<uint8_t*>(ptr(opponent));const uint32_t xd=fp::difference(word(p+752),word(o+752)),xp=fp::differenceProduct(word(p+752),word(o+752),xd),zd=fp::difference(word(p+760),word(o+760));const uint64_t sum=fp::differenceSum(word(p+760),word(o+760),zd,xp);uint32_t distance;__asm__ volatile("fldl %1; fsqrt; fstps %0":"=m"(distance):"m"(sum):"st");
    if(!fp::greater(fp::compare(distance,0x3f800000)) || !fp::less(fp::compare(distance,0x40c00000)))return false;
    if(a[204]==1)return true;if(!finite(word(a+192)))return false;const uint32_t old=word(a+192),updated=fp::less(fp::compare(old,0))?old:fp::decrement(old,dt);return fp::less(fp::compare(updated,0x3f800000));
}
unsigned actualAiStart(const Capture&c){
    if(c.nested || c.after.actor.size()!=232 || c.after.globals.size()<200)return 0;
    const auto*g=c.after.globals.data();const uint32_t type=word(g+aiCommittedTypeRva-0x1761c8),state=word(c.after.actor.data()+164);
    if(g[aiAttackRva-0x1761c8]!=1 || g[aiPlayerPendingRva-0x1761c8]!=1)return 0;
    const AiStartType*m=nullptr;for(auto&t:aiStartTypes)if(t.type==type)m=&t;if(!m || (state!=m->states[0] && state!=m->states[1]))return 0;
    const Event*found[3]{};unsigned count=0;for(auto&e:c.events){const uint32_t site=damageSites[e.site].call;for(auto&t:aiStartTypes)for(auto call:t.calls)if(site==call){if(count>=3 || site!=m->calls[count])return 0;found[count++]=&e;}}
    if(count!=3 || found[0]->eax>32767 || found[1]->args[0]!=0xc0400000 || found[1]->args[1] || found[1]->args[2] || found[1]->owner!=found[2]->args[0] || found[2]->owner!=c.before.opponentAddress)return 0;
    const auto&v=found[1]->ownerAfter;if(v.size()!=16 || word(v.data())!=0xc0400000 || word(v.data()+4) || word(v.data()+8) || word(v.data()+12)!=0xcccccccc)return 0;
    return type;
}
unsigned actualContinuation(const Capture&c){
    if(!c.admittedRoute || c.nested || c.before.globals.size()<328 || c.after.globals.size()<328 || c.before.actor.size()!=232 || c.after.actor.size()!=232)return 0;
    PreserveFp preserve;const uint16_t cw=0x027f;__asm__ volatile("fninit; fldcw %0"::"m"(cw):"st");
    const auto*b=c.before.globals.data(),*a=c.after.globals.data();auto before=[&](uint32_t r){return word(b+r-0x1761c8);};auto after=[&](uint32_t r){return word(a+r-0x1761c8);};
    const uint32_t type=before(0x17625c),index=word(b+328+4+4+0x184788-0x184778);if(type<1 || type>3 || index<1 || index>7 || after(0x17625c)!=type || a[0]!=b[0])return 0;
    const uint32_t timer=before(0x176230),duration=word(b+200+4+16*index),dt=word(b+328);const bool increments=type==1?!fp::greater(fp::compare(timer,duration)):!fp::less(fp::scaledCompare(duration,type==2?0x3fa66666:0x3ff33333,timer));
    for(const auto&e:c.events){const uint32_t site=damageSites[e.site].call;for(auto&t:aiStartTypes)for(auto call:t.calls)if(site==call)return 0;if(site>=0x1d775 && site<=0x1da6c)return 0;}
    if(increments)return a[0x17622d-0x1761c8]==1 && word(c.after.actor.data()+164)==word(c.before.actor.data()+164) && after(0x176230)==fp::multiplyAdd(0x3c23d70a,dt,timer)?1:0;
    uint32_t fatigue=fp::multiplyAdd(word(b+200+28),0x3fc00000,before(0x176258));if(fp::greater(fp::compare(fatigue,0)))fatigue=fp::subtractProduct(fatigue,0x3e19999a,dt);const bool locked=!fp::less(fp::compare(fatigue,0x42c60000));if(locked)fatigue=0x42c80000;
    return a[0x17622d-0x1761c8]==0 && after(0x176230)==0 && word(c.after.actor.data()+164)==0 && after(0x176258)==fatigue && a[0x176256-0x1761c8]==locked?2:0;
}
bool retainAi(){if(continuationCaptureRequested()){const unsigned cls=actualContinuation(*active);if(cls){const unsigned type=word(active->before.globals.data()+0x17625c-0x1761c8);if(continuationCounts[type-1][cls-1]>=2)return false;++continuationCounts[type-1][cls-1];}else{if(continuationUnsupported>=6)return false;++continuationUnsupported;}++sequence;if(cls==2){lastCompletionSequence=sequence;lastCompletionThread=GetCurrentThreadId();}return true;}const unsigned type=actualAiStart(*active);if(type){if(aiStarts[type-1]>=2)return false;++aiStarts[type-1];}else{if(aiNoStart>=6)return false;++aiNoStart;}++sequence;return true;}
bool patch(uintptr_t at,const void*p,unsigned n){DWORD old;if(!VirtualProtect(reinterpret_cast<void*>(at),n,PAGE_EXECUTE_READWRITE,&old))return false;std::memcpy(reinterpret_cast<void*>(at),p,n);FlushInstructionCache(GetCurrentProcess(),reinterpret_cast<void*>(at),n);DWORD unused;return VirtualProtect(reinterpret_cast<void*>(at),n,old,&unused)!=0;}
}
unsigned latestAiSequence(){return sequence;}
unsigned latestContinuationCompletionSequence(){return lastCompletionThread==GetCurrentThreadId() && lastCompletionSequence==sequence?lastCompletionSequence:0;}
bool continuationCaptureRequested(){char value[8]{};GetEnvironmentVariableA("OPENBOXER_AI_CONTINUATION_CAPTURE",value,sizeof(value));return aiCaptureRequested() && std::strcmp(value,"1")==0;}
bool aiCaptureCandidateRequested(){char side[32]{};GetEnvironmentVariableA("OPENBOXER_AI_CAPTURE_SIDE",side,sizeof(side));return aiCaptureRequested() && std::strcmp(side,"candidate")==0;}
bool validateAiCaptureConfiguration(){char side[32]{},dir[384]{},mode[32]{};GetEnvironmentVariableA("OPENBOXER_AI_CAPTURE_SIDE",side,sizeof(side));GetEnvironmentVariableA("OPENBOXER_AI_CAPTURE_DIR",dir,sizeof(dir));GetEnvironmentVariableA("OPENBOXER_AI_MODE",mode,sizeof(mode));bool otherOriginal=false;for(unsigned i=0;i<unsigned(runtime_options::Unit::Count);++i)if(i!=unsigned(runtime_options::Unit::Clip) && i!=unsigned(runtime_options::Unit::Damage) && runtime_options::original(static_cast<runtime_options::Unit>(i)))otherOriginal=true;if(continuationCaptureRequested() && std::strcmp(side,"candidate")==0){char continuationMode[32]{};GetEnvironmentVariableA("OPENBOXER_AI_CONTINUATION_MODE",continuationMode,sizeof(continuationMode));if(std::strcmp(continuationMode,"replace")!=0)return false;}return capturePolicyValid(aiCaptureRequested(),side,dir[0]!=0,std::strcmp(mode,"replace")==0,runtime_options::original(runtime_options::Unit::Damage),runtime_options::original(runtime_options::Unit::Clip),otherOriginal);}
bool aiCaptureRequested(){char flag[8]{};GetEnvironmentVariableA("OPENBOXER_AI_CAPTURE",flag,sizeof(flag));return std::strcmp(flag,"1")==0;}
extern "C" uint32_t damage_observer_before(uint32_t *frame,uint8_t *fp){
Context c;c.site=int32_t(frame[9]);c.returned=frame[10];c.whole=c.site==-1;
if(c.whole){if(active){c.suppressed=true;++suppression;active->nested=true;}const bool route=frame[6]==module+0x1849f8 && c.returned==module+0x2d8eb && frame[11]<=1 && frame[11]==word(ptr(uint32_t(module+0x184790)));const bool scene=*static_cast<uint8_t*>(ptr(uint32_t(module+0x18479d))) && !word(ptr(uint32_t(module+0x177f90)));const bool strike=*static_cast<uint8_t*>(ptr(uint32_t(module+0x1761c9))) && word(ptr(uint32_t(module+0x1761f8)))==1;
unsigned category=5;bool selected=false;
const LONG thread=LONG(GetCurrentThreadId());const bool owned=ownerThread==thread || (route && scene && strike && InterlockedCompareExchange(&ownerThread,thread,0)==0);
if(!aiObservation && route && scene && strike && owned){const unsigned index=word(ptr(uint32_t(module+0x184784)));if(index<8){float duration,timer;std::memcpy(&duration,ptr(uint32_t(module+0x1762a4+16*index)),4);std::memcpy(&timer,ptr(uint32_t(module+0x1761cc)),4);const bool pending=*static_cast<uint8_t*>(ptr(uint32_t(module+0x17622c)));const bool block=*static_cast<uint8_t*>(ptr(uint32_t(module+0x176234)));if(pending){category=(double(duration)*0.5<double(timer)?2u:0u)+unsigned(block);selected=categories[category]<8;}else{float distance;std::memcpy(&distance,ptr(uint32_t(module+0x175eec)),4);category=4;selected=distance>=6 && categories[4]<8;}}}
if(aiObservation && route && scene && !active && sequence<limit && (!aiDeadline || GetTickCount64()<aiDeadline) && aiEligible()){selected=true;const LONG t=LONG(GetCurrentThreadId());if(ownerThread!=t && InterlockedCompareExchange(&ownerThread,t,0)!=0)selected=false;}
if(!active && route && scene && (aiObservation || strike) && selected && sequence<limit){if(!aiObservation){++sequence;++categories[category];}else if(!aiDeadline)aiDeadline=GetTickCount64()+45000;active=new Capture;active->before=snapshot();if(aiObservation && !aiCaptureCandidateRequested()){active->originalInvocations=1;active->actualEntrypoint=uint32_t(module+0x1c4e0);active->admissionReason="original-observation-forward";if(continuationCaptureRequested()){PreserveFp preserve;alignas(16) std::array<uint8_t,544> entry{};std::memcpy(entry.data(),fp,540);__asm__ volatile("fxrstor (%0); fldenv 512(%0)"::"r"(entry.data()):"memory","st");const char*reason=nullptr;active->admittedRoute=admittedContinuation(module,ptr(uint32_t(module+0x1849f8)),int32_t(frame[11]),c.returned,reason);active->admissionReason=reason;}}active->mode=frame[11];active->caller=c.returned;std::memcpy(active->entryFp.data(),fp,540);std::memcpy(active->entryRegs.data(),frame,36);c.record=true;}}
else if(active && !suppression && (c.returned==module+damageSites[c.site].returned || semanticSite==c.site)){c.record=true;auto&e=c.event;e.site=unsigned(c.site);e.owner=frame[6];auto&s=damageSites[c.site];for(unsigned i=0;i<s.count;++i){e.args[i]=frame[11+i];if(s.pointers[i])e.beforeArgs[i]=bytes(ptr(e.args[i]),s.pointers[i]);}e.ownerBefore=bytes(ptr(e.owner),ownerSize(s));combat(e.combatBefore);e.rngBefore=word(ptr(uint32_t(module+0x171b90)));std::memcpy(e.entryFp.data(),fp,540);std::memcpy(e.entryRegs.data(),frame,36);}
const uint32_t target=c.whole?(replacementEnabled?uint32_t(uintptr_t(&damage_candidate_entry)):uint32_t(module+0x1c4e0)):(scripted?uint32_t(uintptr_t(damageScripts[c.site])):(damageSites[c.site].target?uint32_t(module+damageSites[c.site].target):audio));stack.push_back(std::move(c));return target;
}
extern "C" uint32_t damage_observer_return(uint32_t *frame,uint8_t *fp){Context c=std::move(stack.back());stack.pop_back();if(c.suppressed)--suppression;if(c.record && active){if(c.whole){active->after=snapshot();active->eax=frame[7];std::memcpy(active->exitFp.data(),fp,540);std::memcpy(active->exitRegs.data(),frame,36);if(!aiObservation || retainAi())save();completed=*active;delete active;active=nullptr;}else{auto&e=c.event;auto&s=damageSites[e.site];e.eax=frame[7];combat(e.combatAfter);e.rngAfter=word(ptr(uint32_t(module+0x171b90)));e.ownerAfter=bytes(ptr(e.owner),ownerSize(s));for(unsigned i=0;i<s.count;++i)if(s.pointers[i])e.afterArgs[i]=bytes(ptr(e.args[i]),s.pointers[i]);if(std::strcmp(s.name,"sub_4019C9")==0)e.returnedSize=4;else if(std::strcmp(s.name,"sub_401839")==0 || std::strcmp(s.name,"sub_45AC10")==0 || std::strcmp(s.name,"sub_45B210")==0 || std::strcmp(s.name,"sub_401ABE")==0 || std::strcmp(s.name,"sub_401F64")==0 || std::strcmp(s.name,"sub_401CD0")==0 || std::strcmp(s.name,"sub_401F5F")==0)e.returnedSize=16;e.returned=bytes(ptr(e.eax),e.returnedSize);std::memcpy(e.exitFp.data(),fp,540);std::memcpy(e.exitRegs.data(),frame,36);active->events.push_back(std::move(e));}}return c.returned;}
extern "C" void damage_entry();
bool installObserver(uintptr_t base,bool force,bool includeAi){
if(observersInstalled)return module==base;
aiObservation=aiCaptureRequested();
if(!force && !aiObservation && !runtime_options::capture(runtime_options::Unit::Damage))return true;
if(base!=0x400000 || runtime_options::capture(runtime_options::Unit::Strike))return false;
if(aiObservation){char directory[384]{};GetEnvironmentVariableA("OPENBOXER_AI_CAPTURE_DIR",directory,sizeof(directory));if(!directory[0])return false;if(!aiCaptureCandidateRequested())replacementEnabled=false;}module=base;
auto direct=[&](uint32_t rva,uint32_t target,uint8_t opcode){auto*p=static_cast<uint8_t*>(ptr(uint32_t(base+rva)));return p[0]==opcode && uint32_t(base+rva+5+int32_t(word(p+1)))==base+target;};
if(!direct(0x13a7,0x1c4e0,0xe9)||!direct(0x2d8e6,0x13a7,0xe8))return false;
const unsigned siteCount=(aiObservation || includeAi)?unsigned(sizeof(damageSites)/sizeof(*damageSites)):damageLegacySiteCount;
for(unsigned i=0;i<siteCount;++i){auto&s=damageSites[i];if(s.target && !direct(s.call,s.target,0xe8))return false;}
auto *iat=static_cast<uint32_t*>(ptr(uint32_t(base+0x18cc00)));audio=*iat;auto openal=GetModuleHandleA("OpenAL32.dll");if(!character::native::executable(audio) || !openal || audio!=uint32_t(uintptr_t(GetProcAddress(openal,"alGetSourcei"))))return false;
auto *import=static_cast<uint8_t*>(ptr(uint32_t(base+0x1e1f4)));if(import[0]!=0xff || import[1]!=0x15 || word(import+2)!=base+0x18cc00)return false;
char budget[32]{};GetEnvironmentVariableA(aiObservation?"OPENBOXER_AI_CAPTURE_LIMIT":"OPENBOXER_DAMAGE_CAPTURE_LIMIT",budget,sizeof(budget));if(budget[0])limit=unsigned(std::strtoul(budget,nullptr,10));const unsigned maximum=aiObservation?(continuationCaptureRequested()?18u:12u):256u;if(limit>maximum)limit=maximum;
saved.clear();
auto write=[&](uintptr_t at,const void*p,unsigned n){saved.push_back({at,bytes(reinterpret_cast<void*>(at),n)});return saved.back().original.size()==n && patch(at,p,n);};
bool ok=true;for(unsigned i=0;i<siteCount&&ok;++i){auto&s=damageSites[i];if(s.target){uint8_t b[5]={0xe8};uint32_t delta=uint32_t(uintptr_t(damageWrappers[i])-base-s.call-5);std::memcpy(b+1,&delta,4);ok=write(base+s.call,b,5);}else{uint32_t value=uint32_t(uintptr_t(damageWrappers[i]));ok=write(base+0x18cc00,&value,4);}}
if(ok){uint8_t b[5]={0xe9};uint32_t delta=uint32_t(uintptr_t(&damage_entry)-base-0x13a7-5);std::memcpy(b+1,&delta,4);ok=write(base+0x13a7,b,5);}if(!ok)for(auto i=saved.rbegin();i!=saved.rend();++i)if(!i->original.empty())patch(i->at,i->original.data(),unsigned(i->original.size()));observersInstalled=ok;return ok;
}
bool restoreObserver(){
    if(active || !stack.empty())return false;
    bool ok=true;
    for(auto i=saved.rbegin();i!=saved.rend();++i)
        if(!i->original.empty() && !patch(i->at,i->original.data(),unsigned(i->original.size())))ok=false;
    if(ok){saved.clear();observersInstalled=false;}
    return ok;
}

extern "C" uint32_t damage_script_dispatch(unsigned id,void *owner,uint32_t *args){
    const auto &site=damageSites[id];Call c{Callback(site.id),site.call,owner,{},site.count,false,site.scalar};
    // cdecl dependencies do not own ECX; stack cleanup follows approved ABI.
    c.callerCleanup=site.id==2 || site.id==3 || site.id==4 || site.id==7 || site.id==13 || site.id==14 || site.id==15 || site.id==18 || site.id==20;
    if(c.callerCleanup)c.owner=nullptr;
    for(unsigned i=0;i<c.count;++i)c.args[i]=args[i];
    Result r=scripted(c);__asm__ volatile("fxrstor (%0); fldenv 512(%0)"::"r"(r.fp.data()):"memory","st");return r.eax;
}
void setScript(ScriptCallback value){scripted=value;}
void setReplacementEnabled(bool value){replacementEnabled=value;}
uint32_t audioTarget(uintptr_t gameBase){return resolveAudioTarget(audio,gameBase);}
uint32_t callbackTarget(uint32_t site,uint32_t fallback){semanticSite=-1;if(!observersInstalled)return fallback;for(unsigned i=0;i<sizeof(damageSites)/sizeof(*damageSites);++i)if(damageSites[i].call==site){semanticSite=int(i);return uint32_t(uintptr_t(damageWrappers[i]));}return fallback;}
uint32_t capturedCaller(uint32_t caller){return !stack.empty() && stack.back().whole?stack.back().returned:caller;}
void candidateWitness(bool candidate,const char*reason){if(active && std::strncmp(active->label,"guard-",6)!=0){active->candidate=candidate;active->admittedRoute=candidate;active->admissionReason=reason;active->candidateInvocations=candidate?1:0;active->originalInvocations=candidate?0:1;active->actualEntrypoint=candidate?uint32_t(uintptr_t(&update)):uint32_t(module+0x1c4e0);if(std::strncmp(active->label,"ai-continuation-",16)!=0)active->label=candidate?"candidate":"original";}}
const Capture *lastCapture(){return &completed;}
Snapshot captureSnapshot(uintptr_t base){PreserveFp preserve;module=base;return snapshot();}
void restoreSnapshot(uintptr_t base,const Snapshot&s){PreserveFp preserve;std::memcpy(pointer(uint32_t(base+0x1849f8)),s.actor.data(),232);std::memcpy(pointer(uint32_t(base+0x184910)),s.player.data(),176);size_t offset=0;for(auto span:spans){std::memcpy(pointer(uint32_t(base+span.rva)),s.globals.data()+offset,span.size);offset+=span.size;}if(s.playerBody.size()==2288)std::memcpy(pointer(s.playerAddress),s.playerBody.data(),2288);if(s.opponentBody.size()==2288)std::memcpy(pointer(s.opponentAddress),s.opponentBody.data(),2288);if(s.world.size()==12)std::memcpy(pointer(s.worldAddress+10968),s.world.data(),12);}
void beginManualCapture(uintptr_t base,int32_t mode,const char*label,bool candidate){PreserveFp preserve;module=base;++sequence;active=new Capture;active->before=snapshot();active->mode=uint32_t(mode);active->label=label;active->candidate=candidate;std::memcpy(active->entryFp.data(),preserve.bytes.data(),540);}
void endManualCapture(uint32_t eax,const Witness*w){PreserveFp preserve;if(w){active->entryFp=w->entryFp;active->exitFp=w->exitFp;}else std::memcpy(active->exitFp.data(),preserve.bytes.data(),540);active->after=snapshot();active->eax=eax;save();completed=*active;delete active;active=nullptr;}

} // namespace damage
