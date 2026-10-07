#include "../replacement/ms3d/animation.hpp"
#include <cstdio>
#include <cstdint>
#include <cstring>
struct Adapter final:animation::State {
    uint8_t bytes[112]{}; uint32_t time=0; int32_t countValue=1,rate=10;
    unsigned mode=0,countCalls=0,lookupCalls=0; int32_t lookupIndex=0;
    uint32_t get(unsigned n)const{uint32_t v;std::memcpy(&v,bytes+n,4);return v;}
    void put(unsigned n,uint32_t v){std::memcpy(bytes+n,&v,4);}
    int32_t count()override{++countCalls;if(mode==1){put(48,123);time=0x429a0000;}if(mode==3)put(44,0xffffffff);return countValue;}
    uint32_t clock()const override{return time;}
    uint32_t anchor()const override{return get(80);}
    int32_t selector()const override{return int32_t(get(44));}
    int32_t lookupRate(int32_t index)override{++lookupCalls;lookupIndex=index;if(mode==2){put(80,0x43fa0000);put(60,777);time=0x461c3c00;}return rate;}
    uint32_t source(unsigned steps)const override{return get(56+4*steps);}
    void frame(uint32_t v)override{put(48,v);}
    void anchor(uint32_t v)override{put(80,v);}
    void factor(uint32_t v)override{put(76,v);}
};
int main(int argc,char**argv){
    if(argc!=2)return 2;FILE*f=std::fopen(argv[1],"r");if(!f)return 3;
    unsigned cw=0x027f;__asm__ volatile("fldcw %0"::"m"(cw));
    char id[100],hex[225];unsigned time,mode;int rate,count;
    while(std::fscanf(f,"%99s %224s %u %d %d %u",id,hex,&time,&rate,&count,&mode)==6){
        Adapter a;a.time=time;a.rate=rate;a.countValue=count;a.mode=mode;
        for(unsigned i=0;i<112;++i){unsigned v;std::sscanf(hex+2*i,"%2x",&v);a.bytes[i]=uint8_t(v);}
        const uint32_t mx=0x1f80;
        __asm__ volatile("fninit; fldcw %0; ldmxcsr %1"::"m"(cw),"m"(mx));
        const uint32_t result=animation::advance(a);
        unsigned short sw;unsigned mxcsr;__asm__ volatile("fnstsw %0; stmxcsr %1":"=m"(sw),"=m"(mxcsr));
        std::printf("%s %u %u %u %u %d %u %u ",id,result,a.time,a.countCalls,a.lookupCalls,a.lookupIndex,sw,mxcsr);
        for(uint8_t b:a.bytes)std::printf("%02x",unsigned(b));std::puts("");
    }
    std::fclose(f);return 0;
}
