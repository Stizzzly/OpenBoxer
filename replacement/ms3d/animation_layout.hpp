#pragma once
#include "animation.hpp"
#include <cstring>
namespace animation {
inline uint32_t word(const void *p) { uint32_t v; std::memcpy(&v,p,4); return v; }
inline void put(void *p,uint32_t v) { std::memcpy(p,&v,4); }
using Count = int32_t(__attribute__((thiscall)) *)(void *);
using Lookup = void *(__attribute__((thiscall)) *)(void *,int32_t);
using Entry = uint32_t(__attribute__((stdcall)) *)(void *);
struct Dependencies { Count count; Lookup lookup; const void *clock; };
class BinaryState final:public State {
    uint8_t *model_; Dependencies dependencies_;
public:
    BinaryState(void *m,Dependencies d):model_(static_cast<uint8_t*>(m)),dependencies_(d){}
    int32_t count() override { return dependencies_.count(model_+84); }
    uint32_t clock() const override { return word(dependencies_.clock); }
    uint32_t anchor() const override { return word(model_+80); }
    int32_t selector() const override { return int32_t(word(model_+44)); }
    int32_t lookupRate(int32_t i) override { return int32_t(word(static_cast<uint8_t*>(dependencies_.lookup(model_+84,i))+268)); }
    uint32_t source(unsigned steps) const override { return word(model_+56+4*steps); }
    void frame(uint32_t v) override { put(model_+48,v); }
    void anchor(uint32_t v) override { put(model_+80,v); }
    void factor(uint32_t v) override { put(model_+76,v); }
};
}
