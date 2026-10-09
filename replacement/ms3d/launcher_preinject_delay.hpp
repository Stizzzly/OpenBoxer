#pragma once
#include <stdexcept>

namespace launcher_policy {
inline unsigned preinjectObserveDelay(const char* text) {
    if(!text || !*text)return 0;
    unsigned delay=0;
    for(const char* digit=text;*digit;++digit) {
        if(*digit<'0' || *digit>'9')
            throw std::invalid_argument("OPENBOXER_PREINJECT_OBSERVE_DELAY_MS must be unsigned decimal");
        // Saturate before multiplication, so arbitrary-length valid input
        // cannot overflow. Continue parsing after saturation to reject junk.
        if(delay<15000) {
            delay=delay*10+unsigned(*digit-'0');
            if(delay>15000)delay=15000;
        }
    }
    return delay;
}
}
