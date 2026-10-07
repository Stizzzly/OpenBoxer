#pragma once
#include <cstdint>
#include <cstring>
namespace animation::fp {
inline float value(uint32_t bits) { float v; std::memcpy(&v,&bits,4); return v; }
inline uint32_t bits(float v) { uint32_t b; std::memcpy(&b,&v,4); return b; }
inline bool finite(uint32_t b) { return (b & 0x7f800000u) != 0x7f800000u; }
// CW027F specifies 53-bit arithmetic. A binary64 spill preserves every supported
// intermediate (float32 operands and positive int32 rates cannot exhaust its range).
inline double interval(int32_t rate) {
    const float thousand=1000.0f; double out;
    __asm__ volatile("flds %1; fildl %2; fdivrp; fstpl %0" : "=m"(out) : "m"(thousand),"m"(rate) : "st");
    return out;
}
inline uint32_t elapsed(uint32_t now, uint32_t anchor) {
    float a=value(now), b=value(anchor), out;
    __asm__ volatile("flds %1; fsubs %2; fstps %0" : "=m"(out) : "m"(a),"m"(b) : "st");
    return bits(out);
}
inline uint32_t divide(uint32_t numerator, double divisor) {
    float a=value(numerator), out;
    __asm__ volatile("flds %1; fdivl %2; fstps %0" : "=m"(out) : "m"(a),"m"(divisor) : "st");
    return bits(out);
}
inline uint32_t subtract(uint32_t left, double right) {
    float a=value(left), out;
    __asm__ volatile("flds %1; fsubl %2; fstps %0" : "=m"(out) : "m"(a),"m"(right) : "st");
    return bits(out);
}
inline uint32_t moveAnchor(uint32_t anchor, double spacing, int32_t count) {
    float a=value(anchor), out;
    __asm__ volatile("fildl %3; fmull %2; fadds %1; fstps %0" : "=m"(out) : "m"(a),"m"(spacing),"m"(count) : "st");
    return bits(out);
}
inline bool within(double spacing, uint32_t residual) {
    float right=value(residual); uint16_t status;
    // Approved semantic orientation: interval LEFT, stored residual RIGHT.
    // fcompp balances the stack and naturally leaves the x87 C bits as witnesses.
    __asm__ volatile("flds %1; fldl %2; fcompp; fnstsw %0" : "=m"(status) : "m"(right),"m"(spacing) : "st");
    return (status & 0x4100)!=0;
}
inline bool negative(uint32_t factor) {
    float left=value(factor); uint16_t status;
    // Compare the corrected stored factor LEFT against literal +0 RIGHT.
    // The following integer clamp preserves the comparison condition codes.
    __asm__ volatile("fldz; flds %1; fcompp; fnstsw %0" : "=m"(status) : "m"(left) : "st");
    return (status & 0x0100)!=0;
}
}
