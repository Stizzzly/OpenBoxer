#pragma once
#include "strike.hpp"
#include <cstddef>
namespace strike {
static_assert(offsetof(Result,fp)==16 && offsetof(Result,scalar80)==4);
extern "C" void strike_bridge(uintptr_t,uint32_t,const uint32_t*,unsigned,unsigned,Result*,unsigned);
inline uintptr_t originalAudio=0;
inline constexpr uint32_t dependencyRva[]={0x1839,0x19c9,0x5ac10,0x5b780,0x1d4d,0x1811,0x5b2e0,0x5b210,0x1014,0x11e0,0x1cd0,0x1f64,0x5b690,0x1b36,0x18cc00,0x179e,0x1eb0,0x95b74,0x1f5f};
class BinaryState:public State {
    uintptr_t base_;uint8_t *player_;
public:
    BinaryState(uintptr_t base,void *p):base_(base),player_(static_cast<uint8_t*>(p)){}
    uint8_t *player() override{return player_;}
    uint8_t *global(Global g) override{return reinterpret_cast<uint8_t*>(base_+uint32_t(g));}
    Result call(const Call &c) override {
        Result r;uintptr_t target=base_+dependencyRva[unsigned(c.id)];
        if(c.id==Callback::AudioQuery)target=originalAudio?originalAudio:word(reinterpret_cast<void*>(target));
        strike_bridge(target,address(c.owner),c.args.data(),c.count,c.callerCleanup,&r,c.scalar);return r;
    }
};
bool admitted(uintptr_t,void *,uintptr_t,const char *&);
}
