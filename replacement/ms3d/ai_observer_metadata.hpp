#pragma once
struct AiStartType {unsigned type;uint32_t calls[3],states[2];};
inline constexpr AiStartType aiStartTypes[]={{1u,{0x1cdce,0x1cf9f,0x1cfb1},{3,4}},{2u,{0x1d103,0x1d2e8,0x1d2fa},{5,6}},{3u,{0x1d44c,0x1d631,0x1d643},{7,8}}};
inline constexpr uint32_t aiCommittedTypeRva=0x17625c;
inline constexpr uint32_t aiAttackRva=0x17622d;
inline constexpr uint32_t aiPlayerPendingRva=0x1761c8;
