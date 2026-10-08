#pragma once
#include <windows.h>
#include <cstdint>
#include <cstdio>
#include <cstring>
namespace runtime_options {
enum class Unit : unsigned { Io, World, Texture, Upload, Lighting, Selection, Draw, Character, Animation, Clip, Frames, Action, Strike, Damage, Count };
inline constexpr const char *names[]={"ms3d","world","texture","upload","lighting","selection","draw","character","animation","clip","frames","action","strike","damage"};
inline bool diagnosticFlags[unsigned(Unit::Count)]{};
inline bool captureFlags[unsigned(Unit::Count)]{};
inline bool originalFlags[unsigned(Unit::Count)]{};
inline bool configured=false;
inline bool contains(const char *list,const char *name){
    if(!list)return false;
    if(std::strcmp(list,"1")==0 || std::strcmp(list,"all")==0)return true;
    const size_t length=std::strlen(name);
    for(const char *p=list;*p;){while(*p==',' || *p==' ')++p;const char *end=p;while(*end && *end!=',')++end;const char *trim=end;while(trim>p && trim[-1]==' ')--trim;if(size_t(trim-p)==length && std::memcmp(p,name,length)==0)return true;p=end;}
    return false;
}
inline void configure(){
    if(configured)return;
    char diagnostics[256]{},capture[256]{},original[256]{};
    GetEnvironmentVariableA("OPENBOXER_DIAGNOSTICS",diagnostics,sizeof(diagnostics));
    GetEnvironmentVariableA("OPENBOXER_CAPTURE",capture,sizeof(capture));
    GetEnvironmentVariableA("OPENBOXER_ORIGINAL_UNITS",original,sizeof(original));
    for(unsigned i=0;i<unsigned(Unit::Count);++i){diagnosticFlags[i]=contains(diagnostics,names[i]);captureFlags[i]=contains(capture,names[i]);originalFlags[i]=contains(original,names[i]);}
    configured=true;
}
inline bool diagnostics(Unit unit){return diagnosticFlags[unsigned(unit)];}
inline bool capture(Unit unit){return captureFlags[unsigned(unit)];}
// Explicit diagnostic isolation only. Normal launch leaves every flag false
// and installs all replacements; this is not a compatibility fallback.
inline bool original(Unit unit){return originalFlags[unsigned(unit)];}
struct PreserveFp {
    alignas(16) uint8_t bytes[512];
    PreserveFp(){__asm__ volatile("fxsave %0":"=m"(bytes));}
    ~PreserveFp(){__asm__ volatile("fxrstor %0"::"m"(bytes));}
};
inline uint64_t nextSample[unsigned(Unit::Count)]{};
inline uint32_t sequences[unsigned(Unit::Count)]{};
class Timing {
    Unit unit_;bool enabled_=false;uint64_t started_=0;uint32_t sequence_=0;
    void emit(const char *phase) const {
        PreserveFp preserve;
        FILE *file=std::fopen("C:/Users/ADMIN/Boxer-lab/ms3d/runtime-timing.jsonl","ab");
        if(file){const uint64_t now=GetTickCount64();std::fprintf(file,"{\"unit\":\"%s\",\"phase\":\"%s\",\"sequence\":%u,\"thread\":%lu,\"tick_ms\":%llu,\"elapsed_ms\":%llu}\n",names[unsigned(unit_)],phase,sequence_,GetCurrentThreadId(),static_cast<unsigned long long>(now),static_cast<unsigned long long>(now-started_));std::fclose(file);}
    }
  public:
    explicit Timing(Unit unit):unit_(unit){
        if(!diagnostics(unit))return;
        PreserveFp preserve;started_=GetTickCount64();sequence_=++sequences[unsigned(unit)];
        enabled_=sequence_<=3 || started_>=nextSample[unsigned(unit)];
        if(enabled_){nextSample[unsigned(unit)]=started_+1000;emit("enter");}
    }
    void checkpoint(const char *stage) const {if(enabled_)emit(stage);}
    ~Timing(){if(enabled_)emit("leave");}
};
}


