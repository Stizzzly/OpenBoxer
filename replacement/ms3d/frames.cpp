#include "frames.hpp"
namespace frames {
namespace {
int32_t remainder(uint32_t frame,int32_t divisor) {
    const uint32_t wrapped=frame+1u;
    const int64_t dividend=wrapped<=0x7fffffffu?int64_t(wrapped):int64_t(wrapped)-0x100000000ll;
    // Widened truncation remainder is explicit for all finite admitted inputs.
    // Native guards exclude both original divide traps before unit effects.
    return int32_t(dividend/int64_t(divisor)*-int64_t(divisor)+dividend);
}
void writeNext(State &state,unsigned offset,uint32_t frame,int32_t divisor,int32_t start) {
    state.slot(offset,uint32_t(remainder(frame,divisor)));
    if(state.slot(offset)==0)state.slot(offset,uint32_t(start));
}
}
uint32_t next(uint32_t frame,int32_t divisor,int32_t start) {
    const int32_t value=remainder(frame,divisor);
    return uint32_t(value==0?start:value);
}
uint32_t prepare(State &state,uint32_t opaqueECX) {
    int32_t start=0,divisor=1;
    void *record=state.lookup(state.selector());
    if(state.gate()!=0) { start=state.recordStart(record); divisor=state.divisor(record); }
    writeNext(state,52,state.frame(),divisor,start);
    state.slot(60,state.slot(52));
    writeNext(state,64,state.slot(60),divisor,start);
    writeNext(state,68,state.slot(64),divisor,start);
    writeNext(state,72,state.slot(68),divisor,start);
    return state.advance(opaqueECX);
}
}
