#include "draw_trace.hpp"
#include <sstream>
namespace draw {
const char* apiName(Api a){static const char* names[]={"glIsEnabled","glMaterialfv","glMaterialf","glBindTexture","glEnable","glDisable","glBegin","glNormal3fv","glTexCoord2f","glVertex3fv","glEnd","material_vector","material_scalar"};return names[static_cast<unsigned>(a)];}
void Trace::region(const void* p,uint32_t size,const std::string& name){regions.push_back({reinterpret_cast<uintptr_t>(p),size,name});}
void Trace::registerModel(ModelView m,const std::string& p){
 region(m.data(),ModelView::size,p+"model");
 if(m.meshCount()<=0)return;
 region(m.meshes(),m.meshCount()*sizeof(Mesh),p+"meshes");region(m.materials(),m.materialCount()*sizeof(Material),p+"materials");
 region(m.triangles(),m.triangleCount()*sizeof(Triangle),p+"triangles");region(m.vertices(),m.vertexCount()*sizeof(Vertex),p+"vertices");
 for(int32_t i=0;i<m.meshCount();++i)if(m.mesh(i).membershipCount>0)region(m.mesh(i).memberships,m.mesh(i).membershipCount*4,p+"memberships"+std::to_string(i));
}
std::string Trace::role(const void* ptr)const{
 auto n=reinterpret_cast<uintptr_t>(ptr);for(const auto& r:regions)if(n>=r.base && n-r.base<r.size)return r.name+"+"+std::to_string(n-r.base);return "UNREGISTERED";
}
void Trace::append(Api api,const uint32_t* args,unsigned n,const uint32_t* pointer,unsigned words){
 Event e{api,{}, {},pointer?role(pointer):""};if(n)e.args.assign(args,args+n);if(words)e.payload.assign(pointer,pointer+words);++counts[static_cast<unsigned>(api)];
 const auto word=[&](uint32_t v){for(unsigned j=0;j<4;++j){digest^=(v>>(j*8))&255;digest*=1099511628211ULL;}};
 word(static_cast<unsigned>(api));for(auto a:e.args)word(a);for(auto a:e.payload)word(a);for(unsigned char c:e.role){digest^=c;digest*=1099511628211ULL;}events.push_back(std::move(e));
}
void Trace::writeEvents(FILE* f)const{
 std::fputs("[",f);bool first=true;for(const auto& e:events){if(!first)std::fputc(',',f);first=false;std::fprintf(f,"{\"api\":\"%s\",\"dispatch\":%u,\"result\":%u,\"args\":[",apiName(e.api),e.dispatch,e.result);for(unsigned i=0;i<e.args.size();++i)std::fprintf(f,"%s%u",i?",":"",e.args[i]);std::fprintf(f,"],\"role\":\"%s\",\"payload\":[",e.role.c_str());for(unsigned i=0;i<e.payload.size();++i)std::fprintf(f,"%s%u",i?",":"",e.payload[i]);std::fputs("]}",f);}std::fputs("]",f);
}
void snapshot(const char* path,ModelView model,const Dispatch& gl){
 FpPreserver fp;FILE* f=std::fopen(path,"wb");if(!f)return;
 // Version1: magic, version, counts[mesh,material,triangle,vertex], handles5,
 // full model (table pointers zeroed), meshes (membership pointers zeroed),
 // materials/triangles/vertices, then one count+membership payload per mesh.
 int32_t counts[]={model.meshCount(),model.materialCount(),model.triangleCount(),model.vertexCount()};uint32_t header[]={0x35575244,1};std::fwrite(header,4,2,f);std::fwrite(counts,4,4,f);
 for(unsigned i=0;i<4;++i){auto h=gl.handle(i);std::fwrite(&h,4,1,f);}auto h=gl.shininessHandle();std::fwrite(&h,4,1,f);
 std::vector<uint8_t> bytes(ModelView::size);std::memcpy(bytes.data(),model.data(),bytes.size());
 for(unsigned i=0;i<4;++i)writeWord(bytes.data()+ModelView::countsOffset+4+i*8,0);std::fwrite(bytes.data(),1,bytes.size(),f);
 if(counts[0]>0){std::vector<Mesh> meshes(model.meshes(),model.meshes()+counts[0]);for(auto& m:meshes)m.memberships=nullptr;std::fwrite(meshes.data(),sizeof(Mesh),meshes.size(),f);
  std::fwrite(model.materials(),sizeof(Material),counts[1],f);std::fwrite(model.triangles(),sizeof(Triangle),counts[2],f);std::fwrite(model.vertices(),sizeof(Vertex),counts[3],f);
  for(int32_t i=0;i<counts[0];++i){int32_t n=model.mesh(i).membershipCount;std::fwrite(&n,4,1,f);if(n>0)std::fwrite(model.mesh(i).memberships,4,n,f);}
 }std::fclose(f);
}
}
