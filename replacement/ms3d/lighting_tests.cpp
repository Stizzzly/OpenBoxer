#include "lighting.hpp"
#include <cstdio>
int main() { const auto failures=lighting::offlineFixtures(); std::printf("RENDER-0003 offline scenarios=28 failures=%u\n",failures); return failures?1:0; }
