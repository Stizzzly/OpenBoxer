#pragma once
#include "world.hpp"
struct WorldAbiReport { uint32_t before,after,ebp,ebx,esi,edi; int32_t result; };
extern "C" int32_t __cdecl world_abi_probe(world::MapLoad,int32_t,int32_t,int32_t,WorldAbiReport*);
inline bool worldAbiValid(const WorldAbiReport& r) { return r.before==r.after && r.ebp==0x13579bdf && r.ebx==0x2468ace0 && r.esi==0x55aa55aa && r.edi==0xaa55aa55; }
