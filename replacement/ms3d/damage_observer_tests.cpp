#include <cstdint>
#include <cstring>
#include <cstdio>
#include <cstdlib>
#include <array>
extern "C" void damage_observer0();
extern "C" void damage_test_target();
extern "C" void damage_test_probe(void *,uint32_t *);
extern "C" void damage_test_full_probe(void *,const void *,void *);
extern "C" uint32_t damage_script_dispatch(unsigned,void*,uint32_t*){return 0;}
extern "C" void damage_observer63();
extern "C" void damage_observer66();
extern "C" void damage_rand_target();
extern "C" void damage_ctor_target();
extern "C" void damage_new_probe(void*,const void*,void*,unsigned);
static unsigned newKind=0;
static uint32_t returned;static unsigned enters=0,exits=0;
extern "C" uint32_t damage_observer_before(uint32_t *frame,uint8_t *fp){++enters;returned=frame[10];if(frame[6]!=0x12345678 || (newKind==0 && frame[11]!=0x87654321) || (fp[0]|unsigned(fp[1])<<8)!=0x27f)std::abort();__asm__ volatile("fninit");return uint32_t(reinterpret_cast<uintptr_t>(newKind==1?&damage_rand_target:newKind==2?&damage_ctor_target:&damage_test_target));}
extern "C" uint32_t damage_observer_return(uint32_t *frame,uint8_t *){++exits;if(frame[7]!=0xaabbccdd)std::abort();__asm__ volatile("fninit");return returned;}
int main(){uint16_t cw=0x27f;__asm__ volatile("fninit; fldcw %0"::"m"(cw));uint32_t direct[8]{},observed[8]{};damage_test_probe(reinterpret_cast<void*>(&damage_test_target),direct);__asm__ volatile("fninit; fldcw %0"::"m"(cw));damage_test_probe(reinterpret_cast<void*>(&damage_observer0),observed);if(std::memcmp(direct,observed,sizeof(direct)) || enters!=1 || exits!=1)return 1;
alignas(16) std::array<uint8_t,512> input{};alignas(16) std::array<uint8_t,544> directFp{},observedFp{};
__asm__ volatile("fninit; fldcw %1; fxsave %0":"=m"(input):"m"(cw):"st");for(unsigned i=160;i<288;++i)input[i]=uint8_t(0x31+i);
damage_test_full_probe(reinterpret_cast<void*>(&damage_test_target),input.data(),directFp.data());damage_test_full_probe(reinterpret_cast<void*>(&damage_observer0),input.data(),observedFp.data());
if(std::memcmp(directFp.data(),observedFp.data(),540) || enters!=2 || exits!=2){for(unsigned i=0;i<540;++i)if(directFp[i]!=observedFp[i])std::printf("FP offset=%u direct=%02x observed=%02x\n",i,directFp[i],observedFp[i]);std::printf("counts=%u/%u\n",enters,exits);return 2;}
for(unsigned kind=1;kind<=2;++kind){newKind=kind;alignas(16) std::array<uint8_t,560> d{},o{};const unsigned before=enters;damage_new_probe(reinterpret_cast<void*>(kind==1?&damage_rand_target:&damage_ctor_target),input.data(),d.data(),kind==2);damage_new_probe(reinterpret_cast<void*>(kind==1?&damage_observer63:&damage_observer66),input.data(),o.data(),kind==2);if(std::memcmp(d.data(),o.data(),544) || enters!=before+1 || exits!=enters)return 3;}
std::puts("transparent observer return/ret4/callee registers/live80/fullFP/XMM0..7 PASS");return 0;}
