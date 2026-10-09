#include "launcher_preinject_delay.hpp"
#include <cstdio>
#include <initializer_list>

int main() {
    unsigned failures=0;
    auto check=[&](const char* value,unsigned expected) {
        if(launcher_policy::preinjectObserveDelay(value)!=expected)++failures;
    };
    check(nullptr,0);check("",0);check("0",0);check("0000",0);
    check("1",1);check("8000",8000);check("15000",15000);
    check("15001",15000);check("0008000",8000);
    check("99999999999999999999999999999999999999",15000);
    for(const char* value:{"-1","+1"," 1","1 ","1.0","8000x","15000x","x","999999999999999999999999x"}) {
        try {launcher_policy::preinjectObserveDelay(value);++failures;}
        catch(const std::invalid_argument&) {}
    }
    std::printf("pre-injection observation delay default/bounds/invalid policy failures=%u\n",failures);
    return failures?1:0;
}
