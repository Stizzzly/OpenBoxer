#include "damage_fixture_script.hpp"
#include <cstdio>
#include <memory>
struct Probe:damage::State {
 std::vector<uint8_t> globals=std::vector<uint8_t>(0x19f000);
 std::array<uint8_t,232> actorData{};std::array<uint8_t,176> player{};
 std::array<uint8_t,2288> pBody{},oBody{};std::array<uint8_t,11000> world{};
 damage::fixture::Profile profile{actorData.data(),player.data(),pBody.data(),oBody.data(),world.data(),globals.data()};
 std::array<uint8_t,128> expected;unsigned checks=0,failures=0;
 uint8_t*actor()override{return actorData.data();}
 uint8_t*global(uint32_t rva)override{return globals.data()+rva;}
 damage::Result call(const damage::Call&c)override {
  // Model the actual BinaryState assembly boundary: restore the latest opaque
  // effect immediately before entering the independently scripted dependency.
  damage::restoreXmm(liveXmm.data());std::array<uint8_t,128> incoming;damage::saveXmm(incoming.data());
  ++checks;if(incoming!=expected)++failures;
  damage::Result result=profile.call(c);
  for(unsigned i=0;i<128;++i)expected[i]=result.fp[160+i];
  return result;
 }
};
int main(){unsigned failures=0,checks=0;for(const auto&test:damageCases){
 auto p=std::make_unique<Probe>();p->profile.initialize(test);p->profile.patternXmm=true;
 for(unsigned i=0;i<128;++i)p->expected[i]=uint8_t(0x53+7*i);
 const uint16_t cw=0x027f;__asm__ volatile("fninit; fldcw %0"::"m"(cw):"st");
 damage::restoreXmm(p->expected.data());damage::update(*p,int32_t(p->profile.mode));
 std::array<uint8_t,128> actual;damage::saveXmm(actual.data());
 ++p->checks;if(actual!=p->expected)++p->failures;
 failures+=p->failures;checks+=p->checks;
 }
 std::printf("independent patterned incoming/callback-varying all8 XMM boundary-shadow/whole-return cases=60 checks=%u failures=%u\n",checks,failures);
 return failures?1:0;
}
