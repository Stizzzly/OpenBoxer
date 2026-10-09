#include "launcher_worker_delay.hpp"
#include <cstdio>
int main(){unsigned failures=0;auto check=[&](const char*p,unsigned expected){if(launcher_policy::workerObserveDelay(p)!=expected)++failures;};check(nullptr,0);check("",0);check("0",0);check("1000",1000);check("2000",2000);check("2001",2000);check("999999999999999999999",2000);check("0001",1);for(const char*p:{"-1","x","1.0","1000x"," 1"}){try{launcher_policy::workerObserveDelay(p);++failures;}catch(const std::invalid_argument&){}}
std::printf("worker observation delay default/bounds/invalid policy failures=%u\n",failures);return failures?1:0;}
