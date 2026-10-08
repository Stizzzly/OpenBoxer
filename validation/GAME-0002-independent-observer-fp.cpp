#include <cstdint>
#include <cstdio>
#include <cstring>
#include <vector>
#include <cstdlib>
extern "C" void damage_observer0();
extern "C" void audit_target();
extern "C" void audit_probe(void*,uint8_t*);
static std::vector<uint32_t> returns;
static unsigned enters=0,exits=0; static bool nesting=false, nestedDone=false; alignas(16) static uint8_t nested[576]{};
extern "C" uint32_t damage_observer_before(uint32_t*f,uint8_t*fp){++enters;returns.push_back(f[10]);if(!nesting){nesting=true;audit_probe((void*)&damage_observer0,nested);nestedDone=true;nesting=false;}if(f[6]!=0x12345678||f[11]!=0x87654321||(fp[0]|uint16_t(fp[1])<<8)!=0x27f)abort();__asm__ volatile("fninit");uint32_t mx=0x1f80;__asm__ volatile("ldmxcsr %0"::"m"(mx));return uint32_t(uintptr_t(&audit_target));}
extern "C" uint32_t damage_observer_return(uint32_t*f,uint8_t*){++exits;if(f[7]!=0xaabbccdd)abort();auto r=returns.back();returns.pop_back();__asm__ volatile("fninit");return r;}
int main(){alignas(16) uint8_t d[576]{},o[576]{};audit_probe((void*)&audit_target,d);audit_probe((void*)&damage_observer0,o);unsigned checks=0,fail=0;auto eq=[&](unsigned at,unsigned n){++checks;if(memcmp(d+at,o+at,n)){++fail;printf("mismatch offset %u length %u\n",at,n);}};eq(0,5);eq(24,8);for(unsigned i=0;i<3;++i)eq(32+16*i,10);eq(512,12);eq(544,28); ++checks;if(memcmp(d,nested,5)||memcmp(d+24,nested+24,8)||memcmp(d+32,nested+32,10)||memcmp(d+544,nested+544,24))++fail;if(enters!=2||exits!=2||!nestedDone||!returns.empty())++fail;printf("independent FP CW/SW/TOP/tags/MXCSR/live80/registers/flags/ESP: %u checks %u failures\n",checks,fail);return fail?1:0;}

