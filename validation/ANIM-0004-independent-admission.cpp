#include "action_roles.hpp"
#include <cstdio>
int main(){unsigned checks=0,failures=0;constexpr uintptr_t base=0x400000,owner=base+0x17acd0;
{int i=action::role(base,base+0x371BE,base+0x1623E8);if(i!=0||!action::admit(i,base,owner,true,"LEGS_STAND"))++failures;++checks;
if(action::role(base,base+0x371BE + 1,base+0x1623E8)!=-1)++failures;++checks;
if(action::role(base,base+0x371BE,base+0x1623E8 + 1)!=-1)++failures;++checks;
if(action::admit(i,base,owner+4,true,"LEGS_STAND"))++failures;++checks;
if(action::admit(i,base,owner,false,"LEGS_STAND"))++failures;++checks;
if(action::admit(i,base,owner,true,"WRONG"))++failures;++checks;
if(action::admit(i,base,owner,true,nullptr))++failures;++checks;
if(action::admit(-1,base,owner,true,"WRONG"))++failures;++checks;
if(action::admit(8,base,owner,true,"WRONG"))++failures;++checks;
}
{int i=action::role(base,base+0x37221,base+0x163C54);if(i!=1||!action::admit(i,base,owner,true,"LEGS_LEFTHEAD"))++failures;++checks;
if(action::role(base,base+0x37221 + 1,base+0x163C54)!=-1)++failures;++checks;
if(action::role(base,base+0x37221,base+0x163C54 + 1)!=-1)++failures;++checks;
if(action::admit(i,base,owner+4,true,"LEGS_LEFTHEAD"))++failures;++checks;
if(action::admit(i,base,owner,false,"LEGS_LEFTHEAD"))++failures;++checks;
if(action::admit(i,base,owner,true,"WRONG"))++failures;++checks;
if(action::admit(i,base,owner,true,nullptr))++failures;++checks;
if(action::admit(-1,base,owner,true,"WRONG"))++failures;++checks;
if(action::admit(8,base,owner,true,"WRONG"))++failures;++checks;
}
{int i=action::role(base,base+0x37242,base+0x163C40);if(i!=2||!action::admit(i,base,owner,true,"LEGS_RIGHTHEAD"))++failures;++checks;
if(action::role(base,base+0x37242 + 1,base+0x163C40)!=-1)++failures;++checks;
if(action::role(base,base+0x37242,base+0x163C40 + 1)!=-1)++failures;++checks;
if(action::admit(i,base,owner+4,true,"LEGS_RIGHTHEAD"))++failures;++checks;
if(action::admit(i,base,owner,false,"LEGS_RIGHTHEAD"))++failures;++checks;
if(action::admit(i,base,owner,true,"WRONG"))++failures;++checks;
if(action::admit(i,base,owner,true,nullptr))++failures;++checks;
if(action::admit(-1,base,owner,true,"WRONG"))++failures;++checks;
if(action::admit(8,base,owner,true,"WRONG"))++failures;++checks;
}
{int i=action::role(base,base+0x37263,base+0x163C30);if(i!=3||!action::admit(i,base,owner,true,"LEGS_LEFTTORS"))++failures;++checks;
if(action::role(base,base+0x37263 + 1,base+0x163C30)!=-1)++failures;++checks;
if(action::role(base,base+0x37263,base+0x163C30 + 1)!=-1)++failures;++checks;
if(action::admit(i,base,owner+4,true,"LEGS_LEFTTORS"))++failures;++checks;
if(action::admit(i,base,owner,false,"LEGS_LEFTTORS"))++failures;++checks;
if(action::admit(i,base,owner,true,"WRONG"))++failures;++checks;
if(action::admit(i,base,owner,true,nullptr))++failures;++checks;
if(action::admit(-1,base,owner,true,"WRONG"))++failures;++checks;
if(action::admit(8,base,owner,true,"WRONG"))++failures;++checks;
}
{int i=action::role(base,base+0x37284,base+0x163C1C);if(i!=4||!action::admit(i,base,owner,true,"LEGS_RIGHTTORS"))++failures;++checks;
if(action::role(base,base+0x37284 + 1,base+0x163C1C)!=-1)++failures;++checks;
if(action::role(base,base+0x37284,base+0x163C1C + 1)!=-1)++failures;++checks;
if(action::admit(i,base,owner+4,true,"LEGS_RIGHTTORS"))++failures;++checks;
if(action::admit(i,base,owner,false,"LEGS_RIGHTTORS"))++failures;++checks;
if(action::admit(i,base,owner,true,"WRONG"))++failures;++checks;
if(action::admit(i,base,owner,true,nullptr))++failures;++checks;
if(action::admit(-1,base,owner,true,"WRONG"))++failures;++checks;
if(action::admit(8,base,owner,true,"WRONG"))++failures;++checks;
}
{int i=action::role(base,base+0x372A5,base+0x163C0C);if(i!=5||!action::admit(i,base,owner,true,"LEGS_LEFTBOK"))++failures;++checks;
if(action::role(base,base+0x372A5 + 1,base+0x163C0C)!=-1)++failures;++checks;
if(action::role(base,base+0x372A5,base+0x163C0C + 1)!=-1)++failures;++checks;
if(action::admit(i,base,owner+4,true,"LEGS_LEFTBOK"))++failures;++checks;
if(action::admit(i,base,owner,false,"LEGS_LEFTBOK"))++failures;++checks;
if(action::admit(i,base,owner,true,"WRONG"))++failures;++checks;
if(action::admit(i,base,owner,true,nullptr))++failures;++checks;
if(action::admit(-1,base,owner,true,"WRONG"))++failures;++checks;
if(action::admit(8,base,owner,true,"WRONG"))++failures;++checks;
}
{int i=action::role(base,base+0x372C6,base+0x163BFC);if(i!=6||!action::admit(i,base,owner,true,"LEGS_RIGHTBOK"))++failures;++checks;
if(action::role(base,base+0x372C6 + 1,base+0x163BFC)!=-1)++failures;++checks;
if(action::role(base,base+0x372C6,base+0x163BFC + 1)!=-1)++failures;++checks;
if(action::admit(i,base,owner+4,true,"LEGS_RIGHTBOK"))++failures;++checks;
if(action::admit(i,base,owner,false,"LEGS_RIGHTBOK"))++failures;++checks;
if(action::admit(i,base,owner,true,"WRONG"))++failures;++checks;
if(action::admit(i,base,owner,true,nullptr))++failures;++checks;
if(action::admit(-1,base,owner,true,"WRONG"))++failures;++checks;
if(action::admit(8,base,owner,true,"WRONG"))++failures;++checks;
}
{int i=action::role(base,base+0x37329,base+0x163BD0);if(i!=7||!action::admit(i,base,owner,true,"LEGS_BLOCK"))++failures;++checks;
if(action::role(base,base+0x37329 + 1,base+0x163BD0)!=-1)++failures;++checks;
if(action::role(base,base+0x37329,base+0x163BD0 + 1)!=-1)++failures;++checks;
if(action::admit(i,base,owner+4,true,"LEGS_BLOCK"))++failures;++checks;
if(action::admit(i,base,owner,false,"LEGS_BLOCK"))++failures;++checks;
if(action::admit(i,base,owner,true,"WRONG"))++failures;++checks;
if(action::admit(i,base,owner,true,nullptr))++failures;++checks;
if(action::admit(-1,base,owner,true,"WRONG"))++failures;++checks;
if(action::admit(8,base,owner,true,"WRONG"))++failures;++checks;
}
printf("checks=%u failures=%u\n",checks,failures);return failures?1:0;}
