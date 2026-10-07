#pragma once
#include <cstdint>
#include <cstddef>
namespace selection {
struct Vector { uint32_t x,y,z,opaque; };
static_assert(sizeof(Vector)==16);
using Entry=uint32_t (__cdecl*)();
using Init=void* (__attribute__((thiscall)) *)(void*,uint32_t,uint32_t,uint32_t);
using Access=void* (__attribute__((thiscall)) *)(void*,int32_t);
using Normalize=void (__attribute__((thiscall)) *)(void*);
using Angular=double (__cdecl*)(Vector,Vector);
using Magnitude=float (__cdecl*)(float);
using Rotate=void* (__cdecl*)(void*,Vector,float,float,float,float);
using Sin=double (__cdecl*)(double);
using Wave=void (__cdecl*)();
using Scalar=uint32_t (__stdcall*)(uint32_t,uint32_t);
using Color=uint32_t (__stdcall*)(uint32_t,uint32_t,uint32_t,uint32_t,uint32_t);
struct Dependencies { Init init; Access access; Normalize normalize; Angular angular; Magnitude magnitude; Rotate rotate; Sin sine; Wave wave; };
struct State {
 uint8_t* base;
 void (*trace)(void*,unsigned,const uint32_t*,uint32_t)=nullptr;
 void* context=nullptr;
 void* at(uint32_t rva) const { return base+rva; }
};
constexpr uint32_t positions[3]={0x170178,0x170188,0x170198},colors[3]={0x1701b8,0x1701c8,0x1701d8},directions[3]={0x1701f8,0x170204,0x170210},parameters[3]={0x170228,0x17022c,0x170230};
constexpr uint32_t handles[7]={0x17d950,0x17d908,0x184900,0x1851ec,0x184298,0x175d90,0x175ee8};
uint32_t bits(const void*); void put(void*,uint32_t);
uint32_t run(State,const Dependencies&);
using Observer=void(*)(void*,unsigned,State);
void setObserver(Observer,void*);
Dependencies dependencies(uintptr_t);
bool install(uintptr_t,const char*); void fixtureMode(bool); unsigned count();
uint32_t fixtures(uintptr_t); uint32_t offlineFixtures();
void dumpState(const char*,State,uint32_t,uint32_t);
}
extern "C" uint32_t __cdecl selection_score(selection::Angular,const selection::Vector*,const selection::Vector*,selection::Magnitude);
extern "C" uint32_t __cdecl selection_angle(selection::Sin,uint32_t,uint64_t);
