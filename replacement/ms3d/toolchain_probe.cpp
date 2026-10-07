#include <windows.h>
#include <string>
static_assert(sizeof(void*) == 4);
int main(){ std::string value="x86"; return value.size()!=3; }
