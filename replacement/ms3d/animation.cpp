#include "animation.hpp"
#include "animation_fp.hpp"
namespace animation {
Progress predict(uint32_t elapsed, int32_t rate) {
    Progress p{0, fp::divide(elapsed, fp::interval(rate))};
    if (!fp::within(fp::interval(rate), elapsed)) return p;
    uint32_t residual=elapsed;
    do {
        residual=fp::subtract(residual, fp::interval(rate));
        ++p.steps;
    } while (fp::within(fp::interval(rate), residual) && p.steps<=10);
    return p;
}
uint32_t advance(State &state) {
    if (state.count()==0) return 0;
    const uint32_t now=state.clock();
    const uint32_t elapsed=fp::elapsed(now,state.anchor());
    const int32_t index=state.selector();
    const int32_t rate=state.lookupRate(index);
    const Progress p=predict(elapsed,rate);
    uint32_t factor=p.initialFactor;
    if(p.steps) {
        state.frame(state.source(p.steps));
        state.anchor(fp::moveAnchor(state.anchor(),fp::interval(rate),int32_t(p.steps)));
        factor=fp::subtract(factor,double(p.steps));
        if(fp::negative(factor)) factor=0;
    }
    state.factor(factor);
    return factor;
}
}
