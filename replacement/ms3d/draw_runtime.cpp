#include "draw_trace.hpp"
#include "world_runtime.hpp"
#include "runtime_options.hpp"
#include <windows.h>
#include <string>
namespace draw {
namespace {
uintptr_t module;std::string mode;bool isolated=false;unsigned replacements=0,fallbacks=0,baselineCalls=0;
uint32_t originalSlots[13]{};thread_local Trace* active=nullptr;
Observer observer=nullptr;
bool captureHooks=false;
bool readable(const void* ptr,uint64_t bytes){
 uintptr_t p=reinterpret_cast<uintptr_t>(ptr);if(bytes==0)return true;if(!p || bytes>0x10000000 || uint64_t(p)+bytes>0xffffffffULL)return false;
 while(bytes){MEMORY_BASIC_INFORMATION m{};if(!VirtualQuery(reinterpret_cast<void*>(p),&m,sizeof(m)) || m.State!=MEM_COMMIT || (m.Protect&(PAGE_NOACCESS|PAGE_GUARD)))return false;uintptr_t end=reinterpret_cast<uintptr_t>(m.BaseAddress)+m.RegionSize;uint64_t chunk=end-p;if(!chunk)return false;if(chunk>bytes)chunk=bytes;p+=static_cast<uintptr_t>(chunk);bytes-=chunk;}return true;
}
bool executable(uint32_t p){MEMORY_BASIC_INFORMATION m{};return p && VirtualQuery(reinterpret_cast<void*>(p),&m,sizeof(m)) && m.State==MEM_COMMIT && !(m.Protect&(PAGE_NOACCESS|PAGE_GUARD)) && (m.Protect&(PAGE_EXECUTE|PAGE_EXECUTE_READ|PAGE_EXECUTE_READWRITE|PAGE_EXECUTE_WRITECOPY));}
size_t record(Api api,const uint32_t* args,unsigned n,const uint32_t* p=nullptr,unsigned words=0){if(observer){FpPreserver fp;observer(api,args,n);}if(active){FpPreserver fp;const size_t index=active->events.size();active->append(api,args,n,p,words);return index;}return static_cast<size_t>(-1);}
uint32_t complete(size_t index,uint32_t result){if(active && index<active->events.size())active->events[index].result=result;return result;}
uint32_t __stdcall isEnabled(uint32_t a){const auto index=record(Api::IsEnabled,&a,1);const uint32_t result=reinterpret_cast<One>(originalSlots[0])(a);return complete(index,result);}
uint32_t __stdcall materialfv(uint32_t a,uint32_t b,const uint32_t* p){uint32_t args[]={a,b};const auto index=record(Api::Materialfv,args,2,p,4);const uint32_t result=reinterpret_cast<MaterialVector>(originalSlots[1])(a,b,p);return complete(index,result);}
uint32_t __stdcall materialf(uint32_t a,uint32_t b,uint32_t c){uint32_t args[]={a,b,c};const auto index=record(Api::Materialf,args,3);const uint32_t result=reinterpret_cast<Three>(originalSlots[2])(a,b,c);return complete(index,result);}
uint32_t __stdcall bind(uint32_t a,uint32_t b){uint32_t args[]={a,b};const auto index=record(Api::BindTexture,args,2);const uint32_t result=reinterpret_cast<Two>(originalSlots[3])(a,b);return complete(index,result);}
uint32_t __stdcall enable(uint32_t a){const auto index=record(Api::Enable,&a,1);const uint32_t result=reinterpret_cast<One>(originalSlots[4])(a);return complete(index,result);}
uint32_t __stdcall disable(uint32_t a){const auto index=record(Api::Disable,&a,1);const uint32_t result=reinterpret_cast<One>(originalSlots[5])(a);return complete(index,result);}
uint32_t __stdcall begin(uint32_t a){const auto index=record(Api::Begin,&a,1);const uint32_t result=reinterpret_cast<One>(originalSlots[6])(a);return complete(index,result);}
uint32_t __stdcall normal(const uint32_t* p){const auto index=record(Api::Normal,nullptr,0,p,3);const uint32_t result=reinterpret_cast<Pointer>(originalSlots[7])(p);return complete(index,result);}
uint32_t __stdcall uv(uint32_t a,uint32_t b){uint32_t args[]={a,b};const auto index=record(Api::TexCoord,args,2);const uint32_t result=reinterpret_cast<Two>(originalSlots[8])(a,b);return complete(index,result);}
uint32_t __stdcall vertex(const uint32_t* p){const auto index=record(Api::Vertex,nullptr,0,p,3);const uint32_t result=reinterpret_cast<Pointer>(originalSlots[9])(p);return complete(index,result);}
uint32_t __stdcall end(){const auto index=record(Api::End,nullptr,0);const uint32_t result=reinterpret_cast<Zero>(originalSlots[10])();return complete(index,result);}
uint32_t __stdcall vector(uint32_t a,uint32_t b,uint32_t c,uint32_t d,uint32_t e){uint32_t args[]={a,b,c,d,e};const auto index=record(Api::ShaderVector,args,5);const uint32_t result=reinterpret_cast<Vector>(originalSlots[11])(a,b,c,d,e);return complete(index,result);}
uint32_t __stdcall scalar(uint32_t a,uint32_t b){uint32_t args[]={a,b};const auto index=record(Api::ShaderScalar,args,2);const uint32_t result=reinterpret_cast<Two>(originalSlots[12])(a,b);return complete(index,result);}
const uintptr_t wrappers[]={reinterpret_cast<uintptr_t>(&isEnabled),reinterpret_cast<uintptr_t>(&materialfv),reinterpret_cast<uintptr_t>(&materialf),reinterpret_cast<uintptr_t>(&bind),reinterpret_cast<uintptr_t>(&enable),reinterpret_cast<uintptr_t>(&disable),reinterpret_cast<uintptr_t>(&begin),reinterpret_cast<uintptr_t>(&normal),reinterpret_cast<uintptr_t>(&uv),reinterpret_cast<uintptr_t>(&vertex),reinterpret_cast<uintptr_t>(&end),reinterpret_cast<uintptr_t>(&vector),reinterpret_cast<uintptr_t>(&scalar)};
bool writeSlot(void* p,uint32_t value){DWORD old,unused;if(!VirtualProtect(p,4,PAGE_READWRITE,&old))return false;writeWord(p,value);return VirtualProtect(p,4,old,&unused)!=0;}
bool refreshWrappers(const Dispatch& gl){
 for(unsigned i=0;i<13;++i){const auto target=readWord(gl.module+slots[i]);if(target==wrappers[i]){if(!executable(originalSlots[i]))return false;}else if(!executable(target))return false;}
 for(unsigned i=0;i<13;++i){const auto target=readWord(gl.module+slots[i]);if(target!=wrappers[i]){originalSlots[i]=target;if(!writeSlot(gl.module+slots[i],static_cast<uint32_t>(wrappers[i])))return false;}}return true;
}
uint32_t __attribute__((thiscall)) entry(void* object){
 runtime_options::Timing timing(runtime_options::Unit::Draw);
 timing.checkpoint("guard");
 Dispatch gl{reinterpret_cast<uint8_t*>(module)};ModelView model(object);
 if(!isolated && (!supported(model,gl,true) || (captureHooks && !refreshWrappers(gl)))){++fallbacks;if(runtime_options::diagnostics(runtime_options::Unit::Draw) && (fallbacks<=3 || fallbacks==120)){FpPreserver fp;FILE* f=std::fopen("C:/Users/ADMIN/Boxer-lab/ms3d/draw-routes.jsonl","ab");if(f){std::fprintf(f,"{\"route\":\"original-fallback\",\"call\":%u,\"map_identity\":%s}\n",fallbacks,object && readWord(gl.module+mapGlobal)==reinterpret_cast<uintptr_t>(object)?"true":"false");std::fclose(f);}}return reinterpret_cast<Entry>(module+0x26b00)(object);}
 const bool replacement=isolated || mode=="replace";const unsigned call=replacement?++replacements:++baselineCalls;const char* route=replacement?"replacement":"original";Trace trace;const bool capture=runtime_options::capture(runtime_options::Unit::Draw) && !isolated && (call<=3 || call==120);char path[256];
 if(capture){FpPreserver fp;trace.registerModel(model);std::snprintf(path,sizeof(path),"C:/Users/ADMIN/Boxer-lab/ms3d/draw-native-%s-%04u-pre.bin",route,call);snapshot(path,model,gl);active=&trace;}
 uint16_t cw,sw;uint32_t mx;__asm__ volatile("fnstcw %0":"=m"(cw));__asm__ volatile("fnstsw %0":"=m"(sw));__asm__ volatile("stmxcsr %0":"=m"(mx));
 timing.checkpoint("render");const uint32_t result=replacement?render(model,gl):reinterpret_cast<Entry>(module+0x26b00)(object);active=nullptr;timing.checkpoint("render-complete");
 if(capture){uint16_t afterCW,afterSW;uint32_t afterMX;__asm__ volatile("fnstcw %0":"=m"(afterCW));__asm__ volatile("fnstsw %0":"=m"(afterSW));__asm__ volatile("stmxcsr %0":"=m"(afterMX));FpPreserver fp;
  std::snprintf(path,sizeof(path),"C:/Users/ADMIN/Boxer-lab/ms3d/draw-native-%s-%04u-post.bin",route,call);snapshot(path,model,gl);
  FILE* f=std::fopen("C:/Users/ADMIN/Boxer-lab/ms3d/draw-native.jsonl","ab");if(f){std::fprintf(f,"{\"unit\":\"RENDER-0005\",\"route\":\"%s\",\"call\":%u,\"fallbacks\":%u,\"result\":%u,\"cw\":%u,\"sw\":%u,\"mxcsr\":%u,\"after_cw\":%u,\"after_sw\":%u,\"after_mxcsr\":%u,\"digest\":\"%016llx\",\"counts\":[",route,call,fallbacks,result,cw,sw,mx,afterCW,afterSW,afterMX,static_cast<unsigned long long>(trace.digest));for(unsigned i=0;i<13;++i)std::fprintf(f,"%s%u",i?",":"",trace.counts[i]);std::fputs("],\"events\":",f);trace.writeEvents(f);std::fputs("}\n",f);std::fclose(f);}
 }return result;
}
}
bool supported(ModelView model,const Dispatch& gl,bool native){
 FpPreserver preserve;
 uint16_t cw;__asm__ volatile("fnstcw %0":"=m"(cw));if(cw!=0x027f)return false;
 if(native){if(!model.data() || readWord(gl.module+mapGlobal)!=reinterpret_cast<uintptr_t>(model.data()))return false;using Context=void* (__stdcall*)();auto proc=reinterpret_cast<Context>(GetProcAddress(GetModuleHandleA("opengl32.dll"),"wglGetCurrentContext"));if(!proc || !proc())return false;}
 if(!readable(model.data(),ModelView::size))return false;
 for(unsigned i=0;i<13;++i)if(!executable(readWord(gl.module+slots[i])))return false;
 const int32_t meshes=model.meshCount();if(meshes<=0)return true;
 // Resource bounds limit this adapter, not the original renderer's capacity.
 if(meshes>65536 || model.materialCount()<1 || model.materialCount()>65536 || model.triangleCount()<0 || model.triangleCount()>1000000 || model.vertexCount()<0 || model.vertexCount()>1000000)return false;
 if(!readable(model.meshes(),uint64_t(meshes)*sizeof(Mesh)) || !readable(model.materials(),uint64_t(model.materialCount())*sizeof(Material)) || !readable(model.triangles(),uint64_t(model.triangleCount())*sizeof(Triangle)) || !readable(model.vertices(),uint64_t(model.vertexCount())*sizeof(Vertex)))return false;
 struct Range{uintptr_t begin,end;bool membership;};std::vector<Range> ranges;
 const auto distinct=[&](const void* p,uint32_t size,bool membership){if(!size)return true;Range next{reinterpret_cast<uintptr_t>(p),reinterpret_cast<uintptr_t>(p)+size,membership};for(auto old:ranges)if(next.begin<old.end && old.begin<next.end){if(membership && old.membership && next.begin==old.begin && next.end==old.end)return true;return false;}ranges.push_back(next);return true;};
 if(!distinct(model.data(),ModelView::size,false) || !distinct(model.meshes(),meshes*sizeof(Mesh),false) || !distinct(model.materials(),model.materialCount()*sizeof(Material),false) || !distinct(model.triangles(),model.triangleCount()*sizeof(Triangle),false) || !distinct(model.vertices(),model.vertexCount()*sizeof(Vertex),false))return false;
 for(int32_t i=0;i<meshes;++i){const auto& mesh=model.mesh(i);if(mesh.material<0 || mesh.material>=model.materialCount())return false;auto value=model.material(mesh.material).transparency;if((value&0x7f800000)==0x7f800000)return false;if(mesh.membershipCount>1000000 || (mesh.membershipCount>0 && !readable(mesh.memberships,uint64_t(mesh.membershipCount)*4)))return false;
  if(mesh.membershipCount>0 && !distinct(mesh.memberships,mesh.membershipCount*4,true))return false;
  for(int32_t j=0;j<mesh.membershipCount;++j){int32_t t=mesh.memberships[j];if(t<0 || t>=model.triangleCount())return false;for(auto v:model.triangles()[t].vertices)if(v<0 || v>=model.vertexCount())return false;}
 }return true;
}
bool install(uintptr_t base,const char* selected){
 module=base;mode=selected;Dispatch gl{reinterpret_cast<uint8_t*>(base)};
 captureHooks=runtime_options::capture(runtime_options::Unit::Draw) || runtime_options::capture(runtime_options::Unit::Character);
 // Root's independent observer validates baseline route before bootstrap.
 // This adapter only synthesizes the approved metadata jump; no code read.
 for(unsigned i=0;i<13;++i){originalSlots[i]=readWord(gl.module+slots[i]);if(i<11 && !executable(originalSlots[i]))return false;}
 // Only original dispatch is forwarded; wrappers record inside this unit's tag.
 if(captureHooks)for(unsigned i=0;i<13;++i)if(executable(originalSlots[i]) && !writeSlot(gl.module+slots[i],static_cast<uint32_t>(wrappers[i])))return false;
 return world::redirect(base+0x1b54,reinterpret_cast<uintptr_t>(&entry));
}
void fixtureMode(bool b){isolated=b;}unsigned count(){return replacements;}
void setObserver(Observer value){observer=value;}
}
