#pragma once
#include "damage.hpp"
#include "damage_fixture_metadata.hpp"
#include <vector>
namespace damage::fixture {
struct Profile {
    uint8_t *actor;uint8_t *player;uint8_t *playerBody;uint8_t *opponentBody;uint8_t *world;
    uint8_t *globals;uint32_t mode=1,absolute=0x3f800000,finalSqrt=0xffffffff,rngOrdinal=0;
    std::vector<uint32_t> sites;
    bool patternXmm=false;
    uint8_t *global(uint32_t rva){return globals+rva;}
    void set(uint32_t rva,uint32_t value){put(global(rva),value);}
    void initialize(const DamageCase &c){
        std::memset(actor,0xa5,232);std::memset(player,0xa5,176);std::memset(playerBody,0xa5,2288);std::memset(opponentBody,0xa5,2288);std::memset(world,0,11000);
        const uint32_t spans[][2]={{0x1761c8,200},{0x1762a0,128},{0x176370,4},{0x175eec,4},{0x184778,80},{0x185594,4},{0x177f90,16},{0x1849a8,4},{0x184a90,4},{0x1853a4,4},{0x1848e0,16},{0x184ae0,16},{0x184c98,16},{0x17f710,7600},{0x171b90,4},{0x175df0,28}};for(auto&s:spans)std::memset(global(s[0]),0xa5,s[1]);
        mode=1;absolute=0x3f800000;finalSqrt=0xffffffff;rngOrdinal=0;sites.clear();
        for(unsigned off:{28u,32u,36u,44u,48u,52u,108u,112u,116u,140u,164u,208u,228u})put(actor+off,0);
        for(unsigned off:{180u,184u,188u,192u,196u})put(actor+off,0x40800000);
        for(unsigned off:{171u,176u,177u,178u})actor[off]=0;actor[204]=1;put(actor+216,1);
        for(uint32_t rva:{0x1761c9u,0x17622cu,0x18479du})*global(rva)=1;
        for(uint32_t rva:{0x17622du,0x17622eu,0x176234u,0x176256u,0x176254u,0x17626cu,0x184794u,0x17628cu,0x177f9eu})*global(rva)=0;
        for(uint32_t rva:{0x176270u,0x176258u,0x176260u,0x176370u,0x17620cu,0x1761fcu,0x177f90u,0x184784u})set(rva,0);
        set(0x1761f8,1);set(0x176224,1);set(0x184788,1);set(0x184790,1);set(0x1847c4,2);set(0x1761cc,0x40000000);set(0x1761d4,0x42c80000);set(0x176238,0x42c80000);set(0x171b90,0x12345678);
        for(unsigned i=0;i<8;++i){set(0x1762a0+16*i,0x3f800000);set(0x1762a4+16*i,0x40000000);set(0x1762a8+16*i,0x3f800000);}
        for(auto *b:{playerBody,opponentBody}){put(b,address(b));put(b+216,0);put(b+244,address(world));put(b+652,0);b[2284]=1;for(unsigned off:{656u,660u,664u,668u,752u,756u,760u,768u,772u,776u,816u,820u,824u,832u,836u,840u,848u,852u,856u})put(b+off,0);put(b+408,0x3f800000);}
        put(opponentBody+752,0x40000000);put(playerBody+2276,address(opponentBody));put(playerBody+2280,0);put(opponentBody+2276,0);put(opponentBody+2280,address(playerBody));put(world+10968,address(playerBody));put(world+10972,address(opponentBody));put(world+10976,2);put(actor+152,address(opponentBody));put(player+152,address(playerBody));set(0x1849a8,address(playerBody));set(0x184a90,address(opponentBody));set(0x1853a4,address(world));
        for(unsigned i=0;i<c.count;++i){auto p=c.patches[i];switch(p.kind){case 0:set(p.offset,p.value);break;case 1:*global(p.offset)=uint8_t(p.value);break;case 2:put(actor+p.offset,p.value);break;case 3:actor[p.offset]=uint8_t(p.value);break;case 4:put(opponentBody+p.offset,p.value);break;case 5:mode=p.value;set(0x184790,p.value);break;case 6:absolute=p.value;break;case 7:finalSqrt=p.value;break;}}
    }
    Result call(const Call &c){
        sites.push_back(c.site);Result r;r.eax=0xabcd1234;auto *out=c.count?static_cast<uint8_t*>(pointer(c.args[0])):nullptr;
        switch(c.id){
        case Callback::Construct:for(unsigned i=0;i<3;++i)put(static_cast<uint8_t*>(c.owner)+4*i,c.args[i]);r.eax=address(c.owner);break;
        case Callback::Clear:for(unsigned off:{8u,4u,0u})put(static_cast<uint8_t*>(c.owner)+off,0);break;
        case Callback::Element:r.eax=address(static_cast<uint8_t*>(c.owner)+4*c.args[0]);break;
        case Callback::Position:std::memcpy(out,static_cast<uint8_t*>(c.owner)+752,16);r.eax=address(out);break;
        case Callback::Velocity:std::memcpy(out,static_cast<uint8_t*>(c.owner)+768,16);r.eax=address(out);break;
        case Callback::Negate:case Callback::Multiply:case Callback::Add:case Callback::ScalarFirstMultiply:put(out,0x3f800000);put(out+4,0);put(out+8,0);put(out+12,0xcccccccc);r.eax=address(out);break;
        case Callback::Impulse:put(static_cast<uint8_t*>(c.owner)+508,0x3f000000);break;
        case Callback::BodyVector:std::memcpy(static_cast<uint8_t*>(c.owner)+508,out,16);break;
        case Callback::Rand:++rngOrdinal;r.eax=(rngOrdinal*17+63)%32768;set(0x171b90,0x89abcdef+rngOrdinal);break;
        case Callback::AudioQuery:put(pointer(c.args[2]),0x1011);break;
        default:break;
        }
        if(c.scalar){if(c.id==Callback::Sqrt){if(c.site==0x1e2b8 && finalSqrt!=0xffffffff)__asm__ volatile("flds %0"::"m"(finalSqrt):"st");else{uint64_t raw=c.args[0]|uint64_t(c.args[1])<<32;__asm__ volatile("fldl %0; fsqrt"::"m"(raw):"st");}}else{uint32_t raw=c.id==Callback::Absolute?absolute:0x3f800000;__asm__ volatile("flds %0"::"m"(raw):"st");}}
        __asm__ volatile("fxsave (%0); fnstenv 512(%0); fldenv 512(%0)"::"r"(r.fp.data()):"memory");if(c.scalar){__asm__ volatile("fstpt %0":"=m"(r.scalar80)::"st");}
        if(patternXmm)for(unsigned i=0;i<128;++i)r.fp[160+i]=uint8_t(sites.size()*17+(i+1)*3);
        __asm__ volatile("fxrstor (%0); fldenv 512(%0)"::"r"(r.fp.data()):"memory","st");return r;
    }
};
}
