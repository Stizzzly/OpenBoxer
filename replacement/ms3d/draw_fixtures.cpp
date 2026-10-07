#include "draw_trace.hpp"
#include <windows.h>
#include <array>
#include <algorithm>
#include <cstdlib>
namespace draw {
namespace {
enum Mutation:unsigned { None,QueryModel,MaterialRelocate,ShaderLive,ScalarTexture,BindThreshold,EnableThreshold,BeginTables,NormalUvIndex,TexcoordVertices,NormalTriangles,VertexTriangles,VertexCount,EndCount,TextureState,AllSlots };
struct Case { const char* name;int32_t meshCount=1,members=1;uint32_t query=1,field=0x3f800000,texture=17;Mutation mutation=None;bool shared=false,reorder=false,guardOnly=false;int32_t material=0; };
uint32_t primary[13],alternate[13];
const Case cases[]={
 {"zero-disabled",0,1,0},{"zero-enabled",0,1,1},{"negative-disabled",-3,1,0},{"negative-enabled",-3,1,1},
 {"saved-al80",1,1,0x80},{"saved-upper100",1,1,0x100},{"empty-above",1,0},{"negative-members",1,-3},
 {"below",1,1,1,0x3ecccccc},{"equal",1,1,1,0x3ecccccd},{"nextabove",1,1,1,0x3eccccce},
 {"texture-zero",1,1,1,0x3f800000,0},{"texture-ffffffff",1,1,1,0x3f800000,0xffffffff},
 {"multiple",3,2},{"shared",3,2,1,0x3f800000,17,None,true},{"reorder-duplicate",1,4,1,0x3f800000,17,None,false,true},
 {"query-model",1,1,1,0x3f800000,17,QueryModel},{"material-relocate",1,1,1,0x3f800000,17,MaterialRelocate},
 {"shader-live",1,1,1,0x3f800000,17,ShaderLive},{"scalar-texture",1,1,1,0x3ecccccd,0,ScalarTexture},
 {"bind-threshold",1,1,1,0x3ecccccd,17,BindThreshold},{"enable-threshold",1,1,1,0x3f800000,17,EnableThreshold},
 {"begin-tables",1,1,1,0x3f800000,17,BeginTables},{"normal-uv-index",1,2,1,0x3f800000,17,NormalUvIndex},
 {"texcoord-vertices",1,2,1,0x3f800000,17,TexcoordVertices},{"normal-triangles",1,2,1,0x3f800000,17,NormalTriangles},
 {"vertex-triangles",1,2,1,0x3f800000,17,VertexTriangles},{"vertex-count",3,2,1,0x3f800000,17,VertexCount},
 {"end-count",3,2,1,0x3f800000,17,EndCount},{"texture-state",1,1,0x80,0x3f800000,17,TextureState},
 {"all-slots-live",1,2,1,0x3f800000,17,AllSlots},{"negative-material-preflight",1,1,1,0x3f800000,17,None,false,false,true,-1},
 {"material-out-of-range",1,1,1,0x3f800000,17,None,false,false,true,3},
 {"nonfinite-preflight",1,1,1,0x7fc00001,17,None,false,false,true}
};
struct Storage {
 std::vector<uint8_t> model=std::vector<uint8_t>(ModelView::size,0xcc);
 Material materials[2][3];Triangle triangles[2][3];Vertex vertices[2][9];Mesh meshes[2][3];int32_t lists[2][3][4];
 uint8_t* module;Case test;Trace trace;uint32_t enabled=0;
 Storage(uint8_t* b,Case c):module(b),test(c){
  for(unsigned bank=0;bank<2;++bank){
   for(unsigned i=0;i<3;++i){auto* words=reinterpret_cast<uint32_t*>(&materials[bank][i]);for(unsigned k=0;k<20;++k)words[k]=0x3f000000+bank*0x100000+i*0x10000+k*0x100;materials[bank][i].transparency=c.field;materials[bank][i].textureId=c.texture+i; if(c.texture==0 || c.texture==0xffffffff)materials[bank][i].textureId=c.texture;
    auto& t=triangles[bank][i];auto* tw=reinterpret_cast<uint32_t*>(&t);for(unsigned k=0;k<19;++k)tw[k]=0x3e000000+bank*0x100000+i*0x10000+k*0x100;t.vertices[0]=i*3;t.vertices[1]=i*3+1;t.vertices[2]=i*3+2;
    for(unsigned k=0;k<4;++k)lists[bank][i][k]=c.reorder?((k==0 || k==2)?2:(k==1?0:1)):(k%3);
    meshes[bank][i]={c.material<0 || c.guardOnly?c.material:(c.shared?0:static_cast<int32_t>(i)),c.members,lists[bank][i]};
   }
   for(unsigned i=0;i<9;++i){vertices[bank][i].unknown=0xdead0000+bank*0x100+i;for(unsigned k=0;k<3;++k)vertices[bank][i].position[k]=0x40000000+bank*0x100000+i*0x10000+k*0x100;}
  }
  ModelView m(model.data());m.setCounts(c.meshCount,3,3,9);m.setTables(meshes[0],materials[0],triangles[0],vertices[0]);
  trace.region(model.data(),ModelView::size,"model");for(unsigned bnk=0;bnk<2;++bnk){auto suffix=std::to_string(bnk);trace.region(materials[bnk],sizeof(materials[bnk]),"materials"+suffix);trace.region(meshes[bnk],sizeof(meshes[bnk]),"meshes"+suffix);trace.region(triangles[bnk],sizeof(triangles[bnk]),"triangles"+suffix);trace.region(vertices[bnk],sizeof(vertices[bnk]),"vertices"+suffix);for(unsigned i=0;i<3;++i)trace.region(lists[bnk][i],sizeof(lists[bnk][i]),"memberships"+suffix+"_"+std::to_string(i));}
  for(unsigned i=0;i<4;++i)writeWord(module+vectorHandles[i],0xab000000+i);writeWord(module+scalarHandle,0xcd000005);
 }
 ModelView view(){return ModelView(model.data());}
 std::vector<uint8_t> witness()const{
  auto result=model;for(unsigned i=0;i<4;++i)writeWord(result.data()+ModelView::countsOffset+4+i*8,0);
  const auto append=[&](const void* p,size_t size){auto first=static_cast<const uint8_t*>(p);result.insert(result.end(),first,first+size);};
  // Include every alternate/source bank, including inactive lifetimes.
  append(materials,sizeof(materials));append(triangles,sizeof(triangles));append(vertices,sizeof(vertices));
  for(unsigned bank=0;bank<2;++bank)for(auto mesh:meshes[bank]){mesh.memberships=nullptr;append(&mesh,sizeof(mesh));}append(lists,sizeof(lists));
  for(unsigned i=0;i<4;++i)append(module+vectorHandles[i],4);append(module+scalarHandle,4);
  // Include active table roles separately so relocations are observable.
  const ModelView m(const_cast<uint8_t*>(model.data()));for(const void* p:{static_cast<const void*>(m.meshes()),static_cast<const void*>(m.materials()),static_cast<const void*>(m.triangles()),static_cast<const void*>(m.vertices())}){auto role=trace.role(p);append(role.c_str(),role.size()+1);}
  for(unsigned bank=0;bank<2;++bank)for(const auto& mesh:meshes[bank]){auto role=trace.role(mesh.memberships);append(role.c_str(),role.size()+1);}
  for(unsigned i=0;i<13;++i){const uint32_t value=readWord(module+slots[i]);const uint32_t identity=value==primary[i]?0:value==alternate[i]?1:2;append(&identity,4);}
  return result;
 }
};
thread_local Storage* active;
thread_local uint8_t* recorderModule=nullptr;
struct ReplayRecorder { Trace trace;uint32_t query; };
thread_local ReplayRecorder* replayActive=nullptr;
void installRecorders(uint8_t* module,bool alt=false){for(unsigned i=0;i<13;++i)writeWord(module+slots[i],alt?alternate[i]:primary[i]);}
void mutate(Storage& s,Api api){
 auto m=s.view();const auto count=s.trace.counts[static_cast<unsigned>(api)];
 switch(s.test.mutation){
 case QueryModel:if(api==Api::IsEnabled){m.setCounts(2,3,3,9);m.setTables(s.meshes[1],s.materials[1],s.triangles[1],s.vertices[1]);}break;
 case MaterialRelocate:if(api==Api::Materialfv && count==1){s.meshes[0][0].material=2;m.setTables(s.meshes[0],s.materials[1],s.triangles[0],s.vertices[0]);}break;
 case ShaderLive:if(api==Api::ShaderVector){for(unsigned j=0;j<4;++j)writeWord(s.module+vectorHandles[j],0xef000000+count*16+j);writeWord(s.module+scalarHandle,0xee000000+count);for(unsigned j=0;j<3;++j){auto* words=reinterpret_cast<uint32_t*>(&s.materials[1][j]);for(unsigned k=0;k<17;++k)words[k]=0x41000000+count*0x10000+j*0x100+k;}m.setTables(m.meshes(),s.materials[1],m.triangles(),m.vertices());writeWord(s.module+slots[11],alternate[11]);writeWord(s.module+slots[12],alternate[12]);}break;
 case ScalarTexture:if(api==Api::ShaderScalar){m.material(0).textureId=0xffffffff;m.material(0).transparency=0x3eccccce;}break;
 case BindThreshold:if(api==Api::BindTexture)m.material(0).transparency=0x3eccccce;break;
 case EnableThreshold:if(api==Api::Enable)m.material(0).transparency=0x3ecccccd;break;
 case BeginTables:if(api==Api::Begin){s.meshes[1][0].membershipCount=2;s.lists[1][0][0]=2;s.lists[1][0][1]=1;m.setTables(s.meshes[1],m.materials(),m.triangles(),m.vertices());}break;
 case NormalUvIndex:if(api==Api::Normal){unsigned k=(count-1)%3;auto& t=s.triangles[0][(count-1)/3];t.vertices[k]=8;t.s[k]=0x80000000+count;t.t[k]=0x3d000000+count;}break;
 case TexcoordVertices:if(api==Api::TexCoord)m.setTables(m.meshes(),m.materials(),m.triangles(),s.vertices[1]);break;
 case NormalTriangles:if(api==Api::Normal)m.setTables(m.meshes(),m.materials(),s.triangles[1],m.vertices());break;
 case VertexTriangles:if(api==Api::Vertex)m.setTables(m.meshes(),m.materials(),s.triangles[1],m.vertices());break;
 case VertexCount:if(api==Api::Vertex)m.setCounts(1,3,3,9);break;
 case EndCount:if(api==Api::End)m.setCounts(1,3,3,9);break;
 case TextureState:s.enabled=0x55;break;
 case AllSlots:if(api==Api::IsEnabled)installRecorders(s.module,true);break;
 default:break;
 }
}
uint32_t event(Api api,const uint32_t* args,unsigned n,const uint32_t* p,unsigned words,unsigned variant){
 if(replayActive){FpPreserver fp;auto& r=*replayActive;r.trace.append(api,args,n,p,words);uint32_t result=0xc00d0000+static_cast<unsigned>(api);if(api==Api::IsEnabled)result=r.query;if(api==Api::Enable)result=0xe1234567;if(api==Api::Disable)result=0xd7654321;r.trace.events.back().dispatch=variant;r.trace.events.back().result=result;return result;}
 FpPreserver fp;auto& s=*active;s.trace.append(api,args,n,p,words);s.trace.events.back().dispatch=variant;mutate(s,api);
 uint32_t result=0xc00d0000+static_cast<unsigned>(api);if(api==Api::IsEnabled)result=s.test.query;if(api==Api::Enable){s.enabled=1;result=0xe1234567;}if(api==Api::Disable){s.enabled=0;result=0xd7654321;}s.trace.events.back().result=result;return result;
}
template<unsigned V>uint32_t __stdcall query(uint32_t a){return event(Api::IsEnabled,&a,1,nullptr,0,V);}
template<unsigned V>uint32_t __stdcall matv(uint32_t a,uint32_t b,const uint32_t* p){uint32_t args[]={a,b};return event(Api::Materialfv,args,2,p,4,V);}
template<unsigned V>uint32_t __stdcall matf(uint32_t a,uint32_t b,uint32_t c){uint32_t args[]={a,b,c};return event(Api::Materialf,args,3,nullptr,0,V);}
template<unsigned V>uint32_t __stdcall bind(uint32_t a,uint32_t b){uint32_t args[]={a,b};return event(Api::BindTexture,args,2,nullptr,0,V);}
template<unsigned V>uint32_t __stdcall enable(uint32_t a){return event(Api::Enable,&a,1,nullptr,0,V);}
template<unsigned V>uint32_t __stdcall disable(uint32_t a){return event(Api::Disable,&a,1,nullptr,0,V);}
template<unsigned V>uint32_t __stdcall begin(uint32_t a){return event(Api::Begin,&a,1,nullptr,0,V);}
template<unsigned V>uint32_t __stdcall normal(const uint32_t* p){return event(Api::Normal,nullptr,0,p,3,V);}
template<unsigned V>uint32_t __stdcall uv(uint32_t a,uint32_t b){uint32_t args[]={a,b};return event(Api::TexCoord,args,2,nullptr,0,V);}
template<unsigned V>uint32_t __stdcall vertex(const uint32_t* p){return event(Api::Vertex,nullptr,0,p,3,V);}
template<unsigned V>uint32_t __stdcall end(){return event(Api::End,nullptr,0,nullptr,0,V);}
template<unsigned V>uint32_t __stdcall vector(uint32_t a,uint32_t b,uint32_t c,uint32_t d,uint32_t e){uint32_t args[]={a,b,c,d,e};return event(Api::ShaderVector,args,5,nullptr,0,V);}
template<unsigned V>uint32_t __stdcall scalar(uint32_t a,uint32_t b){uint32_t args[]={a,b};return event(Api::ShaderScalar,args,2,nullptr,0,V);}
template<unsigned V>void addresses(uint32_t* p){const uintptr_t values[]={reinterpret_cast<uintptr_t>(&query<V>),reinterpret_cast<uintptr_t>(&matv<V>),reinterpret_cast<uintptr_t>(&matf<V>),reinterpret_cast<uintptr_t>(&bind<V>),reinterpret_cast<uintptr_t>(&enable<V>),reinterpret_cast<uintptr_t>(&disable<V>),reinterpret_cast<uintptr_t>(&begin<V>),reinterpret_cast<uintptr_t>(&normal<V>),reinterpret_cast<uintptr_t>(&uv<V>),reinterpret_cast<uintptr_t>(&vertex<V>),reinterpret_cast<uintptr_t>(&end<V>),reinterpret_cast<uintptr_t>(&vector<V>),reinterpret_cast<uintptr_t>(&scalar<V>)};for(unsigned i=0;i<13;++i)p[i]=values[i];}
uint32_t __attribute__((thiscall)) candidate(void* object){return render(ModelView(object),Dispatch{recorderModule});}
bool sameTrace(const Trace& a,const Trace& b){if(a.events.size()!=b.events.size())return false;for(unsigned i=0;i<a.events.size();++i){const auto& x=a.events[i];const auto& y=b.events[i];if(x.api!=y.api || x.args!=y.args || x.payload!=y.payload || x.role!=y.role || x.dispatch!=y.dispatch || x.result!=y.result)return false;}return true;}
void saveWitness(const char* path,const std::vector<uint8_t>& bytes){FILE* f=std::fopen(path,"wb");if(f){std::fwrite(bytes.data(),1,bytes.size(),f);std::fclose(f);}}
struct Result { Trace trace;AbiReport abi{};std::vector<uint8_t> pre,post;bool valid=true,immutable=true;uint32_t enabled=0,routeBefore=0,routeAfter=0; };
Result runSide(uint8_t* module,const Case& c,Entry entry,const char* directory,const char* side){
 Storage storage(module,c);active=&storage;recorderModule=module;installRecorders(module);Result r;r.pre=storage.witness();char path[512];std::snprintf(path,sizeof(path),"%s/draw-%s-%s-pre.bin",directory,c.name,side);snapshot(path,storage.view(),Dispatch{module});
 const uint16_t cw=0x027f;__asm__ volatile("fninit; fldcw %0"::"m"(cw));const uint32_t mx=0x1f80;__asm__ volatile("ldmxcsr %0"::"m"(mx));
 r.routeBefore=draw::count();if(c.guardOnly){r.valid=!supported(storage.view(),Dispatch{module},false);}
 else {draw_invoke(entry,storage.model.data(),&r.abi);r.valid=r.abi.beforeStack==r.abi.afterStack && r.abi.ebx==0x11223344 && r.abi.esi==0x22334455 && r.abi.edi==0x33445566 && r.abi.ebp==0x44556677 && r.abi.beforeCW==r.abi.afterCW && r.abi.beforeMX==r.abi.afterMX && r.abi.result==((c.query&255)?0xe1234567:0xd7654321);}
 r.routeAfter=draw::count();r.post=storage.witness();r.immutable=r.pre==r.post;if(c.mutation==None && !r.immutable)r.valid=false;r.enabled=storage.enabled;r.trace=std::move(storage.trace);
 std::snprintf(path,sizeof(path),"%s/draw-%s-%s-post.bin",directory,c.name,side);snapshot(path,storage.view(),Dispatch{module});
 std::snprintf(path,sizeof(path),"%s/draw-%s-%s-pre.witness",directory,c.name,side);saveWitness(path,r.pre);std::snprintf(path,sizeof(path),"%s/draw-%s-%s-post.witness",directory,c.name,side);saveWitness(path,r.post);
 active=nullptr;return r;
}
void writeResult(FILE* f,const char* side,const Result& r){
 const auto& a=r.abi;std::fprintf(f,"\"%s\":{\"valid\":%s,\"immutable\":%s,\"enabled\":%u,\"route_count_before\":%u,\"route_count_after\":%u,\"abi\":{\"stack_before\":%u,\"stack_after\":%u,\"stack_balanced\":%s,\"ebx\":%u,\"esi\":%u,\"edi\":%u,\"ebp\":%u,\"eax\":%u,\"cw_before\":%u,\"cw_after\":%u,\"sw_before\":%u,\"sw_after\":%u,\"mxcsr_before\":%u,\"mxcsr_after\":%u},\"events\":",side,r.valid?"true":"false",r.immutable?"true":"false",r.enabled,r.routeBefore,r.routeAfter,a.beforeStack,a.afterStack,a.beforeStack==a.afterStack?"true":"false",a.ebx,a.esi,a.edi,a.ebp,a.result,a.beforeCW,a.afterCW,a.beforeSW,a.afterSW,a.beforeMX,a.afterMX);r.trace.writeEvents(f);std::fputc('}',f);
}
struct ReplaySource {
 std::vector<uint8_t> model;std::vector<Mesh> meshes;std::vector<Material> materials;std::vector<Triangle> triangles;std::vector<Vertex> vertices;std::vector<std::vector<int32_t>> lists;uint32_t handles[5]{};
 bool load(const char* path){
  FILE* f=std::fopen(path,"rb");if(!f)return false;uint32_t header[2];int32_t counts[4];bool ok=std::fread(header,4,2,f)==2 && header[0]==0x35575244 && header[1]==1 && std::fread(counts,4,4,f)==4 && std::fread(handles,4,5,f)==5;
  if(!ok || counts[0]<0 || counts[0]>65536 || counts[1]<0 || counts[1]>65536 || counts[2]<0 || counts[2]>1000000 || counts[3]<0 || counts[3]>1000000){std::fclose(f);return false;}
  model.resize(ModelView::size);meshes.resize(counts[0]);materials.resize(counts[1]);triangles.resize(counts[2]);vertices.resize(counts[3]);lists.resize(counts[0]);
  const auto read=[&](void* data,size_t size,size_t count){return count==0 || std::fread(data,size,count,f)==count;};
  ok=read(model.data(),1,model.size());if(counts[0]>0)ok=ok && read(meshes.data(),sizeof(Mesh),meshes.size()) && read(materials.data(),sizeof(Material),materials.size()) && read(triangles.data(),sizeof(Triangle),triangles.size()) && read(vertices.data(),sizeof(Vertex),vertices.size());
  for(unsigned i=0;ok && i<meshes.size();++i){int32_t n;ok=read(&n,4,1) && n==meshes[i].membershipCount && n<=1000000;if(ok && n>0){lists[i].resize(n);ok=read(lists[i].data(),4,n);}meshes[i].memberships=lists[i].data();}
  ok=ok && std::fgetc(f)==EOF;std::fclose(f);if(!ok)return false;ModelView m(model.data());m.setCounts(counts[0],counts[1],counts[2],counts[3]);m.setTables(meshes.data(),materials.data(),triangles.data(),vertices.data());return true;
 }
 std::vector<uint8_t> witness()const{
  auto bytes=model;for(unsigned i=0;i<4;++i)writeWord(bytes.data()+ModelView::countsOffset+4+i*8,0);
  const auto append=[&](const void* p,size_t size){auto* b=static_cast<const uint8_t*>(p);bytes.insert(bytes.end(),b,b+size);};
  for(auto mesh:meshes){mesh.memberships=nullptr;append(&mesh,sizeof(mesh));}append(materials.data(),materials.size()*sizeof(Material));append(triangles.data(),triangles.size()*sizeof(Triangle));append(vertices.data(),vertices.size()*sizeof(Vertex));for(const auto& list:lists)append(list.data(),list.size()*4);return bytes;
 }
};
uint32_t replay(uint8_t* module,uintptr_t base,bool original,const char* directory){
 char sourcePath[512]{};if(!GetEnvironmentVariableA("OPENBOXER_DRAW_REPLAY",sourcePath,sizeof(sourcePath)))return 0;
 char queryText[32]{};uint32_t queryValue=0x80;if(GetEnvironmentVariableA("OPENBOXER_DRAW_REPLAY_QUERY",queryText,sizeof(queryText)))queryValue=static_cast<uint32_t>(std::strtoul(queryText,nullptr,0));
 FILE* report;char path[512];std::snprintf(path,sizeof(path),"%s/draw-replay.jsonl",directory);report=std::fopen(path,"wb");if(!report)return 41;
 Result results[2];const unsigned first=original?0:1;
 for(unsigned side=first;side<2;++side){ReplaySource source;if(!source.load(sourcePath)){std::fclose(report);return 42;}installRecorders(module);for(unsigned i=0;i<4;++i)writeWord(module+vectorHandles[i],source.handles[i]);writeWord(module+scalarHandle,source.handles[4]);
  ReplayRecorder recorder;recorder.query=queryValue;ModelView model(source.model.data());recorder.trace.registerModel(model);replayActive=&recorder;recorderModule=module;
  const char* name=side?"candidate":"original";std::snprintf(path,sizeof(path),"%s/draw-replay-%s-pre.bin",directory,name);snapshot(path,model,Dispatch{module});
  const uint16_t cw=0x027f;__asm__ volatile("fninit;fldcw %0"::"m"(cw));const uint32_t mx=0x1f80;__asm__ volatile("ldmxcsr %0"::"m"(mx));
  auto& r=results[side];Entry target=side?(original?reinterpret_cast<Entry>(base+0x1b54):&candidate):reinterpret_cast<Entry>(base+0x26b00);
  // Candidate's isolated dispatch does not depend on fixture Storage.
  if(!supported(model,Dispatch{module},false)){replayActive=nullptr;std::fclose(report);return 43;}
  r.pre=source.witness();r.routeBefore=draw::count();draw_invoke(target,source.model.data(),&r.abi);r.routeAfter=draw::count();replayActive=nullptr;r.post=source.witness();r.immutable=r.pre==r.post;r.trace=std::move(recorder.trace);r.valid=r.immutable && r.abi.beforeStack==r.abi.afterStack && r.abi.ebx==0x11223344 && r.abi.esi==0x22334455 && r.abi.edi==0x33445566 && r.abi.ebp==0x44556677 && r.abi.beforeCW==r.abi.afterCW && r.abi.beforeMX==r.abi.afterMX && r.abi.result==((queryValue&255)?0xe1234567:0xd7654321);
  if(original && side==1)r.valid=r.valid && r.routeAfter==r.routeBefore+1;
  std::snprintf(path,sizeof(path),"%s/draw-replay-%s-post.bin",directory,name);snapshot(path,model,Dispatch{module});
 }
 const bool equal=results[1].valid && (!original || (results[0].valid && sameTrace(results[0].trace,results[1].trace) && results[0].abi.afterSW==results[1].abi.afterSW && results[0].abi.result==results[1].abi.result));
 std::fprintf(report,"{\"case\":\"captured-native-source\",\"query\":%u,\"equal\":%s,",queryValue,equal?"true":"false");if(original){writeResult(report,"original",results[0]);std::fputc(',',report);}writeResult(report,"candidate",results[1]);std::fputs("}\n",report);std::fclose(report);return equal?0:44;
}
uint32_t suite(uintptr_t base,bool original){
 FpPreserver preserve;addresses<0>(primary);addresses<1>(alternate);std::vector<uint8_t> fake(original?0:0x19f000);uint8_t* module=original?reinterpret_cast<uint8_t*>(base):fake.data();
 uint32_t savedSlots[13],savedHandles[5];for(unsigned i=0;i<13;++i)savedSlots[i]=readWord(module+slots[i]);for(unsigned i=0;i<4;++i)savedHandles[i]=readWord(module+vectorHandles[i]);savedHandles[4]=readWord(module+scalarHandle);
 // IAT/opaque slots only; never shared DLL code. Main thread is parked by launcher.
 DWORD protections[13]{};if(original)for(unsigned i=0;i<13;++i)if(!VirtualProtect(module+slots[i],4,PAGE_READWRITE,&protections[i]))return 31;
 const char* directory=original?"C:/Users/ADMIN/Boxer-lab/ms3d":".";char path[512];std::snprintf(path,sizeof(path),"%s/draw-%s.jsonl",directory,original?"fixtures":"offline");FILE* f=std::fopen(path,"wb");if(!f)return 32;
 fixtureMode(true);uint32_t failures=0;
 for(const auto& c:cases){Result expected;if(original && !c.guardOnly)expected=runSide(module,c,reinterpret_cast<Entry>(base+0x26b00),directory,"original");Result actual=runSide(module,c,original?reinterpret_cast<Entry>(base+0x1b54):&candidate,directory,"candidate");bool equal=actual.valid;
  if(original && !c.guardOnly)equal=equal && actual.routeAfter==actual.routeBefore+1 && expected.routeAfter==expected.routeBefore && expected.valid && sameTrace(expected.trace,actual.trace) && expected.post==actual.post && expected.abi.result==actual.abi.result && expected.abi.afterSW==actual.abi.afterSW && expected.abi.afterMX==actual.abi.afterMX;
  if(c.guardOnly)equal=equal && actual.routeAfter==actual.routeBefore;
  if(!equal)++failures;std::fprintf(f,"{\"case\":\"%s\",\"mutation\":%u,\"mesh_count\":%d,\"membership_count\":%d,\"query\":%u,\"field\":%u,\"texture\":%u,\"guard_only\":%s,\"equal\":%s,",c.name,c.mutation,c.meshCount,c.members,c.query,c.field,c.texture,c.guardOnly?"true":"false",equal?"true":"false");if(original && !c.guardOnly){writeResult(f,"original",expected);std::fputc(',',f);}writeResult(f,"candidate",actual);std::fputs("}\n",f);std::fflush(f);
 }
 failures+=replay(module,base,original,directory);fixtureMode(false);std::fclose(f);for(unsigned i=0;i<13;++i){writeWord(module+slots[i],savedSlots[i]);if(original){DWORD unused;VirtualProtect(module+slots[i],4,protections[i],&unused);}}for(unsigned i=0;i<4;++i)writeWord(module+vectorHandles[i],savedHandles[i]);writeWord(module+scalarHandle,savedHandles[4]);return failures;
}
}
uint32_t fixtures(uintptr_t base){return suite(base,true);}uint32_t offlineFixtures(){return suite(0,false);}
}
