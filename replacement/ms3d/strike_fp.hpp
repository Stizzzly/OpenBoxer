#pragma once
#include "strike.hpp"
namespace strike::fp {
inline uint16_t status(){uint16_t out;__asm__ volatile("fnstsw %0":"=m"(out));return out;}
inline uint32_t store32(){uint32_t out;__asm__ volatile("fstps %0":"=m"(out)::"st");return out;}
inline uint64_t store64(){uint64_t out;__asm__ volatile("fstpl %0":"=m"(out)::"st");return out;}
inline void load80(const Result &r){__asm__ volatile("fldt %0"::"m"(r.scalar80):"st");}
// State::call leaves a scalar callback's exact ST0 live. The raw80 copy is a
// recording witness, never a round-trip through C++ floating arithmetic.
inline uint32_t scalar32(const Result &){return store32();}
inline uint32_t multiplyAdd(uint32_t a,uint32_t b,uint32_t c){uint32_t out;__asm__ volatile("flds %1; fmuls %2; fadds %3; fstps %0":"=m"(out):"m"(a),"m"(b),"m"(c):"st");return out;}
inline uint32_t subtractProduct(uint32_t a,uint32_t b,uint32_t c){uint32_t out;__asm__ volatile("flds %2; fmuls %3; flds %1; fsubp; fstps %0":"=m"(out):"m"(a),"m"(b),"m"(c):"st");return out;}
inline uint32_t scalarProgress(const Result &,uint32_t scale,uint32_t dt,uint32_t old){uint32_t out;__asm__ volatile("fmuls %1; fmuls %2; fadds %3; fstps %0":"=m"(out):"m"(scale),"m"(dt),"m"(old):"st");return out;}
inline uint16_t compare32(uint32_t left,uint32_t right){uint16_t sw;__asm__ volatile("flds %1; fcomps %2; fnstsw %0":"=m"(sw):"m"(left),"m"(right):"st");return sw;}
inline uint16_t compare80(const Result &,double right){uint16_t sw;__asm__ volatile("fcompl %1; fnstsw %0":"=m"(sw):"m"(right):"st");return sw;}
inline uint16_t scaledCompare(uint32_t left,uint32_t scale,uint32_t right){uint16_t sw;__asm__ volatile("flds %1; fmuls %2; fcomps %3; fnstsw %0":"=m"(sw):"m"(left),"m"(scale),"m"(right):"st");return sw;}
// Repeated accessor values are kept separate: callbacks and float32 stores are
// observable boundaries. Arithmetic runs under the admitted 53-bit x87 CW.
inline uint32_t difference(uint32_t a,uint32_t b){uint32_t out;__asm__ volatile("flds %1; fsubs %2; fstps %0":"=m"(out):"m"(a),"m"(b):"st");return out;}
inline uint32_t differenceProduct(uint32_t a,uint32_t b,uint32_t stored){uint32_t out;__asm__ volatile("flds %1; fsubs %2; fmuls %3; fstps %0":"=m"(out):"m"(a),"m"(b),"m"(stored):"st");return out;}
inline uint64_t differenceSum(uint32_t a,uint32_t b,uint32_t stored,uint32_t x){uint64_t out;__asm__ volatile("flds %1; fsubs %2; fmuls %3; fadds %4; fstpl %0":"=m"(out):"m"(a),"m"(b),"m"(stored),"m"(x):"st");return out;}
inline uint32_t product(uint32_t a,uint32_t b){uint32_t out;__asm__ volatile("flds %1; fmuls %2; fstps %0":"=m"(out):"m"(a),"m"(b):"st");return out;}
inline uint64_t productSum(uint32_t a,uint32_t b,uint32_t x){uint64_t out;__asm__ volatile("flds %1; fmuls %2; fadds %3; fstpl %0":"=m"(out):"m"(a),"m"(b),"m"(x):"st");return out;}
}
