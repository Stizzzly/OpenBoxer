#pragma once
#include "frames.hpp"
#include "animation_layout.hpp"
namespace frames {
using animation::word; using animation::put; using animation::Lookup; using animation::Entry;
extern "C" uint32_t frames_call(Entry,void *,uint32_t opaqueECX);
struct Dependencies { Lookup lookup; Entry advancement; };
class BinaryState final:public State {
    uint8_t *model_; Dependencies dependencies_;
public:
    BinaryState(void *m,Dependencies d):model_(static_cast<uint8_t*>(m)),dependencies_(d){}
    int32_t selector() const override { return int32_t(word(model_+44)); }
    void *lookup(int32_t i) override { return dependencies_.lookup(model_+84,i); }
    int32_t gate() const override { return int32_t(word(model_+40)); }
    int32_t recordStart(void *r) const override { return int32_t(word(static_cast<uint8_t*>(r)+256)); }
    int32_t divisor(void *r) const override { return int32_t(word(static_cast<uint8_t*>(r)+260)); }
    uint32_t frame() const override { return word(model_+48); }
    uint32_t slot(unsigned offset) const override { return word(model_+offset); }
    void slot(unsigned offset,uint32_t value) override { put(model_+offset,value); }
    uint32_t advance(uint32_t opaque) override { return frames_call(dependencies_.advancement,model_,opaque); }
};
}
