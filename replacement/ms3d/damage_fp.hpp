#pragma once
#include "damage.hpp"
namespace damage::fp {
inline uint16_t compare(uint32_t lhs,uint32_t rhs){uint16_t sw;__asm__ volatile("flds %1; fcomps %2; fnstsw %0":"=m"(sw):"m"(lhs),"m"(rhs):"st");return sw;}
inline uint16_t scaledCompare(uint32_t lhs,uint32_t scale,uint32_t rhs){uint16_t sw;__asm__ volatile("flds %1; fmuls %2; fcomps %3; fnstsw %0":"=m"(sw):"m"(lhs),"m"(scale),"m"(rhs):"st");return sw;}
inline bool less(uint16_t sw){return (sw&0x4500)==0x100;}
inline bool greater(uint16_t sw){return (sw&0x4500)==0;}
inline uint32_t scalar32(){uint32_t out;__asm__ volatile("fstps %0":"=m"(out)::"st");return out;}
inline uint32_t decrement(uint32_t old,uint32_t dt){uint32_t out;const uint64_t scale=0x3f847ae147ae147bull;__asm__ volatile("flds %1; fmull %2; fsubrs %3; fstps %0":"=m"(out):"m"(dt),"m"(scale),"m"(old):"st");return out;}
inline uint32_t difference(uint32_t a,uint32_t b){uint32_t out;__asm__ volatile("flds %1; fsubs %2; fstps %0":"=m"(out):"m"(a),"m"(b):"st");return out;}
inline uint32_t differenceProduct(uint32_t a,uint32_t b,uint32_t d){uint32_t out;__asm__ volatile("flds %1; fsubs %2; fmuls %3; fstps %0":"=m"(out):"m"(a),"m"(b),"m"(d):"st");return out;}
inline uint64_t differenceSum(uint32_t a,uint32_t b,uint32_t d,uint32_t x){uint64_t out;__asm__ volatile("flds %1; fsubs %2; fmuls %3; fadds %4; fstpl %0":"=m"(out):"m"(a),"m"(b),"m"(d),"m"(x):"st");return out;}
inline uint32_t subtractProduct(uint32_t old,uint32_t a,uint32_t b){uint32_t out;__asm__ volatile("flds %1; fmuls %2; fsubrs %3; fstps %0":"=m"(out):"m"(a),"m"(b),"m"(old):"st");return out;}
inline uint32_t damage(uint32_t old,uint32_t scale,uint32_t player,uint32_t ai){uint32_t out;const uint32_t coefficient=0x40200000;__asm__ volatile("flds %1; fmuls %2; fmuls %3; fmuls %4; fsubrs %5; fstps %0":"=m"(out):"m"(scale),"m"(player),"m"(coefficient),"m"(ai),"m"(old):"st");return out;}
inline uint32_t pool(int32_t remainder,uint32_t scale,uint32_t offset){uint32_t out;const uint32_t fifty=0x42480000;if(offset)__asm__ volatile("fildl %1; fsubs %2; fmuls %3; fsubs %4; fstps %0":"=m"(out):"m"(remainder),"m"(fifty),"m"(scale),"m"(offset):"st");else __asm__ volatile("fildl %1; fsubs %2; fmuls %3; fstps %0":"=m"(out):"m"(remainder),"m"(fifty),"m"(scale):"st");return out;}
inline uint32_t cooldown(int32_t difficulty){uint32_t out;const uint32_t three=0x40400000,quarter=0x3e800000,numerator=0x40600000;__asm__ volatile("fildl %1; fadds %2; fmuls %3; fdivrs %4; fstps %0":"=m"(out):"m"(difficulty),"m"(three),"m"(quarter),"m"(numerator):"st");return out;}
inline uint32_t add(uint32_t a,uint32_t b){uint32_t out;__asm__ volatile("flds %1; fadds %2; fstps %0":"=m"(out):"m"(a),"m"(b):"st");return out;}
inline uint32_t multiplyAdd(uint32_t a,uint32_t b,uint32_t c){uint32_t out;__asm__ volatile("flds %1; fmuls %2; fadds %3; fstps %0":"=m"(out):"m"(a),"m"(b),"m"(c):"st");return out;}
inline uint32_t progress(uint32_t dt,uint32_t old){uint32_t out;const uint32_t scale=0x3ccccccd;__asm__ volatile("fmuls %1; fmuls %2; fadds %3; fstps %0":"=m"(out):"m"(scale),"m"(dt),"m"(old):"st");return out;}
inline uint32_t product(uint32_t a,uint32_t b){uint32_t out;__asm__ volatile("flds %1; fmuls %2; fstps %0":"=m"(out):"m"(a),"m"(b):"st");return out;}
inline uint64_t productSum(uint32_t a,uint32_t b,uint32_t x){uint64_t out;__asm__ volatile("flds %1; fmuls %2; fadds %3; fstpl %0":"=m"(out):"m"(a),"m"(b),"m"(x):"st");return out;}
inline uint16_t finalCompare(){const uint64_t three=0x4008000000000000ull;uint16_t sw;__asm__ volatile("fcompl %1; fnstsw %0":"=m"(sw):"m"(three):"st");return sw;}
}
