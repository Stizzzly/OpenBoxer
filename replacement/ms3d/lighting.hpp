#pragma once
#include <cstdint>
#include <cstddef>
namespace lighting {
using Entry=uint32_t (__attribute__((thiscall)) *)(void*);
using Color=void (__stdcall*)(float,float,float,float);
using Pair=void (__stdcall*)(uint32_t,uint32_t);
using Cap=void (__stdcall*)(uint32_t);
struct GL { Color color; Pair blend,bind; Cap disable,enable; };
constexpr std::size_t size=0x8d9c4,meshCount=0x8d9a4,meshTable=0x8d9a8,materialCount=0x8d9ac,materialTable=0x8d9b0,triangleCount=0x8d9b4,triangleTable=0x8d9b8,vertexCount=0x8d9bc,vertexTable=0x8d9c0;
uint32_t bits(const void*);
void put(void*,uint32_t);
void* pointer(const void*);
uint32_t stageA(void*);
uint32_t stageB(void*,uint8_t* globalCount,uint8_t* records,uint8_t* texture,const GL&);
bool install(uintptr_t,const char*);
void fixtureMode(bool);
unsigned countA(); unsigned countB();
uint32_t fixtures(uintptr_t); uint32_t offlineFixtures();
}
extern "C" uint32_t __cdecl lighting_color_bits(lighting::Color,uint32_t,uint32_t,uint32_t,uint32_t);
