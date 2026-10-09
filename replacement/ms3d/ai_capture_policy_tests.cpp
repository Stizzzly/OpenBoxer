#include "ai_capture_policy.hpp"
#include <cstdio>
int main(){unsigned failures=0;auto check=[&](bool expected,bool enabled,const char*side,bool dir,bool replace,bool damageOriginal,bool clipOriginal,bool otherOriginal){if(damage::capturePolicyValid(enabled,side,dir,replace,damageOriginal,clipOriginal,otherOriginal)!=expected)++failures;};
check(true,false,"invalid",false,false,true,false,true);
check(true,true,"",true,false,true,true,true);check(true,true,"original",true,false,true,true,true);
check(false,true,"invalid",true,true,false,true,false);check(false,true,"original",false,false,true,true,true);check(false,true,"original",true,false,true,false,true);
check(true,true,"candidate",true,true,false,true,false);check(false,true,"candidate",true,false,false,true,false);check(false,true,"candidate",true,true,true,true,false);check(false,true,"candidate",true,true,false,false,false);check(false,true,"candidate",true,true,false,true,true);
std::printf("explicit capture side/config pre-hook policy failures=%u\n",failures);return failures?1:0;}
