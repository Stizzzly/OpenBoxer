#pragma once
#include "draw.hpp"
#include <vector>
#include <string>
#include <cstdio>
namespace draw {
struct FpPreserver { alignas(16) uint8_t bytes[512];FpPreserver(){__asm__ volatile("fxsave %0":"=m"(bytes));}~FpPreserver(){__asm__ volatile("fxrstor %0"::"m"(bytes));} };
struct Region { uintptr_t base;uint32_t size;std::string name; };
struct Event { Api api;std::vector<uint32_t> args,payload;std::string role;uint32_t dispatch=0,result=0; };
struct Trace {
 std::vector<Region> regions;std::vector<Event> events;uint64_t digest=1469598103934665603ULL;uint32_t counts[13]{};
 void registerModel(ModelView,const std::string& prefix="");
 void region(const void*,uint32_t,const std::string&);
 std::string role(const void*)const;
 void append(Api,const uint32_t*,unsigned,const uint32_t* pointer=nullptr,unsigned words=0);
 void writeEvents(FILE*)const;
};
void snapshot(const char*,ModelView,const Dispatch&);
const char* apiName(Api);
// Single owner for shared GL wrappers. Optional observations never substitute
// another dispatch target and are inactive outside their explicit unit tag.
using Observer = void (*)(Api,const uint32_t *,unsigned);
void setObserver(Observer);
}
