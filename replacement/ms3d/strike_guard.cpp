#include "strike_layout.hpp"
#include "runtime_options.hpp"
#include "strike_capture.hpp"
#include <windows.h>
#include <algorithm>
#include <array>
namespace strike {
namespace {
bool writable(const void *p,size_t n){
    uintptr_t a=reinterpret_cast<uintptr_t>(p),end=a+n;if(!a || end<a)return false;
    while(a<end){MEMORY_BASIC_INFORMATION m{};if(!VirtualQuery(reinterpret_cast<void*>(a),&m,sizeof(m)) || m.State!=MEM_COMMIT || m.Protect&(PAGE_GUARD|PAGE_NOACCESS))return false;
        const DWORD kind=m.Protect&0xff;if(kind!=PAGE_READWRITE && kind!=PAGE_WRITECOPY && kind!=PAGE_EXECUTE_READWRITE && kind!=PAGE_EXECUTE_WRITECOPY)return false;
        uintptr_t next=reinterpret_cast<uintptr_t>(m.BaseAddress)+m.RegionSize;if(next<=a)return false;a=next;}
    return true;
}
bool supportedFloat(uint32_t v){const uint32_t a=v&0x7fffffffu;return a==0 || (a>=0x00800000u && a<=0x47c35000u);}
bool bodyFloat(uint32_t v){const uint32_t a=v&0x7fffffffu;return a==0 || (a>=0x00800000u && a<=0x461c4000u);}
bool vector(const uint8_t *p){return supportedFloat(word(p)) && supportedFloat(word(p+4)) && supportedFloat(word(p+8));}
bool overlap(uintptr_t a,size_t na,uintptr_t b,size_t nb){return a<b+nb && b<a+na;}
}
bool admitted(uintptr_t base,void *object,uintptr_t caller,const char *&reason){
    runtime_options::PreserveFp preserve;
    auto fail=[&](const char *r){reason=r;return false;};
    if(caller!=base+0x2d8a2 || reinterpret_cast<uintptr_t>(object)!=base+0x184910)return fail("caller-owner");
    auto *p=static_cast<uint8_t*>(object);if(!writable(p,176))return fail("player-range");
    alignas(16) std::array<uint8_t,512> env{};__asm__ volatile("fxsave %0":"=m"(env));uint16_t cw,sw;std::memcpy(&cw,env.data(),2);std::memcpy(&sw,env.data()+2,2);
    if(cw!=0x027f || (sw&0x3800) || env[4]!=0)return fail("fp");
    auto b=[&](uint32_t o){return *reinterpret_cast<uint8_t*>(base+o);};auto w=[&](uint32_t o){return word(reinterpret_cast<void*>(base+o));};
    if(b(0x184794) || !b(0x18479d) || w(0x177f90))return fail("inactive-menu");
    for(uint32_t k:{0x176956u,0x17696au,0x176939u,0x17693bu,0x176934u})if(b(k))return fail("movement-block-space");
    const uint32_t keys[]={0x17696e,0x17696c,0x176957},latches[]={0x185146,0x185144,0x18512f};unsigned active=0;
    for(unsigned i=0;i<3;++i)if(b(keys[i])){++active;if(b(latches[i]))return fail("latched");}if(active!=1)return fail("key-count");
    for(uint32_t o:{0x1761c9u,0x1761cau,0x1761c8u,0x1761d0u,0x1761f2u,0x1761f0u,0x176208u,0x17622cu,0x17628cu})if(b(o))return fail("entry-flags");
    if(word(p+164) || p[171] || p[168]>1 || p[169]>1 || p[170]>1 || w(0x1761cc) || w(0x1761fc))return fail("state-timer-combo");
    uint32_t index=w(0x184784);if(index>7)return fail("fighter");uint32_t duration=w(0x1762a4+16*index);if(!supportedFloat(duration) || !duration || duration&0x80000000)return fail("duration");
    for(uint32_t o:{0x1761d4u,0x176238u}){uint32_t v=w(o);if(!supportedFloat(v) || (v&0x80000000 && (v&0x7fffffff)))return fail("health");}
    uint32_t fatigue=w(0x1761f4),dt=w(0x176370);if(!supportedFloat(fatigue) || ((fatigue&0x7fffffffu)>=0x42c60000 || ((fatigue&0x80000000u) && (fatigue&0x7fffffffu))) || !supportedFloat(dt) || ((dt&0x7fffffffu)>0x41200000 || ((dt&0x80000000u) && (dt&0x7fffffffu))))return fail("fatigue-dt");
    for(unsigned o:{12u,28u,76u,92u,108u,124u})if(!vector(p+o))return fail("player-vector");
    if(!supportedFloat(word(p+140)) || !supportedFloat(word(p+172)))return fail("player-scalars");
    uintptr_t world=w(0x1853a4),player=w(0x1849a8),opponent=w(0x184a90);
    if(!world || !player || !opponent || player==opponent || word(p+152)!=player || !writable(reinterpret_cast<void*>(world+10968),12))return fail("body-world");
    if(!writable(reinterpret_cast<void*>(player),2288) || !writable(reinterpret_cast<void*>(opponent),2288) || word(reinterpret_cast<void*>(player+216)))return fail("body-range-graph");
    if(overlap(player,2288,opponent,2288) || overlap(player,2288,reinterpret_cast<uintptr_t>(p),176) || overlap(opponent,2288,reinterpret_cast<uintptr_t>(p),176))return fail("body-alias");
    for(const auto &span:capturedGlobals){const uintptr_t a=base+span.rva;if(!writable(reinterpret_cast<void*>(a),span.size))return fail("global-range");if(overlap(player,2288,a,span.size) || overlap(opponent,2288,a,span.size) || (span.rva!=0x1849a8 && overlap(reinterpret_cast<uintptr_t>(p),176,a,span.size)) || overlap(world+10968,12,a,span.size))return fail("global-alias");}
    if(overlap(world+10968,12,player,2288) || overlap(world+10968,12,opponent,2288) || overlap(world+10968,12,reinterpret_cast<uintptr_t>(p),176))return fail("world-alias");
    const uint32_t count=word(reinterpret_cast<void*>(world+10976));if(count<2 || count>64)return fail("list-count");
    uintptr_t node=word(reinterpret_cast<void*>(world+10968)),tail=word(reinterpret_cast<void*>(world+10972)),previous=0;std::array<uintptr_t,64> seen{};bool foundPlayer=false,foundOpponent=false;
    for(unsigned i=0;i<count;++i){if(!node || !writable(reinterpret_cast<void*>(node),2288))return fail("node-range");for(unsigned j=0;j<i;++j)if(seen[j]==node)return fail("node-cycle");for(unsigned j=0;j<i;++j)if(overlap(node,2288,seen[j],2288))return fail("node-overlap");
        if(overlap(node,2288,reinterpret_cast<uintptr_t>(p),176) || overlap(node,2288,world+10968,12))return fail("node-alias");for(const auto &span:capturedGlobals)if(overlap(node,2288,base+span.rva,span.size))return fail("node-global-alias");seen[i]=node;
        auto *n=reinterpret_cast<uint8_t*>(node);if(word(n)!=node || word(n+244)!=world || n[2284]!=1 || word(n+2280)!=previous)return fail("node-lifetime");
        if(node==player || node==opponent){uint32_t selector=word(n+652);if(selector>1 || !vector(n+752+112*selector))return fail("body-position");foundPlayer|=node==player;foundOpponent|=node==opponent;}
        previous=node;node=word(n+2276);
    }
    if(node || previous!=tail || !foundPlayer || !foundOpponent)return fail("list-membership");
    auto *r=reinterpret_cast<uint8_t*>(player);const uint32_t selector=word(r+652);
    for(unsigned o:{408u,768u,772u,776u})if(!bodyFloat(word(r+o)))return fail("velocity-factor");
    for(unsigned o:{656u,660u,664u,668u})if(!bodyFloat(word(r+o+112*selector)))return fail("angular-source");
    for(unsigned o:{816u,820u,824u,832u,836u,840u,848u,852u,856u})if(!bodyFloat(word(r+o)))return fail("matrix-source");
    reason="admitted";return true;
}
}
