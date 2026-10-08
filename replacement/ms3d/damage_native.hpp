#pragma once
#include "damage.hpp"
#include <cstddef>
namespace damage {
bool admitted(uintptr_t base,void *actor,int32_t mode,uintptr_t caller,const char *&reason);
extern "C" void damage_bridge(uintptr_t,uint32_t,const uint32_t *,unsigned,unsigned,Result *,unsigned,const void *inputXmm=nullptr);
class BinaryState:public State {
    uintptr_t base_;uint8_t *actor_;
public:
    BinaryState(uintptr_t base,void *actor):base_(base),actor_(static_cast<uint8_t*>(actor)){}
    uint8_t *actor() override{return actor_;}
    uint8_t *global(uint32_t rva) override{return reinterpret_cast<uint8_t*>(base_+rva);}
    Result call(const Call &) override;
};
bool install(uintptr_t base,const char *mode);
uint32_t fixtures(uintptr_t base);
uint32_t replays(uintptr_t base);
void routeCounters(unsigned *,unsigned *);
void forceFixtureReplacement(bool);
struct Witness {uint32_t beforeEsp,afterEsp,ebx,esi,edi,ebp,eax;alignas(16) std::array<uint8_t,544> entryFp{},exitFp{};};
extern "C" void damage_invoke(uintptr_t,void*,int32_t,Witness*,const void*entryFp=nullptr);
struct GuardWitness {alignas(16) std::array<uint8_t,544> before{},after{};uint32_t supported=0;};
extern "C" void damage_guard_probe(uintptr_t,void*,int32_t,uintptr_t,const char**,GuardWitness*);
static_assert(offsetof(Result,fp)==16,"bridge FP ABI");
static_assert(offsetof(Witness,entryFp)==32 && offsetof(Witness,exitFp)==576,"raw invocation FP ABI");
static_assert(offsetof(GuardWitness,after)==544 && offsetof(GuardWitness,supported)==1088,"guard probe ABI");
}
