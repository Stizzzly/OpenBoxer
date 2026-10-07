#pragma once
#include "draw_dispatch.hpp"
namespace draw {
uint32_t render(ModelView, const Dispatch &);
bool supported(ModelView, const Dispatch &, bool native);
bool install(uintptr_t, const char *);
void fixtureMode(bool);
unsigned count();
uint32_t fixtures(uintptr_t);
uint32_t offlineFixtures();
struct AbiReport {
    uint32_t beforeStack, afterStack, ebx, esi, edi, ebp, result;
    uint32_t beforeCW, beforeSW, beforeMX, afterCW, afterSW, afterMX;
};
using Entry = uint32_t(__attribute__((thiscall)) *)(void *);
}
extern "C" uint32_t __cdecl draw_above_threshold(uint32_t);
extern "C" uint32_t __cdecl draw_invoke(draw::Entry, void *, draw::AbiReport *);
