#pragma once
#include "clip.hpp"
#include "animation_layout.hpp"
namespace clip {
using animation::word;
using animation::put;
using Lookup=void *(__attribute__((thiscall)) *)(void *,int32_t);
using Compare=int32_t(__attribute__((cdecl)) *)(const char *,const char *);
using Entry=uint32_t(__attribute__((thiscall)) *)(void *,const char *);
struct Dependencies { Lookup lookup; Compare compare; const void *clock; };
class BinaryState final:public State {
    uint8_t *owner_, *model_;
    const char *request_;
    Dependencies dependencies_;
public:
    BinaryState(void *owner,const char *request,Dependencies d):owner_(static_cast<uint8_t*>(owner)),model_(owner_+644),request_(request),dependencies_(d){}
    int32_t bound() const override { return int32_t(word(model_+40)); }
    void *lookup(int32_t i) override { return dependencies_.lookup(model_+84,i); }
    int32_t compare(void *record) override { return dependencies_.compare(static_cast<const char*>(record),request_); }
    void selector(int32_t i) override { put(model_+44,uint32_t(i)); }
    int32_t selector() const override { return int32_t(word(model_+44)); }
    uint32_t start(void *record) const override { return word(static_cast<uint8_t*>(record)+256); }
    void frame(uint32_t value) override { put(model_+48,value); }
    void factor(uint32_t value) override { put(model_+76,value); }
    uint32_t clock() const override { return word(dependencies_.clock); }
    void anchor(uint32_t value) override { put(model_+80,value); }
    uint32_t owner() const override { return uint32_t(reinterpret_cast<uintptr_t>(owner_)); }
};
}
