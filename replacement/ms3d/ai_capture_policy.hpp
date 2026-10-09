#pragma once
#include <cstring>
namespace damage {
inline bool capturePolicyValid(bool enabled,const char*side,bool directory,bool aiReplace,bool damageOriginal,bool clipOriginal,bool otherOriginal){
 if(!enabled)return true;
 if(!directory || !clipOriginal)return false;
 if(!side)side="";
 if(side[0] && std::strcmp(side,"original") && std::strcmp(side,"candidate"))return false;
 if(std::strcmp(side,"candidate")==0)return aiReplace && !damageOriginal && !otherOriginal;
 return true;
}
}
