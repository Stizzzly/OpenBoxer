#include "frames_continuation.hpp"
#include <cstdio>
using namespace animation::continuation;
int main() {
    char model, other;
    if(consume(&model))return 1;
    {
        Scope outer(&model);
        if(consume(&other))return 2;
        { Scope nested(&model); if(consume(&model))return 3; }
        const DWORD actual=admission.thread;
        admission.thread=actual+1;
        if(consume(&model))return 4;
        admission.thread=actual;
        if(!consume(&model))return 5;
        if(consume(&model))return 6;
    }
    if(consume(&model)||admission.model||admission.depth)return 7;
    {Scope fresh(&other);if(!consume(&other))return 8;}
    if(consume(&other))return 9;
    puts("PASS outside/mismatch/reentrant/thread/single-consumption/stale/fresh-scope");
}
