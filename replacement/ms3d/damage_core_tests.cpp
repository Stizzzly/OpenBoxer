#include "damage_fixture_script.hpp"
#include <cstdio>
#include <memory>
struct TestState:damage::State {
    std::vector<uint8_t> globals=std::vector<uint8_t>(0x19f000);
    std::array<uint8_t,232> actorData{};std::array<uint8_t,176> player{};std::array<uint8_t,2288> pBody{},oBody{};std::array<uint8_t,11000> world{};
    damage::fixture::Profile profile{actorData.data(),player.data(),pBody.data(),oBody.data(),world.data(),globals.data()};
    uint8_t *actor() override{return actorData.data();}uint8_t *global(uint32_t rva) override{return globals.data()+rva;}damage::Result call(const damage::Call&c)override{return profile.call(c);}
};
int main(){unsigned failures=0;for(unsigned test=0;test<6;++test){auto state=std::make_unique<TestState>();state->profile.initialize(damageCases[test]);const uint16_t cw=0x27f;__asm__ volatile("fninit; fldcw %0"::"m"(cw):"st");damage::update(*state,1);const bool consumes=test>=4,blocked=test%2;const uint32_t health=damage::word(state->global(0x176238));unsigned getters=0,random=0;for(auto site:state->profile.sites){getters+=site==0x1d9e8;random+=site==0x1da04 || site==0x1da3b || site==0x1da6c;}if(health!=(consumes?(blocked?0x42c60000:0x42be0000):0x42c80000) || *state->global(0x17622c)!=!consumes || getters!=(consumes&&!blocked?100u:0u) || random!=(consumes&&!blocked?300u:0u))++failures;
if(consumes&&!blocked)for(unsigned i=0;i<100;++i)for(unsigned j=0;j<76;++j)if(j && !(j>=40 && j<52) && state->global(0x17f710+76*i)[j]!=0xa5)++failures;}
std::printf("damage delay equality/block mitigation/100getters/300rng/sentinels failures=%u\n",failures);return failures?1:0;}
