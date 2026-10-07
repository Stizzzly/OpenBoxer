#include "strike.hpp"
#include <map>
#include <vector>
#include <cstdio>
#include <cmath>
#include <cstdlib>
using namespace strike;
struct Event {unsigned id;uint16_t sw,cw;uint8_t tag;uint32_t mxcsr;};
struct Fixture:State {
    std::array<uint8_t,176> p{};
    std::map<uint32_t,std::array<uint8_t,128>> g;
    Vector pp{},op{},vel{};
    std::vector<Event> events;
    unsigned positionCalls=0,sqrtCalls=0,bodyCalls=0;
    uint32_t firstArgumentLo=0,firstArgumentHi=0,absoluteBits=0x3fc00000;
    uint32_t audioState=0x1012; bool mutate=false;
    uint8_t *player()override{return p.data();}
    uint8_t *global(Global id)override{return g[uint32_t(id)].data();}
    void set(Global id,uint32_t v){put(global(id),v);}
    void byte(Global id,uint8_t v){*global(id)=v;}
    Result call(const Call &c)override {
        Result out;
        __asm__ volatile("fxsave %0":"=m"(out.fp));
        Event e{};e.id=unsigned(c.id);std::memcpy(&e.cw,out.fp.data(),2);std::memcpy(&e.sw,out.fp.data()+2,2);e.tag=out.fp[4];std::memcpy(&e.mxcsr,out.fp.data()+24,4);events.push_back(e);
        auto ptr=[&](unsigned i){return pointer(c.args[i]);};
        out.eax=0x12345678;
        switch(c.id){
        case Callback::Construct:
            for(unsigned i=0;i<3;++i)put((uint8_t*)c.owner+4*i,c.args[i]);out.eax=address(c.owner);break;
        case Callback::Element:out.eax=address((uint8_t*)c.owner+4*c.args[0]);break;
        case Callback::Position:{auto &v=positionCalls==1?op:pp;++positionCalls;std::memcpy(ptr(0),v.data(),16);out.eax=c.args[0];break;}
        case Callback::Cursor:
            if(mutate){set(Global::Dt,0x3e800000);put(p.data()+140,0x40e00000);put(p.data()+32,0x40a00000);}break;
        case Callback::Velocity:std::memcpy(ptr(0),vel.data(),16);out.eax=c.args[0];break;
        case Callback::Add:case Callback::Multiply:case Callback::ScalarFirstMultiply:
            for(unsigned i=0;i<4;++i)put((uint8_t*)ptr(0)+4*i,0xabc00000+i+unsigned(c.id)*16);out.eax=c.args[0];break;
        case Callback::BodyVector:++bodyCalls;out.eax=bodyCalls==2?0x7654abcd:0x77881234;break;
        case Callback::AudioQuery:put(ptr(2),audioState);break;
        case Callback::Sqrt: {
            uint64_t raw=(uint64_t(c.args[1])<<32)|c.args[0];double value;std::memcpy(&value,&raw,8);double scalar=std::sqrt(value);
            if(!sqrtCalls){firstArgumentLo=c.args[0];firstArgumentHi=c.args[1];}++sqrtCalls;
            out.eax=sqrtCalls==2?0xd00dbeef:0xbabe1234;
            __asm__ volatile("fldl %0; fstpt %1; fldt %1"::"m"(scalar),"m"(out.scalar80):"st");break;}
        case Callback::ValueLength:case Callback::Absolute:case Callback::PointerLength:{
            uint32_t bits=c.id==Callback::PointerLength?0x40400000:absoluteBits;
            __asm__ volatile("flds %0; fstpt %1; fldt %1"::"m"(bits),"m"(out.scalar80):"st");break;}
        default:break;
        }
        __asm__ volatile("fxsave %0":"=m"(out.fp));
        return out;
    }
};
void dump(const void* p,size_t n){auto b=(const uint8_t*)p;for(size_t i=0;i<n;++i)printf("%02x",b[i]);}
int main(){
    const uint32_t boundaries[3][6]={{0x3f7fffff,0x3f800000,0x3f800001,0x407fffff,0x40800000,0x40800001},
      {0x3f7fffff,0x3f800000,0x3f800001,0x409fffff,0x40a00000,0x40a00001},
      {0x3f7fffff,0x3f800000,0x3f800001,0x40bfffff,0x40c00000,0x40c00001}};
    unsigned id=0;
    for(unsigned key=0;key<3;++key)for(unsigned boundary=0;boundary<6;++boundary)for(unsigned hand=0;hand<2;++hand)for(unsigned variant=0;variant<4;++variant){
        Fixture f;f.p.fill(0xa5);f.op={0,0,0,0xfeedface};f.pp={boundaries[key][boundary],0,0,0xdeadbeef};
        f.vel={variant==3?0x40800000u:variant==2?0x40400000u:0x40000000u,0x3f800000,0,0xabcdef01};
        put(f.p.data()+152,0x11223344);put(f.p.data()+164,0);for(unsigned i=0;i<3;++i)f.p[168+i]=0;f.p[168+key]=hand;f.p[171]=0;put(f.p.data()+140,0x40000000);
        f.set(Global::PlayerBody,0x11223344);f.set(Global::OpponentBody,0x55667788);f.byte(key==0?Global::KeyZ:key==1?Global::KeyX:Global::KeyC,1);
        f.set(Global::Timer,0);f.set(Global::Dt,0x3dcccccd);f.set(Global::Fatigue,variant&1?0x41200000:0);f.set(Global::OpponentHealth,0x42c80000);f.set(Global::PlayerHealth,0x42c80000);f.set(Global::Combo,0);f.set(Global::Duration,0x3f812345);f.set(Global::Fighter,0);f.set(Global::Audio,0x31415926);f.byte(Global::Sound,variant!=0);
        f.audioState=variant==1?0x1011:0x1012;f.absoluteBits=variant==0?0x3fbfffff:variant==1?0x3fc00000:0x3fc00001;f.mutate=variant==3;
        alignas(16)std::array<uint8_t,512> env{};uint16_t cw=0x027f;__asm__ volatile("fninit; fldcw %1; fxsave %0":"=m"(env):"m"(cw));
        // An inherited precision sticky witness and nondefault incoming CC.
        uint16_t initialSw=0x4420;std::memcpy(env.data()+2,&initialSw,2);__asm__ volatile("fxrstor %0"::"m"(env));
        const uint32_t result=update(f);__asm__ volatile("fxsave %0":"=m"(env));uint16_t sw;std::memcpy(&sw,env.data()+2,2);
        printf("{\"id\":%u,\"key\":%u,\"boundary\":%u,\"hand\":%u,\"variant\":%u,\"position\":%u,\"distance\":%u,\"timer\":%u,\"fatigue\":%u,\"accumulator\":%u,\"pending\":%u,\"marker\":%u,\"type\":%u,\"latch\":%u,\"attack\":%u,\"eax\":%u,\"sw\":%u,\"sqrtArgument\":\"%08x%08x\",\"player\":\"",
          id++,key,boundary,hand,variant,boundaries[key][boundary],word(f.global(Global::Distance)),word(f.global(Global::Timer)),word(f.global(Global::Fatigue)),word(f.p.data()+140),*f.global(Global::Pending),*f.global(Global::Marker),word(f.global(Global::Type)),*f.global(key==0?Global::LatchZ:key==1?Global::LatchX:Global::LatchC),*f.global(Global::Attack),result,sw,f.firstArgumentHi,f.firstArgumentLo);
        dump(f.p.data(),f.p.size());printf("\",\"events\":[");
        for(unsigned i=0;i<f.events.size();++i){auto &e=f.events[i];printf("%s[%u,%u,%u,%u,%u]",i?",":"",e.id,e.sw,e.cw,e.tag,e.mxcsr);}printf("]}\n");
    }
}
