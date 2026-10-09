#include "damage_fixture_script.hpp"
#include <cstdio>
#include <memory>
struct TestState:damage::State {
 std::vector<uint8_t> globals=std::vector<uint8_t>(0x19f000);std::array<uint8_t,232> a{};std::array<uint8_t,176> p{};std::array<uint8_t,2288> pb{},ob{};std::array<uint8_t,11000>w{};
 damage::fixture::Profile profile{a.data(),p.data(),pb.data(),ob.data(),w.data(),globals.data()};unsigned impulses=0;bool validImpulse=true;
 uint8_t*actor()override{return a.data();}uint8_t*global(uint32_t r)override{return globals.data()+r;}
 damage::Result call(const damage::Call&c)override{if(c.id==damage::Callback::Impulse){++impulses;const auto*v=static_cast<uint8_t*>(damage::pointer(c.args[0]));validImpulse&=c.owner==ob.data() && damage::word(v)==0xc0400000 && !damage::word(v+4) && !damage::word(v+8) && damage::word(v+12)==0xcccccccc;}return profile.call(c);}
 void init(unsigned type,unsigned altern,unsigned difficulty,unsigned variant,uint32_t count,uint32_t budget,uint32_t distance=0x40000000){profile.initialize(damageCases[0]);for(unsigned r:{0x1761c9u,0x1761c8u,0x17622cu,0x176234u,0x17628cu})*global(r)=0;for(unsigned off:{168u,169u,170u})a[off]=uint8_t(altern);damage::put(a.data()+216,type);damage::put(a.data()+200,count);damage::put(a.data()+224,budget);damage::put(global(0x1847c4),difficulty);damage::put(global(0x184788),variant);damage::put(global(0x176230),0x40000000);damage::put(global(0x176370),0x3f800000);damage::put(ob.data()+752,distance);const uint16_t cw=0x27f;__asm__ volatile("fninit; fldcw %0"::"m"(cw):"st");}
};

// Fixed outcomes verify lifecycle boundaries and threshold crossing without an
// implementation-shaped oracle. Whole native comparison is a separate gate.
int main(){
 unsigned failures=0,cases=0;
 const uint32_t thresholds[]={0x40000000,0x40266666,0x40733333};
 for(unsigned type=1;type<=3;++type)for(unsigned alt=0;alt<2;++alt)for(unsigned pending=0;pending<2;++pending){
  for(unsigned completion=0;completion<2;++completion){
   auto s=std::make_unique<TestState>();s->init(type,alt,2,1,0x7fffffff,2);
   *s->global(0x17622d)=1;*s->global(0x1761c8)=uint8_t(pending);
   damage::put(s->global(0x17625c),type);damage::put(s->a.data()+164,2*type+1+alt);
   damage::put(s->global(0x176230),thresholds[type-1]+completion);
   damage::put(s->global(0x176370),0);damage::put(s->global(0x176258),0);damage::put(s->global(0x1762bc),0x40000000);
   const auto table=std::vector<uint8_t>(s->global(0x1762a0),s->global(0x1762a0)+128);
   damage::update(*s,1,false,true);++cases;
   if(*s->global(0x17622d)!=!completion || *s->global(0x1761c8)!=pending || damage::word(s->a.data()+164)!=(completion?0:2*type+1+alt) || damage::word(s->global(0x176230))!=(completion?0:thresholds[type-1]) || damage::word(s->global(0x176258))!=(completion?0x40400000:0) || s->impulses || s->profile.rngOrdinal || damage::word(s->a.data()+200)!=0x7fffffff || std::memcmp(table.data(),s->global(0x1762a0),128))++failures;
   for(unsigned j=0;j<7600;++j)if(s->global(0x17f710)[j]!=0xa5)++failures;
  }
 }
 // Addition 98+2*1.5=101; decay 20*0.15 ->98 avoids lock. With
 // dt zero it crosses 99 and clamps to100. A negative result stays negative.
 for(unsigned row=0;row<3;++row){
  auto s=std::make_unique<TestState>();s->init(1,0,2,1,0,2);*s->global(0x17622d)=1;damage::put(s->global(0x17625c),1);damage::put(s->a.data()+164,3);damage::put(s->global(0x176230),0x40000001);
  damage::put(s->global(0x176258),row==2?0xc0000000:0x42c40000);damage::put(s->global(0x1762bc),row==2?0:0x40000000);damage::put(s->global(0x176370),row==1?0x41a00000:0);
  damage::update(*s,1,false,true);++cases;const uint32_t expected[]={0x42c80000,0x42c40000,0xc0000000};if(damage::word(s->global(0x176258))!=expected[row] || *s->global(0x176256)!=(row==0))++failures;
 }
 // One entry increments across threshold and remains attacking. The next
 // whole entry completes, so no completion or second strike occurs early.
 auto s=std::make_unique<TestState>();s->init(1,0,2,1,0,2);*s->global(0x17622d)=1;damage::put(s->global(0x17625c),1);damage::put(s->a.data()+164,3);damage::put(s->global(0x1762bc),0);
 damage::update(*s,1,false,true);++cases;if(!*s->global(0x17622d) || damage::word(s->global(0x176230))!=0x4000a3d7)++failures;
 damage::update(*s,1,false,true);++cases;if(*s->global(0x17622d) || damage::word(s->global(0x176230)) || damage::word(s->a.data()+164) || s->impulses)++failures;
 std::printf("GAME-0004 lifecycle equality/completion/pending/alias/fatigue/cross-entry cases=%u failures=%u\n",cases,failures);return failures?1:0;
}
