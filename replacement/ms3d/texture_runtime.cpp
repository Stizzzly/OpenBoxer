#include "texture.hpp"
#include "world_runtime.hpp"
#include "runtime_options.hpp"
#include <windows.h>
#include <cstdio>
#include <string>
namespace texture {
namespace {
uintptr_t module;
std::string selectedMode;
bool isolated=false;
unsigned replaced=0,uploads=0;
void log(const char* text) { if(!runtime_options::diagnostics(runtime_options::Unit::Texture))return;FILE* f=std::fopen("C:/Users/ADMIN/Boxer-lab/ms3d/texture-replacement.log","ab"); if(f) { std::fprintf(f,"%s\n",text); std::fclose(f); } }
bool readable(const void* pointer,std::size_t size) {
    uintptr_t begin=reinterpret_cast<uintptr_t>(pointer);
    if(!pointer || size>UINTPTR_MAX-begin) return false;
    const uintptr_t end=begin+size;
    while(begin<end) {
        MEMORY_BASIC_INFORMATION memory{};
        if(!VirtualQuery(reinterpret_cast<void*>(begin),&memory,sizeof(memory)) || memory.State!=MEM_COMMIT || (memory.Protect&(PAGE_GUARD|PAGE_NOACCESS))) return false;
        const uintptr_t next=reinterpret_cast<uintptr_t>(memory.BaseAddress)+memory.RegionSize;
        if(next<=begin) return false; begin=(next<end)?next:end;
    }
    return true;
}
bool supported(void* object) {
    if(!readable(static_cast<uint8_t*>(object)+countOffset,8)) return false;
    const int32_t n=count(object);
    if(n<0 || n>65535) return false;
    if(n==0) return true;
    auto* records=table(object); if(!readable(records,static_cast<std::size_t>(n)*80)) return false;
    for(int32_t i=0;i<n;i++) {
        const auto* text=*reinterpret_cast<const unsigned char**>(records+i*80+76);
        bool terminated=false;
        for(unsigned j=0;j<128;j++) { if(!readable(text+j,1)) return false; if(!text[j]) { terminated=true; break; } if(text[j]>=128) return false; }
        if(!terminated) return false;
    }
    return true;
}
void observe(int32_t index,const char* path,uint32_t oldId,uint32_t newId,bool uploaded) {
    if(uploaded) ++uploads;
    if(!runtime_options::diagnostics(runtime_options::Unit::Texture))return;
    char line[640]; std::snprintf(line,sizeof(line),"RENDER-0001 material=%d filename=%s oldID=%08x newID=%08x delegatedUpload=%d totalUploads=%u",index,path,oldId,newId,uploaded,uploads); log(line);
}
uint32_t __attribute__((thiscall)) entry(void* object) {
    runtime_options::Timing timing(runtime_options::Unit::Texture);
    auto original=reinterpret_cast<Reload>(module+0x16680);
    if(!isolated && (selectedMode!="replace" || !object || !supported(object))) { log("RENDER-0001 route=original"); return original(object); }
    ++replaced;
    const uint32_t result=run(object,{reinterpret_cast<Length>(module+0x99880),reinterpret_cast<Upload>(module+0x1799),isolated?nullptr:observe});
    char line[192]; std::snprintf(line,sizeof(line),"RENDER-0001 route=replacement return=%08x totalCalls=%u totalUploads=%u",result,replaced,uploads); log(line); return result;
}
}
bool install(uintptr_t base,const char* mode) { module=base; selectedMode=mode; return world::redirect(base+0x1a64,reinterpret_cast<uintptr_t>(&entry)); }
void fixtureMode(bool value) { isolated=value; }
unsigned replacementCount() { return replaced; }
}
