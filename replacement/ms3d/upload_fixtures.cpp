#include "upload.hpp"
#include "upload_abi.hpp"
#include "world_runtime.hpp"
#include <windows.h>
#include <array>
#include <vector>
#include <string>
#include <sstream>
#include <cstring>
#include <cstdio>
namespace upload {
namespace {
enum class Kind { nullImage,nullPixels,valid };
enum Mutation : unsigned { none=0,bindImage=1,minImage=2,maxImage=4,mipmapPixels=8,mipmapNull=16,bindID=32,parameterID=64,mipmapID=128,pixelReleaseID=256,recordReleaseID=512 };
struct Case { const char* name; Kind kind=Kind::valid; bool writes=true; uint32_t id=17; int32_t status=0,width=8,height=4; unsigned mutation=none,passes=1; bool nullFilename=false; };
struct Storage { uint32_t before=0x13572468; Image image{}; uint32_t after=0x24681357; };
static_assert(offsetof(Storage,image)==4 && sizeof(Storage)==20);
struct Recorder {
    Case test;
    Storage storage;
    std::array<std::array<uint8_t,32>,3> pixels{};
    const char* filename=".\\raw\\fixture.bmp";
    std::vector<std::string> events;
    uint32_t* output=nullptr;
    unsigned helperCalls=0,generated=0,mipmaps=0,released=0;
    bool valid=true;
    explicit Recorder(const Case& value):test(value) {
        storage.image={test.width,test.height,test.kind==Kind::nullPixels?nullptr:pixels[0].data()};
        for(unsigned i=0;i<3;i++) pixels[i].fill(static_cast<uint8_t>(0x91+i));
        if(test.nullFilename) filename=nullptr;
    }
    std::string pointer(const void* value) const {
        if(!value) return "null";
        if(value==&storage.image) return "record";
        if(value==output) return "localID";
        if(value==filename) return "filename";
        for(unsigned i=0;i<3;i++) if(value==pixels[i].data()) return "pixels"+std::to_string(i);
        return "UNKNOWN_POINTER";
    }
    std::string snapshot() const {
        std::ostringstream out;
        out<<"width="<<storage.image.width<<" height="<<storage.image.height<<" pixels="<<pointer(storage.image.pixels);
        // The retained local exists only inside the upload invocation. Never
        // dereference it after the function has returned.
        if(output) out<<" localID="<<*output;
        out<<" guards="<<std::hex<<storage.before<<'/'<<storage.after;
        return out.str();
    }
    void record(const std::string& name,const std::string& args) { events.push_back(name+"("+args+") | "+snapshot()); }
    bool intact() const {
        if(storage.before!=0x13572468 || storage.after!=0x24681357) return false;
        for(unsigned i=0;i<3;i++) for(const auto byte:pixels[i]) if(byte!=0x91+i) return false;
        return true;
    }
};
thread_local Recorder* current=nullptr;
Image* __cdecl decode(const char* filename) {
    auto& r=*current; r.valid=r.valid && filename==r.filename; ++r.helperCalls;
    r.record("BMP",r.pointer(filename)+",path="+(filename?std::string(filename):"(null)")+",result="+(r.test.kind==Kind::nullImage?"null":"record"));
    return r.test.kind==Kind::nullImage?nullptr:&r.storage.image;
}
void __stdcall generate(int32_t n,uint32_t* id) {
    auto& r=*current; r.output=id; ++r.generated; r.valid=r.valid && n==1 && *id==0;
    r.record("glGenTextures",std::to_string(n)+",localID,initial="+std::to_string(*id)+",write="+std::to_string(r.test.writes)+",generated="+std::to_string(r.test.id));
    if(r.test.writes) *id=r.test.id;
}
void __stdcall bind(uint32_t target,uint32_t id) {
    auto& r=*current; r.valid=r.valid && target==0x0de1;
    r.record("glBindTexture",std::to_string(target)+","+std::to_string(id));
    if(r.test.mutation&bindImage) r.storage.image={-7,9,r.pixels[1].data()};
    if(r.test.mutation&bindID) *r.output=0x12345678;
}
void __stdcall parameter(uint32_t target,uint32_t pname,int32_t value) {
    auto& r=*current; r.valid=r.valid && target==0x0de1 && ((pname==0x2801 && value==0x2703) || (pname==0x2800 && value==0x2601));
    r.record("glTexParameteri",std::to_string(target)+","+std::to_string(pname)+","+std::to_string(value));
    if((pname==0x2801 && (r.test.mutation&minImage)) || (pname==0x2800 && (r.test.mutation&maxImage))) r.storage.image={pname==0x2801?777:333,pname==0x2801?-123:-222,r.pixels[1].data()};
    if(r.test.mutation&parameterID) *r.output=0x87654321;
}
int32_t __stdcall mipmap(uint32_t target,int32_t components,int32_t width,int32_t height,uint32_t format,uint32_t type,const void* pixels) {
    auto& r=*current; ++r.mipmaps;
    r.valid=r.valid && target==0x0de1 && components==3 && format==0x1907 && type==0x1401 && width==r.storage.image.width && height==r.storage.image.height && pixels==r.storage.image.pixels;
    r.record("gluBuild2DMipmaps",std::to_string(target)+","+std::to_string(components)+","+std::to_string(width)+","+std::to_string(height)+","+std::to_string(format)+","+std::to_string(type)+","+r.pointer(pixels)+",status="+std::to_string(r.test.status));
    if(r.test.mutation&mipmapPixels) r.storage.image.pixels=r.pixels[2].data();
    if(r.test.mutation&mipmapNull) r.storage.image.pixels=nullptr;
    if(r.test.mutation&mipmapID) *r.output=0xfedcba98;
    return r.test.status;
}
void __cdecl release(void* pointer) {
    auto& r=*current;
    const bool pixelRelease=(r.released%2)==0;
    r.valid=r.valid && pointer==(pixelRelease?r.storage.image.pixels:static_cast<void*>(&r.storage.image));
    r.record("release",r.pointer(pointer)); ++r.released;
    if(pixelRelease && (r.test.mutation&pixelReleaseID)) *r.output=0x11223344;
    if(!pixelRelease && (r.test.mutation&recordReleaseID)) *r.output=0xabcdef01;
}
Callbacks callbacks() { return {decode,generate,bind,parameter,mipmap,release}; }
uint32_t __cdecl offline(const char* filename) { return run(filename,callbacks()); }
std::vector<Case> cases() {
    return {
        {"null_image",Kind::nullImage},{"null_filename",Kind::nullImage,true,17,0,8,4,none,1,true},
        {"null_pixels",Kind::nullPixels},{"generated_id"},{"unchanged_output",Kind::valid,false},
        {"generated_zero",Kind::valid,true,0},{"generated_FFFFFFFF",Kind::valid,true,0xffffffff},
        {"positive_mipmap_status",Kind::valid,true,19,7},{"negative_mipmap_status",Kind::valid,true,20,-7},
        {"repeated",Kind::valid,true,21,0,8,4,none,2},{"signed_dimensions",Kind::valid,true,22,0,-4096,0},
        {"bind_changes_image",Kind::valid,true,23,0,8,4,bindImage},
        {"min_changes_image",Kind::valid,true,24,0,8,4,minImage},
        {"max_changes_image",Kind::valid,true,25,0,8,4,maxImage},
        {"mipmap_changes_pixels",Kind::valid,true,26,0,8,4,mipmapPixels},
        {"mipmap_null_pixels",Kind::valid,true,27,0,8,4,mipmapNull},
        {"bind_changes_ID",Kind::valid,true,28,0,8,4,bindID},
        {"parameter_changes_ID",Kind::valid,true,29,0,8,4,parameterID},
        {"mipmap_changes_ID",Kind::valid,true,30,0,8,4,mipmapID},
        {"pixel_release_changes_ID",Kind::valid,true,31,0,8,4,pixelReleaseID},
        {"record_release_changes_ID",Kind::valid,true,32,0,8,4,recordReleaseID},
        {"combined_live_reads",Kind::valid,true,33,-11,8,4,bindImage|minImage|maxImage|mipmapPixels|recordReleaseID}
    };
}
uint32_t expected(const Case& test) {
    if(test.kind!=Kind::valid) return 0;
    uint32_t id=test.writes?test.id:0;
    if(test.mutation&bindID) id=0x12345678;
    if(test.mutation&parameterID) id=0x87654321;
    if(test.mutation&mipmapID) id=0xfedcba98;
    if(test.mutation&pixelReleaseID) id=0x11223344;
    if(test.mutation&recordReleaseID) id=0xabcdef01;
    return id;
}
bool oracle(const Recorder& r,uint32_t result) {
    if(!r.valid || !r.intact() || result!=expected(r.test) || r.helperCalls!=r.test.passes) return false;
    int32_t width=r.test.width,height=r.test.height;
    const void* pixels=r.test.kind==Kind::nullPixels?nullptr:r.pixels[0].data();
    if(r.test.mutation&bindImage) { width=-7; height=9; pixels=r.pixels[1].data(); }
    if(r.test.mutation&minImage) { width=777; height=-123; pixels=r.pixels[1].data(); }
    if(r.test.mutation&maxImage) { width=333; height=-222; pixels=r.pixels[1].data(); }
    if(r.test.mutation&mipmapPixels) pixels=r.pixels[2].data();
    if(r.test.mutation&mipmapNull) pixels=nullptr;
    if(r.storage.image.width!=width || r.storage.image.height!=height || r.storage.image.pixels!=pixels) return false;
    const bool valid=r.test.kind==Kind::valid;
    if(r.generated!=(valid?r.test.passes:0) || r.mipmaps!=(valid?r.test.passes:0) || r.released!=(valid?r.test.passes*2:0)) return false;
    std::vector<std::string> names;
    for(const auto& event:r.events) names.push_back(event.substr(0,event.find('(')));
    std::vector<std::string> expectedNames;
    for(unsigned pass=0;pass<r.test.passes;pass++) {
        expectedNames.push_back("BMP");
        if(valid) expectedNames.insert(expectedNames.end(),{"glGenTextures","glBindTexture","glTexParameteri","glTexParameteri","gluBuild2DMipmaps","release","release"});
    }
    return names==expectedNames;
}
struct Result { uint32_t value=0; bool abi=true,fp=true; WorldAbiReport registers{}; uint16_t cwBefore=0,cwAfter=0; uint32_t mxBefore=0,mxAfter=0; };
Result invoke(Entry callable,Recorder& r) {
    current=&r; Result result;
    for(unsigned pass=0;pass<r.test.passes;pass++) {
        r.output=nullptr;
        __asm__ volatile("fnstcw %0":"=m"(result.cwBefore)); __asm__ volatile("stmxcsr %0":"=m"(result.mxBefore));
        result.value=upload_abi_probe(callable,r.filename,&result.registers);
        __asm__ volatile("fnstcw %0":"=m"(result.cwAfter)); __asm__ volatile("stmxcsr %0":"=m"(result.mxAfter));
        r.output=nullptr;
        result.abi=result.abi && worldAbiValid(result.registers);
        result.fp=result.fp && result.cwBefore==result.cwAfter && (result.mxBefore&0xffc0)==(result.mxAfter&0xffc0);
    }
    current=nullptr; return result;
}
void emit(FILE* file,unsigned id,const Recorder& r) {
    for(unsigned i=0;i<r.events.size();i++) std::fprintf(file,"scenario=%u sequence=%u %s\n",id,i,r.events[i].c_str());
    std::fprintf(file,"scenario=%u FINAL %s intact=%d\n",id,r.snapshot().c_str(),r.intact());
}
template<class Function> bool import(uintptr_t module,uint32_t rva,Function function) {
    auto* pointer=reinterpret_cast<Function*>(module+rva); DWORD old;
    if(!VirtualProtect(pointer,4,PAGE_READWRITE,&old)) return false;
    *pointer=function; DWORD ignored; return VirtualProtect(pointer,4,old,&ignored)!=0;
}
}
uint32_t offlineFixtures() {
    unsigned failures=0;
    for(const auto& test:cases()) { Recorder recorder(test); const auto result=invoke(offline,recorder); if(!oracle(recorder,result.value) || !result.abi || !result.fp) ++failures; }
    return failures;
}
uint32_t fixtures(uintptr_t module) {
    const auto inputs=cases();
    FILE* summary=std::fopen("C:/Users/ADMIN/Boxer-lab/ms3d/upload-fixtures-summary.txt","wb");
    FILE* originals=std::fopen("C:/Users/ADMIN/Boxer-lab/ms3d/upload-original-trace.txt","wb");
    FILE* candidates=std::fopen("C:/Users/ADMIN/Boxer-lab/ms3d/upload-candidate-trace.txt","wb");
    if(!summary || !originals || !candidates) return 90;
    if(!world::redirect(module+0x1aaf,reinterpret_cast<uintptr_t>(&decode)) || !world::redirect(module+0x96d10,reinterpret_cast<uintptr_t>(&release)) ||
       !import(module,0x18ca5c,&generate) || !import(module,0x18ca58,&bind) || !import(module,0x18ca54,&parameter) || !import(module,0x18c880,&mipmap) || !install(module,"replace")) return 91;
    fixtureMode(true);
    const auto original=reinterpret_cast<Entry>(module+0x25420),candidate=reinterpret_cast<Entry>(module+0x1799);
    unsigned failures=0,id=0;
    for(const auto& test:inputs) {
        ++id; Recorder a(test),b(test); const auto ar=invoke(original,a); const unsigned count=replacementCount(); const auto br=invoke(candidate,b);
        const bool route=replacementCount()==count+test.passes,trace=a.events==b.events,state=a.snapshot()==b.snapshot();
        const bool exact=route && trace && state && ar.value==br.value && oracle(a,ar.value) && oracle(b,br.value) && ar.abi && br.abi && ar.fp && br.fp;
        if(!exact) ++failures; emit(originals,id,a); emit(candidates,id,b);
        std::fprintf(summary,"scenario=%u name=%s passes=%u originalEAX=%08x candidateEAX=%08x route=%d trace=%d state=%d oracle=%d/%d intact=%d/%d ABI=%d/%d FP=%d/%d result=%s\n",id,test.name,test.passes,ar.value,br.value,route,trace,state,oracle(a,ar.value),oracle(b,br.value),a.intact(),b.intact(),ar.abi,br.abi,ar.fp,br.fp,exact?"PASS":"FAIL");
        std::fprintf(summary," abi originalESP=%08x/%08x candidateESP=%08x/%08x originalNV=%08x/%08x/%08x/%08x candidateNV=%08x/%08x/%08x/%08x x87=%04x/%04x,%04x/%04x MXCSR=%08x/%08x,%08x/%08x\n",ar.registers.before,ar.registers.after,br.registers.before,br.registers.after,ar.registers.ebp,ar.registers.ebx,ar.registers.esi,ar.registers.edi,br.registers.ebp,br.registers.ebx,br.registers.esi,br.registers.edi,ar.cwBefore,ar.cwAfter,br.cwBefore,br.cwAfter,ar.mxBefore,ar.mxAfter,br.mxBefore,br.mxAfter);
        if(!trace) for(unsigned i=0;i<(std::min)(a.events.size(),b.events.size());i++) if(a.events[i]!=b.events[i]) { std::fprintf(summary," firstDifference=%u original=%s candidate=%s\n",i,a.events[i].c_str(),b.events[i].c_str()); break; }
        std::fflush(summary);
    }
    std::fprintf(summary,"TOTAL scenarios=%u failures=%u\n",id,failures); std::fclose(summary); std::fclose(originals); std::fclose(candidates); return failures;
}
}
