#include "loader.hpp"
#include "approved_fixtures.hpp"
#include "world.hpp"
#include "texture.hpp"
#include "upload.hpp"
#include "lighting.hpp"
#include "selection.hpp"
#include "draw.hpp"
#include "character.hpp"
#include "animation.hpp"
#include "clip.hpp"
#include "frames.hpp"
#include "strike.hpp"
#include "character_replay.hpp"
#include "runtime_options.hpp"
#include <windows.h>
#include <wincrypt.h>
#include <cstdio>
#include <cstring>
#include <string>
#include <cfenv>
#include <xmmintrin.h>
#include <psapi.h>
using Loader=bool (__attribute__((thiscall)) *)(void*,const char*);
using Constructor=void* (__attribute__((thiscall)) *)(void*);
static uintptr_t base;
static Loader original;
static ms3d::Allocator allocator;
static std::string mode;
static bool installed=false;
static unsigned replacementCalls=0,fallbackCalls=0,passThroughCalls=0;
static bool comparePayload(void*,void*);
static bool hash(const std::vector<uint8_t>& bytes,const char* expected) {
    HCRYPTPROV provider=0; HCRYPTHASH digest=0;
    if(!CryptAcquireContextA(&provider,nullptr,nullptr,PROV_RSA_AES,CRYPT_VERIFYCONTEXT)) return false;
    bool ok=CryptCreateHash(provider,CALG_SHA_256,0,0,&digest)!=0;
    if(ok) ok=CryptHashData(digest,bytes.data(),static_cast<DWORD>(bytes.size()),0)!=0;
    BYTE value[32]{}; DWORD size=32;
    if(ok) ok=CryptGetHashParam(digest,HP_HASHVAL,value,&size,0)!=0;
    char text[65]{}; for(int i=0;i<32;i++) std::sprintf(text+i*2,"%02x",value[i]);
    if(digest) CryptDestroyHash(digest); CryptReleaseContext(provider,0);
    return ok && std::strcmp(text,expected)==0;
}
static bool read(const char* filename,std::vector<uint8_t>& bytes) {
    FILE* file=std::fopen(filename,"rb"); if(!file) return false;
    if(std::fseek(file,0,SEEK_END)!=0) { std::fclose(file); return false; }
    const long length=std::ftell(file);
    if(length<0 || length>64*1024*1024 || std::fseek(file,0,SEEK_SET)!=0) { std::fclose(file); return false; }
    bytes.resize(static_cast<std::size_t>(length));
    const bool ok=std::fread(bytes.data(),1,bytes.size(),file)==bytes.size();
    return std::fclose(file)==0 && ok;
}
static void log(const char* text) {
    if(!runtime_options::diagnostics(runtime_options::Unit::Io)) return;
    FILE* file=std::fopen("C:/Users/ADMIN/Boxer-lab/ms3d/ms3d-replacement.log","ab");
    if(file) { std::fprintf(file,"%s\n",text); std::fclose(file); }
}
static bool candidate(void* object,const char* filename) {
    const auto fallback=[&]() { ++fallbackCalls; return original(object,filename); };
    if(!object || !filename) return fallback();
    for(const auto* c=reinterpret_cast<const unsigned char*>(filename);*c;c++) if(*c>=128) return fallback();
    char filenameLog[512]; std::snprintf(filenameLog,sizeof(filenameLog),"IO-0003 filename=%s",filename); log(filenameLog);
    const auto& f=ms3d::fields(object);
    const uint8_t* fieldBytes=reinterpret_cast<const uint8_t*>(&f);
    for(std::size_t i=0;i<sizeof(f);i++) if(fieldBytes[i]) return fallback();
    if(std::fegetround()!=FE_TONEAREST || (_mm_getcsr()&0x6000)!=0) return fallback();
    try {
        std::vector<uint8_t> bytes;
        ms3d::Parsed p;
        bool approved=false;
        if(read(filename,bytes)) for(const auto& fixture:approvedFixtures) if(bytes.size()==fixture.length && hash(bytes,fixture.sha256)) { approved=true; break; }
        if(!approved || !ms3d::parse(bytes,p)) {
            log("fallback: unsupported fixture/path"); return fallback();
        }
        if(!ms3d::install(object,bytes,p,allocator)) { log("fallback: staged allocation failed"); return fallback(); }
        ++replacementCalls; log("replacement: IO-0003 approved fixture loaded"); return true;
    } catch(...) { log("fallback: independent reader exception before commit"); return fallback(); }
}
static bool __attribute__((thiscall)) hook(void* object,const char* filename) {
    runtime_options::Timing timing(runtime_options::Unit::Io);
    if(mode=="pass-through") { ++passThroughCalls; log("pass-through: original loader"); return original(object,filename); }
    if(mode=="shadow") {
        bool empty=object!=nullptr;
        if(empty) { const auto* bytes=reinterpret_cast<const uint8_t*>(&ms3d::fields(object)); for(std::size_t i=0;i<32;i++) if(bytes[i]) empty=false; }
        const bool answer=original(object,filename);
        if(empty && filename) {
            void* scratch=allocator.allocate(ms3d::objectSize);
            if(scratch) {
                reinterpret_cast<Constructor>(base+0x15890)(scratch);
                const bool shadowAnswer=candidate(scratch,filename);
                log(answer==shadowAnswer && comparePayload(object,scratch)?"shadow: payload MATCH, original object retained":"shadow: payload DIFFERENCE, original object retained");
                if(shadowAnswer) ms3d::cleanup(scratch,allocator);
                allocator.release(scratch);
            } else log("shadow: skipped scratch allocation failure");
        } else log("shadow: original retained, unsupported nonempty object");
        return answer;
    }
    return candidate(object,filename);
}
extern "C" __declspec(dllexport) DWORD WINAPI ms3d_bootstrap(void* requestedMode) {
    try {
        runtime_options::configure();
        if(reinterpret_cast<uintptr_t>(requestedMode)>=3)for(unsigned unit=0;unit<unsigned(runtime_options::Unit::Count);++unit)if(runtime_options::original(static_cast<runtime_options::Unit>(unit)))return 26;
        if(installed) return 0;
        char modulePath[MAX_PATH]{}; GetModuleFileNameA(nullptr,modulePath,MAX_PATH);
        if(_stricmp(modulePath,"C:\\Users\\ADMIN\\Boxer-lab\\ms3d\\program.exe")!=0) return 10;
        std::vector<uint8_t> moduleBytes;
        if(!read(modulePath,moduleBytes) || !hash(moduleBytes,"77f9ac7b4c4517f7d1776d7c2afdac19440169977bdc5393f58a1a0fbb7bd7d6")) return 11;
        base=reinterpret_cast<uintptr_t>(GetModuleHandleA(nullptr));
        MODULEINFO information{};
        if(!GetModuleInformation(GetCurrentProcess(),reinterpret_cast<HMODULE>(base),&information,sizeof(information)) || information.SizeOfImage!=0x19f000) return 17;
        char selected[32]{}; GetEnvironmentVariableA("OPENBOXER_MS3D_MODE",selected,sizeof(selected)); mode=selected;
        if(reinterpret_cast<uintptr_t>(requestedMode)>=3 && reinterpret_cast<uintptr_t>(requestedMode)<=21) mode="replace";
        if(mode.empty()) mode="shadow";
        if(mode!="pass-through" && mode!="shadow" && mode!="replace") return 12;
        allocator={reinterpret_cast<ms3d::Allocate>(base+0x95420),reinterpret_cast<ms3d::Release>(base+0x958b0)};
        auto* slot=reinterpret_cast<Loader*>(base+0x162210);
        if(reinterpret_cast<uintptr_t>(*slot)!=base+0x1c21) return 13;
        original=*slot;
        DWORD old=0;
        if(!VirtualProtect(slot,sizeof(*slot),PAGE_READWRITE,&old)) return 14;
        if(!runtime_options::original(runtime_options::Unit::Io)) *slot=hook;
        DWORD unused; if(!VirtualProtect(slot,sizeof(*slot),old,&unused)) return 15;
        installed=true;
        if(reinterpret_cast<uintptr_t>(requestedMode)!=3) {
            char worldMode[32]{}; GetEnvironmentVariableA("OPENBOXER_WORLD_MODE",worldMode,sizeof(worldMode));
            const char* selected=worldMode[0]?worldMode:mode.c_str();
            if(!runtime_options::original(runtime_options::Unit::World) && !world::install(base,selected)) return 18;
            char textureMode[32]{}; GetEnvironmentVariableA("OPENBOXER_TEXTURE_MODE",textureMode,sizeof(textureMode));
            if(!runtime_options::original(runtime_options::Unit::Texture) && !texture::install(base,textureMode[0]?textureMode:mode.c_str())) return 19;
            char uploadMode[32]{}; GetEnvironmentVariableA("OPENBOXER_UPLOAD_MODE",uploadMode,sizeof(uploadMode));
            if(!runtime_options::original(runtime_options::Unit::Upload) && !upload::install(base,uploadMode[0]?uploadMode:mode.c_str())) return 21;
            char lightingMode[32]{}; GetEnvironmentVariableA("OPENBOXER_LIGHTING_MODE",lightingMode,sizeof(lightingMode));
            if(!runtime_options::original(runtime_options::Unit::Lighting) && !lighting::install(base,lightingMode[0]?lightingMode:mode.c_str())) return 22;
            char selectionMode[32]{}; GetEnvironmentVariableA("OPENBOXER_SELECTION_MODE",selectionMode,sizeof(selectionMode));
            if(!runtime_options::original(runtime_options::Unit::Selection) && !selection::install(base,selectionMode[0]?selectionMode:mode.c_str())) return 23;
            char drawMode[32]{}; GetEnvironmentVariableA("OPENBOXER_DRAW_MODE",drawMode,sizeof(drawMode));
            if(!runtime_options::original(runtime_options::Unit::Draw) && !draw::install(base,drawMode[0]?drawMode:mode.c_str())) return 24;
            char characterMode[32]{}; GetEnvironmentVariableA("OPENBOXER_CHARACTER_MODE",characterMode,sizeof(characterMode));
            if(!runtime_options::original(runtime_options::Unit::Character) && !character::install(base,characterMode[0]?characterMode:mode.c_str())) return 25;
            char animationMode[32]{}; GetEnvironmentVariableA("OPENBOXER_ANIMATION_MODE",animationMode,sizeof(animationMode));
            if(!runtime_options::original(runtime_options::Unit::Animation) && !animation::install(base,animationMode[0]?animationMode:mode.c_str())) return 27;
            char clipMode[32]{}; GetEnvironmentVariableA("OPENBOXER_CLIP_MODE",clipMode,sizeof(clipMode));
            if(!runtime_options::original(runtime_options::Unit::Clip) && !clip::install(base,clipMode[0]?clipMode:mode.c_str())) return 28;
            char actionMode[32]{}; GetEnvironmentVariableA("OPENBOXER_ACTION_MODE",actionMode,sizeof(actionMode));
            if(!clip::installAction(base,actionMode[0]?actionMode:mode.c_str(),!runtime_options::original(runtime_options::Unit::Clip)))return 30;
            char framesMode[32]{}; GetEnvironmentVariableA("OPENBOXER_FRAMES_MODE",framesMode,sizeof(framesMode));
            if(!runtime_options::original(runtime_options::Unit::Frames) && !frames::install(base,framesMode[0]?framesMode:mode.c_str())) return 29;
            char strikeMode[32]{}; GetEnvironmentVariableA("OPENBOXER_STRIKE_MODE",strikeMode,sizeof(strikeMode));
            if((!runtime_options::original(runtime_options::Unit::Strike) || runtime_options::capture(runtime_options::Unit::Strike)) && !strike::install(base,strikeMode[0]?strikeMode:mode.c_str())) return 30;
        }
        log("bootstrap: hash/slot guard passed, virtual slot installed"); return 0;
    } catch(...) { return 16; }
}
extern "C" __declspec(dllexport) DWORD WINAPI world0001_fixture_worker(void*) {
    if(!installed) return 20;
    return world::fixtures(base);
}
extern "C" __declspec(dllexport) DWORD WINAPI texture_fixture_worker(void*) {
    if(!installed) return 20;
    return texture::fixtures(base);
}
extern "C" __declspec(dllexport) DWORD WINAPI upload_fixture_worker(void*) {
    if(!installed) return 20;
    return upload::fixtures(base);
}
extern "C" __declspec(dllexport) DWORD WINAPI lighting_fixture_worker(void*) {
    if(!installed) return 20;
    return lighting::fixtures(base);
}
extern "C" __declspec(dllexport) DWORD WINAPI selection_fixture_worker(void*) {
    if(!installed) return 20;
    return selection::fixtures(base);
}
extern "C" __declspec(dllexport) DWORD WINAPI draw_fixture_worker(void*) {
    if(!installed) return 20;
    return draw::fixtures(base);
}
extern "C" __declspec(dllexport) DWORD WINAPI character_fixture_worker(void*) {
    if(!installed) return 20;
    return character::fixtures(base);
}
extern "C" __declspec(dllexport) DWORD WINAPI character_replay_worker(void*) {
    if(!installed) return 20;
    return character::replays(base);
}
static void dump(FILE* stream,const void* data,std::size_t size) {
    uint32_t length=static_cast<uint32_t>(size); std::fwrite(&length,4,1,stream);
    if(size) std::fwrite(data,1,size,stream);
}
static void payloadDump(void* object,const char* name) {
    FILE* stream=std::fopen(name,"wb"); if(!stream) return;
    auto f=ms3d::fields(object);
    const int32_t counts[]={f.groups,f.materials,f.triangles,f.vertices}; dump(stream,counts,sizeof(counts));
    dump(stream,f.vertexData,f.vertices*16); dump(stream,f.triangleData,f.triangles*76);
    for(int32_t i=0;i<f.groups;i++) {
        auto* group=static_cast<uint8_t*>(f.groupData)+i*12; dump(stream,group,8);
        int32_t count; void* indices; std::memcpy(&count,group+4,4); std::memcpy(&indices,group+8,4);
        dump(stream,indices,count*4);
    }
    for(int32_t i=0;i<f.materials;i++) {
        auto* material=static_cast<uint8_t*>(f.materialData)+i*80; dump(stream,material,76);
        char* text; std::memcpy(&text,material+76,4); dump(stream,text,std::strlen(text)+1);
    }
    std::fclose(stream);
}
static bool comparePayload(void* first,void* second) {
    const auto& a=ms3d::fields(first); const auto& b=ms3d::fields(second);
    if(a.groups!=b.groups || a.materials!=b.materials || a.triangles!=b.triangles || a.vertices!=b.vertices) return false;
    if((a.vertices && std::memcmp(a.vertexData,b.vertexData,a.vertices*16)) || (a.triangles && std::memcmp(a.triangleData,b.triangleData,a.triangles*76))) return false;
    for(int32_t i=0;i<a.groups;i++) {
        auto* x=static_cast<uint8_t*>(a.groupData)+i*12; auto* y=static_cast<uint8_t*>(b.groupData)+i*12;
        if(std::memcmp(x,y,8)) return false;
        uint32_t count; void* xp; void* yp; std::memcpy(&count,x+4,4); std::memcpy(&xp,x+8,4); std::memcpy(&yp,y+8,4);
        if(std::memcmp(xp,yp,count*4)) return false;
    }
    for(int32_t i=0;i<a.materials;i++) {
        auto* x=static_cast<uint8_t*>(a.materialData)+i*80; auto* y=static_cast<uint8_t*>(b.materialData)+i*80;
        if(std::memcmp(x,y,76)) return false;
        char* xp; char* yp; std::memcpy(&xp,x+76,4); std::memcpy(&yp,y+76,4);
        if(std::strcmp(xp,yp)) return false;
    }
    return true;
}
extern "C" __declspec(dllexport) DWORD WINAPI ms3d_fixture_worker(void*) {
    if(!installed) return 20;
    FILE* report=std::fopen("C:/Users/ADMIN/Boxer-lab/ms3d/ms3d-fixtures.txt","wb"); if(!report) return 21;
    unsigned short control=0; __asm__ volatile("fnstcw %0":"=m"(control));
    std::fprintf(report,"IO-0003 worker x87=%04x MXCSR=%08x mode=%s\n",control,_mm_getcsr(),mode.c_str()); std::fflush(report);
    auto ctor=reinterpret_cast<Constructor>(base+0x15890);
    DWORD failures=0;
    constexpr int fixtureCount=sizeof(approvedFixtures)/sizeof(approvedFixtures[0]);
    for(int fixture=1;fixture<=fixtureCount+1;fixture++) {
        char path[256]{};
        if(fixture<=fixtureCount) std::sprintf(path,"C:/Users/ADMIN/Boxer-lab/ms3d/%s",approvedFixtures[fixture-1].path);
        else std::strcpy(path,"C:/Users/ADMIN/Boxer-lab/ms3d/IO0003-definitely-missing.ms3d");
        void* a=allocator.allocate(ms3d::objectSize); void* b=allocator.allocate(ms3d::objectSize);
        if(!a || !b) { std::fprintf(report,"allocation failed fixture=%d\n",fixture); std::fclose(report); return 22; }
        ctor(a); ctor(b);
        std::vector<uint8_t> beforeA(static_cast<uint8_t*>(a),static_cast<uint8_t*>(a)+ms3d::objectSize);
        std::vector<uint8_t> beforeB(static_cast<uint8_t*>(b),static_cast<uint8_t*>(b)+ms3d::objectSize);
        uintptr_t oldBefore,oldAfter,newBefore,newAfter;
        unsigned short cwBefore=0,cwAfter=0; __asm__ volatile("fnstcw %0":"=m"(cwBefore));
        const unsigned mxBefore=_mm_getcsr();
        __asm__ volatile("movl %%esp,%0":"=m"(oldBefore));
        const bool ar=original(a,path);
        __asm__ volatile("movl %%esp,%0":"=m"(oldAfter));
        Loader* vtable=*reinterpret_cast<Loader**>(b);
        Loader routed=vtable[1];
        const unsigned replaceBefore=replacementCalls,fallbackBefore=fallbackCalls;
        __asm__ volatile("movl %%esp,%0":"=m"(newBefore));
        const bool br=routed(b,path);
        __asm__ volatile("movl %%esp,%0":"=m"(newAfter));
        __asm__ volatile("fnstcw %0":"=m"(cwAfter));
        const unsigned mxAfter=_mm_getcsr();
        const bool routedCorrectly=routed==hook && (fixture<=fixtureCount ? replacementCalls==replaceBefore+1 && fallbackCalls==fallbackBefore && ar && br : fallbackCalls==fallbackBefore+1 && !ar && !br);
        bool unchanged=std::memcmp(a,beforeA.data(),ms3d::fieldsOffset)==0 && std::memcmp(b,beforeB.data(),ms3d::fieldsOffset)==0;
        if(!ar) unchanged=unchanged && std::memcmp(a,beforeA.data(),ms3d::objectSize)==0;
        if(!br) unchanged=unchanged && std::memcmp(b,beforeB.data(),ms3d::objectSize)==0;
        const bool equal=ar==br && unchanged && comparePayload(a,b) && routedCorrectly && oldBefore==oldAfter && newBefore==newAfter && cwBefore==cwAfter && (mxBefore&0xffc0)==(mxAfter&0xffc0);
        char output[256]{}; std::sprintf(output,"C:/Users/ADMIN/Boxer-lab/ms3d/fixture-%02d-original.bin",fixture); payloadDump(a,output);
        std::sprintf(output,"C:/Users/ADMIN/Boxer-lab/ms3d/fixture-%02d-candidate.bin",fixture); payloadDump(b,output);
        const auto f=ms3d::fields(a);
        std::fprintf(report,"fixture=%d path=%s original=%d candidate=%d equal=%d unrelated_unchanged=%d counts=%d/%d/%d/%d route=%s vslot=%p hook=%p expected_sha256=%s cleanup_buffers=%d\n",fixture,path,ar,br,equal,unchanged,f.vertices,f.triangles,f.groups,f.materials,fixture<=fixtureCount?"replacement":"fallback",reinterpret_cast<void*>(routed),reinterpret_cast<void*>(&hook),fixture<=fixtureCount?approvedFixtures[fixture-1].sha256:"missing",ar?2*(f.groups+f.materials+4)+2:2);
        std::fflush(report);
        std::fprintf(report,"  actual_replacement_calls=%u actual_fallback_calls=%u stack_original=%08lx/%08lx stack_candidate=%08lx/%08lx x87=%04x/%04x mxcsr=%08x/%08x route_valid=%d\n",replacementCalls-replaceBefore,fallbackCalls-fallbackBefore,static_cast<unsigned long>(oldBefore),static_cast<unsigned long>(oldAfter),static_cast<unsigned long>(newBefore),static_cast<unsigned long>(newAfter),cwBefore,cwAfter,mxBefore,mxAfter,routedCorrectly); std::fflush(report);
        if(!equal) failures++;
        if(ar) ms3d::cleanup(a,allocator); if(br) ms3d::cleanup(b,allocator);
        allocator.release(a); allocator.release(b);
    }
    std::fclose(report); return failures;
}
BOOL WINAPI DllMain(HINSTANCE,DWORD,LPVOID) { return TRUE; }

extern "C" __declspec(dllexport) DWORD WINAPI animation_fixture_worker(void*) { return installed?animation::fixtures(base):20; }
extern "C" __declspec(dllexport) DWORD WINAPI animation_replay_worker(void*) { return installed?animation::replays(base):20; }

extern "C" __declspec(dllexport) DWORD WINAPI clip_fixture_worker(void*) { return installed?clip::fixtures(base):20; }
extern "C" __declspec(dllexport) DWORD WINAPI clip_replay_worker(void*) { return installed?clip::replays(base):20; }


extern "C" __declspec(dllexport) DWORD WINAPI frames_fixture_worker(void*) { return installed?frames::fixtures(base):20; }
extern "C" __declspec(dllexport) DWORD WINAPI frames_replay_worker(void*) { return installed?frames::replays(base):20; }


extern "C" __declspec(dllexport) DWORD WINAPI action_fixture_worker(void*) { return installed?clip::actionFixtures(base):20; }
extern "C" __declspec(dllexport) DWORD WINAPI action_replay_worker(void*) { return installed?clip::actionReplays(base):20; }

extern "C" __declspec(dllexport) DWORD WINAPI strike_fixture_worker(void*) { return installed?strike::fixtures(base):20; }
extern "C" __declspec(dllexport) DWORD WINAPI strike_replay_worker(void*) { return installed?strike::replays(base):20; }
