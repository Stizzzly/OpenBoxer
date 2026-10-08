#include "damage_capture.hpp"
#include <vector>
#include <cstdio>
int main(){
    // Fresh process: no observer installation, capture, or snapshot initializes
    // the observer's private module/audio fields.
    std::vector<uint8_t> image(0x18cc04,0);
    const uintptr_t base=reinterpret_cast<uintptr_t>(image.data());
    damage::put(image.data()+0x18cc00,0x10002aa0);
    if(damage::resolveAudioTarget(0,base)!=0x10002aa0)return 1;
    damage::put(image.data()+0x18cc00,0x10003000);
    if(damage::resolveAudioTarget(0,base)!=0x10003000)return 2;
    if(damage::resolveAudioTarget(0x10002aa0,0)!=0x10002aa0)return 3;
    std::puts("damage uncaptured audio uses explicit live game base PASS");
    return 0;
}
