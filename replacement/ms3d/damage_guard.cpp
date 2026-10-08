#include "damage_native.hpp"
#include "damage_fp.hpp"
#include "runtime_options.hpp"
#include <windows.h>
#include <array>
#include <initializer_list>
namespace damage {
extern "C" bool damage_guard_check(uintptr_t base,void *actor,int32_t mode,uintptr_t caller,const char **reason){return admitted(base,actor,mode,caller,*reason);}
namespace {
bool writable(uintptr_t a,size_t n){if(!a || a+n<a)return false;const uintptr_t end=a+n;while(a<end){MEMORY_BASIC_INFORMATION m{};if(!VirtualQuery(reinterpret_cast<void*>(a),&m,sizeof(m)) || m.State!=MEM_COMMIT || m.Protect&(PAGE_GUARD|PAGE_NOACCESS))return false;const DWORD k=m.Protect&0xff;if(k!=PAGE_READWRITE && k!=PAGE_WRITECOPY && k!=PAGE_EXECUTE_READWRITE && k!=PAGE_EXECUTE_WRITECOPY)return false;uintptr_t next=uintptr_t(m.BaseAddress)+m.RegionSize;if(next<=a)return false;a=next;}return true;}
bool finite(uint32_t raw){uint32_t mag=raw&0x7fffffff;return !mag || (mag>=0x00800000 && mag<=0x461c4000);}
bool nonnegative(uint32_t raw){return !(raw&0x80000000) || !(raw&0x7fffffff);}
bool positive(uint32_t raw){return finite(raw) && raw && !(raw&0x80000000);}
bool overlap(uintptr_t a,size_t na,uintptr_t b,size_t nb){return a<b+nb && b<a+na;}
constexpr std::array<std::array<uint32_t,2>,18> spans={{{0x1761c8,200},{0x1762a0,128},{0x176370,4},{0x175eec,4},{0x184778,80},{0x185594,4},{0x177f90,16},{0x1849a8,4},{0x184a90,4},{0x1853a4,4},{0x1848e0,16},{0x184ae0,16},{0x184c98,16},{0x17f710,7600},{0x171b90,4},{0x175df0,28},{0x184910,176},{0x1849f8,232}}};
}
bool admitted(uintptr_t base,void *object,int32_t mode,uintptr_t caller,const char *&reason){
    PreserveFp preserve;auto fail=[&](const char*r){reason=r;return false;};
    if(base!=0x400000 || caller!=base+0x2d8eb || uintptr_t(object)!=base+0x1849f8)return fail("module-caller-actor");
    for(auto span:spans)if(!writable(base+span[0],span[1]))return fail("static-range");
    auto *a=static_cast<uint8_t*>(object);auto w=[&](uint32_t rva){return word(pointer(uint32_t(base+rva)));};auto b=[&](uint32_t rva){return *static_cast<uint8_t*>(pointer(uint32_t(base+rva)));};
    alignas(16) std::array<uint8_t,512> env{};__asm__ volatile("fxsave %0":"=m"(env));uint16_t cw,sw;std::memcpy(&cw,env.data(),2);std::memcpy(&sw,env.data()+2,2);if(cw!=0x027f || (sw&0x3840) || env[4])return fail("fp-entry");
    if(mode<0 || mode>1 || w(0x184790)!=uint32_t(mode))return fail("mode");if(b(0x184794) || !b(0x18479d) || w(0x177f90))return fail("inactive-menu");
    if(mode && (w(0x1847c4)<1 || w(0x1847c4)>5))return fail("difficulty");const uint32_t playerIndex=w(0x184784),aiIndex=w(0x184788);if(playerIndex>7 || aiIndex<1 || aiIndex>7)return fail("fighter-index");
    if(b(0x1761c9)!=1 || w(0x1761f8)!=1)return fail("ordinary-Z");for(uint32_t off:{0x17622du,0x17622eu,0x176256u,0x176254u,0x17626cu})if(b(off))return fail("excluded-state");if(w(0x176270))return fail("combo-count");
    for(uint32_t off:{0x17622cu,0x176234u,0x17628cu})if(b(off)>1)return fail("global-bool");for(unsigned off:{171u,176u,177u,178u,204u})if(a[off]>1)return fail("actor-bool");if(word(a+208)>1 || word(a+216)<1 || word(a+216)>3 || word(a+164)>11 || word(a+164)==10)return fail("actor-state");
    for(unsigned off:{28u,32u,36u,44u,48u,52u,108u,112u,116u,140u,180u,184u,188u,192u,196u,228u})if(!finite(word(a+off)))return fail("actor-scalars");
    for(uint32_t off:{0x1761ccu,0x1761d4u,0x176238u,0x176258u,0x176260u,0x176370u})if(!finite(w(off)))return fail("global-scalars");
    if(!nonnegative(w(0x1761d4)) || (!(w(0x176258)&0x80000000) && (w(0x176258)&0x7fffffff)>=0x42c60000) || !nonnegative(w(0x176370)))return fail("health-fatigue-dt");
    if(fp::greater(fp::compare(w(0x176260),0x3f19999a)))return fail("combo-timer");
    const uint32_t playerFactor=w(0x1762a0+16*playerIndex),duration=w(0x1762a4+16*playerIndex),aiFactor=w(0x1762a8+16*aiIndex);
    if(!positive(playerFactor)||!positive(duration)||!positive(aiFactor))return fail("fighter-factors");
    const uint32_t finalHealth=fp::damage(w(0x176238),0x40000000,playerFactor,aiFactor);if(!(finalHealth&0x7fffffff) || finalHealth&0x80000000 || (finalHealth&0x7f800000)==0x7f800000)return fail("unsafe-health");
    const uintptr_t player=w(0x1849a8),opponent=w(0x184a90),world=w(0x1853a4);
    if(!world || player==opponent || !writable(player,2288) || !writable(opponent,2288) || !writable(world+10968,12) || word(a+152)!=opponent || w(0x184910+152)!=player)return fail("body-ranges-owner");
    if(overlap(player,2288,opponent,2288) || overlap(player,2288,world+10968,12) || overlap(opponent,2288,world+10968,12))return fail("body-world-alias");
    for(auto span:spans)if(overlap(player,2288,base+span[0],span[1]) || overlap(opponent,2288,base+span[0],span[1]) || overlap(world+10968,12,base+span[0],span[1]))return fail("static-body-alias");
    if(word(pointer(uint32_t(world+10976)))!=2)return fail("list-count");const uintptr_t head=word(pointer(uint32_t(world+10968))),tail=word(pointer(uint32_t(world+10972)));if(!((head==player && tail==opponent)||(head==opponent && tail==player)))return fail("list-members");
    for(uintptr_t body:{player,opponent}){auto *r=static_cast<uint8_t*>(pointer(uint32_t(body)));if(word(r)!=body || word(r+244)!=world || r[2284]!=1 || word(r+652) || word(r+216))return fail("body-lifetime");if(word(r+2280)!=(body==head?0:head) || word(r+2276)!=(body==tail?0:tail))return fail("list-links");if(body==player){for(unsigned off:{752u,756u,760u})if(!finite(word(r+off)))return fail("player-position");}else for(unsigned off:{408u,656u,660u,664u,668u,752u,756u,760u,768u,772u,776u,816u,820u,824u,832u,836u,840u,848u,852u,856u})if(!finite(word(r+off)))return fail("opponent-scalars");}
    if(!b(0x17622c)){auto *p=static_cast<uint8_t*>(pointer(uint32_t(player))),*o=static_cast<uint8_t*>(pointer(uint32_t(opponent)));uint32_t xd=fp::difference(word(p+752),word(o+752)),xp=fp::differenceProduct(word(p+752),word(o+752),xd),zd=fp::difference(word(p+760),word(o+760));const uint64_t sum=fp::differenceSum(word(p+760),word(o+760),zd,xp);uint32_t distance;__asm__ volatile("fldl %1; fsqrt; fstps %0":"=m"(distance):"m"(sum):"st");if(fp::less(fp::compare(distance,0x40c00000)))return fail("near-pending-zero");}
    reason="admitted";return true;
}
}
