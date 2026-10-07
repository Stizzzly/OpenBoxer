#pragma once
#include "world.hpp"
namespace world {
bool redirect(uintptr_t source,uintptr_t destination);
Callbacks dependencies(uintptr_t module);
void setFixtureMode(bool enabled);
unsigned replacementCount();
}
