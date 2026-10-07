#pragma once
#include <windows.h>
#include <cstdint>
namespace animation::continuation {
struct Admission { void *model=nullptr; DWORD thread=0; unsigned depth=0; bool consumed=false; };
inline thread_local Admission admission;
struct Scope {
    bool admitted=false;
    explicit Scope(void *model) { if(admission.depth==0) { admission={model,GetCurrentThreadId(),1,false}; admitted=true; } else ++admission.depth; }
    ~Scope() { if(admitted)admission={}; else --admission.depth; }
};
inline bool consume(void *model) { if(admission.depth!=1 || admission.consumed || admission.model!=model || admission.thread!=GetCurrentThreadId())return false; admission.consumed=true; return true; }
}
namespace animation {
bool framesSupported(void *model,const void *preparedState);
unsigned nativeReplacementCount();
unsigned nativeContinuationCount();
}
