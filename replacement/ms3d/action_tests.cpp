#include "action_roles.hpp"
#include "clip.hpp"
#include <array>
#include <cstdio>
int main() {
 unsigned failures=clip::offlineActionFixtures("action-offline-fixtures.jsonl");
 constexpr uintptr_t base=0x400000,owner=base+0x17acd0;
 unsigned tested=0;
 for(unsigned i=0;i<8;++i) {
  const auto &r=action::roles[i];
  const int role=action::role(base,base+r.caller,base+r.literal);
  if(role!=int(i) || !action::admit(role,base,owner,true,r.name))++failures;
  for(unsigned reject=0;i>0 && reject<5;++reject) {
   const int selected=action::role(base,base+r.caller+(reject==0),base+r.literal+(reject==1));
   const bool accepted=action::admit(selected,base,owner+(reject==2),reject!=3,reject==4?"wrong-name":r.name);
   std::array<uint32_t,28> state{}; const auto before=state; unsigned effects=0;
   if(accepted) { state[11]=7; ++effects; }
   if(accepted || effects || state!=before)++failures;
   ++tested;
  }
 }
 for(uint32_t excluded:{0x372e7u,0x37308u,0x3734au,0x371dfu,0x37200u})
  if(action::role(base,base+excluded,base+0x163c54)!=-1)++failures;
 std::printf("Action role identity: 8 accepted; seven new roles %u rejected; existing idle retains movement fallback; fixtures 32, failures=%u\n",tested,failures);
 return failures?1:0;
}
