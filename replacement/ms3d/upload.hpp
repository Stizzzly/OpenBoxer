#pragma once
#include <cstdint>
#include <cstddef>
namespace upload {
struct Image { int32_t width,height; void* pixels; };
static_assert(sizeof(Image)==12 && offsetof(Image,pixels)==8 && sizeof(void*)==4);
using Entry=uint32_t (__cdecl*)(const char*);
using Decode=Image* (__cdecl*)(const char*);
using Generate=void (__stdcall*)(int32_t,uint32_t*);
using Bind=void (__stdcall*)(uint32_t,uint32_t);
using Parameter=void (__stdcall*)(uint32_t,uint32_t,int32_t);
using Mipmap=int32_t (__stdcall*)(uint32_t,int32_t,int32_t,int32_t,uint32_t,uint32_t,const void*);
using Release=void (__cdecl*)(void*);
struct Observation {
    const char* filename;
    bool image=false,pixels=false,mipmap=false,pixelReleased=false,recordReleased=false;
    int32_t width=0,height=0,status=0;
    uint32_t generated=0,result=0;
};
using Observe=void (*)(const Observation&);
struct Callbacks { Decode decode; Generate generate; Bind bind; Parameter parameter; Mipmap mipmap; Release release; Observe observe=nullptr; };
uint32_t run(const char*,const Callbacks&);
bool install(uintptr_t,const char*);
void fixtureMode(bool);
unsigned replacementCount();
uint32_t fixtures(uintptr_t);
uint32_t offlineFixtures();
}
