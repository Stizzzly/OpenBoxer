#include "selection.hpp"
#include "world_runtime.hpp"
#include "runtime_options.hpp"
#include <windows.h>
#include <cstdio>
#include <cstring>
#include <string>
namespace selection {
namespace {
uintptr_t module=0; std::string mode; bool isolated=false; unsigned replaced=0,fallbacks=0;
struct Window { uint32_t rva,size; };
constexpr Window windows[]={{0x177f88,4},{0x17810c,176},{0x184c98,12},{0x184770,4},{0x184c90,4},{0x170174,48},{0x1701b4,48},{0x1701f4,40},{0x170224,16},{0x176370,4},{0x184af0,4},{0x177f84,4},{0x170354,4},{0x1855f0,4},{0x1855f4,4},{0x17028c,204}};
struct FP { alignas(16) uint8_t data[512]; FP() { __asm__ volatile("fxsave %0":"=m"(data)); } ~FP() { __asm__ volatile("fxrstor %0"::"m"(data)); } };
bool finite(uint32_t n) { return (n&0x7f800000)!=0x7f800000; }
bool supported(State s) {
 uint16_t cw; __asm__ volatile("fnstcw %0":"=m"(cw)); if(cw!=0x027f || bits(s.at(0x177f88))!=3) return false;
 for(unsigned i=0;i<3;++i) for(unsigned c=0;c<3;++c) if(!finite(bits(s.at(0x178110+i*56+c*4)))) return false;
 for(unsigned c=0;c<3;++c) if(!finite(bits(s.at(0x184c98+c*4)))) return false;
 if(!finite(bits(s.at(0x184c90)))) return false;
 if((bits(s.at(0x184c98))&0x7fffffff)==0 && (bits(s.at(0x184c9c))&0x7fffffff)==0 && (bits(s.at(0x184ca0))&0x7fffffff)==0) return false;
 if(static_cast<int32_t>(bits(s.at(0x170354)))<0 || bits(s.at(0x170354))>50 || static_cast<int32_t>(bits(s.at(0x1855f0)))<0 || bits(s.at(0x1855f0))>50) return false;
 for(uint32_t slot:{0x1855ccu,0x1855dcu}) { MEMORY_BASIC_INFORMATION m{}; auto* p=reinterpret_cast<void*>(bits(s.at(slot))); if(!p || !VirtualQuery(p,&m,sizeof(m)) || m.State!=MEM_COMMIT || (m.Protect&(PAGE_NOACCESS|PAGE_GUARD)) || !(m.Protect&(PAGE_EXECUTE|PAGE_EXECUTE_READ|PAGE_EXECUTE_READWRITE|PAGE_EXECUTE_WRITECOPY))) return false; }
 return true;
}
struct Trace { uint32_t args[7][5]{},results[7]{}; };
void trace(void* p,unsigned i,const uint32_t* a,uint32_t result) { auto& t=*static_cast<Trace*>(p); for(unsigned j=0;j<5;++j)t.args[i][j]=a[j];t.results[i]=result; }
uint32_t __cdecl entry() {
 runtime_options::Timing timing(runtime_options::Unit::Selection);
 State s{reinterpret_cast<uint8_t*>(module)};
 if(!isolated && (mode!="replace" || !supported(s))) { ++fallbacks; return reinterpret_cast<Entry>(module+0x362a0)(); }
 ++replaced; const bool capture=runtime_options::capture(runtime_options::Unit::Selection) && !isolated && (replaced<=3 || replaced==120); Trace t;
 char path[256]; if(capture) { std::snprintf(path,sizeof(path),"C:/Users/ADMIN/Boxer-lab/ms3d/selection-native-%04u-pre.bin",replaced); dumpState(path,s,replaced,0); s.trace=trace;s.context=&t; }
 const auto result=run(s,dependencies(module));
 if(capture) {
  std::snprintf(path,sizeof(path),"C:/Users/ADMIN/Boxer-lab/ms3d/selection-native-%04u-post.bin",replaced); dumpState(path,s,replaced,result);
  FP preserve; FILE* f=std::fopen("C:/Users/ADMIN/Boxer-lab/ms3d/selection-replacement.log","ab");
  if(f) { uint16_t cw,sw; uint32_t mx; __asm__ volatile("fnstcw %0":"=m"(cw));__asm__ volatile("fnstsw %0":"=m"(sw));__asm__ volatile("stmxcsr %0":"=m"(mx));
   std::fprintf(f,"RENDER-0004 route=replacement count=%u fallback=%u EAX=%08x CW=%04x exceptions=%02x MXCSR=%08x\n",replaced,fallbacks,result,cw,sw&63,mx);
   for(unsigned i=0;i<7;++i) std::fprintf(f,"call=%u args=%08x/%08x/%08x/%08x/%08x result=%08x\n",i,t.args[i][0],t.args[i][1],t.args[i][2],t.args[i][3],t.args[i][4],t.results[i]); std::fclose(f);
  }
 }
 return result;
}
}
void dumpState(const char* path,State s,uint32_t call,uint32_t result) {
 FP preserve; uint16_t cw,sw;uint32_t mx; __asm__ volatile("fnstcw %0":"=m"(cw));__asm__ volatile("fnstsw %0":"=m"(sw));__asm__ volatile("stmxcsr %0":"=m"(mx));
 FILE* f=std::fopen(path,"wb");if(!f)return; const uint32_t header[]={0x344c4553,1,call,result,cw,sw,mx,sizeof(windows)/sizeof(windows[0])+9};std::fwrite(header,4,8,f);
 for(auto w:windows){std::fwrite(&w,4,2,f);std::fwrite(s.at(w.rva),1,w.size,f);}for(auto rva:handles){const Window w{rva,4};std::fwrite(&w,4,2,f);std::fwrite(s.at(rva),1,4,f);}for(uint32_t rva:{0x1855ccu,0x1855dcu}){const Window w{rva,4};std::fwrite(&w,4,2,f);std::fwrite(s.at(rva),1,4,f);}std::fclose(f);
}
bool install(uintptr_t b,const char* selected) { module=b;mode=selected;return world::redirect(b+0x16e0,reinterpret_cast<uintptr_t>(&entry)); }
void fixtureMode(bool b){isolated=b;}unsigned count(){return replaced;}
}
