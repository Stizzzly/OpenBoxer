#include "upload.hpp"
#include "world_runtime.hpp"
#include "runtime_options.hpp"
#include <cstdio>
#include <string>
namespace upload {
namespace {
uintptr_t module;
std::string mode;
bool isolated=false;
unsigned replaced=0,decoded=0,mipmaps=0,releases=0;
void log(const char* line) { if(!runtime_options::diagnostics(runtime_options::Unit::Upload))return;FILE* file=std::fopen("C:/Users/ADMIN/Boxer-lab/ms3d/upload-replacement.log","ab"); if(file) { std::fprintf(file,"%s\n",line); std::fclose(file); } }
void observe(const Observation& o) {
    ++decoded; if(o.mipmap) ++mipmaps; releases+=o.pixelReleased+o.recordReleased;
    if(!runtime_options::diagnostics(runtime_options::Unit::Upload))return;
    char line[768]; std::snprintf(line,sizeof(line),"RENDER-0002 filename=%s helperImage=%d initialPixels=%d mipmap=%d dimensions=%d/%d generatedID=%08x finalID=%08x status=%d pixelReleased=%d recordReleased=%d calls=%u decodeCalls=%u mipmapCalls=%u releaseCalls=%u",o.filename?o.filename:"(null)",o.image,o.pixels,o.mipmap,o.width,o.height,o.generated,o.result,o.status,o.pixelReleased,o.recordReleased,replaced,decoded,mipmaps,releases); log(line);
}
Callbacks dependencies() {
    return {reinterpret_cast<Decode>(module+0x1aaf),*reinterpret_cast<Generate*>(module+0x18ca5c),*reinterpret_cast<Bind*>(module+0x18ca58),*reinterpret_cast<Parameter*>(module+0x18ca54),*reinterpret_cast<Mipmap*>(module+0x18c880),reinterpret_cast<Release>(module+0x96d10),isolated?nullptr:observe};
}
uint32_t __cdecl entry(const char* filename) {
    runtime_options::Timing timing(runtime_options::Unit::Upload);
    if(!isolated && mode!="replace") return reinterpret_cast<Entry>(module+0x25420)(filename);
    ++replaced; return run(filename,dependencies());
}
}
bool install(uintptr_t base,const char* selected) { module=base; mode=selected; return world::redirect(base+0x1799,reinterpret_cast<uintptr_t>(&entry)); }
void fixtureMode(bool value) { isolated=value; }
unsigned replacementCount() { return replaced; }
}
