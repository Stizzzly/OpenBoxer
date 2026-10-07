#include "strike_capture.hpp"
#include "strike_fp.hpp"
#include "runtime_options.hpp"
#include <windows.h>
#include <cstdio>
namespace strike {
namespace {
uintptr_t fixtureBase;uint8_t *fixturePlayer;unsigned positionCalls=0,caseNumber=0,selectedKey=0;
class Recorder:public BinaryState {public:using BinaryState::BinaryState;Result call(const Call &c) override {return observedCall(*this,c,0);}};
uint32_t bits(float value){uint32_t out;std::memcpy(&out,&value,4);return out;}
uint32_t separation(){if(caseNumber<21)return bits(float(caseNumber%7));const uint32_t upper[]={0x40800000,0x40a00000,0x40c00000};switch(caseNumber){case 21:return 0x3f7fffff;case 22:return 0x3f800001;case 23:return upper[selectedKey]-1;case 24:return upper[selectedKey]+1;case 25:return 0x1b000000;case 26:return 0x1a000000;case 27:return 0x00800001;default:return 0x40400000;}}
Result scripted(const Call &c){
    Result r;r.eax=(caseNumber%2?0xfedc0000:0xabcd0000)|0x1234;
    auto *out=c.count?static_cast<uint8_t*>(pointer(c.args[0])):nullptr;
    switch(c.id){
    case Callback::Construct:for(unsigned i=0;i<3;++i)put(static_cast<uint8_t*>(c.owner)+4*i,c.args[i]);r.eax=address(c.owner);break;
    case Callback::Element:r.eax=address(static_cast<uint8_t*>(c.owner)+4*c.args[0]);break;
    case Callback::Position:{const unsigned pc=positionCalls++;const uint32_t x=pc==1?separation():caseNumber==27?0x00800000:0;put(out,x);put(out+4,0);put(out+8,caseNumber==28 && pc==1?0x16800000:0);put(out+12,0xabcdef01);r.eax=address(out);break;}
    case Callback::Velocity:put(out,bits(caseNumber%3==0?0.f:caseNumber%3==1?3.f:4.f));put(out+4,0);put(out+8,0);put(out+12,0xa5a5a5a5);r.eax=address(out);break;
    case Callback::Add:case Callback::Multiply:case Callback::ScalarFirstMultiply:put(out,0x3f800000);put(out+4,0);put(out+8,0);put(out+12,0xcccccccc);r.eax=address(out);break;
    case Callback::BodyVector:std::memcpy(static_cast<uint8_t*>(c.owner)+508,out,16);break;
    case Callback::Impulse:put(static_cast<uint8_t*>(c.owner)+508,0x40400000);break;
    case Callback::AudioQuery:put(pointer(c.args[2]),caseNumber%2?0x1012:0x1011);break;
    case Callback::Cursor:if(caseNumber>=14){put(fixturePlayer+140,0x3f000000);put(reinterpret_cast<void*>(fixtureBase+0x176370),0x3e800000);}break;
    default:break;
    }
    const bool scalar=c.id==Callback::Sqrt || c.id==Callback::ValueLength || c.id==Callback::Absolute || c.id==Callback::PointerLength;
    if(scalar){
        if(c.id==Callback::Sqrt){uint64_t raw=uint64_t(c.args[0])|(uint64_t(c.args[1])<<32);__asm__ volatile("fldl %0; fsqrt"::"m"(raw):"st");}
        else {uint32_t raw=c.id==Callback::Absolute?c.args[0]:c.id==Callback::ValueLength?bits(caseNumber==31?1.5f:caseNumber%2?2.f:1.f):0x3f800000;__asm__ volatile("flds %0"::"m"(raw):"st");}
    }
    __asm__ volatile("fxsave %0":"=m"(r.fp));
    if(scalar){__asm__ volatile("fstpt %0":"=m"(r.scalar80)::"st");__asm__ volatile("fxrstor %0"::"m"(r.fp));}
    return r;
}
void restoreSnapshot(uintptr_t base,void *player,const Snapshot &s){std::memcpy(player,s.player.data(),176);size_t off=0;for(const auto &span:capturedGlobals){std::memcpy(reinterpret_cast<void*>(base+span.rva),s.globals.data()+off,span.size);off+=span.size;}}
}
extern "C" uint32_t strike_dispatch(void *,uintptr_t);
extern "C" void strike_entry();
extern "C" uint32_t __attribute__((thiscall)) strike_fixture_candidate(void *owner){return strike_dispatch(owner,fixtureBase+0x2d8a2);}
struct Witness {uint32_t beforeEsp,afterEsp,ebx,esi,edi,ebp,eax;};
extern "C" void strike_invoke(uintptr_t,void *,Witness *);
extern "C" uint32_t strike_dispatch(void *,uintptr_t);
uint32_t fixtures(uintptr_t base){
    runtime_options::PreserveFp preserve;fixtureBase=base;fixturePlayer=reinterpret_cast<uint8_t*>(base+0x184910);
    Snapshot saved;snapshot(base,fixturePlayer,saved);if(!installObservers(base))return 31;scriptCallbacks(&scripted);captureLabel("fixture");
    std::array<uint8_t,2288> rb{},other{};std::array<uint8_t,11000> world{};
    FILE *report=std::fopen("C:/Users/ADMIN/Boxer-lab/ms3d/strike-fixtures.txt","wb");
    unsigned failures=0;Snapshot guardBase;
    for(unsigned key=0;key<3;++key)for(unsigned n=0;n<36;++n){caseNumber=n;selectedKey=key;
        for(unsigned candidate=0;candidate<2;++candidate){
            std::memset(fixturePlayer,0,176);for(const auto &span:capturedGlobals)std::memset(reinterpret_cast<void*>(base+span.rva),0,span.size);
            rb.fill(0);other.fill(0);world.fill(0);
            auto set=[&](uint32_t rva,uint32_t v){put(reinterpret_cast<void*>(base+rva),v);};
            *reinterpret_cast<uint8_t*>(base+0x18479d)=1;const uint32_t keys[]={0x17696e,0x17696c,0x176957};*reinterpret_cast<uint8_t*>(base+keys[key])=1;
            set(0x1762a4,0x3f9e0652);set(0x1761d4,0x42c80000);set(0x176238,0x42c80000);set(0x176370,0x3c23d70a);set(0x1761f4,n%2?0x41200000:0);if(n==29 || n==30)set(0x176370,n==29?0x00800000:0x01000000);
            *reinterpret_cast<uint8_t*>(base+0x177f9e)=n%2;fixturePlayer[168+key]=(n/7)%2;
            set(0x1849a8,address(rb.data()));set(0x184a90,address(other.data()));set(0x1853a4,address(world.data()));put(fixturePlayer+152,address(rb.data()));
            put(world.data()+10968,address(rb.data()));put(world.data()+10972,address(other.data()));put(world.data()+10976,2);
            for(auto *body:{rb.data(),other.data()}){put(body,address(body));put(body+244,address(world.data()));body[2284]=1;}
            put(rb.data()+2276,address(other.data()));put(other.data()+2280,address(rb.data()));
            const uint16_t cw=0x027f;__asm__ volatile("fninit; fldcw %0"::"m"(cw):"st");if(n>=30){alignas(16) std::array<uint8_t,512> initial{};__asm__ volatile("fxsave %0":"=m"(initial));uint16_t initialSw=0x4532;std::memcpy(initial.data()+2,&initialSw,2);__asm__ volatile("fxrstor %0"::"m"(initial):"st");}positionCalls=0;
            snapshot(base,fixturePlayer,guardBase);char reason[64];std::snprintf(reason,sizeof(reason),"isolated-key%u-case%u",key,n);beginCapture(base,fixturePlayer,reason,candidate!=0);
            unsigned supportedCalls=0,fallbackCalls=0;fixtureRouteCounters(&supportedCalls,&fallbackCalls);Witness witness{};strike_invoke(candidate?reinterpret_cast<uintptr_t>(&strike_fixture_candidate):base+0x1ef90,fixturePlayer,&witness);endCapture(witness.eax);fixtureRouteCounters(nullptr,nullptr);
            const bool route=!candidate || (supportedCalls==1 && fallbackCalls==0);if(!route)++failures;
            const bool abi=witness.beforeEsp==witness.afterEsp && witness.ebx==0x11223344 && witness.esi==0x22334455 && witness.edi==0x33445566 && witness.ebp==0x44556677;if(!abi)++failures;
            if(report)std::fprintf(report,"key=%u case=%u candidate=%u eax=%08x abi=%u esp=%08x/%08x regs=%08x/%08x/%08x/%08x\n",key,n,candidate,witness.eax,abi,witness.beforeEsp,witness.afterEsp,witness.ebx,witness.esi,witness.edi,witness.ebp);if(report)std::fprintf(report,"route key=%u case=%u candidate=%u replacement=%u fallback=%u valid=%u\n",key,n,candidate,supportedCalls,fallbackCalls,route);
        }
    }
    // Guard probes are read-only; all mutation vectors restore the same known supported input.
    captureLabel("guard");
    auto resetGuard=[&](){restoreSnapshot(base,fixturePlayer,guardBase);rb=guardBase.playerBody;other=guardBase.opponentBody;};
    for(unsigned rejection=0;rejection<24;++rejection){resetGuard();const uint16_t cw=0x027f;__asm__ volatile("fninit; fldcw %0"::"m"(cw):"st");
        auto byte=[&](uint32_t rva,uint8_t v){*reinterpret_cast<uint8_t*>(base+rva)=v;};auto value=[&](uint32_t rva,uint32_t v){put(reinterpret_cast<void*>(base+rva),v);};
        uintptr_t caller=base+0x2d8a2;
        switch(rejection){case 0:caller++;break;case 1:byte(0x17696e,1);break;case 2:byte(0x18512f,1);break;case 3:byte(0x176934,1);break;case 4:value(0x177f90,1);break;case 5:byte(0x1761c8,1);break;case 6:value(0x1761cc,0x3dcccccd);break;case 7:byte(0x1761f2,1);break;case 8:value(0x1761d4,0xbf800000);break;case 9:byte(0x1761f0,1);break;case 10:value(0x1761fc,1);break;case 11:rb[2284]=0;break;case 12:put(rb.data(),0);break;case 13:put(rb.data()+244,0);break;case 14:put(rb.data()+216,1);break;case 15:put(world.data()+10976,1);break;case 16:put(other.data()+2280,0);break;case 17:put(rb.data()+2276,address(rb.data()));break;case 18:put(rb.data()+652,2);break;case 19:put(rb.data()+768,0x7fc00000);break;case 20:{const uint16_t wrong=0x037f;__asm__ volatile("fldcw %0"::"m"(wrong));break;}case 21:value(0x176370,0x7f800000);break;case 22:value(0x176370,1);break;case 23:put(fixturePlayer+152,0);break;}
        Snapshot before,after;snapshot(base,fixturePlayer,before);const char *reason=nullptr;bool accepted=admitted(base,fixturePlayer,caller,reason);snapshot(base,fixturePlayer,after);
        bool unchanged=before.player==after.player && before.playerBody==after.playerBody && before.opponentBody==after.opponentBody && before.globals==after.globals && before.fp==after.fp;
        if(accepted || !unchanged)++failures;if(report)std::fprintf(report,"guard=%u rejected=%u unchanged=%u reason=%s\n",rejection,!accepted,unchanged,reason);
        put(world.data()+10976,2);
    }
    resetGuard();*reinterpret_cast<uint8_t*>(base+0x176957)=0;const uint16_t cw=0x027f;__asm__ volatile("fninit; fldcw %0"::"m"(cw):"st");positionCalls=0;
    unsigned replaced=0,fellBack=0;fixtureRouteCounters(&replaced,&fellBack);beginCapture(base,fixturePlayer,"isolated-key-count-fallback",false);Witness entryWitness{};strike_invoke(reinterpret_cast<uintptr_t>(&strike_entry),fixturePlayer,&entryWitness);endCapture(entryWitness.eax);
    const bool entryAbi=entryWitness.beforeEsp==entryWitness.afterEsp && entryWitness.ebx==0x11223344 && entryWitness.esi==0x22334455 && entryWitness.edi==0x33445566 && entryWitness.ebp==0x44556677;if(!entryAbi)++failures;if(report)std::fprintf(report,"entrywrapper abi=%u eax=%08x\n",entryAbi,entryWitness.eax);fixtureRouteCounters(nullptr,nullptr);
    if(replaced || fellBack!=1)++failures;if(report)std::fprintf(report,"fallback replacement=%u original=%u exactlyonce=%u\n",replaced,fellBack,!replaced && fellBack==1);
    if(report)std::fclose(report);scriptCallbacks(nullptr);restoreObservers(base);restoreSnapshot(base,fixturePlayer,saved);captureLabel("native");return failures;
}

}
