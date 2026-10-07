#pragma once
#include "clip_layout.hpp"
#include "character_trace.hpp"
#include <array>
#include <vector>
namespace clip {
struct Fp {
    uint16_t cw=0,sw=0; uint8_t tag=0; uint32_t mx=0;
    bool operator==(const Fp &b) const { return cw==b.cw && sw==b.sw && tag==b.tag && mx==b.mx; }
};
inline Fp environment() { alignas(16) uint8_t e[512]; __asm__ volatile("fxsave %0":"=m"(e)); Fp f; std::memcpy(&f.cw,e,2); std::memcpy(&f.sw,e+2,2); f.tag=e[4]; f.mx=word(e+24); return f; }
inline void fpJson(FILE *f,const Fp &e) { std::fprintf(f,"{\"cw\":%u,\"sw\":%u,\"top\":%u,\"abridged_tag\":%u,\"mxcsr\":%u}",e.cw,e.sw,(e.sw>>11)&7,unsigned(e.tag),e.mx); }
inline void words(FILE *f,const uint8_t *m,unsigned count=28) { std::fputc('[',f); for(unsigned i=0;i<count;++i) std::fprintf(f,"%s%u",i?",":"",word(m+4*i)); std::fputc(']',f); }
inline void hex(FILE *f,const void *p,size_t size) { const auto *bytes=static_cast<const uint8_t*>(p); std::fputc('"',f); for(size_t i=0;i<size;++i) std::fprintf(f,"%02x",bytes[i]); std::fputc('"',f); }
struct CallbackEvent { bool lookup=false; int32_t index=0,result=0; uint32_t record=0; std::string name,request; bool operator==(const CallbackEvent &b) const { return lookup==b.lookup && index==b.index && result==b.result && record==b.record && name==b.name && request==b.request; } };
struct Events {
    std::vector<CallbackEvent> entries;
    void lookup(int32_t index,uint32_t record) { character::FpPreserver fp; entries.push_back({true,index,0,record,{},{}}); }
    void compare(const char *name,const char *request,int32_t result,uint32_t record) { character::FpPreserver fp; entries.push_back({false,0,result,record,name,request}); }
    void write(FILE *f) const { std::fputc('[',f); for(size_t i=0;i<entries.size();++i) { const auto &e=entries[i]; std::fprintf(f,"%s{\"name\":\"%s\",\"collection_role\":\"model+84\",\"record_role\":%u,\"index\":%d,\"result\":%d",i?",":"",e.lookup?"lookup":"compare",e.record,e.index,e.result); if(!e.lookup) { std::fputs(",\"record_name_hex\":",f); hex(f,e.name.data(),e.name.size()); std::fputs(",\"request_hex\":",f); hex(f,e.request.data(),e.request.size()); } std::fputc('}',f); } std::fputc(']',f); }
};
// Typed data only: normalized model pointer roles, raw animation data records,
// bounded request bytes, raw clock/return data. No executable data is captured.
struct CaptureHeader { uint32_t magic=0x32504c43,version=1,count=0,requestSize=0,clock=0,afterClock=0,eax=0,cw=0,sw=0; };
inline void normalize(uint8_t *m) { for(unsigned o:{12u,16u,20u,28u,32u,36u}) put(m+o,0); put(m+88,1); put(m+92,2); put(m+96,3); }
}
