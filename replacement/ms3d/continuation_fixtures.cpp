#include "damage_capture.hpp"
#include "damage_fixture_script.hpp"
#include "continuation_fixture_metadata.hpp"
#include "runtime_options.hpp"
#include <cstdio>
namespace damage {
namespace {
fixture::Profile *profile=nullptr;Result scripted(const Call&c){return profile->call(c);}uintptr_t fixtureBase=0;
bool same(const Snapshot&a,const Snapshot&b){return a.actor==b.actor && a.player==b.player && a.playerBody==b.playerBody && a.opponentBody==b.opponentBody && a.globals==b.globals && a.world==b.world && a.playerAddress==b.playerAddress && a.opponentAddress==b.opponentAddress && a.worldAddress==b.worldAddress && a.spanSizes==b.spanSizes;}
bool abi(const Witness&w){return w.beforeEsp==w.afterEsp && w.ebx==0x11223344 && w.esi==0x22334455 && w.edi==0x33445566 && w.ebp==0x44556677;}
}
extern "C" uint32_t damage_dispatch(void *,uintptr_t,int32_t);
extern "C" uint32_t __attribute__((thiscall)) continuation_damage_fixture_candidate(void *actor,int32_t mode){return damage_dispatch(actor,fixtureBase+0x2d8eb,mode);}
extern "C" uint32_t __attribute__((thiscall)) continuation_damage_wrong_caller_candidate(void *actor,int32_t mode){return damage_dispatch(actor,fixtureBase+0x2d8ec,mode);}
uint32_t continuationFixtures(uintptr_t base){
    PreserveFp preserve;fixtureBase=base;auto saved=captureSnapshot(base);
    if(!installObserver(base,true,true))return 31;
    std::array<uint8_t,2288> playerBody{},opponentBody{};std::array<uint8_t,11000> world{};
    fixture::Profile state{reinterpret_cast<uint8_t*>(base+0x1849f8),reinterpret_cast<uint8_t*>(base+0x184910),playerBody.data(),opponentBody.data(),world.data(),reinterpret_cast<uint8_t*>(base)};
    profile=&state;state.patternXmm=true;setScript(&scripted);forceFixtureReplacement(false);forceFixtureContinuationReplacement(true);setReplacementEnabled(false);
    FILE *report=std::fopen("C:/Users/ADMIN/Boxer-lab/ms3d/ai-continuation-fixtures.txt","wb");unsigned failures=0;
    alignas(16) std::array<uint8_t,544> entryFp{};const uint16_t entryCw=0x027f;__asm__ volatile("fninit; fldcw %1; fxsave (%0); fnstenv 512(%0); fldenv 512(%0)"::"r"(entryFp.data()),"m"(entryCw):"memory","st");
    for(unsigned i=0;i<128;++i)entryFp[160+i]=uint8_t(0x31+i*7);
    for(unsigned n=0;n<sizeof(continuationCases)/sizeof(*continuationCases);++n){auto &test=continuationCases[n];for(unsigned candidate=0;candidate<2;++candidate){
        state.initialize(test);__asm__ volatile("fxrstor %0"::"m"(entryFp):"memory","st");
        const char *reason=nullptr;if(!admittedContinuation(base,state.actor,int32_t(state.mode),base+0x2d8eb,reason)){++failures;if(report)std::fprintf(report,"case=%u name=%s preflight=0 reason=%s\n",n,test.name,reason);continue;}
        beginManualCapture(base,int32_t(state.mode),candidate?"ai-continuation-fixture-candidate":"ai-continuation-fixture-original",candidate!=0);unsigned replaced=0,fallback=0;routeCounters(&replaced,&fallback);
        Witness witness{};damage_invoke(candidate?uintptr_t(&continuation_damage_fixture_candidate):base+0x1c4e0,state.actor,int32_t(state.mode),&witness,entryFp.data());
        endManualCapture(witness.eax,&witness);routeCounters(nullptr,nullptr);const bool route=!candidate || (replaced==1 && fallback==0);if(!route)++failures;
        const bool abi=witness.beforeEsp==witness.afterEsp && witness.ebx==0x11223344 && witness.esi==0x22334455 && witness.edi==0x33445566 && witness.ebp==0x44556677;if(!abi)++failures;
        if(report)std::fprintf(report,"case=%u name=%s candidate=%u eax=%08x events=%u replacement=%u fallback=%u route=%u abi=%u esp=%08x/%08x regs=%08x/%08x/%08x/%08x\n",n,test.name,candidate,witness.eax,unsigned(state.sites.size()),replaced,fallback,route,abi,witness.beforeEsp,witness.afterEsp,witness.ebx,witness.esi,witness.edi,witness.ebp);
    }}
    // Rejections are probed without executing an unsupported original input.
    // Both state and the full saved FP image must survive this read-only guard.
    const char *names[]={"valid-control","wrong-caller","wrong-actor","wrong-mode","pending1-control","attack0","recoil","lock","death","victory","combo-active","combo-count","unsupported-CW","nonfinite","completion-control","world-count","selector","graph","self","link","actor-bool","block2","combo-above","combo-subnormal","alternator2","player-attack","marker","combo-upper-control","AIpending","block1","menu","inactive","type0","type4","state-mismatch","negative-timer","NaN-timer","zero-duration","NaN-duration","NaN-factor-completion","NaN-factor-unused","factor-subnormal","negative-health","zero-health","dt-negative","dt-too-large","pending2","timer-max-control","mode0-control"};
    for(unsigned test=0;test<sizeof(names)/sizeof(*names);++test){
        state.initialize(continuationCases[0]);uintptr_t caller=base+0x2d8eb;void *actor=state.actor;int32_t mode=int32_t(state.mode);uint16_t cw=0x027f;
        switch(test){
        case 1:++caller;break;case 2:actor=state.player;break;case 3:mode=0;break;case 4:*state.global(0x1761c8)=1;break;
        case 5:*state.global(0x17622d)=0;break;case 6:*state.global(0x17622e)=1;break;case 7:*state.global(0x176256)=1;break;
        case 8:*state.global(0x184794)=1;break;case 9:*state.global(0x176254)=1;break;case 10:*state.global(0x17626c)=1;break;
        case 11:state.set(0x176270,1);break;case 12:cw=0x037f;break;case 13:put(state.actor+140,0x7fc00000);break;
        case 14:state.set(0x176230,0x40000001);break;case 15:put(state.world+10976,3);break;
        case 16:put(state.opponentBody+652,1);break;case 17:put(state.opponentBody+216,1);break;
        case 18:put(state.opponentBody,0);break;case 19:put(state.opponentBody+2280,0);break;
        case 20:state.actor[204]=2;break;case 21:*state.global(0x176234)=2;break;
        case 22:state.set(0x176260,0x3fccccce);break;case 23:state.set(0x176260,1);break;case 24:state.actor[168]=2;break;case 25:*state.global(0x1761c9)=1;break;case 26:*state.global(0x17628c)=1;break;case 27:state.set(0x176260,0x3fcccccd);break;
        case 28:*state.global(0x17622c)=1;break;case 29:*state.global(0x176234)=1;break;case 30:state.set(0x177f90,1);break;case 31:*state.global(0x18479d)=0;break;
        case 32:state.set(0x17625c,0);break;case 33:state.set(0x17625c,4);break;case 34:put(state.actor+164,5);break;
        case 35:state.set(0x176230,0xbf800000);break;case 36:state.set(0x176230,0x7fc00000);break;case 37:state.set(0x1762b4,0);break;case 38:state.set(0x1762b4,0x7fc00000);break;
        case 39:state.set(0x176230,0x40000001);state.set(0x1762bc,0x7fc00000);break;case 40:state.set(0x1762bc,0x7fc00000);break;
        case 41:state.set(0x176230,0x40000001);state.set(0x1762bc,1);break;case 42:state.set(0x1761d4,0xbf800000);break;case 43:state.set(0x176238,0);break;
        case 44:state.set(0x176370,0xbf800000);break;case 45:state.set(0x176370,0x461c4001);break;case 46:*state.global(0x1761c8)=2;break;case 47:state.set(0x176230,0x461c4000);break;case 48:mode=0;state.set(0x184790,0);break;
        }
        auto before=captureSnapshot(base);const char *reason=nullptr;GuardWitness fp;
        __asm__ volatile("fxrstor (%0); fldenv 512(%0); fldcw %1"::"r"(entryFp.data()),"m"(cw):"memory","st");damage_continuation_guard_probe(base,actor,mode,caller,&reason,&fp);auto after=captureSnapshot(base);
        const bool readOnly=same(before,after),exactFp=std::memcmp(fp.before.data(),fp.after.data(),540)==0,noCallbacks=state.sites.empty();
        const bool accepted=fp.supported!=0,expected=test==0 || test==4 || test==14 || test==27 || test==40 || test==47 || test==48;const bool pass=accepted==expected && readOnly && exactFp && noCallbacks;if(!pass)++failures;
        if(report)std::fprintf(report,"guard=%u name=%s accepted=%u expected=%u read_only=%u full_fp_exact=%u no_callbacks=%u pass=%u reason=%s\n",test,names[test],accepted,expected,readOnly,exactFp,noCallbacks,pass,reason?reason:"missing");
    }
    // Safely exercise the actual dispatcher fallback once with otherwise valid
    // wait input. Original reference and dispatch share identical typed scripts.
    Snapshot reference;std::vector<uint32_t> referenceSites;uint32_t referenceEax=0;
    for(unsigned candidate=0;candidate<2;++candidate){
        state.initialize(continuationCases[0]);__asm__ volatile("fxrstor %0"::"m"(entryFp):"memory","st");
        beginManualCapture(base,int32_t(state.mode),candidate?"ai-continuation-guard-candidate":"ai-continuation-guard-reference",candidate!=0);unsigned replaced=0,fallback=0;routeCounters(&replaced,&fallback);Witness witness{};
        damage_invoke(candidate?uintptr_t(&continuation_damage_wrong_caller_candidate):base+0x1c4e0,state.actor,int32_t(state.mode),&witness,entryFp.data());endManualCapture(witness.eax,&witness);routeCounters(nullptr,nullptr);
        auto result=captureSnapshot(base);bool equivalent=true;if(!candidate){reference=result;referenceSites=state.sites;referenceEax=witness.eax;}else equivalent=same(reference,result) && referenceSites==state.sites && referenceEax==witness.eax;
        const bool route=candidate?(replaced==0 && fallback==1):(!replaced && !fallback),validAbi=abi(witness);if(!route || !validAbi || !equivalent)++failures;
        if(report)std::fprintf(report,"guard_dispatch candidate=%u replacement=%u fallback=%u route=%u abi=%u equivalent_state_sites_eax=%u events=%u eax=%08x\n",candidate,replaced,fallback,route,validAbi,equivalent,unsigned(state.sites.size()),witness.eax);
    }
    setScript(nullptr);profile=nullptr;forceFixtureContinuationReplacement(false);const bool restored=restoreObserver();if(!restored)++failures;restoreSnapshot(base,saved);if(report){std::fprintf(report,"cleanup observer_restored=%u failures=%u\n",restored,failures);std::fclose(report);}return failures;
}
}
