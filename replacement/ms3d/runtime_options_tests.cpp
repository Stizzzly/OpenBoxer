#include "runtime_options.hpp"
#include <cstdio>
#include <initializer_list>
int main(){
    unsigned failures=0;
    for(const char *list:{"1","all","draw","world,draw,character"," world, draw, character "})if(!runtime_options::contains(list,"draw"))++failures;
    for(const char *list:{"","drawing","character","world,character","draw-extra","alligator"})if(runtime_options::contains(list,"draw"))++failures;
    SetEnvironmentVariableA("OPENBOXER_DIAGNOSTICS",nullptr);
    SetEnvironmentVariableA("OPENBOXER_CAPTURE",nullptr);
    SetEnvironmentVariableA("OPENBOXER_ORIGINAL_UNITS",nullptr);
    runtime_options::configure();
    for(unsigned i=0;i<unsigned(runtime_options::Unit::Count);++i){const auto unit=static_cast<runtime_options::Unit>(i);if(runtime_options::diagnostics(unit) || runtime_options::capture(unit) || runtime_options::original(unit))++failures;}
    runtime_options::configured=false;
    SetEnvironmentVariableA("OPENBOXER_DIAGNOSTICS","world,draw,character");
    SetEnvironmentVariableA("OPENBOXER_CAPTURE","character");
    SetEnvironmentVariableA("OPENBOXER_ORIGINAL_UNITS","upload");
    runtime_options::configure();
    if(!runtime_options::diagnostics(runtime_options::Unit::World) || !runtime_options::diagnostics(runtime_options::Unit::Draw) || !runtime_options::diagnostics(runtime_options::Unit::Character) || runtime_options::diagnostics(runtime_options::Unit::Io))++failures;
    if(!runtime_options::capture(runtime_options::Unit::Character) || runtime_options::capture(runtime_options::Unit::Draw))++failures;
    if(!runtime_options::original(runtime_options::Unit::Upload) || runtime_options::original(runtime_options::Unit::Character))++failures;
    std::printf("Runtime option isolation/defaultoff failures=%u\n",failures);return failures?1:0;
}

