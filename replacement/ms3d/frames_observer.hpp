#pragma once
#include <windows.h>
#include <cstdint>
#include <cstring>
namespace frames {
inline bool advancementCall(uintptr_t base,uintptr_t target) {
    uint8_t call[5]={0xe8}; const uint32_t relative=uint32_t(target-(base+0x5e4d)); std::memcpy(call+1,&relative,4);
    auto *site=reinterpret_cast<void*>(base+0x5e48); DWORD old=0,unused=0;
    if(!VirtualProtect(site,5,PAGE_EXECUTE_READWRITE,&old))return false;
    std::memcpy(site,call,5); const bool flushed=FlushInstructionCache(GetCurrentProcess(),site,5)!=0;
    return VirtualProtect(site,5,old,&unused)!=0 && flushed;
}
}
