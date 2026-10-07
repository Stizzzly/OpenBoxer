#include "selection.hpp"
#include "texture_abi.hpp"
#include "world_runtime.hpp"
#include <windows.h>
#include <vector>
#include <string>
#include <sstream>
#include <cstring>
#include <cstdio>
#include <algorithm>
namespace selection {
namespace {
enum Script : unsigned { freshPositions=1,productMutation=2,shrink=4,angularMode=8,modeAgain=16,liveSin=32,componentMutation=64,waveMutation=128,dispatchMutation=256,globalAccess=512,angularMutation=1024 };
struct Case { const char* name; int32_t count=3,mode=0; unsigned pattern=0,script=0,passes=1; uint32_t angle=0x3f000000,returnValue=0x6a5b4c3d; bool real=false,realWave=false,zero=false; uint64_t angular=0x3fe0000010000001ULL; uint16_t initialSW=0;uint32_t initialMX=0x1f80; };
struct Recorder {
 Case test; State state; unsigned inits=0,accesses=0,angulars=0,magnitudes=0,rotations=0,sines=0,dispatches=0; Vector* origin=nullptr; void* reference=nullptr; void* direction=nullptr; void* rotation=nullptr; Vector alternate{}; std::vector<std::string> events; bool valid=true;
 uint32_t checkpoints[3][42]{};int32_t sortCount=0;
 Recorder(Case c,State s):test(c),state(s) {}
 std::string snapshot() const {
  std::ostringstream out;out<<std::hex<<"count="<<bits(state.at(0x177f88))<<" mode="<<bits(state.at(0x184770));
  for(unsigned i=0;i<42;++i)out<<','<<bits(state.at(0x178110+i*4));for(unsigned k=0;k<3;++k){for(unsigned c=0;c<3;++c)out<<','<<bits(state.at(positions[k]+c*4));for(unsigned c=0;c<3;++c)out<<','<<bits(state.at(colors[k]+c*4));for(unsigned c=0;c<3;++c)out<<','<<bits(state.at(directions[k]+c*4));out<<','<<bits(state.at(parameters[k]));}
  for(auto rva:handles)out<<','<<bits(state.at(rva));
  if(test.realWave){for(uint32_t rva:{0x176370u,0x184af0u,0x177f84u,0x170354u,0x1855f0u,0x1855f4u})out<<','<<bits(state.at(rva));for(unsigned i=0;i<51;++i)out<<','<<bits(state.at(0x17028c+i*4));}
  return out.str();
 }
 void event(const char* name,const uint32_t* values,unsigned n) { std::ostringstream out;out<<name<<'(';for(unsigned i=0;i<n;++i)out<<std::hex<<values[i]<<',';out<<") | "<<snapshot();events.push_back(out.str()); }
 unsigned role(void* p) { if(p==state.at(0x184c98))return 1;if(p==origin)return 2;if(p==reference)return 3;if(p==rotation)return 5;if(p==direction)return 4;return 9; }
};
thread_local Recorder* active=nullptr;
void* __attribute__((thiscall)) init(void* target,uint32_t x,uint32_t y,uint32_t z) {
 auto& r=*active; ++r.inits;
 if(r.inits==1)r.origin=static_cast<Vector*>(target);else if(r.inits==2)r.reference=target;else if(x==0 && y==0xbf800000 && z==0)r.rotation=target;else r.direction=target;
 const uint32_t args[]={r.role(target),x,y,z,bits(static_cast<uint8_t*>(target)+12)};r.event("init",args,5);
 if(args[4]!=0xcccccccc)r.valid=false;put(target,x);put(static_cast<uint8_t*>(target)+4,y);put(static_cast<uint8_t*>(target)+8,z);return target;
}
void* __attribute__((thiscall)) access(void* target,int32_t index) {
 auto& r=*active;const unsigned role=r.role(target);const uint32_t args[]={role,static_cast<uint32_t>(index),bits(static_cast<uint8_t*>(target)+index*4),role==1?0:bits(static_cast<uint8_t*>(target)+12)};r.event("access",args,4);
 if(role==5 && args[3]!=0xabcdef01)r.valid=false;
 if(target==r.origin) {
  ++r.accesses;if(r.test.script&freshPositions)put(r.state.at(0x178110+index*4),0x3f800000+r.accesses*0x10000);
 }
 if((r.test.script&globalAccess)&&target==r.state.at(0x184c98))put(r.state.at(0x184c98),0x40400000+r.accesses*0x1000);
 if((r.test.script&componentMutation)&&role==5){put(r.state.at(0x178110+24+index*4),0x12340000+index);put(r.state.at(0x184770),2);}
 return static_cast<uint8_t*>(target)+index*4;
}
void __attribute__((thiscall)) normalize(void* target) {
 auto& r=*active;uint32_t args[]={r.role(target),bits(target),bits(static_cast<uint8_t*>(target)+4),bits(static_cast<uint8_t*>(target)+8),bits(static_cast<uint8_t*>(target)+12)};r.event("normalize",args,5);
 if(args[4]!=0xcccccccc)r.valid=false;
}
double __cdecl angular(Vector a,Vector b) {
 auto& r=*active;const uint32_t args[]={a.x,a.y,a.z,a.opaque,b.x,b.y,b.z,b.opaque};r.event("angular",args,8);++r.angulars;
 if(a.opaque!=0xcccccccc || b.opaque!=0xcccccccc)r.valid=false;
 if(r.test.script&angularMode)put(r.state.at(0x184770),1);
 if(r.test.script&angularMutation){put(r.state.at(0x178110+(r.angulars-1)*56+44),0x40a00000);put(r.state.at(0x177f88),2);}
 double result;std::memcpy(&result,&r.test.angular,8);return result;
}
float __cdecl magnitude(float value) {
 auto& r=*active;uint32_t arg;std::memcpy(&arg,&value,4);r.event("magnitude",&arg,1);++r.magnitudes;
 if(r.test.script&productMutation)put(r.state.at(0x178110+(r.magnitudes-1)*56+44),0x40e00000);
 if(r.test.script&shrink)put(r.state.at(0x177f88),1);
 uint32_t result=0x3f400000;float f;std::memcpy(&f,&result,4);return f;
}
void* __cdecl rotate(void* output,Vector v,float angle,float x,float y,float z) {
 auto& r=*active;uint32_t args[9]={v.x,v.y,v.z,v.opaque};std::memcpy(args+4,&angle,4);std::memcpy(args+5,&x,4);std::memcpy(args+6,&y,4);std::memcpy(args+7,&z,4);args[8]=bits(static_cast<uint8_t*>(output)+12);r.event("rotate",args,9);++r.rotations;
 if(args[3]!=0xcccccccc || args[5]!=0x3f800000 || args[6]!=0 || args[7]!=0)r.valid=false;
 r.alternate={0x3e800000+r.rotations*0x10000,0xbf000000,0x3f400000,0xabcdef01};
 // The returned buffer deliberately differs from the nominal output argument.
 put(output,0xdeaddead);if(r.test.script&modeAgain)put(r.state.at(0x184770),2);return &r.alternate;
}
double __cdecl sine(double value) { auto& r=*active;uint32_t args[2];std::memcpy(args,&value,8);r.event("sin",args,2);++r.sines;if(r.test.script&liveSin)put(r.state.at(0x184c90),0xbf800000);return value; }
void __cdecl wave() {
 auto& r=*active;r.event("wave",nullptr,0);
 if(r.test.script&waveMutation)for(unsigned i=0;i<3;++i){put(r.state.at(0x178110+i*56),0x41000000+i*0x100000);put(r.state.at(0x178110+i*56+24),0xbf800000+i);put(r.state.at(0x178110+i*56+36),70+i);}
}
uint32_t __stdcall scalarAlternate(uint32_t,uint32_t);
uint32_t __stdcall colorAlternate(uint32_t,uint32_t,uint32_t,uint32_t,uint32_t);
uint32_t dispatch(const char* name,const uint32_t* values,unsigned n) {
 auto& r=*active;r.event(name,values,n);++r.dispatches;
 if((r.test.script&dispatchMutation)&&r.dispatches==1) {
  for(unsigned i=1;i<7;++i)put(r.state.at(handles[i]),100+i);
  for(unsigned k=0;k<3;++k){for(unsigned c=0;c<3;++c)put(r.state.at(colors[k]+c*4),0x3e800000+k*0x10000+c*0x100);put(r.state.at(parameters[k]),90+k);}
  put(r.state.at(0x1855dc),reinterpret_cast<uintptr_t>(&scalarAlternate));put(r.state.at(0x1855cc),reinterpret_cast<uintptr_t>(&colorAlternate));
 }
 return r.test.returnValue;
}
uint32_t __stdcall scalar(uint32_t h,uint32_t p){const uint32_t a[]={h,p};return dispatch("scalar",a,2);}
uint32_t __stdcall scalarAlternate(uint32_t h,uint32_t p){const uint32_t a[]={h,p};return dispatch("scalar-alt",a,2);}
uint32_t __stdcall color(uint32_t h,uint32_t a,uint32_t b,uint32_t c,uint32_t d){const uint32_t args[]={h,a,b,c,d};return dispatch("vector",args,5);}
uint32_t __stdcall colorAlternate(uint32_t h,uint32_t a,uint32_t b,uint32_t c,uint32_t d){const uint32_t args[]={h,a,b,c,d};return dispatch("vector-alt",args,5);}
Dependencies fake(){return {init,access,normalize,angular,magnitude,rotate,sine,wave};}
uint32_t __cdecl offline(){return run(active->state,fake());}
void checkpoint(void* context,unsigned phase,State s){auto& r=*static_cast<Recorder*>(context);for(unsigned i=0;i<42;++i)r.checkpoints[phase][i]=bits(s.at(0x178110+i*4));if(phase==0)r.sortCount=static_cast<int32_t>(bits(s.at(0x177f88)));}
void reset(Recorder& r) {
 auto s=r.state;put(s.at(0x177f88),static_cast<uint32_t>(r.test.count));put(s.at(0x184770),static_cast<uint32_t>(r.test.mode));put(s.at(0x184c90),r.test.angle);
 for(unsigned c=0;c<3;++c)put(s.at(0x184c98+c*4),r.test.zero?0:(c==2?0x3f800000:0));
 for(unsigned i=0;i<3;++i){for(unsigned j=0;j<14;++j)put(s.at(0x178110+i*56+j*4),0x55000000+i*0x10000+j);uint32_t x;
  if(r.test.pattern==0)x=0x3f800000+i*0x800000;else if(r.test.pattern==1)x=0x40400000-i*0x400000;else if(r.test.pattern==2)x=i==2?0x3f800000:0x40000000;else if(r.test.pattern==3)x=i==0?0x40000000:i==1?0x40400000:0x3f800000;else x=i==0?0x4b800000:i==1?0xcb800000:0x33800001;
  put(s.at(0x178110+i*56),x);put(s.at(0x178110+i*56+4),r.test.pattern==4?0x40133334:0x40133333);put(s.at(0x178110+i*56+8),r.test.pattern==4?0x41200001:0x41200000);put(s.at(0x178110+i*56+36),21+i);
 }
 put(s.at(0x17810c),0x76543210);put(s.at(0x1781b8),0x87654321);
 for(unsigned k=0;k<3;++k){for(unsigned c=0;c<3;++c){put(s.at(positions[k]+c*4),0xa0000000+k*16+c);put(s.at(colors[k]+c*4),0xb0000000+k*16+c);put(s.at(directions[k]+c*4),0xc0000000+k*16+c);}put(s.at(parameters[k]),0xd0000000+k);}
 for(uint32_t gap:{0x170184u,0x170194u,0x1701a4u,0x1701c4u,0x1701d4u,0x1701e4u})put(s.at(gap),0x13572468);
 for(unsigned i=0;i<7;++i)put(s.at(handles[i]),10+i);put(s.at(0x1855dc),reinterpret_cast<uintptr_t>(&scalar));put(s.at(0x1855cc),reinterpret_cast<uintptr_t>(&color));
 if(r.test.realWave){for(unsigned i=0;i<51;++i)put(s.at(0x17028c+i*4),0x3f000000);put(s.at(0x170354),0);put(s.at(0x1855f0),0);put(s.at(0x176370),0x3c23d70a);put(s.at(0x184af0),0);put(s.at(0x177f84),0);put(s.at(0x1855f4),0);}
}
std::vector<Case> cases() {
 std::vector<Case> out={{"ordered"},{"reverse",3,0,1},{"indirect_ties",3,0,2},{"second_exchange",3,0,3},{"repeated",3,0,1,0,2},{"cancellation",3,0,4},{"count0",0},{"count1",1},{"count2",2},{"negative",-2},{"mode1",3,1},{"mode2",3,2},{"mode_other",3,9},{"count0_mode1",0,1},{"negative_mode2",-1,2},{"angle_zero",3,1,0,0,1,0},{"angle_negative",3,1,0,0,1,0xbf800000},{"return_zero",3,0,0,0,1,0x3f000000,0},{"return_ffffffff",3,0,0,0,1,0x3f000000,0xffffffff},{"position_live",3,0,0,freshPositions},{"product_live",3,0,0,productMutation},{"count_shrink",3,0,0,shrink},{"angular_mode",3,0,0,angularMode},{"independent_mode",3,1,0,modeAgain},{"sin_live",3,1,0,liveSin},{"components_live",3,1,0,componentMutation},{"wave_live",3,0,1,waveMutation},{"dispatch_live",3,0,0,dispatchMutation},{"global_access_live",3,0,0,globalAccess},{"angular_distance_count",3,0,0,angularMutation}};
 return out;
}
struct Result { uint32_t value;WorldAbiReport abi{};uint16_t cw=0,sw=0;uint32_t mx=0;bool valid=false; };
Result invoke(Entry entry,Recorder& r) {
 active=&r;Result result{};uint16_t savedCW;uint32_t savedMX;__asm__ volatile("fnstcw %0":"=m"(savedCW));__asm__ volatile("stmxcsr %0":"=m"(savedMX));const uint16_t cw=0x027f;const uint32_t mx=r.test.initialMX;__asm__ volatile("fnclex; fldcw %0"::"m"(cw));__asm__ volatile("ldmxcsr %0"::"m"(mx));
 if(r.test.initialSW&63){uint32_t environment[7];__asm__ volatile("fnstenv %0":"=m"(environment));environment[1]=(environment[1]&~63u)|(r.test.initialSW&63u);__asm__ volatile("fldenv %0"::"m"(environment));}
 setObserver(checkpoint,&r);
 for(unsigned pass=0;pass<r.test.passes;++pass){r.inits=0;r.origin=nullptr;r.reference=nullptr;r.direction=nullptr;r.rotation=nullptr;r.dispatches=0;r.angulars=0;r.magnitudes=0;r.accesses=0;r.rotations=0;r.sines=0;result.value=texture_abi_probe(reinterpret_cast<texture::Reload>(entry),nullptr,&result.abi);__asm__ volatile("fnstcw %0":"=m"(result.cw));__asm__ volatile("fnstsw %0":"=m"(result.sw));__asm__ volatile("stmxcsr %0":"=m"(result.mx));result.valid=worldAbiValid(result.abi)&&r.valid&&r.dispatches==7&&result.cw==cw&&(result.sw&0x3800)==0;}
 setObserver(nullptr,nullptr);
 active=nullptr;__asm__ volatile("fldcw %0"::"m"(savedCW));__asm__ volatile("ldmxcsr %0"::"m"(savedMX));return result;
}
bool oracle(const Recorder& r,const Result& result) {
 if(!result.valid || result.value!=r.test.returnValue || bits(r.state.at(0x17810c))!=0x76543210 || bits(r.state.at(0x1781b8))!=0x87654321)return false;
 for(unsigned k=0;k<3;++k){for(unsigned c=0;c<3;++c){if(bits(r.state.at(positions[k]+c*4))!=bits(r.state.at(0x178110+k*56+c*4)) && !(r.test.script&dispatchMutation))return false;if(!(r.test.script&dispatchMutation) && bits(r.state.at(colors[k]+c*4))!=0x3f800000)return false;}
  if(!(r.test.script&dispatchMutation) && bits(r.state.at(parameters[k]))!=bits(r.state.at(0x178110+k*56+36)))return false;
 }
 if(r.test.count==3 && r.test.pattern==2 && r.test.script==0){const uint32_t expected[]={23,22,7};for(unsigned i=0;i<3;++i)if(bits(r.state.at(0x178110+i*56+36))!=expected[i])return false;}
 if(r.test.count==3 && r.test.pattern==1 && r.test.script==0){const uint32_t expected[]={23,7,7};for(unsigned i=0;i<3;++i)if(bits(r.state.at(0x178110+i*56+36))!=expected[i])return false;}
 for(uint32_t gap:{0x170184u,0x170194u,0x1701a4u,0x1701c4u,0x1701d4u,0x1701e4u})if(bits(r.state.at(gap))!=0x13572468)return false;
 return true;
}
void emit(FILE* f,unsigned id,const Recorder& r,const Result& v) {
 std::fprintf(f,"{\"scenario\":%u,\"name\":\"%s\",\"eax\":%u,\"cw\":%u,\"exceptions\":%u,\"mxcsr\":%u,\"records\":[",id,r.test.name,v.value,v.cw,v.sw&63,v.mx);
 for(unsigned i=0;i<42;++i)std::fprintf(f,"%s%u",i?",":"",bits(r.state.at(0x178110+i*4)));std::fprintf(f,"],\"selected\":[");bool first=true;for(unsigned k=0;k<3;++k){for(auto rva:{positions[k],colors[k],directions[k]})for(unsigned c=0;c<3;++c){std::fprintf(f,"%s%u",first?"":",",bits(r.state.at(rva+c*4)));first=false;}std::fprintf(f,",%u",bits(r.state.at(parameters[k])));}std::fprintf(f,"],\"events\":[");for(unsigned i=0;i<r.events.size();++i)std::fprintf(f,"%s\"%s\"",i?",":"",r.events[i].c_str());std::fprintf(f,"]}\n");
}
void emitCheckpoint(FILE* f,unsigned id,const Recorder& r) {
 std::fprintf(f,"{\"scenario\":%u,\"label\":\"%s\",\"count\":%d",id,r.test.name,r.sortCount);
 const char* names[]={"sortInputRecords","postSortRecords","postWaveRecords"};
 for(unsigned phase=0;phase<3;++phase){std::fprintf(f,",\"%s\":[",names[phase]);for(unsigned i=0;i<3;++i){std::fprintf(f,"%s[",i?",":"");for(unsigned j=0;j<14;++j)std::fprintf(f,"%s%u",j?",":"",r.checkpoints[phase][i*14+j]);std::fprintf(f,"]");}std::fprintf(f,"]");}
 std::fprintf(f,",\"selected\":{");const char* fields[]={"positions","colors","directions"};
 for(unsigned kind=0;kind<3;++kind){std::fprintf(f,"%s\"%s\":[",kind?",":"",fields[kind]);for(unsigned k=0;k<3;++k){const uint32_t address=kind==0?positions[k]:kind==1?colors[k]:directions[k];std::fprintf(f,"%s[",k?",":"");for(unsigned c=0;c<3;++c)std::fprintf(f,"%s%u",c?",":"",bits(r.state.at(address+c*4)));std::fprintf(f,"]");}std::fprintf(f,"]");}
 std::fprintf(f,",\"parameters\":[%u,%u,%u]},\"dispatchMutatesSelected\":%s}\n",bits(r.state.at(parameters[0])),bits(r.state.at(parameters[1])),bits(r.state.at(parameters[2])),r.test.script&dispatchMutation?"true":"false");
}
bool patchMath(uintptr_t b) {return world::redirect(b+0x1839,reinterpret_cast<uintptr_t>(init))&&world::redirect(b+0x19c9,reinterpret_cast<uintptr_t>(access))&&world::redirect(b+0x1811,reinterpret_cast<uintptr_t>(normalize))&&world::redirect(b+0x17f8,reinterpret_cast<uintptr_t>(angular))&&world::redirect(b+0x11e0,reinterpret_cast<uintptr_t>(magnitude))&&world::redirect(b+0x101e,reinterpret_cast<uintptr_t>(rotate))&&world::redirect(b+0x95c34,reinterpret_cast<uintptr_t>(sine))&&world::redirect(b+0x1393,reinterpret_cast<uintptr_t>(wave));}
struct ReplayWindow {uint32_t rva;std::vector<uint8_t> data;};
bool loadReplay(const char* path,std::vector<ReplayWindow>& windows,Case& c){
 FILE* f=std::fopen(path,"rb");if(!f)return false;uint32_t header[8];bool valid=std::fread(header,4,8,f)==8&&header[0]==0x344c4553&&header[1]==1&&header[4]==0x027f&&header[7]==25;
 const uint32_t expected[][2]={{0x177f88,4},{0x17810c,176},{0x184c98,12},{0x184770,4},{0x184c90,4},{0x170174,48},{0x1701b4,48},{0x1701f4,40},{0x170224,16},{0x176370,4},{0x184af0,4},{0x177f84,4},{0x170354,4},{0x1855f0,4},{0x1855f4,4},{0x17028c,204},{0x17d950,4},{0x17d908,4},{0x184900,4},{0x1851ec,4},{0x184298,4},{0x175d90,4},{0x175ee8,4},{0x1855cc,4},{0x1855dc,4}};
 for(unsigned i=0;valid&&i<25;++i){uint32_t shape[2];valid=std::fread(shape,4,2,f)==2&&shape[0]==expected[i][0]&&shape[1]==expected[i][1];if(valid){ReplayWindow w{shape[0],std::vector<uint8_t>(shape[1])};valid=std::fread(w.data.data(),1,w.data.size(),f)==w.data.size();if(valid)windows.push_back(std::move(w));}}
 if(valid)valid=std::fgetc(f)==EOF;std::fclose(f);if(!valid)return false;c.initialSW=static_cast<uint16_t>(header[5]);c.initialMX=header[6];return true;
}
void resetReplay(State s,const std::vector<ReplayWindow>& windows){for(const auto& w:windows)if(w.rva!=0x1855cc&&w.rva!=0x1855dc)std::memcpy(s.at(w.rva),w.data.data(),w.data.size());put(s.at(0x1855cc),reinterpret_cast<uintptr_t>(color));put(s.at(0x1855dc),reinterpret_cast<uintptr_t>(scalar));}
bool replay(uintptr_t base,const char* path,FILE* summary,FILE* originals,FILE* candidates){
 Case c{"native_replay"};c.real=true;c.realWave=true;std::vector<ReplayWindow> windows;if(!loadReplay(path,windows,c)){std::fprintf(summary,"nativeReplay result=BLOCKED invalid approved snapshot schema\n");return false;}
 char eax[32]{};if(GetEnvironmentVariableA("OPENBOXER_SELECTION_REPLAY_EAX",eax,sizeof(eax)))c.returnValue=static_cast<uint32_t>(std::stoul(eax,nullptr,16));
 State s{reinterpret_cast<uint8_t*>(base)};Recorder a(c,s),b(c,s);resetReplay(s,windows);dumpState("C:/Users/ADMIN/Boxer-lab/ms3d/selection-replay-input.bin",s,0,0);
 const auto ar=invoke(reinterpret_cast<Entry>(base+0x362a0),a);const auto expected=a.snapshot();dumpState("C:/Users/ADMIN/Boxer-lab/ms3d/selection-replay-original.bin",s,0,ar.value);emit(originals,0,a,ar);
 std::vector<std::vector<uint8_t>> results;for(const auto& w:windows)if(w.rva!=0x1855cc&&w.rva!=0x1855dc)results.emplace_back(static_cast<uint8_t*>(s.at(w.rva)),static_cast<uint8_t*>(s.at(w.rva))+w.data.size());
 resetReplay(s,windows);const unsigned before=count();const auto br=invoke(reinterpret_cast<Entry>(base+0x16e0),b);const bool route=count()==before+1;bool state=expected==b.snapshot();unsigned i=0;for(const auto& w:windows)if(w.rva!=0x1855cc&&w.rva!=0x1855dc){state=state&&std::memcmp(s.at(w.rva),results[i].data(),w.data.size())==0;++i;}
 dumpState("C:/Users/ADMIN/Boxer-lab/ms3d/selection-replay-candidate.bin",s,0,br.value);emit(candidates,0,b,br);
 const bool match=route&&state&&a.events==b.events&&ar.valid&&br.valid&&ar.value==br.value&&ar.cw==br.cw&&(ar.sw&63)==(br.sw&63)&&ar.mx==br.mx;
 std::fprintf(summary,"nativeReplay file=%s mode=%u count=%u route=%d state=%d trace=%d ABI=%d/%d EAX=%08x/%08x CW=%04x/%04x exceptions=%02x/%02x MXCSR=%08x/%08x result=%s\n",path,bits(s.at(0x184770)),bits(s.at(0x177f88)),route,state,a.events==b.events,ar.valid,br.valid,ar.value,br.value,ar.cw,br.cw,ar.sw&63,br.sw&63,ar.mx,br.mx,match?"PASS":"FAIL");std::fflush(summary);return match;
}
}
uint32_t offlineFixtures() {
 FILE* output=std::fopen("C:/Users/ADMIN/CLionProjects/OpenBoxer/validation/RENDER-0004-offline-checkpoints.jsonl","wb");
 std::vector<uint8_t> image(0x19f000,0);unsigned failures=0,id=0;for(auto c:cases()){Recorder r(c,{image.data()});reset(r);const auto result=invoke(offline,r);if(output)emitCheckpoint(output,++id,r);if(!oracle(r,result)){++failures;std::printf("offline FAIL name=%s valid=%d eax=%08x flags=%02x\n",c.name,result.valid,result.value,result.sw&63);}}if(output)std::fclose(output);return failures;
}
uint32_t fixtures(uintptr_t base) {
 FILE* summary=std::fopen("C:/Users/ADMIN/Boxer-lab/ms3d/selection-fixtures-summary.txt","wb");FILE* originals=std::fopen("C:/Users/ADMIN/Boxer-lab/ms3d/selection-original-trace.jsonl","wb");FILE* candidates=std::fopen("C:/Users/ADMIN/Boxer-lab/ms3d/selection-candidate-trace.jsonl","wb");if(!summary||!originals||!candidates)return 90;
 fixtureMode(true);unsigned failures=0,id=0;auto all=cases();
 char replayPath[1024]{};if(GetEnvironmentVariableA("OPENBOXER_SELECTION_REPLAY_FILE",replayPath,sizeof(replayPath))){if(!replay(base,replayPath,summary,originals,candidates))++failures;char only[8]{};if(GetEnvironmentVariableA("OPENBOXER_SELECTION_REPLAY_ONLY",only,sizeof(only))&&std::strcmp(only,"1")==0){std::fprintf(summary,"TOTAL replayPairs=1 failures=%u\n",failures);std::fclose(summary);std::fclose(originals);std::fclose(candidates);return failures;}}
 // Real CPU-only math first, before any helper is redirected. Wave is isolated
 // for the first four cases and retained for the final real-dependency case.
 std::vector<Case> real;for(int mode:{0,1,2,9}){Case c{"original_math"};c.mode=mode;c.real=true;real.push_back(c);}Case zero{"original_math_zero"};zero.real=true;zero.zero=true;real.push_back(zero);Case w{"original_math_wave"};w.real=true;w.realWave=true;real.push_back(w);
 auto pair=[&](Case c){++id;State s{reinterpret_cast<uint8_t*>(base)};Recorder a(c,s),b(c,s);reset(a);char path[256];std::snprintf(path,sizeof(path),"C:/Users/ADMIN/Boxer-lab/ms3d/selection-%02u-input.bin",id);dumpState(path,s,id,0);const auto ar=invoke(reinterpret_cast<Entry>(base+0x362a0),a);const auto state=a.snapshot();std::snprintf(path,sizeof(path),"C:/Users/ADMIN/Boxer-lab/ms3d/selection-%02u-original.bin",id);dumpState(path,s,id,ar.value);emit(originals,id,a,ar);reset(b);const auto before=count();const auto br=invoke(reinterpret_cast<Entry>(base+0x16e0),b);const bool route=count()==before+c.passes;const bool match=route&&ar.valid&&br.valid&&ar.value==br.value&&ar.cw==br.cw&&(ar.sw&63)==(br.sw&63)&&ar.mx==br.mx&&state==b.snapshot()&&a.events==b.events&&oracle(a,ar)&&oracle(b,br);if(!match)++failures;std::snprintf(path,sizeof(path),"C:/Users/ADMIN/Boxer-lab/ms3d/selection-%02u-candidate.bin",id);dumpState(path,s,id,br.value);emit(candidates,id,b,br);std::fprintf(summary,"scenario=%u name=%s mode=%d count=%d pattern=%u script=%u passes=%u realmath=%d realwave=%d route=%d state=%d trace=%d ABI=%d/%d EAX=%08x/%08x CW=%04x/%04x exceptions=%02x/%02x MXCSR=%08x/%08x result=%s\n",id,c.name,c.mode,c.count,c.pattern,c.script,c.passes,c.real,c.realWave,route,state==b.snapshot(),a.events==b.events,ar.valid,br.valid,ar.value,br.value,ar.cw,br.cw,ar.sw&63,br.sw&63,ar.mx,br.mx,match?"PASS":"FAIL");std::fprintf(summary," ESP=%08x/%08x:%08x/%08x NV=%08x/%08x/%08x/%08x:%08x/%08x/%08x/%08x\n",ar.abi.before,ar.abi.after,br.abi.before,br.abi.after,ar.abi.ebp,ar.abi.ebx,ar.abi.esi,ar.abi.edi,br.abi.ebp,br.abi.ebx,br.abi.esi,br.abi.edi);if(a.events!=b.events)for(unsigned i=0;i<(std::min)(a.events.size(),b.events.size());++i)if(a.events[i]!=b.events[i]){std::fprintf(summary,"firstDifference=%u original=%s candidate=%s\n",i,a.events[i].c_str(),b.events[i].c_str());break;}std::fflush(summary);};
 if(!world::redirect(base+0x1393,reinterpret_cast<uintptr_t>(wave)))return 91;
 for(auto c:real){if(c.realWave && !world::redirect(base+0x1393,base+0x36180))return 92;pair(c);}
 if(!patchMath(base))return 93;for(auto c:all)pair(c);
 std::fprintf(summary,"TOTAL scenarios=%u failures=%u\n",id,failures);std::fclose(summary);std::fclose(originals);std::fclose(candidates);return failures;
}
}
