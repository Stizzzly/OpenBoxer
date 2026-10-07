#include "world.hpp"
#include <cstring>
#include <cstdio>
namespace world {
static_assert(sizeof(void*)==4);
void* State::address(uint32_t va) const { return image+(va-0x400000); }
uint8_t State::byte(uint32_t va) const { return *static_cast<volatile uint8_t*>(address(va)); }
int32_t State::integer(uint32_t va) const { return *static_cast<volatile int32_t*>(address(va)); }
void* State::pointer(uint32_t va) const { return *static_cast<void* volatile*>(address(va)); }
void State::storeByte(uint32_t va,uint8_t value) { *static_cast<volatile uint8_t*>(address(va))=value; }
void State::store(uint32_t va,uint32_t value) { *static_cast<volatile uint32_t*>(address(va))=value; }
void State::storePointer(uint32_t va,void* value) { *static_cast<void* volatile*>(address(va))=value; }
int32_t run(State s,const Callbacks& c,int32_t selector) {
    s.store(0x585594,0);
    if(s.byte(0x577f9f)) c.audioReset(s.address(0x575e20));
    void* block=c.allocate(0x8d9c4);
    s.storePointer(0x585560,block?c.construct(block):nullptr);
    if(selector>=1 && selector<=8) {
        char name[64]; std::sprintf(name,".\\base\\maps\\%d\\map.ms3d",selector);
        void* model=s.pointer(0x585560);
        auto* table=*static_cast<LoadModel**>(model); table[1](model,name);
    }
    c.reload(s.pointer(0x585560));
    block=c.allocate(0x8d9c4);
    s.storePointer(0x585564,block?c.construct(block):nullptr);
    if(selector>=1 && selector<=8) {
        char name[64]; std::sprintf(name,".\\base\\maps\\%d\\light.ms3d",selector);
        void* model=s.pointer(0x585564);
        auto* table=*static_cast<LoadModel**>(model); table[1](model,name);
    }
    c.reload(s.pointer(0x585564));
    c.lightA(s.pointer(0x585564)); c.lightB(s.pointer(0x585564));
    c.stageA(); c.stageB(); c.stageC();
    if(s.byte(0x577fb8)) c.optional(s.integer(0x584770));
    if(s.integer(0x585594)==0 || s.integer(0x585594)==1) c.stageD();
    s.store(0x575d94,static_cast<uint32_t>(c.random()%4));
    for(int32_t choice=0;choice<4;choice++) {
        if(s.integer(0x575d94)==choice) {
            alignas(4) uint8_t holder[16]; alignas(4) uint8_t scratch[4];
            std::memset(holder,0xcc,16); std::memset(scratch,0xcc,4);
            char name[64]; std::sprintf(name,".\\base\\music\\%d.ogg",choice+1);
            c.holder(holder,name,scratch);
            c.audioLoad(s.address(0x57b2b0),holder,1,1);
            c.holderDestroy(holder);
        }
    }
    if(s.byte(0x577f9f)) c.audioPlay(s.address(0x57b2b0));
    s.store(0x56fe70,0xbfc00000); s.storeByte(0x5853a2,1);
    c.cursor(320,240);
    s.store(0x58477c,0x3f800000); s.store(0x584780,0);
    s.storeByte(0x584794,0); s.store(0x584798,1);
    s.storeByte(0x58479c,0); s.storeByte(0x58479d,0); s.store(0x584778,0);
    if(s.byte(0x577fb8)) {
        if(s.integer(0x584788)==0 && s.integer(0x584798)==1) s.store(0x584778,0xc1800000);
        if(s.integer(0x584788)==0 && s.integer(0x584798)>1) s.store(0x584778,0);
        if(s.integer(0x584788)>0 && s.integer(0x584798)==1) s.store(0x584778,0xc1000000);
        if(s.integer(0x584788)>0 && s.integer(0x584798)>1) s.store(0x584778,0);
    }
    s.store(0x56ff18,0x3f800000); s.store(0x585570,0);
    s.store(0x56ff1c,0x40a00000); s.store(0x56ff20,0x3f800000);
    s.store(0x585574,0xc0000000); s.store(0x56ff24,0x40a00000);
    if(s.integer(0x585594)==1) c.callback2000A(2000);
    if(s.integer(0x585594)==2) c.callback2000B(2000);
    return c.final();
}
}
