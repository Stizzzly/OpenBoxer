#pragma once
#include <stdexcept>
namespace launcher_policy {
inline unsigned workerObserveDelay(const char*text){
 if(!text || !*text)return 0;
 unsigned delay=0;
 for(const char*p=text;*p;++p){if(*p<'0' || *p>'9')throw std::invalid_argument("OPENBOXER_WORKER_OBSERVE_DELAY_MS must be unsigned decimal");if(delay<2000){delay=delay*10+unsigned(*p-'0');if(delay>2000)delay=2000;}}
 return delay;
}
}
