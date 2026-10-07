#include "texture.hpp"
#include <cstdio>
int main() { const auto failures=texture::offlineFixtures(); std::printf("RENDER-0001 offline scenarios=18 failures=%u\n",failures); return failures?1:0; }
