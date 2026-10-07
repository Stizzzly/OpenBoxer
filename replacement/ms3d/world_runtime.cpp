#include "world_runtime.hpp"
#include "runtime_options.hpp"
#include <windows.h>
#include <cstdio>
#include <cstring>
#include <string>
namespace world {
namespace {
uintptr_t moduleBase;
Callbacks callbacks;
std::string mode;
bool fixtureMode=false;
unsigned replaced=0;
void log(const char* line) {
    if(!runtime_options::diagnostics(runtime_options::Unit::World)) return;
    FILE* f=std::fopen("C:/Users/ADMIN/Boxer-lab/ms3d/world-replacement.log","ab");
    if(f) { std::fprintf(f,"%s\n",line); std::fclose(f); }
}
int32_t __cdecl entry(int32_t selector,int32_t unused1,int32_t unused2) {
    runtime_options::Timing timing(runtime_options::Unit::World);
    char line[160]; std::sprintf(line,"WORLD-0001 enter selector=%d unused=%d/%d mode=%s",selector,unused1,unused2,mode.c_str()); log(line);
    auto original=reinterpret_cast<MapLoad>(moduleBase+0x2e970);
    if(!fixtureMode && (mode!="replace" || selector<1 || selector>7)) {
        log("WORLD-0001 route=original"); return original(selector,unused1,unused2);
    }
    ++replaced;
    const int32_t result=run({reinterpret_cast<uint8_t*>(moduleBase)},callbacks,selector);
    std::sprintf(line,"WORLD-0001 route=replacement return=%d",result); log(line); return result;
}
}
bool redirect(uintptr_t source,uintptr_t destination) {
    uint8_t jump[5]={0xe9};
    const uint32_t displacement=static_cast<uint32_t>(destination-(source+5));
    std::memcpy(jump+1,&displacement,4);
    DWORD protection=0;
    if(!VirtualProtect(reinterpret_cast<void*>(source),5,PAGE_EXECUTE_READWRITE,&protection)) return false;
    std::memcpy(reinterpret_cast<void*>(source),jump,5);
    const bool flushed=FlushInstructionCache(GetCurrentProcess(),reinterpret_cast<void*>(source),5)!=0;
    DWORD ignored=0; const bool restored=VirtualProtect(reinterpret_cast<void*>(source),5,protection,&ignored)!=0;
    return flushed && restored;
}
Callbacks dependencies(uintptr_t module) {
    Callbacks c{};
    c.allocate=reinterpret_cast<Allocate>(module+0x95420);
    c.construct=reinterpret_cast<Construct>(module+0x1956);
    c.reload=reinterpret_cast<ThisVoid>(module+0x1a64);
    c.lightA=reinterpret_cast<ThisVoid>(module+0x1690); c.lightB=reinterpret_cast<ThisVoid>(module+0x1974);
    c.stageA=reinterpret_cast<NoArg>(module+0x10e1); c.stageB=reinterpret_cast<NoArg>(module+0x127b); c.stageC=reinterpret_cast<NoArg>(module+0x1131); c.stageD=reinterpret_cast<NoArg>(module+0x1e2e);
    c.optional=reinterpret_cast<IntArg>(module+0x13d4); c.callback2000A=reinterpret_cast<IntArg>(module+0x14ec); c.callback2000B=reinterpret_cast<IntArg>(module+0x12fd);
    c.random=reinterpret_cast<IntResult>(module+0x9a750); c.final=reinterpret_cast<IntResult>(module+0x1122);
    c.holder=reinterpret_cast<Holder>(module+0x1924); c.audioLoad=reinterpret_cast<AudioLoad>(module+0x1fc8);
    c.holderDestroy=reinterpret_cast<ThisVoid>(module+0x1555); c.audioReset=reinterpret_cast<ThisVoid>(module+0x169f); c.audioPlay=reinterpret_cast<ThisVoid>(module+0x1bf9);
    c.cursor=*reinterpret_cast<Cursor*>(module+0x18cca0);
    return c;
}
bool install(uintptr_t module,const char* selected) {
    moduleBase=module; callbacks=dependencies(module); mode=selected;
    if(!redirect(module+0x10be,reinterpret_cast<uintptr_t>(&entry))) return false;
    log("WORLD-0001 installed approved synthesized thunk route"); return true;
}
void setFixtureMode(bool enabled) { fixtureMode=enabled; if(enabled) callbacks=dependencies(moduleBase); }
unsigned replacementCount() { return replaced; }
}
