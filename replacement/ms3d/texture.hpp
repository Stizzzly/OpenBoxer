#pragma once
#include <cstdint>
#include <cstddef>
namespace texture {
using Reload=uint32_t (__attribute__((thiscall)) *)(void*);
using Length=uint32_t (__cdecl*)(const char*);
using Upload=uint32_t (__cdecl*)(const char*);
using Observe=void (*)(int32_t,const char*,uint32_t,uint32_t,bool);
struct Callbacks { Length length; Upload upload; Observe observe=nullptr; };
constexpr std::size_t countOffset=0x8d9ac,tableOffset=0x8d9b0;
int32_t count(void*);
uint8_t* table(void*);
uint32_t run(void*,const Callbacks&);
bool install(uintptr_t,const char*);
uint32_t fixtures(uintptr_t);
uint32_t offlineFixtures();
void fixtureMode(bool);
unsigned replacementCount();
}
