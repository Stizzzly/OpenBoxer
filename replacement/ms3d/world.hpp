#pragma once
#include <cstdint>
#include <cstddef>
namespace world {
using Allocate=void* (__cdecl*)(uint32_t);
using Construct=void* (__attribute__((thiscall)) *)(void*);
using ThisVoid=void (__attribute__((thiscall)) *)(void*);
using LoadModel=bool (__attribute__((thiscall)) *)(void*,const char*);
using NoArg=void (__cdecl*)();
using IntArg=void (__cdecl*)(int32_t);
using IntResult=int32_t (__cdecl*)();
using Holder=void* (__attribute__((thiscall)) *)(void*,const char*,void*);
using AudioLoad=void (__attribute__((thiscall)) *)(void*,void*,int32_t,int32_t);
using Cursor=int32_t (__stdcall*)(int32_t,int32_t);
using MapLoad=int32_t (__cdecl*)(int32_t,int32_t,int32_t);
struct Callbacks {
    Allocate allocate;
    Construct construct;
    ThisVoid reload,lightA,lightB;
    NoArg stageA,stageB,stageC,stageD;
    IntArg optional,callback2000A,callback2000B;
    IntResult random,final;
    Holder holder;
    AudioLoad audioLoad;
    ThisVoid holderDestroy,audioReset,audioPlay;
    Cursor cursor;
};
// Approved preferred VAs are translated here; this abstraction reads/writes
// widths exactly and can also operate over an independently allocated test image.
struct State {
    uint8_t* image;
    uint8_t byte(uint32_t va) const;
    int32_t integer(uint32_t va) const;
    void* pointer(uint32_t va) const;
    void storeByte(uint32_t va,uint8_t value);
    void store(uint32_t va,uint32_t value);
    void storePointer(uint32_t va,void* value);
    void* address(uint32_t va) const;
};
int32_t run(State state,const Callbacks& callbacks,int32_t selector);
struct FixtureResult { uint32_t scenarios=0,failures=0; };
bool install(uintptr_t module,const char* mode);
uint32_t fixtures(uintptr_t module);
}
