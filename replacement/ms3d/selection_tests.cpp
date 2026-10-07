#include "selection.hpp"
#include <cstdio>
int main(){const auto failures=selection::offlineFixtures();std::printf("RENDER-0004 offline failures=%u\n",failures);return failures?1:0;}
