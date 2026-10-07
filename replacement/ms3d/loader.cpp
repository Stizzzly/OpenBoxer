#include "loader.hpp"
#include <cstring>
#include <algorithm>
namespace ms3d {
namespace {
uint16_t u16(const uint8_t* p) { return uint16_t(p[0])|(uint16_t(p[1])<<8); }
void integer(uint8_t* p,int32_t v) { std::memcpy(p,&v,4); }
void pointer(uint8_t* p,void* v) { std::memcpy(p,&v,4); }
}
Fields& fields(void* object) { return *reinterpret_cast<Fields*>(static_cast<uint8_t*>(object)+fieldsOffset); }
bool parse(const std::vector<uint8_t>& bytes,Parsed& result) {
    Parsed p;
    std::size_t at=14;
    auto room=[&](std::size_t n) { return at<=bytes.size() && n<=bytes.size()-at; };
    auto count=[&](uint32_t& out) { if(!room(2)) return false; out=u16(bytes.data()+at); at+=2; return true; };
    if(bytes.size()<14 || !count(p.vertices)) return false;
    p.vertexOffset=at; if(!room(p.vertices*15)) return false; at+=p.vertices*15;
    if(!count(p.triangles)) return false;
    p.triangleOffset=at; if(!room(p.triangles*70)) return false; at+=p.triangles*70;
    uint32_t groups=0; if(!count(groups)) return false;
    for(uint32_t i=0;i<groups;i++) {
        if(!room(35)) return false;
        at+=33; uint32_t size=u16(bytes.data()+at); at+=2;
        if(!room(size*2+1)) return false;
        Group group; group.indices.reserve(size);
        for(uint32_t j=0;j<size;j++,at+=2) group.indices.push_back(u16(bytes.data()+at));
        group.material=static_cast<int8_t>(bytes[at++]); p.groups.push_back(std::move(group));
    }
    if(!count(p.materials)) return false;
    p.materialOffset=at; if(!room(p.materials*361)) return false;
    for(uint32_t i=0;i<p.materials;i++) if(!std::memchr(bytes.data()+at+i*361+105,0,128)) return false;
    result=std::move(p); return true;
}
bool install(void* object,const std::vector<uint8_t>& bytes,const Parsed& p,Allocator a) {
    std::vector<void*> owned;
    owned.reserve(5+p.groups.size()+p.materials);
    struct Guard { std::vector<void*>& pointers; Release release; bool committed=false; ~Guard() { if(!committed) for(auto it=pointers.rbegin();it!=pointers.rend();++it) release(*it); } } guard{owned,a.release};
    auto alloc=[&](uint32_t size)->uint8_t* { auto* v=static_cast<uint8_t*>(a.allocate(size)); if(v) owned.push_back(v); return v; };
    auto fail=[]() { return false; };
    auto* temporary=alloc(static_cast<uint32_t>(bytes.size())); if(!temporary) return fail();
    std::memcpy(temporary,bytes.data(),bytes.size());
    auto* vertices=alloc(p.vertices*16); if(!vertices) return fail();
    for(uint32_t i=0;i<p.vertices;i++) { auto* source=temporary+p.vertexOffset+i*15; auto* dest=vertices+i*16; dest[0]=source[13]; std::memcpy(dest+4,source+1,12); }
    auto* triangles=alloc(p.triangles*76); if(!triangles) return fail();
    for(uint32_t i=0;i<p.triangles;i++) {
        auto* source=temporary+p.triangleOffset+i*70; auto* dest=triangles+i*76;
        std::memcpy(dest,source+8,48);
        for(int j=0;j<3;j++) { float t; std::memcpy(&t,source+56+j*4,4); float transformed=static_cast<float>(1.0-static_cast<double>(t)); std::memcpy(dest+48+j*4,&transformed,4); integer(dest+60+j*4,u16(source+2+j*2)); }
    }
    auto* groups=alloc(static_cast<uint32_t>(p.groups.size()*12)); if(!groups) return fail();
    for(std::size_t i=0;i<p.groups.size();i++) {
        const auto& group=p.groups[i]; auto* indices=alloc(static_cast<uint32_t>(group.indices.size()*4)); if(!indices) return fail();
        for(std::size_t j=0;j<group.indices.size();j++) integer(indices+j*4,static_cast<int32_t>(group.indices[j]));
        integer(groups+i*12,group.material); integer(groups+i*12+4,static_cast<int32_t>(group.indices.size())); pointer(groups+i*12+8,indices);
    }
    auto* materials=alloc(p.materials*80); if(!materials) return fail();
    for(uint32_t i=0;i<p.materials;i++) {
        auto* source=temporary+p.materialOffset+i*361; auto* dest=materials+i*80;
        std::memcpy(dest,source+32,72);
        const auto length=std::strlen(reinterpret_cast<char*>(source+105))+1;
        auto* name=alloc(static_cast<uint32_t>(length)); if(!name) return fail();
        std::memcpy(name,source+105,length); pointer(dest+76,name);
    }
    Fields output{static_cast<int32_t>(p.groups.size()),groups,static_cast<int32_t>(p.materials),materials,static_cast<int32_t>(p.triangles),triangles,static_cast<int32_t>(p.vertices),vertices};
    std::memcpy(static_cast<uint8_t*>(object)+fieldsOffset,&output,sizeof(output));
    guard.committed=true;
    a.release(temporary); return true;
}
void cleanup(void* object,Allocator a) {
    auto& f=fields(object);
    for(int32_t i=0;i<f.groups;i++) { void* value; std::memcpy(&value,static_cast<uint8_t*>(f.groupData)+i*12+8,4); a.release(value); }
    for(int32_t i=0;i<f.materials;i++) { void* value; std::memcpy(&value,static_cast<uint8_t*>(f.materialData)+i*80+76,4); a.release(value); }
    a.release(f.groupData); a.release(f.materialData); a.release(f.triangleData); a.release(f.vertexData);
    f={};
}
}
