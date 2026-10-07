#include "clip.hpp"
namespace clip {
uint32_t select(State &state) {
    uint32_t index=0, result=0xcccccccc;
    while(static_cast<int32_t>(index)<state.bound()) {
        void *record=state.lookup(static_cast<int32_t>(index));
        if(state.compare(record)==0) {
            state.selector(static_cast<int32_t>(index));
            record=state.lookup(state.selector());
            state.frame(state.start(record));
            state.factor(0x3dcccccd);
            state.anchor(state.clock());
            return state.owner();
        }
        result=++index;
    }
    return result;
}
}
