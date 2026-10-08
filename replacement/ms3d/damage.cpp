#include "damage.hpp"
#include "damage_fp.hpp"
#include <initializer_list>
namespace damage {
namespace {
Result invoke(State&s,uint32_t site,Callback id,void *owner,std::initializer_list<uint32_t> args={},bool cleanup=false,bool scalar=false){Call c{id,site,owner,{},unsigned(args.size()),cleanup,scalar};unsigned i=0;for(auto a:args)c.args[i++]=a;Result result=s.call(c);restoreXmm(result.fp.data()+160);saveXmm(s.liveXmm.data());return result;}
uint32_t read(State&s,uint32_t rva){return word(s.global(rva));}
void write(State&s,uint32_t rva,uint32_t value){put(s.global(rva),value);}
uint8_t byte(State&s,uint32_t rva){return *s.global(rva);}
void copy16(void *out,const void *in){for(unsigned i=0;i<4;++i)put(static_cast<uint8_t*>(out)+4*i,word(static_cast<const uint8_t*>(in)+4*i));}
void *body(State&s){return pointer(word(s.actor()+152));}
void construct(State&s,uint32_t site,void*out,uint32_t x,uint32_t y,uint32_t z){invoke(s,site,Callback::Construct,out,{x,y,z});}
uint32_t *element(State&s,uint32_t site,void*v,unsigned index){return static_cast<uint32_t*>(pointer(invoke(s,site,Callback::Element,v,{index}).eax));}
Result root(State&s,uint32_t site,uint64_t value){return invoke(s,site,Callback::Sqrt,nullptr,{uint32_t(value),uint32_t(value>>32)},true,true);}
void position(State&s,uint32_t site,void*owner,Vector&local,void*out){auto r=invoke(s,site,Callback::Position,owner,{address(local.data())});copy16(out,pointer(r.eax));}
void zeroY(State&s,uint32_t site,void*v){put(element(s,site,v,1),0);}
bool lt(State&s,uint32_t rva,uint32_t rhs){return fp::less(fp::compare(read(s,rva),rhs));}
bool gt(State&s,uint32_t rva,uint32_t rhs){return fp::greater(fp::compare(read(s,rva),rhs));}
}
uint32_t update(State&s,int32_t mode){
    // Own CRT copies/zeroing may use SSE even when compiler SSE is disabled.
    // Track the exact incoming/last opaque XMM effect and restore it at each
    // dependency boundary and after all own temporaries have been destroyed.
    saveXmm(s.liveXmm.data());struct LastOpaqueXmm {State&s;~LastOpaqueXmm(){restoreXmm(s.liveXmm.data());}} xmmReturn{s};
    auto *a=s.actor();
    // All distinct locals persist through the whole invocation, including loop
    // output reuse and opaque fourth words. No per-event re-poisoning occurs.
    std::array<Vector,15> locals;for(auto&v:locals)v.fill(0xcccccccc);
    enum {PosPlayer,PosOpponent,Neg,MoveForward,MoveRetreat,Hit,HitPosition,PoolVelocity,PoolCopy,TailPosition,Zero,TailVelocity,Add,SpeedMultiply,Limit};
    uint32_t audio=0xcccccccc;
    for(unsigned i=0;i<5;++i){const unsigned offsets[]={180,184,188,192,196};const unsigned clear[]={177,176,178};const unsigned off=offsets[i];if(fp::less(fp::compare(word(a+off),0))){if(i<3)a[clear[i]]=0;}else put(a+off,fp::decrement(word(a+off),read(s,0x176370)));}
    int32_t difficulty;
    if(mode==1){for(int32_t i=1;i<=7;++i)if(int32_t(read(s,0x184788))==i)put(a+220,uint32_t(i));difficulty=int32_t(read(s,0x1847c4));}else{put(a+220,3);difficulty=2;}
    position(s,0x1c70b,pointer(read(s,0x1849a8)),locals[PosPlayer],s.global(0x1761dc));
    position(s,0x1c740,pointer(read(s,0x184a90)),locals[PosOpponent],s.global(0x176240));
    void *p=s.global(0x1761dc),*o=s.global(0x176240);
    auto *x1=element(s,0x1c76f,p,0),*x2=element(s,0x1c77d,o,0);uint32_t xd=fp::difference(word(x1),word(x2));
    x1=element(s,0x1c793,p,0);x2=element(s,0x1c7a1,o,0);uint32_t xp=fp::differenceProduct(word(x1),word(x2),xd);
    auto *z1=element(s,0x1c7bd,p,2),*z2=element(s,0x1c7cb,o,2);uint32_t zd=fp::difference(word(z1),word(z2));
    z1=element(s,0x1c7e1,p,2);z2=element(s,0x1c7ef,o,2);auto r=root(s,0x1c80a,fp::differenceSum(word(z1),word(z2),zd,xp));write(s,0x175eec,fp::scalar32());
    copy16(s.global(0x184ae0),a+12);copy16(s.global(0x184c98),a+28);
    // Native preconditions and callback stability guarantee this active gate.
    const bool active=!byte(s,0x184794) && byte(s,0x18479d);(void)active;
    if((!a[204] || byte(s,0x176256)) && (gt(s,0x175eec,0x40c00000) || fp::greater(fp::compare(word(element(s,0x1c8be,o,0)),0x41180000)))){a[176]=1;a[177]=0;a[178]=0;put(a+180,0);put(a+188,0);}
    if((byte(s,0x17622e) || ((!a[204] || byte(s,0x176256)) && lt(s,0x175eec,0x40c00000))) && lt(s,0x175eec,0x41000000) && fp::less(fp::compare(word(element(s,0x1c95e,o,0)),0x41180000))){a[177]=1;a[176]=0;a[178]=0;put(a+184,0);put(a+188,0);}
    if(a[204] && !byte(s,0x176256)){a[178]=1;a[177]=0;a[176]=0;put(a+180,0);put(a+184,0);}
    uint32_t choice=0xcccccccc;if(a[176])choice=0;else if(a[177])choice=2;else if(a[178])choice=1;
    if(a[176])put(a+184,0x40800000);if(a[177])put(a+180,0x40800000);if(a[178])put(a+188,0x40800000);
    if(choice==1 && !byte(s,0x17622d) && !byte(s,0x176234)){
        r=invoke(s,0x1cac6,Callback::Negate,nullptr,{address(locals[Neg].data()),address(a+44)},true);
        r=invoke(s,0x1cad6,Callback::Multiply,nullptr,{address(locals[MoveForward].data()),r.eax,0x41280000},true);copy16(a+76,pointer(r.eax));put(a+164,1);
    }else if(choice==2 && !byte(s,0x17622d) && !byte(s,0x176234) && lt(s,0x175eec,0x41000000) && fp::less(fp::compare(word(element(s,0x1cb47,o,0)),0x41700000))){
        put(a+164,2);r=invoke(s,0x1cb7b,Callback::Multiply,nullptr,{address(locals[MoveRetreat].data()),address(a+44),0x41280000},true);copy16(a+76,pointer(r.eax));
    }else{construct(s,0x1cbad,a+76,0,0,0);invoke(s,0x1cbb8,Callback::Clear,a+124);if(word(a+164)==1 || word(a+164)==2)put(a+164,0);}
    if(fp::less(fp::compare(word(a+192),0x3f800000))){a[204]=1;put(a+200,0);unsigned site=difficulty==1?0x1cc19:(difficulty<=3?0x1cc3e:0x1cc67);r=invoke(s,site,Callback::Rand,nullptr,{},true);const int32_t divisor=difficulty==1?3:(difficulty<=3?4:5);put(a+224,uint32_t(int32_t(r.eax)%divisor+1));}
    const uint32_t upper[]={0x40800000,0x40a00000,0x40c00000};
    for(int32_t type=1;type<=3;++type){if(read(s,0x177f90)!=0)continue;if(fp::less(fp::compare(read(s,0x1761d4),0)))continue;if(byte(s,0x17622d) || byte(s,0x17622e) || byte(s,0x17622c) || byte(s,0x176256) || byte(s,0x176234) || !a[204] || int32_t(word(a+216))!=type)continue;if(!gt(s,0x175eec,0x3f800000))continue;lt(s,0x175eec,upper[type-1]);/* admitted far miss cannot initiate */}
    // Both existing recoil byte tests have no admitted continuation.
    const uint8_t recoilFirst=byte(s,0x17622e),recoilSecond=byte(s,0x17622e);(void)recoilFirst;(void)recoilSecond;
    if(byte(s,0x17622c) && fp::less(fp::scaledCompare(read(s,0x1762a4+16*read(s,0x184784)),0x3f000000,read(s,0x1761cc)))){
        *s.global(0x17622c)=0;if(!byte(s,0x176234)){*s.global(0x17622e)=1;put(a+164,9);}
        if(read(s,0x176224)!=read(s,0x1761f8) && !byte(s,0x176234)){write(s,0x17620c,read(s,0x17620c)+1);write(s,0x176224,read(s,0x1761f8));*s.global(0x176208)=1;}
        write(s,0x1761fc,0);write(s,0x176260,0x3fcccccd);
        construct(s,0x1d775,locals[Hit].data(),0x3f000000,0,0);invoke(s,0x1d7bf,Callback::Impulse,body(s),{address(locals[Hit].data())});
        if(byte(s,0x177f9e)){if(byte(s,0x176234))invoke(s,0x1d7ed,Callback::AudioPlay,s.global(0x178070));else invoke(s,0x1d7e1,Callback::AudioPlay,s.global(0x178050));if(!byte(s,0x176234)){if(read(s,0x184788)==5)invoke(s,0x1d80b,Callback::AudioPlay,s.global(0x185340));else invoke(s,0x1d817,Callback::AudioPlay,s.global(0x185320));}}
        if(!fp::less(fp::compare(read(s,0x176238),0)))write(s,0x176238,fp::damage(read(s,0x176238),byte(s,0x176234)?0x3ecccccd:0x40000000,read(s,0x1762a0+16*read(s,0x184784)),read(s,0x1762a8+16*read(s,0x184788))));
        if(!byte(s,0x176234)){
            position(s,0x1d900,body(s),locals[HitPosition],s.global(0x1848e0));void *effect=s.global(0x1848e0);
            uint32_t value=fp::add(word(element(s,0x1d92f,effect,1)),0x3f19999a);put(element(s,0x1d949,effect,1),value);
            value=fp::difference(word(element(s,0x1d95d,effect,0)),0x3e99999a);put(element(s,0x1d977,effect,0),value);
            value=fp::difference(word(element(s,0x1d98b,effect,2)),0x3e4ccccd);put(element(s,0x1d9a5,effect,2),value);
            for(unsigned i=0;i<100;++i){*s.global(0x17f710+76*i)=1;r=invoke(s,0x1d9e8,Callback::Velocity,pointer(read(s,0x184a90)),{address(locals[PoolVelocity].data())});copy16(locals[PoolCopy].data(),pointer(r.eax));r=invoke(s,0x1da04,Callback::Rand,nullptr,{},true);write(s,0x17f738+76*i,fp::pool(int32_t(r.eax)%100,0x3f800000,0x42700000));r=invoke(s,0x1da3b,Callback::Rand,nullptr,{},true);write(s,0x17f73c+76*i,fp::pool(int32_t(r.eax)%100,0x3c23d70a,0));r=invoke(s,0x1da6c,Callback::Rand,nullptr,{},true);write(s,0x17f740+76*i,fp::pool(int32_t(r.eax)%100,0x3f800000,0));}
        }
    }
    if(fp::less(fp::compare(word(a+196),0x3f800000)))put(a+208,1);
    put(a+212,fp::cooldown(difficulty));
    if(byte(s,0x17628c) && lt(s,0x175eec,0x40c00000) && word(a+208) && !read(s,0x177f90) && !byte(s,0x17622d)){*s.global(0x176234)=1;put(a+228,0);*s.global(0x17628c)=0;put(a+164,11);write(s,0x1761fc,0x3fcccccd);}
    if(byte(s,0x176234)){if(!fp::greater(fp::compare(word(a+228),0x3eb33333)))put(a+228,fp::multiplyAdd(0x3c23d70a,read(s,0x176370),word(a+228)));else{*s.global(0x176234)=0;put(a+208,0);put(a+228,0);put(a+164,0);put(a+196,word(a+212));}}
    if(byte(s,0x1761c9) && !byte(s,0x176234))*s.global(0x17628c)=0;
    if(choice!=0 && choice!=1 && choice!=2){a[171]=0;construct(s,0x1dc28,a+76,0,0,0);}
    // The four excluded AIattack arms read their governing byte independently.
    for(unsigned i=0;i<4;++i){const auto attack=byte(s,0x17622d);(void)attack;}
    if(gt(s,0x176258,0))write(s,0x176258,fp::subtractProduct(read(s,0x176258),0x3e19999a,read(s,0x176370)));
    fp::compare(read(s,0x176258),0x42c60000);const auto lock=byte(s,0x176256);(void)lock;
    fp::compare(read(s,0x176238),0);const auto victory=byte(s,0x176254),combo=byte(s,0x17626c);(void)victory;(void)combo;
    if(gt(s,0x176260,0x3f19999a)){*s.global(0x17626c)=0;write(s,0x176260,0);const int32_t count=int32_t(read(s,0x176270));const bool two=count==2,three=count==3,four=count>=4,more=count>1;(void)two;(void)three;(void)four;(void)more;write(s,0x176270,0);write(s,0x176288,15);}
    invoke(s,0x1e037,Callback::Cursor,a);
    position(s,0x1e04c,body(s),locals[TailPosition],a+12);
    construct(s,0x1e079,locals[Zero].data(),0,0,0);invoke(s,0x1e08e,Callback::AngularZero,body(s),{address(locals[Zero].data())});
    r=invoke(s,0x1e0a3,Callback::Velocity,body(s),{address(locals[TailVelocity].data())});copy16(a+92,pointer(r.eax));
    r=invoke(s,0x1e0e5,Callback::ValueLength,nullptr,{word(a+92),word(a+96),word(a+100),word(a+104)},true,true);uint32_t length=fp::scalar32();r=invoke(s,0x1e0f0,Callback::Absolute,nullptr,{length},true,true);put(a+172,fp::scalar32());
    zeroY(s,0x1e109,a+76);if(fp::less(fp::scaledCompare(word(a+172),0x447a0000,0x44bb8000)))r=invoke(s,0x1e145,Callback::Add,nullptr,{address(locals[Add].data()),address(a+76),address(a+108)},true);else r=invoke(s,0x1e17e,Callback::Multiply,nullptr,{address(locals[SpeedMultiply].data()),address(a+76),0x3dcccccd},true);copy16(a+124,pointer(r.eax));
    invoke(s,0x1e1b2,Callback::BodyVector,body(s),{address(a+124)});r=invoke(s,0x1e1bd,Callback::PointerLength,a+92,{},false,true);put(a+140,fp::progress(read(s,0x176370),word(a+140)));
    invoke(s,0x1e1f4,Callback::AudioQuery,nullptr,{read(s,0x175df0),0x1010,address(&audio)},true);
    if(byte(s,0x177f9e) && a[171] && audio!=0x1012)invoke(s,0x1e230,Callback::AudioPlay,s.global(0x175df0));
    if(byte(s,0x177f9e) && !a[171] && audio==0x1012)invoke(s,0x1e261,Callback::AudioStop,s.global(0x175df0));
    x1=element(s,0x1e26e,a+92,0);x2=element(s,0x1e27d,a+92,0);xp=fp::product(word(x1),word(x2));z1=element(s,0x1e294,a+92,2);z2=element(s,0x1e2a3,a+92,2);r=root(s,0x1e2b8,fp::productSum(word(z1),word(z2),xp));const uint16_t sw=fp::finalCompare();
    if(fp::greater(sw)){zeroY(s,0x1e2d5,a+92);r=invoke(s,0x1e2f3,Callback::ScalarFirstMultiply,nullptr,{address(locals[Limit].data()),0xc0000000,address(a+92)},true);return invoke(s,0x1e305,Callback::BodyVector,body(s),{r.eax}).eax;}
    return (r.eax&0xffff0000u)|sw;
}
}
