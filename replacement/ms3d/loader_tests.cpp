#include "loader.hpp"
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <set>
#include <stdexcept>
static std::set<void*> live;
static unsigned allocation=0,failAt=0;
static void* __cdecl allocate(uint32_t size) {
    if(++allocation==failAt) return nullptr;
    void* value=std::malloc(size?size:1); if(!value) return nullptr;
    std::memset(value,0xa7,size?size:1); live.insert(value); return value;
}
static void __cdecl release(void* value) { if(value) { if(live.erase(value)!=1) std::abort(); std::free(value); } }
static void require(bool value,const char* reason) { if(!value) throw std::runtime_error(reason); }
static void put16(std::vector<uint8_t>& out,uint16_t value) { out.push_back(value&255); out.push_back(value>>8); }
static void putFloat(uint8_t* out,float value) { std::memcpy(out,&value,4); }
static int32_t integer(const uint8_t* out) { int32_t value; std::memcpy(&value,out,4); return value; }
static void* pointer(const uint8_t* out) { void* value; std::memcpy(&value,out,4); return value; }
int main() {
    try {
        std::vector<uint8_t> bytes(14); std::memcpy(bytes.data(),"MS3D000000",10); bytes[10]=4;
        put16(bytes,1); std::size_t vertex=bytes.size(); bytes.resize(vertex+15); bytes[vertex+13]=255; putFloat(bytes.data()+vertex+1,-17.5f);
        put16(bytes,1); std::size_t triangle=bytes.size(); bytes.resize(triangle+70);
        bytes[triangle+2]=255; bytes[triangle+3]=255;
        putFloat(bytes.data()+triangle+56,-2.0f); putFloat(bytes.data()+triangle+60,.25f); putFloat(bytes.data()+triangle+64,1.0f);
        put16(bytes,1); bytes.resize(bytes.size()+33); put16(bytes,1); put16(bytes,65535); bytes.push_back(0x80);
        put16(bytes,1); std::size_t material=bytes.size(); bytes.resize(material+361);
        for(unsigned i=0;i<72;i++) bytes[material+32+i]=static_cast<uint8_t>(i+1);
        std::memcpy(bytes.data()+material+105,".\\raw\\texture.bmp",18);
        ms3d::Parsed parsed; require(ms3d::parse(bytes,parsed),"synthetic valid parse");
        std::vector<uint8_t> object(ms3d::objectSize,0x5b); std::memset(object.data()+ms3d::fieldsOffset,0,32);
        const auto before=object;
        require(ms3d::install(object.data(),bytes,parsed,{allocate,release}),"install");
        const auto& f=ms3d::fields(object.data());
        require(f.vertices==1 && f.triangles==1 && f.groups==1 && f.materials==1,"counts");
        auto* v=static_cast<uint8_t*>(f.vertexData); require(v[0]==255 && v[1]==0xa7 && v[2]==0xa7 && v[3]==0xa7,"bone/padding");
        require(std::memcmp(v+4,bytes.data()+vertex+1,12)==0,"position bit copy");
        auto* t=static_cast<uint8_t*>(f.triangleData); float result[3]; std::memcpy(result,t+48,12);
        require(result[0]==3 && result[1]==.75f && result[2]==0 && integer(t+60)==65535,"UV/index arithmetic");
        require(t[72]==0xa7 && t[75]==0xa7,"triangle padding");
        auto* g=static_cast<uint8_t*>(f.groupData); require(integer(g)==-128 && integer(g+4)==1 && integer(static_cast<uint8_t*>(pointer(g+8)))==65535,"signed material/unsigned index");
        auto* m=static_cast<uint8_t*>(f.materialData); require(std::memcmp(m,bytes.data()+material+32,72)==0 && m[72]==0xa7 && m[75]==0xa7,"material bit copy/ID untouched");
        require(std::strcmp(static_cast<char*>(pointer(m+76)),".\\raw\\texture.bmp")==0,"raw filename");
        require(std::memcmp(object.data(),before.data(),ms3d::fieldsOffset)==0,"unrelated bytes");
        ms3d::cleanup(object.data(),{allocate,release}); require(live.empty(),"ownership cleanup");
        for(unsigned fail=1;fail<=7;fail++) {
            object=before; allocation=0; failAt=fail;
            require(!ms3d::install(object.data(),bytes,parsed,{allocate,release}),"injected allocation failure");
            require(object==before && live.empty(),"failure must preserve object and release staged buffers");
        }
        failAt=0;
        for(std::size_t length=0;length<bytes.size();length++) {
            std::vector<uint8_t> truncated(bytes.begin(),bytes.begin()+length); ms3d::Parsed unused;
            require(!ms3d::parse(truncated,unused),"truncation guard");
        }
        std::puts("PASS: independent synthetic payload, untouched bytes, ownership, seven allocation failures, truncation guards"); return 0;
    } catch(const std::exception& e) { std::fprintf(stderr,"FAIL: %s\n",e.what()); return 1; }
}
