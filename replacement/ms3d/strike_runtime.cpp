#include "strike_layout.hpp"
#include "strike_capture.hpp"
#include "world_runtime.hpp"
#include "runtime_options.hpp"
#include <windows.h>
#include <cstdio>
#include <cstring>
namespace strike {
namespace {uintptr_t module=0;bool replacement=false,capture=false;unsigned counts[3]{},rejections[3]{};unsigned *replacementCounter=nullptr,*fallbackCounter=nullptr;
class CapturedState:public BinaryState{public:using BinaryState::BinaryState;Result call(const Call &c) override{return observedCall(*this,c,0);}Result originalCall(const Call &c){return BinaryState::call(c);}};
}
extern "C" void strike_entry();
extern "C" uint32_t strike_dispatch(void *p,uintptr_t caller){
    const char *reason=nullptr;const bool supported=admitted(module,p,caller,reason);
    bool recording=false;
    if(capture && !capturing() && caller==module+0x2d8a2 && address(p)==module+0x184910){const uint32_t keys[]={0x17696e,0x17696c,0x176957},latches[]={0x185146,0x185144,0x18512f};unsigned selected=3,active=0;for(unsigned i=0;i<3;++i)if(*reinterpret_cast<uint8_t*>(module+keys[i])){selected=i;++active;}if(active==1 && !*reinterpret_cast<uint8_t*>(module+latches[selected]) && (supported?counts[selected]<2:rejections[selected]<3)){if(supported)++counts[selected];else ++rejections[selected];recording=true;beginCapture(module,p,reason,supported&&replacement);}}
    uint32_t eax;
    if(supported && replacement){if(replacementCounter)++*replacementCounter;CapturedState state(module,p);eax=update(state);}
    else {if(fallbackCounter)++*fallbackCounter;Result r;strike_bridge(module+0x1ef90,address(p),nullptr,0,0,&r,0);eax=r.eax;}
    if(recording)endCapture(eax);
    if(capture && counts[0]>=2 && counts[1]>=2 && counts[2]>=2){restoreObservers(module);capture=false;}
    return eax;
}
void fixtureRouteCounters(unsigned *supported,unsigned *fallback){replacementCounter=supported;fallbackCounter=fallback;}
bool install(uintptr_t base,const char *mode){if(base!=0x400000)return false;const auto *entry=reinterpret_cast<const uint8_t*>(base+0x1bef),*caller=reinterpret_cast<const uint8_t*>(base+0x2d89d);if(entry[0]!=0xe9 || uintptr_t(base+0x1bef+5+int32_t(word(entry+1)))!=base+0x1ef90 || caller[0]!=0xe8 || uintptr_t(base+0x2d89d+5+int32_t(word(caller+1)))!=base+0x1bef)return false;module=base;replacement=std::strcmp(mode,"replace")==0;capture=runtime_options::capture(runtime_options::Unit::Strike);if(capture && !installObservers(base))return false;return world::redirect(base+0x1bef,reinterpret_cast<uintptr_t>(&strike_entry));}

}
