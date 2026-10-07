#include "animation.hpp"
#include "animation_fp.hpp"
#include <cstdio>
int main() {
    const uint16_t cw=0x027f;
    __asm__ volatile("fninit; fldcw %0"::"m"(cw));
    if(animation::fp::interval(10)!=100.0) return 1;
    for(unsigned n=1;n<=11;++n) if(animation::predict(animation::fp::bits(float(n)*100),10).steps!=n) return 2;
    if(animation::predict(animation::fp::bits(2500),10).steps!=11) return 3;
    return int(animation::offlineFixtures("animation-offline-fixtures.jsonl"));
}
