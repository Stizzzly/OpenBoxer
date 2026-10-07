#pragma once
#include <cstdint>
#include <cstring>
namespace action {
struct Role { unsigned state; uint32_t caller,literal; const char *name; };
inline constexpr Role roles[]={
 {0,0x371be,0x1623e8,"LEGS_STAND"},
 {3,0x37221,0x163c54,"LEGS_LEFTHEAD"},{4,0x37242,0x163c40,"LEGS_RIGHTHEAD"},
 {5,0x37263,0x163c30,"LEGS_LEFTTORS"},{6,0x37284,0x163c1c,"LEGS_RIGHTTORS"},
 {7,0x372a5,0x163c0c,"LEGS_LEFTBOK"},{8,0x372c6,0x163bfc,"LEGS_RIGHTBOK"},
 {11,0x37329,0x163bd0,"LEGS_BLOCK"}};
inline int role(uintptr_t base,uintptr_t caller,uintptr_t request) {
 for(unsigned i=0;i<8;++i)if(caller==base+roles[i].caller && request==base+roles[i].literal)return int(i);
 return -1;
}
inline bool admit(int index,uintptr_t base,uintptr_t owner,bool lifetime,const char *bytes) {
 return index>=0 && index<8 && owner==base+0x17acd0 && lifetime && bytes && std::strcmp(bytes,roles[index].name)==0;
}
}
