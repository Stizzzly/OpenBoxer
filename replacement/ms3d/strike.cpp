#include "strike.hpp"
#include "strike_fp.hpp"
namespace strike {
namespace {
Result invoke(State &s,Callback id,void *owner,std::initializer_list<uint32_t> args={},bool cleanup=false,bool scalar=false){
    Call c{id,owner,{},unsigned(args.size()),cleanup,scalar};unsigned i=0;for(uint32_t a:args)c.args[i++]=a;return s.call(c);
}
void copy16(void *out,const void *in){for(unsigned i=0;i<4;++i)put(static_cast<uint8_t*>(out)+4*i,word(static_cast<const uint8_t*>(in)+4*i));}
uint32_t read(State &s,Global g){return word(s.global(g));}
void write(State &s,Global g,uint32_t v){put(s.global(g),v);}
void *body(State &s){return pointer(word(s.player()+152));}
uint32_t element(State &s,void *v,unsigned index){return word(pointer(invoke(s,Callback::Element,v,{index}).eax));}
void zeroY(State &s,void *v){put(pointer(invoke(s,Callback::Element,v,{1}).eax),0);}
void construct(State &s,void *out,uint32_t x,uint32_t y,uint32_t z){invoke(s,Callback::Construct,out,{x,y,z});}
Vector local(){return {0xcccccccc,0xcccccccc,0xcccccccc,0xcccccccc};}
Result root(State &s,uint64_t input){return invoke(s,Callback::Sqrt,nullptr,{uint32_t(input),uint32_t(input>>32)},true,true);}
bool greater(uint16_t sw){return (sw&0x4500)==0;}
bool less(uint16_t sw){return (sw&0x4500)==0x100;}
}
uint32_t update(State &s){
    uint8_t *p=s.player();Vector playerPosition=local(),opponentPosition=local();
    Result r=invoke(s,Callback::Position,pointer(read(s,Global::PlayerBody)),{address(playerPosition.data())});
    copy16(s.global(Global::PositionPlayer),pointer(r.eax));
    r=invoke(s,Callback::Position,pointer(read(s,Global::OpponentBody)),{address(opponentPosition.data())});
    copy16(s.global(Global::PositionOpponent),pointer(r.eax));
    void *a=s.global(Global::PositionPlayer),*b=s.global(Global::PositionOpponent);
    uint32_t x1=element(s,a,0),x2=element(s,b,0);uint32_t xd=fp::difference(x1,x2);
    x1=element(s,a,0);x2=element(s,b,0);uint32_t xp=fp::differenceProduct(x1,x2,xd);
    uint32_t z1=element(s,a,2),z2=element(s,b,2);uint32_t zd=fp::difference(z1,z2);
    z1=element(s,a,2);z2=element(s,b,2);
    r=root(s,fp::differenceSum(z1,z2,zd,xp));write(s,Global::Distance,fp::scalar32(r));
    copy16(s.global(Global::PlayerVector),p+12);copy16(s.global(Global::DirectionVector),p+28);
    construct(s,p+76,0,0,0);
    const Global keys[]={Global::KeyZ,Global::KeyX,Global::KeyC};
    const Global latches[]={Global::LatchZ,Global::LatchX,Global::LatchC};
    const uint32_t upper[]={0x40800000,0x40a00000,0x40c00000};
    unsigned selected=0;while(selected<3 && !*s.global(keys[selected]))++selected;
    fp::compare32(read(s,Global::OpponentHealth),0);
    *s.global(latches[selected])=1;write(s,Global::Type,selected+1);
    if(greater(fp::compare32(read(s,Global::Distance),0x3f800000)) && less(fp::compare32(read(s,Global::Distance),upper[selected]))){
        *s.global(Global::Pending)=1;Vector impulse=local();construct(s,impulse.data(),0x40400000,0,0);
        invoke(s,Callback::Impulse,body(s),{address(impulse.data())});*s.global(Global::Marker)=1;
    }
    const uint8_t hand=p[168+selected];put(p+164,hand?3+2*selected:4+2*selected);p[168+selected]=hand?0:1;*s.global(Global::Attack)=1;
    p[171]=0;construct(s,p+76,0,0,0);
    fp::compare32(read(s,Global::Timer),word(s.global(Global::Duration)+16*read(s,Global::Fighter)));
    if(selected>=1)fp::scaledCompare(word(s.global(Global::Duration)+16*read(s,Global::Fighter)),0x3fa66666,read(s,Global::Timer));
    if(selected>=2)fp::scaledCompare(word(s.global(Global::Duration)+16*read(s,Global::Fighter)),0x3ff33333,read(s,Global::Timer));
    // Admitted exact-zero timer and positive finite duration take increment.
    write(s,Global::Timer,fp::multiplyAdd(0x3c23d70a,read(s,Global::Dt),read(s,Global::Timer)));
    if(greater(fp::compare32(read(s,Global::Fatigue),0)))write(s,Global::Fatigue,fp::subtractProduct(read(s,Global::Fatigue),0x3e19999a,read(s,Global::Dt)));
    fp::compare32(read(s,Global::Fatigue),0x42c60000);
    fp::compare32(read(s,Global::PlayerHealth),0);
    fp::compare32(read(s,Global::Combo),0x3f19999a);
    invoke(s,Callback::Cursor,p);
    Vector forward=local();construct(s,forward.data(),0,0,0x3f800000);
    Vector direction=local();copy16(direction.data(),p+28);zeroY(s,direction.data());invoke(s,Callback::Normalize,direction.data());
    Vector position=local();r=invoke(s,Callback::Position,body(s),{address(position.data())});copy16(p+12,pointer(r.eax));
    Vector angular=local();construct(s,angular.data(),0,0,0);invoke(s,Callback::AngularZero,body(s),{address(angular.data())});
    Vector velocity=local();r=invoke(s,Callback::Velocity,body(s),{address(velocity.data())});copy16(p+92,pointer(r.eax));
    r=invoke(s,Callback::ValueLength,nullptr,{word(p+92),word(p+96),word(p+100),word(p+104)},true,true);
    const uint32_t length=fp::scalar32(r);r=invoke(s,Callback::Absolute,nullptr,{length},true,true);put(p+172,fp::scalar32(r));
    zeroY(s,p+76);Vector motion=local();
    if(less(fp::scaledCompare(word(p+172),0x447a0000,0x44bb8000)))r=invoke(s,Callback::Add,nullptr,{address(motion.data()),address(p+76),address(p+108)},true);
    else r=invoke(s,Callback::Multiply,nullptr,{address(motion.data()),address(p+76),0x3dcccccd},true);
    copy16(p+124,pointer(r.eax));invoke(s,Callback::BodyVector,body(s),{address(p+124)});
    r=invoke(s,Callback::PointerLength,p+92,{},false,true);put(p+140,fp::scalarProgress(r,0x3ccccccd,read(s,Global::Dt),word(p+140)));
    uint32_t audio=0xcccccccc;invoke(s,Callback::AudioQuery,nullptr,{read(s,Global::Audio),0x1010,address(&audio)},true);
    if(*s.global(Global::Sound)){
        if(p[171] && audio!=0x1012)invoke(s,Callback::AudioPlay,s.global(Global::Audio));
        if(!p[171] && audio==0x1012)invoke(s,Callback::AudioStop,s.global(Global::Audio));
    }
    x1=element(s,p+92,0);x2=element(s,p+92,0);xp=fp::product(x1,x2);
    z1=element(s,p+92,2);z2=element(s,p+92,2);r=root(s,fp::productSum(z1,z2,xp));
    const uint16_t sw=fp::compare80(r,3.0);
    if(greater(sw)){
        zeroY(s,p+92);Vector braking=local();r=invoke(s,Callback::ScalarFirstMultiply,nullptr,{address(braking.data()),0xc0000000,address(p+92)},true);
        return invoke(s,Callback::BodyVector,body(s),{r.eax}).eax;
    }
    return (r.eax&0xffff0000u)|sw;
}
}
