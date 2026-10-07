#include "lighting.hpp"
#include "world_runtime.hpp"
#include "runtime_options.hpp"
#include <windows.h>
#include <vector>
#include <string>
#include <cstdio>
namespace lighting {
namespace {
uintptr_t module;
std::string mode;
bool isolated=false;
unsigned callsA=0,callsB=0;
void log(const char* text) { if(!runtime_options::diagnostics(runtime_options::Unit::Lighting))return;FILE* f=std::fopen("C:/Users/ADMIN/Boxer-lab/ms3d/lighting-replacement.log","ab"); if(f) { std::fprintf(f,"%s\n",text); std::fclose(f); } }
bool readable(const void* value,std::size_t length) {
    uintptr_t begin=reinterpret_cast<uintptr_t>(value); if(!value || length>UINTPTR_MAX-begin) return false;
    const uintptr_t end=begin+length;
    while(begin<end) { MEMORY_BASIC_INFORMATION memory{}; if(!VirtualQuery(reinterpret_cast<void*>(begin),&memory,sizeof(memory)) || memory.State!=MEM_COMMIT || (memory.Protect&(PAGE_GUARD|PAGE_NOACCESS))) return false; const auto next=reinterpret_cast<uintptr_t>(memory.BaseAddress)+memory.RegionSize; if(next<=begin) return false; begin=next<end?next:end; }
    return true;
}
bool finite(uint32_t value) { return (value&0x7f800000)!=0x7f800000 && ((value&0x7f800000)!=0 || (value&0x007fffff)==0); }
bool supported(void* object,bool b) {
    uint16_t control=0; __asm__ volatile("fnstcw %0":"=m"(control)); if(control!=0x027f || !readable(object,size)) return false;
    auto* model=static_cast<uint8_t*>(object);
    const int32_t meshes=static_cast<int32_t>(bits(model+meshCount)),triangles=static_cast<int32_t>(bits(model+triangleCount)),materials=static_cast<int32_t>(bits(model+materialCount)),vertices=static_cast<int32_t>(bits(model+vertexCount));
    if(meshes==0 && triangles==0) return true;
    if(meshes<0 || meshes>5000 || triangles<0 || triangles>5000 || materials<=0 || materials>65535 || vertices<=0 || vertices>65535 || (b && triangles>3)) return false;
    struct Range { uintptr_t begin,end; }; std::vector<Range> ranges;
    const auto add=[&](const void* pointer,std::size_t length) { if(!length) return true; if(!readable(pointer,length)) return false; const auto begin=reinterpret_cast<uintptr_t>(pointer),end=begin+length; for(const auto& r:ranges) if(begin<r.end && r.begin<end) return false; ranges.push_back({begin,end}); return true; };
    if(!add(model,size)) return false;
    auto* groups=static_cast<uint8_t*>(pointer(model+meshTable)); auto* ts=static_cast<uint8_t*>(pointer(model+triangleTable)); auto* vs=static_cast<uint8_t*>(pointer(model+vertexTable)); auto* ms=static_cast<uint8_t*>(pointer(model+materialTable));
    if(!add(groups,meshes*12) || !add(ts,triangles*76) || !add(vs,vertices*16) || !add(ms,materials*80)) return false;
    uint32_t total=0;
    for(int32_t mesh=0;mesh<meshes;mesh++) {
        auto* group=groups+mesh*12; const int32_t material=static_cast<int32_t>(bits(group)),n=static_cast<int32_t>(bits(group+4));
        if(material<0 || material>=materials || n<0 || n>5000 || total+static_cast<uint32_t>(n)>5000) return false;
        auto* list=static_cast<uint8_t*>(pointer(group+8)); if(!add(list,n*4)) return false;
        for(int32_t i=0;i<n;i++) { const int32_t index=static_cast<int32_t>(bits(list+i*4)); if(index<0 || index>=triangles) return false; }
        total+=n;
    }
    if(total!=static_cast<uint32_t>(triangles) || (b && total>3)) return false;
    for(int32_t triangle=0;triangle<triangles;triangle++) for(unsigned j=0;j<3;j++) { const int32_t vertex=static_cast<int32_t>(bits(ts+triangle*76+60+j*4)); if(vertex<0 || vertex>=vertices) return false; }
    for(int32_t vertex=0;vertex<vertices;vertex++) for(unsigned j=0;j<3;j++) if(!finite(bits(vs+vertex*16+4+j*4))) return false;
    for(int32_t material=0;material<materials;material++) for(unsigned offset:{16u,20u,24u,48u}) if(!finite(bits(ms+material*80+offset))) return false;
    if(b) for(int32_t index=0;index<triangles;index++) { auto* record=model+4+index*116; const int32_t selected=static_cast<int32_t>(bits(record)); if(selected<0 || selected>=materials) return false; for(unsigned v=0;v<3;v++) for(unsigned j=0;j<3;j++) if(!finite(bits(record+4+v*32+j*4))) return false; }
    return true;
}
GL dependencies() { return {*reinterpret_cast<Color*>(module+0x18ca9c),*reinterpret_cast<Pair*>(module+0x18ca98),*reinterpret_cast<Pair*>(module+0x18ca58),*reinterpret_cast<Cap*>(module+0x18cab8),*reinterpret_cast<Cap*>(module+0x18ca68)}; }
void snapshot(const char* stage,unsigned call,uint32_t eax,void* data,uint32_t length,uint32_t records) {
    if(isolated || !runtime_options::capture(runtime_options::Unit::Lighting)) return;
    char path[256]; std::snprintf(path,sizeof(path),"C:/Users/ADMIN/Boxer-lab/ms3d/lighting-native-%s-%04u.bin",stage,call);
    FILE* f=std::fopen(path,"wb"); if(!f) return;
    uint16_t cw,sw; uint32_t mx; __asm__ volatile("fnstcw %0":"=m"(cw)); __asm__ volatile("fnstsw %0":"=m"(sw)); __asm__ volatile("stmxcsr %0":"=m"(mx));
    const uint32_t header[]={0x33474c52,stage[0]=='A'?1u:2u,call,eax,records,cw,sw,mx,length};
    std::fwrite(header,4,9,f); if(length) std::fwrite(data,1,length,f); std::fclose(f);
}
uint32_t __attribute__((thiscall)) entryA(void* model) {
    runtime_options::Timing timing(runtime_options::Unit::Lighting);
    bool valid=isolated; try { if(!isolated) valid=mode=="replace" && supported(model,false); } catch(...) { valid=false; }
    if(!valid) return reinterpret_cast<Entry>(module+0x27af0)(model);
    ++callsA; const auto result=stageA(model);
    if(!runtime_options::diagnostics(runtime_options::Unit::Lighting) && !runtime_options::capture(runtime_options::Unit::Lighting))return result;
    auto* object=static_cast<uint8_t*>(model); uint32_t produced=0;
    for(int32_t mesh=0;mesh<static_cast<int32_t>(bits(object+meshCount));mesh++) produced+=bits(static_cast<uint8_t*>(pointer(object+meshTable))+mesh*12+4);
    snapshot("A",callsA,result,object+4,(produced<3?produced:3)*116,produced);
    char line[256]; std::snprintf(line,sizeof(line),"RENDER-0003 A route=replacement EAX=%08x calls=%u meshes=%u triangles=%u",result,callsA,bits(static_cast<uint8_t*>(model)+meshCount),bits(static_cast<uint8_t*>(model)+triangleCount)); log(line);
    if(!isolated) for(unsigned i=0;i<3 && i<produced;i++) { auto* r=object+4+i*116; std::snprintf(line,sizeof(line),"RENDER-0003 A record=%u material=%08x center=%08x/%08x/%08x tail=%08x",i,bits(r),bits(r+100),bits(r+104),bits(r+108),bits(r+112)); log(line); }
    return result;
}
uint32_t __attribute__((thiscall)) entryB(void* model) {
    runtime_options::Timing timing(runtime_options::Unit::Lighting);
    bool valid=isolated; try { if(!isolated) valid=mode=="replace" && supported(model,true); } catch(...) { valid=false; }
    if(!valid) return reinterpret_cast<Entry>(module+0x35d90)(model);
    ++callsB; const auto result=stageB(model,reinterpret_cast<uint8_t*>(module+0x177f88),reinterpret_cast<uint8_t*>(module+0x178110),reinterpret_cast<uint8_t*>(module+0x174f08),dependencies());
    if(!runtime_options::diagnostics(runtime_options::Unit::Lighting) && !runtime_options::capture(runtime_options::Unit::Lighting))return result;
    snapshot("B",callsB,result,reinterpret_cast<void*>(module+0x178110),3*56,bits(reinterpret_cast<void*>(module+0x177f88)));
    char line[320]; uint16_t cw,sw; __asm__ volatile("fnstcw %0":"=m"(cw)); __asm__ volatile("fnstsw %0":"=m"(sw));
    std::snprintf(line,sizeof(line),"RENDER-0003 B route=replacement EAX=%08x calls=%u count=%u CW=%04x SW=%04x blend=0302/1 bind=0DE1/%08x culling=enabled color=white",result,callsB,bits(reinterpret_cast<void*>(module+0x177f88)),cw,sw,bits(reinterpret_cast<void*>(module+0x174f08))); log(line);
    for(unsigned i=0;i<3 && i<bits(static_cast<uint8_t*>(model)+triangleCount);i++) { auto* r=reinterpret_cast<uint8_t*>(module+0x178110+i*56); std::snprintf(line,sizeof(line),"RENDER-0003 B record=%u XYZ=%08x/%08x/%08x RGB=%08x/%08x/%08x",i,bits(r),bits(r+4),bits(r+8),bits(r+12),bits(r+16),bits(r+20)); log(line); }
    return result;
}
}
bool install(uintptr_t base,const char* selected) { module=base; mode=selected; return world::redirect(base+0x1690,reinterpret_cast<uintptr_t>(&entryA)) && world::redirect(base+0x1974,reinterpret_cast<uintptr_t>(&entryB)); }
void fixtureMode(bool value) { isolated=value; }
unsigned countA() { return callsA; } unsigned countB() { return callsB; }
}
