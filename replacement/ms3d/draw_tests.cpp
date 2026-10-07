#include "draw.hpp"
#include <cstdio>
int main(){const auto failures=draw::offlineFixtures();std::printf("RENDER-0005 offline fixtures failures=%u\n",failures);return failures?1:0;}
