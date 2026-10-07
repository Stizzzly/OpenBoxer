#include "upload.hpp"
#include <cstdio>
int main() { const auto failures=upload::offlineFixtures(); std::printf("RENDER-0002 offline scenarios=22 failures=%u\n",failures); return failures?1:0; }
