#include "world_fixtures.hpp"
#include <cstdio>
int main() { const unsigned failures=world::offlineFixtures(); std::printf("WORLD-0001 offline scenarios=2648 failures=%u\n",failures); return failures?1:0; }
